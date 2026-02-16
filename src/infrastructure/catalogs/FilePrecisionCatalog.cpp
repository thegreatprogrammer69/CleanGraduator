#include "FilePrecisionCatalog.h"

#include <algorithm>
#include <charconv>
#include <fstream>
#include <optional>
#include <stdexcept>
#include <string>

#include "application/fmt/fmt_application.h"

namespace {

inline void trim(std::string& s)
{
    auto notSpace = [](unsigned char ch) {
        return !std::isspace(ch);
    };

    s.erase(s.begin(), std::find_if(s.begin(), s.end(), notSpace));
    s.erase(std::find_if(s.rbegin(), s.rend(), notSpace).base(), s.end());
}

inline bool tryParseDouble(std::string token, double& out)
{
    // нормализуем десятичный разделитель
    std::replace(token.begin(), token.end(), ',', '.');

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

using application::models::Precision;

FilePrecisionCatalog::FilePrecisionCatalog(
    FilePrecisionCatalogPorts ports,
    std::string filePath
)
    : logger_(ports.logger)
{
    std::ifstream file(filePath);
    if (!file.is_open()) {
        logger_.error("Failed to open precision catalog file: {}", filePath);
        throw std::runtime_error("failed to open file: " + filePath);
    }

    std::string line;

    while (std::getline(file, line)) {

        trim(line);

        if (line.empty() || line[0] == '#')
            continue;

        double value{};

        if (tryParseDouble(line, value)) {
            Precision precision;
            precision.precision = value;
            precisions_.push_back(precision);
            logger_.info("Loaded precision model: {}", precisions_.back());
        } else {
            logger_.error("Failed to parse precision line: {}", line);
        }
    }
}

std::vector<Precision> FilePrecisionCatalog::list() const
{
    return precisions_;
}

std::optional<Precision> FilePrecisionCatalog::at(int idx) const
{
    if (idx >= 0 && idx < static_cast<int>(precisions_.size()))
        return precisions_.at(idx);

    return std::nullopt;
}

} // namespace infra::catalogs
