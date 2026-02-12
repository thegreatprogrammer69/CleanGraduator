#ifndef CLEANGRADUATOR_PERSISTENTCONFIGURATOR_H
#define CLEANGRADUATOR_PERSISTENTCONFIGURATOR_H


namespace application::models {
    struct ComponentSettings;
}

namespace application::ports {
    class IComponentSettingsRepository;
    class IRuntimeConfigurator;
}

namespace application::ineractors {
    class PersistentConfigurator {
    public:
        PersistentConfigurator(
            ports::IRuntimeConfigurator& configurator,
            ports::IComponentSettingsRepository& repo);

        void loadAndApply();

        void save(const models::ComponentSettings& settings);

    private:
        ports::IRuntimeConfigurator& configurator_;
        ports::IComponentSettingsRepository& repo_;
    };
}


#endif //CLEANGRADUATOR_PERSISTENTCONFIGURATOR_H