#include "SaveCalibrationResult.h"

#include "domain/ports/calibration/result/ICalibrationResultSource.h"

namespace application::usecase {

SaveCalibrationResult::SaveCalibrationResult(SaveCalibrationResultDeps deps)
    : result_source_(deps.result_source)
    , batch_context_provider_(deps.batch_context_provider)
    , batch_context_allocator_(deps.batch_context_allocator)
    , saver_(deps.saver)
    , explorer_(deps.explorer)
{
    result_source_.addObserver(*this);
    if (const auto current = result_source_.currentResult(); current.has_value()) {
        current_result_ = current;
        ensureBatchContext();
    }
}

SaveCalibrationResult::~SaveCalibrationResult() {
    result_source_.removeObserver(*this);
}

SaveCalibrationResult::State SaveCalibrationResult::state() const {
    return {
        batch_context_provider_.current(),
        status_,
        error_,
        current_result_.has_value(),
        can_reveal_
    };
}

application::ports::ICalibrationResultSaver::Result SaveCalibrationResult::save() {
    if (!current_result_.has_value()) {
        status_ = Status::Error;
        error_ = "Нет результата для сохранения";
        return {false, error_};
    }

    ensureBatchContext();
    const auto batch_context = batch_context_provider_.current();
    if (!batch_context.has_value()) {
        status_ = Status::Error;
        error_ = "Не удалось подготовить номер партии";
        return {false, error_};
    }

    return saveToDirectory(batch_context->full_path);
}

application::ports::ICalibrationResultSaver::Result
SaveCalibrationResult::saveAs(const std::filesystem::path& directory) {
    if (!current_result_.has_value()) {
        status_ = Status::Error;
        error_ = "Нет результата для сохранения";
        return {false, error_};
    }

    if (directory.empty()) {
        status_ = Status::Error;
        error_ = "Не выбрана папка для сохранения";
        return {false, error_};
    }

    return saveToDirectory(directory);
}

application::ports::ICalibrationResultExplorer::Result SaveCalibrationResult::revealInExplorer() {
    if (!last_saved_directory_.has_value()) {
        return {false, "Папка сохранения ещё не подготовлена"};
    }

    if (!can_reveal_) {
        return {false, "Сначала сохраните результат"};
    }

    return explorer_.reveal(*last_saved_directory_);
}

void SaveCalibrationResult::onCalibrationResultUpdated(const domain::common::CalibrationResult& result) {
    current_result_ = result;
    status_ = Status::Unsaved;
    error_.clear();
    can_reveal_ = false;
    last_saved_directory_.reset();
    batch_context_allocator_.allocate();
}

application::ports::ICalibrationResultSaver::Result
SaveCalibrationResult::saveToDirectory(const std::filesystem::path& directory) {
    status_ = Status::Saving;
    error_.clear();

    const auto result = saver_.save(*current_result_, directory);
    if (result.success) {
        status_ = Status::Saved;
        can_reveal_ = true;
        last_saved_directory_ = directory;
        return result;
    }

    status_ = Status::Error;
    error_ = result.error;
    can_reveal_ = false;
    last_saved_directory_.reset();
    return result;
}

void SaveCalibrationResult::ensureBatchContext() {
    if (batch_context_provider_.current().has_value()) {
        return;
    }

    batch_context_allocator_.allocate();
}

}
