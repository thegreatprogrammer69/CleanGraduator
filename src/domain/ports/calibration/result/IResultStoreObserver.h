#ifndef CLEANGRADUATOR_IRESULTOBSERVER_H
#define CLEANGRADUATOR_IRESULTOBSERVER_H

namespace domain::common {
    class CalibrationResult;
}

namespace domain::ports {
    struct IResultStoreObserver {
        virtual ~IResultStoreObserver() = default;
        virtual void onResultUpdated(const common::CalibrationResult& result) = 0;
    };
}

#endif //CLEANGRADUATOR_IRESULTOBSERVER_H