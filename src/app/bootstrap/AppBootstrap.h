#ifndef CLEANGRADUATOR_APPBOOTSTRAP_H
#define CLEANGRADUATOR_APPBOOTSTRAP_H

#include <memory>
#include <string>
#include <vector>

#include "application/orchestrators/AngleFromVideoInteractor.h"
#include "application/orchestrators/VideoSourceManager.h"
#include "application/ports/outbound/IDisplacementCatalog.h"
#include "application/ports/outbound/IGaugeCatalog.h"
#include "application/ports/outbound/IPrecisionCatalog.h"
#include "application/ports/outbound/IPressureUnitCatalog.h"
#include "application/ports/outbound/IPrinterCatalog.h"
#include "application/ports/outbound/ISettingsStorage.h"
#include "application/ports/outbound/IVideoSourceStorage.h"
#include "application/usecases/settings/ConfigureComponent.h"
#include "application/usecases/settings/OpenSelectedCameras.h"
#include "domain/ports/inbound/IAngleCalculator.h"
#include "domain/ports/inbound/ICalibrationCalculator.h"
#include "domain/ports/inbound/IPressureActuator.h"
#include "domain/ports/outbound/IPressureActuatorObserver.h"
#include "domain/ports/inbound/IPressureSource.h"
#include "domain/ports/inbound/IProcessLifecycle.h"
#include "domain/ports/inbound/IVideoSource.h"
#include "domain/ports/inbound/IVideoSourceOverlay.h"
#include "domain/ports/outbound/ILogger.h"
#include "domain/ports/outbound/IResultStore.h"
#include "domain/ports/outbound/IResultStoreObserver.h"
#include "ui/widgets/QtMainWindow.h"
#include "viewmodels/settings/CameraGridSettingsViewModel.h"
#include "viewmodels/video/VideoSourceGridViewModel.h"
#include "viewmodels/video/VideoSourceViewModel.h"
#include "viewmodels/MainWindowViewModel.h"

class AppBootstrap {
public:
    explicit AppBootstrap(const std::string &setup_dir, const std::string &catalogs_dir, const std::string &logs_dir);
    ~AppBootstrap();

    void initialize();
    ui::QtMainWindow& mainWindow();

    domain::ports::ILogger& createLogger(const std::string &logger_name);

private:
    std::unique_ptr<domain::ports::IProcessLifecycle> createLifecycle();
    std::vector<std::unique_ptr<domain::ports::IVideoSource>> createVideoSources();
    std::unique_ptr<domain::ports::IAngleCalculator> createAnglemeter();

    std::vector<std::unique_ptr<application::orchestrators::AngleFromVideoInteractor>> createAngleFromVideoInteractor();

private:
    std::string setup_dir_;
    std::string catalogs_dir_;
    std::string logs_dir_;

    ////////////////////////////////////////////////////////////////////////////////////////
    // Domain Ports

    // Время (для синхронизации сущностей)
    std::vector<std::unique_ptr<domain::ports::IClock>> clock_;

    // Логеры
    std::vector<std::unique_ptr<domain::ports::ILogger>> loggers_;

    // Видеокамеры
    std::vector<std::unique_ptr<domain::ports::IVideoSource>> video_sources_;
    std::vector<std::unique_ptr<domain::ports::IVideoSourceObserver>> video_source_observers_;
    // Источники угла
    std::vector<std::unique_ptr<domain::ports::IAngleSource>> angle_sources_;
    // Рисовщики на кадре. Например рисование мушки, обводка стрелочки и прочее.
    std::vector<std::unique_ptr<domain::ports::IVideoSourceOverlay>> video_source_overlays__;

    // Жизненный цикл и наблюдатель, который реагирует на состояние и запускает какие-либо процессы
    std::unique_ptr<domain::ports::IProcessLifecycle> process_lifecycle_;
    std::unique_ptr<domain::ports::IProcessLifecycleObserver> process_runner_;

    // Конкретная реализация угломера
    std::unique_ptr<domain::ports::IAngleCalculator> anglemeter_;
    std::unique_ptr<domain::ports::ICalibrationCalculator> calibrator_;

    // Источник давления. Может быть какой-то датик давления, например ДМ5002М
    std::unique_ptr<domain::ports::IPressureSource> pressure_source_;
    std::unique_ptr<domain::ports::IPressureActuatorObserver> pressure_oservers_;

    // Задатчик давления. Делавет весь процесс градуировки
    std::unique_ptr<domain::ports::IPressureActuator> pressure_actuator_;

    // Хранение результата и наблюдатель за результатом. Наблюдать может MVVM таблицы градуировки например.
    std::unique_ptr<domain::ports::IResultStore> result_store_;
    std::unique_ptr<domain::ports::IResultStoreObserver> result_store_observer_;


    ////////////////////////////////////////////////////////////////////////////////////////
    // Application Ports

    // Catalogs
    std::unique_ptr<application::ports::IDisplacementCatalog> displacement_catalog_;
    std::unique_ptr<application::ports::IGaugeCatalog> gauge_catalog_;
    std::unique_ptr<application::ports::IPrecisionCatalog> precision_catalog_;
    std::unique_ptr<application::ports::IPressureUnitCatalog> pressure_unit_catalog_;
    std::unique_ptr<application::ports::IPrinterCatalog> printer_catalog_;

    // Video Source Storage
    std::unique_ptr<application::ports::IVideoSourceStorage> video_source_storage_;

    // Settings Storage
    std::unique_ptr<application::ports::ISettingsStorage> settings_storage_;


    // Application Orchestrators
    // Оркестратор, который является источником угла
    std::vector<std::unique_ptr<application::orchestrators::AngleFromVideoInteractor>> angle_from_video_interactors_;
    std::unique_ptr<application::orchestrators::VideoSourceManager> video_source_manager_;

    // Application Use Cases
    std::unique_ptr<application::usecase::ConfigureComponent> configure_component_use_case_;
    std::unique_ptr<application::usecase::OpenSelectedCameras> open_selected_cameras_use_case_;


    ////////////////////////////////////////////////////////////////////////////////////////
    // ViewModels
    std::vector<std::unique_ptr<mvvm::VideoSourceViewModel>> video_source_view_models_;
    std::unique_ptr<mvvm::VideoSourceGridViewModel> video_source_grid_view_model_;


    std::unique_ptr<mvvm::MainWindowViewModel> mainWindowViewModel_;


    ////////////////////////////////////////////////////////////////////////////////////////
    // Qt UI
    std::unique_ptr<ui::QtMainWindow> mainWindow_;
};

#endif //CLEANGRADUATOR_APPBOOTSTRAP_H
