#ifndef CLEANGRADUATOR_ICIRCLEOVERLAYVIEWMODEL_H
#define CLEANGRADUATOR_ICIRCLEOVERLAYVIEWMODEL_H

#include <cstdint>

#include "viewmodels/Observable.h"

namespace mvvm {

class ICircleOverlayViewModel {
public:
    virtual ~ICircleOverlayViewModel() = default;

    virtual Observable<float>& circleDiameterPercent() = 0;
    virtual Observable<std::uint32_t>& circleColor1() = 0;
    virtual Observable<std::uint32_t>& circleColor2() = 0;
};

}

#endif //CLEANGRADUATOR_ICIRCLEOVERLAYVIEWMODEL_H
