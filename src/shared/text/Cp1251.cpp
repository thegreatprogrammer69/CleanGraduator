#include "Cp1251.h"

#include <cstdint>

namespace shared::text {
namespace {

char32_t cp1251ByteToCodePoint(unsigned char ch) {
    if (ch < 0x80) {
        return ch;
    }

    if (ch >= 0xC0) {
        return 0x0410 + (ch - 0xC0);
    }

    switch (ch) {
        case 0x80: return 0x0402;
        case 0x81: return 0x0403;
        case 0x82: return 0x201A;
        case 0x83: return 0x0453;
        case 0x84: return 0x201E;
        case 0x85: return 0x2026;
        case 0x86: return 0x2020;
        case 0x87: return 0x2021;
        case 0x88: return 0x20AC;
        case 0x89: return 0x2030;
        case 0x8A: return 0x0409;
        case 0x8B: return 0x2039;
        case 0x8C: return 0x040A;
        case 0x8D: return 0x040C;
        case 0x8E: return 0x040B;
        case 0x8F: return 0x040F;
        case 0x90: return 0x0452;
        case 0x91: return 0x2018;
        case 0x92: return 0x2019;
        case 0x93: return 0x201C;
        case 0x94: return 0x201D;
        case 0x95: return 0x2022;
        case 0x96: return 0x2013;
        case 0x97: return 0x2014;
        case 0x99: return 0x2122;
        case 0x9A: return 0x0459;
        case 0x9B: return 0x203A;
        case 0x9C: return 0x045A;
        case 0x9D: return 0x045C;
        case 0x9E: return 0x045B;
        case 0x9F: return 0x045F;
        case 0xA0: return 0x00A0;
        case 0xA1: return 0x040E;
        case 0xA2: return 0x045E;
        case 0xA3: return 0x0408;
        case 0xA4: return 0x00A4;
        case 0xA5: return 0x0490;
        case 0xA6: return 0x00A6;
        case 0xA7: return 0x00A7;
        case 0xA8: return 0x0401;
        case 0xA9: return 0x00A9;
        case 0xAA: return 0x0404;
        case 0xAB: return 0x00AB;
        case 0xAC: return 0x00AC;
        case 0xAD: return 0x00AD;
        case 0xAE: return 0x00AE;
        case 0xAF: return 0x0407;
        case 0xB0: return 0x00B0;
        case 0xB1: return 0x00B1;
        case 0xB2: return 0x0406;
        case 0xB3: return 0x0456;
        case 0xB4: return 0x0491;
        case 0xB5: return 0x00B5;
        case 0xB6: return 0x00B6;
        case 0xB7: return 0x00B7;
        case 0xB8: return 0x0451;
        case 0xB9: return 0x2116;
        case 0xBA: return 0x0454;
        case 0xBB: return 0x00BB;
        case 0xBC: return 0x0458;
        case 0xBD: return 0x0405;
        case 0xBE: return 0x0455;
        case 0xBF: return 0x0457;
        default: return 0x003F;
    }
}

void appendUtf8(char32_t cp, std::string& out) {
    if (cp <= 0x7F) {
        out.push_back(static_cast<char>(cp));
    } else if (cp <= 0x7FF) {
        out.push_back(static_cast<char>(0xC0 | ((cp >> 6) & 0x1F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else if (cp <= 0xFFFF) {
        out.push_back(static_cast<char>(0xE0 | ((cp >> 12) & 0x0F)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else {
        out.push_back(static_cast<char>(0xF0 | ((cp >> 18) & 0x07)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    }
}

unsigned char codePointToCp1251(char32_t cp) {
    if (cp <= 0x7F) {
        return static_cast<unsigned char>(cp);
    }

    if (cp >= 0x0410 && cp <= 0x044F) {
        return static_cast<unsigned char>(0xC0 + (cp - 0x0410));
    }

    switch (cp) {
        case 0x0401: return 0xA8;
        case 0x0451: return 0xB8;
        case 0x2116: return 0xB9;
        case 0x0402: return 0x80;
        case 0x0403: return 0x81;
        case 0x201A: return 0x82;
        case 0x0453: return 0x83;
        case 0x201E: return 0x84;
        case 0x2026: return 0x85;
        case 0x2020: return 0x86;
        case 0x2021: return 0x87;
        case 0x20AC: return 0x88;
        case 0x2030: return 0x89;
        case 0x0409: return 0x8A;
        case 0x2039: return 0x8B;
        case 0x040A: return 0x8C;
        case 0x040C: return 0x8D;
        case 0x040B: return 0x8E;
        case 0x040F: return 0x8F;
        case 0x0452: return 0x90;
        case 0x2018: return 0x91;
        case 0x2019: return 0x92;
        case 0x201C: return 0x93;
        case 0x201D: return 0x94;
        case 0x2022: return 0x95;
        case 0x2013: return 0x96;
        case 0x2014: return 0x97;
        case 0x2122: return 0x99;
        case 0x0459: return 0x9A;
        case 0x203A: return 0x9B;
        case 0x045A: return 0x9C;
        case 0x045C: return 0x9D;
        case 0x045B: return 0x9E;
        case 0x045F: return 0x9F;
        case 0x00A0: return 0xA0;
        case 0x040E: return 0xA1;
        case 0x045E: return 0xA2;
        case 0x0408: return 0xA3;
        case 0x00A4: return 0xA4;
        case 0x0490: return 0xA5;
        case 0x00A6: return 0xA6;
        case 0x00A7: return 0xA7;
        case 0x00A9: return 0xA9;
        case 0x0404: return 0xAA;
        case 0x00AB: return 0xAB;
        case 0x00AC: return 0xAC;
        case 0x00AD: return 0xAD;
        case 0x00AE: return 0xAE;
        case 0x0407: return 0xAF;
        case 0x00B0: return 0xB0;
        case 0x00B1: return 0xB1;
        case 0x0406: return 0xB2;
        case 0x0456: return 0xB3;
        case 0x0491: return 0xB4;
        case 0x00B5: return 0xB5;
        case 0x00B6: return 0xB6;
        case 0x00B7: return 0xB7;
        case 0x0454: return 0xBA;
        case 0x00BB: return 0xBB;
        case 0x0458: return 0xBC;
        case 0x0405: return 0xBD;
        case 0x0455: return 0xBE;
        case 0x0457: return 0xBF;
        default: return static_cast<unsigned char>('?');
    }
}

char32_t nextUtf8CodePoint(std::string_view input, std::size_t& i) {
    const unsigned char c0 = static_cast<unsigned char>(input[i]);

    if (c0 < 0x80) {
        ++i;
        return c0;
    }

    if ((c0 & 0xE0) == 0xC0 && i + 1 < input.size()) {
        const unsigned char c1 = static_cast<unsigned char>(input[i + 1]);
        if ((c1 & 0xC0) == 0x80) {
            i += 2;
            return ((c0 & 0x1F) << 6) | (c1 & 0x3F);
        }
    }

    if ((c0 & 0xF0) == 0xE0 && i + 2 < input.size()) {
        const unsigned char c1 = static_cast<unsigned char>(input[i + 1]);
        const unsigned char c2 = static_cast<unsigned char>(input[i + 2]);
        if ((c1 & 0xC0) == 0x80 && (c2 & 0xC0) == 0x80) {
            i += 3;
            return ((c0 & 0x0F) << 12) | ((c1 & 0x3F) << 6) | (c2 & 0x3F);
        }
    }

    if ((c0 & 0xF8) == 0xF0 && i + 3 < input.size()) {
        const unsigned char c1 = static_cast<unsigned char>(input[i + 1]);
        const unsigned char c2 = static_cast<unsigned char>(input[i + 2]);
        const unsigned char c3 = static_cast<unsigned char>(input[i + 3]);
        if ((c1 & 0xC0) == 0x80 && (c2 & 0xC0) == 0x80 && (c3 & 0xC0) == 0x80) {
            i += 4;
            return ((c0 & 0x07) << 18) | ((c1 & 0x3F) << 12) | ((c2 & 0x3F) << 6) | (c3 & 0x3F);
        }
    }

    ++i;
    return static_cast<char32_t>('?');
}

} // namespace

std::string cp1251ToUtf8(std::string_view cp1251) {
    std::string result;
    result.reserve(cp1251.size() * 2);

    for (unsigned char ch : cp1251) {
        appendUtf8(cp1251ByteToCodePoint(ch), result);
    }

    return result;
}

std::string utf8ToCp1251(std::string_view utf8) {
    std::string result;
    result.reserve(utf8.size());

    for (std::size_t i = 0; i < utf8.size();) {
        const char32_t cp = nextUtf8CodePoint(utf8, i);
        result.push_back(static_cast<char>(codePointToCp1251(cp)));
    }

    return result;
}

} // namespace shared::text
