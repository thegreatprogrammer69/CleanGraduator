#include "VideoSourceGridSettingsViewModel.h"

#include "application/usecases/settings/ApplyGridString.h"

mvvm::VideoSourceGridSettingsViewModel::VideoSourceGridSettingsViewModel(application::usecases::ApplyCameraGridSettings &use_case)
    : use_case_(use_case)
{
}

mvvm::VideoSourceGridSettingsViewModel::~VideoSourceGridSettingsViewModel() = default;

void mvvm::VideoSourceGridSettingsViewModel::applySettings() {
    const auto result = use_case_.execute(settings.get_copy());
    settings.set(result.correct_settings);
    error.set(result.error);
}
