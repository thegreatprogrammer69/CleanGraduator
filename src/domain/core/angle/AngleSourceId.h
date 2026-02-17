#ifndef CLEANGRADUATOR_ANGLESOURCEID_H
#define CLEANGRADUATOR_ANGLESOURCEID_H

namespace domain::common {
    struct AngleSourceId {
        explicit AngleSourceId(int v) : value(v) {}
        AngleSourceId() : value(-1) {}
        int value;
        bool operator==(const AngleSourceId& other) const { return value == other.value; }
        bool operator<(const AngleSourceId& other) const { return value < other.value; }
    };
}

#endif //CLEANGRADUATOR_ANGLESOURCEID_H