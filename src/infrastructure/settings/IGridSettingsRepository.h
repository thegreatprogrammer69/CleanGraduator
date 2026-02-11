#ifndef CLEANGRADUATOR_FILEUSERSETTINGSREPOSITORY_H
#define CLEANGRADUATOR_FILEUSERSETTINGSREPOSITORY_H

#include <string>
#include <optional>

#include "application/ports/repo/IGridSettingsRepository.h"

namespace infrastructure::settings {

    class GridSettingsRepository final
        : public application::ports::IGridSettingsRepository {
    public:
        explicit GridSettingsRepository(std::string iniFilePath);
        void save(const application::dto::VideoSourceGridSettings &) override;
        application::dto::VideoSourceGridSettings load() const override;

    private:
        application::dto::VideoSourceGridSettings loadFromFile() const;

    private:
        std::string path_;

        // ---- Cache ----
        mutable std::optional<application::dto::VideoSourceGridSettings> cache_;
    };

} // namespace infrastructure::settings

#endif
