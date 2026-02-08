#ifndef CLEANGRADUATOR_CALIBRATIONINPUT_H
#define CLEANGRADUATOR_CALIBRATIONINPUT_H
#include "PressurePoints.h"
#include "TimeSeries.h"

namespace domain::common {

    struct AngleSeries : TimeSeries {};
    struct PressureSeries : TimeSeries {};

    struct CalibrationInput {
        CalibrationPoints points;
        AngleSeries angles;
        PressureSeries pressures;
    };

}



#endif //CLEANGRADUATOR_CALIBRATIONINPUT_H