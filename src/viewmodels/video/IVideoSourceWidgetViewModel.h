#ifndef CLEANGRADUATOR_IVIDEOSOURCEWIDGETVIEWMODEL_H
#define CLEANGRADUATOR_IVIDEOSOURCEWIDGETVIEWMODEL_H

#include <string>

#include "domain/core/video/VideoFrame.h"
#include "viewmodels/Observable.h"
#include "viewmodels/video/ICircleOverlayConfigurable.h"

namespace mvvm {

class IVideoSourceWidgetViewModel : public ICircleOverlayConfigurable {
public:
    ~IVideoSourceWidgetViewModel() override = default;

    virtual Observable<domain::common::VideoFramePtr>& frameStream() = 0;
    virtual const Observable<domain::common::VideoFramePtr>& frameStream() const = 0;

    virtual Observable<bool>& openedState() = 0;
    virtual const Observable<bool>& openedState() const = 0;

    virtual Observable<std::string>& errorState() = 0;
    virtual const Observable<std::string>& errorState() const = 0;
};

}

#endif //CLEANGRADUATOR_IVIDEOSOURCEWIDGETVIEWMODEL_H
