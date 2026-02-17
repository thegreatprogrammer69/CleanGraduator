#ifndef CLEANGRADUATOR_LOGSOURCESTORAGE_H
#define CLEANGRADUATOR_LOGSOURCESTORAGE_H
#include <unordered_map>
#include "application/ports/logging/ILogSourcesStorage.h"

namespace infra::storage {
    class LogSourcesStorage final : public application::ports::ILogSourcesStorage {
    public:
        void addLogSource(application::models::LogSource source);

        // ILogSourcesStorage
        std::vector<std::string> all() const override;
        std::optional<application::models::LogSource> at(const std::string& name) const override;

    private:
        std::unordered_map<std::string, application::models::LogSource> sources_;
    };
}


#endif //CLEANGRADUATOR_LOGSOURCESTORAGE_H