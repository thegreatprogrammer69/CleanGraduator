#ifndef CLEANGRADUATOR_ITELEMETRYSOURCE_H
#define CLEANGRADUATOR_ITELEMETRYSOURCE_H
#include <vector>
#include "TelemetryValue.h"
#include "TelemetryField.h"

namespace domain::ports {

    struct ITelemetrySource {
        virtual ~ITelemetrySource() = default;

        // Статическая схема
        virtual std::vector<TelemetryField> telemetrySchema() const = 0;

        // Динамические значения
        virtual std::vector<TelemetryValue> telemetryValues() const = 0;
    };

}

#endif //CLEANGRADUATOR_ITELEMETRYSOURCE_H