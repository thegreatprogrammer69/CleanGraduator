#include "fmt_application.h"

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
}
