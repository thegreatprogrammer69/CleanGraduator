add_library(ui STATIC
        resources.qrc

        widgets/control/QtDualValveControlWidget.cpp
        widgets/control/QtDualValveControlWidget.h
        widgets/control/QtMotorControlWidget.cpp
        widgets/control/QtMotorControlWidget.h
        widgets/control/QtCalibrationSessionControlWidget.cpp
        widgets/control/QtCalibrationSessionControlWidget.h
        widgets/control/QtControlWidget.cpp
        widgets/control/QtControlWidget.h

        widgets/calibration/recording/QtCalibrationSeriesWidget.cpp
        widgets/calibration/recording/QtCalibrationSeriesWidget.h
        widgets/calibration/result/QtCalibrationResultTableModel.cpp
        widgets/calibration/result/QtCalibrationResultTableModel.h
        widgets/calibration/result/QtCalibrationResultTableWidget.cpp
        widgets/calibration/result/QtCalibrationResultTableWidget.h
        widgets/calibration/result/QtCalibrationResultSaveWidget.cpp
        widgets/calibration/result/QtCalibrationResultSaveWidget.h


        widgets/video/QtVideoSourceGridWidget.cpp
        widgets/video/QtVideoSourceGridWidget.h
        widgets/video/QtGLVideoSourceWidget.cpp
        widgets/video/QtGLVideoSourceWidget.h

        widgets/settings/QtCameraGridSettingsWidget.cpp
        widgets/settings/QtCameraGridSettingsWidget.h
        widgets/settings/QtCircleOverlaySettingsWidget.cpp
        widgets/settings/QtCircleOverlaySettingsWidget.h
        widgets/settings/QtInfoSettingsWidget.cpp
        widgets/settings/QtInfoSettingsWidget.h
        widgets/settings/QtSettingsWidget.cpp
        widgets/settings/QtSettingsWidget.h

        widgets/status_bar/QtAppStatusBarWidget.cpp
        widgets/status_bar/QtAppStatusBarWidget.h
        widgets/status_bar/QtMotorDriverStatusBarWidget.cpp
        widgets/status_bar/QtMotorDriverStatusBarWidget.h
        widgets/status_bar/QtPressureSensorStatusBarWidget.cpp
        widgets/status_bar/QtPressureSensorStatusBarWidget.h

        widgets/logging/QtLogViewerWidget.cpp
        widgets/logging/QtLogViewerWidget.h

        widgets/QtMainWindow.cpp
        widgets/QtMainWindow.h
        widgets/video/QtD3D11VideoSourceWidget.h
        widgets/video/QtD3D11VideoSourceWidget.cpp

)

if(WIN32)
    target_compile_definitions(ui PRIVATE PLATFORM_WINDOWS)
elseif(UNIX)
    target_compile_definitions(ui PRIVATE PLATFORM_LINUX)
endif()

target_include_directories(ui
        PUBLIC
        ${PROJECT_SOURCE_DIR}/src
)


target_link_libraries(ui PRIVATE
        $<$<PLATFORM_ID:Windows>:d3d11>
        $<$<PLATFORM_ID:Windows>:dxgi>
        $<$<PLATFORM_ID:Windows>:d3dcompiler>
)

target_link_libraries(ui
        PUBLIC
        Qt5::Widgets
        Qt5::OpenGL
        OpenGL::GL
        domain
        application
        viewmodels
)
