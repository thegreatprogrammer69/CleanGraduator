#include "PressureSourceNotifier.h"
#include "../../domain/ports/pressure/IPressureSourceObserver.h"

using namespace infra::pressure::detail;
using namespace domain::ports;
using namespace domain::common;

void PressureSourceNotifier::addObserver(IPressureSourceObserver& observer) {
    std::lock_guard<std::mutex> lock(mutex_);
    observers_.push_back(&observer);
}

void PressureSourceNotifier::removeObserver(IPressureSourceObserver& observer) {
    std::lock_guard<std::mutex> lock(mutex_);
    observers_.erase(
        std::remove(observers_.begin(), observers_.end(), &observer),
        observers_.end()
    );
}

void PressureSourceNotifier::notifyPressure(const PressurePacket& packet) {
    std::vector<IPressureSourceObserver*> copy;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        copy = observers_;
    }

    for (auto* o : copy)
        o->onPressurePacket(packet);
}

void PressureSourceNotifier::notifyOpened() {
    std::vector<IPressureSourceObserver*> copy;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        copy = observers_;
    }

    for (auto* o : copy)
        o->onPressureSourceOpened();
}

void PressureSourceNotifier::notifyOpenFailed(const PressureSourceError& error) {
    std::vector<IPressureSourceObserver*> copy;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        copy = observers_;
    }

    for (auto* o : copy)
        o->onPressureSourceOpenFailed(error);
}

void PressureSourceNotifier::notifyClosed(const PressureSourceError& error) {
    std::vector<IPressureSourceObserver*> copy;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        copy = observers_;
    }

    for (auto* o : copy)
        o->onPressureSourceClosed(error);
}