#ifndef CLEANGRADUATOR_FMT_DOMAIN_H
#define CLEANGRADUATOR_FMT_DOMAIN_H
#include <iosfwd>

namespace domain::common {
    enum class CalibrationCellSeverity;
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

std::ostream& operator<<(std::ostream& os, const domain::common::Angle& f); // Выводит значение вместе с его текущим Unit
std::ostream& operator<<(std::ostream& os, const domain::common::AngleUnit& f);
std::ostream& operator<<(std::ostream& os, const domain::common::Pressure& f);
std::ostream& operator<<(std::ostream& os, const domain::common::PressureUnit& f);
std::ostream& operator<<(std::ostream& os, const domain::common::Timestamp& f);
std::ostream& operator<<(std::ostream& os, const domain::common::PixelFormat& f);
std::ostream& operator<<(std::ostream& os, const domain::common::VideoBuffer& f);
std::ostream& operator<<(std::ostream& os, const domain::common::VideoFrame& f);
std::ostream& operator<<(std::ostream& os, const domain::common::ProcessLifecycleState& f);
std::ostream& operator<<(std::ostream& os, const domain::common::CalibrationCellSeverity& f);

#endif //CLEANGRADUATOR_FMT_DOMAIN_H
