#ifndef CLEANGRADUATOR_QTD3D11VIDEOSOURCEWIDGET_H
#define CLEANGRADUATOR_QTD3D11VIDEOSOURCEWIDGET_H

#ifdef PLATFORM_WINDOWS

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcommon.h>
#include <wrl/client.h>
#endif

#include <QWidget>
#include <QPaintEngine>

#include <memory>
#include <mutex>

#include "domain/core/video/PixelFormat.h"
#include "viewmodels/Observable.h"

namespace mvvm {
class VideoSourceViewModel;
}

namespace domain::common {
struct VideoFrame;
using VideoFramePtr = std::shared_ptr<VideoFrame>;
}

namespace ui {

class QtD3D11VideoSourceWidget final : public QWidget
{
public:
    explicit QtD3D11VideoSourceWidget(mvvm::VideoSourceViewModel& model, QWidget* parent = nullptr);
    ~QtD3D11VideoSourceWidget() override;

    QPaintEngine* paintEngine() const override { return nullptr; }

protected:
    void showEvent(QShowEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    void setVideoFrame(domain::common::VideoFramePtr frame);

    bool ensureD3D();
    bool createDevice();
    bool createSwapChain();
    bool createBackBuffer();
    void resizeSwapChain();
    void releaseBackBuffer();
    void releaseVideoTexture();

    bool createShaders();
    static bool compileShader(
        const char* source,
        const char* entry,
        const char* target,
        ID3DBlob** blob);

    bool ensureVideoTexture(const domain::common::VideoFrame& frame);
    bool uploadFrame(const domain::common::VideoFrame& frame);
    void render(bool noVideo);

private:
    std::mutex mutex_;
    domain::common::VideoFramePtr current_frame_;
    mvvm::Observable<domain::common::VideoFramePtr>::Subscription frame_sub_;
    mvvm::Observable<bool>::Subscription is_opened_sub_;

    bool is_source_opened_{false};

#ifdef _WIN32
    HWND hwnd_{nullptr};

    Microsoft::WRL::ComPtr<ID3D11Device> device_;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> context_;
    Microsoft::WRL::ComPtr<IDXGISwapChain> swap_chain_;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> rtv_;

    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertex_shader_;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixel_shader_;
    Microsoft::WRL::ComPtr<ID3D11Buffer> constant_buffer_;
    Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler_;

    // Сырая RGB24 текстура: width = frame.width * 3, format = R8_UNORM
    Microsoft::WRL::ComPtr<ID3D11Texture2D> video_texture_;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> video_srv_;

    bool d3d_inited_{false};
#endif

    bool textureInitialized_{false};
    int frameWidth_{0};
    int frameHeight_{0};
    int textureWidthBytes_{0};

    struct alignas(16) ShaderParams {
        unsigned int noVideo;
        unsigned int width;
        unsigned int height;
        unsigned int textureWidthBytes;
    };
};

} // namespace ui

#endif // PLATFORM_WINDOWS
#endif // CLEANGRADUATOR_QTD3D11VIDEOSOURCEWIDGET_H