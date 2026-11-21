#include "win32_dx11.hpp"

#include <d3d11.h>
#include <dxgi.h>
#include <windows.h>
#include <wrl/client.h>

#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

#include <cstdint>
#include <fstream>
#include <vector>

using Microsoft::WRL::ComPtr;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

struct Window::Impl {
    HINSTANCE instance = GetModuleHandleW(nullptr);
    HWND hwnd = nullptr;
    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> context;
    ComPtr<IDXGISwapChain> swapChain;
    ComPtr<ID3D11RenderTargetView> target;
    bool capture = false;
    bool platform = false;
    bool renderer = false;
    bool finished = false;
    unsigned resizeW = 0, resizeH = 0;
    int width = 0, height = 0;

    bool CreateTarget() {
        ComPtr<ID3D11Texture2D> backBuffer;
        return SUCCEEDED(swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer)))
            && SUCCEEDED(device->CreateRenderTargetView(backBuffer.Get(), nullptr, &target));
    }

    void ResizeTarget() {
        if (capture || !resizeW || !resizeH || !swapChain)
            return;
        context->OMSetRenderTargets(0, nullptr, nullptr);
        target.Reset();
        if (SUCCEEDED(swapChain->ResizeBuffers(0, resizeW, resizeH, DXGI_FORMAT_UNKNOWN, 0))) {
            width = static_cast<int>(resizeW);
            height = static_cast<int>(resizeH);
        }
        finished |= !CreateTarget();
        resizeW = resizeH = 0;
    }

    bool SaveBmp(const std::string& path) {
        ComPtr<ID3D11Texture2D> buffer;
        if (FAILED(swapChain->GetBuffer(0, IID_PPV_ARGS(&buffer))))
            return false;
        D3D11_TEXTURE2D_DESC desc{};
        buffer->GetDesc(&desc);
        desc.Usage = D3D11_USAGE_STAGING;
        desc.BindFlags = 0;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        desc.MiscFlags = 0;
        ComPtr<ID3D11Texture2D> staging;
        if (FAILED(device->CreateTexture2D(&desc, nullptr, &staging)))
            return false;
        context->CopyResource(staging.Get(), buffer.Get());
        D3D11_MAPPED_SUBRESOURCE map{};
        if (FAILED(context->Map(staging.Get(), 0, D3D11_MAP_READ, 0, &map)))
            return false;
        std::ofstream file(path, std::ios::binary);
        BITMAPFILEHEADER header{};
        header.bfType = 0x4D42;
        header.bfOffBits = sizeof(header) + sizeof(BITMAPINFOHEADER);
        header.bfSize = header.bfOffBits + desc.Width * desc.Height * 4;
        BITMAPINFOHEADER info{};
        info.biSize = sizeof(info);
        info.biWidth = static_cast<LONG>(desc.Width);
        info.biHeight = -static_cast<LONG>(desc.Height);
        info.biPlanes = 1;
        info.biBitCount = 32;
        file.write(reinterpret_cast<const char*>(&header), sizeof(header));
        file.write(reinterpret_cast<const char*>(&info), sizeof(info));
        std::vector<char> row(desc.Width * 4);
        for (unsigned y = 0; y < desc.Height; ++y) {
            const auto* rgba = static_cast<const char*>(map.pData) + y * map.RowPitch;
            for (unsigned x = 0; x < desc.Width; ++x) {
                row[x*4] = rgba[x*4+2];
                row[x*4+1] = rgba[x*4+1];
                row[x*4+2] = rgba[x*4];
                row[x*4+3] = static_cast<char>(255);
            }
            file.write(row.data(), static_cast<std::streamsize>(row.size()));
        }
        context->Unmap(staging.Get(), 0);
        return static_cast<bool>(file);
    }

    static LRESULT CALLBACK Procedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
        if (message == WM_NCCREATE) {
            const auto* args = reinterpret_cast<CREATESTRUCTW*>(lParam);
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(args->lpCreateParams));
        }
        auto* app = reinterpret_cast<Impl*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
        if (app && app->platform && !app->capture && ImGui_ImplWin32_WndProcHandler(hwnd, message, wParam, lParam))
            return 1;
        if (app && message == WM_SIZE && wParam != SIZE_MINIMIZED) {
            app->resizeW = LOWORD(lParam);
            app->resizeH = HIWORD(lParam);
        }
        if (app && message == WM_DPICHANGED && !app->capture) {
            const auto& rect = *reinterpret_cast<const RECT*>(lParam);
            SetWindowPos(hwnd, nullptr, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top,
                         SWP_NOZORDER | SWP_NOACTIVATE);
        }
        if (message == WM_DESTROY) {
            if (app)
                app->finished = true;
            return 0;
        }
        return DefWindowProcW(hwnd, message, wParam, lParam);
    }
};

