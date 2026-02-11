#ifndef CLEANGRADUATOR_ICameraGridSettingsRepository_H
#define CLEANGRADUATOR_ICameraGridSettingsRepository_H

namespace application::ports {

    struct IVideoSourceGridSettingsRepository {
        virtual ~IVideoSourceGridSettingsRepository() = default;

        virtual dto::VideoSourceGridSettings loadGridSettings() const = 0;
        virtual void saveGridSettings(const dto::VideoSourceGridSettings& settings) = 0;


    };

} // namespace application::ports

#endif //CLEANGRADUATOR_ICameraGridSettingsRepository_H