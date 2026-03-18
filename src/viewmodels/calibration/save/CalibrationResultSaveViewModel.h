#ifndef CLEANGRADUATOR_CALIBRATIONRESULTSAVEVIEWMODEL_H
#define CLEANGRADUATOR_CALIBRATIONRESULTSAVEVIEWMODEL_H

#include <filesystem>
#include <optional>

#include "application/ports/calibration/result/ICalibrationResultSaver.h"
#include "application/ports/desktop/IFileExplorerLauncher.h"
#include "application/usecases/calibration/CalibrationResultSaveControl.h"
#include "domain/ports/calibration/result/ICalibrationResultObserver.h"
#include "domain/ports/calibration/result/ICalibrationResultSource.h"
#include "viewmodels/Observable.h"

namespace mvvm {

struct CalibrationResultSaveViewModelDeps {
    application::usecase::CalibrationResultSaveControl& control;
    domain::ports::ICalibrationResultSource& result_source;
};

class CalibrationResultSaveViewModel final : public domain::ports::ICalibrationResultObserver {
public:
    enum class SaveState {
        NotSaved,
        Saving,
        Saved,
        Error,
    };

    explicit CalibrationResultSaveViewModel(CalibrationResultSaveViewModelDeps deps);
    ~CalibrationResultSaveViewModel() override;

    void save();
    void saveAs(const std::filesystem::path& output_path);
    void showInExplorer();

    void onCalibrationResultUpdated(const domain::common::CalibrationResult& result) override;

    Observable<int> party_id{0};
    Observable<std::string> status_text{"Не сохранено"};
    Observable<SaveState> save_state{SaveState::NotSaved};
    Observable<std::string> error_text{std::string()};
    Observable<bool> can_save{false};
    Observable<bool> can_show_in_explorer{false};
    Observable<bool> can_save_as{false};

private:
    void refreshPreview();
    void setState(SaveState state, std::string error_text = {});
    void handleSaveResult(const application::ports::ICalibrationResultSaver::Result& result);

    application::usecase::CalibrationResultSaveControl& control_;
    domain::ports::ICalibrationResultSource& result_source_;
};

}

#endif //CLEANGRADUATOR_CALIBRATIONRESULTSAVEVIEWMODEL_H
