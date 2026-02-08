#ifndef CLEANGRADUATOR_IRESULTSTORE_H
#define CLEANGRADUATOR_IRESULTSTORE_H

namespace domain::common {
    class Result;
}

namespace domain::ports {
    class IResultObserver;
    struct IResultStore {
        virtual ~IResultStore() = default;
        virtual common::Result& current() = 0;
        virtual void save() = 0;
        virtual void addObserver(IResultObserver&) = 0;
    };
}

#endif //CLEANGRADUATOR_IRESULTSTORE_H