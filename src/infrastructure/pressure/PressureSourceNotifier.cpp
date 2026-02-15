#include "PressureSourceNotifier.h"

#include "../../domain/ports/pressure/IPressureSourceObserver.h"

using namespace infra::pressure::detail;
using namespace domain::ports;
using namespace domain::common;

void PressureSourceNotifier::addObserver(IPressureSourceObserver& observer) {
    observers_.push_back(&observer);
}

void PressureSourceNotifier::removeObserver(IPressureSourceObserver& observer) {
    observers_.erase(
        std::remove(observers_.begin(), observers_.end(), &observer),
        observers_.end()
    );
}

void PressureSourceNotifier::notifyPressure(const PressurePacket& packet) {
    for (auto* o : observers_)
        o->onPressurePacket(packet);
}

void PressureSourceNotifier::notifyOpened() {
    for (auto* o : observers_)
        o->onPressureSourceOpened();
}

void PressureSourceNotifier::notifyOpenFailed(const PressureSourceError& error) {
    for (auto* o : observers_)
        o->onPressureSourceOpenFailed(error);
}

void PressureSourceNotifier::notifyClosed(const PressureSourceError& error) {
    for (auto* o : observers_)
        o->onPressureSourceClosed(error);
}
