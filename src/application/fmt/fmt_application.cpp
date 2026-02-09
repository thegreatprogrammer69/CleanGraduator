#include "fmt_application.h"

#include "application/model/DisplacementRecord.h"
#include "../dto/GaugeRecord.h"
#include "../dto/PrinterRecord.h"

#include <ostream>
#include <string>
#include <vector>
#include <codecvt>
#include <locale>

namespace {

    // wstring → UTF-8 для std::ostream
    inline std::string to_utf8(const std::wstring& ws) {
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
        return conv.to_bytes(ws);
    }

    template <typename T>
    std::ostream& print_vector(std::ostream& os, const std::vector<T>& v) {
        os << "[";
        for (size_t i = 0; i < v.size(); ++i) {
            if (i != 0) os << ", ";
            os << v[i];
        }
        os << "]";
        return os;
    }

} // namespace

std::ostream& operator<<(std::ostream& os,
                         const application::model::DisplacementRecord& f) {
    os << "DisplacementRecord{"
       << "name=\"" << to_utf8(f.name) << "\", "
       << "id=" << f.id
       << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os,
                         const application::model::GaugeRecord& f) {
    os << "GaugeRecord{"
       << "name=\"" << to_utf8(f.name) << "\", "
       << "values=";
    print_vector(os, f.values);
    os << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os,
                         const application::model::PrinterRecord& f) {
    os << "PrinterRecord{"
       << "name=\"" << to_utf8(f.name) << "\", "
       << "path=\"" << to_utf8(f.path) << "\""
       << "}";
    return os;
}
