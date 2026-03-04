#include <algorithm>
#include <cstring>
#include <iterator>
#include <mutex>
#include <qglobal.h>
#include <QMetaObject>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QShowEvent>
#include <qtextstream.h>

#include "app/ControlPanelWidget.h"
#include "domain/core/video/PixelFormat.h"
#include "domain/core/video/VideoFrame.h"
#include "viewmodels/video/VideoSourceViewModel.h"
#ifdef PLATFORM_WINDOWS

#include "QtD3D11VideoSourceWidget.h"

#ifndef _WIN32
#error This implementation requires Windows + Direct3D 11
#endif

#include <QMetaObject>
#include <QShowEvent>
#include <QResizeEvent>
#include <QPaintEvent>
#include <QDebug>

#include <d3dcompiler.h>

#include <algorithm>
#include <cstring>

#include "domain/core/video/VideoFrame.h"
#include "viewmodels/video/VideoSourceViewModel.h"

using namespace ui;
using namespace domain::common;
using Microsoft::WRL::ComPtr;

namespace {

static constexpr const char* kVertexShaderHlsl = R"(
struct VSOut
{
    float4 pos : SV_POSITION;
    float2 uv  : TEXCOORD0;
};

VSOut main(uint vertexId : SV_VertexID)
{
    VSOut o;

    float2 positions[4] = {
        float2(-1.0, -1.0),
        float2(-1.0,  1.0),
        float2( 1.0, -1.0),
        float2( 1.0,  1.0)
    };

    float2 uvs[4] = {
        float2(0.0, 1.0),
        float2(0.0, 0.0),
        float2(1.0, 1.0),
        float2(1.0, 0.0)
    };

    o.pos = float4(positions[vertexId], 0.0, 1.0);
    o.uv  = uvs[vertexId];
    return o;
}
)";

static constexpr const char* kPixelShaderHlsl = R"(
Texture2D    gVideoTex : register(t0);
SamplerState gSampler  : register(s0);

cbuffer Params : register(b0)
{
    uint uNoVideo;
    uint uFormat;      // 0 = RGBA, 1 = YUYV packed
    uint uWidth;
    uint uHeight;
    uint uPackedWidth;
    uint _pad0;
    uint _pad1;
    uint _pad2;
};

struct VSOut
{
    float4 pos : SV_POSITION;
    float2 uv  : TEXCOORD0;
};

float3 yuvToRgb(float y, float u, float v)
{
    // BT.601 limited range (похоже на то, что обычно приходит с камер YUYV)
    float Y = max(y - 16.0 / 255.0, 0.0) * 1.16438356;
    float U = u - 0.5;
    float V = v - 0.5;

    float r = Y + 1.59602678 * V;
    float g = Y - 0.39176229 * U - 0.81296764 * V;
    float b = Y + 2.01723214 * U;

    return saturate(float3(r, g, b));
}

float4 main(VSOut i) : SV_TARGET
{
    if (uNoVideo != 0)
    {
        return float4(0.05, 0.05, 0.05, 1.0);
    }

    float2 uv = saturate(i.uv);

    // 0 = уже обычная RGBA8 текстура
    if (uFormat == 0)
    {
        return gVideoTex.Sample(gSampler, uv);
    }

    // 1 = YUYV packed into RGBA8:
    // texel.r = Y0, texel.g = U, texel.b = Y1, texel.a = V
    float fw = max((float)uWidth, 1.0);
    float fh = max((float)uHeight, 1.0);
    float fpw = max((float)uPackedWidth, 1.0);

    float px = min(uv.x * fw, fw - 1.0);
    float py = min(uv.y * fh, fh - 1.0);

    uint ix = (uint)px;
    float packedX = (floor(px * 0.5) + 0.5) / fpw;
    float packedY = (floor(py) + 0.5) / fh;

    float4 s = gVideoTex.Sample(gSampler, float2(packedX, packedY));

    float y = ((ix & 1u) == 0u) ? s.r : s.b;
    float u = s.g;
    float v = s.a;

    return float4(yuvToRgb(y, u, v), 1.0);
}
)";

} // namespace

