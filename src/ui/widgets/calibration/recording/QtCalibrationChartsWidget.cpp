#include "QtCalibrationChartsWidget.h"

#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QLabel>
#include <QMetaObject>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QVBoxLayout>

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <optional>
#include <utility>

namespace {

using PressureEntry = mvvm::CalibrationSeriesViewModel::PressureEntry;
using AngleEntry = mvvm::CalibrationSeriesViewModel::AngleEntry;

QColor colorForIndex(int i)
{
    static const std::array<QColor, 8> colors = {
        QColor(0, 140, 255),
        QColor(220, 0, 120),
        QColor(30, 170, 90),
        QColor(240, 140, 0),
        QColor(130, 90, 220),
        QColor(0, 160, 160),
        QColor(200, 80, 80),
        QColor(130, 130, 130)
    };
    return colors[static_cast<std::size_t>(i) % colors.size()];
}

std::optional<float> pressureAtTime(const std::vector<PressureEntry>& pressure, float time)
{
    if (pressure.empty()) {
        return std::nullopt;
    }

    auto it = std::lower_bound(pressure.begin(), pressure.end(), time,
        [](const auto& p, const float t) { return p.time < t; });

    if (it == pressure.begin()) {
        return it->pressure;
    }

    if (it == pressure.end()) {
        return pressure.back().pressure;
    }

    const auto& right = *it;
    const auto& left = *(it - 1);

    const float dt = right.time - left.time;
    if (std::fabs(dt) < std::numeric_limits<float>::epsilon()) {
        return right.pressure;
    }

    const float ratio = (time - left.time) / dt;
    return left.pressure + ratio * (right.pressure - left.pressure);
}

} // namespace

namespace ui {

class QtCalibrationChartsWidget::PlotCanvas final : public QWidget
{
public:
    struct Series {
        QString name;
        QColor color;
        QVector<QPointF> points;
    };

    explicit PlotCanvas(QWidget* parent = nullptr)
        : QWidget(parent)
    {
        setMinimumSize(480, 320);
    }

    void setAxes(QString x_label, QString y_label)
    {
        x_label_ = std::move(x_label);
        y_label_ = std::move(y_label);
        update();
    }

    void setSeries(std::vector<Series> series)
    {
        series_ = std::move(series);
        update();
    }

protected:
    void paintEvent(QPaintEvent*) override
    {
        QPainter p(this);
        p.fillRect(rect(), QColor(18, 18, 18));
        p.setRenderHint(QPainter::Antialiasing, true);

        const QRectF plot = rect().adjusted(58, 16, -16, -42);
        if (plot.width() <= 0 || plot.height() <= 0) {
            return;
        }

        p.setPen(QColor(70, 70, 70));
        p.drawRect(plot);

        p.setPen(QColor(190, 190, 190));
        p.drawText(QRectF(plot.left(), plot.bottom() + 6, plot.width(), 20),
                   Qt::AlignCenter, x_label_);

        p.save();
        p.translate(14, plot.top() + plot.height() / 2.0);
        p.rotate(-90.0);
        p.drawText(QRectF(-plot.height() / 2.0, -14, plot.height(), 20),
                   Qt::AlignCenter, y_label_);
        p.restore();

        if (series_.empty()) {
            p.setPen(QColor(170, 170, 170));
            p.drawText(plot, Qt::AlignCenter, tr("Нет данных"));
            return;
        }

        double min_x = std::numeric_limits<double>::max();
        double max_x = std::numeric_limits<double>::lowest();
        double min_y = std::numeric_limits<double>::max();
        double max_y = std::numeric_limits<double>::lowest();

        for (const auto& s : series_) {
            for (const auto& pt : s.points) {
                min_x = std::min(min_x, pt.x());
                max_x = std::max(max_x, pt.x());
                min_y = std::min(min_y, pt.y());
                max_y = std::max(max_y, pt.y());
            }
        }

        if (!std::isfinite(min_x) || !std::isfinite(max_x) || !std::isfinite(min_y) || !std::isfinite(max_y)) {
            return;
        }

        if (std::fabs(max_x - min_x) < 1e-9) {
            max_x += 1.0;
            min_x -= 1.0;
        }

        if (std::fabs(max_y - min_y) < 1e-9) {
            max_y += 1.0;
            min_y -= 1.0;
        }

        const double sx = plot.width() / (max_x - min_x);
        const double sy = plot.height() / (max_y - min_y);

        int legend_row = 0;
        for (const auto& s : series_) {
            if (s.points.size() < 2) {
                continue;
            }

            QPainterPath path;
            const auto first = s.points.front();
            path.moveTo(plot.left() + (first.x() - min_x) * sx,
                        plot.bottom() - (first.y() - min_y) * sy);

            for (int i = 1; i < s.points.size(); ++i) {
                const auto pt = s.points[i];
                path.lineTo(plot.left() + (pt.x() - min_x) * sx,
                            plot.bottom() - (pt.y() - min_y) * sy);
            }

            p.setPen(QPen(s.color, 2));
            p.drawPath(path);

            const int lx = static_cast<int>(plot.left()) + 12 + legend_row * 140;
            const int ly = 12;
            p.drawLine(lx, ly, lx + 18, ly);
            p.setPen(QColor(220, 220, 220));
            p.drawText(lx + 24, ly + 4, s.name);
            ++legend_row;
        }
    }

private:
    QString x_label_;
    QString y_label_;
    std::vector<Series> series_;
};

QtCalibrationChartsWidget::QtCalibrationChartsWidget(mvvm::CalibrationSeriesViewModel& vm, QWidget* parent)
    : QWidget(parent)
    , vm_(vm)
{
    buildUi();
    rebuildSourceSelectors();
    updatePlot();

    current_pressure_sub_ = vm_.current_pressure.subscribe([this](const auto&) {
        QMetaObject::invokeMethod(this, [this] { updatePlot(); }, Qt::QueuedConnection);
    });

    current_angle_sub_ = vm_.current_angle.subscribe([this](const auto&) {
        QMetaObject::invokeMethod(this, [this] { updatePlot(); }, Qt::QueuedConnection);
    });

    source_ids_sub_ = vm_.source_ids.subscribe([this](const auto&) {
        QMetaObject::invokeMethod(this, [this] {
            rebuildSourceSelectors();
            updatePlot();
        }, Qt::QueuedConnection);
    });
}

QtCalibrationChartsWidget::~QtCalibrationChartsWidget() = default;

void QtCalibrationChartsWidget::buildUi()
{
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(8, 8, 8, 8);
    root->setSpacing(8);

    auto* top = new QGridLayout();
    top->setHorizontalSpacing(8);
    top->setVerticalSpacing(6);

    top->addWidget(new QLabel(tr("График:"), this), 0, 0);
    graph_type_ = new QComboBox(this);
    graph_type_->addItem(tr("Давление / Время"));
    graph_type_->addItem(tr("Угол / Время"));
    graph_type_->addItem(tr("Угол / Давление"));
    top->addWidget(graph_type_, 0, 1);

    source_hint_ = new QLabel(tr("Источники угла:"), this);
    top->addWidget(source_hint_, 1, 0, Qt::AlignTop);

    source_panel_ = new QWidget(this);
    source_layout_ = new QVBoxLayout(source_panel_);
    source_layout_->setContentsMargins(0, 0, 0, 0);
    source_layout_->setSpacing(4);
    top->addWidget(source_panel_, 1, 1);

    top->setColumnStretch(1, 1);

    root->addLayout(top);

    plot_canvas_ = new PlotCanvas(this);
    root->addWidget(plot_canvas_, 1);

    connect(graph_type_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &QtCalibrationChartsWidget::onGraphTypeChanged);
}

void QtCalibrationChartsWidget::rebuildSourceSelectors()
{
    while (auto* item = source_layout_->takeAt(0)) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }
    source_checks_.clear();

