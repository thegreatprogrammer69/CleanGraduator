#ifndef CLEANGRADUATOR_POINTID_H
#define CLEANGRADUATOR_POINTID_H

namespace domain::common {
    struct PointId {
        PointId(const unsigned int id, const float pressure) : id(id), pressure(pressure) {}
        PointId() = default;
        unsigned int id;
        float pressure;
        bool operator==(const PointId& rhs) const { return id == rhs.id; }
    };
}

namespace std {

    template<>
    struct hash<domain::common::PointId>
    {
        size_t operator()(const domain::common::PointId& id) const noexcept
        {
            return std::hash<int>{}(id.id);
        }
    };

} // namespace std

#endif //CLEANGRADUATOR_POINTID_H