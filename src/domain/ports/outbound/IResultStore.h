#ifndef CLEANGRADUATOR_IRESULTSTORE_H
#define CLEANGRADUATOR_IRESULTSTORE_H

namespace domain::common {
    class CalibrationResult;
}

namespace domain::ports {
    class IResultStoreObserver;
    struct IResultStore {
        virtual ~IResultStore() = default;
        virtual common::CalibrationResult& current() = 0;
        virtual void save() = 0;
        virtual void addObserver(IResultStoreObserver&) = 0;
        virtual void removeObserver(IResultStoreObserver&) = 0;
    };
}

#endif //CLEANGRADUATOR_IRESULTSTORE_H