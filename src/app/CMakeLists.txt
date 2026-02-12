if (WIN32)
    add_definitions(-DPLATFORM_WINDOWS)
elseif (UNIX)
    add_definitions(-DPLATFORM_LINUX)
endif ()

    add_executable(CleanGraduator
        main.cpp
        bootstrap/AppBootstrap.cpp
        bootstrap/AppBootstrap.h
        bootstrap/DIContainer.h
)

target_link_libraries(CleanGraduator
        PRIVATE
        domain
        application
        ui
        infrastructure
        viewmodels
)

target_link_libraries(CleanGraduator PRIVATE Qt5::Widgets)
