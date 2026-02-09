#include "FileUserSettingsRepository.h"

#include <fstream>
#include <sstream>
#include <unordered_map>
#include <cctype>
#include <algorithm>
#include <limits>

namespace {

// ---------- small ini helpers ----------

inline std::string trim(std::string s) {
    auto notSpace = [](unsigned char ch) { return !std::isspace(ch); };
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), notSpace));
    s.erase(std::find_if(s.rbegin(), s.rend(), notSpace).base(), s.end());
    return s;
}

inline std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

// INI storage: section -> (key -> value)
using Ini = std::unordered_map<std::string, std::unordered_map<std::string, std::string>>;

Ini parseIni(std::istream& in) {
    Ini ini;
    std::string section;

    std::string line;
    while (std::getline(in, line)) {
        line = trim(line);
        if (line.empty()) continue;
        if (line[0] == ';' || line[0] == '#') continue;

        // section
        if (line.front() == '[' && line.back() == ']') {
            section = toLower(trim(line.substr(1, line.size() - 2)));
            continue;
        }

        // key=value
        auto eq = line.find('=');
        if (eq == std::string::npos) continue;

        std::string key = toLower(trim(line.substr(0, eq)));
        std::string val = trim(line.substr(eq + 1));

        ini[section][key] = val;
    }

    return ini;
}

bool writeTextFileAtomic(const std::string& path, const std::string& content) {
    // very simple "atomic-ish": write to temp then replace
    const std::string tmp = path + ".tmp";

    {
        std::ofstream out(tmp, std::ios::binary | std::ios::trunc);
        if (!out) return false;
        out << content;
        if (!out.good()) return false;
    }

    // best effort rename
    std::remove(path.c_str());
    return std::rename(tmp.c_str(), path.c_str()) == 0;
}

std::optional<std::string> getStr(const Ini& ini, const std::string& section, const std::string& key) {
    auto sIt = ini.find(section);
    if (sIt == ini.end()) return std::nullopt;
    auto kIt = sIt->second.find(key);
    if (kIt == sIt->second.end()) return std::nullopt;
    return kIt->second;
}

std::optional<int> parseInt(const std::string& s) {
    std::string t = trim(s);
    if (t.empty()) return std::nullopt;

    // strict-ish parse
    char* end = nullptr;
    long v = std::strtol(t.c_str(), &end, 10);
    if (end == t.c_str() || *end != '\0') return std::nullopt;
    if (v < std::numeric_limits<int>::min() || v > std::numeric_limits<int>::max()) return std::nullopt;
    return static_cast<int>(v);
}

std::optional<unsigned int> parseUInt(const std::string& s) {
    std::string t = trim(s);
    if (t.empty()) return std::nullopt;

    char* end = nullptr;
    unsigned long v = std::strtoul(t.c_str(), &end, 10);
    if (end == t.c_str() || *end != '\0') return std::nullopt;
    if (v > std::numeric_limits<unsigned int>::max()) return std::nullopt;
    return static_cast<unsigned int>(v);
}

bool parseBool(const std::string& s, bool fallback) {
    std::string t = toLower(trim(s));
    if (t == "1" || t == "true" || t == "yes" || t == "on") return true;
    if (t == "0" || t == "false" || t == "no"  || t == "off") return false;
    return fallback;
}

std::string boolToIni(bool v) { return v ? "1" : "0"; }

std::string optIntToIni(const std::optional<int>& v) {
    return v ? std::to_string(*v) : std::string{};
}

// ---------- mapping helpers (Ids) ----------
// предполагаем, что у record-id есть поле int value, как в примере.

template <typename IdT>
std::optional<IdT> optIdFromIni(const Ini& ini, const std::string& sec, const std::string& key) {
    auto s = getStr(ini, sec, key);
    if (!s) return std::nullopt;

    auto v = parseInt(*s);
    if (!v) return std::nullopt;

    return IdT{*v};
}

template <typename IdT>
std::string optIdToIni(const std::optional<IdT>& id) {
    if (!id) return {};
    return std::to_string(id->value);
}

} // namespace

