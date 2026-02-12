#include "FileGaugeCatalog.h"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <cwctype>

namespace {

// trim для std::wstring
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
        size_t idx = 0;
        out = std::stod(token, &idx);
        // допускаем пробелы после числа
        for (; idx < token.size(); ++idx) {
            if (!std::iswspace(token[idx])) return false;
        }
        return true;
    } catch (...) {
        return false;
    }
}

} // namespace

namespace infra::catalogs {

FileGaugeCatalog::FileGaugeCatalog(std::string filePath) {
    std::wifstream in(filePath);
    if (!in.is_open()) throw std::runtime_error("failed to open file: " + filePath);

    // Чтобы корректно читать wide-строки
    in.imbue(std::locale(""));

    std::wstring line;

    while (std::getline(in, line)) {
        trim(line);
        if (line.empty()) continue;
        if (!line.empty() && line[0] == L'#') continue;

        // Базовый формат: name;v1,v2,v3
        // Также допускается: name;v1;v2;v3 (если хочешь — это удобно для Excel/CSV)
        auto parts = split(line, L';');
        if (parts.size() < 2) continue;

        std::wstring name = parts[0];
        trim(name);
        if (name.empty()) continue;

        std::vector<double> values;

        if (parts.size() == 2) {
            // values в одной колонке, разделение запятыми/пробелами
            std::wstring valuesPart = parts[1];
            trim(valuesPart);

            // сначала разбиваем по запятой
            auto byComma = split(valuesPart, L',');
            for (auto& t : byComma) {
                trim(t);
                if (t.empty()) continue;
                double v{};
                if (tryParseDouble(t, v)) values.push_back(v);
            }

            // если запятых не было или ничего не распарсилось — попробуем по пробелам
            if (values.empty()) {
                std::wstringstream ss(valuesPart);
                std::wstring tok;
                while (ss >> tok) {
                    double v{};
                    if (tryParseDouble(tok, v)) values.push_back(v);
                }
            }
        } else {
            // name;v1;v2;v3...
            for (size_t i = 1; i < parts.size(); ++i) {
                auto t = parts[i];
                trim(t);
                if (t.empty()) continue;
                double v{};
                if (tryParseDouble(t, v)) values.push_back(v);
            }
        }

        gauges_.push_back(application::models::Gauge{
            .name = std::move(name),
            .values = std::move(values)
        });
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
