#ifndef CLEANGRADUATOR_CALIBRATIONRESULTSAVEVIEWMODEL_H
#define CLEANGRADUATOR_CALIBRATIONRESULTSAVEVIEWMODEL_H

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>

#include "application/models/info/BatchContext.h"
#include "domain/ports/calibration/result/ICalibrationResultObserver.h"
#include "viewmodels/Observable.h"

namespace application::usecase {
    class SaveCalibrationResult;
}

namespace domain::ports {
    class ICalibrationResultSource;
}

namespace mvvm {
    struct CalibrationResultSaveViewModelDeps {
        application::usecase::SaveCalibrationResult& save_result;
        domain::ports::ICalibrationResultSource& result_source;
    };

    class CalibrationResultSaveViewModel final : public domain::ports::ICalibrationResultObserver {
    public:
        enum class SaveState {
            NotSaved,
            Saving,
            Saved,
            Error
        };

        struct ViewState {
            int party_id{0};
            SaveState status{SaveState::NotSaved};
            std::string status_text;
            std::string status_color;
            std::filesystem::path path;
            std::string last_error;
            bool can_save{false};
            bool can_show_in_explorer{false};
            bool can_save_as{false};
        };

        explicit CalibrationResultSaveViewModel(CalibrationResultSaveViewModelDeps deps);
        ~CalibrationResultSaveViewModel() override;

        void save();
        void saveAs(const std::filesystem::path& directory);
        void showInExplorer();

        void onCalibrationResultUpdated(const domain::common::CalibrationResult& result) override;

        Observable<ViewState> state;

    private:
        void refreshState();
        ViewState makeBaseState() const;
        static ViewState savedState(int party_id,
                                    const std::filesystem::path& path,
                                    const std::string& error = {});

    private:
        CalibrationResultSaveViewModelDeps deps_;
        std::uint64_t current_revision_{0};
        std::optional<std::uint64_t> saved_revision_;
        std::optional<application::models::BatchContext> last_saved_batch_;
        std::filesystem::path last_saved_path_;
    };
}

#endif //CLEANGRADUATOR_CALIBRATIONRESULTSAVEVIEWMODEL_H
