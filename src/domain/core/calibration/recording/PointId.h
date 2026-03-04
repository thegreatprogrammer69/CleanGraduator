#ifndef CLEANGRADUATOR_POINTID_H
#define CLEANGRADUATOR_POINTID_H

namespace domain::common {
    struct PointId {
        PointId(const unsigned int value) : value(value) {}
        PointId() = default;
        unsigned int value;
        bool operator==(const PointId& rhs) const { return value == rhs.value; }
    };
}

namespace std {

    template<>
    struct hash<domain::common::PointId>
    {
        size_t operator()(const domain::common::PointId& id) const noexcept
        {
            return std::hash<int>{}(id.value);
        }
    };

} // namespace std

#endif //CLEANGRADUATOR_POINTID_H