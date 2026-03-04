#ifndef CLEANGRADUATOR_INMEMORYCALIBRATIONRECORDER_H
#define CLEANGRADUATOR_INMEMORYCALIBRATIONRECORDER_H

#include <map>
#include <optional>
#include <unordered_map>
#include <vector>

#include "InMemoryCalibrationRecorderConfig.h"
#include "../CalibrationRecorderPorts.h"
#include "domain/fmt/Logger.h"
#include "domain/ports/calibration/recording/ICalibrationRecorder.h"

namespace infra::calib {

    class InMemoryCalibrationRecorder final : public domain::ports::ICalibrationRecorder {
    public:
        InMemoryCalibrationRecorder(
            CalibrationRecorderPorts ports,
            InMemoryCalibrationRecorderConfig config
        );

        void beginSession(domain::common::CalibrationSessionId id) override;
        void record(const domain::common::PressureSample& sample) override;
        void record(const domain::common::AngleSample& sample) override;
        void endSession() override;

        std::vector<domain::common::CalibrationSessionId> sessions() const override;
        std::optional<domain::common::CalibrationSession> session(
            domain::common::CalibrationSessionId id
        ) const override;

    private:
        fmt::Logger logger_;
        InMemoryCalibrationRecorderConfig config_;

        std::optional<domain::common::CalibrationSession> active_session_;
        std::unordered_map<
            domain::common::CalibrationSessionId,
            domain::common::CalibrationSession
        > sessions_;
    };

} // namespace infra::calib

#endif // CLEANGRADUATOR_INMEMORYCALIBRATIONRECORDER_H