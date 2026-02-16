#include "ViewModelsBootstrap.h"

#include "ApplicationBootstrap.h"
#include "UseCasesBootstrap.h"
#include "viewmodels/logging/LogViewerViewModel.h"
#include "viewmodels/MainWindowViewModel.h"
#include "viewmodels/settings/CameraGridSettingsViewModel.h"
#include "viewmodels/settings/InfoSettingsViewModel.h"
#include "viewmodels/settings/SettingsViewModel.h"
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
    CameraGridSettingsViewModelDeps deps;
    deps.open_selected = *use_cases_.open_selected_cameras;
    deps.open_all = *use_cases_.open_all_cameras;
    deps.close_all = *use_cases_.close_all_cameras;
    camera_grid_settings = std::make_unique<CameraGridSettingsViewModel>(deps);
}

void ViewModelsBootstrap::createInfoSettings() {
    InfoSettingsViewModelDeps deps;
    deps.storage = *app_.info_settings_storage;
    deps.displacement_catalog = *app_.displacement_catalog;
    deps.gauge_catalog = *app_.gauge_catalog;
    deps.precision_catalog = *app_.precision_catalog;
    deps.pressure_unit_catalog = *app_.pressure_unit_catalog;
    deps.printer_catalog = *app_.printer_catalog;

    info_settings = std::make_unique<InfoSettingsViewModel>(deps);
}

void ViewModelsBootstrap::createSettings() {
    SettingsViewModelDeps deps;
    deps.camera_grid = *camera_grid_settings;
    deps.info_settings = *info_settings;
    settings = std::make_unique<SettingsViewModel>(deps);
}

void ViewModelsBootstrap::createLogViewer() {
    log_viewer = std::make_unique<LogViewerViewModel>(*app_.log_sources_storage);
}

void ViewModelsBootstrap::createMainWindow() {
    MainWindowViewModelDeps deps;
    deps.log_viewer = *log_viewer;
    deps.grid = *video_source_grid;
    deps.settings = *settings;
    main_window = std::make_unique<MainWindowViewModel>(deps);
}
