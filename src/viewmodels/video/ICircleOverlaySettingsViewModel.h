#ifndef CLEANGRADUATOR_ICIRCLEOVERLAYSETTINGSVIEWMODEL_H
#define CLEANGRADUATOR_ICIRCLEOVERLAYSETTINGSVIEWMODEL_H

#include <cstdint>

#include "viewmodels/Observable.h"

namespace mvvm {

class ICircleOverlaySettingsViewModel {
public:
    virtual ~ICircleOverlaySettingsViewModel() = default;

    virtual Observable<int>& circleDiameterPercent() = 0;
    virtual const Observable<int>& circleDiameterPercent() const = 0;

    virtual Observable<std::uint32_t>& circleColor1() = 0;
    virtual const Observable<std::uint32_t>& circleColor1() const = 0;

    virtual Observable<std::uint32_t>& circleColor2() = 0;
    virtual const Observable<std::uint32_t>& circleColor2() const = 0;
};

}

#endif //CLEANGRADUATOR_ICIRCLEOVERLAYSETTINGSVIEWMODEL_H
