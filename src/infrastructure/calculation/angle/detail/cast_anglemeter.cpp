#include "cast_anglemeter.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <random>
#include <utility>
#include <vector>

namespace {

constexpr float kPi = 3.14159265358979323846f;

inline posf_t toPosF(const uint16_t x, const uint16_t y) noexcept {
    return posf_t{ static_cast<float>(x), static_cast<float>(y) };
}

inline int fastBrightness(const uint8_t r, const uint8_t g, const uint8_t b) noexcept {
    // Быстрая аппроксимация деления на 3.
    return ((static_cast<int>(r) + static_cast<int>(g) + static_cast<int>(b)) * 21845) >> 16;
}

inline float angleDiffDeg(float a, float b) noexcept {
    float d = std::fmod(std::fabs(a - b), 360.0f);
    return (d > 180.0f) ? (360.0f - d) : d;
}

inline float angleMeanDeg(float a, float b) noexcept {
    const float ra = a * kPi / 180.0f;
    const float rb = b * kPi / 180.0f;

    const float s = std::sin(ra) + std::sin(rb);
    const float c = std::cos(ra) + std::cos(rb);

    float out = std::atan2(s, c) * 180.0f / kPi;
    if (out < 0.0f) out += 360.0f;
    return out;
}

struct Scan1D {
    uint16_t lo = 0;
    uint16_t hi = 0;
    uint32_t stamp = 0;

    [[nodiscard]] bool valid(const uint32_t generation) const noexcept {
        return stamp == generation;
    }

    void set(const int lo_, const int hi_, const uint32_t generation) noexcept {
        lo = static_cast<uint16_t>(lo_);
        hi = static_cast<uint16_t>(hi_);
        stamp = generation;
    }
};

struct Edge1D {
    int pos = 0;
    int diff = 0;
};

struct Candidate1D {
    int lo = 0;
    int hi = 0;
    float bg1 = 0.0f;
    float bg2 = 0.0f;
    float avgDark = 0.0f;
};

struct LineValidation {
    bool ok = false;
    int count = 0;
    float mse = std::numeric_limits<float>::infinity();
};

class AngleMeterImpl final {
public:
    explicit AngleMeterImpl(anglemeter_config cfg)
        : config_(std::move(cfg))
        , rng_(1234567u) {
        prepareScratch();
    }

    void setConfig(anglemeter_config cfg) {
        config_ = std::move(cfg);
        prepareScratch();
    }

    void setImageSize(const int width, const int height) {
        if (width == img_width_ && height == img_height_) {
            return;
        }

        if (width <= 0 || height <= 0) {
            img_width_ = 0;
            img_height_ = 0;
            gray_.clear();
            col_scans_.clear();
            row_scans_.clear();
            return;
        }

        img_width_ = width;
        img_height_ = height;

        const size_t width_sz = static_cast<size_t>(img_width_);
        const size_t height_sz = static_cast<size_t>(img_height_);
        if (width_sz > (std::numeric_limits<size_t>::max() / height_sz)) {
            img_width_ = 0;
            img_height_ = 0;
            gray_.clear();
            col_scans_.clear();
            row_scans_.clear();
            return;
        }

        const size_t pixel_count = static_cast<size_t>(img_width_) * static_cast<size_t>(img_height_);
        gray_.resize(pixel_count);

        col_scans_.resize(static_cast<size_t>(img_width_));
        row_scans_.resize(static_cast<size_t>(img_height_));

        const size_t max_points = static_cast<size_t>(std::max(img_width_, img_height_));
        points_edge_1_.reserve(max_points);
        points_edge_2_.reserve(max_points);
        points_center_.reserve(max_points);

        ransac_order_.reserve(max_points);
        ransac_scores_.reserve(max_points);
        ransac_inliers_.reserve(max_points);
    }

    void setAngleTransformation(float (*func_ptr)(float)) noexcept {
        transform_angle_ = func_ptr;
    }

    bool getArrowAngle(const unsigned char* img, float* angle) {
        if (img == nullptr || angle == nullptr) {
            return false;
        }
        if (img_width_ <= 0 || img_height_ <= 0) {
            return false;
        }

        buildGray(img);

        points_edge_1_.clear();
        points_edge_2_.clear();
        points_center_.clear();

        active_col_generation_ = 0;
        active_row_generation_ = 0;

        int dir = 0;
        if (!scan(dir)) {
            return false;
        }

        collectPoints(dir);
        if (points_center_.size() < 2) {
            return false;
        }

        float out_angle = 0.0f;
        if (!estimateAngle(dir, out_angle)) {
            return false;
        }

        last_angle_deg_ = out_angle;
        *angle = out_angle;
        return true;
    }

private:
    struct LineModel {
        float nx = 0.0f;
        float ny = 0.0f;
        posf_t ref{};
    };

private:
    int img_width_ = 0;
    int img_height_ = 0;

    anglemeter_config config_{};
    float last_angle_deg_ = 0.0f;
    float (*transform_angle_)(float) = nullptr;

