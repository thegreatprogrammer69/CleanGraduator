#include "UiBootstrap.h"

#include "ViewModelsBootstrap.h"

UiBootstrap::UiBootstrap(ViewModelsBootstrap &view_models)
    : view_models_(view_models)
{
}

UiBootstrap::~UiBootstrap() {
}

void UiBootstrap::initialize() {
    createMainWindow();
}

void UiBootstrap::createMainWindow() {
    main_window = std::make_unique<ui::QtMainWindow>(*view_models_.main_window);
}
