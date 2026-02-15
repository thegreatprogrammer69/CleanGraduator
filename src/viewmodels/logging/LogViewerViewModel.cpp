#include "LogViewerViewModel.h"
#include "application/ports/outbound/logging/ILogSourcesStorage.h"

mvvm::LogViewerViewModel::LogViewerViewModel(application::ports::ILogSourcesStorage &log_source_storage)
    : log_source_storage_(log_source_storage)
{
}

mvvm::LogViewerViewModel::~LogViewerViewModel() {
}

std::vector<std::string> mvvm::LogViewerViewModel::loadNames() const {
    return log_source_storage_.all();
}

void mvvm::LogViewerViewModel::selectLogSource(const std::string &name) {
    log_source.set(log_source_storage_.at(name));
}
