// RenderCompare - Minimal side-by-side DX8 vs BGFX comparison harness
// Builds as a standalone executable. Compile twice with different backends.
//
// Build (DX8):
//   cmake -B build_dx8 -GNinja -DWANT_DX9=ON
//   ninja render_compare
//
// Build (BGFX):
//   cmake -B build_bgfx -GNinja -DENABLE_BGFX_BACKEND=ON -DWANT_DX9=OFF
//   ninja render_compare

#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <string>

#include "ww3d.h"
#include "dx8wrapper.h"
#include "dx8vertexbuffer.h"
#include "dx8indexbuffer.h"
#include "shader.h"
#include "vertmaterial.h"
#include "texture.h"
#include "vector3.h"

// Simple colored triangle vertex data
struct SimpleVertex
{
    float x, y, z;
    unsigned int diffuse;
    float u, v;
};

static SimpleVertex g_triangle[] = {
    { 0.0f,  0.5f, 0.0f, 0xFFFF0000, 0.5f, 0.0f}, // Red, top
    { 0.5f, -0.5f, 0.0f, 0xFF00FF00, 1.0f, 1.0f}, // Green, bottom-right
    {-0.5f, -0.5f, 0.0f, 0xFF0000FF, 0.0f, 1.0f}, // Blue, bottom-left
};

// Save a HBITMAP to a BMP file
static bool SaveBitmapToFile(HBITMAP hBitmap, const char* filename)
{
    BITMAP bmp;
    GetObject(hBitmap, sizeof(BITMAP), &bmp);

    int width = bmp.bmWidth;
    int height = bmp.bmHeight;
    int rowSize = ((width * 3 + 3) & ~3); // 24-bit, padded to 4 bytes
    int imageSize = rowSize * height;

    BITMAPFILEHEADER bfh = {};
    bfh.bfType = 0x4D42; // 'BM'
    bfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + imageSize;
    bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    BITMAPINFOHEADER bih = {};
    bih.biSize = sizeof(BITMAPINFOHEADER);
    bih.biWidth = width;
    bih.biHeight = height;
    bih.biPlanes = 1;
    bih.biBitCount = 24;
    bih.biCompression = BI_RGB;
    bih.biSizeImage = imageSize;

    // Get pixel data
    unsigned char* pixels = new unsigned char[imageSize];
    HDC hdcMem = CreateCompatibleDC(NULL);
    SelectObject(hdcMem, hBitmap);

    BITMAPINFO bi = {};
    bi.bmiHeader = bih;
    GetDIBits(hdcMem, hBitmap, 0, height, pixels, &bi, DIB_RGB_COLORS);

    DeleteDC(hdcMem);

    // Flip rows (BMP is bottom-up, GDI is top-down)
    unsigned char* flipped = new unsigned char[imageSize];
    for (int y = 0; y < height; ++y) {
        memcpy(flipped + y * rowSize, pixels + (height - 1 - y) * rowSize, rowSize);
    }

    FILE* f = fopen(filename, "wb");
    if (!f) {
        delete[] pixels;
        delete[] flipped;
        return false;
    }

    fwrite(&bfh, sizeof(bfh), 1, f);
    fwrite(&bih, sizeof(bih), 1, f);
    fwrite(flipped, imageSize, 1, f);
    fclose(f);

    delete[] pixels;
    delete[] flipped;
    return true;
}

// Capture window client area to BMP
static bool CaptureWindow(HWND hwnd, const char* filename)
{
    RECT rc;
    GetClientRect(hwnd, &rc);
    int width = rc.right - rc.left;
    int height = rc.bottom - rc.top;

    HDC hdcScreen = GetDC(hwnd);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);
    HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, width, height);
    SelectObject(hdcMem, hBitmap);

    BitBlt(hdcMem, 0, 0, width, height, hdcScreen, 0, 0, SRCCOPY);

    bool ok = SaveBitmapToFile(hBitmap, filename);

    DeleteObject(hBitmap);
    DeleteDC(hdcMem);
    ReleaseDC(hwnd, hdcScreen);

    return ok;
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int main(int argc, char** argv)
{
    const char* outputFile = (argc > 1) ? argv[1] : "render_compare.bmp";
    int width = 640;
    int height = 480;

    // Create window
    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = "RenderCompare";
    RegisterClass(&wc);

    HWND hwnd = CreateWindow(
        "RenderCompare", "RenderCompare",
        WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, width + 16, height + 39,
        NULL, NULL, wc.hInstance, NULL);

    if (!hwnd) {
        fprintf(stderr, "Failed to create window\n");
        return 1;
    }

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    // Init WW3D
    WW3DErrorType err = WW3D::Init(hwnd, false);
    if (err != WW3D_ERROR_OK) {
        fprintf(stderr, "WW3D::Init failed: %d\n", err);
        DestroyWindow(hwnd);
        return 1;
    }

    // Set resolution
    WW3D::Set_Device_Resolution(width, height, 32, false);

    // Create vertex buffer
    VertexBufferClass* vb = new VertexBufferClass(
        DX8_FVF_XYZDUV1,
        3,
        BufferEnum::BUFFER_TYPE_DYNAMIC_DX8);

    {
        VertexBufferClass::WriteLockClass lock(vb);
        SimpleVertex* verts = (SimpleVertex*)lock.Get_Vertex_Array();
        memcpy(verts, g_triangle, sizeof(g_triangle));
    }

    // Create index buffer (simple triangle)
    IndexBufferClass* ib = new IndexBufferClass(3);
    {
        IndexBufferClass::WriteLockClass lock(ib);
        unsigned short* indices = lock.Get_Index_Array();
        indices[0] = 0;
        indices[1] = 1;
        indices[2] = 2;
    }

    // Setup camera
    Matrix3D camera(true);
    camera.Translate(Vector3(0, 0, -2));
    WW3D::Set_Camera(&camera);

    // Setup simple material
    ShaderClass shader;
    shader.Set_Depth_Compare(ShaderClass::PASS_ALWAYS);
    shader.Set_Depth_Write(false);

    // Render one frame
    WW3D::Begin_Render(true, true, Vector3(0.2f, 0.2f, 0.2f));
    WW3D::Set_Shader(shader);
    WW3D::Set_Texture(0, NULL);

    DX8Wrapper::Set_Vertex_Buffer(vb);
    DX8Wrapper::Set_Index_Buffer(ib, 0);
    DX8Wrapper::Draw_Triangles(0, 1, 0, 3);

    WW3D::End_Render();

    // Capture screenshot
    if (CaptureWindow(hwnd, outputFile)) {
        printf("Screenshot saved to %s\n", outputFile);
    } else {
        fprintf(stderr, "Failed to save screenshot\n");
    }

    // Cleanup
    delete vb;
    delete ib;
    WW3D::Shutdown();
    DestroyWindow(hwnd);

    return 0;
}
