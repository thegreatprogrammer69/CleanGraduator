#ifndef CLEANGRADUATOR_ANGLESOURCEID_H
#define CLEANGRADUATOR_ANGLESOURCEID_H
#include <functional>

namespace domain::common {
    struct SourceId {
        explicit SourceId(int v) : value(v) {}
        SourceId() : value(-1) {}
        ~SourceId() = default;
        int value;
        bool operator==(const SourceId& other) const { return value == other.value; }
        bool operator<(const SourceId& other) const { return value < other.value; }
    };
}

namespace std {

    template<>
    struct hash<domain::common::SourceId>
    {
        size_t operator()(const domain::common::SourceId& id) const noexcept
        {
            return std::hash<int>{}(id.value);
        }
    };

} // namespace std


#endif //CLEANGRADUATOR_ANGLESOURCEID_H