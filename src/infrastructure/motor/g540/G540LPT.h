#ifndef CLEANGRADUATOR_G540BOARDLPT_H
#define CLEANGRADUATOR_G540BOARDLPT_H
#include <memory>

#include "G540LptConfig.h"
#include "G540Ports.h"
#include "IG540.h"
#include "domain/fmt/Logger.h"

namespace infra::hardware {
    class G540LPT : public IG540 {
    public:
        explicit G540LPT(const G540Ports& ports, const G540LptConfig &config);
        ~G540LPT() override;

        void start() override;
        void stop() override;
        void emergencyStop() override;
        bool stopped() const override;

        void applyFrequency(int hz) override;
        void applyDirection(G540Direction direction) override;
        void applyFlapsState(G540FlapsState flaps_state) override;
        G540LimitsState getLimitsState() const override;

    private:
        void run();
        unsigned char readState() const;

    private:
        struct G540LptImpl;
        std::unique_ptr<G540LptImpl> impl_;
        const G540Ports& ports_;
        G540LptConfig config_;
        fmt::Logger logger_;
    };
}

#endif //CLEANGRADUATOR_G540BOARDLPT_H