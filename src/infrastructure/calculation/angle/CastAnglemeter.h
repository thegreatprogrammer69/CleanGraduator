#ifndef CLEANGRADUATOR_CASTANGLEMETER_H
#define CLEANGRADUATOR_CASTANGLEMETER_H
#include "domain/ports/inbound/IAngleCalculator.h"
#include "CastAnglemeterConfig.h"
#include "AnglemeterPorts.h"
#include "domain/fmt/FmtLogger.h"

namespace infra::calc {
    class CastAnglemeter : public domain::ports::IAngleCalculator {
    public:
        CastAnglemeter(AnglemeterPorts, CastAnglemeterConfig);
        ~CastAnglemeter() override;
        domain::common::Angle calculate(const domain::common::AnglemeterInput&) override;
    private:
        struct CastAnglemeterImpl;
        std::unique_ptr<CastAnglemeterImpl> impl_;
        fmt::FmtLogger logger_;
    };
}

#endif //CLEANGRADUATOR_CASTANGLEMETER_H
