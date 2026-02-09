#ifndef CLEANGRADUATOR_SETTINGSVIEWMODEL_H
#define CLEANGRADUATOR_SETTINGSVIEWMODEL_H
#include "Observable.h"
#include "application/dto/CameraString.h"
#include "application/dto/PrinterRecord.h"
#include "application/dto/DisplacementRecord.h"
#include "application/dto/FrameCrosshair.h"
#include "application/dto/GaugeRecord.h"
#include "application/dto/PressureUnitRecord.h"
#include "application/dto/PrecisionRecord.h"
#include "application/dto/UserSettings.h"

namespace application::usecases {
    class SettingsUseCase;
}

namespace mvvm {

    class SettingsViewModel {
    public:
        explicit SettingsViewModel(application::usecases::SettingsUseCase& use_case);
        ~SettingsViewModel();

        using PrinterRecords = std::vector<application::dto::PrinterRecord>;
        using DisplacementRecords = std::vector<application::dto::DisplacementRecord>;
        using GaugeRecords = std::vector<application::dto::GaugeRecord>;
        using PressureUnitRecords = std::vector<application::dto::PressureUnitRecord>;
        using PrecisionRecords = std::vector<application::dto::PrecisionRecord>;

        Observable<PrinterRecords> printers;
        Observable<DisplacementRecords> displacements;
        Observable<GaugeRecords> gauges;
        Observable<PressureUnitRecords> pressure_units;
        Observable<PrecisionRecords> precisions;

        Observable<application::dto::UserSettings> state;


    private:
        application::usecases::SettingsUseCase& use_case_;
    };
}

#endif //CLEANGRADUATOR_SETTINGSVIEWMODEL_H