#ifndef CLEANGRADUATOR_INIFILE_H
#define CLEANGRADUATOR_INIFILE_H

#include <string>
#include <unordered_map>
#include <cstdint>

namespace utils::ini {

    class IniSection {
    public:
        bool has(const std::string& key) const noexcept;

        std::string getString(const std::string& key,
                              const std::string& defaultValue = {}) const;

        bool getBool(const std::string& key, bool defaultValue = false) const;
        int getInt(const std::string& key, int defaultValue = 0) const;
        float getFloat(const std::string& key, float defaultValue = 0.0f) const;
        std::uint32_t getHex(const std::string& key,
                             std::uint32_t defaultValue = 0) const;

        void set(const std::string& key, const std::string& value);
        void set(const std::string& key, bool value);
        void set(const std::string& key, int value);
        void set(const std::string& key, float value);
        void setHex(const std::string& key, std::uint32_t value);

        const std::unordered_map<std::string, std::string>& values() const noexcept;

    private:
        std::unordered_map<std::string, std::string> values_;
    };

    class IniFile {
    public:
        bool load(const std::string& path);
        bool save(const std::string& path) const;

        const std::unordered_map<std::string, IniSection>& sections() const;

        IniSection& operator[](const std::string& section);
        const IniSection& operator[](const std::string& section) const;

        bool hasSection(const std::string& section) const noexcept;

    private:
        std::unordered_map<std::string, IniSection> sections_;
    };

} // namespace utils::ini



#endif //CLEANGRADUATOR_INIFILE_H