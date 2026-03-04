#ifndef CLEANGRADUATOR_ANGLESOURCEID_H
#define CLEANGRADUATOR_ANGLESOURCEID_H
#include <functional>

namespace domain::common {
    struct AngleSourceId {
        explicit AngleSourceId(int v) : value(v) {}
        AngleSourceId() : value(-1) {}
        ~AngleSourceId() = default;
        int value;
        bool operator==(const AngleSourceId& other) const { return value == other.value; }
        bool operator<(const AngleSourceId& other) const { return value < other.value; }
    };
}

namespace std {

    template<>
    struct hash<domain::common::AngleSourceId>
    {
        size_t operator()(const domain::common::AngleSourceId& id) const noexcept
        {
            return std::hash<int>{}(id.value);
        }
    };

} // namespace std


#endif //CLEANGRADUATOR_ANGLESOURCEID_H