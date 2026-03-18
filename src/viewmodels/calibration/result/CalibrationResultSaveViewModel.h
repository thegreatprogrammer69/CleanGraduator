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
    application::usecase::SaveCalibrationResult& use_case;
    domain::ports::ICalibrationResultSource& result_source;
};

class CalibrationResultSaveViewModel final : public domain::ports::ICalibrationResultObserver {
public:
    enum class SaveState {
        Unsaved,
        Saving,
        Saved,
        Error,
    };

    explicit CalibrationResultSaveViewModel(CalibrationResultSaveViewModelDeps deps);
    ~CalibrationResultSaveViewModel() override;

    void save();
    void saveAs(const std::filesystem::path& directory);

    void onCalibrationResultUpdated(const domain::common::CalibrationResult& result) override;

    Observable<std::string> party_label{std::string("Партия № —")};
    Observable<std::string> status_text{std::string("Не сохранено")};
    Observable<SaveState> status_state{SaveState::Unsaved};
    Observable<std::string> error_text{std::string()};
    Observable<bool> can_save{false};
    Observable<bool> can_show_in_explorer{false};
    Observable<bool> can_save_as{false};
    Observable<std::optional<std::filesystem::path>> saved_directory{std::nullopt};

private:
    void refreshPreview();
    void markUnsaved();
    void applySaveResult(const application::usecase::SaveCalibrationResult::Result& result, bool keep_preview_party);

    application::usecase::SaveCalibrationResult& use_case_;
    domain::ports::ICalibrationResultSource& result_source_;
};

} // namespace mvvm

#endif //CLEANGRADUATOR_CALIBRATIONRESULTSAVEVIEWMODEL_H
