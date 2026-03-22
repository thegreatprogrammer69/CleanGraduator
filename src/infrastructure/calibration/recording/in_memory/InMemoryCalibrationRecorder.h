#ifndef CLEANGRADUATOR_INMEMORYCALIBRATIONRECORDER_H
#define CLEANGRADUATOR_INMEMORYCALIBRATIONRECORDER_H

#include <optional>
#include <unordered_map>
#include <vector>

#include "../CalibrationRecorderPorts.h"
#include "domain/core/calibration/common/CalibrationLayout.h"
#include "domain/fmt/Logger.h"
#include "domain/ports/calibration/recording/ICalibrationRecorder.h"
#include "domain/core/calibration/recording/CalibrationRecorderEvent.h"
#include "shared/ThreadSafeObserverList.h"

namespace infra::calib {

    class InMemoryCalibrationRecorder final : public domain::ports::ICalibrationRecorder {
    public:
        explicit InMemoryCalibrationRecorder(CalibrationRecorderPorts ports);

        void startRecording(domain::common::CalibrationRecordingContext ctx) override;
        void stopRecording() override;

        void beginSession(domain::common::CalibrationSessionId id) override;
        void record(const domain::common::PressureSample& sample) override;
        void record(const domain::common::AngleSample& sample) override;
        void endSession() override;

        std::vector<domain::common::CalibrationSessionId> sessions() const override;

        std::optional<domain::common::CalibrationSession> session(
            domain::common::CalibrationSessionId id
        ) const override;

        domain::ports::CalibrationAngleCounts angleCounts() const override;

        void addObserver(domain::ports::ICalibrationRecorderObserver& observer) override;
        void removeObserver(domain::ports::ICalibrationRecorderObserver& observer) override;

    private:

        void notify(const domain::common::CalibrationRecorderEvent& ev);

        fmt::Logger logger_;

        bool recording_active_ = false;

        std::optional<domain::common::CalibrationSession> active_session_;
        std::optional<domain::common::MotorDirection> last_direction_;
        domain::ports::CalibrationAngleCounts angle_counts_;

        std::unordered_map<
            domain::common::CalibrationSessionId,
            domain::common::CalibrationSession
        > sessions_;

        ThreadSafeObserverList<domain::ports::ICalibrationRecorderObserver> observers_;
    };

}

#endif
