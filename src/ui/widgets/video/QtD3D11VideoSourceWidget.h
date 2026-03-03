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

        // Чтобы Qt не лез своим painter'ом в native HWND
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

        Microsoft::WRL::ComPtr<ID3D11Texture2D> video_texture_;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> video_srv_;

        bool d3d_inited_{false};
#endif

        bool textureInitialized_{false};
        domain::common::PixelFormat currentFormat_{domain::common::PixelFormat::RGB24};

        int frameWidth_{0};
        int frameHeight_{0};
        int packedWidth_{0}; // для YUYV = width / 2, для RGB = width

        struct alignas(16) ShaderParams {
            unsigned int noVideo;
            unsigned int format;      // 0 = RGBA (из RGB24 repack), 1 = YUYV packed into RGBA8
            unsigned int width;
            unsigned int height;
            unsigned int packedWidth;
            unsigned int _pad0;
            unsigned int _pad1;
            unsigned int _pad2;
        };
    };

}

#endif // PLATFORM_WINDOWS

#endif // CLEANGRADUATOR_QTD3D11VIDEOSOURCEWIDGET_H