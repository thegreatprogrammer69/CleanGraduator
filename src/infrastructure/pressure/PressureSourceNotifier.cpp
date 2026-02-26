#include "PressureSourceNotifier.h"
#include "domain/ports/pressure/IPressureSourceObserver.h"
#include "domain/ports/pressure/IPressureSink.h"

using namespace infra::pressure::detail;
using namespace domain::ports;
using namespace domain::common;

void PressureSourceNotifier::addObserver(IPressureSourceObserver& observer) {
    observers_.add(observer);
}

void PressureSourceNotifier::removeObserver(IPressureSourceObserver& observer) {
    observers_.remove(observer);
}

void PressureSourceNotifier::addSink(IPressureSink &sink) {
    sinks_.add(sink);
}

void PressureSourceNotifier::removeSink(IPressureSink &sink) {
    sinks_.remove(sink);
}

void PressureSourceNotifier::notifyPressure(const PressurePacket& packet) {
    sinks_.notify([&packet](IPressureSink& s) {
        s.onPressure(packet);
    });
}

void PressureSourceNotifier::notifyEvent(const PressureSourceEvent &event) {
    observers_.notify([&event](IPressureSourceObserver& o) {
        o.onPressureSourceEvent(event);
    });
}