    std::vector<uint8_t> gray_;
    std::vector<Scan1D> col_scans_;
    std::vector<Scan1D> row_scans_;

    std::vector<posf_t> points_edge_1_;
    std::vector<posf_t> points_edge_2_;
    std::vector<posf_t> points_center_;

    // Scratch buffers для scanRow/scanCol
    std::vector<Edge1D> mins_;
    std::vector<Edge1D> maxs_;
    std::vector<Candidate1D> candidates_;

    // Scratch buffers для robust fit
    std::vector<int> ransac_order_;
    std::vector<float> ransac_scores_;
    std::vector<int> ransac_inliers_;

    std::mt19937 rng_;

    uint32_t col_generation_counter_ = 1;
    uint32_t row_generation_counter_ = 1;
    uint32_t active_col_generation_ = 0;
    uint32_t active_row_generation_ = 0;

private:
    void prepareScratch() {
        const size_t mp = static_cast<size_t>(std::max(1, config_.max_pairs));
        mins_.reserve(mp);
        maxs_.reserve(mp);
        candidates_.reserve(mp * mp);
    }

    uint32_t nextColGeneration() {
        ++col_generation_counter_;
        if (col_generation_counter_ == 0) {
            col_generation_counter_ = 1;
            for (auto& s : col_scans_) s.stamp = 0;
        }
        return col_generation_counter_;
    }

    uint32_t nextRowGeneration() {
        ++row_generation_counter_;
        if (row_generation_counter_ == 0) {
            row_generation_counter_ = 1;
            for (auto& s : row_scans_) s.stamp = 0;
        }
        return row_generation_counter_;
    }

    void buildGray(const unsigned char* img) {
        const size_t pixel_count = static_cast<size_t>(img_width_) * static_cast<size_t>(img_height_);
        if (gray_.size() < pixel_count) {
            gray_.resize(pixel_count);
        }

        const unsigned char* p = img;

        for (size_t i = 0; i < pixel_count; ++i, p += 3) {
            gray_[i] = static_cast<uint8_t>(fastBrightness(p[0], p[1], p[2]));
        }
    }

    [[nodiscard]] inline int grayAt(const int x, const int y) const noexcept {
        return gray_[static_cast<size_t>(y) * static_cast<size_t>(img_width_) + static_cast<size_t>(x)];
    }

    static void pushBestMinEdge(std::vector<Edge1D>& dst, const int max_pairs, const int pos, const int diff) {
        if (static_cast<int>(dst.size()) < max_pairs) {
            dst.push_back({ pos, diff });
            return;
        }

        auto worst = std::max_element(
            dst.begin(), dst.end(),
            [](const Edge1D& a, const Edge1D& b) { return a.diff < b.diff; }
        );

        if (diff < worst->diff) {
            *worst = Edge1D{ pos, diff };
        }
    }

    static void pushBestMaxEdge(std::vector<Edge1D>& dst, const int max_pairs, const int pos, const int diff) {
        if (static_cast<int>(dst.size()) < max_pairs) {
            dst.push_back({ pos, diff });
            return;
        }

        auto worst = std::min_element(
            dst.begin(), dst.end(),
            [](const Edge1D& a, const Edge1D& b) { return a.diff < b.diff; }
        );

        if (diff > worst->diff) {
            *worst = Edge1D{ pos, diff };
        }
    }

    float meanCol(const int x, int y0, int y1) const {
        const int step2 = config_.scan_step * 2;
        y0 = std::max(y0, 0);
        y1 = std::min(y1, img_height_ - 1);
        if (y0 > y1 || x < 0 || x >= img_width_) {
            return 0.0f;
        }
        if (y0 < 0 || y1 >= img_height_ - step2 + step2) {
            return 0.0f;
        }

        int sum = 0;
        int cnt = 0;
        for (int y = y0; y <= y1; ++y) {
            sum += grayAt(x, y);
            ++cnt;
        }

        return (cnt > 0) ? (static_cast<float>(sum) / static_cast<float>(cnt)) : 0.0f;
    }

    float meanRow(const int y, int x0, int x1) const {
        x0 = std::max(x0, 0);
        x1 = std::min(x1, img_width_ - 1);
        if (x0 > x1 || y < 0 || y >= img_height_) {
            return 0.0f;
        }

        const size_t base = static_cast<size_t>(y) * static_cast<size_t>(img_width_);
        int sum = 0;
        int cnt = 0;

        for (int x = x0; x <= x1; ++x) {
            sum += gray_[base + static_cast<size_t>(x)];
            ++cnt;
        }

        return (cnt > 0) ? (static_cast<float>(sum) / static_cast<float>(cnt)) : 0.0f;
    }

