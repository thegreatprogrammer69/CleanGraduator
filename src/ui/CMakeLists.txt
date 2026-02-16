add_library(ui STATIC
        resources.qrc

        widgets/layout/QtStackPanelWidget.cpp
        widgets/layout/QtStackPanelWidget.h

        widgets/video/QtVideoSourceGridWidget.cpp
        widgets/video/QtVideoSourceGridWidget.h
        widgets/video/QtVideoSourceWidget.cpp
        widgets/video/QtVideoSourceWidget.h

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

        widgets/logging/QtLogViewerWidget.cpp
        widgets/logging/QtLogViewerWidget.h

        widgets/QtMainWindow.cpp
        widgets/QtMainWindow.h

)

target_include_directories(ui
        PUBLIC
        ${PROJECT_SOURCE_DIR}/src
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