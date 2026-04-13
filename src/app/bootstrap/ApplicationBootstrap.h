#ifndef CLEANGRADUATOR_APPBOOTSTRAP_H
#define CLEANGRADUATOR_APPBOOTSTRAP_H

#include <memory>
#include <string>
#include <vector>

namespace domain::ports {
    struct ICalibrationRecorder;
    class ICalibrationStrategy;
    class ICalibrationResultSource;
    class ICalibrationResultValidationSource;
    struct IValveDriver;
    struct IResultStore;
    struct IPressureSource;
    class ICalibrationCalculator;
    struct IAngleCalculator;
    struct IAngleSource;
    struct IVideoSource;
    struct IClock;
    struct ISessionClock;
    struct ILogger;
    struct IMotorDriver;
}

namespace application::ports {
    struct IFileLoggingControl;
    struct ILogSourcesStorage;
    struct IInfoSettingsStorage;
    struct IPrinterCatalog;
    struct IPressureUnitCatalog;
    struct IGaugePrecisionCatalog;
    struct IGaugeCatalog;
    struct IDisplacementCatalog;
    struct IVideoAngleSourcesStorage;
}

namespace application::orchestrators {
    class MotorControlInteractor;
    class VideoSourceManager;
}

namespace infra::logging {
    class NamedMultiLogger;
}

class ApplicationBootstrap {
public:
    explicit ApplicationBootstrap(const std::string &setup_dir, const std::string &catalogs_dir, const std::string &logs_dir);
    ~ApplicationBootstrap();

    void initialize();

    domain::ports::ILogger& createLogger(const std::string &logger_name);

    ////////////////////////////////////////////////////////////////////////////////////////
    std::unique_ptr<domain::ports::IClock> uptime_clock;

    ////////////////////////////////////////////////////////////////////////////////////////
    // Логеры
    std::unique_ptr<application::ports::ILogSourcesStorage> log_sources_storage;
    std::unique_ptr<application::ports::IFileLoggingControl> file_logging_control;
    std::vector<std::unique_ptr<domain::ports::ILogger>> loggers;

    ////////////////////////////////////////////////////////////////////////////////////////
    // Время процесса
    std::unique_ptr<domain::ports::ISessionClock> session_clock;

    ////////////////////////////////////////////////////////////////////////////////////////
    // Угломер и калибратор
    std::unique_ptr<domain::ports::IAngleCalculator> anglemeter;


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
    // Драйвер двигателя
    std::unique_ptr<domain::ports::IMotorDriver> motor_driver;

    ////////////////////////////////////////////////////////////////////////////////////////
    // Задатчик давления. Делавет весь процесс градуировки
    // std::unique_ptr<domain::ports::IPressureActuator> pressure_actuator;

    ////////////////////////////////////////////////////////////////////////////////////////
    // Калибровка
    std::unique_ptr<domain::ports::ICalibrationCalculator> calibration_calculator;
    std::unique_ptr<domain::ports::ICalibrationStrategy> calibration_strategy;
    std::unique_ptr<domain::ports::ICalibrationRecorder> calibration_recorder;
    std::unique_ptr<domain::ports::ICalibrationResultSource> calibration_result_source;
    std::unique_ptr<domain::ports::ICalibrationResultValidationSource> calibration_result_validation_source;

    ////////////////////////////////////////////////////////////////////////////////////////
    // Каталоги
    std::unique_ptr<application::ports::IDisplacementCatalog> displacement_catalog;
    std::unique_ptr<application::ports::IGaugeCatalog> gauge_catalog;
    std::unique_ptr<application::ports::IGaugePrecisionCatalog> precision_catalog;
    std::unique_ptr<application::ports::IPressureUnitCatalog> pressure_unit_catalog;
    std::unique_ptr<application::ports::IPrinterCatalog> printer_catalog;

    ////////////////////////////////////////////////////////////////////////////////////////
    // Сохранение настроек
    std::unique_ptr<application::ports::IInfoSettingsStorage> info_settings_storage;


    ////////////////////////////////////////////////////////////////////////////////////////
    // Реагирует на состояние и запускает какие-либо процессы


private:
    std::string setup_dir_;
    std::string catalogs_dir_;
    std::string logs_dir_;
    std::string session_logs_dir_;
    domain::ports::ILogger* session_file_logger_ = nullptr;

    ////////////////////////////////////////////////////////////////////////////////////////
    void createLogSourcesStorage();
    void createFileLoggingControl();
    void createSessionLogsDirectory();

    ////////////////////////////////////////////////////////////////////////////////////////
    void createSessionClock();
    void createClock();

    ////////////////////////////////////////////////////////////////////////////////////////
    void createAnglemeter();

    ////////////////////////////////////////////////////////////////////////////////////////
    void createVideoSources();
    void createAngleSources();
    void createVideoSourcesStorage();
    void createVideoSourcesManager();

    ////////////////////////////////////////////////////////////////////////////////////////
    void createPressureSource();
    void createMotorDriver();

    ////////////////////////////////////////////////////////////////////////////////////////
    void createCalibrationStrategy();
    void createCalibrationRecorder();
    void createCalibrationCalculator();
    void createCalibrationResultSource();
    void createCalibrationResultValidationSource();

    ////////////////////////////////////////////////////////////////////////////////////////
    void createDisplacementCatalog();
    void createPrinterCatalog();
    void createPrecisionCatalog();
    void createPressureUnitCatalog();
    void createGaugeCatalog();

    ////////////////////////////////////////////////////////////////////////////////////////
    void createInfoSettingsStorage();

    ////////////////////////////////////////////////////////////////////////////////////////
    void createProcessRunner();


};

#endif //CLEANGRADUATOR_APPBOOTSTRAP_H
