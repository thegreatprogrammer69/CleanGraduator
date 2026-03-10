#ifndef CLEANGRADUATOR_CALIBRATIONRESULTTABLEVIEWMODEL_H
#define CLEANGRADUATOR_CALIBRATIONRESULTTABLEVIEWMODEL_H

namespace domain::ports {
    class ICalibrationResultSource;
}

namespace mvvm {
    struct CalibrationResultTableViewModelDeps {
        domain::ports::ICalibrationResultSource& result_source;
    };

    class CalibrationResultTableViewModel {
    public:

    };
}

#endif //CLEANGRADUATOR_CALIBRATIONRESULTTABLEVIEWMODEL_H