QtD3D11VideoSourceWidget::QtD3D11VideoSourceWidget(mvvm::VideoSourceViewModel& model, QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_NativeWindow, true);
    setAttribute(Qt::WA_PaintOnScreen, true);
    setAttribute(Qt::WA_NoSystemBackground, true);
    setAutoFillBackground(false);

    // Если Observable может дергать callback не из UI thread —
    // это безопаснее, чем прямой вызов QWidget::update() из worker thread.
    frame_sub_ = model.frame.subscribe([this](const auto& a) {
        const auto frame = a.new_value;
        QMetaObject::invokeMethod(
            this,
            [this, frame]() { setVideoFrame(frame); },
            Qt::QueuedConnection);
    });

    is_opened_sub_ = model.is_opened.subscribe([this](const auto& a) {
        const bool opened = a.new_value;
        QMetaObject::invokeMethod(
            this,
            [this, opened]() {
                std::lock_guard lock(mutex_);
                is_source_opened_ = opened;
                if (!is_source_opened_) {
                    current_frame_.reset();
                }
                update();
            },
            Qt::QueuedConnection);
    });
}

QtD3D11VideoSourceWidget::~QtD3D11VideoSourceWidget() {
    releaseVideoTexture();
    releaseBackBuffer();
    swap_chain_.Reset();
    constant_buffer_.Reset();
    sampler_.Reset();
    pixel_shader_.Reset();
    vertex_shader_.Reset();
    context_.Reset();
    device_.Reset();
}

void QtD3D11VideoSourceWidget::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    ensureD3D();
    update();
}

void QtD3D11VideoSourceWidget::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    if (d3d_inited_) {
        resizeSwapChain();
    }
}

void QtD3D11VideoSourceWidget::paintEvent(QPaintEvent* event) {
    QWidget::paintEvent(event);

    if (!ensureD3D()) {
        return;
    }

    VideoFramePtr frame;
    bool isOpened = false;
    {
        std::lock_guard lock(mutex_);
        frame = current_frame_;
        isOpened = is_source_opened_;
    }

    bool hasRenderableFrame = false;
    if (frame && (frame->format == PixelFormat::RGB24 || frame->format == PixelFormat::YUYV)) {
        if (ensureVideoTexture(*frame)) {
            hasRenderableFrame = uploadFrame(*frame);
        }
    }

    render(!isOpened || !hasRenderableFrame);
}

void QtD3D11VideoSourceWidget::setVideoFrame(VideoFramePtr frame) {
    std::lock_guard lock(mutex_);
    current_frame_ = std::move(frame);
    update();
}

bool QtD3D11VideoSourceWidget::ensureD3D() {
    if (d3d_inited_) {
        return true;
    }

    hwnd_ = reinterpret_cast<HWND>(winId());
    if (!hwnd_) {
        return false;
    }

    if (!createDevice()) {
        return false;
    }
    if (!createSwapChain()) {
        return false;
    }
    if (!createBackBuffer()) {
        return false;
    }
    if (!createShaders()) {
        return false;
    }

    d3d_inited_ = true;
    return true;
}

bool QtD3D11VideoSourceWidget::createDevice() {
    UINT flags = 0;
#ifndef NDEBUG
    // Для старых машин debug layer часто не установлен.
    // Если включать — делать это опционально.
    // flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    const D3D_FEATURE_LEVEL levels[] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3
    };

    D3D_FEATURE_LEVEL createdLevel = D3D_FEATURE_LEVEL_9_3;

    const HRESULT hr = D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        flags,
        levels,
        static_cast<UINT>(std::size(levels)),
        D3D11_SDK_VERSION,
        &device_,
        &createdLevel,
        &context_);

    if (FAILED(hr) || !device_ || !context_) {
        qWarning() << "D3D11CreateDevice failed, hr =" << Qt::hex << static_cast<quint32>(hr);
        return false;
    }

    return true;
}