    bool scanCol(const int x, int y_from, int y_to, const uint32_t generation) {
        if (x < 0 || x >= img_width_) {
            return false;
        }

        const int step = config_.scan_step;
        const int step2 = step * 2;
        const int max_pairs = std::max(1, config_.max_pairs);

        const int y_min = std::max(step2, y_from);
        const int y_max = std::min(img_height_ - step2 - 1, y_to);

        if (y_min > y_max) {
            return false;
        }

        mins_.clear();
        maxs_.clear();
        candidates_.clear();

        for (int y = y_min; y <= y_max; ++y) {
            const int br1 = grayAt(x, y - step2);
            const int br2 = grayAt(x, y);
            const int dBr = br2 - br1;
            const int edge_pos = y - step;

            if (dBr < -config_.bright_lim) {
                pushBestMinEdge(mins_, max_pairs, edge_pos, dBr);
            }
            if (dBr > config_.bright_lim) {
                pushBestMaxEdge(maxs_, max_pairs, edge_pos, dBr);
            }
        }

        if (mins_.empty() || maxs_.empty()) {
            return false;
        }

        std::sort(
            mins_.begin(), mins_.end(),
            [](const Edge1D& a, const Edge1D& b) { return a.diff < b.diff; }
        );
        std::sort(
            maxs_.begin(), maxs_.end(),
            [](const Edge1D& a, const Edge1D& b) { return a.diff > b.diff; }
        );

        for (const Edge1D& min_e : mins_) {
            for (const Edge1D& max_e : maxs_) {
                const int span = max_e.pos - min_e.pos;
                if (span < config_.min_object_width || span > config_.max_object_width) {
                    continue;
                }

                const float avg_dark = meanCol(x, min_e.pos, max_e.pos);
                if (avg_dark >= static_cast<float>(config_.bright_lim)) {
                    continue;
                }

                const float bg_up = meanCol(x, min_e.pos - config_.background_probe, min_e.pos - 1);
                const float bg_down = meanCol(x, max_e.pos + 1, max_e.pos + config_.background_probe);

                if (bg_up < static_cast<float>(config_.background_min) ||
                    bg_down < static_cast<float>(config_.background_min)) {
                    continue;
                }

                if ((bg_up - avg_dark) < static_cast<float>(config_.min_background_delta) ||
                    (bg_down - avg_dark) < static_cast<float>(config_.min_background_delta)) {
                    continue;
                }

                candidates_.push_back(Candidate1D{
                    min_e.pos,
                    max_e.pos,
                    bg_up,
                    bg_down,
                    avg_dark
                });
            }
        }

        if (candidates_.empty()) {
            return false;
        }

        const auto score = [](const Candidate1D& c) noexcept {
            // Немного учитываем и общий светлый фон, и контраст.
            return (c.bg1 + c.bg2) + 0.5f * std::min(c.bg1 - c.avgDark, c.bg2 - c.avgDark);
        };

        const auto best = std::max_element(
            candidates_.begin(), candidates_.end(),
            [&](const Candidate1D& a, const Candidate1D& b) {
                return score(a) < score(b);
            }
        );

        col_scans_[static_cast<size_t>(x)].set(best->lo, best->hi, generation);
        return true;
    }

    bool scanRow(const int y, int x_from, int x_to, const uint32_t generation) {
        if (y < 0 || y >= img_height_) {
            return false;
        }

        const int step = config_.scan_step;
        const int step2 = step * 2;
        const int max_pairs = std::max(1, config_.max_pairs);

        const int x_min = std::max(step2, x_from);
        const int x_max = std::min(img_width_ - step2 - 1, x_to);

        if (x_min > x_max) {
            return false;
        }

        mins_.clear();
        maxs_.clear();
        candidates_.clear();

        for (int x = x_min; x <= x_max; ++x) {
            const int br1 = grayAt(x - step2, y);
            const int br2 = grayAt(x, y);
            const int dBr = br2 - br1;
            const int edge_pos = x - step;

            if (dBr < -config_.bright_lim) {
                pushBestMinEdge(mins_, max_pairs, edge_pos, dBr);
            }
            if (dBr > config_.bright_lim) {
                pushBestMaxEdge(maxs_, max_pairs, edge_pos, dBr);
            }
        }

        if (mins_.empty() || maxs_.empty()) {
            return false;
        }

        std::sort(
            mins_.begin(), mins_.end(),
            [](const Edge1D& a, const Edge1D& b) { return a.diff < b.diff; }
        );
        std::sort(
            maxs_.begin(), maxs_.end(),
            [](const Edge1D& a, const Edge1D& b) { return a.diff > b.diff; }
        );

        for (const Edge1D& min_e : mins_) {
            for (const Edge1D& max_e : maxs_) {
                const int span = max_e.pos - min_e.pos;
                if (span < config_.min_object_width || span > config_.max_object_width) {
                    continue;
                }

                const float avg_dark = meanRow(y, min_e.pos, max_e.pos);
                if (avg_dark >= static_cast<float>(config_.bright_lim)) {
                    continue;
                }

                const float bg_left = meanRow(y, min_e.pos - config_.background_probe, min_e.pos - 1);
                const float bg_right = meanRow(y, max_e.pos + 1, max_e.pos + config_.background_probe);

                if (bg_left < static_cast<float>(config_.background_min) ||
                    bg_right < static_cast<float>(config_.background_min)) {
                    continue;
                }

                if ((bg_left - avg_dark) < static_cast<float>(config_.min_background_delta) ||
                    (bg_right - avg_dark) < static_cast<float>(config_.min_background_delta)) {
                    continue;
                }

                candidates_.push_back(Candidate1D{
                    min_e.pos,
                    max_e.pos,
                    bg_left,
                    bg_right,
                    avg_dark
                });
            }
        }

        if (candidates_.empty()) {
            return false;
        }

        const auto score = [](const Candidate1D& c) noexcept {
            return (c.bg1 + c.bg2) + 0.5f * std::min(c.bg1 - c.avgDark, c.bg2 - c.avgDark);
        };

        const auto best = std::max_element(
            candidates_.begin(), candidates_.end(),
            [&](const Candidate1D& a, const Candidate1D& b) {
                return score(a) < score(b);
            }
        );

        row_scans_[static_cast<size_t>(y)].set(best->lo, best->hi, generation);
        return true;
    }

