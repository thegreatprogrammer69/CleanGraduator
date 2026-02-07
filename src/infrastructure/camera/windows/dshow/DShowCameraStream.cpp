#include <dshow.h>
#include <windows.h>
#include <wrl/client.h>
#include <atlbase.h>
#include <atlcom.h>
#pragma comment(lib, "strmiids.lib")

#include "../DShowCameraStream.h"
#include "../SampleGrabberCB.h"

namespace infra::camera {

static CComPtr<IBaseFilter> CreateCaptureFilterByIndex(int index);

struct DShowCameraStream::DShowCameraStreamImpl {
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

    ~DShowCameraStreamImpl() {
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
            CoUninitialize();
        }
    }
};

DShowCameraStream::DShowCameraStream(const CameraPorts& ports, const CameraConfig& config)
    : impl_(std::make_unique<DShowCameraStreamImpl>())
    , config_(config)
    , ports_(ports)
{
}

DShowCameraStream::~DShowCameraStream() = default;

void DShowCameraStream::addSink(IVideoSink &sink) {
    sinks_.push_back(&sink);
}

void DShowCameraStream::removeSink(IVideoSink &sink) {
    sinks_.erase(std::remove(sinks_.begin(), sinks_.end(), &sink));
}

bool DShowCameraStream::start() {
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hr)) return false;
    impl_->com_initialized = true;

    // Graph
    hr = impl_->graph.CoCreateInstance(CLSID_FilterGraph);
    if (FAILED(hr)) return false;

    // CaptureGraphBuilder2
    hr = impl_->capture.CoCreateInstance(CLSID_CaptureGraphBuilder2);
    if (FAILED(hr)) return false;

    hr = impl_->capture->SetFiltergraph(impl_->graph);
    if (FAILED(hr)) return false;

    impl_->graph.QueryInterface(&impl_->control);

    // Source filter
    impl_->source_filter = CreateCaptureFilterByIndex(index_);
    if (!impl_->source_filter) return false;

    hr = impl_->graph->AddFilter(impl_->source_filter, L"Video Source");
    if (FAILED(hr)) return false;

    // SampleGrabber
    hr = impl_->grabber_filter.CoCreateInstance(CLSID_SampleGrabber);
    if (FAILED(hr)) return false;

    hr = impl_->grabber_filter.QueryInterface(&impl_->grabber);
    if (FAILED(hr)) return false;

    AM_MEDIA_TYPE mt{};
    mt.majortype  = MEDIATYPE_Video;
    mt.subtype    = MEDIASUBTYPE_RGB24;
    mt.formattype = FORMAT_VideoInfo;
    mt.cbFormat   = sizeof(VIDEOINFOHEADER);
    mt.pbFormat   = (BYTE*)CoTaskMemAlloc(mt.cbFormat);

    hr = impl_->grabber->SetMediaType(&mt);
    if (FAILED(hr)) return false;

    hr = impl_->graph->AddFilter(impl_->grabber_filter, L"SampleGrabber");
    if (FAILED(hr)) return false;

    // Null Renderer
    hr = impl_->null_renderer.CoCreateInstance(CLSID_NullRenderer);
    if (FAILED(hr)) return false;

    hr = impl_->graph->AddFilter(impl_->null_renderer, L"NullRenderer");
    if (FAILED(hr)) return false;

    // RenderStream
    hr = impl_->capture->RenderStream(
        &PIN_CATEGORY_CAPTURE,
        &MEDIATYPE_Video,
        impl_->source_filter,
        impl_->grabber_filter,
        impl_->null_renderer
    );
    if (FAILED(hr)) return false;

    // Frame size
    AM_MEDIA_TYPE mt_out{};
    hr = impl_->grabber->GetConnectedMediaType(&mt_out);
    if (SUCCEEDED(hr) && mt_out.formattype == FORMAT_VideoInfo) {
        auto vih = reinterpret_cast<VIDEOINFOHEADER*>(mt_out.pbFormat);
        impl_->width  = vih->bmiHeader.biWidth;
        impl_->height = vih->bmiHeader.biHeight;
    }

    impl_->grabber->SetBufferSamples(FALSE);
    impl_->grabber->SetOneShot(FALSE);

    impl_->grabber_cb = new SampleGrabberCB(this);
    impl_->grabber->SetCallback(impl_->grabber_cb, 1);

    return SUCCEEDED(impl_->control->Run());
}

void DShowCameraStream::stop() {
    impl_.reset(); // всё убьётся в деструкторе impl
}

void DShowCameraStream::onFrame(double time, BYTE* data, long size) {
    auto frame = std::make_shared<VideoFrame>();
    frame->width  = impl_->width;
    frame->height = impl_->height;
    frame->format = PixelFormat::RGB24;
    frame->buffer = VideoBuffer(size);

    std::memcpy(frame->buffer.data, data, size);

    const time_point_t ts = ports_.clock.now();

    std::vector<IVideoSink*> sinks_copy;
    {
        std::lock_guard lock(sinks_mutex_);
        sinks_copy = sinks_;
    }

    for (auto* sink : sinks_copy) {
        sink->onVideoFrame(ts, frame);
    }
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

}