#ifndef CLEANGRADUATOR_GAUGE_H
#define CLEANGRADUATOR_GAUGE_H

#include <string>
#include <vector>

namespace application::models {
    struct Gauge {
        std::wstring name;
        std::vector<double> values;
    };
}

#endif // CLEANGRADUATOR_GAUGE_H
