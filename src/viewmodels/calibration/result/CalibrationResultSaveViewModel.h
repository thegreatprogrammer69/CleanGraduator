#ifndef CLEANGRADUATOR_CALIBRATIONRESULTSAVEVIEWMODEL_H
#define CLEANGRADUATOR_CALIBRATIONRESULTSAVEVIEWMODEL_H

#include <filesystem>
#include <string>
#include <vector>

#include "application/usecases/calibration/SaveCalibrationResult.h"
#include "domain/ports/calibration/result/ICalibrationResultObserver.h"
#include "viewmodels/Observable.h"

namespace domain::ports {
class ICalibrationResultSource;
}

namespace mvvm {

enum class CalibrationResultSaveState {
    NotSaved,
    Saving,
    Saved,
    Error,
};

struct CalibrationResultSaveViewModelDeps {
    application::usecase::SaveCalibrationResult& save_use_case;
    domain::ports::ICalibrationResultSource& result_source;
};

class CalibrationResultSaveViewModel final : public domain::ports::ICalibrationResultObserver {
public:
    explicit CalibrationResultSaveViewModel(CalibrationResultSaveViewModelDeps deps);
    ~CalibrationResultSaveViewModel() override;

    void onCalibrationResultUpdated(const domain::common::CalibrationResult& result) override;

    application::usecase::SaveCalibrationResult::Result save();
    application::usecase::SaveCalibrationResult::Result save(const std::vector<int>& camera_ids);
    application::usecase::SaveCalibrationResult::Result saveAs(const std::filesystem::path& directory);
    application::usecase::SaveCalibrationResult::Result saveAs(const std::filesystem::path& directory,
                                                               const std::vector<int>& camera_ids);
    bool canRevealInExplorer() const;
    std::vector<int> availableCameraIds() const;

    Observable<int> batch_number{0};
    Observable<std::string> batch_text{std::string("Партия № —")};
    Observable<CalibrationResultSaveState> save_state{CalibrationResultSaveState::NotSaved};
    Observable<std::string> save_state_text{std::string("Не сохранено")};
    Observable<std::string> error_text{std::string()};
    Observable<bool> can_save{false};
    Observable<bool> can_save_as{false};
    Observable<bool> can_show_in_explorer{false};
    Observable<std::filesystem::path> last_saved_path{};
    Observable<std::vector<int>> available_camera_ids{};

private:
    std::vector<domain::common::SourceId> mapToSourceIds(const std::vector<int>& camera_ids) const;
    void applySaveResult(const application::usecase::SaveCalibrationResult::Result& result);
    void updateBatchInfo();

    application::usecase::SaveCalibrationResult& save_use_case_;
    domain::ports::ICalibrationResultSource& result_source_;
};

}

#endif //CLEANGRADUATOR_CALIBRATIONRESULTSAVEVIEWMODEL_H
