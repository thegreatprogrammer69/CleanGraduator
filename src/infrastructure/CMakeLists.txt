set(SOURCES


#        catalogs/FileDisplacementCatalog.cpp
#        catalogs/FileGaugeCatalog.cpp
#        catalogs/FilePrinterCatalog.cpp

        utils/ini/IniFile.cpp

        calculation/angle/CastAnglemeter.cpp
        calculation/angle/detail/cast_anglemeter.cpp

        hardware/dm5002/DM5002RFPressureSensor.cpp
        hardware/g540/G540LPT.cpp

        logging/ConsoleLogger.cpp
        logging/FileLogger.cpp

        clock/SessionClock.cpp

        process/ProcessLifecycle.cpp
        process/ProcessRunner.cpp

        settings/QtCameraGridSettingsRepository.cpp
)

if(WIN32)
    message(STATUS "Configuring for Windows")
    add_definitions(-DPLATFORM_WINDOWS)
    set(PLATFORM_SOURCES
            platform/com/ComPort_windows.cpp
            platform/lpt/LptPort_windows.cpp
            platform/sleep/sleep_windows.cpp
            camera/windows/dshow/DShowCamera.cpp
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
            camera/linux/gstreamer/GStreamerCamera.cpp
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