#include "ApplyVideoGridSettings.h"

#include "application/ports/outbound/ISettingsStorage.h"
#include "application/services/VideoSourceRepository.h"
#include "domain/ports/inbound/IVideoSource.h"

using namespace application::usecase;
using namespace application::models;

ApplyVideoGridSettings::ApplyVideoGridSettings(ApplyVideoGridSettingsPorts ports)
    : logger_(ports.logger)
    , settings_storage_(ports.settings_storage)
    , video_source_repository_(ports.video_source_repository)
{
    logger_.info("constructor called");
}

ApplyVideoGridSettings::~ApplyVideoGridSettings() {
    logger_.info("destructor called");
}

ApplyVideoGridSettingsResult ApplyVideoGridSettings::execute(const VideoGridSettings &settings) {
    std::unordered_set<int> new_active;

    // Открываем активные
    for (int idx : settings.activeIndexes()) {
        auto vs_opt = video_source_repository_.at(idx);
        if (!vs_opt.has_value())
            continue;

        auto& vs = vs_opt.value();
        vs.source.open();

        new_active.insert(idx);
    }

    // Закрываем остальные
    const auto all_sources = video_source_repository_.all();

    for (size_t i = 0; i < all_sources.size(); ++i) {
        if (new_active.find(static_cast<int>(i)) == new_active.end()) {
            all_sources[i].source.close();
        }
    }

    ApplyVideoGridSettingsResult result{};
    result.corrected_settings = VideoGridSettings(std::vector<int>(new_active.begin(), new_active.end()));

    settings_storage_.saveVideoGridSettings(result.corrected_settings);

    return result;
}
