#ifndef CLEANGRADUATOR_LOGVIEWERVIEWMODEL_H
#define CLEANGRADUATOR_LOGVIEWERVIEWMODEL_H
#include <memory>
#include <string>

#include "application/models/logging/LogSource.h"
#include "viewmodels/Observable.h"

namespace application::ports {
    struct ILogSource;
    struct ILogSourcesStorage;
}


namespace mvvm {
    class LogViewerViewModel {
    public:
        explicit LogViewerViewModel(application::ports::ILogSourcesStorage& log_source_storage);
        ~LogViewerViewModel();

        Observable<std::optional<application::models::LogSource>> log_source{};

        std::vector<std::string> loadNames() const;
        void selectLogSource(const std::string& name);

    private:
        application::ports::ILogSourcesStorage& log_source_storage_;
    };
}


#endif //CLEANGRADUATOR_LOGVIEWERVIEWMODEL_H