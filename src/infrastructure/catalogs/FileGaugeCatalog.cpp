#include "FileGaugeCatalog.h"

#include <algorithm>
#include <codecvt>
#include <cwctype>
#include <fstream>
#include <locale>
#include <sstream>
#include <stdexcept>

#include "application/fmt/fmt_application.h"

namespace {

inline std::string to_utf8(const std::wstring& ws) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
    return conv.to_bytes(ws);
}

inline void trim(std::wstring& s) {
    auto notSpace = [](wchar_t ch) { return !std::iswspace(ch); };
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), notSpace));
    s.erase(std::find_if(s.rbegin(), s.rend(), notSpace).base(), s.end());
}

inline std::vector<std::wstring> split(const std::wstring& s, wchar_t delim) {
    std::vector<std::wstring> out;
    std::wstringstream ss(s);
    std::wstring item;
    while (std::getline(ss, item, delim)) out.push_back(item);
    return out;
}

    inline bool tryParseDouble(const std::wstring& token, double& out) {
    try {
        std::wstring normalized = token;

        // нормализуем разделитель
        std::replace(normalized.begin(), normalized.end(), L',', L'.');

        // парсим в C-locale
        std::string utf8 = to_utf8(normalized);
        std::istringstream iss(utf8);
        iss.imbue(std::locale::classic());

        iss >> out;

        return !iss.fail() && iss.eof();
    } catch (...) {
        return false;
    }
}
} // namespace

namespace infra::catalogs {

FileGaugeCatalog::FileGaugeCatalog(FileGaugeCatalogPorts ports, std::string filePath)
    : logger_(ports.logger)
{
    std::wifstream in(filePath);
    if (!in.is_open()) {
        logger_.error("Failed to open gauge catalog file: {}", filePath);
        throw std::runtime_error("failed to open file: " + filePath);
    }

    in.imbue(std::locale(""));

    std::wstring line;

    while (std::getline(in, line)) {
        trim(line);
        if (line.empty() || line[0] == L'#') continue;

        auto parts = split(line, L';');
        if (parts.size() < 2) {
            logger_.error("Failed to parse gauge line: {}", to_utf8(line));
            continue;
        }

        std::wstring name = parts[0];
        trim(name);
        if (name.empty()) {
            logger_.error("Gauge name is empty in line: {}", to_utf8(line));
            continue;
        }

        std::vector<double> values;

        if (parts.size() == 2) {
            std::wstring valuesPart = parts[1];
            trim(valuesPart);

            auto byComma = split(valuesPart, L',');
            for (auto& t : byComma) {
                trim(t);
                if (t.empty()) continue;
                double v{};
                if (tryParseDouble(t, v)) values.push_back(v);
                else logger_.error("Failed to parse gauge value '{}' in line: {}", to_utf8(t), to_utf8(line));
            }

            if (values.empty()) {
                std::wstringstream ss(valuesPart);
                std::wstring tok;
                while (ss >> tok) {
                    double v{};
                    if (tryParseDouble(tok, v)) values.push_back(v);
                    else logger_.error("Failed to parse gauge value '{}' in line: {}", to_utf8(tok), to_utf8(line));
                }
            }
        } else {
            for (size_t i = 1; i < parts.size(); ++i) {
                auto t = parts[i];
                trim(t);
                if (t.empty()) continue;
                double v{};
                if (tryParseDouble(t, v)) values.push_back(v);
                else logger_.error("Failed to parse gauge value '{}' in line: {}", to_utf8(t), to_utf8(line));
            }
        }

        if (values.empty()) {
            logger_.error("Gauge has no valid values in line: {}", to_utf8(line));
            continue;
        }

        gauges_.push_back(application::models::Gauge{
            .name = std::move(name),
            .values = std::move(values)
        });
        logger_.info("Loaded gauge model: {}", gauges_.back());
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
