#include "ViewModelsBootstrap.h"

#include "ApplicationBootstrap.h"
#include "UseCasesBootstrap.h"
#include "viewmodels/logging/LogViewerViewModel.h"
#include "viewmodels/MainWindowViewModel.h"
#include "viewmodels/settings/CameraGridSettingsViewModel.h"
#include "viewmodels/settings/InfoSettingsViewModel.h"
#include "viewmodels/settings/SettingsViewModel.h"
#include "viewmodels/status_bar/AppStatusBarViewModel.h"
#include "viewmodels/status_bar/MotorDriverStatusViewModel.h"
#include "viewmodels/status_bar/PressureSensorStatusBarViewModel.h"
#include "viewmodels/video/VideoSourceGridViewModel.h"
#include "viewmodels/video/VideoSourceViewModel.h"

using namespace mvvm;

ViewModelsBootstrap::ViewModelsBootstrap(ApplicationBootstrap &application, UseCasesBootstrap &use_cases)
    : app_(application), use_cases_(use_cases)
{
}

ViewModelsBootstrap::~ViewModelsBootstrap() {
}

void ViewModelsBootstrap::initialize() {
    createVideoSourceViewModels();
    createVideoSourceGrid();

    createCameraGridSettings();
    createInfoSettings();
    createSettings();

    createStatusBar();
    createMotorDriverStatus();
    createPressureSensorStatus();

    createLogViewer();

    createMainWindow();
}

void ViewModelsBootstrap::createVideoSourceViewModels() {
    for (const auto& vs : app_.video_sources) {
        video_source_view_models.emplace_back(std::make_unique<VideoSourceViewModel>(*vs));
    }
}

void ViewModelsBootstrap::createVideoSourceGrid() {
    constexpr int columns = 2;
    constexpr int rows = 4;
    constexpr double aspectRatioWH = 4.0 / 3.0;

    VideoSourceGridViewModel::Slots vm_slots;
    int i = 0;
    for (const auto& view_model : video_source_view_models) {

        int row = i % rows;
        int col = i / rows;

        vm_slots.emplace_back(row, col, *view_model);

        ++i;
    }

    video_source_grid = std::make_unique<VideoSourceGridViewModel>(vm_slots, rows, columns, aspectRatioWH);
}

void ViewModelsBootstrap::createCameraGridSettings() {
    CameraGridSettingsViewModelDeps deps {
        *use_cases_.open_selected_cameras,
        *use_cases_.open_all_cameras,
        *use_cases_.close_all_cameras,
    };
    camera_grid_settings = std::make_unique<CameraGridSettingsViewModel>(deps);
}

void ViewModelsBootstrap::createInfoSettings() {
    InfoSettingsViewModelDeps deps{
        *app_.info_settings_storage,
        *app_.displacement_catalog,
        *app_.gauge_catalog,
        *app_.precision_catalog,
        *app_.pressure_unit_catalog,
        *app_.printer_catalog,
    };

    info_settings =
        std::make_unique<InfoSettingsViewModel>(deps);
}


void ViewModelsBootstrap::createSettings() {
    SettingsViewModelDeps deps{
        *camera_grid_settings,
        *info_settings,
    };

    settings =
        std::make_unique<SettingsViewModel>(deps);
}

void ViewModelsBootstrap::createStatusBar() {
    AppStatusBarViewModelDeps deps {
        *app_.process_lifecycle,
        *app_.session_clock,
        *app_.uptime_clock
    };

    app_status_bar = std::make_unique<AppStatusBarViewModel>(deps);
}

void ViewModelsBootstrap::createMotorDriverStatus() {
    MotorDriverStatusViewModelDeps deps{
        *app_.motor_driver
    };

    motor_driver_status = std::make_unique<MotorDriverStatusViewModel>(deps);
}

void ViewModelsBootstrap::createPressureSensorStatus() {
    PressureSensorStatusBarViewModelDeps deps{
        *app_.pressure_source
    };

    pressure_sensor_status = std::make_unique<PressureSensorStatusBarViewModel>(deps);
}

void ViewModelsBootstrap::createLogViewer() {
    log_viewer = std::make_unique<LogViewerViewModel>(*app_.log_sources_storage);
}

void ViewModelsBootstrap::createMainWindow() {
    StatusBarViewModels status_bar_view_models {
        *app_status_bar,
        *motor_driver_status,
        *pressure_sensor_status
    };

    MainWindowViewModelDeps deps{
        *log_viewer,
        *video_source_grid,
        *settings,
        status_bar_view_models
    };

    main_window =
        std::make_unique<MainWindowViewModel>(deps);
}
