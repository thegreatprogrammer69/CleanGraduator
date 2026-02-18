#include "UseCasesBootstrap.h"

#include "ApplicationBootstrap.h"
#include "application/orchestrators/motor/MotorControlInteractor.h"
#include "application/usecases/cameras/OpenSelectedCameras.h"
#include "application/usecases/cameras/OpenAllCameras.h"
#include "application/usecases/cameras/CloseAllCameras.h"


using namespace application::usecase;
using namespace application::orchestrators;

UseCasesBootstrap::UseCasesBootstrap(ApplicationBootstrap &application): app_(application) {}

UseCasesBootstrap::~UseCasesBootstrap() {
}

void UseCasesBootstrap::initialize() {
    createOpenSelectedCameras();
    createOpenAllCameras();
    createCloseAllCameras();

    createMotorControlInteractor();
}

void UseCasesBootstrap::createOpenSelectedCameras() {
    open_selected_cameras = std::make_unique<OpenSelectedCameras>(*app_.video_source_manager);
}

void UseCasesBootstrap::createOpenAllCameras() {
    open_all_cameras = std::make_unique<OpenAllCameras>(*app_.video_source_manager);
}

void UseCasesBootstrap::createCloseAllCameras() {
    close_all_cameras = std::make_unique<CloseAllCameras>(*app_.video_source_manager);
}

void UseCasesBootstrap::createMotorControlInteractor() {
    motor_control_interactor = std::make_unique<MotorControlInteractor>(*app_.motor_driver);
}
