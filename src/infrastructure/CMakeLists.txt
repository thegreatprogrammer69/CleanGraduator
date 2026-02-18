set(SOURCES
        catalogs/FileDisplacementCatalog.cpp
        catalogs/FileGaugeCatalog.cpp
        catalogs/FilePrinterCatalog.cpp
        catalogs/FileGaugePrecisionCatalog.cpp
        catalogs/FilePressureUnitCatalog.cpp

        utils/ini/IniFile.cpp

        calculation/calibration/LinearCalibrationCalculator.cpp
        calculation/angle/CastAnglemeter.cpp
        calculation/angle/detail/cast_anglemeter.cpp

        video/VideoSourceNotifier.cpp

        pressure/PressureSourceNotifier.cpp
        pressure/dm5002/DM5002RFPressureSensor.cpp
        pressure/dm5002/DM5002PressureSensor.cpp

        motor/g540/as_lpt/G540LptMotorDriver.cpp

        logging/ConsoleLogger.cpp
        logging/FileLogger.cpp
        logging/NamedMultiLogger.cpp

        clock/SessionClock.cpp
        clock/UptimeClock.cpp

        process/ProcessLifecycle.cpp
        process/ProcessRunner.cpp

        overlay/crosshair/CrosshairVideoOverlay.cpp

        storage/QtInfoSettingsStorage.cpp
        storage/VideoAngleSourcesStorage.cpp
        storage/LogSourcesStorage.cpp

        factory/AngleCalculatorFactory.cpp
        factory/CalibrationCalculatorFactory.cpp
        factory/VideoSourceFactory.cpp
        factory/VideoSourceOverlayFactory.cpp
        factory/PressureSourceFactory.cpp
        factory/MotorDriverFactory.cpp

        motor/g540/as_lpt/G540LptMotorDriver.cpp

        calibration/motion_controller/stand4/FrequencyCalculator.cpp
        calibration/motion_controller/stand4/PressurePointsTracker.cpp
        calibration/motion_controller/stand4/CalibrationMotionController4.cpp
        calibration/motion_controller/stand4/CalibrationMotionController4Telemetry.cpp

)

if (WIN32)
    add_definitions(-DPLATFORM_WINDOWS)
elseif (UNIX)
    add_definitions(-DPLATFORM_LINUX)
endif ()

if(WIN32)
    message(STATUS "Configuring for Windows")
    add_definitions(-DPLATFORM_WINDOWS)
    set(PLATFORM_SOURCES
            platform/com/ComPort_windows.cpp
            platform/lpt/LptPort_windows.cpp
            platform/sleep/sleep_windows.cpp
            video/windows/dshow/DShowCamera.cpp
            video/windows/dshow/SampleGrabberCB.cpp
    )
elseif(UNIX)
    message(STATUS "Configuring for Linux")
    add_definitions(-DPLATFORM_LINUX)
    set(PLATFORM_SOURCES
            platform/com/ComPort_linux.cpp
#            platform/lpt/LptPort_linux.cpp
            platform/lpt/LptPort_console.cpp
            platform/sleep/sleep_linux.cpp
            video/linux/v4l/V4LCamera.cpp
            video/linux/gstreamer/GStreamerCamera.cpp
            calibration/motion_controller/stand4/CalibrationMotionController4Telemetry.cpp
            calibration/motion_controller/algorithms/return_to_home.cpp
            calibration/motion_controller/algorithms/return_to_home.h

    )
endif()


add_library(infrastructure STATIC
        ${SOURCES}
        ${PLATFORM_SOURCES}
)


if(UNIX)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GSTREAMER REQUIRED
            gstreamer-1.0
            gstreamer-base-1.0
            gstreamer-app-1.0
    )
else()
    set(GSTREAMER_INCLUDE_DIRS, "")
    set(GSTREAMER_LIBRARIES, "")
    set(GSTREAMER_CFLAGS_OTHER, "")
endif()


target_include_directories(infrastructure
        PUBLIC
        ${PROJECT_SOURCE_DIR}/src
        ${GSTREAMER_INCLUDE_DIRS}
)

target_link_libraries(infrastructure
        PUBLIC
        domain
        Qt5::Core
        ${GSTREAMER_LIBRARIES}
)

target_compile_options(infrastructure PRIVATE
        ${GSTREAMER_CFLAGS_OTHER}
)