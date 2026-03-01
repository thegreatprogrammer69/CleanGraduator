#ifndef CLEANGRADUATOR_CALIBRATIONVERDICT_H
#define CLEANGRADUATOR_CALIBRATIONVERDICT_H
#include <string>
#include <variant>

namespace domain::common {
    struct CalibrationStrategyVerdict {
        struct None {};
        struct Complete {};
        struct Fault {
            std::string error;
        };

        using Data = std::variant<None, Complete, Fault>;
        Data data;
        CalibrationStrategyVerdict(Data data) : data(std::move(data)) {}
    };
}

#endif //CLEANGRADUATOR_CALIBRATIONVERDICT_H