#ifndef CLEANGRADUATOR_LOGVIEWERVIEWMODEL_H
#define CLEANGRADUATOR_LOGVIEWERVIEWMODEL_H

#include <optional>
#include <string>
#include <vector>

#include "application/models/logging/LogSource.h"
#include "application/models/logging/LogEntry.h"
#include "application/ports/outbound/logging/ILogSink.h"
#include "viewmodels/Observable.h"

namespace application::ports {
    struct ILogSourcesStorage;
}

namespace mvvm {

    class LogViewerViewModel final : public application::ports::ILogSink
    {
    public:
        explicit LogViewerViewModel(
            application::ports::ILogSourcesStorage& storage);

        ~LogViewerViewModel() override;

        // Текущий выбранный источник
        Observable<std::optional<application::models::LogSource>> log_source{};

        // Новое поступившее событие
        Observable<application::models::LogEntry> new_log{};

        std::vector<std::string> loadNames() const;
        void selectLogSource(const std::string& name);

    protected:
        void onLog(const application::models::LogEntry& log) override;

    private:
        application::ports::ILogSourcesStorage& storage_;
    };

}

#endif
