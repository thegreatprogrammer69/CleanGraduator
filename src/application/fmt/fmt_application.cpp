#include "fmt_application.h"

#include <ostream>
#include <string>
#include <vector>
#include <codecvt>
#include <locale>

#include "domain/fmt/fmt_domain.h"

namespace {
    inline std::string to_utf8(const std::wstring& ws) {
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
        return conv.to_bytes(ws);
    }
}

namespace application::models {

std::ostream& operator<<(std::ostream& os, const Displacement& f) {
    return os << "Displacement{id=" << f.id << ", name='" << to_utf8(f.name) << "'}";
}

std::ostream& operator<<(std::ostream& os, const Gauge& f) {
    os << "Gauge{name='" << to_utf8(f.name) << "', values=[";
    for (size_t i = 0; i < f.values.size(); ++i) {
        os << f.values[i];
        if (i + 1 < f.values.size()) {
            os << ", ";
        }
    }
    return os << "]}";
}

std::ostream& operator<<(std::ostream& os, const Precision& f) {
    return os << "Precision{precision=" << f.precision << "}";
}

std::ostream& operator<<(std::ostream& os, const PressureUnit& f) {
    return os << "PressureUnit{unit=" << static_cast<int>(f.unit) << "}";
}

std::ostream& operator<<(std::ostream& os, const Printer& f) {
    return os << "Printer{name='" << to_utf8(f.name) << "', path='" << to_utf8(f.path) << "'}";
}

} // namespace application::models
