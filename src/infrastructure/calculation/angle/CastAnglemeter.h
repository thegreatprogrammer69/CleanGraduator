#ifndef CLEANGRADUATOR_CASTANGLEMETER_H
#define CLEANGRADUATOR_CASTANGLEMETER_H
#include "domain/ports/inbound/IAngleCalculator.h"
#include "CastAnglemeterConfig.h"
#include "AnglemeterPorts.h"
#include "domain/fmt/Logger.h"
#include <vector>

namespace infra::calc {
    class CastAnglemeter : public domain::ports::IAngleCalculator {
    public:
        CastAnglemeter(AnglemeterPorts, CastAnglemeterConfig);
        ~CastAnglemeter() override;
        domain::common::Angle calculate(const domain::common::AnglemeterInput&) override;

        void setBrightLim(int brightLim);
        void setMaxPairs(int maxPairs);
        void setScanStep(int scanStep);
        void setOffsets(const std::vector<int>& offsets);

    private:
        struct CastAnglemeterImpl;
        std::unique_ptr<CastAnglemeterImpl> impl_;
        fmt::Logger logger_;
    };
}

#endif //CLEANGRADUATOR_CASTANGLEMETER_H
