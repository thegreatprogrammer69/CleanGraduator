#ifndef CLEANGRADUATOR_FMT_DOMAIN_H
#define CLEANGRADUATOR_FMT_DOMAIN_H
#include <iosfwd>

namespace domain::common {
    class Angle;
    class AngleUnit;
}

namespace fmt {
    using namespace domain::common;
    std::ostream& operator<<(std::ostream& os, const Angle& f); // Выводит значение вместе с его текущим Unit
    std::ostream& operator<<(std::ostream& os, const AngleUnit& f);
}

#endif //CLEANGRADUATOR_FMT_DOMAIN_H