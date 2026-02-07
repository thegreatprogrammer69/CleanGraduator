set(SOURCES
        hardware/dm5002/DM5002RFPressureSensor.cpp
        hardware/g540/G540LPT.cpp
        logging/ConsoleLogger.cpp
        logging/FileLogger.cpp
        clock/SessionClock.cpp
)

if(WIN32)
    message(STATUS "Configuring for Windows")
    add_definitions(-DPLATFORM_WINDOWS)
    set(PLATFORM_SOURCES
            platform/com/ComPort_windows.cpp
            platform/lpt/LptPort_windows.cpp
            platform/sleep/sleep_windows.cpp
            camera/windows/DShowVideoStream.cpp
            camera/windows/SampleGrabberCB.cpp
    )
elseif(UNIX)
    message(STATUS "Configuring for Linux")
    add_definitions(-DPLATFORM_LINUX)
    set(PLATFORM_SOURCES
            platform/com/ComPort_linux.cpp
            platform/lpt/LptPort_linux.cpp
            platform/sleep/sleep_linux.cpp
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