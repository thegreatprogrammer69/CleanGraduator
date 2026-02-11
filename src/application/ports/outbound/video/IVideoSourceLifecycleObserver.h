#ifndef CLEANGRADUATOR_IVIDEOSOURCELIFECYCLE_H
#define CLEANGRADUATOR_IVIDEOSOURCELIFECYCLE_H

namespace application::ports {
    struct IVideoSourceLifecycleObserver {
        virtual void onSourceOpened() = 0;
        virtual void onSourceClosed() = 0;
        virtual ~IVideoSourceLifecycleObserver() = default;
    };
}


#endif //CLEANGRADUATOR_IVIDEOSOURCELIFECYCLE_H