add_executable(CleanGraduator
        main.cpp
        bootstrap/AppBootstrap.cpp
        bootstrap/AppBootstrap.h
        bootstrap/DIContainer.h
        bootstrap/modules/DomainModule.cpp
        bootstrap/modules/DomainModule.h
        bootstrap/modules/InfrastructureModule.cpp
        bootstrap/modules/InfrastructureModule.h
        bootstrap/modules/ViewModelsModule.cpp
        bootstrap/modules/ViewModelsModule.h
        bootstrap/modules/UiModule.cpp
        bootstrap/modules/UiModule.h
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