bool QtD3D11VideoSourceWidget::createSwapChain() {
    ComPtr<IDXGIDevice> dxgiDevice;
    if (FAILED(device_.As(&dxgiDevice)) || !dxgiDevice) {
        return false;
    }

    ComPtr<IDXGIAdapter> adapter;
    if (FAILED(dxgiDevice->GetAdapter(&adapter)) || !adapter) {
        return false;
    }

    ComPtr<IDXGIFactory> factory;
    if (FAILED(adapter->GetParent(IID_PPV_ARGS(&factory))) || !factory) {
        return false;
    }

    const float dpr = devicePixelRatioF();
    const UINT w = static_cast<UINT>(std::max(1, static_cast<int>(width() * dpr)));
    const UINT h = static_cast<UINT>(std::max(1, static_cast<int>(height() * dpr)));

    DXGI_SWAP_CHAIN_DESC desc = {};
    desc.BufferDesc.Width = w;
    desc.BufferDesc.Height = h;
    desc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.BufferDesc.RefreshRate.Numerator = 0;
    desc.BufferDesc.RefreshRate.Denominator = 1;

    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;

    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.BufferCount = 1; // Win7-safe
    desc.OutputWindow = hwnd_;
    desc.Windowed = TRUE;
    desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    desc.Flags = 0;

    const HRESULT hr = factory->CreateSwapChain(device_.Get(), &desc, &swap_chain_);
    if (FAILED(hr) || !swap_chain_) {
        qWarning() << "CreateSwapChain failed, hr =" << Qt::hex << static_cast<quint32>(hr);
        return false;
    }

    factory->MakeWindowAssociation(hwnd_, DXGI_MWA_NO_ALT_ENTER);
    return true;
}

bool QtD3D11VideoSourceWidget::createBackBuffer() {
    if (!swap_chain_) {
        return false;
    }

    ComPtr<ID3D11Texture2D> backBuffer;
    const HRESULT hr = swap_chain_->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
    if (FAILED(hr) || !backBuffer) {
        qWarning() << "GetBuffer(backBuffer) failed, hr =" << Qt::hex << static_cast<quint32>(hr);
        return false;
    }

    if (FAILED(device_->CreateRenderTargetView(backBuffer.Get(), nullptr, &rtv_)) || !rtv_) {
        qWarning() << "CreateRenderTargetView failed";
        return false;
    }

    return true;
}

void QtD3D11VideoSourceWidget::releaseBackBuffer() {
    if (context_) {
        context_->OMSetRenderTargets(0, nullptr, nullptr);
    }
    rtv_.Reset();
}

void QtD3D11VideoSourceWidget::resizeSwapChain() {
    if (!swap_chain_) {
        return;
    }

    const float dpr = devicePixelRatioF();
    const UINT w = static_cast<UINT>(std::max(1, static_cast<int>(width() * dpr)));
    const UINT h = static_cast<UINT>(std::max(1, static_cast<int>(height() * dpr)));

    releaseBackBuffer();

    const HRESULT hr = swap_chain_->ResizeBuffers(
        0,
        w,
        h,
        DXGI_FORMAT_UNKNOWN,
        0);

    if (FAILED(hr)) {
        qWarning() << "ResizeBuffers failed, hr =" << Qt::hex << static_cast<quint32>(hr);
        return;
    }

    createBackBuffer();
}

void QtD3D11VideoSourceWidget::releaseVideoTexture() {
    video_srv_.Reset();
    video_texture_.Reset();
    textureInitialized_ = false;
    frameWidth_ = 0;
    frameHeight_ = 0;
    packedWidth_ = 0;
}

