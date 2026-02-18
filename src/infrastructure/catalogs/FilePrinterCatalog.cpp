#include "FilePrinterCatalog.h"

#include <algorithm>
#include <cctype>
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

} // namespace

namespace infra::catalogs {

FilePrinterCatalog::FilePrinterCatalog(CatalogPorts ports, std::string filePath)
    : logger_(ports.logger)
{
    std::ifstream in(filePath, std::ios::binary);
    if (!in.is_open()) {
        logger_.error("Failed to open printer catalog file: {}", filePath);
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
            logger_.error("Failed to parse printer line: {}", line);
            continue;
        }

        std::string name = parts[0];
        std::string path = parts[1];
        trim(name);
        trim(path);

        if (name.empty() || path.empty()) {
            logger_.error("Printer name/path is empty in line: {}", line);
            continue;
        }

        try {
            application::models::Printer printer;
            printer.name = from_utf8(name);
            printer.path = from_utf8(path);
            printers_.push_back(printer);
            logger_.info("Loaded printer model: {}", printers_.back());
        } catch (const std::range_error&) {
            logger_.error("Invalid UTF-8 in printer line: {}", line);
        }
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
