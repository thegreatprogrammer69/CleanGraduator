if (WIN32)
    add_definitions(-DPLATFORM_WINDOWS)
elseif (UNIX)
    add_definitions(-DPLATFORM_LINUX)
endif ()

    add_executable(CleanGraduator
        main.cpp
        test1.cpp test2.cpp test3.cpp
        bootstrap/ApplicationBootstrap.cpp
        bootstrap/UseCasesBootstrap.cpp
        bootstrap/ViewModelsBootstrap.cpp
        bootstrap/UiBootstrap.cpp
            ControlPanelWidget.cpp
            ControlPanelWidget.h

    )

target_link_libraries(CleanGraduator
        PRIVATE
        domain
        application
        infrastructure
        viewmodels
        ui
)

target_link_libraries(CleanGraduator PRIVATE Qt5::Widgets)
