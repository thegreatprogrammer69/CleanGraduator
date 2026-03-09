#ifndef CLEANGRADUATOR_CALIBRATIONRESULTBUILDER_H
#define CLEANGRADUATOR_CALIBRATIONRESULTBUILDER_H
#include "CalibrationResultBuilderPorts.h"
#include "domain/ports/calibration/recording/ICalibrationRecorderObserver.h"
#include "domain/ports/calibration/result/ICalibrationResultSource.h"
#include "shared/ThreadSafeObserverList.h"

namespace application::orchestrators {
    class CalibrationResultBuilder final :
        public domain::ports::ICalibrationRecorderObserver,
        public domain::ports::ICalibrationResultSource
    {
    public:
        // TODO Я сегодня остановился здесь
        explicit CalibrationResultBuilder(CalibrationResultBuilderPorts ports);

        // ICalibrationRecorderObserver
        void onCalibrationRecorderEvent(const domain::common::CalibrationRecorderEvent &ev) override;

        // ICalibrationResultSource
        const domain::common::CalibrationResult & currentResult() const override;
        void addObserver(domain::ports::ICalibrationResultObserver &) override;
        void removeObserver(domain::ports::ICalibrationResultObserver &) override;

    private:
        domain::common::CalibrationResult calibration_result_;
        ThreadSafeObserverList<domain::ports::ICalibrationResultObserver> observers_;
    };
}

#endif //CLEANGRADUATOR_CALIBRATIONRESULTBUILDER_H