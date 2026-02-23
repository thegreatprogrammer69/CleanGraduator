#ifndef CLEANGRADUATOR_GAUGEPRECISION_H
#define CLEANGRADUATOR_GAUGEPRECISION_H

namespace domain::common {
    struct GaugePrecision {
        double value{0.0};
        bool isCorrect() const { return value >= 0.5; }
        bool operator==(const GaugePrecision& rhs) const { return value == rhs.value; }
    };
}

#endif //CLEANGRADUATOR_GAUGEPRECISION_H