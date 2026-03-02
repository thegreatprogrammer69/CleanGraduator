#ifndef CLEANGRADUATOR_POINTID_H
#define CLEANGRADUATOR_POINTID_H

namespace domain::common {
    struct PointId {
        unsigned int id;
        bool operator==(const PointId& rhs) const { return id == rhs.id; }
    };
}

#endif //CLEANGRADUATOR_POINTID_H