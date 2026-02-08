#include "FilePrinterCatalog.h"

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

} // namespace

namespace infra::catalogs {

    FilePrinterCatalog::FilePrinterCatalog(std::string filePath) {
        std::wifstream in(filePath);
        if (!in.is_open()) throw std::runtime_error("failed to open file: " + filePath);

        // Чтобы корректно читать wide-строки
        in.imbue(std::locale(""));

        std::wstring line;

        while (std::getline(in, line)) {
            trim(line);
            if (line.empty()) continue;
            if (!line.empty() && line[0] == L'#') continue; // комментарий

            // Формат: name;path
            auto parts = split(line, L';');
            if (parts.size() < 2) continue;

            std::wstring name = parts[0];
            std::wstring path = parts[1];
            trim(name);
            trim(path);

            if (name.empty() || path.empty()) continue;

            printers_.push_back(application::model::PrinterRecord{
                .name = std::move(name),
                .path = std::move(path)
            });
        }
    }

    std::vector<application::model::PrinterRecord> FilePrinterCatalog::list() const {
        return printers_;
    }

    std::optional<application::model::PrinterRecord> FilePrinterCatalog::at(int idx) const {
        if (idx >= 0 && idx < printers_.size()) {
            return printers_.at(idx);
        }
        return std::nullopt;
    }
} // namespace infrastructure::catalogs
