add_library(application STATIC
    fmt/fmt_application.cpp
    interactors/AngleFromVideoInteractor.cpp
        ports/outbound/settings/ISettingsRepository.h
        dto/CameraString.h
        dto/UserSettings.h
        dto/FrameCrosshair.h
        usecases/settings/SettingsUseCase.h
        usecases/settings/SettingsUseCase.cpp
)

target_include_directories(application
        PUBLIC
        ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(application
        PUBLIC
        domain
)