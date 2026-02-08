set(SOURCES
        catalogs/FileDisplacementCatalog.cpp
        catalogs/FileGaugeCatalog.cpp
        catalogs/FilePrinterCatalog.cpp
        hardware/dm5002/DM5002RFPressureSensor.cpp
        hardware/g540/G540LPT.cpp
        fmt/fmt_application.cpp
        fmt/fmt_domain.cpp
        logging/ConsoleLogger.cpp
        logging/FileLogger.cpp
        clock/SessionClock.cpp
        process/ProcessLifecycle.cpp
        process/ProcessRunner.cpp
)

if(WIN32)
    message(STATUS "Configuring for Windows")
    add_definitions(-DPLATFORM_WINDOWS)
    set(PLATFORM_SOURCES
            platform/com/ComPort_windows.cpp
            platform/lpt/LptPort_windows.cpp
            platform/sleep/sleep_windows.cpp
            camera/windows/dshow/DShowVideoStream.cpp
            camera/windows/dshow/SampleGrabberCB.cpp
    )
elseif(UNIX)
    message(STATUS "Configuring for Linux")
    add_definitions(-DPLATFORM_LINUX)
    set(PLATFORM_SOURCES
            platform/com/ComPort_linux.cpp
            platform/lpt/LptPort_linux.cpp
            platform/sleep/sleep_linux.cpp
            camera/linux/v4l/V4LCamera.cpp
    )
endif()

add_library(infrastructure STATIC
        ${SOURCES}
        ${PLATFORM_SOURCES}
)

target_include_directories(infrastructure
        PUBLIC
        ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(infrastructure
        PUBLIC
        domain
)