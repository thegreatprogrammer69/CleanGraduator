#include "SampleGrabberCB.h"
#include "DShowCamera.h"


namespace infra::camera {

SampleGrabberCB::SampleGrabberCB(DShowCamera *owner): owner_(owner) {}

HRESULT SampleGrabberCB::QueryInterface(const IID &riid, void **ppv) {
    if (riid == IID_ISampleGrabberCB || riid == IID_IUnknown) {
        *ppv = static_cast<ISampleGrabberCB*>(this);
        AddRef();
        return S_OK;
    }
    *ppv = nullptr;
    return E_NOINTERFACE;
}

ULONG SampleGrabberCB::AddRef() {
    return ++ref_count_;
}

ULONG SampleGrabberCB::Release() {
    auto v = --ref_count_;
    if (v == 0) delete this;
    return v;
}

HRESULT SampleGrabberCB::SampleCB(double, IMediaSample *) {
    return E_NOTIMPL;
}

HRESULT SampleGrabberCB::BufferCB(double time, BYTE *buffer, long size) {
    owner_->onFrame(time, buffer, size);
    return S_OK;
}

}
