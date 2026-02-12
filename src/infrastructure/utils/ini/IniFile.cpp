#include "IniFile.h"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

namespace utils::ini {

namespace {

// ---------- helpers ----------

std::string trim(std::string s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(),
        [](unsigned char c) { return !std::isspace(c); }));
    s.erase(std::find_if(s.rbegin(), s.rend(),
        [](unsigned char c) { return !std::isspace(c); }).base(), s.end());
    return s;
}

bool parseBool(const std::string& value, bool defaultValue) {
    if (value == "true" || value == "1") return true;
    if (value == "false" || value == "0") return false;
    return defaultValue;
}

} // namespace

// ================= IniSection =================

bool IniSection::has(const std::string& key) const noexcept {
    return values_.find(key) != values_.end();
}

std::string IniSection::getString(const std::string& key,
                                  const std::string& defaultValue) const {
    auto it = values_.find(key);
    return it != values_.end() ? it->second : defaultValue;
}

bool IniSection::getBool(const std::string& key, bool defaultValue) const {
    auto it = values_.find(key);
    return it != values_.end()
        ? parseBool(it->second, defaultValue)
        : defaultValue;
}

int IniSection::getInt(const std::string& key, int defaultValue) const {
    auto it = values_.find(key);
    return it != values_.end()
        ? std::stoi(it->second)
        : defaultValue;
}

float IniSection::getFloat(const std::string& key, float defaultValue) const {
    auto it = values_.find(key);
    return it != values_.end()
        ? std::stof(it->second)
        : defaultValue;
}

std::uint32_t IniSection::getHex(const std::string& key,
                                 std::uint32_t defaultValue) const {
    auto it = values_.find(key);
    return it != values_.end()
        ? static_cast<std::uint32_t>(std::stoul(it->second, nullptr, 16))
        : defaultValue;
}

void IniSection::set(const std::string& key, const std::string& value) {
    values_[key] = value;
}

void IniSection::set(const std::string& key, bool value) {
    values_[key] = value ? "true" : "false";
}

void IniSection::set(const std::string& key, int value) {
    values_[key] = std::to_string(value);
}

void IniSection::set(const std::string& key, float value) {
    std::ostringstream oss;
    oss << value;
    values_[key] = oss.str();
}

void IniSection::setHex(const std::string& key, std::uint32_t value) {
    std::ostringstream oss;
    oss << std::hex << std::uppercase << value;
    values_[key] = oss.str();
}

const std::unordered_map<std::string, std::string>&
IniSection::values() const noexcept {
    return values_;
}

// ================= IniFile =================

bool IniFile::load(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return false;
    }

    sections_.clear();

    std::string line;
    std::string currentSection;

    while (std::getline(file, line)) {
        line = trim(line);

        if (line.empty() || line[0] == ';' || line[0] == '#') {
            continue;
        }

        if (line.front() == '[' && line.back() == ']') {
            currentSection = trim(line.substr(1, line.size() - 2));
            sections_[currentSection];
            continue;
        }

        auto pos = line.find('=');
        if (pos == std::string::npos || currentSection.empty()) {
            continue;
        }

        std::string key = trim(line.substr(0, pos));
        std::string value = trim(line.substr(pos + 1));

        sections_[currentSection].set(key, value);
    }

    return true;
}

bool IniFile::save(const std::string& path) const {
    std::ofstream file(path, std::ios::trunc);
    if (!file.is_open()) {
        return false;
    }

    for (const auto& sectionPair : sections_) {
        file << "[" << sectionPair.first << "]\n";

        for (const auto& kv : sectionPair.second.values()) {
            file << kv.first << "=" << kv.second << "\n";
        }

        file << "\n";
    }

    return true;
}

const std::unordered_map<std::string, IniSection> & IniFile::sections() const {
    return sections_;
}

IniSection& IniFile::operator[](const std::string& section) {
    return sections_[section];
}

const IniSection& IniFile::operator[](const std::string& section) const {
    static IniSection empty;
    auto it = sections_.find(section);
    return it != sections_.end() ? it->second : empty;
}

bool IniFile::hasSection(const std::string& section) const noexcept {
    return sections_.find(section) != sections_.end();
}

} // namespace utils::ini
