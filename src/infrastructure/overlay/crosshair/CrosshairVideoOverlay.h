#ifndef CLEANGRADUATOR_CROSSHAIRVIDEOOVERLAY_H
#define CLEANGRADUATOR_CROSSHAIRVIDEOOVERLAY_H
#include "domain/ports/inbound/IVideoSourceOverlay.h"
#include "CrosshairVideoOverlayConfig.h"

namespace infra::overlay {
    class CrosshairVideoOverlay final : public domain::ports::IVideoSourceOverlay{
    public:
        explicit CrosshairVideoOverlay(CrosshairVideoOverlayConfig config);
        ~CrosshairVideoOverlay() override = default;
        domain::common::VideoFramePacket draw(domain::common::VideoFramePacket) const override;
    private:
        CrosshairVideoOverlayConfig config_;
    };
}


#endif //CLEANGRADUATOR_CROSSHAIRVIDEOOVERLAY_H