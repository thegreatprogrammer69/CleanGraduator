add_library(domain STATIC fmt/fmt_domain.cpp
        ports/pressure/IPressureSourceObserver.h
        core/common/PressurePacket.h
        core/common/PressureSourceError.h
        core/motor/valve/ValvePosition.h
        ports/motor/IDualValveDriver.h
        core/motor/valve/DualValve.h
        core/motor/valve/DualValve.cpp
        "ports/motor/IMotorDriver.h"
        core/motor/motor/MotorDirection.h
        "core/motor/motor/MotorLimitsState.h"
        core/motor/motor/MotorState.h
        core/motor/motor/Motor.h
        core/motor/motor/Motor.cpp
        core/motor/motor/MotorFault.h
        ports/motor/IMotorDriverObserver.h)


target_include_directories(domain
        PUBLIC
        ${PROJECT_SOURCE_DIR}/src
)