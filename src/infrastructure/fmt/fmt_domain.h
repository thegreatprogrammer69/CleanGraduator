#ifndef CLEANGRADUATOR_FMT_DOMAIN_H
#define CLEANGRADUATOR_FMT_DOMAIN_H
#include <iosfwd>

namespace domain::common {
    class Angle;
    enum class AngleUnit;
    class Pressure;
    enum class PressureUnit;
    class Timestamp;
    enum class PixelFormat;
    struct VideoBuffer;
    struct VideoFrame;
    enum class ProcessLifecycleState;
}

namespace fmt {
    using namespace domain::common;
    std::ostream& operator<<(std::ostream& os, const Angle& f); // Выводит значение вместе с его текущим Unit
    std::ostream& operator<<(std::ostream& os, const AngleUnit& f);
    std::ostream& operator<<(std::ostream& os, const Pressure& f);
    std::ostream& operator<<(std::ostream& os, const PressureUnit& f);
    std::ostream& operator<<(std::ostream& os, const Timestamp& f);
    std::ostream& operator<<(std::ostream& os, const PixelFormat& f);
    std::ostream& operator<<(std::ostream& os, const VideoBuffer& f);
    std::ostream& operator<<(std::ostream& os, const VideoFrame& f);
    std::ostream& operator<<(std::ostream& os, const ProcessLifecycleState& f);
}

#endif //CLEANGRADUATOR_FMT_DOMAIN_H
