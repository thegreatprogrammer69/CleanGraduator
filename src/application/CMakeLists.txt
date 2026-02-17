add_library(application STATIC
        fmt/fmt_application.cpp

        orchestrators/angle/AngleFromVideoInteractor.cpp
        orchestrators/calibration/recording/CalibrationRecorder.cpp
        orchestrators/video/VideoSourceManager.cpp
        orchestrators/settings/CalibrationSettingsQuery.cpp

        usecases/settings/ConfigureComponent.cpp

        usecases/cameras/OpenSelectedCameras.cpp
        usecases/cameras/CloseAllCameras.cpp
        usecases/cameras/OpenAllCameras.cpp

)

target_include_directories(application
        PUBLIC
        ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(application
        PUBLIC
        domain
)