    void trackColsDirectional(
        const uint32_t generation,
        const int x_start,
        const int x_stop,
        const int x_step,
        int y_from_init,
        int y_to_init
    ) {
        if (x_step != -1 && x_step != 1) {
            return;
        }

        const int step2 = config_.scan_step * 2;

        int last_lo = std::max(step2, y_from_init);
        int last_hi = std::min(img_height_ - step2 - 1, y_to_init);
        int stable_fail_count = 0;

        for (int x = x_start + x_step;
             (x_step < 0 ? x >= x_stop : x <= x_stop);
             x += x_step) {

            const int y_from = std::max(step2, last_lo - config_.track_margin);
            const int y_to = std::min(img_height_ - step2 - 1, last_hi + config_.track_margin);

            if (!scanCol(x, y_from, y_to, generation)) {
                ++stable_fail_count;
                if (stable_fail_count >= config_.stable_fail_limit) {
                    break;
                }
                continue;
            }

            stable_fail_count = 0;

            const Scan1D& s = col_scans_[static_cast<size_t>(x)];
            last_lo = static_cast<int>(s.lo);
            last_hi = static_cast<int>(s.hi);
        }
    }

    void trackRowsDirectional(
        const uint32_t generation,
        const int y_start,
        const int y_stop,
        const int y_step,
        int x_from_init,
        int x_to_init
    ) {
        if (y_step != -1 && y_step != 1) {
            return;
        }

        const int step2 = config_.scan_step * 2;

        int last_lo = std::max(step2, x_from_init);
        int last_hi = std::min(img_width_ - step2 - 1, x_to_init);
        int stable_fail_count = 0;

        for (int y = y_start + y_step;
             (y_step < 0 ? y >= y_stop : y <= y_stop);
             y += y_step) {

            const int x_from = std::max(step2, last_lo - config_.track_margin);
            const int x_to = std::min(img_width_ - step2 - 1, last_hi + config_.track_margin);

            if (!scanRow(y, x_from, x_to, generation)) {
                ++stable_fail_count;
                if (stable_fail_count >= config_.stable_fail_limit) {
                    break;
                }
                continue;
            }

            stable_fail_count = 0;

            const Scan1D& s = row_scans_[static_cast<size_t>(y)];
            last_lo = static_cast<int>(s.lo);
            last_hi = static_cast<int>(s.hi);
        }
    }

    LineValidation validateColsRange(const int x_min, const int x_max, const uint32_t generation) const {
        LineValidation out{};

        double sum_x = 0.0;
        double sum_y = 0.0;
        double sum_xx = 0.0;
        double sum_xy = 0.0;
        double sum_y2 = 0.0;
        int n = 0;

        for (int x = x_min; x <= x_max; ++x) {
            const Scan1D& s = col_scans_[static_cast<size_t>(x)];
            if (!s.valid(generation)) {
                continue;
            }

            const double y = 0.5 * (static_cast<double>(s.lo) + static_cast<double>(s.hi));

            sum_x += static_cast<double>(x);
            sum_y += y;
            sum_xx += static_cast<double>(x) * static_cast<double>(x);
            sum_xy += static_cast<double>(x) * y;
            sum_y2 += y * y;
            ++n;
        }

        out.count = n;
        if (n < config_.min_track_points) {
            return out;
        }

        const double denom = static_cast<double>(n) * sum_xx - sum_x * sum_x;
        if (std::fabs(denom) < 1e-12) {
            return out;
        }

        const double a = (static_cast<double>(n) * sum_xy - sum_x * sum_y) / denom;
        const double b = (sum_y - a * sum_x) / static_cast<double>(n);

        const double mse =
            (sum_y2
            + a * a * sum_xx
            + static_cast<double>(n) * b * b
            + 2.0 * a * b * sum_x
            - 2.0 * a * sum_xy
            - 2.0 * b * sum_y) / static_cast<double>(n);

        out.mse = static_cast<float>(mse);
        out.ok = (mse <= static_cast<double>(config_.regression_mse_limit));
        return out;
    }

