#ifndef CLEANGRADUATOR_ICIRCLEOVERLAYCONFIGURABLE_H
#define CLEANGRADUATOR_ICIRCLEOVERLAYCONFIGURABLE_H

#include "viewmodels/Observable.h"
#include "CircleOverlaySettings.h"

namespace mvvm {

class ICircleOverlayConfigurable {
public:
    virtual ~ICircleOverlayConfigurable() = default;

    virtual Observable<CircleOverlaySettings>& circleOverlaySettings() = 0;
    virtual const Observable<CircleOverlaySettings>& circleOverlaySettings() const = 0;
};

}

#endif // CLEANGRADUATOR_ICIRCLEOVERLAYCONFIGURABLE_H
