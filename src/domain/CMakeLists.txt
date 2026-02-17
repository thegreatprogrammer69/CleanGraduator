add_library(domain STATIC

        fmt/fmt_domain.cpp

        core/calibration/recording/CalibrationDataset.cpp
        core/calibration/recording/TimeSeries.cpp
        core/calibration/calculation/CalibrationMotionPhase.h
        core/calibration/calculation/PressurePoints.h
        ports/calibration/motion_controller/CalibrationMotionInput.h
)


target_include_directories(domain
        PUBLIC
        ${PROJECT_SOURCE_DIR}/src
)