    LineValidation validateRowsRange(const int y_min, const int y_max, const uint32_t generation) const {
        LineValidation out{};

        double sum_y = 0.0;
        double sum_x = 0.0;
        double sum_yy = 0.0;
        double sum_xy = 0.0;
        double sum_x2 = 0.0;
        int n = 0;

        for (int y = y_min; y <= y_max; ++y) {
            const Scan1D& s = row_scans_[static_cast<size_t>(y)];
            if (!s.valid(generation)) {
                continue;
            }

            const double x = 0.5 * (static_cast<double>(s.lo) + static_cast<double>(s.hi));

            sum_y += static_cast<double>(y);
            sum_x += x;
            sum_yy += static_cast<double>(y) * static_cast<double>(y);
            sum_xy += static_cast<double>(y) * x;
            sum_x2 += x * x;
            ++n;
        }

        out.count = n;
        if (n < config_.min_track_points) {
            return out;
        }

        const double denom = static_cast<double>(n) * sum_yy - sum_y * sum_y;
        if (std::fabs(denom) < 1e-12) {
            return out;
        }

        const double a = (static_cast<double>(n) * sum_xy - sum_y * sum_x) / denom;
        const double b = (sum_x - a * sum_y) / static_cast<double>(n);

        const double mse =
            (sum_x2
            + a * a * sum_yy
            + static_cast<double>(n) * b * b
            + 2.0 * a * b * sum_y
            - 2.0 * a * sum_xy
            - 2.0 * b * sum_x) / static_cast<double>(n);

        out.mse = static_cast<float>(mse);
        out.ok = (mse <= static_cast<double>(config_.regression_mse_limit));
        return out;
    }

    bool scanCols(int& dir_out) {
        const int margin = config_.scan_step * 2;
        if (img_width_ <= margin * 2 || img_height_ <= margin * 2) {
            return false;
        }

        const int center_left = img_width_ / 4;
        const int center_right = (3 * img_width_) / 4;
        const std::vector<int> fallback_offsets{ 0 };
        const auto& offsets = config_.offsets.empty() ? fallback_offsets : config_.offsets;

        auto tryHalf = [&](const int x_start, const int x_min, const int x_max, const uint32_t gen) -> LineValidation {
            if (x_start < margin || x_start >= img_width_ - margin) {
                return {};
            }

            if (!scanCol(x_start, margin, img_height_ - margin - 1, gen)) {
                return {};
            }

            const Scan1D& s = col_scans_[static_cast<size_t>(x_start)];
            const int lo = std::max(margin, static_cast<int>(s.lo) - config_.track_margin);
            const int hi = std::min(img_height_ - margin - 1, static_cast<int>(s.hi) + config_.track_margin);

            if (x_start > x_min) {
                trackColsDirectional(gen, x_start, x_min, -1, lo, hi);
            }
            if (x_start < x_max) {
                trackColsDirectional(gen, x_start, x_max, +1, lo, hi);
            }

            return validateColsRange(x_min, x_max, gen);
        };

        for (const int dx : offsets) {
            const uint32_t left_gen = nextColGeneration();
            const uint32_t right_gen = nextColGeneration();

            const LineValidation left = tryHalf(center_left + dx, 0, img_width_ / 2, left_gen);
            const LineValidation right = tryHalf(center_right + dx, img_width_ / 2, img_width_ - 1, right_gen);

            if (left.ok && right.ok) {
                if (left.count >= right.count) {
                    active_col_generation_ = left_gen;
                    dir_out = 1; // left
                } else {
                    active_col_generation_ = right_gen;
                    dir_out = 2; // right
                }
                return true;
            }

            if (left.ok) {
                active_col_generation_ = left_gen;
                dir_out = 1;
                return true;
            }

            if (right.ok) {
                active_col_generation_ = right_gen;
                dir_out = 2;
                return true;
            }
        }

        return false;
    }

