add_library(application STATIC
        fmt/fmt_application.cpp

        orchestrators/AngleFromVideoInteractor.cpp
        orchestrators/VideoSourceManager.cpp

        usecases/settings/ConfigureComponent.cpp

        services/calibration/CalibrationPolicyService.cpp

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