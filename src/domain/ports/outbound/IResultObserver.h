#ifndef CLEANGRADUATOR_IRESULTOBSERVER_H
#define CLEANGRADUATOR_IRESULTOBSERVER_H

namespace domain::common {
    class Result;
}

namespace domain::ports {
    struct IResultObserver {
        virtual ~IResultObserver() = default;
        virtual void onResultUpdated(const common::Result& result) = 0;
    };
}

#endif //CLEANGRADUATOR_IRESULTOBSERVER_H