#include "VideoSourceViewModel.h"

#include <type_traits>
#include <variant>

#include "domain/core/video/VideoFramePacket.h"
#include "domain/core/video/VideoSourceEvent.h"
#include "domain/ports/video/IVideoSource.h"

#include <algorithm>

using namespace mvvm;

VideoSourceViewModel::VideoSourceViewModel(domain::ports::IVideoSource& video_source)
    : video_source_(video_source)
{
    circle_diameter_percent.set_before_change([](const int&, int& proposed) {
        proposed = std::clamp(proposed, 0, 100);
        return true;
    });

    circle_color1.set_before_change([](const std::uint32_t&, std::uint32_t& proposed) {
        proposed |= 0x000000FFu;
        return true;
    });

    circle_color2.set_before_change([](const std::uint32_t&, std::uint32_t& proposed) {
        proposed |= 0x000000FFu;
        return true;
    });

    video_source_.addObserver(*this);
    video_source_.addSink(*this);
}

VideoSourceViewModel::~VideoSourceViewModel() {
    video_source_.removeSink(*this);
    video_source_.removeObserver(*this);
}

void VideoSourceViewModel::onVideoSourceEvent(const domain::common::VideoSourceEvent &event) {
    std::visit([this](const auto& ev) {
        using T = std::decay_t<decltype(ev)>;

        if constexpr (std::is_same_v<T, domain::common::VideoSourceEvent::Opened>) {
            is_opened.set(true);
            error.set({});
        } else if constexpr (std::is_same_v<T, domain::common::VideoSourceEvent::OpenFailed>
                             || std::is_same_v<T, domain::common::VideoSourceEvent::Failed>) {
            is_opened.set(false);
            error.set(ev.error.reason);
        } else if constexpr (std::is_same_v<T, domain::common::VideoSourceEvent::Closed>) {
            is_opened.set(false);
        }
    }, event.data);
}

void VideoSourceViewModel::onVideoFrame(const domain::common::VideoFramePacket& frame_packet) {
    frame.set(frame_packet.frame);
    error.set({});
}
