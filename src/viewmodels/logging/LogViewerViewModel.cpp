#include "LogViewerViewModel.h"

#include "application/ports/outbound/logging/ILogSource.h"
#include "application/ports/outbound/logging/ILogSourcesStorage.h"

using namespace mvvm;
using namespace application::models;
using namespace application::ports;

LogViewerViewModel::LogViewerViewModel(
        ILogSourcesStorage& storage)
    : storage_(storage)
{
}

LogViewerViewModel::~LogViewerViewModel()
{
    auto current = log_source.get_copy();
    if (current)
        current->source->removeSink(*this);
}

std::vector<std::string> LogViewerViewModel::loadNames() const
{
    return storage_.all();
}

void LogViewerViewModel::selectLogSource(const std::string& name)
{
    auto old_source = log_source.get_copy();
    auto new_source = storage_.at(name);

    if (old_source == new_source)
        return;

    if (old_source)
        old_source->source->removeSink(*this);

    if (new_source)
        new_source->source->addSink(*this);

    log_source.set(new_source);
}

void LogViewerViewModel::onLog(const LogEntry& log)
{
    // Фильтр: если источник уже снят — игнорируем
    if (!log_source.get_copy())
        return;

    new_log.set(log);
}