Window::Window(int width, int height, bool capture) : impl_(std::make_unique<Impl>()) {
    auto& d = *impl_;
    d.width = width;
    d.height = height;
    d.capture = capture;
    ImGui_ImplWin32_EnableDpiAwareness();
    WNDCLASSW windowClass{};
    windowClass.lpfnWndProc = Impl::Procedure;
    windowClass.hInstance = d.instance;
    windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    windowClass.lpszClassName = L"M3EImGuiGallery";
    if (!RegisterClassW(&windowClass))
        return;
    RECT area{0, 0, width, height};
    AdjustWindowRect(&area, WS_OVERLAPPEDWINDOW, FALSE);
    d.hwnd = CreateWindowW(windowClass.lpszClassName, L"Material 3 Expressive - Dear ImGui",
                          WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, area.right - area.left, area.bottom - area.top,
                          nullptr, nullptr, d.instance, &d);
    if (!d.hwnd)
        return;
    DXGI_SWAP_CHAIN_DESC desc{};
    desc.BufferCount = 2;
    desc.BufferDesc.Width = static_cast<unsigned>(width);
    desc.BufferDesc.Height = static_cast<unsigned>(height);
    desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.OutputWindow = d.hwnd;
    desc.SampleDesc.Count = 1;
    desc.Windowed = TRUE;
    desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    const auto create = [&](D3D_DRIVER_TYPE driver) {
        return D3D11CreateDeviceAndSwapChain(nullptr, driver, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &desc,
                                             &d.swapChain, &d.device, nullptr, &d.context);
    };
    if (FAILED(create(capture ? D3D_DRIVER_TYPE_WARP : D3D_DRIVER_TYPE_HARDWARE)) &&
        FAILED(create(D3D_DRIVER_TYPE_WARP)))
        return;
    if (!d.CreateTarget())
        return;
    d.platform = ImGui_ImplWin32_Init(d.hwnd);
    d.renderer = ImGui_ImplDX11_Init(d.device.Get(), d.context.Get());
    if (!capture) {
        ShowWindow(d.hwnd, SW_SHOW);
        UpdateWindow(d.hwnd);
    }
}

Window::~Window() {
    auto& d = *impl_;
    if (d.renderer)
        ImGui_ImplDX11_Shutdown();
    if (d.platform)
        ImGui_ImplWin32_Shutdown();
    if (d.hwnd)
        DestroyWindow(d.hwnd);
    UnregisterClassW(L"M3EImGuiGallery", d.instance);
}

bool Window::Valid() const { return impl_->platform && impl_->renderer && impl_->target; }

bool Window::Poll() {
    MSG msg{};
    while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    impl_->ResizeTarget();
    return !impl_->finished;
}

float Window::DpiScale() const { return ImGui_ImplWin32_GetDpiScaleForHwnd(impl_->hwnd); }

void Window::BeginFrame(bool capture, float deltaTime) {
    ImGui_ImplDX11_NewFrame();
    if (capture) {
        auto& io = ImGui::GetIO();
        io.DisplaySize = {static_cast<float>(impl_->width), static_cast<float>(impl_->height)};
        io.DisplayFramebufferScale = {1, 1};
        io.DeltaTime = deltaTime;
    } else {
        ImGui_ImplWin32_NewFrame();
    }
    ImGui::NewFrame();
}

bool Window::Render(const std::string& capturePath) {
    auto& d = *impl_;
    ImGui::Render();
    constexpr float clear[] = {0, 0, 0, 1};
    d.context->OMSetRenderTargets(1, d.target.GetAddressOf(), nullptr);
    d.context->ClearRenderTargetView(d.target.Get(), clear);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    if (!capturePath.empty())
        return d.SaveBmp(capturePath);
    if (!d.capture)
        return SUCCEEDED(d.swapChain->Present(1, 0));
    return true;
}
