#include "Color.h"

#include <sstream>
#include <iomanip>
#include <stdexcept>

// Конструкторы
Color::Color() : r_(0), g_(0), b_(0) {}

Color::Color(uint8_t r, uint8_t g, uint8_t b)
    : r_(r), g_(g), b_(b) {}

// HEX -> Color
Color Color::FromHex(const std::string& hex) {
    if (hex.size() != 7 || hex[0] != '#') {
        throw std::invalid_argument("Invalid HEX format. Use #RRGGBB");
    }

    auto hexToByte = [](const std::string& s) -> uint8_t {
        return static_cast<uint8_t>(std::stoi(s, nullptr, 16));
    };

    return Color(
        hexToByte(hex.substr(1, 2)),
        hexToByte(hex.substr(3, 2)),
        hexToByte(hex.substr(5, 2))
    );
}

// Геттеры
uint8_t Color::R() const { return r_; }
uint8_t Color::G() const { return g_; }
uint8_t Color::B() const { return b_; }

// Сеттеры
void Color::SetR(uint8_t r) { r_ = r; }
void Color::SetG(uint8_t g) { g_ = g; }
void Color::SetB(uint8_t b) { b_ = b; }

void Color::Set(uint8_t r, uint8_t g, uint8_t b) {
    r_ = r;
    g_ = g;
    b_ = b;
}

// Color -> HEX
std::string Color::ToHex() const {
    std::ostringstream oss;
    oss << '#'
        << std::uppercase << std::hex << std::setfill('0')
        << std::setw(2) << static_cast<int>(r_)
        << std::setw(2) << static_cast<int>(g_)
        << std::setw(2) << static_cast<int>(b_);
    return oss.str();
}

// Операторы
bool Color::operator==(const Color& other) const {
    return r_ == other.r_ &&
           g_ == other.g_ &&
           b_ == other.b_;
}

bool Color::operator!=(const Color& other) const {
    return !(*this == other);
}