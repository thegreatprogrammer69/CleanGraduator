#ifndef CLEANGRADUATOR_UISETTINGSDTO_H
#define CLEANGRADUATOR_UISETTINGSDTO_H
#include <optional>

#include "application/dto/FrameCrosshair.h"
#include "application/dto/CameraString.h"
#include "PrinterRecord.h"
#include "application/dto/DisplacementRecord.h"
#include "GaugeRecord.h"
#include "application/dto/PrecisionRecord.h"
#include "application/dto/PressureUnitRecord.h"

namespace application::dto {

    struct UserSettings {
        // selections
        std::optional<PrinterRecordId> printerId;
        std::optional<DisplacementRecordId> displacementId;
        std::optional<GaugeRecordId> gaugeId;
        std::optional<PrecisionRecordId> precisionId;
        std::optional<PressureUnitRecordId> pressureUnitId;

        // inputs
        CameraString cameraString{};
        FrameCrosshair frameCrosshair{};

        // ui flags
        bool openCamerasOnStartup = false;
    };

} // namespace application::dto

#endif //CLEANGRADUATOR_UISETTINGSDTO_H