bool QtD3D11VideoSourceWidget::compileShader(
    const char* source,
    const char* entry,
    const char* target,
    ID3DBlob** blob)
{
    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifndef NDEBUG
    flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    ComPtr<ID3DBlob> errors;
    const HRESULT hr = D3DCompile(
        source,
        std::strlen(source),
        nullptr,
        nullptr,
        nullptr,
        entry,
        target,
        flags,
        0,
        blob,
        &errors);

    if (FAILED(hr)) {
        if (errors) {
            qWarning() << "D3DCompile:" << reinterpret_cast<const char*>(errors->GetBufferPointer());
        }
        return false;
    }

    return true;
}

bool QtD3D11VideoSourceWidget::createShaders() {
    if (vertex_shader_ && pixel_shader_ && constant_buffer_ && sampler_) {
        return true;
    }

    ComPtr<ID3DBlob> vsBlob;
    ComPtr<ID3DBlob> psBlob;

    if (!compileShader(kVertexShaderHlsl, "main", "vs_4_0", &vsBlob)) {
        return false;
    }
    if (!compileShader(kPixelShaderHlsl, "main", "ps_4_0", &psBlob)) {
        return false;
    }

    if (FAILED(device_->CreateVertexShader(
            vsBlob->GetBufferPointer(),
            vsBlob->GetBufferSize(),
            nullptr,
            &vertex_shader_))) {
        return false;
    }

    if (FAILED(device_->CreatePixelShader(
            psBlob->GetBufferPointer(),
            psBlob->GetBufferSize(),
            nullptr,
            &pixel_shader_))) {
        return false;
    }

    D3D11_BUFFER_DESC cbDesc = {};
    cbDesc.ByteWidth = sizeof(ShaderParams);
    cbDesc.Usage = D3D11_USAGE_DEFAULT;
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    if (FAILED(device_->CreateBuffer(&cbDesc, nullptr, &constant_buffer_))) {
        return false;
    }

    D3D11_SAMPLER_DESC sampDesc = {};
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.MinLOD = 0.0f;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

    if (FAILED(device_->CreateSamplerState(&sampDesc, &sampler_))) {
        return false;
    }

    return true;
}