    bool scanRows(int& dir_out) {
        const int margin = config_.scan_step * 2;
        if (img_width_ <= margin * 2 || img_height_ <= margin * 2) {
            return false;
        }

        const int center_top = img_height_ / 4;
        const int center_bottom = (3 * img_height_) / 4;
        const std::vector<int> fallback_offsets{ 0 };
        const auto& offsets = config_.offsets.empty() ? fallback_offsets : config_.offsets;

        auto tryHalf = [&](const int y_start, const int y_min, const int y_max, const uint32_t gen) -> LineValidation {
            if (y_start < margin || y_start >= img_height_ - margin) {
                return {};
            }

            if (!scanRow(y_start, margin, img_width_ - margin - 1, gen)) {
                return {};
            }

            const Scan1D& s = row_scans_[static_cast<size_t>(y_start)];
            const int lo = std::max(margin, static_cast<int>(s.lo) - config_.track_margin);
            const int hi = std::min(img_width_ - margin - 1, static_cast<int>(s.hi) + config_.track_margin);

            if (y_start > y_min) {
                trackRowsDirectional(gen, y_start, y_min, -1, lo, hi);
            }
            if (y_start < y_max) {
                trackRowsDirectional(gen, y_start, y_max, +1, lo, hi);
            }

            return validateRowsRange(y_min, y_max, gen);
        };

        for (const int dy : offsets) {
            const uint32_t top_gen = nextRowGeneration();
            const uint32_t bot_gen = nextRowGeneration();

            const LineValidation top = tryHalf(center_top + dy, 0, img_height_ / 2, top_gen);
            const LineValidation bottom = tryHalf(center_bottom + dy, img_height_ / 2, img_height_ - 1, bot_gen);

            if (top.ok && bottom.ok) {
                if (top.count >= bottom.count) {
                    active_row_generation_ = top_gen;
                    dir_out = 3; // up
                } else {
                    active_row_generation_ = bot_gen;
                    dir_out = 4; // down
                }
                return true;
            }

            if (top.ok) {
                active_row_generation_ = top_gen;
                dir_out = 3;
                return true;
            }

            if (bottom.ok) {
                active_row_generation_ = bot_gen;
                dir_out = 4;
                return true;
            }
        }

        return false;
    }

    bool scan(int& dir_out) {
        const float a = last_angle_deg_;

        const bool rows_first =
            (a <= 45.0f) ||
            (a >= 315.0f) ||
            ((a >= 135.0f) && (a <= 225.0f));

        if (rows_first) {
            if (scanRows(dir_out)) {
                return true;
            }
            return scanCols(dir_out);
        }

        if (scanCols(dir_out)) {
            return true;
        }
        return scanRows(dir_out);
    }

    void collectPoints(const int dir) {
        points_edge_1_.clear();
        points_edge_2_.clear();
        points_center_.clear();

        if (dir == 1 || dir == 2) {
            const uint32_t gen = active_col_generation_;
            if (gen == 0) return;

            if (dir == 1) {
                for (int x = img_width_ / 2 - 1; x >= 0; --x) {
                    const Scan1D& s = col_scans_[static_cast<size_t>(x)];
                    if (!s.valid(gen)) continue;

                    points_edge_1_.push_back(toPosF(static_cast<uint16_t>(x), s.lo));
                    points_edge_2_.push_back(toPosF(static_cast<uint16_t>(x), s.hi));
                    points_center_.push_back(posf_t{
                        static_cast<float>(x),
                        0.5f * (static_cast<float>(s.lo) + static_cast<float>(s.hi))
                    });
                }
            } else {
                for (int x = img_width_ / 2 + 1; x < img_width_; ++x) {
                    const Scan1D& s = col_scans_[static_cast<size_t>(x)];
                    if (!s.valid(gen)) continue;

                    points_edge_1_.push_back(toPosF(static_cast<uint16_t>(x), s.lo));
                    points_edge_2_.push_back(toPosF(static_cast<uint16_t>(x), s.hi));
                    points_center_.push_back(posf_t{
                        static_cast<float>(x),
                        0.5f * (static_cast<float>(s.lo) + static_cast<float>(s.hi))
                    });
                }
            }
            return;
        }

        if (dir == 3 || dir == 4) {
            const uint32_t gen = active_row_generation_;
            if (gen == 0) return;

            if (dir == 3) {
                for (int y = img_height_ / 2 - 1; y >= 0; --y) {
                    const Scan1D& s = row_scans_[static_cast<size_t>(y)];
                    if (!s.valid(gen)) continue;

                    points_edge_1_.push_back(toPosF(s.lo, static_cast<uint16_t>(y)));
                    points_edge_2_.push_back(toPosF(s.hi, static_cast<uint16_t>(y)));
                    points_center_.push_back(posf_t{
                        0.5f * (static_cast<float>(s.lo) + static_cast<float>(s.hi)),
                        static_cast<float>(y)
                    });
                }
            } else {
                for (int y = img_height_ / 2 + 1; y < img_height_; ++y) {
                    const Scan1D& s = row_scans_[static_cast<size_t>(y)];
                    if (!s.valid(gen)) continue;

                    points_edge_1_.push_back(toPosF(s.lo, static_cast<uint16_t>(y)));
                    points_edge_2_.push_back(toPosF(s.hi, static_cast<uint16_t>(y)));
                    points_center_.push_back(posf_t{
                        0.5f * (static_cast<float>(s.lo) + static_cast<float>(s.hi)),
                        static_cast<float>(y)
                    });
                }
            }
        }
    }

