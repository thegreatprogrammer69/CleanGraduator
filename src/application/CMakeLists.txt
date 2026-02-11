add_library(application STATIC
        fmt/fmt_application.cpp

        interactors/AngleFromVideoInteractor.cpp

        services/VideoSourceGridService.cpp

        usecases/settings/ApplyGridString.cpp
        usecases/settings/OpenAllCameras.cpp
)

target_include_directories(application
        PUBLIC
        ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(application
        PUBLIC
        domain
)