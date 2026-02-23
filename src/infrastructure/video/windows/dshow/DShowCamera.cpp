#include <dshow.h>
#include <windows.h>
#include <wrl/client.h>
#include <atlbase.h>
#include <atlcom.h>
#pragma comment(lib, "strmiids.lib")

#include "DShowCamera.h"
#include "SampleGrabberCB.h"
#include "domain/core/video/VideoFrame.h"
#include "domain/core/video/VideoFramePacket.h"
#include "domain/core/video/VideoSourceError.h"
#include "domain/ports/clock/IClock.h"

using namespace infra::camera;

static CComPtr<IBaseFilter> CreateCaptureFilterByIndex(int index);

struct DShowCamera::DShowCameraImpl {
    // COM
    CComPtr<IGraphBuilder> graph;
    CComPtr<ICaptureGraphBuilder2> capture;
    CComPtr<IMediaControl> control;

    CComPtr<IBaseFilter> source_filter;
    CComPtr<IBaseFilter> grabber_filter;
    CComPtr<ISampleGrabber> grabber;
    CComPtr<IBaseFilter> null_renderer;

    SampleGrabberCB* grabber_cb = nullptr;

    int width  = 0;
    int height = 0;

    bool com_initialized = false;

    ~DShowCameraImpl() {
        if (control) {
            control->Stop();
        }

        if (grabber_cb) {
            grabber_cb->Release();
            grabber_cb = nullptr;
        }

        graph.Release();
        capture.Release();
        control.Release();
        grabber.Release();
        grabber_filter.Release();
        source_filter.Release();
        null_renderer.Release();

        if (com_initialized) {
            // CoUninitialize();
        }
    }
};

DShowCamera::DShowCamera(VideoSourcePorts ports, DShowCameraConfig config)
    : logger_(ports.logger)
    , clock_(ports.clock)
    , config_(config)
{
    logger_.info("DShowCamera constructor called: camera index={}", config_.index);
}

DShowCamera::~DShowCamera() {
    logger_.info("DShowCamera destructor called");
}

bool DShowCamera::open() {
    auto abort_opening = [this]() {
        const domain::common::VideoSourceError err {this->logger_.lastError()};
        notifier_.notifyFailed(err);
    };

    logger_.info("DShowCamera::start() begin");

    impl_ = std::make_unique<DShowCameraImpl>();

    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_SPEED_OVER_MEMORY);
    if (FAILED(hr)) {
        logger_.error("CoInitializeEx failed: hr=0x{:08X}", hr);
        abort_opening(); return false;
    }
    impl_->com_initialized = true;
    logger_.info("COM initialized");

    hr = impl_->graph.CoCreateInstance(CLSID_FilterGraph);
    if (FAILED(hr)) {
        logger_.error("Create FilterGraph failed: hr=0x{:08X}", hr);
        abort_opening(); return false;
    }

    hr = impl_->capture.CoCreateInstance(CLSID_CaptureGraphBuilder2);
    if (FAILED(hr)) {
        logger_.error("Create CaptureGraphBuilder2 failed: hr=0x{:08X}", hr);
        abort_opening(); return false;
    }

    hr = impl_->capture->SetFiltergraph(impl_->graph);
    if (FAILED(hr)) {
        logger_.error("SetFiltergraph failed: hr=0x{:08X}", hr);
        abort_opening(); return false;
    }

    impl_->graph.QueryInterface(&impl_->control);
    logger_.info("Graph and control interfaces created");

    impl_->source_filter = CreateCaptureFilterByIndex(config_.index);
    if (!impl_->source_filter) {
        logger_.error("Device not found, index={}", config_.index);
        abort_opening(); return false;
    }

    hr = impl_->graph->AddFilter(impl_->source_filter, L"Video Source");
    if (FAILED(hr)) {
        logger_.error("Add source filter failed: hr=0x{:08X}", hr);
        abort_opening(); return false;
    }

    hr = impl_->grabber_filter.CoCreateInstance(CLSID_SampleGrabber);
    if (FAILED(hr)) {
        logger_.error("Create SampleGrabber filter failed: hr=0x{:08X}", hr);
        abort_opening(); return false;
    }

    hr = impl_->grabber_filter.QueryInterface(&impl_->grabber);
    if (FAILED(hr)) {
        logger_.error("Query SampleGrabber interface failed: hr=0x{:08X}", hr);
        abort_opening(); return false;
    }

    AM_MEDIA_TYPE mt{};
    mt.majortype  = MEDIATYPE_Video;
    mt.subtype    = MEDIASUBTYPE_RGB24;
    mt.formattype = FORMAT_VideoInfo;
    mt.cbFormat   = sizeof(VIDEOINFOHEADER);
    mt.pbFormat   = (BYTE*)CoTaskMemAlloc(mt.cbFormat);

    hr = impl_->grabber->SetMediaType(&mt);
    if (FAILED(hr)) {
        logger_.error("SetMediaType failed: hr=0x{:08X}", hr);
        abort_opening(); return false;
    }

    hr = impl_->graph->AddFilter(impl_->grabber_filter, L"SampleGrabber");
    if (FAILED(hr)) {
        logger_.error("Add SampleGrabber failed: hr=0x{:08X}", hr);
        abort_opening(); return false;
    }

    hr = impl_->null_renderer.CoCreateInstance(CLSID_NullRenderer);
    if (FAILED(hr)) {
        logger_.error("Create NullRenderer failed: hr=0x{:08X}", hr);
        abort_opening(); return false;
    }

    hr = impl_->graph->AddFilter(impl_->null_renderer, L"NullRenderer");
    if (FAILED(hr)) {
        logger_.error("Add NullRenderer failed: hr=0x{:08X}", hr);
        abort_opening(); return false;
    }

    hr = impl_->capture->RenderStream(
        &PIN_CATEGORY_CAPTURE,
        &MEDIATYPE_Video,
        impl_->source_filter,
        impl_->grabber_filter,
        impl_->null_renderer
    );
    if (FAILED(hr)) {
        logger_.error("RenderStream failed: hr=0x{:08X}", hr);
        abort_opening(); return false;
    }

    AM_MEDIA_TYPE mt_out{};
    hr = impl_->grabber->GetConnectedMediaType(&mt_out);
    if (SUCCEEDED(hr) && mt_out.formattype == FORMAT_VideoInfo) {
        auto vih = reinterpret_cast<VIDEOINFOHEADER*>(mt_out.pbFormat);
        impl_->width  = vih->bmiHeader.biWidth;
        impl_->height = vih->bmiHeader.biHeight;

        logger_.info("Video format: {}x{}", impl_->width, impl_->height);
    } else {
        logger_.warn("Failed to get connected media type");
    }

    impl_->grabber->SetBufferSamples(FALSE);
    impl_->grabber->SetOneShot(FALSE);

    impl_->grabber_cb = new SampleGrabberCB(this);
    impl_->grabber->SetCallback(impl_->grabber_cb, 1);

    hr = impl_->control->Run();
    if (FAILED(hr)) {
        logger_.error("Graph Run failed: hr=0x{:08X}", hr);
        abort_opening(); return false;
    }

    logger_.info("DShowCamera started successfully");

    notifier_.notifyOpened();
    return true;
}

