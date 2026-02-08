#ifndef CLEANGRADUATOR_FMT_APPLICATION_H
#define CLEANGRADUATOR_FMT_APPLICATION_H
#include <iosfwd>

namespace application::model {
    class DisplacementRecord;
    class GaugeRecord;
    class PrinterRecord;
}

std::ostream& operator<<(std::ostream& os, const application::model::DisplacementRecord& f);
std::ostream& operator<<(std::ostream& os, const application::model::GaugeRecord& f);
std::ostream& operator<<(std::ostream& os, const application::model::PrinterRecord& f);


#endif //CLEANGRADUATOR_FMT_APPLICATION_H