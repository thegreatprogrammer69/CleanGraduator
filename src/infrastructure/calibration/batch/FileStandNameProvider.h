#ifndef CLEANGRADUATOR_FILESTANDNAMEPROVIDER_H
#define CLEANGRADUATOR_FILESTANDNAMEPROVIDER_H

#include "IStandNameProvider.h"
#include "domain/fmt/Logger.h"

#include <QString>

namespace domain::ports {
    struct ILogger;
}

namespace infra::calib {
    class FileStandNameProvider final : public IStandNameProvider {
    public:
        FileStandNameProvider(domain::ports::ILogger& logger, QString stand_file_path);
        std::optional<std::string> current() override;

    private:
        fmt::Logger logger_;
        QString stand_file_path_;
    };
}

#endif //CLEANGRADUATOR_FILESTANDNAMEPROVIDER_H
