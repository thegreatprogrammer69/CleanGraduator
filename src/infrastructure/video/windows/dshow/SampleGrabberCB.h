#ifndef CLEANGRADUATOR_SAMPLEGRABBERCB_H
#define CLEANGRADUATOR_SAMPLEGRABBERCB_H
#include "QEDIT.H"
#include <atomic>

namespace infra::camera {

class DShowCamera;

class SampleGrabberCB final : public ISampleGrabberCB {
public:
    explicit SampleGrabberCB(DShowCamera* owner);

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, void** ppv) override;

    STDMETHODIMP_(ULONG) AddRef() override;

    STDMETHODIMP_(ULONG) Release() override;

    // Нам не нужен SampleCB
    STDMETHODIMP SampleCB(double, IMediaSample*) override;

    // Основной callback
    STDMETHODIMP BufferCB(double time, BYTE* buffer, long size) override;

private:
    std::atomic<ULONG> ref_count_{1};
    DShowCamera* owner_;
};

}

#endif //CLEANGRADUATOR_SAMPLEGRABBERCB_H
