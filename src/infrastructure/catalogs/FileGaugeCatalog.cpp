#include "FileGaugeCatalog.h"

#include <algorithm>
#include <cctype>
#include <charconv>
#include <codecvt>
#include <fstream>
#include <locale>
#include <sstream>
#include <stdexcept>

#include "application/fmt/fmt_application.h"

namespace {
std::wstring from_utf8(std::string_view s) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
    return conv.from_bytes(s.data(), s.data() + s.size());
}

void trim(std::string& s) {
    auto notSpace = [](unsigned char ch) { return !std::isspace(ch); };
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), notSpace));
    s.erase(std::find_if(s.rbegin(), s.rend(), notSpace).base(), s.end());
}

std::vector<std::string> split(const std::string& s, char delim) {
    std::vector<std::string> out;
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) out.push_back(item);
    return out;
}

bool tryParseDouble(std::string token, double& out) {
    std::replace(token.begin(), token.end(), ',', '.');
    trim(token);

    auto result = std::from_chars(
        token.data(),
        token.data() + token.size(),
        out
    );

    return result.ec == std::errc() &&
           result.ptr == token.data() + token.size();
}
}

namespace infra::catalogs {

FileGaugeCatalog::FileGaugeCatalog(CatalogPorts ports, std::string filePath)
    : logger_(ports.logger)
{
    std::ifstream in(filePath, std::ios::binary);
    if (!in.is_open()) {
        logger_.error("Failed to open gauge catalog file: {}", filePath);
        throw std::runtime_error("failed to open file: " + filePath);
    }

    std::string line;
    bool first_line = true;

    while (std::getline(in, line)) {
        if (first_line) {
            first_line = false;
            if (line.size() >= 3 &&
                static_cast<unsigned char>(line[0]) == 0xEF &&
                static_cast<unsigned char>(line[1]) == 0xBB &&
                static_cast<unsigned char>(line[2]) == 0xBF) {
                line.erase(0, 3);
            }
        }

        trim(line);
        if (line.empty() || line[0] == '#') continue;

        auto parts = split(line, ';');
        if (parts.size() < 2) {
            logger_.error("Failed to parse gauge line: {}", line);
            continue;
        }

        std::string name_utf8 = parts[0];
        trim(name_utf8);
        if (name_utf8.empty()) {
            logger_.error("Gauge name is empty in line: {}", line);
            continue;
        }

        std::vector<double> values;

        for (size_t i = 1; i < parts.size(); ++i) {
            auto token = parts[i];
            trim(token);
            if (token.empty()) continue;

            double v{};
            if (tryParseDouble(token, v)) {
                values.push_back(v);
            } else {
                logger_.error("Failed to parse gauge value '{}' in line: {}", token, line);
            }
        }

        if (values.empty()) {
            logger_.error("Gauge has no valid values in line: {}", line);
            continue;
        }

        try {
            application::models::Gauge g;
            g.name = from_utf8(name_utf8);
            g.points.value = std::move(values);
            gauges_.push_back(g);
            logger_.info("Loaded gauge model: {}", gauges_.back());
        } catch (const std::range_error&) {
            logger_.error("Invalid UTF-8 in gauge line: {}", line);
        }
    }
}

std::vector<application::models::Gauge> FileGaugeCatalog::list() const {
    return gauges_;
}

std::optional<application::models::Gauge> FileGaugeCatalog::at(int idx) const {
    if (idx >= 0 && idx < static_cast<int>(gauges_.size())) {
        return gauges_.at(idx);
    }
    return std::nullopt;
}

} // namespace infra::catalogs
