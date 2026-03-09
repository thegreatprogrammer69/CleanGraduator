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

        widgets/calibration/QtCalibrationSeriesWidget.cpp

        widgets/video/QtVideoSourceGridWidget.cpp
        widgets/video/QtVideoSourceGridWidget.h
        widgets/video/QtGLVideoSourceWidget.cpp
        widgets/video/QtGLVideoSourceWidget.h

        widgets/settings/QtCameraGridSettingsWidget.cpp
        widgets/settings/QtCameraGridSettingsWidget.h
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
