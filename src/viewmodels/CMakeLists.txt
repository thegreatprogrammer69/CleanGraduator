add_library(viewmodels STATIC
        calibration/recording/CalibrationSeriesViewModel.cpp
        calibration/result/CalibrationResultTableViewModel.cpp
        calibration/result/CalibrationResultSaveViewModel.cpp

        control/DualValveControlViewModel.cpp
        control/MotorControlViewModel.cpp
        control/CalibrationSessionControlViewModel.cpp
        control/ControlViewModel.cpp

        video/VideoSourceViewModel.cpp
        video/VideoSourceGridViewModel.cpp

        settings/CameraGridSettingsViewModel.cpp
        settings/CircleOverlaySettingsViewModel.cpp
        settings/InfoSettingsViewModel.cpp
        settings/SettingsViewModel.cpp

        status_bar/AppStatusBarViewModel.cpp
        status_bar/MotorDriverStatusViewModel.cpp
        status_bar/PressureSensorStatusBarViewModel.cpp

        logging/LogViewerViewModel.cpp

        MainWindowViewModel.cpp

)

target_include_directories(viewmodels
        PUBLIC
        ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(viewmodels
        PUBLIC
        domain
        application
)
