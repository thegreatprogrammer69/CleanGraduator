add_library(domain STATIC
        fmt/fmt_domain.cpp
        core/calibration/recording/TimeSeries.cpp
        ../infrastructure/calibration/tracking/PressurePointsTracker.cpp
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
        core/calibration/analysis/CalibrationCellIssue.h
        core/calibration/result/CalibrationCellKey.h
        core/calibration/analysis/CalibrationIssueSeverity.h
        core/calibration/calculation/CalibrationCellComputation.h
        core/calibration/common/CalibrationLayout.h
        core/calibration/result/CalibrationResult.cpp
        core/calibration/result/CalibrationResultIssue.h
        core/calibration/result/CalibrationResultIssueType.h
)


target_include_directories(domain
        PUBLIC
        ${PROJECT_SOURCE_DIR}/src
)