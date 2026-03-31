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

void trim(std::string& s) {
    auto notSpace = [](unsigned char ch) { return !std::isspace(ch); };
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), notSpace));
    s.erase(std::find_if(s.rbegin(), s.rend(), notSpace).base(), s.end());
}

std::vector<std::string> split(const std::string& s, char delim) {
    std::vector<std::string> out;
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        out.push_back(item);
    }
    return out;
}

bool tryParseDouble(std::string token, float& out) {
    // Заменяем запятую на точку для корректного парсинга дробных чисел
    std::replace(token.begin(), token.end(), ',', '.');
    trim(token);

    if (token.empty()) return false;

    auto result = std::from_chars(
        token.data(),
        token.data() + token.size(),
        out
    );

    return result.ec == std::errc() &&
           result.ptr == token.data() + token.size();
}

} // namespace

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
        // Удаление BOM-маркера UTF-8, если он есть
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

        // Пропускаем пустые строки, комментарии и декоративные элементы таблицы
        if (line.empty() || line[0] == '#' || line[0] == '-') continue;
        // Пропускаем строку с заголовками колонок
        if (line.find("ИМЯ") != std::string::npos || line.find("NAME") != std::string::npos) continue;

        // Разделяем строку по новому разделителю '|'
        auto parts = split(line, '|');
        if (parts.size() < 3) {
            logger_.error("Failed to parse gauge line (expected at least 3 columns): {}", line);
            continue;
        }

        // 1. Имя
        std::string name_utf8 = parts[0];
        trim(name_utf8);
        if (name_utf8.empty()) {
            logger_.error("Gauge name is empty in line: {}", line);
            continue;
        }

        // 2. Central Pressure
        float central_pressure{};
        std::string cp_str = parts[1];
        if (!tryParseDouble(cp_str, central_pressure)) {
            logger_.error("Failed to parse central_pressure '{}' in line: {}", cp_str, line);
            continue;
        }

        // 3. Pressure Points (разделены запятыми)
        std::vector<float> values;
        auto points_tokens = split(parts[2], ',');

        for (auto& token : points_tokens) {
            trim(token);
            if (token.empty()) continue;

            float v{};
            if (tryParseDouble(token, v)) {
                values.push_back(v);
            } else {
                logger_.error("Failed to parse gauge point value '{}' in line: {}", token, line);
            }
        }

        if (values.empty()) {
            logger_.error("Gauge has no valid points in line: {}", line);
            continue;
        }

        // Сборка финального объекта
        try {
            application::models::Gauge g;
            g.name = name_utf8;
            g.central_pressure = central_pressure;
            // Предполагается, что структура points имеет поле value типа std::vector<float>,
            // как это было в вашей предыдущей реализации
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