bool QtD3D11VideoSourceWidget::ensureVideoTexture(const VideoFrame& frame) {
    const bool isYuyv = (frame.format == PixelFormat::YUYV);

    if (isYuyv && (frame.width % 2) != 0) {
        return false;
    }

    const int desiredPackedWidth = isYuyv ? (frame.width / 2) : frame.width;
    const int desiredHeight = frame.height;

    if (textureInitialized_
        && currentFormat_ == frame.format
        && packedWidth_ == desiredPackedWidth
        && frameHeight_ == desiredHeight
        && frameWidth_ == frame.width) {
        return true;
    }

    releaseVideoTexture();

    D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width = static_cast<UINT>(desiredPackedWidth);
    texDesc.Height = static_cast<UINT>(desiredHeight);
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc.SampleDesc.Count = 1;
    texDesc.Usage = D3D11_USAGE_DYNAMIC;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    if (FAILED(device_->CreateTexture2D(&texDesc, nullptr, &video_texture_)) || !video_texture_) {
        return false;
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = texDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;

    if (FAILED(device_->CreateShaderResourceView(video_texture_.Get(), &srvDesc, &video_srv_)) || !video_srv_) {
        releaseVideoTexture();
        return false;
    }

    textureInitialized_ = true;
    currentFormat_ = frame.format;
    frameWidth_ = frame.width;
    frameHeight_ = frame.height;
    packedWidth_ = desiredPackedWidth;

    return true;
}

bool QtD3D11VideoSourceWidget::uploadFrame(const VideoFrame& frame) {
    if (!video_texture_) {
        return false;
    }

    D3D11_MAPPED_SUBRESOURCE mapped = {};
    const HRESULT hr = context_->Map(video_texture_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    if (FAILED(hr)) {
        return false;
    }

    const auto* src = static_cast<const unsigned char*>(frame.buffer.data);
    if (!src || frame.buffer.size == 0) {
        context_->Unmap(video_texture_.Get(), 0);
        return false;
    }

    if (frame.format == PixelFormat::YUYV) {
        const size_t srcRowBytes = static_cast<size_t>(frame.width) * 2; // YUYV
        for (int y = 0; y < frame.height; ++y) {
            auto* dstRow = static_cast<unsigned char*>(mapped.pData) + static_cast<size_t>(y) * mapped.RowPitch;
            const auto* srcRow = src + static_cast<size_t>(y) * srcRowBytes;
            std::memcpy(dstRow, srcRow, srcRowBytes);
        }
    }
    else if (frame.format == PixelFormat::RGB24) {
        for (int y = 0; y < frame.height; ++y) {
            auto* dstRow = static_cast<unsigned char*>(mapped.pData) + static_cast<size_t>(y) * mapped.RowPitch;
            const auto* srcRow = src + static_cast<size_t>(y) * static_cast<size_t>(frame.width) * 3;

            for (int x = 0; x < frame.width; ++x) {
                dstRow[x * 4 + 0] = srcRow[x * 3 + 0]; // R
                dstRow[x * 4 + 1] = srcRow[x * 3 + 1]; // G
                dstRow[x * 4 + 2] = srcRow[x * 3 + 2]; // B
                dstRow[x * 4 + 3] = 255;               // A
            }
        }
    }
    else {
        context_->Unmap(video_texture_.Get(), 0);
        return false;
    }

    context_->Unmap(video_texture_.Get(), 0);
    return true;
}

void QtD3D11VideoSourceWidget::render(bool noVideo) {
    if (!context_ || !rtv_ || !swap_chain_) {
        return;
    }

    const float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    context_->OMSetRenderTargets(1, rtv_.GetAddressOf(), nullptr);
    context_->ClearRenderTargetView(rtv_.Get(), clearColor);

    const float dpr = devicePixelRatioF();
    D3D11_VIEWPORT vp = {};
    vp.TopLeftX = 0.0f;
    vp.TopLeftY = 0.0f;
    vp.Width = static_cast<float>(std::max(1, static_cast<int>(width() * dpr)));
    vp.Height = static_cast<float>(std::max(1, static_cast<int>(height() * dpr)));
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    context_->RSSetViewports(1, &vp);

    ShaderParams params = {};
    params.noVideo = noVideo ? 1u : 0u;
    params.format = (currentFormat_ == PixelFormat::YUYV) ? 1u : 0u;
    params.width = static_cast<unsigned int>(std::max(1, frameWidth_));
    params.height = static_cast<unsigned int>(std::max(1, frameHeight_));
    params.packedWidth = static_cast<unsigned int>(std::max(1, packedWidth_));

    context_->UpdateSubresource(constant_buffer_.Get(), 0, nullptr, &params, 0, 0);

    context_->IASetInputLayout(nullptr);
    context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    context_->VSSetShader(vertex_shader_.Get(), nullptr, 0);
    context_->PSSetShader(pixel_shader_.Get(), nullptr, 0);

    ID3D11Buffer* cbs[] = { constant_buffer_.Get() };
    context_->PSSetConstantBuffers(0, 1, cbs);

    ID3D11SamplerState* samplers[] = { sampler_.Get() };
    context_->PSSetSamplers(0, 1, samplers);

    ID3D11ShaderResourceView* srvs[] = { noVideo ? nullptr : video_srv_.Get() };
    context_->PSSetShaderResources(0, 1, srvs);

    context_->Draw(4, 0);

    // Разлинковка SRV, чтобы потом безопасно пересоздавать texture
    ID3D11ShaderResourceView* nullSrvs[] = { nullptr };
    context_->PSSetShaderResources(0, 1, nullSrvs);

    // Для multi-stream dashboard обычно лучше не блокироваться на vsync.
    // Если нужен vsync, поменяйте на Present(1, 0).
    swap_chain_->Present(0, 0);
}

#endif
