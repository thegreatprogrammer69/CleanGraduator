#ifndef CLEANGRADUATOR_CASTANGLEMETER_H
#define CLEANGRADUATOR_CASTANGLEMETER_H
#include "domain/ports/inbound/IAngleCalculator.h"
#include "CastAnglemeterConfig.h"
#include "CastAnglemeterPorts.h"
#include "domain/fmt/FmtLogger.h"

namespace infra::calculation {
    class CastAnglemeter : public domain::ports::IAngleCalculator {
    public:
        CastAnglemeter(CastAnglemeterPorts, CastAnglemeterConfig);
        ~CastAnglemeter() override;
        domain::common::Angle calculate(const domain::common::AnglemeterInput&) override;
    private:
        struct CastAnglemeterImpl;
        std::unique_ptr<CastAnglemeterImpl> impl_;
        fmt::FmtLogger logger_;
    };
}

#endif //CLEANGRADUATOR_CASTANGLEMETER_H