void DShowCamera::close() {
    logger_.info("DShowCamera::stop()");
    impl_.reset();
    logger_.info("DShowCamera stopped");
    notifier_.notifyClosed();
}

void DShowCamera::addObserver(domain::ports::IVideoSourceObserver &o) {
    notifier_.addObserver(o);
}

void DShowCamera::removeObserver(domain::ports::IVideoSourceObserver &o) {
    notifier_.removeObserver(o);
}

void DShowCamera::onFrame(double time, BYTE* data, long size) {
    using namespace domain::common;

    auto frame = std::make_shared<VideoFrame>();
    frame->width  = impl_->width;
    frame->height = impl_->height;
    frame->format = PixelFormat::RGB24;
    frame->buffer = VideoBuffer(size);

    std::memcpy(frame->buffer.data, data, size);

    const auto ts = clock_.now();

    VideoFramePacket packet;
    packet.frame = frame;
    packet.timestamp = ts;

    notifier_.notifyFrame(packet);
}


static CComPtr<IBaseFilter> CreateCaptureFilterByIndex(int index) {
    CComPtr<ICreateDevEnum> dev_enum;
    CComPtr<IEnumMoniker> enum_moniker;

    HRESULT hr = dev_enum.CoCreateInstance(CLSID_SystemDeviceEnum);
    if (FAILED(hr)) return nullptr;

    hr = dev_enum->CreateClassEnumerator(
        CLSID_VideoInputDeviceCategory,
        &enum_moniker,
        0
    );
    if (FAILED(hr) || hr == S_FALSE) return nullptr;

    ULONG fetched = 0;
    CComPtr<IMoniker> moniker;
    int i = 0;

    while (enum_moniker->Next(1, &moniker, &fetched) == S_OK) {
        if (i == index) {
            CComPtr<IBaseFilter> filter;
            if (SUCCEEDED(moniker->BindToObject(
                nullptr, nullptr, IID_IBaseFilter, (void**)&filter))) {
                return filter;
                }
            break;
        }
        moniker.Release();
        ++i;
    }

    return nullptr;
}
