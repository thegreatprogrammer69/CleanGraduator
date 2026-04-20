#include <algorithm>
#include <cstring>
#include <mutex>

#include <qglobal.h>
#include <QMetaObject>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QShowEvent>
#include <QDebug>

#include "domain/core/video/PixelFormat.h"
#include "domain/core/video/VideoFrame.h"
#include "viewmodels/video/VideoSourceViewModel.h"

#ifdef PLATFORM_WINDOWS

#include "QtD3D11VideoSourceWidget.h"

#ifndef _WIN32
#error This implementation requires Windows + Direct3D 11
#endif

#include <d3dcompiler.h>

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
        float2(0.0, 0.0),
        float2(0.0, 1.0),
        float2(1.0, 0.0),
        float2(1.0, 1.0)
    };

    o.pos = float4(positions[vertexId], 0.0, 1.0);
    o.uv  = uvs[vertexId];
    return o;
}
)";

static constexpr const char* kPixelShaderHlsl = R"(
Texture2D<float> gVideoTex : register(t0);
SamplerState     gSampler  : register(s0);

cbuffer Params : register(b0)
{
    uint uNoVideo;
    uint uWidth;
    uint uHeight;
    uint uTextureWidthBytes;
    float uViewportWidth;
    float uViewportHeight;
    float uCircleDiameterPx;
    float uBandWidthPx;      // ширина одной полосы; всего полос 3
    float4 uCircleColor1;
    float4 uCircleColor2;
};

struct VSOut
{
    float4 pos : SV_POSITION;
    float2 uv  : TEXCOORD0;
};

float4 sampleBaseColor(VSOut i)
{
    if (uNoVideo != 0)
    {
        return float4(240.0 / 255.0, 240.0 / 255.0, 240.0 / 255.0, 1.0);
    }

    uint width  = max(uWidth,  1u);
    uint height = max(uHeight, 1u);

    float2 uv = saturate(i.uv);

    uint x = min((uint)(uv.x * (float)width),  width  - 1u);
    uint y = min((uint)(uv.y * (float)height), height - 1u);

    uint baseX = x * 3u;

    float b = gVideoTex.Load(int3((int)(baseX + 0u), (int)y, 0));
    float g = gVideoTex.Load(int3((int)(baseX + 1u), (int)y, 0));
    float r = gVideoTex.Load(int3((int)(baseX + 2u), (int)y, 0));

    return float4(r, g, b, 1.0);
}

float4 circleOverlay(float2 fragCoord)
{
    if (uNoVideo != 0)
    {
        return float4(0.0, 0.0, 0.0, 0.0);
    }

    if (uCircleDiameterPx <= 0.0 || uViewportWidth <= 0.0 || uViewportHeight <= 0.0)
    {
        return float4(0.0, 0.0, 0.0, 0.0);
    }

    const float bandWidth = max(uBandWidthPx, 1.0);
    const float totalWidth = bandWidth * 3.0;

    float2 center = float2(uViewportWidth * 0.5, uViewportHeight * 0.5);
    float2 delta = fragCoord - center;

    float radius = uCircleDiameterPx * 0.5;
    float dist = length(delta);

    float outerR = radius + totalWidth * 0.5;
    float innerR = radius - totalWidth * 0.5;

    if (dist < innerR || dist > outerR)
    {
        return float4(0.0, 0.0, 0.0, 0.0);
    }

    // 0 -> внешняя граница, 1 -> внутренняя граница
    float t = (outerR - dist) / totalWidth;

    float4 color;
    if (t < (1.0 / 3.0))
    {
        color = uCircleColor1;
    }
    else if (t < (2.0 / 3.0))
    {
        color = uCircleColor2;
    }
    else
    {
        color = uCircleColor1;
    }

    // мягкие края только по внешней и внутренней границе всего кольца
    float edgeFade = min(dist - innerR, outerR - dist);
    color.a *= saturate(edgeFade);

    return color;
}

float4 main(VSOut i) : SV_TARGET
{
    float4 baseColor = sampleBaseColor(i);
    float4 overlay = circleOverlay(i.pos.xy);
    return float4(lerp(baseColor.rgb, overlay.rgb, overlay.a), 1.0);
}
)";

void unpackColor(std::uint32_t rgba, float (&out)[4])
{
    out[0] = static_cast<float>((rgba >> 24) & 0xFFu) / 255.0f;
    out[1] = static_cast<float>((rgba >> 16) & 0xFFu) / 255.0f;
    out[2] = static_cast<float>((rgba >> 8) & 0xFFu) / 255.0f;
    out[3] = static_cast<float>(rgba & 0xFFu) / 255.0f;
}

} // namespace

QtD3D11VideoSourceWidget::QtD3D11VideoSourceWidget(mvvm::VideoSourceViewModel& model, QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_NativeWindow, true);
    setAttribute(Qt::WA_PaintOnScreen, true);
    setAttribute(Qt::WA_NoSystemBackground, true);
    setAutoFillBackground(false);

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

    circle_diameter_sub_ = model.circleDiameterPercent().subscribe([this](const auto& a) {
        const float diameter = a.new_value;
        QMetaObject::invokeMethod(
            this,
            [this, diameter]() {
                std::lock_guard lock(mutex_);
                circleDiameterPercent_ = diameter;
                update();
            },
            Qt::QueuedConnection);
    });

    circle_color1_sub_ = model.circleColor1().subscribe([this](const auto& a) {
        const std::uint32_t color = a.new_value;
        QMetaObject::invokeMethod(
            this,
            [this, color]() {
                std::lock_guard lock(mutex_);
                circleColor1_ = color;
                update();
            },
            Qt::QueuedConnection);
    });

    circle_color2_sub_ = model.circleColor2().subscribe([this](const auto& a) {
        const std::uint32_t color = a.new_value;
        QMetaObject::invokeMethod(
            this,
            [this, color]() {
                std::lock_guard lock(mutex_);
                circleColor2_ = color;
                update();
            },
            Qt::QueuedConnection);
    });

    circleDiameterPercent_ = model.circleDiameterPercent().get_copy();
    circleColor1_ = model.circleColor1().get_copy();
    circleColor2_ = model.circleColor2().get_copy();
}

