#ifndef CLEANGRADUATOR_CALIBRATIONRESULTVALIDATIONSOURCE_H
#define CLEANGRADUATOR_CALIBRATIONRESULTVALIDATIONSOURCE_H

#include <optional>

#include "application/ports/catalogs/IGaugePrecisionCatalog.h"
#include "application/ports/settings/IInfoSettingsStorage.h"
#include "domain/fmt/Logger.h"
#include "domain/ports/calibration/result/ICalibrationResultObserver.h"
#include "domain/ports/calibration/result/ICalibrationResultSource.h"
#include "domain/ports/calibration/result/ICalibrationResultValidationSource.h"
#include "shared/ThreadSafeObserverList.h"

namespace application::orchestrators {
    struct CalibrationResultValidationSourceDeps {
        domain::ports::ILogger& logger;
        domain::ports::ICalibrationResultSource& result_source;
        application::ports::IInfoSettingsStorage& settings_storage;
        application::ports::IGaugePrecisionCatalog& precision_catalog;
    };

    class CalibrationResultValidationSource final
        : public domain::ports::ICalibrationResultObserver
        , public domain::ports::ICalibrationResultValidationSource {
    public:
        explicit CalibrationResultValidationSource(CalibrationResultValidationSourceDeps deps);
        ~CalibrationResultValidationSource() override;

        void onCalibrationResultUpdated(const domain::common::CalibrationResult& result) override;

        const std::optional<domain::common::CalibrationResultValidation>& currentValidation() const override;
        void addObserver(domain::ports::ICalibrationResultValidationObserver& observer) override;
        void removeObserver(domain::ports::ICalibrationResultValidationObserver& observer) override;
        void requestRefresh() override;

    private:
        void rebuild();
        void notifyObservers() const;
        float resolvePrecisionPercent() const;
        bool isKuEnabled() const;

    private:
        fmt::Logger logger_;
        CalibrationResultValidationSourceDeps deps_;
        std::optional<domain::common::CalibrationResult> current_result_;
        std::optional<domain::common::CalibrationResultValidation> current_validation_;
        ThreadSafeObserverList<domain::ports::ICalibrationResultValidationObserver> observers_;
    };
}

#endif //CLEANGRADUATOR_CALIBRATIONRESULTVALIDATIONSOURCE_H
