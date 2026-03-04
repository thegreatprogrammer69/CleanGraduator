#ifndef GRADUATOR_STRING_FORMATTING
#define GRADUATOR_STRING_FORMATTING

#include <algorithm>
#include <vector>
#include <string>
#include <sstream>
#include <locale>
#include <iomanip>
#include <type_traits>
#include <cctype>
#include <optional>
#include <utility>
#include "fmt_domain.h"


namespace fmt {

struct result {
    std::string str;
    bool ok = true;
};

namespace detail {

enum class base_t { dec, hex_lower, hex_upper, oct, bin };

struct spec {
    bool has_spec = false;

    // float: {:.2f}
    bool float_fixed = false;
    int  precision = -1;

    // integer width/pad: {:08d} or {:8d}
    int  width = 0;
    char fill = ' ';
    bool zero_pad = false;

    // integer base: {:x} {:X} {:o} {:b}
    base_t base = base_t::dec;

    // thousands grouping: {:n}
    bool grouped = false;
};

template <class T>
struct is_std_vector : std::false_type {};

template <class T, class A>
struct is_std_vector<std::vector<T, A>> : std::true_type {};

template <class T>
inline constexpr bool is_std_vector_v = is_std_vector<std::decay_t<T>>::value;

template <class T>
struct is_std_optional : std::false_type {};

template <class T>
struct is_std_optional<std::optional<T>> : std::true_type {};

template <class T>
inline constexpr bool is_std_optional_v = is_std_optional<std::decay_t<T>>::value;


inline bool is_digit(char c) { return c >= '0' && c <= '9'; }

inline int parse_int(const std::string& s, std::size_t& i) {
    int v = 0;
    bool any = false;
    while (i < s.size() && is_digit(s[i])) {
        any = true;
        v = v * 10 + (s[i] - '0');
        ++i;
    }
    return any ? v : -1;
}

// Parse inside {...}. Supported: "", ":.2f", ":08d", ":8d", ":x", ":X", ":o", ":b", ":n"
inline bool parse_spec(const std::string& fmt, std::size_t lbrace, std::size_t rbrace, spec& out) {
    out = spec{};
    if (lbrace + 1 > rbrace) return false;

    // content between braces
    std::string inner = fmt.substr(lbrace + 1, rbrace - (lbrace + 1));
    if (inner.empty()) return true;

    if (inner[0] != ':') return false;
    out.has_spec = true;

    std::size_t i = 1; // after ':'
    if (i >= inner.size()) return true;

    // special: .precision f  => {:.2f}
    if (inner[i] == '.') {
        ++i;
        if (i >= inner.size()) return false;
        std::size_t j = i;
        int p = parse_int(inner, i);
        if (p < 0 || i == j) return false;
        out.precision = p;
        if (i < inner.size() && inner[i] == 'f') {
            out.float_fixed = true;
            ++i;
        } else {
            // if no 'f', still treat as precision request for floats
        }
        return i == inner.size();
    }

    // grouping: {:n} (can be combined? keep it simple: only n alone)
    if (inner[i] == 'n' && i + 1 == inner.size()) {
        out.grouped = true;
        return true;
    }

    // base: x/X/o/b (alone)
    if ((inner[i] == 'x' || inner[i] == 'X' || inner[i] == 'o' || inner[i] == 'b') && i + 1 == inner.size()) {
        if (inner[i] == 'x') out.base = base_t::hex_lower;
        if (inner[i] == 'X') out.base = base_t::hex_upper;
        if (inner[i] == 'o') out.base = base_t::oct;
        if (inner[i] == 'b') out.base = base_t::bin;
        return true;
    }

    // width / pad: {:08d} or {:8d}
    // optional leading '0' => zero pad
    if (inner[i] == '0') {
        out.zero_pad = true;
        out.fill = '0';
        ++i;
    }
    std::size_t j = i;
    int w = parse_int(inner, i);
    if (w < 0 || i == j) return false;
    out.width = w;

    // optional type letter (only d supported, and can be omitted)
    if (i < inner.size()) {
        char t = inner[i];
        if (t == 'd') { ++i; }
        else return false;
    }
    return i == inner.size();
}

// ---------- to_string helpers ----------

inline std::string to_string_bool(bool v) {
    return v ? "true" : "false";
}

inline std::string to_string_cstr(const char* s) {
    return s ? std::string(s) : std::string{};
}

inline std::string stream_to_string(const void* ptr) {
    std::ostringstream oss;
    oss << ptr;
    return oss.str();
}

template <class T>
inline std::string stream_to_string(const T& v, const std::locale& loc) {
    std::ostringstream oss;
    oss.imbue(loc);
    oss << v;
    return oss.str();
}

template <class T> inline std::string format_value(const T& value, const spec& sp, const std::locale& loc);

// integer formatting with base/width/zero-pad
template <class Int>
inline std::string format_integer(Int v, const spec& sp, const std::locale& loc) {
    using U = typename std::make_unsigned<typename std::remove_reference<Int>::type>::type;

    bool neg = false;
    U uv{};
    if constexpr (std::is_signed<Int>::value) {
        if (v < 0) { neg = true; uv = static_cast<U>(-v); }
        else uv = static_cast<U>(v);
    } else {
        uv = static_cast<U>(v);
    }

    auto digits_bin = [](U x) {
        int c = 1;
        while (x >>= 1) ++c;
        return c;
    };

    std::string s;

    // base conversion
    if (sp.base == base_t::dec && sp.grouped) {
        std::ostringstream oss;
        oss.imbue(loc);
        // use locale grouping with put_money? Not needed. Use standard locale numpunct for grouping via iostream:
        // std::ostringstream with locale will apply grouping when using std::showbase? Actually grouping happens with
        // numpunct in facet when using std::put_money / or when formatting? For integers, grouping is applied by
        // num_put when the stream has the locale. So just output the integer.
        if (neg) oss << '-';
        oss << uv;
        s = oss.str();
    } else {
        int base = 10;
        bool upper = false;
        if (sp.base == base_t::hex_lower || sp.base == base_t::hex_upper) { base = 16; upper = (sp.base == base_t::hex_upper); }
        else if (sp.base == base_t::oct) base = 8;
        else if (sp.base == base_t::bin) base = 2;

        if (base == 10) {
            s = std::to_string(uv);
        } else if (base == 2) {
            int bits = digits_bin(uv);
            s.reserve(bits);
            for (int i = bits - 1; i >= 0; --i) {
                s.push_back(((uv >> i) & 1u) ? '1' : '0');
            }
        } else {
            static const char* low = "0123456789abcdef";
            static const char* up  = "0123456789ABCDEF";
            const char* alph = upper ? up : low;
            U x = uv;
            if (x == 0) s = "0";
            else {
                while (x) {
                    s.push_back(alph[x % base]);
                    x /= base;
                }
                std::reverse(s.begin(), s.end());
            }
        }

        if (neg) s.insert(s.begin(), '-');
    }

    // width/pad (left)
    if (sp.width > 0 && static_cast<int>(s.size()) < sp.width) {
        int pad = sp.width - static_cast<int>(s.size());
        // if negative and zero-pad, keep '-' at front
        if (sp.zero_pad && !s.empty() && s[0] == '-') {
            std::string zeros(pad, '0');
            s = "-" + zeros + s.substr(1);
        } else {
            s.insert(s.begin(), pad, sp.fill);
        }
    }

    return s;
}

// float formatting: precision + fixed
template <class F>
inline std::string format_float(F v, const spec& sp, const std::locale& loc) {
    std::ostringstream oss;
    oss.imbue(loc);
    if (sp.float_fixed) oss << std::fixed;
    if (sp.precision >= 0) oss << std::setprecision(sp.precision);
    oss << v;
    return oss.str();
}


template <class Vec>
inline std::string format_vector(const Vec& v, const spec& sp, const std::locale& loc) {
    (void)sp; // спецификации для контейнеров пока игнорируем (можно расширить позже)
    using Elem = typename Vec::value_type;

    std::string out;
    out.push_back('[');

    bool first = true;
    for (const auto& e : v) {
        if (!first) out += ", ";
        first = false;

        // Важно: рекурсивно форматируем элемент тем же механизмом.
        out += format_value<Elem>(e, spec{}, loc);
    }

    out.push_back(']');
    return out;
}

template <class T>
inline std::string format_optional(const std::optional<T>& opt, const spec& sp, const std::locale& loc)
{
    (void)sp; // спецификации пока игнорируем

    if (!opt) {
        return "null";
    }

    // Рекурсивно форматируем значение
    return format_value<T>(*opt, spec{}, loc);
}


// Decide how to stringify with spec
template <class T>
inline std::string format_value(const T& value, const spec& sp, const std::locale& loc) {
    // bool
    if constexpr (std::is_same<T, bool>::value) {
        (void)sp;
        return to_string_bool(value);
    }
    // std::string
    else if constexpr (std::is_same<T, std::string>::value) {
        (void)sp;
        return value;
    }
    // c-string
    else if constexpr (std::is_same<T, const char*>::value || std::is_same<T, char*>::value) {
        (void)sp;
        return to_string_cstr(value);
    }
    // floats
    else if constexpr (std::is_floating_point<T>::value) {
        return format_float(value, sp, loc);
    }
    // integers (includes char? treat char as integer by default)
    else if constexpr (std::is_integral<T>::value) {
        return format_integer(value, sp, loc);
    }
    // pointers
    else if constexpr (std::is_pointer<T>::value) {
        (void)sp;
        return stream_to_string(static_cast<const void*>(value));
    }
    // std::vector<T>
    else if constexpr (is_std_vector_v<T>) {
        return format_vector(value, sp, loc);
    }
    // std::optional<T>
    else if constexpr (is_std_optional_v<T>) {
        return format_optional(value, sp, loc);
    }
    // fallback: operator<<
    else {
        (void)sp; // specs for user types are not supported (kept minimal)
        return stream_to_string(value, loc);
    }
}

inline void replace_first(std::string& s, std::size_t pos, std::size_t len, const std::string& repl) {
    s.replace(pos, len, repl);
}




} // namespace detail

// --------- Public API ---------

// Writes into out. Returns ok=false on any format mismatch, but DOES NOT throw.
template <typename... Args>
bool format_to(std::string& out, const std::string& format, const Args&... args) {
    out = format;
    bool ok = true;
    std::locale loc; // current global locale

    std::size_t search_from = 0;
    std::size_t arg_index = 0;

    auto apply_one = [&](auto&& arg) {
        // find next '{'
        std::size_t l = out.find('{', search_from);
        while (l != std::string::npos) {
            if (l + 1 < out.size() && out[l + 1] == '{') {
                // escaped "{{" -> keep one '{'
                out.erase(l, 1);
                search_from = l + 1;
                l = out.find('{', search_from);
                continue;
            }
            break;
        }

        if (l == std::string::npos) {
            ok = false; // too many args
            return;
        }

        // find matching '}'
        std::size_t r = out.find('}', l + 1);
        if (r == std::string::npos) {
            ok = false; // missing closing brace
            return;
        }
        if (r + 1 < out.size() && out[r + 1] == '}') {
            // "}" followed by "}" is not a valid placeholder close, it's escaped "}}"
            // We'll treat placeholder close at r, and next '}' stays.
        }

        detail::spec sp;
        if (!detail::parse_spec(out, l, r, sp)) {
            ok = false;
            // degrade: treat as "{}"
            sp = detail::spec{};
        }

        std::string repl = detail::format_value(std::forward<decltype(arg)>(arg), sp, loc);
        detail::replace_first(out, l, (r - l + 1), repl);
        search_from = l + repl.size();
        ++arg_index;
    };

    // Apply each argument
    (apply_one(args), ...);

    // After args, ensure no remaining unescaped placeholders
    // Also unescape "}}" and "{{"
    // First, check remaining placeholders
    std::size_t l = out.find('{');
    while (l != std::string::npos) {
        if (l + 1 < out.size() && out[l + 1] == '{') {
            l = out.find('{', l + 2);
            continue;
        }
        // a '{' that isn't escaped means not enough args / invalid
        ok = false;
        break;
    }

    // Unescape braces: "{{"->"{", "}}"->"}"
    // Do it in a simple pass
    for (std::size_t i = 0; i + 1 < out.size();) {
        if (out[i] == '{' && out[i + 1] == '{') {
            out.erase(i, 1);
            ++i;
        } else if (out[i] == '}' && out[i + 1] == '}') {
            out.erase(i, 1);
            ++i;
        } else {
            ++i;
        }
    }

    return ok;
}

// Convenience: returns only string (ошибки тихо игнорируются — строка всё равно вернётся).
template <typename... Args>
std::string format(const std::string& format, const Args&... args) {
    std::string out;
    (void)format_to(out, format, args...);
    return out;
}

// If you want to know ok-state.
template <typename... Args>
result format_res(const std::string& format, const Args&... args) {
    result r;
    r.ok = format_to(r.str, format, args...);
    return r;
}

} // namespace format


#endif