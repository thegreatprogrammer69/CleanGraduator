#include "CameraGridSettingsViewModel.h"

#include "application/orchestrators/VideoSourceManager.h"
#include "application/usecases/settings/OpenSelectedCameras.h"

using namespace mvvm;

CameraGridSettingsViewModel::CameraGridSettingsViewModel(application::usecase::OpenSelectedCameras &open_use_sase,
    application::services::VideoSourceManager &manager)
        : manager_(manager), open_use_sase_(open_use_sase)
{
}

void CameraGridSettingsViewModel::open()
{
    open_use_sase_.execute(cameraInput.get_copy());
}

void CameraGridSettingsViewModel::openAll()
{
    manager_.openAll();
}

void CameraGridSettingsViewModel::closeAll()
{
    manager_.closeAll();
}
