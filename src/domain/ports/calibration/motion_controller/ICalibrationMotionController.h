#ifndef CLEANGRADUATOR_ICALIBRATIONMOTIONCONTROLLER_H
#define CLEANGRADUATOR_ICALIBRATIONMOTIONCONTROLLER_H
#include "CalibrationMotionInput.h"

namespace domain::ports {
    struct ICalibrationMotionObserver;

    struct ICalibrationMotionController {
        virtual ~ICalibrationMotionController() = default;

        // Запуск движения согласно режиму
        virtual void start(CalibrationMotionInput) = 0;

        // Корректная остановка (штатное завершение)
        virtual void stop() = 0;

        // Внештатный стоп
        virtual void abort() = 0;

        virtual bool isRunning() const = 0;

        virtual void addObserver(ICalibrationMotionObserver&) = 0;
        virtual void removeObserver(ICalibrationMotionObserver&) = 0;
    };

}

#endif //CLEANGRADUATOR_ICALIBRATIONMOTIONCONTROLLER_H