#ifndef CLEANGRADUATOR_RESULTRECORD_H
#define CLEANGRADUATOR_RESULTRECORD_H
#include <chrono>

#include "../model/GaugeRecord.h"
#include "domain/core/result/Result.h"

namespace application::dto {
    struct ResultRecord {
        domain::common::Result result;
        GaugeRecord gauge;
        std::chrono::system_clock::duration duration;
        int printerId;
    };
    struct ResultRecordId {
        int value;
        bool operator==(const ResultRecordId& o) const {  return value == o.value;  }
        bool operator<(const ResultRecordId& o) const {  return value < o.value;  }
    };
}

#endif //CLEANGRADUATOR_RESULTRECORD_H