#ifndef CLEANGRADUATOR_IDUALVALVEDRIVEROBSERVER_H
#define CLEANGRADUATOR_IDUALVALVEDRIVEROBSERVER_H

namespace domain::ports {

    struct IValveDriverObserver {
        virtual ~IValveDriverObserver() = default;

        virtual void onInputFlapOpened() = 0;
        virtual void onOutputFlapOpened() = 0;
        virtual void onFlapsClosed() = 0;
    };

}


#endif //CLEANGRADUATOR_IDUALVALVEDRIVEROBSERVER_H