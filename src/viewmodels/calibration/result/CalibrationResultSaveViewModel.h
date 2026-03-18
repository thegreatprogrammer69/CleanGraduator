#ifndef CLEANGRADUATOR_CALIBRATIONRESULTSAVEVIEWMODEL_H
#define CLEANGRADUATOR_CALIBRATIONRESULTSAVEVIEWMODEL_H

#include <filesystem>
#include <optional>
#include <string>

#include "application/usecases/calibration/SaveCalibrationResult.h"
#include "domain/ports/calibration/result/ICalibrationResultObserver.h"
#include "viewmodels/Observable.h"

namespace domain::ports {
    class ICalibrationResultSource;
}

namespace mvvm {

struct CalibrationResultSaveViewModelDeps {
    domain::ports::ICalibrationResultSource& result_source;
    application::usecase::SaveCalibrationResult& save_result;
};

class CalibrationResultSaveViewModel final : public domain::ports::ICalibrationResultObserver {
public:
    enum class SaveState {
        Saved,
        Saving,
        Unsaved,
        Error
    };

    explicit CalibrationResultSaveViewModel(CalibrationResultSaveViewModelDeps deps);
    ~CalibrationResultSaveViewModel() override;

    void save();
    void saveAs(const std::filesystem::path& directory);

    void onCalibrationResultUpdated(const domain::common::CalibrationResult& result) override;

    Observable<std::string> party_title{std::string("Партия № —")};
    Observable<std::string> status_text{std::string("Не сохранено")};
    Observable<std::string> status_color{std::string("#6b7280")};
    Observable<std::string> error_text{std::string()};
    Observable<bool> can_save{false};
    Observable<bool> can_save_as{false};
    Observable<bool> can_show_in_explorer{false};
    Observable<std::optional<std::filesystem::path>> last_saved_directory{std::nullopt};

private:
    void refreshDraftState();
    void applyState(SaveState state, const std::string& error = std::string());
    bool hasResult() const;

    domain::ports::ICalibrationResultSource& result_source_;
    application::usecase::SaveCalibrationResult& save_result_;
    std::optional<application::models::BatchContext> draft_batch_;
};

}

#endif //CLEANGRADUATOR_CALIBRATIONRESULTSAVEVIEWMODEL_H
