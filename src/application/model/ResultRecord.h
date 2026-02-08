#ifndef CLEANGRADUATOR_RESULTRECORD_H
#define CLEANGRADUATOR_RESULTRECORD_H
#include <chrono>

#include "GaugeRecord.h"
#include "domain/core/result/Result.h"

namespace application::model {
    struct ResultRecord {
        domain::common::Result result;
        GaugeRecord gauge;
        std::chrono::system_clock::duration duration;
        int printerId;
    };
}

#endif //CLEANGRADUATOR_RESULTRECORD_H