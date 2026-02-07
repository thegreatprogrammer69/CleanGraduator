add_library(ui STATIC
        widgets/video/VideoOpenGLWidget.cpp
        widgets/video/VideoOpenGLWidget.h
        presenters/VideoStreamPresenter.cpp
        presenters/VideoStreamPresenter.h
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