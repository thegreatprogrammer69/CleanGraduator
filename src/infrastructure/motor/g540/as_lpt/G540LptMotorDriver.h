#ifndef CLEANGRADUATOR_G540BOARDLPT_H
#define CLEANGRADUATOR_G540BOARDLPT_H
#include <memory>

#include "G540LptMotorDriverConfig.h"
#include "G540LptMotorDriverPorts.h"
#include "domain/fmt/Logger.h"
#include "domain/ports/motor/IMotorDriver.h"

namespace infra::hardware {
    class G540LptMotorDriver : public domain::ports::IMotorDriver  {
    public:
        explicit G540LptMotorDriver(const G540LptMotorDriverPorts& ports, const G540LptMotorDriverConfig &config);
        ~G540LptMotorDriver() override;


    private:
        struct G540LptImpl;
        std::unique_ptr<G540LptImpl> impl_;
        fmt::Logger logger_;
    };
}

#endif //CLEANGRADUATOR_G540BOARDLPT_H