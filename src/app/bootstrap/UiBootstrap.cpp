#include "UiBootstrap.h"
#include "ViewModelsBootstrap.h"
#include "ui/widgets/QtMainWindow.h"
#include "ui/audio/QtCalibrationSoundNotifier.h"

UiBootstrap::UiBootstrap(ViewModelsBootstrap &view_models)
    : view_models_(view_models)
{
}

UiBootstrap::~UiBootstrap() {
}

void UiBootstrap::initialize() {
    calibration_sound_notifier_ = std::make_unique<ui::QtCalibrationSoundNotifier>();
    view_models_.calibration_session_control->setSoundNotifier(calibration_sound_notifier_.get());
    createMainWindow();
}

void UiBootstrap::createMainWindow() {
    main_window = std::make_unique<ui::QtMainWindow>(*view_models_.main_window, nullptr);
}