    std::vector<int> source_ids;
    source_ids.reserve(vm_.openedSources().size());
    for (const auto& src : vm_.openedSources()) {
        source_ids.push_back(src.value);
    }

    std::sort(source_ids.begin(), source_ids.end());
    source_ids.erase(std::unique(source_ids.begin(), source_ids.end()), source_ids.end());

    for (int id : source_ids) {
        auto* cb = new QCheckBox(tr("Камера %1").arg(id), source_panel_);
        cb->setProperty("sourceId", id);
        cb->setChecked(true);
        connect(cb, &QCheckBox::toggled, this, [this](bool) { updatePlot(); });
        source_layout_->addWidget(cb);
        source_checks_.push_back(cb);
    }

    source_layout_->addStretch(1);
}

bool QtCalibrationChartsWidget::isAngleGraph() const
{
    const auto type = static_cast<GraphType>(graph_type_->currentIndex());
    return type == GraphType::AngleByTime || type == GraphType::AngleByPressure;
}

std::vector<int> QtCalibrationChartsWidget::selectedAngleSourceIds() const
{
    std::vector<int> ids;
    for (auto* cb : source_checks_) {
        if (cb->isChecked()) {
            ids.push_back(cb->property("sourceId").toInt());
        }
    }
    return ids;
}

void QtCalibrationChartsWidget::onGraphTypeChanged(int)
{
    const bool show_sources = isAngleGraph();
    source_panel_->setVisible(show_sources);
    source_hint_->setVisible(show_sources);
    updatePlot();
}

void QtCalibrationChartsWidget::updatePlot()
{
    const auto type = static_cast<GraphType>(graph_type_->currentIndex());

    std::vector<PlotCanvas::Series> out;

    if (type == GraphType::PressureByTime) {
        PlotCanvas::Series s;
        s.name = tr("Давление");
        s.color = colorForIndex(0);
        for (const auto& p : vm_.pressureHistory()) {
            s.points.push_back(QPointF(p.time, p.pressure));
        }

        plot_canvas_->setAxes(tr("Время"), tr("Давление"));
        out.push_back(std::move(s));
        plot_canvas_->setSeries(std::move(out));
        return;
    }

    const auto selected = selectedAngleSourceIds();
    int color_idx = 0;

    for (int source_id : selected) {
        PlotCanvas::Series s;
        s.name = tr("Камера %1").arg(source_id);
        s.color = colorForIndex(color_idx++);

        const auto& angle = vm_.angleHistory(domain::common::SourceId{source_id});
        for (const auto& a : angle) {
            if (type == GraphType::AngleByTime) {
                s.points.push_back(QPointF(a.time, a.angle));
            } else {
                const auto pressure = pressureAtTime(vm_.pressureHistory(), a.time);
                if (pressure.has_value()) {
                    s.points.push_back(QPointF(*pressure, a.angle));
                }
            }
        }

        if (!s.points.isEmpty()) {
            out.push_back(std::move(s));
        }
    }

    if (type == GraphType::AngleByTime) {
        plot_canvas_->setAxes(tr("Время"), tr("Угол"));
    } else {
        plot_canvas_->setAxes(tr("Давление"), tr("Угол"));
    }

    plot_canvas_->setSeries(std::move(out));
}

} // namespace ui
