add_library(ui STATIC
        widgets/video/VideoOpenGLWidget.cpp
        widgets/video/VideoOpenGLWidget.h
        ../application/presenters/VideoFramePresenter.cpp
        ../application/presenters/VideoFramePresenter.h
        widgets/layout/StackPanelWidget.cpp
        widgets/layout/StackPanelWidget.h
        adapters/QtVideoSourceAdapter.cpp
        adapters/QtVideoSourceAdapter.h
        views/QtVideoSourceView.cpp
        views/QtVideoSourceView.h
        widgets/video/QtVideoOpenGLWidget.cpp
        widgets/video/QtVideoOpenGLWidget.h
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
)