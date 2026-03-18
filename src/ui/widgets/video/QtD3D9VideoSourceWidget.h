#ifndef CLEANGRADUATOR_QTD3D9VIDEOSOURCEWIDGET_H
#define CLEANGRADUATOR_QTD3D9VIDEOSOURCEWIDGET_H
#ifdef PLATFORM_WINDOWS

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <d3d9.h>
#include <wrl/client.h>
#endif

#include <d3dcommon.h>
#include <QWidget>
#include <QPaintEngine>

#include <memory>
#include <mutex>

#include "domain/core/video/PixelFormat.h"
#include "viewmodels/Observable.h"

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dcompiler.lib")

namespace mvvm {
    class VideoSourceViewModel;
}

namespace domain::common {
    struct VideoFrame;
    using VideoFramePtr = std::shared_ptr<VideoFrame>;
}

namespace ui {

    class QtD3D9VideoSourceWidget final : public QWidget
    {
    public:
        explicit QtD3D9VideoSourceWidget(mvvm::VideoSourceViewModel& model, QWidget* parent = nullptr);
        ~QtD3D9VideoSourceWidget() override;

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

        Microsoft::WRL::ComPtr<IDirect3D9> d3d_;
        Microsoft::WRL::ComPtr<IDirect3DDevice9> device_;
        Microsoft::WRL::ComPtr<IDirect3DSurface9> back_buffer_;

        Microsoft::WRL::ComPtr<IDirect3DPixelShader9> pixel_shader_;

        Microsoft::WRL::ComPtr<IDirect3DTexture9> video_texture_;

        D3DPRESENT_PARAMETERS present_params_{};
        bool d3d_inited_{false};
#endif

        bool textureInitialized_{false};
        domain::common::PixelFormat currentFormat_{domain::common::PixelFormat::RGB24};

        int frameWidth_{0};
        int frameHeight_{0};
        int packedWidth_{0}; // для YUYV = width / 2, для RGB = width

        struct alignas(16) ShaderParams {
            unsigned int noVideo;
            unsigned int format;      // 0 = RGBA (из RGB24 repack), 1 = YUYV packed into A8R8G8B8
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

#endif // CLEANGRADUATOR_QTD3D9VIDEOSOURCEWIDGET_H