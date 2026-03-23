add_library(application STATIC
        fmt/fmt_application.cpp

        orchestrators/calibration/process/CalibrationOrchestrator.cpp
        orchestrators/calibration/process/CalibrationSafetyMonitor.cpp
        orchestrators/calibration/result/CalibrationResultBuilder.cpp
        orchestrators/calibration/result/CalibrationResultValidationSource.cpp
        orchestrators/video/VideoSourceManager.cpp
        orchestrators/settings/CalibrationContextProvider.cpp
        orchestrators/motor/MotorControlInteractor.cpp

        usecases/cameras/OpenSelectedCameras.cpp
        usecases/cameras/CloseAllCameras.cpp
        usecases/cameras/OpenAllCameras.cpp
        usecases/calibration/CalibrationSessionControl.cpp
        usecases/calibration/SaveCalibrationResult.cpp
        ports/calibration/result/ICalibrationResultSaver.h

)

target_include_directories(application
        PUBLIC
        ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(application
        PUBLIC
        domain
)
