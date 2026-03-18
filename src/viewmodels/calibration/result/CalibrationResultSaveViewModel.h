#ifndef CLEANGRADUATOR_CALIBRATIONRESULTSAVEVIEWMODEL_H
#define CLEANGRADUATOR_CALIBRATIONRESULTSAVEVIEWMODEL_H

#include <filesystem>
#include <optional>
#include <string>

#include "application/usecases/calibration/result/SaveCalibrationResult.h"
#include "domain/ports/calibration/result/ICalibrationResultObserver.h"
#include "viewmodels/Observable.h"

namespace domain::ports { class ICalibrationResultSource; }

namespace mvvm {

struct CalibrationResultSaveViewModelDeps {
    application::usecase::SaveCalibrationResult& save_use_case;
    domain::ports::ICalibrationResultSource& result_source;
};

class CalibrationResultSaveViewModel final : public domain::ports::ICalibrationResultObserver {
public:
    enum class Status {
        Unsaved,
        Saving,
        Saved,
        Error,
    };

    explicit CalibrationResultSaveViewModel(CalibrationResultSaveViewModelDeps deps);
    ~CalibrationResultSaveViewModel() override;

    void save();
    void saveAs(const std::filesystem::path& directory);
    void revealInExplorer();
    void refresh();

    Observable<std::optional<int>> batch_number;
    Observable<Status> status{Status::Unsaved};
    Observable<std::string> status_text{std::string("Не сохранено")};
    Observable<std::string> error_text{std::string()};
    Observable<bool> can_save{false};
    Observable<bool> can_reveal{false};

    void onCalibrationResultUpdated(const domain::common::CalibrationResult& result) override;

private:
    void publishState();
    static Status mapStatus(application::usecase::SaveCalibrationResult::Status status);
    static std::string statusText(Status status);

private:
    application::usecase::SaveCalibrationResult& save_use_case_;
    domain::ports::ICalibrationResultSource& result_source_;
};

}

#endif //CLEANGRADUATOR_CALIBRATIONRESULTSAVEVIEWMODEL_H
