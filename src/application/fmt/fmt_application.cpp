#include "fmt_application.h"

#include <ostream>
#include <string>
#include <vector>
#include <codecvt>
#include <locale>

#include "domain/fmt/fmt_domain.h"

namespace application::models {

std::ostream& operator<<(std::ostream& os, const Displacement& f) {
    return os << "Displacement{id=" << f.id << ", name='" << f.name << "'}";
}

std::ostream& operator<<(std::ostream& os, const Gauge& f) {
    os << "Gauge{name='" << f.name << "', values=[";
    for (size_t i = 0; i < f.points.value.size(); ++i) {
        os << f.points.value[i];
        if (i + 1 < f.points.value.size()) {
            os << ", ";
        }
    }
    return os << "]}";
}

std::ostream& operator<<(std::ostream& os, const GaugePrecision& f) {
    return os << "Precision{precision=" << f.precision.value << "}";
}

std::ostream& operator<<(std::ostream& os, const PressureUnit& f) {
    return os << "PressureUnit{unit=" << static_cast<int>(f.unit) << "}";
}

std::ostream& operator<<(std::ostream& os, const Printer& f) {
    return os << "Printer{name='" << f.name << "', path='" << f.path << "'}";
}

} // namespace application::models
