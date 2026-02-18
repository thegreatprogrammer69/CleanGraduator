//
// Created by mint on 18.02.2026.
//

#include "DualValveControlViewModel.h"

#include "domain/ports/motor/IDualValveDriver.h"

mvvm::DualValveControlViewModel::DualValveControlViewModel(DualValveControlViewModelDeps deps)
    : dual_valve_driver_(deps.dual_valve_driver)
{
    dual_valve_driver_.addObserver(*this);
}

mvvm::DualValveControlViewModel::~DualValveControlViewModel() {
    dual_valve_driver_.removeObserver(*this);
}

void mvvm::DualValveControlViewModel::openInputFlap() {
    dual_valve_driver_.openInputFlap();
}

void mvvm::DualValveControlViewModel::openOutputFlap() {
    dual_valve_driver_.openOutputFlap();
}

void mvvm::DualValveControlViewModel::closeFlaps() {
    dual_valve_driver_.closeFlaps();
}

void mvvm::DualValveControlViewModel::onInputFlapOpened() {
    flaps_state.set(InputFlapOpened);
}

void mvvm::DualValveControlViewModel::onOutputFlapOpened() {
    flaps_state.set(OutputFlapOpened);
}

void mvvm::DualValveControlViewModel::onFlapsClosed() {
    flaps_state.set(FlapsClosed);
}
