#ifndef CLEANGRADUATOR_CASTANGLEMETERCONFIGURATOR_H
#define CLEANGRADUATOR_CASTANGLEMETERCONFIGURATOR_H
#include "application/ports/outbound/IRuntimeConfigurator.h"

namespace infra::configurators {
    class AnglemeterConfigurator final : public application::ports::IRuntimeConfigurator {
    public:
        application::models::ConfigSchema schema() const override;

        void setInt(const std::string &name, int value) override;
        void setBool(const std::string &name, bool value) override;
        void setText(const std::string &name, const std::string &value) override;
        void invoke(const std::string &action) override;
    };
}


#endif //CLEANGRADUATOR_CASTANGLEMETERCONFIGURATOR_H