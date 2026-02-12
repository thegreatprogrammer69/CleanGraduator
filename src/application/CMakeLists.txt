add_library(application STATIC
        fmt/fmt_application.cpp

        orchestrators/AngleFromVideoInteractor.cpp
        orchestrators/PersistentConfigurator.cpp
        orchestrators/VideoSourceManager.cpp

        usecases/settings/ConfigureComponent.cpp
        usecases/settings/OpenSelectedCameras.cpp

)

target_include_directories(application
        PUBLIC
        ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(application
        PUBLIC
        domain
)