add_library(domain STATIC fmt/fmt_domain.cpp
        core/motor/motor/FrequencyLimits.h
        ports/calibration_actuator/ICalibrationActuator.h)


target_include_directories(domain
        PUBLIC
        ${PROJECT_SOURCE_DIR}/src
)