    static bool fitLinePCA(const std::vector<posf_t>& pts, const std::vector<int>* idx, LineModel& out) {
        const int count = idx ? static_cast<int>(idx->size()) : static_cast<int>(pts.size());
        if (count < 2) {
            return false;
        }

        double mx = 0.0;
        double my = 0.0;

        if (idx) {
            for (const int i : *idx) {
                mx += pts[static_cast<size_t>(i)].x;
                my += pts[static_cast<size_t>(i)].y;
            }
        } else {
            for (const posf_t& p : pts) {
                mx += p.x;
                my += p.y;
            }
        }

        mx /= static_cast<double>(count);
        my /= static_cast<double>(count);

        double sxx = 0.0;
        double sxy = 0.0;
        double syy = 0.0;

        if (idx) {
            for (const int i : *idx) {
                const double ux = static_cast<double>(pts[static_cast<size_t>(i)].x) - mx;
                const double uy = static_cast<double>(pts[static_cast<size_t>(i)].y) - my;
                sxx += ux * ux;
                sxy += ux * uy;
                syy += uy * uy;
            }
        } else {
            for (const posf_t& p : pts) {
                const double ux = static_cast<double>(p.x) - mx;
                const double uy = static_cast<double>(p.y) - my;
                sxx += ux * ux;
                sxy += ux * uy;
                syy += uy * uy;
            }
        }

        const float theta = 0.5f * std::atan2(static_cast<float>(2.0 * sxy), static_cast<float>(sxx - syy));
        const float dirx = std::cos(theta);
        const float diry = std::sin(theta);

        out.nx = -diry;
        out.ny = dirx;
        out.ref = posf_t{ static_cast<float>(mx), static_cast<float>(my) };
        return true;
    }

    static int countInliers(const std::vector<posf_t>& pts, const LineModel& line, const float eps) {
        int cnt = 0;
        for (const posf_t& p : pts) {
            const float dx = p.x - line.ref.x;
            const float dy = p.y - line.ref.y;
            const float d = line.nx * dx + line.ny * dy;
            cnt += (std::fabs(d) <= eps);
        }
        return cnt;
    }

    bool fitLineRobust(const std::vector<posf_t>& pts, LineModel& out) {
        const int cnt = static_cast<int>(pts.size());
        if (cnt < 2) {
            return false;
        }

        const int min_inliers = std::max(2, static_cast<int>(
            std::ceil(config_.ransac_min_inliers_ratio * static_cast<float>(cnt))
        ));
        const float eps = std::fabs(config_.ransac_eps);

        // Быстрый путь: TLS/PCA по всем точкам.
        LineModel tls{};
        if (fitLinePCA(pts, nullptr, tls) && countInliers(pts, tls, eps) >= min_inliers) {
            out = tls;
            return true;
        }

        ransac_scores_.resize(static_cast<size_t>(cnt));
        ransac_order_.resize(static_cast<size_t>(cnt));
        ransac_inliers_.clear();

        float mx = 0.0f;
        float my = 0.0f;
        for (const posf_t& p : pts) {
            mx += p.x;
            my += p.y;
        }
        mx /= static_cast<float>(cnt);
        my /= static_cast<float>(cnt);

        for (int i = 0; i < cnt; ++i) {
            const float dx = pts[static_cast<size_t>(i)].x - mx;
            const float dy = pts[static_cast<size_t>(i)].y - my;
            ransac_scores_[static_cast<size_t>(i)] = dx * dx + dy * dy;
            ransac_order_[static_cast<size_t>(i)] = i;
        }

        std::sort(
            ransac_order_.begin(), ransac_order_.end(),
            [&](const int a, const int b) {
                return ransac_scores_[static_cast<size_t>(a)] > ransac_scores_[static_cast<size_t>(b)];
            }
        );

        int best_inliers = -1;
        int best_i1 = -1;
        int best_i2 = -1;

        int K = 2;
        const int Kmax = cnt;

        for (int it = 0; it < config_.ransac_iterations; ++it) {
            if (K < Kmax) {
                K = 2 + (it * (Kmax - 2)) / std::max(1, config_.ransac_iterations - 1);
                if (K > Kmax) K = Kmax;
            }

            const int i1 = ransac_order_[static_cast<size_t>(rng_() % static_cast<uint32_t>(K))];
            const int i2 = ransac_order_[static_cast<size_t>(rng_() % static_cast<uint32_t>(K))];
            if (i1 == i2) {
                continue;
            }

            const posf_t& p1 = pts[static_cast<size_t>(i1)];
            const posf_t& p2 = pts[static_cast<size_t>(i2)];

            const float dx = p2.x - p1.x;
            const float dy = p2.y - p1.y;
            const float len2 = dx * dx + dy * dy;
            if (len2 < 1e-12f) {
                continue;
            }

            float nx = -dy;
            float ny = dx;
            const float inv = 1.0f / std::sqrt(nx * nx + ny * ny);
            nx *= inv;
            ny *= inv;

            // Быстрый reject.
            int side_balance = 0;
            for (int t = 0; t < 8; ++t) {
                const int j = static_cast<int>(rng_() % static_cast<uint32_t>(cnt));
                const float ddx = pts[static_cast<size_t>(j)].x - p1.x;
                const float ddy = pts[static_cast<size_t>(j)].y - p1.y;
                const float d = nx * ddx + ny * ddy;
                if (d > eps) ++side_balance;
                else if (d < -eps) --side_balance;
            }
            if (std::abs(side_balance) > 6) {
                continue;
            }

            int inliers = 0;
            for (int i = 0; i < cnt; ++i) {
                const float ddx = pts[static_cast<size_t>(i)].x - p1.x;
                const float ddy = pts[static_cast<size_t>(i)].y - p1.y;
                const float d = nx * ddx + ny * ddy;
                inliers += (std::fabs(d) <= eps);
            }

            if (inliers > best_inliers) {
                best_inliers = inliers;
                best_i1 = i1;
                best_i2 = i2;

                if (best_inliers == cnt) {
                    break;
                }
            }
        }

        if (best_inliers < min_inliers || best_i1 < 0 || best_i2 < 0) {
            return false;
        }

        const posf_t& p1 = pts[static_cast<size_t>(best_i1)];
        const posf_t& p2 = pts[static_cast<size_t>(best_i2)];

        float nx = -(p2.y - p1.y);
        float ny =  (p2.x - p1.x);
        const float inv = 1.0f / std::sqrt(nx * nx + ny * ny);
        nx *= inv;
        ny *= inv;

        ransac_inliers_.clear();
        ransac_inliers_.reserve(static_cast<size_t>(best_inliers));

        for (int i = 0; i < cnt; ++i) {
            const float ddx = pts[static_cast<size_t>(i)].x - p1.x;
            const float ddy = pts[static_cast<size_t>(i)].y - p1.y;
            const float d = nx * ddx + ny * ddy;
            if (std::fabs(d) <= eps) {
                ransac_inliers_.push_back(i);
            }
        }

        return fitLinePCA(pts, &ransac_inliers_, out);
    }

