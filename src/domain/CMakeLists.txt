add_library(domain STATIC fmt/fmt_domain.cpp
        ports/outbound/IPressureSourceObserver.h
        core/common/PressurePacket.h
        core/common/PressureSourceError.h)


target_include_directories(domain
        PUBLIC
        ${PROJECT_SOURCE_DIR}/src
)