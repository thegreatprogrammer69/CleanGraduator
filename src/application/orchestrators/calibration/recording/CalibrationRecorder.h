#ifndef CLEANGRADUATOR_CALIBRATIONRECORDER_H
#define CLEANGRADUATOR_CALIBRATIONRECORDER_H
#include <atomic>
#include <functional>

#include "application/ports/calibration/recording/ICalibrationRecorder.h"
#include "domain/fmt/Logger.h"
#include "domain/ports/angle/IAngleSink.h"
#include "domain/ports/pressure/IPressureSourceObserver.h"
#include "domain/core/calibration/recording/CalibrationDataset.h"

namespace domain::ports {
    struct IAngleSource;
    struct IPressureSource;
}

namespace application::ports {
    struct IVideoAngleSourcesStorage;
}

namespace application::orchestrators {
    struct CalibrationRecorderPorts {
        domain::ports::ILogger& logger;
        ports::IVideoAngleSourcesStorage& angle_sources_storage;
        domain::ports::IPressureSource& pressure_source;
    };

    class CalibrationRecorder final
        : public ports::ICalibrationRecorder
        , public domain::ports::IAngleSink
        , public domain::ports::IPressureSourceObserver
    {
    public:
        explicit CalibrationRecorder(CalibrationRecorderPorts ports);
        ~CalibrationRecorder() override;

        bool start(ports::CalibrationRecorderInput input) override;
        void stop() override;

        void resetSession() override;
        bool isRecording() const override;

    protected:

        void onAngle(const domain::common::AnglePacket &) noexcept override;
        void onPressurePacket(const domain::common::PressurePacket &) override;

        void onPressureSourceOpened() override {}
        void onPressureSourceOpenFailed(const domain::common::PressureSourceError &) override {}
        void onPressureSourceClosed(const domain::common::PressureSourceError &) override {}

    private:
        void subscribe();
        void unsubscribe();

    private:
        // State
        std::atomic_bool is_recording_{false};

        std::vector<std::function<void()>> unsubscribers_;

        // Data
        domain::common::CalibrationDataset dataset_;

        // Ports
        fmt::Logger logger_;
        ports::IVideoAngleSourcesStorage& angle_sources_storage_;
        domain::ports::IPressureSource& pressure_source_;

        // Input
        std::vector<domain::common::AngleSourceId> active_sources_ids_;
        domain::common::PressureUnit pressure_unit_;
        domain::common::AngleUnit angle_unit_;

    };
}


#endif //CLEANGRADUATOR_CALIBRATIONRECORDER_H