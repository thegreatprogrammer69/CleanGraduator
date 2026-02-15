#ifndef CLEANGRADUATOR_APPBOOTSTRAP_H
#define CLEANGRADUATOR_APPBOOTSTRAP_H

#include <memory>
#include <string>
#include <vector>


namespace infra::process {
    class ProcessRunner;
}

namespace application::orchestrators {
    class VideoSourceManager;
}

namespace application::ports {
    struct ISettingsStorage;
    struct IPrinterCatalog;
    struct IPressureUnitCatalog;
    struct IPrecisionCatalog;
    struct IGaugeCatalog;
    struct IDisplacementCatalog;
    struct IVideoAngleSourcesStorage;
}

namespace infra::storage {
    class LogSourcesStorage;
}

namespace infra::logging {
    class NamedMultiLogger;
}

namespace domain::ports {
    struct IResultStore;
    struct IPressureActuator;
    struct IPressureSource;
    class ICalibrationCalculator;
    struct IAngleCalculator;
    struct IAngleSource;
    struct IVideoSource;
    struct IClock;
    struct IProcessLifecycle;
    struct ILogger;
}

class ApplicationBootstrap {
public:
    explicit ApplicationBootstrap(const std::string &setup_dir, const std::string &catalogs_dir, const std::string &logs_dir);
    ~ApplicationBootstrap();

    void initialize();

    domain::ports::ILogger& createLogger(const std::string &logger_name);

    ////////////////////////////////////////////////////////////////////////////////////////
    // Логеры
    std::unique_ptr<infra::storage::LogSourcesStorage> log_sources_storage;
    std::vector<std::unique_ptr<domain::ports::ILogger>> loggers;

    ////////////////////////////////////////////////////////////////////////////////////////
    // Жизненный цикл и Время
    std::unique_ptr<domain::ports::IProcessLifecycle> process_lifecycle;
    domain::ports::IClock* session_clock;
    std::unique_ptr<domain::ports::IClock> uptime_clock;

    ////////////////////////////////////////////////////////////////////////////////////////
    // Угломер и калибратор
    std::unique_ptr<domain::ports::IAngleCalculator> anglemeter;
    std::unique_ptr<domain::ports::ICalibrationCalculator> calibrator;

    ////////////////////////////////////////////////////////////////////////////////////////
    // Видеокамеры
    std::vector<std::unique_ptr<domain::ports::IVideoSource>> video_sources;
    std::vector<std::unique_ptr<domain::ports::IAngleSource>> angle_sources;
    std::unique_ptr<application::ports::IVideoAngleSourcesStorage> videoangle_sources_storage;
    // Оркестратор, открывающий и закрывающий камеры по индексам
    std::unique_ptr<application::orchestrators::VideoSourceManager> video_source_manager;

    ////////////////////////////////////////////////////////////////////////////////////////
    // Источник давления. Может быть какой-то датик давления, например ДМ5002М
    std::unique_ptr<domain::ports::IPressureSource> pressure_source;

    ////////////////////////////////////////////////////////////////////////////////////////
    // Задатчик давления. Делавет весь процесс градуировки
    std::unique_ptr<domain::ports::IPressureActuator> pressure_actuator;

    ////////////////////////////////////////////////////////////////////////////////////////
    // Хранение результата
    std::unique_ptr<domain::ports::IResultStore> result_store;


    ////////////////////////////////////////////////////////////////////////////////////////
    // Каталоги
    std::unique_ptr<application::ports::IDisplacementCatalog> displacement_catalog;
    std::unique_ptr<application::ports::IGaugeCatalog> gauge_catalog;
    std::unique_ptr<application::ports::IPrecisionCatalog> precision_catalog;
    std::unique_ptr<application::ports::IPressureUnitCatalog> pressure_unit_catalog;
    std::unique_ptr<application::ports::IPrinterCatalog> printer_catalog;

    ////////////////////////////////////////////////////////////////////////////////////////
    // Сохранение настроек
    std::unique_ptr<application::ports::ISettingsStorage> settings_storage;


    ////////////////////////////////////////////////////////////////////////////////////////
    // Реагирует на состояние и запускает какие-либо процессы
    std::unique_ptr<infra::process::ProcessRunner> process_runner;

private:
    std::string setup_dir_;
    std::string catalogs_dir_;
    std::string logs_dir_;

    ////////////////////////////////////////////////////////////////////////////////////////
    void createLogSourcesStorage();

    ////////////////////////////////////////////////////////////////////////////////////////
    void createLifecycle();
    void createClock();

    ////////////////////////////////////////////////////////////////////////////////////////
    void createAnglemeter();
    void createCalibrator();

    ////////////////////////////////////////////////////////////////////////////////////////
    void createVideoSources();
    void createAngleSources();
    void createVideoSourcesStorage();
    void createVideoSourcesManager();

    ////////////////////////////////////////////////////////////////////////////////////////
    void createPressureSource();

    ////////////////////////////////////////////////////////////////////////////////////////
    void createDisplacementCatalog();
    void createPrinterCatalog();
    void createPrecisionCatalog();
    void createPressureUnitCatalog();
    void createGaugeCatalog();

    ////////////////////////////////////////////////////////////////////////////////////////
    void createSettingsStorage();

    ////////////////////////////////////////////////////////////////////////////////////////
    void createProcessRunner();


};

#endif //CLEANGRADUATOR_APPBOOTSTRAP_H
