#include "FilePrinterCatalog.h"

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

} // namespace

namespace infra::catalogs {

FilePrinterCatalog::FilePrinterCatalog(FilePrinterCatalogPorts ports, std::string filePath)
    : logger_(ports.logger)
{
    std::wifstream in(filePath);
    if (!in.is_open()) {
        logger_.error("Failed to open printer catalog file: {}", filePath);
        throw std::runtime_error("failed to open file: " + filePath);
    }

    in.imbue(std::locale(""));

    std::wstring line;

    while (std::getline(in, line)) {
        trim(line);
        if (line.empty() || line[0] == L'#') continue;

        auto parts = split(line, L';');
        if (parts.size() < 2) {
            logger_.error("Failed to parse printer line: {}", to_utf8(line));
            continue;
        }

        std::wstring name = parts[0];
        std::wstring path = parts[1];
        trim(name);
        trim(path);

        if (name.empty() || path.empty()) {
            logger_.error("Printer name/path is empty in line: {}", to_utf8(line));
            continue;
        }

        printers_.push_back(application::models::Printer{
            .name = std::move(name),
            .path = std::move(path)
        });
        logger_.info("Loaded printer model: {}", printers_.back());
    }
}

std::vector<application::models::Printer> FilePrinterCatalog::list() const {
    return printers_;
}

std::optional<application::models::Printer> FilePrinterCatalog::at(int idx) const {
    if (idx >= 0 && idx < static_cast<int>(printers_.size())) {
        return printers_.at(idx);
    }
    return std::nullopt;
}
} // namespace infra::catalogs