    float angleOfLine(const LineModel& line, const int dir) const {
        float tx = line.ny;
        float ty = -line.nx;

        switch (dir) {
            case 1: // left
                if (tx > 0.0f) { tx = -tx; ty = -ty; }
                break;
            case 2: // right
                if (tx < 0.0f) { tx = -tx; ty = -ty; }
                break;
            case 3: // up
                if (ty > 0.0f) { tx = -tx; ty = -ty; }
                break;
            case 4: // down
                if (ty < 0.0f) { tx = -tx; ty = -ty; }
                break;
            default:
                break;
        }

        float angle = std::atan2(-tx, ty) * 180.0f / kPi;
        if (angle < 0.0f) {
            angle += 360.0f;
        }
        return angle;
    }

    void applyTransform(float& angle) const {
        if (transform_angle_ != nullptr) {
            angle = transform_angle_(angle);
        }
    }

    bool estimateAngle(const int dir, float& out_angle) {
        LineModel line_1{};
        LineModel line_2{};
        const bool ok_1 = fitLineRobust(points_edge_1_, line_1);
        const bool ok_2 = fitLineRobust(points_edge_2_, line_2);

        if (ok_1 && ok_2) {
            float a1 = angleOfLine(line_1, dir);
            float a2 = angleOfLine(line_2, dir);

            applyTransform(a1);
            applyTransform(a2);

            if (angleDiffDeg(a1, a2) <= config_.angle_match_tolerance_deg) {
                out_angle = angleMeanDeg(a1, a2);
                return true;
            }
        }

        // Fallback: центр полосы. Часто спасает кадры, где одна грань стрелки размазана.
        LineModel center_line{};
        if (fitLineRobust(points_center_, center_line)) {
            float a = angleOfLine(center_line, dir);
            applyTransform(a);
            out_angle = a;
            return true;
        }

        return false;
    }
};

} // namespace

struct anglemeter_t {
    explicit anglemeter_t(anglemeter_config cfg)
        : impl(std::move(cfg)) {}

    AngleMeterImpl impl;
};

void anglemeterCreate(anglemeter_t** am_ptr, anglemeter_config config) {
    if (am_ptr == nullptr) {
        return;
    }
    *am_ptr = new anglemeter_t(std::move(config));
}

void anglemeterDestroy(anglemeter_t* am) {
    delete am;
}

void anglemeterSetConfig(anglemeter_t* am, anglemeter_config config) {
    if (am == nullptr) {
        return;
    }
    am->impl.setConfig(std::move(config));
}

void anglemeterSetImageSize(anglemeter_t* am, const int width, const int height) {
    if (am == nullptr) {
        return;
    }
    am->impl.setImageSize(width, height);
}

void anglemeterSetAngleTransformation(anglemeter_t* am, float (*func_ptr)(float)) {
    if (am == nullptr) {
        return;
    }
    am->impl.setAngleTransformation(func_ptr);
}

bool anglemeterGetArrowAngle(anglemeter_t* am, const unsigned char* img, float* angle) {
    if (am == nullptr) {
        return false;
    }
    return am->impl.getArrowAngle(img, angle);
}
