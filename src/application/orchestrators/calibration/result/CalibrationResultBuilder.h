#ifndef CLEANGRADUATOR_CALIBRATIONRESULTBUILDER_H
#define CLEANGRADUATOR_CALIBRATIONRESULTBUILDER_H
#include "CalibrationResultBuilderPorts.h"
#include "domain/fmt/Logger.h"
#include "domain/ports/calibration/recording/ICalibrationRecorderObserver.h"
#include "domain/ports/calibration/result/ICalibrationResultSource.h"
#include "../../../../shared/list/ThreadSafeObserverList.h"

namespace application::orchestrators {
    class CalibrationResultBuilder final :
        public domain::ports::ICalibrationRecorderObserver,
        public domain::ports::ICalibrationResultSource
    {
    public:
        explicit CalibrationResultBuilder(CalibrationResultBuilderPorts ports);
        ~CalibrationResultBuilder();

        // ICalibrationRecorderObserver
        void onCalibrationRecorderEvent(const domain::common::CalibrationRecorderEvent &ev) override;

        // ICalibrationResultSource
        const std::optional<domain::common::CalibrationResult>& currentResult() const override;
        void addObserver(domain::ports::ICalibrationResultObserver &) override;
        void removeObserver(domain::ports::ICalibrationResultObserver &) override;

    private:
        void handleEvent(const domain::common::CalibrationRecorderEvent::RecordingStarted& e);
        void handleEvent(const domain::common::CalibrationRecorderEvent::SessionEnded& e);
        void handleEvent(const domain::common::CalibrationRecorderEvent::RecordingStopped& e);

    private:
        fmt::Logger logger_;
        CalibrationResultBuilderPorts ports_;
        std::optional<domain::common::CalibrationResult> active_result_;
        std::optional<domain::common::CalibrationLayout> active_layout_;
        ThreadSafeObserverList<domain::ports::ICalibrationResultObserver> observers_;
    };
}

#endif //CLEANGRADUATOR_CALIBRATIONRESULTBUILDER_H