QtD3D11VideoSourceWidget::~QtD3D11VideoSourceWidget()
{
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

void QtD3D11VideoSourceWidget::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    ensureD3D();
    update();
}

void QtD3D11VideoSourceWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    if (d3d_inited_) {
        resizeSwapChain();
    }
}

void QtD3D11VideoSourceWidget::paintEvent(QPaintEvent* event)
{
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
    if (frame && frame->format == PixelFormat::RGB24) {
        if (ensureVideoTexture(*frame)) {
            hasRenderableFrame = uploadFrame(*frame);
        }
    }

    render(!isOpened || !hasRenderableFrame);
}

void QtD3D11VideoSourceWidget::setVideoFrame(VideoFramePtr frame)
{
    std::lock_guard lock(mutex_);
    current_frame_ = std::move(frame);
    update();
}

bool QtD3D11VideoSourceWidget::ensureD3D()
{
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

bool QtD3D11VideoSourceWidget::createDevice()
{
    UINT flags = 0;

#ifndef NDEBUG
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

bool QtD3D11VideoSourceWidget::createSwapChain()
{
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
    desc.BufferCount = 1;
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

bool QtD3D11VideoSourceWidget::createBackBuffer()
{
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

void QtD3D11VideoSourceWidget::releaseBackBuffer()
{
    if (context_) {
        context_->OMSetRenderTargets(0, nullptr, nullptr);
    }
    rtv_.Reset();
}

void QtD3D11VideoSourceWidget::resizeSwapChain()
{
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

void QtD3D11VideoSourceWidget::releaseVideoTexture()
{
    video_srv_.Reset();
    video_texture_.Reset();
    textureInitialized_ = false;
    frameWidth_ = 0;
    frameHeight_ = 0;
    textureWidthBytes_ = 0;
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

bool QtD3D11VideoSourceWidget::createShaders()
{
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
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
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

bool QtD3D11VideoSourceWidget::ensureVideoTexture(const VideoFrame& frame)
{
    if (frame.format != PixelFormat::RGB24) {
        return false;
    }

    const int desiredWidth = frame.width;
    const int desiredHeight = frame.height;
    const int desiredTextureWidthBytes = frame.width * 3;

    if (textureInitialized_
        && frameWidth_ == desiredWidth
        && frameHeight_ == desiredHeight
        && textureWidthBytes_ == desiredTextureWidthBytes) {
        return true;
    }

    releaseVideoTexture();

    D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width = static_cast<UINT>(desiredTextureWidthBytes);
    texDesc.Height = static_cast<UINT>(desiredHeight);
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_R8_UNORM;
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
    frameWidth_ = frame.width;
    frameHeight_ = frame.height;
    textureWidthBytes_ = desiredTextureWidthBytes;

    return true;
}

bool QtD3D11VideoSourceWidget::uploadFrame(const VideoFrame& frame)
{
    if (!video_texture_ || frame.format != PixelFormat::RGB24) {
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

    const size_t srcRowBytes = static_cast<size_t>(frame.width) * 3;
    for (int y = 0; y < frame.height; ++y) {
        auto* dstRow = static_cast<unsigned char*>(mapped.pData) + static_cast<size_t>(y) * mapped.RowPitch;
        const auto* srcRow = src + static_cast<size_t>(y) * srcRowBytes;
        std::memcpy(dstRow, srcRow, srcRowBytes);
    }

    context_->Unmap(video_texture_.Get(), 0);
    return true;
}

void QtD3D11VideoSourceWidget::render(bool noVideo)
{
    if (!context_ || !rtv_ || !swap_chain_) {
        return;
    }

    const float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    context_->OMSetRenderTargets(1, rtv_.GetAddressOf(), nullptr);
    context_->ClearRenderTargetView(rtv_.Get(), clearColor);

    const float dpr = devicePixelRatioF();
    const float viewportWidth = static_cast<float>(std::max(1, static_cast<int>(width() * dpr)));
    const float viewportHeight = static_cast<float>(std::max(1, static_cast<int>(height() * dpr)));

    D3D11_VIEWPORT vp = {};
    vp.TopLeftX = 0.0f;
    vp.TopLeftY = 0.0f;
    vp.Width = viewportWidth;
    vp.Height = viewportHeight;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    context_->RSSetViewports(1, &vp);

    ShaderParams params = {};
    params.noVideo = noVideo ? 1u : 0u;
    params.width = static_cast<unsigned int>(std::max(1, frameWidth_));
    params.height = static_cast<unsigned int>(std::max(1, frameHeight_));
    params.textureWidthBytes = static_cast<unsigned int>(std::max(1, textureWidthBytes_));
    params.viewportWidth = viewportWidth;
    params.viewportHeight = viewportHeight;
    params.circleDiameterPx = std::max(0.0f, viewportHeight * (circleDiameterPercent_ / 100.0f));
    params.padding0 = 2.0f; // ширина одной полосы в px; итоговая толщина = 3 * padding0
    unpackColor(circleColor1_, params.circleColor1);
    unpackColor(circleColor2_, params.circleColor2);

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

    ID3D11ShaderResourceView* nullSrvs[] = { nullptr };
    context_->PSSetShaderResources(0, 1, nullSrvs);

    swap_chain_->Present(0, 0);
}

#endif // PLATFORM_WINDOWS
