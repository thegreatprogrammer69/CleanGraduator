add_library(application STATIC
        fmt/fmt_application.cpp

        dto/settings/camera_grid/VideoSourceGridString.cpp

        interactors/AngleFromVideoInteractor.cpp

        services/VideoSourceGridService.cpp

        usecases/settings/ApplyCameraGridSettings.cpp
)

target_include_directories(application
        PUBLIC
        ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(application
        PUBLIC
        domain
)