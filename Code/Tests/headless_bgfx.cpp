#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cstdio>
#include <bgfx/bgfx.h>

int main(int argc, char** argv) {
    printf("Headless BGFX init test\\n");

    // Detect interactive session
    bool interactive = false;
    HWINSTA hWinSta = GetProcessWindowStation();
    if (hWinSta) {
        USEROBJECTFLAGS uof = {0};
        DWORD dwSize = 0;
        if (GetUserObjectInformation(hWinSta, UOI_FLAGS, &uof, sizeof(uof), &dwSize)) {
            interactive = (uof.dwFlags & WSF_VISIBLE) != 0;
        }
    }
    printf("Interactive session: %s\\n", interactive ? "YES" : "NO");

    // Prime bgfx context
    bgfx::renderFrame(-1);

    // Set platform data - NULL nwh for headless
    bgfx::PlatformData pd;
    memset(&pd, 0, sizeof(pd));
    pd.nwh = nullptr;  // Headless mode - no window handle
    bgfx::setPlatformData(pd);

    // Init bgfx
    bgfx::Init init;
    init.type = bgfx::RendererType::Direct3D11;
    init.vendorId = BGFX_PCI_ID_NONE;
    init.deviceId = 0;
    init.debug = false;
    init.profile = false;
    init.resolution.width = 640;
    init.resolution.height = 480;
    init.resolution.reset = BGFX_RESET_NONE;  // No vsync in headless
    init.resolution.maxFrameLatency = 1;

    bool ok = bgfx::init(init);
    printf("bgfx::init result: %d\\n", ok);

    if (ok) {
        printf("SUCCESS: Headless BGFX initialized!\\n");

        // Try to create a frame buffer for offscreen rendering
        bgfx::FrameBufferHandle fbh = bgfx::createFrameBuffer(640, 480, bgfx::TextureFormat::BGRA8);
        printf("Frame buffer handle: %d (valid: %d)\\n", fbh.idx, bgfx::isValid(fbh));

        if (bgfx::isValid(fbh)) {
            bgfx::destroy(fbh);
        }

        bgfx::shutdown();
        return 0;
    } else {
        printf("FAILED: Headless BGFX init failed\\n");
        return 1;
    }
}
