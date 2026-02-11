add_library(application STATIC
        fmt/fmt_application.cpp

        interactors/AngleFromVideoInteractor.cpp

        usecases/settings/ApplyGridString.cpp
        usecases/settings/OpenAllCameras.cpp
        ports/factory/IAnglemeterFactory.h
        ports/factory/ICalibrationCalculatorFactory.h
        dto/settings/grid/VideoSourceGridSettings.h
        usecases/settings/ChangeGridSettings.cpp
        usecases/settings/ChangeGridSettings.h
        command/Command.h
        command/CommandType.h
        usecases/settings/grid/CameraGridUseCase.cpp
        usecases/settings/grid/CameraGridUseCase.h
        usecases/settings/grid/CameraGridCommand.h
        services/CameraGridService.cpp
        services/CameraGridService.h
        usecases/grid/GridUseCase.cpp
        usecases/grid/GridUseCase.h
        usecases/grid/Command.h
        usecases/grid/ApplySettingsCommand.h
        usecases/grid/PullSettingsCommand.h
        usecases/grid/CloseAllCommand.h
        usecases/grid/OpenAllCommand.h
        usecases/grid/Result.h
        usecases/grid/OpenAllResult.h
        usecases/grid/ApplySettingsResult.h
        usecases/grid/PullSettingsResult.h
        usecases/video/ApplyVideoSourceGridSettings.h
        usecases/video/ApplyVideoSourceGridSettingsResult.h
)

target_include_directories(application
        PUBLIC
        ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(application
        PUBLIC
        domain
)