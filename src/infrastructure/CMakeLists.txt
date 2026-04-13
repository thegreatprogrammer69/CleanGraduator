set(SOURCES
        angle/from_video/AngleSourceFromVideo.cpp
        angle/fake/FakeAngleSourceFromVideo.cpp

        catalogs/FileDisplacementCatalog.cpp
        catalogs/FileGaugeCatalog.cpp
        catalogs/FilePrinterCatalog.cpp
        catalogs/FileGaugePrecisionCatalog.cpp
        catalogs/FilePressureUnitCatalog.cpp


        calculation/calibration/LinearCalibrationCalculator.cpp
        calculation/angle/CastAnglemeter.cpp
        calculation/angle/detail/cast_anglemeter.cpp

        video/VideoSourceNotifier.cpp

        pressure/PressureSourceNotifier.cpp
        pressure/dm5002/DM5002PressureSensor.cpp
        pressure/fake/FakePressureSource.cpp

        logging/ConsoleLogger.cpp
        logging/FileLogger.cpp
        logging/FileLoggingControl.cpp
        logging/NamedMultiLogger.cpp

        clock/SessionClock.cpp
        clock/UptimeClock.cpp

        overlay/crosshair/CrosshairVideoOverlay.cpp

        storage/QtInfoSettingsStorage.cpp
        storage/VideoAngleSourcesStorage.cpp
        storage/LogSourcesStorage.cpp

        factory/AngleSourceFactory.cpp
        factory/AngleCalculatorFactory.cpp
        factory/CalibrationCalculatorFactory.cpp
        factory/VideoSourceFactory.cpp
        factory/VideoSourceOverlayFactory.cpp
        factory/PressureSourceFactory.cpp
        factory/MotorDriverFactory.cpp

        calibration/batch/BatchContextProvider.cpp
        calibration/strats/stand4/Stand4FrequencyCalculator.cpp
        calibration/strats/stand4/Stand4CalibrationStrategy.cpp
        calibration/recording/in_memory/InMemoryCalibrationRecorder.cpp
        calibration/result/in_file/FileCalibrationResultSaver.cpp

        motor/g540/as_lpt/G540LptMotorDriver.cpp
        motor/notifier/MotorDriverNotifier.cpp

        ../shared/thread/ThreadWorker.cpp
        ../shared/ini/IniFile.cpp
        ../shared/watchdog/SoftwareWatchdog.cpp

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
#            platform/lpt/LptPort_console.cpp
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
            calibration/batch/BatchContextProviderPorts.h
            calibration/batch/BatchContextProvider.cpp
            calibration/batch/BatchContextProvider.h
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
