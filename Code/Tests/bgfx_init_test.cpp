// Minimal bgfx init test - standalone
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <objbase.h>  // CoInitializeEx, CoUninitialize
#include <cstdio>
#include <cstring>
#include <bgfx/bgfx.h>
#include <bgfx/platform.h>

int main() {
    printf("BGFX Init Test starting...\n");
    
    // Initialize COM
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    printf("CoInitializeEx: 0x%08x\n", hr);
    
    // Create window
    WNDCLASS wc = {};
    wc.lpfnWndProc = DefWindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = "BGFXTest";
    RegisterClass(&wc);
    
    HWND hwnd = CreateWindow("BGFXTest", "BGFXTest", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, NULL, NULL, wc.hInstance, NULL);
    printf("HWND: %p\n", hwnd);
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
    
    // Prime render thread
    printf("Calling bgfx::renderFrame(0)...\n");
    bgfx::RenderFrame::Enum rf = bgfx::renderFrame(0);
    printf("renderFrame returned: %d\n", rf);
    
    bgfx::PlatformData pd;
    memset(&pd, 0, sizeof(pd));
    pd.nwh = hwnd;
    bgfx::setPlatformData(pd);
    
    // Try D3D11 directly
    printf("\n=== Trying D3D11 ===\n");
    bgfx::Init init;
    memset(&init, 0, sizeof(init));
    init.type = bgfx::RendererType::Direct3D11;
    init.vendorId = BGFX_PCI_ID_NONE;
    init.resolution.width = 640;
    init.resolution.height = 480;
    init.resolution.reset = BGFX_RESET_VSYNC;
    init.resolution.maxFrameLatency = 1;
    
    bool ok = bgfx::init(init);
    printf("bgfx::init(D3D11) = %d\n", ok);
    if (ok) {
        printf("Renderer: %d\n", bgfx::getRendererType());
        bgfx::frame();
        bgfx::shutdown();
    }
    
    // Try auto
    if (!ok) {
        printf("\n=== Trying Auto ===\n");
        init.type = bgfx::RendererType::Count;
        ok = bgfx::init(init);
        printf("bgfx::init(Auto) = %d\n", ok);
        if (ok) {
            printf("Renderer: %d\n", bgfx::getRendererType());
            bgfx::frame();
            bgfx::shutdown();
        }
    }
    
    // Try OpenGL
    if (!ok) {
        printf("\n=== Trying OpenGL ===\n");
        init.type = bgfx::RendererType::OpenGL;
        ok = bgfx::init(init);
        printf("bgfx::init(GL) = %d\n", ok);
        if (ok) {
            printf("Renderer: %d\n", bgfx::getRendererType());
            bgfx::frame();
            bgfx::shutdown();
        }
    }
    
    // Try D3D12
    if (!ok) {
        printf("\n=== Trying D3D12 ===\n");
        init.type = bgfx::RendererType::Direct3D12;
        ok = bgfx::init(init);
        printf("bgfx::init(D3D12) = %d\n", ok);
        if (ok) {
            printf("Renderer: %d\n", bgfx::getRendererType());
            bgfx::frame();
            bgfx::shutdown();
        }
    }
    
    printf("\n=== Complete ===\n");
    DestroyWindow(hwnd);
    CoUninitialize();
    return ok ? 0 : 1;
}
