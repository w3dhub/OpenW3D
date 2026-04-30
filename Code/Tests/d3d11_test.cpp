// Minimal D3D11 test - standalone, no bgfx
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <cstdio>
#include <comdef.h>

int main() {
    printf("D3D11 Test starting...\n");
    
    // Initialize COM
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    printf("CoInitializeEx: 0x%08x\n", hr);
    
    // Create window
    WNDCLASS wc = {};
    wc.lpfnWndProc = DefWindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = "D3D11Test";
    RegisterClass(&wc);
    
    HWND hwnd = CreateWindow("D3D11Test", "D3D11Test", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, NULL, NULL, wc.hInstance, NULL);
    printf("HWND: %p\n", hwnd);
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
    
    // Test 1: IDXGIFactory adapter enumeration
    printf("\n=== Test 1: DXGI Adapter Enumeration ===\n");
    IDXGIFactory* factory = nullptr;
    hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
    printf("CreateDXGIFactory: 0x%08x\n", hr);
    if (factory) {
        for (UINT i = 0;; i++) {
            IDXGIAdapter* adapter = nullptr;
            hr = factory->EnumAdapters(i, &adapter);
            if (FAILED(hr)) break;
            DXGI_ADAPTER_DESC desc;
            adapter->GetDesc(&desc);
            printf("  Adapter %u: %ls (VendorId=0x%x, DeviceId=0x%x)\n", 
                   i, desc.Description, desc.VendorId, desc.DeviceId);
            
            // Test 2: D3D11CreateDevice for this adapter
            ID3D11Device* device = nullptr;
            ID3D11DeviceContext* context = nullptr;
            D3D_FEATURE_LEVEL featureLevel;
            hr = D3D11CreateDevice(adapter, D3D_DRIVER_TYPE_UNKNOWN, NULL, 0,
                NULL, 0, D3D11_SDK_VERSION, &device, &featureLevel, &context);
            printf("  D3D11CreateDevice: 0x%08x (feature level: %d)\n", hr, featureLevel);
            if (device) device->Release();
            if (context) context->Release();
            adapter->Release();
        }
        factory->Release();
    }
    
    // Test 3: D3D11CreateDevice with NULL adapter (auto-select)
    printf("\n=== Test 2: D3D11CreateDevice (auto-select) ===\n");
    ID3D11Device* device = nullptr;
    ID3D11DeviceContext* context = nullptr;
    D3D_FEATURE_LEVEL featureLevel;
    hr = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0,
        NULL, 0, D3D11_SDK_VERSION, &device, &featureLevel, &context);
    printf("D3D11CreateDevice(HARDWARE): 0x%08x\n", hr);
    if (device) device->Release();
    if (context) context->Release();
    
    // Test 4: D3D11CreateDevice with WARP (software)
    printf("\n=== Test 3: D3D11CreateDevice (WARP) ===\n");
    hr = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_WARP, NULL, 0,
        NULL, 0, D3D11_SDK_VERSION, &device, &featureLevel, &context);
    printf("D3D11CreateDevice(WARP): 0x%08x\n", hr);
    if (device) device->Release();
    if (context) context->Release();
    
    // Test 5: SwapChain creation
    printf("\n=== Test 4: SwapChain Creation ===\n");
    if (SUCCEEDED(D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0,
        NULL, 0, D3D11_SDK_VERSION, &device, &featureLevel, &context))) {
        
        IDXGIDevice* dxgiDevice = nullptr;
        hr = device->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice);
        printf("QueryInterface(IDXGIDevice): 0x%08x\n", hr);
        
        IDXGIAdapter* dxgiAdapter = nullptr;
        if (dxgiDevice) {
            dxgiDevice->GetAdapter(&dxgiAdapter);
            dxgiDevice->Release();
        }
        
        IDXGIFactory* dxgiFactory = nullptr;
        if (dxgiAdapter) {
            dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&dxgiFactory);
            dxgiAdapter->Release();
        }
        
        if (dxgiFactory) {
            DXGI_SWAP_CHAIN_DESC scd = {};
            scd.BufferCount = 1;
            scd.BufferDesc.Width = 640;
            scd.BufferDesc.Height = 480;
            scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            scd.BufferDesc.RefreshRate.Numerator = 60;
            scd.BufferDesc.RefreshRate.Denominator = 1;
            scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            scd.OutputWindow = hwnd;
            scd.SampleDesc.Count = 1;
            scd.SampleDesc.Quality = 0;
            scd.Windowed = TRUE;
            scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
            
            IDXGISwapChain* swapChain = nullptr;
            hr = dxgiFactory->CreateSwapChain(device, &scd, &swapChain);
            printf("CreateSwapChain: 0x%08x\n", hr);
            if (swapChain) swapChain->Release();
            dxgiFactory->Release();
        }
        device->Release();
        if (context) context->Release();
    }
    
    printf("\n=== Test Complete ===\n");
    DestroyWindow(hwnd);
    CoUninitialize();
    return 0;
}
