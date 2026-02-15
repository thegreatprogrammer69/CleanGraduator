#ifndef CLEANGRADUATOR_USECASESBOOTSTRAP_H
#define CLEANGRADUATOR_USECASESBOOTSTRAP_H
#include <memory>

#include "application/usecases/cameras/CloseAllCameras.h"
#include "application/usecases/cameras/OpenAllCameras.h"
#include "application/usecases/cameras/OpenSelectedCameras.h"

class ApplicationBootstrap;

class UseCasesBootstrap {
public:
    explicit UseCasesBootstrap(ApplicationBootstrap& application)
        : application_(application)
    {}

    void initialize();

    // Камеры
    std::unique_ptr<application::usecase::OpenSelectedCameras> open_selected_cameras;
    std::unique_ptr<application::usecase::OpenAllCameras> open_all_cameras;
    std::unique_ptr<application::usecase::CloseAllCameras> close_all_cameras;

    // Конфигурируемы объект
    // std::unique_ptr<application::usecase::ConfigureComponent> configure_component;

private:
    ApplicationBootstrap& application_;
    std::unique_ptr<application::usecase::OpenSelectedCameras> createOpenSelectedCameras();
    std::unique_ptr<application::usecase::OpenAllCameras> createOpenAllCameras();
    std::unique_ptr<application::usecase::CloseAllCameras> createCloseAllCameras();

};


#endif //CLEANGRADUATOR_USECASESBOOTSTRAP_H