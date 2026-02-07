#ifndef CLEANGRADUATOR_G540BOARDLPT_H
#define CLEANGRADUATOR_G540BOARDLPT_H
#include <atomic>
#include <chrono>
#include <thread>

#include "G540LptConfig.h"
#include "G540Ports.h"
#include "IG540.h"

#include "infrastructure/platform/lpt/LptPort.h"

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
        const G540Ports& ports_;
        G540LptConfig config_;
        platform::LptPort lpt_port_;

        std::atomic<bool> stopped_{true};
        mutable std::thread worker_;

        std::atomic<G540Direction> direction_;
        std::atomic<std::chrono::steady_clock::duration> half_period_{};
        struct {std::atomic<unsigned char> b1; std::atomic<unsigned char> b2; } step_bytes_{};
        struct {std::atomic<unsigned char> begin; std::atomic<unsigned char> end; } limit_bytes_{};
    };
}

#endif //CLEANGRADUATOR_G540BOARDLPT_H