// --------- your namespace ----------
namespace infrastructure::settings {

FileUserSettingsRepository::FileUserSettingsRepository(std::string iniFilePath)
    : path_(std::move(iniFilePath)) {}

application::dto::UserSettings FileUserSettingsRepository::load() const {
    application::dto::UserSettings dto{};

    std::ifstream in(path_, std::ios::binary);
    if (!in) {
        // файла нет — вернём дефолты DTO
        return dto;
    }

    const Ini ini = parseIni(in);

    // selections
    dto.printerId       = optIdFromIni<application::model::PrinterRecordId>(ini, "selections", "printer_id");
    dto.displacementId  = optIdFromIni<application::model::DisplacementRecordId>(ini, "selections", "displacement_id");
    dto.gaugeId         = optIdFromIni<application::model::GaugeRecordId>(ini, "selections", "gauge_id");
    dto.precisionId     = optIdFromIni<application::model::PrecisionRecordId>(ini, "selections", "precision_id");
    dto.pressureUnitId  = optIdFromIni<application::model::PressureUnitRecordId>(ini, "selections", "pressure_unit_id");

    // inputs: camera_string
    {
        auto s = getStr(ini, "inputs", "camera_string");
        // если ключа нет — оставляем dto.cameraString как есть
        // но у тебя cameraString НЕ optional, значит нужно выбрать дефолт.
        // Самый безопасный дефолт: пустая строка => empty() == true
        if (s) dto.cameraString = application::model::CameraString(*s);
        else   dto.cameraString = application::model::CameraString("");
    }

    // inputs: frameCrosshair
    {
        // предполагаем, что в FrameCrosshair поля: visible/visable, radius, color
        // (я использую `visible`, но если у тебя пока `visable` — подставь нужное имя)
        auto vStr = getStr(ini, "inputs", "frame_crosshair_visible");
        auto rStr = getStr(ini, "inputs", "frame_crosshair_radius");
        auto cStr = getStr(ini, "inputs", "frame_crosshair_color");

        // дефолты берём из текущего dto.frameCrosshair (если у него есть дефолтные значения)
        // иначе: visible=false, radius=0, color=0xFFFFFFFF — по ситуации
        bool visibleFallback = dto.frameCrosshair.visible;
        dto.frameCrosshair.visible = vStr ? parseBool(*vStr, visibleFallback) : visibleFallback;

        if (rStr) {
            if (auto r = parseUInt(*rStr)) dto.frameCrosshair.radius = *r;
        }
        if (cStr) {
            if (auto c = parseUInt(*cStr)) dto.frameCrosshair.color = *c;
        }
    }

    // ui flags
    {
        auto s = getStr(ini, "ui", "open_cameras_on_startup");
        dto.openCamerasOnStartup = s ? parseBool(*s, dto.openCamerasOnStartup) : dto.openCamerasOnStartup;
    }

    return dto;
}

void FileUserSettingsRepository::save(const application::dto::UserSettings& settings) {
    std::ostringstream out;

    // [selections]
    out << "[selections]\n";
    out << "printer_id="        << optIdToIni(settings.printerId) << "\n";
    out << "displacement_id="   << optIdToIni(settings.displacementId) << "\n";
    out << "gauge_id="          << optIdToIni(settings.gaugeId) << "\n";
    out << "precision_id="      << optIdToIni(settings.precisionId) << "\n";
    out << "pressure_unit_id="  << optIdToIni(settings.pressureUnitId) << "\n";
    out << "\n";

    // [inputs]
    out << "[inputs]\n";
    out << "camera_string=" << settings.cameraString.toString() << "\n";
    out << "frame_crosshair_visible=" << boolToIni(settings.frameCrosshair.visible) << "\n";
    out << "frame_crosshair_radius="  << settings.frameCrosshair.radius << "\n";
    out << "frame_crosshair_color="   << settings.frameCrosshair.color << "\n";
    out << "\n";

    // [ui]
    out << "[ui]\n";
    out << "open_cameras_on_startup=" << boolToIni(settings.openCamerasOnStartup) << "\n";

    // write
    const std::string content = out.str();
    (void)writeTextFileAtomic(path_, content);
    // Можно вернуть bool/expected и пробрасывать ошибку выше,
    // но контракт порта сейчас void — оставляем best-effort.
}

} // namespace infrastructure::settings
