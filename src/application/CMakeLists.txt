add_library(application STATIC
        fmt/fmt_application.cpp

        interactors/AngleFromVideoInteractor.cpp

        usecases/settings/ChangeGridSettings.cpp

        usecases/settings/grid/CameraGridUseCase.cpp

)

target_include_directories(application
        PUBLIC
        ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(application
        PUBLIC
        domain
)