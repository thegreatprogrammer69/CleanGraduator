#ifndef CLEANGRADUATOR_QTVIDEOSOURCEADAPTER_H
#define CLEANGRADUATOR_QTVIDEOSOURCEADAPTER_H
#include "domain/ports/outbound/IVideoSink.h"
#include <QObject>
#include <mutex>

namespace domain::ports {
    struct IVideoSource;
}

namespace ui::adapters {
    class QtVideoSourceAdapter final : public QObject, domain::ports::IVideoSink {
        Q_OBJECT
    public:
        explicit QtVideoSourceAdapter(domain::ports::IVideoSource& video_source);
        ~QtVideoSourceAdapter() override;

        domain::common::VideoFramePtr lastVideoFrame() const;

        signals:
            void videoFrameReady();

    private: // IVideoSink
        void onVideoFrame(const domain::common::Timestamp &, domain::common::VideoFramePtr) override;

    private:
        mutable std::mutex mutex_;
        domain::common::VideoFramePtr latest_frame_;
        domain::ports::IVideoSource& video_source_;
    };
}

#endif //CLEANGRADUATOR_QTVIDEOSOURCEADAPTER_H