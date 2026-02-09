#include "SettingsViewModel.h"

mvvm::SettingsViewModel::SettingsViewModel(application::usecases::SettingsUseCase &use_case)
    : use_case_(use_case)
{
}

mvvm::SettingsViewModel::~SettingsViewModel() {

}
