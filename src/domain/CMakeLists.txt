add_library(domain STATIC
        fmt/fmt_domain.cpp
        core/calibration/recording/TimeSeries.cpp
        core/calibration/tracking/PressurePointsTracker.cpp
        core/drivers/motor/MotorDriverEvent.h
        core/drivers/motor/MotorDriverError.h
        core/drivers/motor/MotorFlapsState.h
        "core/drivers/motor/MotorDriverState.h"
        core/drivers/motor/MotorFrequency.h
        core/angle/AngleSourceEvent.h
        ports/angle/IAngleSink.h
        core/video/VideoSourceEvent.h
        ports/video/IVideoSink.h
        ports/pressure/IPressureSink.h
)


target_include_directories(domain
        PUBLIC
        ${PROJECT_SOURCE_DIR}/src
)