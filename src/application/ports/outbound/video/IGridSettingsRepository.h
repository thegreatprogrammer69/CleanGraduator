#ifndef CLEANGRADUATOR_IGRIDSETTINGSREPOSITORY_H
#define CLEANGRADUATOR_IGRIDSETTINGSREPOSITORY_H

namespace application::dto {
    class VideoSourceGridString;
}

namespace application::ports {
    class IGridSettingsRepository {
    public:
        virtual void save(const dto::VideoSourceGridString&) = 0;
        virtual ~IGridSettingsRepository() = default;
    };
}

#endif //CLEANGRADUATOR_IGRIDSETTINGSREPOSITORY_H