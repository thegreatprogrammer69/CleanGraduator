#ifndef CLEANGRADUATOR_CALIBRATIONSERIESVIEWMODEL_H
#define CLEANGRADUATOR_CALIBRATIONSERIESVIEWMODEL_H

#include <vector>

#include "application/orchestrators/settings/CalibrationSettingsQuery.h"
#include "application/orchestrators/video/VideoSourceManager.h"
#include "domain/core/calibration/recording/CalibrationRecorderEvent.h"
#include "domain/ports/calibration/recording/ICalibrationRecorder.h"
#include "domain/ports/calibration/recording/ICalibrationRecorderObserver.h"
#include "viewmodels/Observable.h"

namespace application::orchestrators {
    class VideoSourceManager;
}

namespace mvvm {
    struct CalibrationSeriesViewModelDeps
    {
        application::orchestrators::VideoSourceManager& video_source_manager;
        domain::ports::ICalibrationRecorder& recorder;
    };

    class CalibrationSeriesViewModel final
        : public domain::ports::ICalibrationRecorderObserver
    {
    public:
        struct PressureEntry {
            float time;
            float pressure;
            bool in_session = false;

            bool operator==(const PressureEntry& other) const {
                return time == other.time
                    && pressure == other.pressure
                    && in_session == other.in_session;
            }
        };

        struct AngleEntry {
            float time;
            float angle;
            bool in_session = false;

            bool operator==(const AngleEntry& other) const {
                return time == other.time
                    && angle == other.angle
                    && in_session == other.in_session;
            }
        };

    public:

        explicit CalibrationSeriesViewModel(CalibrationSeriesViewModelDeps deps);
        ~CalibrationSeriesViewModel() override;

        // realtime stream
        Observable<PressureEntry> current_pressure{};
        Observable<std::pair<domain::common::SourceId, AngleEntry>> current_angle{};
        Observable<std::vector<domain::common::SourceId>> source_ids{};

        // history access
        const std::vector<PressureEntry>& pressureHistory() const;
        const std::vector<AngleEntry>& angleHistory(domain::common::SourceId source_id) const;
        const std::vector<domain::common::SourceId> &openedSources() const;

    protected:

        void onCalibrationRecorderEvent(const domain::common::CalibrationRecorderEvent& ev) override;

    private:

        CalibrationSeriesViewModelDeps deps_;

        std::vector<PressureEntry> pressure_history_;
        std::unordered_map<domain::common::SourceId, std::vector<AngleEntry>> angle_history_;

        bool in_session_ = false;
    };

}

#endif