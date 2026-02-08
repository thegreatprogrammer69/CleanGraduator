#include "fmt_domain.h"

#include <chrono>
#include <ostream>

#include "domain/core/measurement/Angle.h"
#include "domain/core/measurement/AngleUnit.h"
#include "domain/core/measurement/Pressure.h"
#include "domain/core/measurement/PressureUnit.h"
#include "domain/core/measurement/Timestamp.h"
#include "domain/core/process/ProcessLifecycleState.h"
#include "domain/core/video/PixelFormat.h"
#include "domain/core/video/VideoBuffer.h"
#include "domain/core/video/VideoFrame.h"


using namespace domain::common;

std::ostream& operator<<(std::ostream& os, const Angle& f) {
    os << f.value() << ' ' << f.unit();
    return os;
}

std::ostream& operator<<(std::ostream& os, const AngleUnit& f) {
    switch (f) {
        case AngleUnit::rad:
            os << "rad";
            break;
        case AngleUnit::deg:
            os << "deg";
            break;
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const Pressure& f) {
    os << f.value() << ' ' << f.unit();
    return os;
}

std::ostream& operator<<(std::ostream& os, const PressureUnit& f) {
    switch (f) {
        case PressureUnit::Pa:
            os << "Pa";
            break;
        case PressureUnit::kPa:
            os << "kPa";
            break;
        case PressureUnit::MPa:
            os << "MPa";
            break;
        case PressureUnit::bar:
            os << "bar";
            break;
        case PressureUnit::atm:
            os << "atm";
            break;
        case PressureUnit::mmHg:
            os << "mmHg";
            break;
        case PressureUnit::mmH2O:
            os << "mmH2O";
            break;
        case PressureUnit::kgf_cm2:
            os << "kgf/cm2";
            break;
        case PressureUnit::kgf_m2:
            os << "kgf/m2";
            break;
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const Timestamp& f) {
    const auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(f.toDuration()).count();
    os << ns << " ns";
    return os;
}

std::ostream& operator<<(std::ostream& os, const PixelFormat& f) {
    switch (f) {
        case PixelFormat::RGB24:
            os << "RGB24";
            break;
        case PixelFormat::YUYV:
            os << "YUYV";
            break;
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const VideoBuffer& f) {
    os << "VideoBuffer{size=" << f.size << ", data=" << static_cast<const void*>(f.data) << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const VideoFrame& f) {
    os << "VideoFrame{width=" << f.width
       << ", height=" << f.height
       << ", format=" << f.format
       << ", buffer=" << f.buffer
       << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const ProcessLifecycleState& f) {
    switch (f) {
        case ProcessLifecycleState::Idle:
            os << "Idle";
            break;
        case ProcessLifecycleState::Forward:
            os << "Forward";
            break;
        case ProcessLifecycleState::Backward:
            os << "Backward";
            break;
        case ProcessLifecycleState::Stopping:
            os << "Stopping";
            break;
    }
    return os;
}
