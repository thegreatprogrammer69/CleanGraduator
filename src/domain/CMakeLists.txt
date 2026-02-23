add_library(domain STATIC
        fmt/fmt_domain.cpp
        core/calibration/recording/TimeSeries.cpp
        core/calibration/tracking/PressurePointsTracker.cpp
)


target_include_directories(domain
        PUBLIC
        ${PROJECT_SOURCE_DIR}/src
)