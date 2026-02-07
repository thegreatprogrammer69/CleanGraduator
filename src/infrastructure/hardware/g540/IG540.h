#ifndef CLEANGRADUATOR_IG540BOARDDRIVER_H
#define CLEANGRADUATOR_IG540BOARDDRIVER_H

namespace infra::hardware {
    enum class G540LimitsState {
        None,
        Begin,
        End,
        Both            // Оба концевика активны (авария / ошибка)
    };

    enum class G540Direction {
        Neutral, Backward, Forward
    };

    enum class G540FlapsState {
        Uninitialized,
        CloseBoth,
        OpenInput,
        OpenOutput
    };

    class IG540 {
    public:
        virtual ~IG540() = default;

        virtual void start() = 0;
        virtual void stop() = 0;
        virtual void emergencyStop() = 0;
        virtual bool stopped() const = 0;

        virtual void applyFrequency(int hz) = 0;
        virtual void applyDirection(G540Direction direction) = 0;
        virtual void applyFlapsState(G540FlapsState flaps_state) = 0;
        virtual G540LimitsState getLimitsState() const = 0;
    };
}

#endif //CLEANGRADUATOR_IG540BOARDDRIVER_H