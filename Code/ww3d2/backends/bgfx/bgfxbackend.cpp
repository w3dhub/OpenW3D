// bgfxbackend.cpp - OpenW3D render backend

#include "backends/bgfx/bgfxbackend.h"
#include "backends/bgfx/ShaderKey.h"
#include "backends/bgfx/ShaderVariantCache.h"
#include "rddesc.h"
#include "dx8wrapper.h"
#include "shader.h"
#include "texture.h"
#include "vertmaterial.h"
#include "lightenvironment.h"
#include "dx8vertexbuffer.h"
#include "dx8indexbuffer.h"
#include "matrix4.h"
#include "matrix3d.h"
#include "surfaceclass.h"
#include "ww3dformat.h"

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <stdio.h>  // for fopen, fread, fclose
#include <string.h> // for strlen

// Platform detection
#if defined(_WIN32) || defined(_WIN64)
#define BGFX_PLATFORM_WINDOWS 1
#elif defined(__linux__)
#define BGFX_PLATFORM_LINUX 1
#elif defined(__APPLE__) && defined(__MACH__)
#define BGFX_PLATFORM_OSX 1
#else
#error "Unsupported platform"
#endif

#if BGFX_PLATFORM_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#elif BGFX_PLATFORM_LINUX
#include <X11/Xlib.h>
#elif BGFX_PLATFORM_OSX
#include <Cocoa/Cocoa.h>
#endif

namespace
{
    const int DEFAULT_WIDTH = 1024;
    const int DEFAULT_HEIGHT = 768;
    const int DEFAULT_BIT_DEPTH = 32;

    // Map DX8 fill mode to bgfx state flags
    uint64_t FillMode_To_BGFX(unsigned fillmode)
    {
        switch (fillmode) {
            case DX8RenderState::WIREFRAME:
                return BGFX_STATE_PT_LINES;
            case DX8RenderState::POINT:
                return BGFX_STATE_PT_POINTS;
            case DX8RenderState::SOLID:
            default:
                return BGFX_STATE_NONE;
        }
    }

    // Map DX8 cull mode to bgfx state flags
    uint64_t CullMode_To_BGFX(unsigned cullmode)
    {
        switch (cullmode) {
            case DX8RenderState::NONE:
                return BGFX_STATE_NONE;
            case DX8RenderState::CW:
                return BGFX_STATE_CULL_CW;
            case DX8RenderState::CCW:
                return BGFX_STATE_CULL_CCW;
            default:
                return BGFX_STATE_CULL_CCW;
        }
    }

    // Map DX8 blend factors to bgfx state flags
    uint64_t BlendFactor_To_BGFX(unsigned factor)
    {
        switch (factor) {
            case DX8RenderState::ZERO:           return BGFX_STATE_BLEND_ZERO;
            case DX8RenderState::ONE:            return BGFX_STATE_BLEND_ONE;
            case DX8RenderState::SRCCOLOR:       return BGFX_STATE_BLEND_SRC_COLOR;
            case DX8RenderState::INVERTSRCCOLOR: return BGFX_STATE_BLEND_INV_SRC_COLOR;
            case DX8RenderState::SRCALPHA:       return BGFX_STATE_BLEND_SRC_ALPHA;
            case DX8RenderState::INVERTSRCALPHA: return BGFX_STATE_BLEND_INV_SRC_ALPHA;
            case DX8RenderState::DESTALPHA:      return BGFX_STATE_BLEND_DST_ALPHA;
            case DX8RenderState::INVERTDESTALPHA: return BGFX_STATE_BLEND_INV_DST_ALPHA;
            case DX8RenderState::DESTCOLOR:       return BGFX_STATE_BLEND_DST_COLOR;
            case DX8RenderState::INVERTDESTCOLOR: return BGFX_STATE_BLEND_INV_DST_COLOR;
            case DX8RenderState::SRCALPHASAT:    return BGFX_STATE_BLEND_SRC_ALPHA_SAT;
            default:                             return BGFX_STATE_BLEND_ONE;
        }
    }
}

BGFXBackend::BGFXBackend() :
    m_hwnd(nullptr),
    m_nativeDisplay(nullptr),
    m_width(DEFAULT_WIDTH),
    m_height(DEFAULT_HEIGHT),
    m_bitDepth(DEFAULT_BIT_DEPTH),
    m_windowed(true),
    m_currentDevice(0),
    m_swapInterval(1),
    m_textureBitDepth(DEFAULT_BIT_DEPTH),
    m_systemTexture(BGFX_INVALID_HANDLE),
    m_frameBuffer(BGFX_INVALID_HANDLE),
    m_currentProgram(BGFX_INVALID_HANDLE),
    m_whiteTexture(BGFX_INVALID_HANDLE),
    m_initialized(false),
    m_currentState(BGFX_STATE_DEFAULT),
    m_currentColor(0xFF000000),
    m_viewportX(0),
    m_viewportY(0),
    m_viewportWidth(DEFAULT_WIDTH),
    m_viewportHeight(DEFAULT_HEIGHT),
    m_viewportMinZ(0.0f),
    m_viewportMaxZ(1.0f),
    m_scissorEnabled(false),
    m_scissorX(0),
    m_scissorY(0),
    m_scissorWidth(0),
    m_scissorHeight(0),
    m_activeFrameBuffer(BGFX_INVALID_HANDLE),
    m_diffuseUniform(BGFX_INVALID_HANDLE),
    m_specularUniform(BGFX_INVALID_HANDLE),
    m_emissiveUniform(BGFX_INVALID_HANDLE),
    m_ambientUniform(BGFX_INVALID_HANDLE),
    m_opacityUniform(BGFX_INVALID_HANDLE),
    m_lightingEnableUniform(BGFX_INVALID_HANDLE),
    m_alphaTestUniform(BGFX_INVALID_HANDLE),
    m_ambientLightUniform(BGFX_INVALID_HANDLE),
    m_fogColorUniform(BGFX_INVALID_HANDLE),
    m_fogParamsUniform(BGFX_INVALID_HANDLE),
    m_lightDirUniform(BGFX_INVALID_HANDLE),
    m_lightDiffuseUniform(BGFX_INVALID_HANDLE),
    m_modelUniform(BGFX_INVALID_HANDLE),
    m_viewProjUniform(BGFX_INVALID_HANDLE),
    m_camPosUniform(BGFX_INVALID_HANDLE),
    m_defaultProgram(BGFX_INVALID_HANDLE),
    m_worldDirty(false),
    m_viewDirty(false),
    m_projectionDirty(false),
    m_currentVB(nullptr),
    m_currentIB(nullptr),
    m_currentIBOffset(0),
    m_currentMaterial(nullptr),
    m_currentLightEnv(nullptr),
    m_shaderModeUniform(BGFX_INVALID_HANDLE),
    m_shaderFlagsUniform(BGFX_INVALID_HANDLE),
    m_detailFuncUniform(BGFX_INVALID_HANDLE),
    m_activeTextureStages(0)
{
    m_samplerUniforms.fill(BGFX_INVALID_HANDLE);
    m_boundTextures.fill(BGFX_INVALID_HANDLE);
    m_textureFlags.fill(UINT32_MAX);
    m_textureMipLevels.fill(0);
    m_textureUvIndices.fill(0);

    // Populate device enumeration
    // Device 0: Auto-detect
    // Device 1: Vulkan
    // Device 2: OpenGL
    // Device 3: Direct3D 11
    // Device 4: Direct3D 12
    m_deviceNames.emplace_back("Auto");
    m_deviceNames.emplace_back("Vulkan");
    m_deviceNames.emplace_back("OpenGL");
#if defined(_WIN32) || defined(_WIN64)
    m_deviceNames.emplace_back("Direct3D 11");
    m_deviceNames.emplace_back("Direct3D 12");
#endif
}

BGFXBackend::~BGFXBackend()
{
    Shutdown();
}

bool BGFXBackend::Init(void * hwnd, bool lite)
{
    if (m_initialized) {
        return true;
    }

    m_hwnd = hwnd;

    // Set platform-specific data for bgfx
    bgfx::PlatformData pd;
    pd.ndt = nullptr;
    pd.nwh = hwnd;
    pd.context = nullptr;
    pd.queue = nullptr;
    pd.backBuffer = nullptr;
    pd.backBufferDS = nullptr;

#if BGFX_PLATFORM_WINDOWS
    pd.ndt = nullptr; // Windows doesn't need native display
    pd.nwh = hwnd;    // Window handle is the HWND
    pd.type = bgfx::NativeWindowHandleType::Default;
#elif BGFX_PLATFORM_LINUX
    if (!m_nativeDisplay) {
        m_nativeDisplay = XOpenDisplay(nullptr);
    }
    pd.ndt = m_nativeDisplay;
    pd.nwh = (void*)nullptr; // Will be set from hwnd
    pd.type = bgfx::NativeWindowHandleType::Default;
#elif BGFX_PLATFORM_OSX
    pd.ndt = nullptr;
    pd.nwh = hwnd;
    pd.type = bgfx::NativeWindowHandleType::Default;
#endif

    bgfx::setPlatformData(pd);

    // Determine renderer type based on selected device
    bgfx::RendererType::Enum renderer = bgfx::RendererType::Count;  // 0 = auto-detect
    if (m_currentDevice == 1) {
        renderer = bgfx::RendererType::Vulkan;
    } else if (m_currentDevice == 2) {
        renderer = bgfx::RendererType::OpenGL;
    } else if (m_currentDevice == 3) {
        renderer = bgfx::RendererType::Direct3D11;
    } else if (m_currentDevice == 4) {
        renderer = bgfx::RendererType::Direct3D12;
    }

    // Initialize bgfx with fallback strategy
    bool bgfx_ok = false;

    // First try: requested renderer (or auto-detect)
    {
        bgfx::Init init;
        init.type = renderer;
        init.vendorId = BGFX_PCI_ID_NONE;
        init.deviceId = 0;
        init.debug = false;
        init.profile = false;
        init.resolution.width = static_cast<uint32_t>(m_width);
        init.resolution.height = static_cast<uint32_t>(m_height);
        init.resolution.reset = BGFX_RESET_VSYNC;
        init.resolution.maxFrameLatency = 1;

        WWDEBUG_SAY(("BGFXBackend: trying bgfx::init with renderer type %d\n", renderer));
        bgfx_ok = bgfx::init(init);
    }

    // Fallback: if auto-detect failed, try D3D11 explicitly on Windows
    if (!bgfx_ok && renderer == bgfx::RendererType::Count) {
#if defined(_WIN32) || defined(_WIN64)
        WWDEBUG_SAY(("BGFXBackend: auto-detect failed, trying Direct3D11\n"));
        bgfx::Init init;
        init.type = bgfx::RendererType::Direct3D11;
        init.vendorId = BGFX_PCI_ID_NONE;
        init.deviceId = 0;
        init.debug = false;
        init.profile = false;
        init.resolution.width = static_cast<uint32_t>(m_width);
        init.resolution.height = static_cast<uint32_t>(m_height);
        init.resolution.reset = BGFX_RESET_VSYNC;
        init.resolution.maxFrameLatency = 1;
        bgfx_ok = bgfx::init(init);
        if (bgfx_ok) {
            WWDEBUG_SAY(("BGFXBackend: Direct3D11 succeeded\n"));
        }
#endif
    }

    // Fallback: try OpenGL
    if (!bgfx_ok) {
        WWDEBUG_SAY(("BGFXBackend: trying OpenGL\n"));
        bgfx::Init init;
        init.type = bgfx::RendererType::OpenGL;
        init.vendorId = BGFX_PCI_ID_NONE;
        init.deviceId = 0;
        init.debug = false;
        init.profile = false;
        init.resolution.width = static_cast<uint32_t>(m_width);
        init.resolution.height = static_cast<uint32_t>(m_height);
        init.resolution.reset = BGFX_RESET_VSYNC;
        init.resolution.maxFrameLatency = 1;
        bgfx_ok = bgfx::init(init);
    }

    if (!bgfx_ok) {
        WWDEBUG_SAY(("BGFXBackend: all renderer attempts failed\n"));
        return false;
    }

    m_initialized = true;

    m_samplerUniforms[0] = bgfx::createUniform("s_diffuse", bgfx::UniformType::Sampler);
    for (int stage = 1; stage < MAX_TEXTURE_STAGES; ++stage) {
        char samplerName[16];
        snprintf(samplerName, sizeof(samplerName), "s_tex%d", stage);
        m_samplerUniforms[stage] = bgfx::createUniform(samplerName, bgfx::UniformType::Sampler);
    }

    m_diffuseUniform = bgfx::createUniform(BGFXMaterialUniforms::DIFFUSE_COLOR, bgfx::UniformType::Vec4);
    m_specularUniform = bgfx::createUniform(BGFXMaterialUniforms::SPECULAR_COLOR, bgfx::UniformType::Vec4);
    m_emissiveUniform = bgfx::createUniform(BGFXMaterialUniforms::EMISSIVE_COLOR, bgfx::UniformType::Vec4);
    m_ambientUniform = bgfx::createUniform(BGFXMaterialUniforms::AMBIENT_COLOR, bgfx::UniformType::Vec4);
    m_opacityUniform = bgfx::createUniform(BGFXMaterialUniforms::OPACITY, bgfx::UniformType::Vec4);
    m_lightingEnableUniform = bgfx::createUniform(BGFXMaterialUniforms::LIGHTING_ENABLE, bgfx::UniformType::Vec4);
    m_alphaTestUniform = bgfx::createUniform("u_alphaTest", bgfx::UniformType::Vec4);

    // Fog uniforms
    m_fogColorUniform = bgfx::createUniform("u_fogColor", bgfx::UniformType::Vec4);
    m_fogParamsUniform = bgfx::createUniform("u_fogParams", bgfx::UniformType::Vec4);

    // Light environment uniforms (arrays of 4)
    m_ambientLightUniform = bgfx::createUniform("u_ambientLight", bgfx::UniformType::Vec4);
    m_lightDirUniform = bgfx::createUniform("u_lightDir", bgfx::UniformType::Vec4, 4);
    m_lightDiffuseUniform = bgfx::createUniform("u_lightDiffuse", bgfx::UniformType::Vec4, 4);
    m_lightPosUniform = bgfx::createUniform("u_lightPos", bgfx::UniformType::Vec4, 4);

    // Transform uniforms (mat4 arrays for bgfx)
    m_modelUniform = bgfx::createUniform("u_uberModel", bgfx::UniformType::Vec4, 4);
    m_viewProjUniform = bgfx::createUniform("u_uberViewProj", bgfx::UniformType::Vec4, 4);
    m_camPosUniform = bgfx::createUniform("u_uberCamPos", bgfx::UniformType::Vec4);

    // Ubershader uniforms
    m_shaderModeUniform = bgfx::createUniform("u_shaderMode", bgfx::UniformType::Vec4);
    m_shaderFlagsUniform = bgfx::createUniform("u_shaderFlags", bgfx::UniformType::Vec4);
    m_detailFuncUniform = bgfx::createUniform("u_detailFunc", bgfx::UniformType::Vec4);

    // Load default mesh shader program
    {
        const char* suffix = "120"; // default to OpenGL/D3D11
        bgfx::RendererType::Enum rt = bgfx::getRendererType();
        switch (rt) {
            case bgfx::RendererType::Vulkan:       suffix = "spirv"; break;
            case bgfx::RendererType::Metal:        suffix = "metal"; break;
            case bgfx::RendererType::OpenGLES:     suffix = "120";   break;
            case bgfx::RendererType::Direct3D11:   suffix = "s_5_0"; break;
            case bgfx::RendererType::Direct3D12:   suffix = "s_5_0"; break;
            case bgfx::RendererType::OpenGL:       suffix = "120";   break;
            default:                               suffix = "120";   break;
        }
        char vsPath[512];
        char fsPath[512];
        const char* shaderDirs[] = {
            "./shaders/",
            "../shaders/",
            "shaders/",
            "../../shaders/",
            nullptr
        };
        bgfx::ShaderHandle vs = BGFX_INVALID_HANDLE;
        bgfx::ShaderHandle fs = BGFX_INVALID_HANDLE;
        for (int i = 0; shaderDirs[i] && !bgfx::isValid(vs); ++i) {
            snprintf(vsPath, sizeof(vsPath), "%svs_mesh_%s.bin", shaderDirs[i], suffix);
            snprintf(fsPath, sizeof(fsPath), "%sfs_mesh_%s.bin", shaderDirs[i], suffix);
            vs = Load_Shader(vsPath);
            if (bgfx::isValid(vs)) {
                fs = Load_Shader(fsPath);
            }
            if (bgfx::isValid(vs) && bgfx::isValid(fs)) {
                WWDEBUG_SAY(("BGFXBackend: Loaded shaders from %s\n", shaderDirs[i]));
                break;
            }
            vs = BGFX_INVALID_HANDLE;
            fs = BGFX_INVALID_HANDLE;
        }
        if (bgfx::isValid(vs) && bgfx::isValid(fs)) {
            m_defaultProgram = bgfx::createProgram(vs, fs, true); // destroy shaders when program is destroyed
        } else {
            WWDEBUG_SAY(("BGFXBackend: Failed to load default mesh shaders (suffix: %s)\n", suffix));
            WWDEBUG_SAY(("BGFXBackend: Tried paths: ./shaders/, ../shaders/, shaders/, ../../shaders/\n"));
        }
    }

    // Set initial view dimensions
    Update_ViewDimensions();

    // Set vsync
    Set_Swap_Interval(m_swapInterval);

    // Initialize render state cache with defaults
    m_render_state[DX8RenderState::FILLMODE] = DX8RenderState::SOLID;
    m_render_state[DX8RenderState::CULLMODE] = DX8RenderState::CCW;
    m_render_state[DX8RenderState::BLENDENABLE] = 0;
    m_render_state[DX8RenderState::ZENABLE] = 1;
    m_render_state[DX8RenderState::ZWRITEENABLE] = 1;

    return true;
}

void BGFXBackend::Shutdown()
{
    if (!m_initialized) {
        return;
    }

    if (bgfx::isValid(m_frameBuffer)) {
        bgfx::destroy(m_frameBuffer);
        m_frameBuffer = BGFX_INVALID_HANDLE;
    }

    if (bgfx::isValid(m_systemTexture)) {
        bgfx::destroy(m_systemTexture);
        m_systemTexture = BGFX_INVALID_HANDLE;
    }

    for (bgfx::UniformHandle& sampler : m_samplerUniforms) {
        if (bgfx::isValid(sampler)) {
            bgfx::destroy(sampler);
            sampler = BGFX_INVALID_HANDLE;
        }
    }

    bgfx::UniformHandle uniforms[] = {
        m_diffuseUniform,
        m_specularUniform,
        m_emissiveUniform,
        m_ambientUniform,
        m_opacityUniform,
        m_lightingEnableUniform,
        m_alphaTestUniform,
        m_ambientLightUniform,
        m_lightDirUniform,
        m_lightDiffuseUniform,
        m_lightPosUniform,
        m_fogColorUniform,
        m_fogParamsUniform,
        m_modelUniform,
        m_viewProjUniform,
        m_camPosUniform,
        m_shaderModeUniform,
        m_shaderFlagsUniform,
        m_detailFuncUniform,
    };

    for (bgfx::UniformHandle uniform : uniforms) {
        if (bgfx::isValid(uniform)) {
            bgfx::destroy(uniform);
        }
    }

    m_diffuseUniform = BGFX_INVALID_HANDLE;
    m_specularUniform = BGFX_INVALID_HANDLE;
    m_emissiveUniform = BGFX_INVALID_HANDLE;
    m_ambientUniform = BGFX_INVALID_HANDLE;
    m_opacityUniform = BGFX_INVALID_HANDLE;
    m_lightingEnableUniform = BGFX_INVALID_HANDLE;
    m_alphaTestUniform = BGFX_INVALID_HANDLE;
    m_ambientLightUniform = BGFX_INVALID_HANDLE;
    m_lightDirUniform = BGFX_INVALID_HANDLE;
    m_lightDiffuseUniform = BGFX_INVALID_HANDLE;
    m_lightPosUniform = BGFX_INVALID_HANDLE;
    m_fogColorUniform = BGFX_INVALID_HANDLE;
    m_fogParamsUniform = BGFX_INVALID_HANDLE;
    m_modelUniform = BGFX_INVALID_HANDLE;
    m_viewProjUniform = BGFX_INVALID_HANDLE;
    m_camPosUniform = BGFX_INVALID_HANDLE;
    m_shaderModeUniform = BGFX_INVALID_HANDLE;
    m_shaderFlagsUniform = BGFX_INVALID_HANDLE;
    m_detailFuncUniform = BGFX_INVALID_HANDLE;

    if (bgfx::isValid(m_defaultProgram)) {
        bgfx::destroy(m_defaultProgram);
        m_defaultProgram = BGFX_INVALID_HANDLE;
    }

    if (bgfx::isValid(m_whiteTexture)) {
        bgfx::destroy(m_whiteTexture);
        m_whiteTexture = BGFX_INVALID_HANDLE;
    }

    Clear_Resource_Caches();
    m_shaderCache.Clear();
    bgfx::shutdown();
    m_initialized = false;

#if BGFX_PLATFORM_LINUX
    if (m_nativeDisplay) {
        XCloseDisplay(static_cast<Display*>(m_nativeDisplay));
        m_nativeDisplay = nullptr;
    }
#endif
}

bool BGFXBackend::Set_Any_Render_Device()
{
    m_currentDevice = 0;
    if (m_initialized) {
        Clear_Resource_Caches();
        Shutdown();
        return Init(m_hwnd, false);
    }
    return true;
}

bool BGFXBackend::Set_Render_Device(const char * dev_name, int width, int height, int bits, int windowed, bool resize_window)
{
    for (int i = 0; i < Get_Render_Device_Count(); ++i) {
        if (strcmp(Get_Render_Device_Name(i), dev_name) == 0) {
            return Set_Render_Device(i, width, height, bits, windowed, resize_window);
        }
    }
    return false;
}

bool BGFXBackend::Set_Render_Device(int dev, int resx, int resy, int bits, int windowed, bool resize_window)
{
    if (dev >= 0 && dev < Get_Render_Device_Count()) {
        m_currentDevice = dev;
    }
    return Set_Device_Resolution(resx, resy, bits, windowed, resize_window);
}

bool BGFXBackend::Set_Next_Render_Device()
{
    int count = Get_Render_Device_Count();
    if (count <= 0) {
        return false;
    }
    m_currentDevice = (m_currentDevice + 1) % count;
    if (m_initialized) {
        Clear_Resource_Caches();
        Shutdown();
        return Init(m_hwnd, false);
    }
    return true;
}

bool BGFXBackend::Toggle_Windowed()
{
    m_windowed = !m_windowed;
    if (m_initialized) {
        uint32_t resetFlags = m_windowed ? 0 : BGFX_RESET_FULLSCREEN;
        bgfx::reset(m_width, m_height, resetFlags);
        Update_ViewDimensions();
    }
    return true;
}

bool BGFXBackend::Is_Windowed()
{
    return m_windowed;
}

int BGFXBackend::Get_Render_Device_Count()
{
    return static_cast<int>(m_deviceNames.size());
}

int BGFXBackend::Get_Render_Device()
{
    return m_currentDevice;
}

const RenderDeviceDescClass & BGFXBackend::Get_Render_Device_Desc(int deviceidx)
{
    // Return a static empty desc like NullBackend
    static RenderDeviceDescClass nullDesc;
    return nullDesc;
}

const char * BGFXBackend::Get_Render_Device_Name(int device_index)
{
    if (device_index >= 0 && device_index < Get_Render_Device_Count()) {
        return m_deviceNames[device_index].c_str();
    }
    return "BGFX Renderer";
}

bool BGFXBackend::Set_Device_Resolution(int width, int height, int bits, int windowed, bool resize_window)
{
    if (width > 0) m_width = width;
    if (height > 0) m_height = height;
    if (bits > 0) m_bitDepth = bits;
    if (windowed >= 0) m_windowed = (windowed != 0);

    if (m_initialized) {
        uint32_t resetFlags = m_windowed ? 0 : BGFX_RESET_FULLSCREEN;
        bgfx::reset(m_width, m_height, resetFlags);
        Update_ViewDimensions();

        // If we have an active off-screen framebuffer, it needs to be recreated
        // by the caller since bgfx framebuffers are tied to texture dimensions.
        // We reset to default framebuffer on resize.
        if (bgfx::isValid(m_activeFrameBuffer)) {
            Set_Default_FrameBuffer();
        }
    }

    return true;
}

void BGFXBackend::Get_Device_Resolution(int & set_w, int & set_h, int & set_bits, bool & set_windowed)
{
    set_w = m_width;
    set_h = m_height;
    set_bits = m_bitDepth;
    set_windowed = m_windowed;
}

void BGFXBackend::Get_Render_Target_Resolution(int & set_w, int & set_h, int & set_bits, bool & set_windowed)
{
    Get_Device_Resolution(set_w, set_h, set_bits, set_windowed);
}

int BGFXBackend::Get_Device_Resolution_Width()
{
    return m_width;
}

int BGFXBackend::Get_Device_Resolution_Height()
{
    return m_height;
}

bool BGFXBackend::Registry_Save_Render_Device(const char * sub_key)
{
    return false;
}

bool BGFXBackend::Registry_Load_Render_Device(const char * sub_key, bool resize_window)
{
    return false;
}

bool BGFXBackend::Registry_Save_Render_Device(const char *sub_key, int device, int width, int height, int depth, bool windowed, int texture_depth)
{
    return false;
}

bool BGFXBackend::Registry_Load_Render_Device(const char * sub_key, char *device, int device_len, int &width, int &height, int &depth, int &windowed, int &texture_depth)
{
    return false;
}

void BGFXBackend::Begin_Scene()
{
    // Reset view to full viewport at start of each scene
    bgfx::setViewRect(0, 0, 0, static_cast<uint16_t>(m_width), static_cast<uint16_t>(m_height));
    bgfx::setViewMode(0, bgfx::ViewMode::Sequential);
    
    // Touch view to ensure it's rendered even if no draw calls
    bgfx::touch(0);

    // Reset texture stage tracking for the new frame
    m_activeTextureStages = 0;
}

void BGFXBackend::End_Scene(bool flip_frame)
{
    // Bind all active texture stages before frame submission
    for (int i = 0; i < m_activeTextureStages; ++i) {
        if (bgfx::isValid(m_boundTextures[i]) && bgfx::isValid(m_samplerUniforms[i])) {
            bgfx::setTexture(static_cast<uint8_t>(i), m_samplerUniforms[i], m_boundTextures[i]);
        }
    }

    if (flip_frame) {
        bgfx::frame(true); // Block until rendering is complete
    }
}

void BGFXBackend::Flip_To_Primary()
{
    bgfx::frame(true);
}

void BGFXBackend::Clear(bool clear_color, bool clear_z_stencil, const Vector3 &color, float z, unsigned int stencil)
{
    uint16_t flags = 0;

    if (clear_color) {
        flags |= BGFX_CLEAR_COLOR;
    }

    if (clear_z_stencil) {
        flags |= BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL;
    }

    m_currentColor = Color_To_ABGR(color);
    
    // Set view clear - this will be applied when the view is submitted
    bgfx::setViewClear(0, flags, m_currentColor, 1.0f, 0);
    
    // Ensure the view rect is set for the clear to apply
    bgfx::setViewRect(0, 0, 0, static_cast<uint16_t>(m_width), static_cast<uint16_t>(m_height));
}

void BGFXBackend::Set_Swap_Interval(int swap)
{
    m_swapInterval = swap;
    if (m_initialized) {
        uint32_t resetFlags = (swap == 0) ? BGFX_RESET_NONE : (BGFX_RESET_VSYNC);
        bgfx::reset(m_width, m_height, resetFlags);
    }
}

int BGFXBackend::Get_Swap_Interval()
{
    return m_swapInterval;
}

void BGFXBackend::Set_Texture_Bitdepth(int depth)
{
    m_textureBitDepth = depth;
}

int BGFXBackend::Get_Texture_Bitdepth()
{
    return m_textureBitDepth;
}

void BGFXBackend::Set_Viewport(const void* viewport)
{
    if (!viewport) {
        // Reset to full screen viewport
        Set_Viewport(0, 0, m_width, m_height, 0.0f, 1.0f);
        return;
    }

    // Unpack D3DVIEWPORT9 struct: { X, Y, Width, Height, MinZ, MaxZ }
    // The struct layout is: DWORD X, Y, Width, Height; float MinZ, MaxZ;
    const uint8_t* vp = static_cast<const uint8_t*>(viewport);
    uint32_t x = *reinterpret_cast<const uint32_t*>(vp + 0);
    uint32_t y = *reinterpret_cast<const uint32_t*>(vp + 4);
    uint32_t width = *reinterpret_cast<const uint32_t*>(vp + 8);
    uint32_t height = *reinterpret_cast<const uint32_t*>(vp + 12);
    float minZ = *reinterpret_cast<const float*>(vp + 16);
    float maxZ = *reinterpret_cast<const float*>(vp + 20);

    Set_Viewport(static_cast<int>(x), static_cast<int>(y),
                 static_cast<int>(width), static_cast<int>(height),
                 minZ, maxZ);
}

void BGFXBackend::Set_Viewport(int x, int y, int width, int height, float minZ, float maxZ)
{
    m_viewportX = x;
    m_viewportY = y;
    m_viewportWidth = width;
    m_viewportHeight = height;
    m_viewportMinZ = minZ;
    m_viewportMaxZ = maxZ;

    // Clamp to device resolution
    if (m_viewportWidth > m_width) m_viewportWidth = m_width;
    if (m_viewportHeight > m_height) m_viewportHeight = m_height;
    if (m_viewportX < 0) m_viewportX = 0;
    if (m_viewportY < 0) m_viewportY = 0;

    // Set bgfx view rect for position/size
    bgfx::setViewRect(0,
                      static_cast<uint16_t>(m_viewportX),
                      static_cast<uint16_t>(m_viewportY),
                      static_cast<uint16_t>(m_viewportWidth),
                      static_cast<uint16_t>(m_viewportHeight));

    // Set scissor if enabled, otherwise scissor matches viewport
    if (m_scissorEnabled) {
        bgfx::setViewScissor(0,
                             static_cast<uint16_t>(m_scissorX),
                             static_cast<uint16_t>(m_scissorY),
                             static_cast<uint16_t>(m_scissorWidth),
                             static_cast<uint16_t>(m_scissorHeight));
    } else {
        bgfx::setViewScissor(0,
                             static_cast<uint16_t>(m_viewportX),
                             static_cast<uint16_t>(m_viewportY),
                             static_cast<uint16_t>(m_viewportWidth),
                             static_cast<uint16_t>(m_viewportHeight));
    }

    // Note: minZ/maxZ are not directly mapped in bgfx (bgfx uses 0-1 depth range by default).
    // If non-default depth range is needed, it would require shader-side handling or
    // bgfx::setViewMode with custom projection matrices.
}

// =============================================================================
// Scissor Rect Management
// =============================================================================

void BGFXBackend::Enable_Scissor(bool enable)
{
    m_scissorEnabled = enable;

    // Update scissor to match current viewport state
    if (m_scissorEnabled) {
        bgfx::setViewScissor(0,
                             static_cast<uint16_t>(m_scissorX),
                             static_cast<uint16_t>(m_scissorY),
                             static_cast<uint16_t>(m_scissorWidth),
                             static_cast<uint16_t>(m_scissorHeight));
    } else {
        // Disable scissor by setting it to viewport bounds
        bgfx::setViewScissor(0,
                             static_cast<uint16_t>(m_viewportX),
                             static_cast<uint16_t>(m_viewportY),
                             static_cast<uint16_t>(m_viewportWidth),
                             static_cast<uint16_t>(m_viewportHeight));
    }
}

void BGFXBackend::Set_Scissor(int x, int y, int width, int height)
{
    m_scissorX = x;
    m_scissorY = y;
    m_scissorWidth = width;
    m_scissorHeight = height;

    if (m_scissorEnabled) {
        bgfx::setViewScissor(0,
                             static_cast<uint16_t>(m_scissorX),
                             static_cast<uint16_t>(m_scissorY),
                             static_cast<uint16_t>(m_scissorWidth),
                             static_cast<uint16_t>(m_scissorHeight));
    }
}

bool BGFXBackend::Is_Scissor_Enabled() const
{
    return m_scissorEnabled;
}

void BGFXBackend::Set_DX8_Render_State(int state, unsigned value)
{
    m_render_state[state] = value;
    Apply_Render_State(state, value);
}

void BGFXBackend::Apply_Render_State(int state, unsigned value)
{
    // Update cached state
    m_render_state[state] = value;

    // Build bgfx state from cached render state
    uint64_t newState = BGFX_STATE_DEFAULT;

    // Fill mode
    auto fillIt = m_render_state.find(DX8RenderState::FILLMODE);
    if (fillIt != m_render_state.end()) {
        newState |= FillMode_To_BGFX(fillIt->second);
    }

    // Cull mode
    auto cullIt = m_render_state.find(DX8RenderState::CULLMODE);
    if (cullIt != m_render_state.end()) {
        newState |= CullMode_To_BGFX(cullIt->second);
    }

    // Z enable - use proper depth test
    auto zIt = m_render_state.find(DX8RenderState::ZENABLE);
    if (zIt != m_render_state.end() && zIt->second) {
        newState |= BGFX_STATE_DEPTH_TEST_LEQUAL;
    } else {
        newState |= BGFX_STATE_DEPTH_TEST_ALWAYS;
    }

    // Z write enable
    auto zwIt = m_render_state.find(DX8RenderState::ZWRITEENABLE);
    if (zwIt != m_render_state.end() && zwIt->second) {
        newState |= BGFX_STATE_WRITE_Z;
    }

    // Blend enable with proper src/dst factors
    auto blendEnableIt = m_render_state.find(DX8RenderState::BLENDENABLE);
    bool blendEnabled = (blendEnableIt != m_render_state.end() && blendEnableIt->second);
    if (blendEnabled) {
        auto srcBlendIt = m_render_state.find(DX8RenderState::SRCBLEND);
        auto dstBlendIt = m_render_state.find(DX8RenderState::DESTBLEND);
        auto blendOpIt  = m_render_state.find(DX8RenderState::BLENDOP);

        uint64_t srcFactor = (srcBlendIt != m_render_state.end())
            ? BlendFactor_To_BGFX(srcBlendIt->second) : BGFX_STATE_BLEND_ONE;
        uint64_t dstFactor = (dstBlendIt != m_render_state.end())
            ? BlendFactor_To_BGFX(dstBlendIt->second) : BGFX_STATE_BLEND_ZERO;

        newState |= BGFX_STATE_BLEND_FUNC(srcFactor, dstFactor);

        // Blend operation
        if (blendOpIt != m_render_state.end()) {
            switch (blendOpIt->second) {
                case DX8RenderState::ADD:         newState |= BGFX_STATE_BLEND_EQUATION_ADD; break;
                case DX8RenderState::SUBTRACT:    newState |= BGFX_STATE_BLEND_EQUATION_SUB; break;
                case DX8RenderState::REVSUBTRACT: newState |= BGFX_STATE_BLEND_EQUATION_REVSUB; break;
                case DX8RenderState::MIN:         newState |= BGFX_STATE_BLEND_EQUATION_MIN; break;
                case DX8RenderState::MAX:         newState |= BGFX_STATE_BLEND_EQUATION_MAX; break;
            }
        }
    }

    // Alpha test (D3DRS_ALPHATESTENABLE) — handled in shader via uniform
    auto alphaTestIt = m_render_state.find(DX8RenderState::ALPHATESTENABLE);
    bool alphaTestEnabled = (alphaTestIt != m_render_state.end() && alphaTestIt->second);
    if (alphaTestEnabled) {
        auto alphaRefIt = m_render_state.find(DX8RenderState::ALPHAREF);
        float threshold = (alphaRefIt != m_render_state.end())
            ? ((alphaRefIt->second & 0x000000FF) / 255.0f) : 0.5f;
        float alphaTestVal[4] = { 1.0f, threshold, 0.0f, 0.0f };
        bgfx::setUniform(m_alphaTestUniform, alphaTestVal);
    }

    // Fog
    auto fogIt = m_render_state.find(DX8RenderState::FOGENABLE);
    bool fogEnabled = (fogIt != m_render_state.end() && fogIt->second);
    if (fogEnabled) {
        auto fogStartIt = m_render_state.find(DX8RenderState::FOGSTART);
        auto fogEndIt   = m_render_state.find(DX8RenderState::FOGEND);
        auto fogColorIt = m_render_state.find(DX8RenderState::FOGCOLOR);
        auto fogDensityIt = m_render_state.find(DX8RenderState::FOGDENSITY);

        float fogVal[4];
        fogVal[0] = (fogStartIt != m_render_state.end()) ? *reinterpret_cast<const float*>(&fogStartIt->second) : 0.0f;
        fogVal[1] = (fogEndIt   != m_render_state.end()) ? *reinterpret_cast<const float*>(&fogEndIt->second)   : 100.0f;
        fogVal[2] = (fogDensityIt != m_render_state.end()) ? *reinterpret_cast<const float*>(&fogDensityIt->second) : 1.0f;
        fogVal[3] = 1.0f; // enable flag

        uint32_t fogColorU32 = (fogColorIt != m_render_state.end()) ? fogColorIt->second : 0xFFFFFFFF;
        float fogColor[4];
        fogColor[0] = ((fogColorU32 >> 16) & 0xFF) / 255.0f; // R
        fogColor[1] = ((fogColorU32 >> 8)  & 0xFF) / 255.0f; // G
        fogColor[2] = ((fogColorU32 >> 0)  & 0xFF) / 255.0f; // B
        fogColor[3] = ((fogColorU32 >> 24) & 0xFF) / 255.0f; // A
        bgfx::setUniform(m_fogColorUniform, fogColor);
        bgfx::setUniform(m_fogParamsUniform, fogVal);
    }

    // Depth bias (SLOPESCALEDEPTHBIAS, DEPTHBIAS)
    auto slopeBiasIt = m_render_state.find(DX8RenderState::SLOPESCALEDEPTHBIAS);
    auto constBiasIt = m_render_state.find(DX8RenderState::DEPTHBIAS);
    if (slopeBiasIt != m_render_state.end() || constBiasIt != m_render_state.end()) {
        float slopeBias = (slopeBiasIt != m_render_state.end()) ? *reinterpret_cast<const float*>(&slopeBiasIt->second) : 0.0f;
        float constBias = (constBiasIt != m_render_state.end()) ? *reinterpret_cast<const float*>(&constBiasIt->second) : 0.0f;
        // bgfx supports depth bias via BGFX_STATE_DEPTH_BIAS but it's renderer-dependent
        // Most renderers handle this via rasterizer state; skip for now
        (void)slopeBias;
        (void)constBias;
    }

    // Stencil enable
    auto stencilIt = m_render_state.find(DX8RenderState::STENCILENABLE);
    bool stencilEnabled = (stencilIt != m_render_state.end() && stencilIt->second);
    if (stencilEnabled) {
        // Build stencil state - start with read mask 0xFF
        uint32_t stencilState = BGFX_STENCIL_TEST_ALWAYS;
        
        // Stencil func
        auto stencilFuncIt = m_render_state.find(DX8RenderState::STENCILFUNC);
        if (stencilFuncIt != m_render_state.end()) {
            stencilState |= Compare_Stencil_Func(stencilFuncIt->second);
        }
        
        // Stencil ref
        auto stencilRefIt = m_render_state.find(DX8RenderState::STENCILREF);
        if (stencilRefIt != m_render_state.end()) {
            stencilState |= BGFX_STENCIL_FUNC_REF(static_cast<uint32_t>(stencilRefIt->second));
        }
        
        // Stencil mask (read mask)
        auto stencilMaskIt = m_render_state.find(DX8RenderState::STENCILMASK);
        if (stencilMaskIt != m_render_state.end()) {
            stencilState |= BGFX_STENCIL_FUNC_RMASK(static_cast<uint32_t>(stencilMaskIt->second));
        }
        
        // Stencil fail operation
        auto stencilFailIt = m_render_state.find(DX8RenderState::STENCILFAIL);
        if (stencilFailIt != m_render_state.end()) {
            stencilState |= Compare_Stencil_Op(stencilFailIt->second);
        }
        
        // Stencil Z fail operation (depth fail)
        auto stencilZFailIt = m_render_state.find(DX8RenderState::STENCILZFAIL);
        if (stencilZFailIt != m_render_state.end()) {
            stencilState |= Compare_Stencil_Op_Z(stencilZFailIt->second);
        }
        
        // Stencil pass operation (depth pass)
        auto stencilPassIt = m_render_state.find(DX8RenderState::STENCILPASS);
        if (stencilPassIt != m_render_state.end()) {
            stencilState |= Compare_Stencil_Op_Pass(stencilPassIt->second);
        }
        
        bgfx::setStencil(stencilState);
    } else {
        bgfx::setStencil(0);
    }

    m_currentState = newState;
    bgfx::setState(newState);
}

// Helper: Map DX8 stencil comparison function to bgfx stencil test flags
uint32_t BGFXBackend::Compare_Stencil_Func(unsigned dx8func)
{
    switch (dx8func) {
        case 0:  // NEVER
            return BGFX_STENCIL_TEST_NEVER;
        case 1:  // LESS
            return BGFX_STENCIL_TEST_LESS;
        case 2:  // EQUAL
            return BGFX_STENCIL_TEST_EQUAL;
        case 3:  // LESS_EQUAL
            return BGFX_STENCIL_TEST_LEQUAL;
        case 4:  // GREATER
            return BGFX_STENCIL_TEST_GREATER;
        case 5:  // NOT_EQUAL
            return BGFX_STENCIL_TEST_NOTEQUAL;
        case 6:  // GREATER_EQUAL
            return BGFX_STENCIL_TEST_GEQUAL;
        case 7:  // ALWAYS
        default:
            return BGFX_STENCIL_TEST_ALWAYS;
    }
}

// Helper: Map DX8 stencil operation to bgfx stencil op flags (for fail case)
uint32_t BGFXBackend::Compare_Stencil_Op(unsigned dx8op)
{
    switch (dx8op) {
        case 0:  // KEEP
            return BGFX_STENCIL_OP_FAIL_S_KEEP;
        case 1:  // ZERO
            return BGFX_STENCIL_OP_FAIL_S_ZERO;
        case 2:  // REPLACE
            return BGFX_STENCIL_OP_FAIL_S_REPLACE;
        case 3:  // INCRSAT
            return BGFX_STENCIL_OP_FAIL_S_INCRSAT;
        case 4:  // DECRSAT
            return BGFX_STENCIL_OP_FAIL_S_DECRSAT;
        case 5:  // INVERT
            return BGFX_STENCIL_OP_FAIL_S_INVERT;
        case 6:  // INCR
            return BGFX_STENCIL_OP_FAIL_S_INCR;
        case 7:  // DECR
            return BGFX_STENCIL_OP_FAIL_S_DECR;
        default:
            return BGFX_STENCIL_OP_FAIL_S_KEEP;
    }
}

// Helper: Map DX8 stencil operation to bgfx stencil op flags (for depth fail case)
uint32_t BGFXBackend::Compare_Stencil_Op_Z(unsigned dx8op)
{
    switch (dx8op) {
        case 0:  // KEEP
            return BGFX_STENCIL_OP_FAIL_Z_KEEP;
        case 1:  // ZERO
            return BGFX_STENCIL_OP_FAIL_Z_ZERO;
        case 2:  // REPLACE
            return BGFX_STENCIL_OP_FAIL_Z_REPLACE;
        case 3:  // INCRSAT
            return BGFX_STENCIL_OP_FAIL_Z_INCRSAT;
        case 4:  // DECRSAT
            return BGFX_STENCIL_OP_FAIL_Z_DECRSAT;
        case 5:  // INVERT
            return BGFX_STENCIL_OP_FAIL_Z_INVERT;
        case 6:  // INCR
            return BGFX_STENCIL_OP_FAIL_Z_INCR;
        case 7:  // DECR
            return BGFX_STENCIL_OP_FAIL_Z_DECR;
        default:
            return BGFX_STENCIL_OP_FAIL_Z_KEEP;
    }
}

// Helper: Map DX8 stencil operation to bgfx stencil op flags (for depth pass case)
uint32_t BGFXBackend::Compare_Stencil_Op_Pass(unsigned dx8op)
{
    switch (dx8op) {
        case 0:  // KEEP
            return BGFX_STENCIL_OP_PASS_Z_KEEP;
        case 1:  // ZERO
            return BGFX_STENCIL_OP_PASS_Z_ZERO;
        case 2:  // REPLACE
            return BGFX_STENCIL_OP_PASS_Z_REPLACE;
        case 3:  // INCRSAT
            return BGFX_STENCIL_OP_PASS_Z_INCRSAT;
        case 4:  // DECRSAT
            return BGFX_STENCIL_OP_PASS_Z_DECRSAT;
        case 5:  // INVERT
            return BGFX_STENCIL_OP_PASS_Z_INVERT;
        case 6:  // INCR
            return BGFX_STENCIL_OP_PASS_Z_INCR;
        case 7:  // DECR
            return BGFX_STENCIL_OP_PASS_Z_DECR;
        default:
            return BGFX_STENCIL_OP_PASS_Z_KEEP;
    }
}

void BGFXBackend::Set_Light_Environment(const void* env)
{
    float lightingEnable[4] = { env ? 1.0f : 0.0f, 0.0f, 0.0f, 0.0f };
    if (bgfx::isValid(m_lightingEnableUniform)) {
        bgfx::setUniform(m_lightingEnableUniform, lightingEnable);
    }
}

void* BGFXBackend::_Get_DX8_Front_Buffer()
{
    return nullptr; // Not applicable for bgfx
}

void BGFXBackend::Update_ViewDimensions()
{
    m_viewportX = 0;
    m_viewportY = 0;
    m_viewportWidth = m_width;
    m_viewportHeight = m_height;

    bgfx::setViewRect(0, 0, 0, static_cast<uint16_t>(m_width), static_cast<uint16_t>(m_height));
    bgfx::setViewMode(0, bgfx::ViewMode::Sequential);

    // Reset scissor to full viewport
    if (!m_scissorEnabled) {
        bgfx::setViewScissor(0, 0, 0, static_cast<uint16_t>(m_width), static_cast<uint16_t>(m_height));
    }
}

// AutoSelect_Renderer removed - bgfx::RendererType::Count is used directly for auto-detect

// =============================================================================
// Texture Management
// =============================================================================

bgfx::TextureHandle BGFXBackend::Create_System_Texture(int width, int height, bgfx::TextureFormat::Enum format, uint64_t flags)
{
    bgfx::TextureHandle handle = bgfx::createTexture2D(
        static_cast<uint16_t>(width),
        static_cast<uint16_t>(height),
        false,
        1,
        format,
        flags,
        nullptr
    );
    return handle;
}

void BGFXBackend::Destroy_Texture(bgfx::TextureHandle handle)
{
    if (bgfx::isValid(handle)) {
        bgfx::destroy(handle);
    }
}

void BGFXBackend::Set_Texture(int stage, bgfx::TextureHandle handle, uint32_t flags, uint8_t mip)
{
    if (stage < 0 || stage >= MAX_TEXTURE_STAGES) {
        return;
    }

    m_boundTextures[stage] = handle;
    m_textureMipLevels[stage] = mip;

    if (flags != UINT32_MAX) {
        m_textureFlags[stage] = flags;
    }

    if (!bgfx::isValid(handle) || !bgfx::isValid(m_samplerUniforms[stage])) {
        return;
    }

    const uint32_t samplerFlags = (m_textureFlags[stage] == UINT32_MAX) ? BGFX_TEXTURE_NONE : m_textureFlags[stage];
    bgfx::setTexture(static_cast<uint8_t>(stage), m_samplerUniforms[stage], handle, samplerFlags);
}

// =============================================================================
// Vertex Buffer Management
// =============================================================================

bgfx::VertexBufferHandle BGFXBackend::Create_Vertex_Buffer(void* data, int vertexCount, const bgfx::VertexLayout& layout, uint64_t flags)
{
    const bgfx::Memory* mem = bgfx::copy(data, vertexCount * layout.getStride());
    return bgfx::createVertexBuffer(mem, layout, flags);
}

void BGFXBackend::Destroy_Vertex_Buffer(bgfx::VertexBufferHandle handle)
{
    if (bgfx::isValid(handle)) {
        bgfx::destroy(handle);
    }
}

void BGFXBackend::Set_Vertex_Buffer(int stream, bgfx::VertexBufferHandle handle, uint32_t startVertex, uint32_t numVertices)
{
    bgfx::setVertexBuffer(stream, handle, startVertex, numVertices);
}

// =============================================================================
// Index Buffer Management
// =============================================================================

bgfx::IndexBufferHandle BGFXBackend::Create_Index_Buffer(void* data, int indexCount, bool index32bit, uint64_t flags)
{
    const bgfx::Memory* mem = bgfx::copy(data, indexCount * (index32bit ? 4 : 2));
    return bgfx::createIndexBuffer(mem, flags);
}

void BGFXBackend::Destroy_Index_Buffer(bgfx::IndexBufferHandle handle)
{
    if (bgfx::isValid(handle)) {
        bgfx::destroy(handle);
    }
}

void BGFXBackend::Set_Index_Buffer(bgfx::IndexBufferHandle handle, int startIndex, int numIndices)
{
    bgfx::setIndexBuffer(handle, startIndex, numIndices);
}

// =============================================================================
// Draw Commands
// =============================================================================

void BGFXBackend::Draw(int numVertices, int numInstances)
{
    bgfx::submit(0, m_currentProgram, 0, numInstances);
}

void BGFXBackend::Draw_Indexed(int numIndices, int numInstances, int startIndex, int startVertex)
{
    bgfx::submit(0, m_currentProgram, 0, numInstances);
}

// =============================================================================
// Shader Program Management
// =============================================================================

bgfx::ShaderHandle BGFXBackend::Load_Shader(const char* shaderPath)
{
    // Handle empty path
    if (!shaderPath || shaderPath[0] == '\0') {
        return BGFX_INVALID_HANDLE;
    }

    // Try to load shader from filesystem
    FILE* file = fopen(shaderPath, "rb");
    if (!file) {
        // File not found - will fall back to embedded shader
        return BGFX_INVALID_HANDLE;
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (fileSize <= 0) {
        fclose(file);
        return BGFX_INVALID_HANDLE;
    }

    // Read shader binary into buffer
    // Use bgfx::copy to make bgfx own the memory copy
    uint8_t* buffer = new uint8_t[fileSize];
    size_t bytesRead = fread(buffer, 1, fileSize, file);
    fclose(file);

    if (bytesRead != static_cast<size_t>(fileSize)) {
        delete[] buffer;
        return BGFX_INVALID_HANDLE;
    }

    // Copy data into bgfx-managed memory
    const bgfx::Memory* mem = bgfx::copy(buffer, static_cast<uint32_t>(fileSize));
    delete[] buffer;  // bgfx::copy made its own copy, we can free ours;

    if (!mem || !mem->data || mem->size == 0) {
        return BGFX_INVALID_HANDLE;
    }

    // Create shader from binary data
    bgfx::ShaderHandle handle = bgfx::createShader(mem);

    if (!bgfx::isValid(handle)) {
        return BGFX_INVALID_HANDLE;
    }

    return handle;
}

bgfx::ProgramHandle BGFXBackend::Create_Program(const char* vsPath, const char* fsPath)
{
    bgfx::ShaderHandle vs = Load_Shader(vsPath);
    bgfx::ShaderHandle fs = Load_Shader(fsPath);
    
    if (!bgfx::isValid(vs) || !bgfx::isValid(fs)) {
        return BGFX_INVALID_HANDLE;
    }
    
    return Create_Program(vs, fs);
}

bgfx::ProgramHandle BGFXBackend::Create_Program(bgfx::ShaderHandle vs, bgfx::ShaderHandle fs)
{
    if (!bgfx::isValid(vs) || !bgfx::isValid(fs)) {
        return BGFX_INVALID_HANDLE;
    }
    
    return bgfx::createProgram(vs, fs, true);
}

void BGFXBackend::Destroy_Shader(bgfx::ShaderHandle handle)
{
    if (bgfx::isValid(handle)) {
        bgfx::destroy(handle);
    }
}

void BGFXBackend::Destroy_Program(bgfx::ProgramHandle handle)
{
    if (bgfx::isValid(handle)) {
        bgfx::destroy(handle);
    }
}

void BGFXBackend::Set_Program(bgfx::ProgramHandle program)
{
    // Note: In bgfx, programs are submitted directly to views, not set separately
    // This stores the program for later use in submit
    m_currentProgram = program;
}

const char* BGFXBackend::Get_Renderer_Name()
{
    return bgfx::getRendererName(bgfx::getRendererType());
}

bgfx::RendererType::Enum BGFXBackend::Get_Renderer_Type()
{
    return bgfx::getRendererType();
}

// =============================================================================
// Uniform (Constant Buffer) Management
// =============================================================================

bgfx::UniformHandle BGFXBackend::Create_Uniform(const char* name, bgfx::UniformType::Enum type, uint16_t num)
{
    return bgfx::createUniform(name, type, num);
}

void BGFXBackend::Destroy_Uniform(bgfx::UniformHandle handle)
{
    if (bgfx::isValid(handle)) {
        bgfx::destroy(handle);
    }
}

void BGFXBackend::Set_Uniform(bgfx::UniformHandle handle, const void* value, uint16_t num)
{
    if (bgfx::isValid(handle)) {
        bgfx::setUniform(handle, value, num);
    }
}

// =============================================================================
// Frame Buffer (Render Target) Management
// =============================================================================

bgfx::FrameBufferHandle BGFXBackend::Create_FrameBuffer(int width, int height, bgfx::TextureFormat::Enum format, uint64_t textureFlags)
{
    bgfx::TextureHandle colorHandle = bgfx::createTexture2D(
        static_cast<uint16_t>(width),
        static_cast<uint16_t>(height),
        false,
        1,
        format,
        textureFlags | BGFX_TEXTURE_RT_WRITE_ONLY,
        nullptr
    );

    bgfx::FrameBufferHandle fb = bgfx::createFrameBuffer(1, &colorHandle, true);
    return fb;
}

bgfx::FrameBufferHandle BGFXBackend::Create_FrameBuffer(int width, int height, bgfx::TextureFormat::Enum colorFormat, bgfx::TextureFormat::Enum depthFormat, uint64_t textureFlags)
{
    bgfx::TextureHandle colorHandle = bgfx::createTexture2D(
        static_cast<uint16_t>(width),
        static_cast<uint16_t>(height),
        false,
        1,
        colorFormat,
        textureFlags | BGFX_TEXTURE_RT_WRITE_ONLY,
        nullptr
    );

    bgfx::TextureHandle depthHandle = bgfx::createTexture2D(
        static_cast<uint16_t>(width),
        static_cast<uint16_t>(height),
        false,
        1,
        depthFormat,
        BGFX_TEXTURE_RT_WRITE_ONLY,
        nullptr
    );

    bgfx::TextureHandle handles[2] = { colorHandle, depthHandle };
    bgfx::FrameBufferHandle fb = bgfx::createFrameBuffer(2, handles, true);
    return fb;
}

bgfx::FrameBufferHandle BGFXBackend::Create_FrameBuffer_Attachment(bgfx::TextureHandle color, bgfx::TextureHandle depth)
{
    // Create MRT framebuffer with color and depth attachments
    bgfx::TextureHandle handles[2] = { color, depth };
    bgfx::FrameBufferHandle fb = bgfx::createFrameBuffer(2, handles, true);
    return fb;
}

void BGFXBackend::Destroy_FrameBuffer(bgfx::FrameBufferHandle handle)
{
    if (bgfx::isValid(handle)) {
        bgfx::destroy(handle);
    }
}

void BGFXBackend::Set_FrameBuffer(bgfx::FrameBufferHandle handle, int width, int height)
{
    m_activeFrameBuffer = handle;
    bgfx::setViewFrameBuffer(0, handle);

    if (bgfx::isValid(handle) && width > 0 && height > 0) {
        bgfx::setViewRect(0, 0, 0, static_cast<uint16_t>(width), static_cast<uint16_t>(height));
        // Update internal viewport tracking for this framebuffer
        m_viewportX = 0;
        m_viewportY = 0;
        m_viewportWidth = width;
        m_viewportHeight = height;
    } else if (!bgfx::isValid(handle)) {
        // Reset to device resolution
        Update_ViewDimensions();
    }
}

void BGFXBackend::Set_FrameBuffer(bgfx::FrameBufferHandle handle, bgfx::TextureHandle color, bgfx::TextureHandle depth)
{
    m_activeFrameBuffer = handle;
    bgfx::setViewFrameBuffer(0, handle);
}

void BGFXBackend::Set_Default_FrameBuffer()
{
    m_activeFrameBuffer = BGFX_INVALID_HANDLE;
    bgfx::setViewFrameBuffer(0, BGFX_INVALID_HANDLE);
    Update_ViewDimensions();
}

void BGFXBackend::Clear_Resource_Caches()
{
    for (auto& pair : m_vbCache) {
        if (bgfx::isValid(pair.second)) {
            bgfx::destroy(pair.second);
        }
    }
    m_vbCache.clear();

    for (auto& pair : m_ibCache) {
        if (bgfx::isValid(pair.second)) {
            bgfx::destroy(pair.second);
        }
    }
    m_ibCache.clear();

    for (auto& pair : m_textureCache) {
        if (bgfx::isValid(pair.second) && pair.second.idx != m_whiteTexture.idx) {
            bgfx::destroy(pair.second);
        }
    }
    m_textureCache.clear();

    m_boundTextures.fill(BGFX_INVALID_HANDLE);
    m_textureFlags.fill(UINT32_MAX);
    m_textureMipLevels.fill(0);
    m_textureUvIndices.fill(0);
    m_activeTextureStages = 0;
}

// =============================================================================
// Material Integration
// =============================================================================

void BGFXBackend::Set_Material_Colors(const Vector3& diffuse, float alpha, const Vector3& specular, float shininess, const Vector3& emissive, const Vector3& ambient)
{
    float diffuseVec4[4] = { diffuse.X, diffuse.Y, diffuse.Z, alpha };
    float specularVec4[4] = { specular.X, specular.Y, specular.Z, shininess };
    float emissiveVec4[4] = { emissive.X, emissive.Y, emissive.Z, 1.0f };
    float ambientVec4[4] = { ambient.X, ambient.Y, ambient.Z, 1.0f };
    float opacityVec4[4] = { alpha, 0.0f, 0.0f, 0.0f };

    if (bgfx::isValid(m_diffuseUniform)) {
        bgfx::setUniform(m_diffuseUniform, diffuseVec4);
    }
    if (bgfx::isValid(m_specularUniform)) {
        bgfx::setUniform(m_specularUniform, specularVec4);
    }
    if (bgfx::isValid(m_emissiveUniform)) {
        bgfx::setUniform(m_emissiveUniform, emissiveVec4);
    }
    if (bgfx::isValid(m_ambientUniform)) {
        bgfx::setUniform(m_ambientUniform, ambientVec4);
    }
    if (bgfx::isValid(m_opacityUniform)) {
        bgfx::setUniform(m_opacityUniform, opacityVec4);
    }
}

void BGFXBackend::Set_Material_Uniforms(bgfx::UniformHandle diffuseHandle, bgfx::UniformHandle specularHandle, bgfx::UniformHandle emissiveHandle, const MaterialColors& colors)
{
    float diffuseVec4[4] = { colors.Diffuse.X, colors.Diffuse.Y, colors.Diffuse.Z, colors.Alpha };
    float specularVec4[4] = { colors.Specular.X, colors.Specular.Y, colors.Specular.Z, colors.Shininess };
    float emissiveVec4[4] = { colors.Emissive.X, colors.Emissive.Y, colors.Emissive.Z, 1.0f };

    if (bgfx::isValid(diffuseHandle)) {
        bgfx::setUniform(diffuseHandle, diffuseVec4);
    }
    if (bgfx::isValid(specularHandle)) {
        bgfx::setUniform(specularHandle, specularVec4);
    }
    if (bgfx::isValid(emissiveHandle)) {
        bgfx::setUniform(emissiveHandle, emissiveVec4);
    }
}

void BGFXBackend::Set_Texture_Stage(int stage, bgfx::TextureHandle handle, uint32_t flags, uint8_t mip, int uvIndex)
{
    if (stage < 0 || stage >= MAX_TEXTURE_STAGES) {
        return;
    }

    m_textureUvIndices[stage] = uvIndex;

    // Track active stage count
    if (stage + 1 > m_activeTextureStages) {
        m_activeTextureStages = stage + 1;
    }

    Set_Texture(stage, handle, flags, mip);
}


uint64_t BGFXBackend::Apply_Shader(const ShaderClass& shader, bool lightingEnabled)
{
    // Use BGFXMaterialMapper to convert shader to bgfx state
    uint64_t state = BGFXMaterialMapper::Make_Render_State(shader, lightingEnabled);

    // Apply the state
    bgfx::setState(state);

    // Update our tracked state
    m_currentState = state;

    return state;
}

void BGFXBackend::Begin_Material_Pass()
{
    // Reset bound textures to prevent cross-material leakage
    m_boundTextures.fill(BGFX_INVALID_HANDLE);
    m_textureFlags.fill(UINT32_MAX);
    m_textureMipLevels.fill(0);
    m_textureUvIndices.fill(0);
    m_activeTextureStages = 0;
}

void BGFXBackend::End_Material_Pass()
{
    // Material pass complete — state is submitted via bgfx::submit()
    // No explicit cleanup needed since bgfx resets transient state per-submit
}

// ---------------------------------------------------------------------------
// Backend dispatch overrides — bridge W3D state to bgfx
// ---------------------------------------------------------------------------

static void Matrix4ToFloat16Transpose(const Matrix4& src, float dst[16])
{
    // W3D Matrix4 is row-major; bgfx GLSL expects column-major.
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            dst[col * 4 + row] = src[row][col];
        }
    }
}

void BGFXBackend::Apply_Shader_State(const ShaderClass& shader)
{
    // Build ShaderKey from the W3D shader
    ShaderKey key = ShaderKey_From_DX8(shader);

    // Update texture stage count from tracked active stages
    if (m_activeTextureStages > key.texStageCount) {
        key.texStageCount = static_cast<uint8_t>(m_activeTextureStages);
    }

    // Get or create the ubershader variant
    const ShaderVariantCache::ShaderVariant* variant = m_shaderCache.GetOrCreate(key);
    if (variant && bgfx::isValid(variant->program)) {
        m_currentProgram = variant->program;
    } else {
        // Fallback to default program if ubershader not available
        m_currentProgram = m_defaultProgram;
    }

    // ---- Set ubershader uniforms ----

    // Shader mode: encodes gradient, texturing, lighting flags
    float shaderMode[4] = {
        static_cast<float>(key.priGradient),   // x: primary gradient mode
        static_cast<float>(key.secGradient),   // y: secondary gradient on/off
        static_cast<float>(key.texturing),     // z: texturing on/off
        static_cast<float>(key.lighting),      // w: lighting on/off
    };
    if (bgfx::isValid(m_shaderModeUniform)) {
        bgfx::setUniform(m_shaderModeUniform, shaderMode);
    }

    // Shader flags: packed bitfield for fog, alpha test, tex stages, detail funcs
    float shaderFlags[4] = {
        static_cast<float>((key.fogMode << 0) |
                           (key.alphaTest << 4) |
                           (key.texStageCount << 8)),  // x: packed flags
        static_cast<float>(key.blendMode),              // y: blend mode enum
        0.0f, 0.0f  // z, w: reserved
    };
    if (bgfx::isValid(m_shaderFlagsUniform)) {
        bgfx::setUniform(m_shaderFlagsUniform, shaderFlags);
    }

    // Detail functions: post-detail color and alpha modes
    float detailFunc[4] = {
        static_cast<float>(key.detailColor),  // x: detail color func
        static_cast<float>(key.detailAlpha),  // y: detail alpha func
        0.0f, 0.0f  // z, w: reserved
    };
    if (bgfx::isValid(m_detailFuncUniform)) {
        bgfx::setUniform(m_detailFuncUniform, detailFunc);
    }

    // ---- Build bgfx render state from ShaderKey ----
    uint64_t state = BGFX_STATE_NONE;

    // Depth comparison
    switch (static_cast<ShaderClass::DepthCompareType>(key.depthCompare)) {
        case ShaderClass::PASS_NEVER:    state |= BGFX_STATE_DEPTH_TEST_NEVER;    break;
        case ShaderClass::PASS_LESS:     state |= BGFX_STATE_DEPTH_TEST_LESS;     break;
        case ShaderClass::PASS_EQUAL:    state |= BGFX_STATE_DEPTH_TEST_EQUAL;    break;
        case ShaderClass::PASS_LEQUAL:   state |= BGFX_STATE_DEPTH_TEST_LEQUAL;   break;
        case ShaderClass::PASS_GREATER:  state |= BGFX_STATE_DEPTH_TEST_GREATER;  break;
        case ShaderClass::PASS_NOTEQUAL: state |= BGFX_STATE_DEPTH_TEST_NOTEQUAL; break;
        case ShaderClass::PASS_GEQUAL:   state |= BGFX_STATE_DEPTH_TEST_GEQUAL;   break;
        case ShaderClass::PASS_ALWAYS:   state |= BGFX_STATE_DEPTH_TEST_ALWAYS;   break;
        default:                         state |= BGFX_STATE_DEPTH_TEST_LEQUAL;   break;
    }

    // Depth write
    if (key.depthWrite) {
        state |= BGFX_STATE_WRITE_Z;
    }

    // Blend mode
    switch (key.blendMode) {
        case 0: // opaque
            state |= BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_ONE, BGFX_STATE_BLEND_ZERO);
            break;
        case 1: // additive
            state |= BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_ONE, BGFX_STATE_BLEND_ONE);
            break;
        case 2: // alpha
            state |= BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA);
            break;
        case 3: // multiplicative
            state |= BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_DST_COLOR, BGFX_STATE_BLEND_ZERO);
            break;
        case 4: // screen
            state |= BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_INV_SRC_COLOR, BGFX_STATE_BLEND_ONE);
            break;
        default: // other/custom: default to alpha blend
            state |= BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA);
            break;
    }

    // Cull mode — from cached DX8 render state
    auto cullIt = m_render_state.find(DX8RenderState::CULLMODE);
    if (cullIt != m_render_state.end()) {
        state |= CullMode_To_BGFX(cullIt->second);
    } else {
        state |= BGFX_STATE_CULL_CCW; // default
    }

    // Fill mode — from cached DX8 render state
    auto fillIt = m_render_state.find(DX8RenderState::FILLMODE);
    if (fillIt != m_render_state.end()) {
        state |= FillMode_To_BGFX(fillIt->second);
    }

    bgfx::setState(state);
    m_currentState = state;

    // ---- Bind active texture stages ----
    for (int i = 0; i < m_activeTextureStages; ++i) {
        if (bgfx::isValid(m_boundTextures[i]) && bgfx::isValid(m_samplerUniforms[i])) {
            bgfx::setTexture(static_cast<uint8_t>(i), m_samplerUniforms[i], m_boundTextures[i]);
        }
    }
}

static bgfx::TextureFormat::Enum WW3DFormat_To_BGFX(WW3DFormat fmt)
{
    switch (fmt) {
        case WW3D_FORMAT_A8R8G8B8:
        case WW3D_FORMAT_X8R8G8B8:
            return bgfx::TextureFormat::BGRA8;
        case WW3D_FORMAT_R8G8B8:
            return bgfx::TextureFormat::RGB8;
        case WW3D_FORMAT_R5G6B5:
            return bgfx::TextureFormat::R5G6B5;
        case WW3D_FORMAT_A1R5G5B5:
        case WW3D_FORMAT_X1R5G5B5:
            return bgfx::TextureFormat::RGB5A1;
        case WW3D_FORMAT_A4R4G4B4:
        case WW3D_FORMAT_X4R4G4B4:
            return bgfx::TextureFormat::RGBA4;
        case WW3D_FORMAT_DXT1:
            return bgfx::TextureFormat::BC1;
        case WW3D_FORMAT_DXT3:
            return bgfx::TextureFormat::BC2;
        case WW3D_FORMAT_DXT5:
            return bgfx::TextureFormat::BC3;
        case WW3D_FORMAT_A8:
            return bgfx::TextureFormat::A8;
        case WW3D_FORMAT_L8:
            return bgfx::TextureFormat::R8;
        default:
            return bgfx::TextureFormat::BGRA8; // safe fallback
    }
}

static int WW3DFormat_BytesPerPixel(WW3DFormat fmt)
{
    switch (fmt) {
        case WW3D_FORMAT_R8G8B8:     return 3;
        case WW3D_FORMAT_A8R8G8B8:
        case WW3D_FORMAT_X8R8G8B8:  return 4;
        case WW3D_FORMAT_R5G6B5:
        case WW3D_FORMAT_A1R5G5B5:
        case WW3D_FORMAT_X1R5G5B5:
        case WW3D_FORMAT_A4R4G4B4:
        case WW3D_FORMAT_X4R4G4B4:
        case WW3D_FORMAT_A8L8:      return 2;
        case WW3D_FORMAT_R3G3B2:
        case WW3D_FORMAT_A8:
        case WW3D_FORMAT_A8R3G3B2:
        case WW3D_FORMAT_P8:
        case WW3D_FORMAT_L8:
        case WW3D_FORMAT_A4L4:      return 1;
        default:                     return 4; // compressed and unknown fallbacks
    }
}

void BGFXBackend::Apply_Texture_State(unsigned stage, TextureClass* texture)
{
    if (stage >= MAX_TEXTURE_STAGES) {
        return;
    }

    if (!texture) {
        m_boundTextures[stage] = BGFX_INVALID_HANDLE;
        return;
    }

    // Track active stage count
    if (static_cast<int>(stage) + 1 > m_activeTextureStages) {
        m_activeTextureStages = static_cast<int>(stage) + 1;
    }

    // Check cache
    auto it = m_textureCache.find(texture);
    if (it != m_textureCache.end()) {
        m_boundTextures[stage] = it->second;
        return;
    }

    // NOTE: This path reads back from the D3D9 texture via Peek_DX8_Texture().
    // For a fully cross-platform backend, textures should be created natively
    // as BGFXTexture instead of read back from D3D9 on every bind.
    bgfx::TextureHandle bgfxTex = BGFX_INVALID_HANDLE;

    IDirect3DTexture9* d3dTex = texture->Peek_DX8_Texture();
    if (d3dTex) {
        SurfaceClass* surface = texture->Get_Surface_Level(0);
        if (surface) {
            int pitch = 0;
            void* pixels = surface->Lock(&pitch);
            if (pixels) {
                SurfaceClass::SurfaceDescription desc;
                surface->Get_Description(desc);

                int width  = (int)desc.Width;
                int height = (int)desc.Height;
                WW3DFormat wwfmt = desc.Format;
                bgfx::TextureFormat::Enum bgfxFmt = WW3DFormat_To_BGFX(wwfmt);

                // For uncompressed formats, copy row-by-row to handle pitch
                if (wwfmt >= WW3D_FORMAT_R8G8B8 && wwfmt <= WW3D_FORMAT_A4L4) {
                    int bpp = WW3DFormat_BytesPerPixel(wwfmt);
                    int dstPitch = width * bpp;
                    uint8_t* dst = new uint8_t[dstPitch * height];
                    const uint8_t* src = static_cast<const uint8_t*>(pixels);
                    for (int y = 0; y < height; ++y) {
                        memcpy(dst + y * dstPitch, src + y * pitch, dstPitch);
                    }
                    surface->Unlock();

                    const bgfx::Memory* mem = bgfx::copy(dst, dstPitch * height);
                    delete[] dst;

                    bgfxTex = bgfx::createTexture2D(
                        (uint16_t)width, (uint16_t)height, false, 1,
                        bgfxFmt, BGFX_TEXTURE_NONE, mem);
                } else if (wwfmt == WW3D_FORMAT_DXT1 || wwfmt == WW3D_FORMAT_DXT3 || wwfmt == WW3D_FORMAT_DXT5) {
                    // Compressed textures: pitch is row pitch in bytes for compressed blocks
                    int blockSize = (wwfmt == WW3D_FORMAT_DXT1) ? 8 : 16;
                    int blocksY = (height + 3) / 4;
                    int dataSize = pitch * blocksY;
                    surface->Unlock();

                    const bgfx::Memory* mem = bgfx::copy(pixels, dataSize);
                    bgfxTex = bgfx::createTexture2D(
                        (uint16_t)width, (uint16_t)height, false, 1,
                        bgfxFmt, BGFX_TEXTURE_NONE, mem);
                } else {
                    // Other formats: raw copy fallback
                    int size = pitch * height;
                    surface->Unlock();
                    WWDEBUG_SAY(("BGFXBackend: unhandled texture format, using raw copy fallback\n"));

                    const bgfx::Memory* mem = bgfx::copy(pixels, size);
                    bgfxTex = bgfx::createTexture2D(
                        (uint16_t)width, (uint16_t)height, false, 1,
                        bgfxFmt, BGFX_TEXTURE_NONE, mem);
                }
            } else {
                WWDEBUG_SAY(("BGFXBackend: failed to lock texture surface\n"));
            }
            surface->Release_Ref();
        }
    }

    if (!bgfx::isValid(bgfxTex)) {
        // Fallback: 1x1 white placeholder
        if (!bgfx::isValid(m_whiteTexture)) {
            uint32_t white = 0xFFFFFFFF;
            m_whiteTexture = bgfx::createTexture2D(1, 1, false, 1,
                bgfx::TextureFormat::BGRA8, BGFX_TEXTURE_NONE, bgfx::copy(&white, sizeof(white)));
        }
        bgfxTex = m_whiteTexture;
    }

    m_textureCache[texture] = bgfxTex;
    m_boundTextures[stage] = bgfxTex;
}

void BGFXBackend::Apply_Material_State(const VertexMaterialClass* material)
{
    m_currentMaterial = material;
    if (!material) return;

    Vector3 diffuse, specular, emissive, ambient;
    material->Get_Diffuse(&diffuse);
    material->Get_Specular(&specular);
    material->Get_Emissive(&emissive);
    material->Get_Ambient(&ambient);
    float opacity  = material->Get_Opacity();
    float shininess = material->Get_Shininess();

    float diff[4]  = { diffuse.X,  diffuse.Y,  diffuse.Z,  opacity };
    float spec[4]  = { specular.X, specular.Y, specular.Z, shininess };
    float emis[4]  = { emissive.X, emissive.Y, emissive.Z, 0.0f };
    float amb[4]   = { ambient.X,  ambient.Y,  ambient.Z,  0.0f };
    float op[4]    = { opacity, 0.0f, 0.0f, 0.0f };

    bgfx::setUniform(m_diffuseUniform,  diff);
    bgfx::setUniform(m_specularUniform, spec);
    bgfx::setUniform(m_emissiveUniform, emis);
    bgfx::setUniform(m_ambientUniform,  amb);
    bgfx::setUniform(m_opacityUniform,  op);
}

void BGFXBackend::Apply_Light_Environment_State(LightEnvironmentClass* env)
{
    m_currentLightEnv = env;
    float enable[4] = { env ? 1.0f : 0.0f, 0.0f, 0.0f, 0.0f };
    bgfx::setUniform(m_lightingEnableUniform, enable);

    if (!env) {
        // Zero out lights when disabled
        float zero[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        bgfx::setUniform(m_ambientLightUniform, zero);
        float zeroLights[4][4] = {};
        bgfx::setUniform(m_lightDirUniform, zeroLights, 4);
        bgfx::setUniform(m_lightDiffuseUniform, zeroLights, 4);
        return;
    }

    // Upload ambient light
    const Vector3& ambient = env->Get_Equivalent_Ambient();
    float amb[4] = { ambient.X, ambient.Y, ambient.Z, 1.0f };
    bgfx::setUniform(m_ambientLightUniform, amb);

    // Upload directional lights (up to 4)
    int lightCount = env->Get_Light_Count();
    float dirs[4][4];
    float diffs[4][4];
    float poss[4][4];
    for (int i = 0; i < 4; ++i) {
        if (i < lightCount) {
            const Vector3& dir = env->Get_Light_Direction(i);
            const Vector3& diff = env->Get_Light_Diffuse(i);
            dirs[i][0] = dir.X;  dirs[i][1] = dir.Y;  dirs[i][2] = dir.Z;  dirs[i][3] = 1.0f; // type=1 directional
            diffs[i][0] = diff.X; diffs[i][1] = diff.Y; diffs[i][2] = diff.Z; diffs[i][3] = 1.0f;
            poss[i][0] = poss[i][1] = poss[i][2] = poss[i][3] = 0.0f;
        } else {
            dirs[i][0] = dirs[i][1] = dirs[i][2] = dirs[i][3] = 0.0f;
            diffs[i][0] = diffs[i][1] = diffs[i][2] = diffs[i][3] = 0.0f;
            poss[i][0] = poss[i][1] = poss[i][2] = poss[i][3] = 0.0f;
        }
    }
    bgfx::setUniform(m_lightDirUniform, dirs, 4);
    bgfx::setUniform(m_lightDiffuseUniform, diffs, 4);
    bgfx::setUniform(m_lightPosUniform, poss, 4);
}

// Store individual DX8 lights for sorting-renderer path
static Vector3 s_dx8LightDirs[4];
static Vector3 s_dx8LightDiffuses[4];
static Vector3 s_dx8LightPoss[4];
static float   s_dx8LightRanges[4];
static int     s_dx8LightTypes[4];
static bool    s_dx8LightEnabled[4] = { false, false, false, false };

void BGFXBackend::Set_DX8_Light(int index, const void* light)
{
    if (index < 0 || index >= 4) return;

    if (!light) {
        s_dx8LightEnabled[index] = false;
    } else {
        const D3DLIGHT9* d3dLight = static_cast<const D3DLIGHT9*>(light);
        s_dx8LightEnabled[index] = true;
        s_dx8LightDirs[index] = Vector3(d3dLight->Direction.x, d3dLight->Direction.y, d3dLight->Direction.z);
        s_dx8LightDiffuses[index] = Vector3(d3dLight->Diffuse.r, d3dLight->Diffuse.g, d3dLight->Diffuse.b);
        s_dx8LightPoss[index] = Vector3(d3dLight->Position.x, d3dLight->Position.y, d3dLight->Position.z);
        s_dx8LightRanges[index] = d3dLight->Range;
        s_dx8LightTypes[index] = (int)d3dLight->Type; // 1=dir, 2=point, 3=spot
    }

    // Upload to shader immediately (bgfx uniforms are per-submit)
    float enable[4] = { 1.0f, 0.0f, 0.0f, 0.0f };
    bgfx::setUniform(m_lightingEnableUniform, enable);

    float dirs[4][4];
    float diffs[4][4];
    float poss[4][4];
    for (int i = 0; i < 4; ++i) {
        if (s_dx8LightEnabled[i]) {
            dirs[i][0] = s_dx8LightDirs[i].X;  dirs[i][1] = s_dx8LightDirs[i].Y;  dirs[i][2] = s_dx8LightDirs[i].Z;  dirs[i][3] = (float)s_dx8LightTypes[i];
            diffs[i][0] = s_dx8LightDiffuses[i].X; diffs[i][1] = s_dx8LightDiffuses[i].Y; diffs[i][2] = s_dx8LightDiffuses[i].Z; diffs[i][3] = 1.0f;
            poss[i][0] = s_dx8LightPoss[i].X;  poss[i][1] = s_dx8LightPoss[i].Y;  poss[i][2] = s_dx8LightPoss[i].Z;  poss[i][3] = s_dx8LightRanges[i];
        } else {
            dirs[i][0] = dirs[i][1] = dirs[i][2] = dirs[i][3] = 0.0f;
            diffs[i][0] = diffs[i][1] = diffs[i][2] = diffs[i][3] = 0.0f;
            poss[i][0] = poss[i][1] = poss[i][2] = poss[i][3] = 0.0f;
        }
    }
    bgfx::setUniform(m_lightDirUniform, dirs, 4);
    bgfx::setUniform(m_lightDiffuseUniform, diffs, 4);
    bgfx::setUniform(m_lightPosUniform, poss, 4);
}

void BGFXBackend::Set_Transform_World(const Matrix4& m)
{
    m_worldMatrix = m;
    m_worldDirty = true;
}

void BGFXBackend::Set_Transform_View(const Matrix4& m)
{
    m_viewMatrix = m;
    m_viewDirty = true;
}

void BGFXBackend::Set_Transform_Projection(const Matrix4& m)
{
    m_projectionMatrix = m;
    m_projectionDirty = true;
}

static int GetTexCoordSize(unsigned fvf, int index)
{
    // D3DFVF texcoord sizes are encoded in 2-bit fields starting at bit 16.
    // 0=1 component, 1=2, 2=3, 3=4.
    static const int sizeTable[4] = { 1, 2, 3, 4 };
    int shift = 16 + index * 2;
    int bits = (fvf >> shift) & 0x3;
    return sizeTable[bits];
}

static bool BuildVertexLayoutFromFVF(unsigned fvf, bgfx::VertexLayout& layout)
{
    layout.begin();

    unsigned posType = fvf & D3DFVF_POSITION_MASK;

    if (posType == D3DFVF_XYZ) {
        layout.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float);
    } else if (posType == D3DFVF_XYZRHW) {
        layout.add(bgfx::Attrib::Position, 4, bgfx::AttribType::Float);
    } else if (posType >= D3DFVF_XYZB1 && posType <= D3DFVF_XYZB5) {
        int blendCount = (posType - D3DFVF_XYZB1) / 2 + 1;
        layout.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float);
        if (fvf & D3DFVF_LASTBETA_UBYTE4) {
            // All but the last weight are floats; the last is UBYTE4 bone indices.
            for (int i = 0; i < blendCount - 1; ++i) {
                layout.add(bgfx::Attrib::Weight, 1, bgfx::AttribType::Float);
            }
            layout.add(bgfx::Attrib::Indices, 4, bgfx::AttribType::Uint8, true);
        } else {
            for (int i = 0; i < blendCount; ++i) {
                layout.add(bgfx::Attrib::Weight, 1, bgfx::AttribType::Float);
            }
        }
    }

    if (fvf & D3DFVF_NORMAL) {
        layout.add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float);
    }

    // Point size not supported in modern bgfx — skipped
    // if (fvf & D3DFVF_PSIZE) {
    //     layout.add(bgfx::Attrib::PointSize, 1, bgfx::AttribType::Float);
    // }

    if (fvf & D3DFVF_DIFFUSE) {
        layout.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true);
    }

    if (fvf & D3DFVF_SPECULAR) {
        layout.add(bgfx::Attrib::Color1, 4, bgfx::AttribType::Uint8, true);
    }

    // Texture coordinates — respect per-set component counts.
    int texCount = (fvf & D3DFVF_TEXCOUNT_MASK) >> D3DFVF_TEXCOUNT_SHIFT;
    for (int i = 0; i < texCount && i < 8; ++i) {
        int components = GetTexCoordSize(fvf, i);
        bgfx::Attrib::Enum attr = (bgfx::Attrib::Enum)(bgfx::Attrib::TexCoord0 + i);
        layout.add(attr, components, bgfx::AttribType::Float);
    }

    layout.end();
    return layout.getStride() > 0;
}

void BGFXBackend::Set_Vertex_Buffer(const VertexBufferClass* vb)
{
    if (!vb) {
        m_currentVB = nullptr;
        return;
    }
    m_currentVB = vb;

    bool isDynamic = (vb->Type() == BUFFER_TYPE_DYNAMIC_DX8 || vb->Type() == BUFFER_TYPE_DYNAMIC_SORTING);

    if (!isDynamic) {
        auto it = m_vbCache.find(vb);
        if (it != m_vbCache.end()) {
            bgfx::setVertexBuffer(0, it->second);
            return;
        }
    }

    // Lock and copy data
    VertexBufferClass::WriteLockClass lock(const_cast<VertexBufferClass*>(vb));
    void* data = lock.Get_Vertex_Array();
    int count = vb->Get_Vertex_Count();

    bgfx::VertexLayout layout;
    if (!BuildVertexLayoutFromFVF(vb->FVF_Info().Get_FVF(), layout)) {
        WWDEBUG_SAY(("BGFXBackend: unsupported FVF format for vertex buffer\n"));
        return;
    }

    if (isDynamic) {
        // Transient buffers for per-frame dynamic data
        bgfx::TransientVertexBuffer tvb;
        bgfx::allocTransientVertexBuffer(&tvb, count, layout);
        if (tvb.data) {
            memcpy(tvb.data, data, count * layout.getStride());
            bgfx::setVertexBuffer(0, &tvb);
        } else {
            WWDEBUG_SAY(("BGFXBackend: failed to allocate transient vertex buffer\n"));
        }
    } else {
        const bgfx::Memory* mem = bgfx::copy(data, count * layout.getStride());
        bgfx::VertexBufferHandle handle = bgfx::createVertexBuffer(mem, layout);
        m_vbCache[vb] = handle;
        bgfx::setVertexBuffer(0, handle);
    }
}

void BGFXBackend::Set_Index_Buffer(const IndexBufferClass* ib, unsigned short index_base_offset)
{
    if (!ib) {
        m_currentIB = nullptr;
        m_currentIBOffset = 0;
        return;
    }
    m_currentIB = ib;
    m_currentIBOffset = index_base_offset;

    bool isDynamic = (ib->Type() == BUFFER_TYPE_DYNAMIC_DX8 || ib->Type() == BUFFER_TYPE_DYNAMIC_SORTING);

    if (!isDynamic) {
        auto it = m_ibCache.find(ib);
        if (it != m_ibCache.end()) {
            bgfx::setIndexBuffer(it->second);
            return;
        }
    }

    IndexBufferClass::WriteLockClass lock(const_cast<IndexBufferClass*>(ib));
    unsigned short* data = lock.Get_Index_Array();
    int count = ib->Get_Index_Count();

    if (isDynamic) {
        bgfx::TransientIndexBuffer tib;
        bgfx::allocTransientIndexBuffer(&tib, count);
        if (tib.data) {
            memcpy(tib.data, data, count * sizeof(unsigned short));
            bgfx::setIndexBuffer(&tib);
        } else {
            WWDEBUG_SAY(("BGFXBackend: failed to allocate transient index buffer\n"));
        }
    } else {
        const bgfx::Memory* mem = bgfx::copy(data, count * sizeof(unsigned short));
        bgfx::IndexBufferHandle handle = bgfx::createIndexBuffer(mem);
        m_ibCache[ib] = handle;
        bgfx::setIndexBuffer(handle);
    }
}

void BGFXBackend::Draw_Indexed_Triangles(unsigned short start_index, unsigned short polygon_count,
                                         unsigned short min_vertex_index, unsigned short vertex_count)
{
    if (!bgfx::isValid(m_currentProgram)) {
        WWDEBUG_SAY(("BGFXBackend: no program set, skipping draw\n"));
        return;
    }

    // Set buffer ranges for this draw call
    // Try cache first, then fall back to current buffer pointers
    if (m_currentIB) {
        auto it = m_ibCache.find(m_currentIB);
        if (it != m_ibCache.end()) {
            bgfx::setIndexBuffer(it->second, start_index, polygon_count * 3);
        }
        // If not in cache, the buffer should have been set via Set_Index_Buffer already
    }
    if (m_currentVB) {
        auto it = m_vbCache.find(m_currentVB);
        if (it != m_vbCache.end()) {
            bgfx::setVertexBuffer(0, it->second, min_vertex_index, vertex_count);
        }
        // If not in cache, the buffer should have been set via Set_Vertex_Buffer already
    }

    bgfx::setState(m_currentState);
    bgfx::submit(0, m_currentProgram);
}

void BGFXBackend::Draw_Indexed_Strip(unsigned short start_index, unsigned short index_count,
                                      unsigned short min_vertex_index, unsigned short vertex_count)
{
    if (!bgfx::isValid(m_currentProgram)) {
        WWDEBUG_SAY(("BGFXBackend: no program set, skipping draw\n"));
        return;
    }

    if (m_currentIB) {
        auto it = m_ibCache.find(m_currentIB);
        if (it != m_ibCache.end()) {
            bgfx::setIndexBuffer(it->second, start_index, index_count);
        }
    }
    if (m_currentVB) {
        auto it = m_vbCache.find(m_currentVB);
        if (it != m_vbCache.end()) {
            bgfx::setVertexBuffer(0, it->second, min_vertex_index, vertex_count);
        }
    }

    bgfx::setState(m_currentState | BGFX_STATE_PT_TRISTRIP);
    bgfx::submit(0, m_currentProgram);
}

void BGFXBackend::Commit_Render_State()
{
    // Apply transforms
    if (m_worldDirty || m_viewDirty || m_projectionDirty) {
        float modelMtx[16];
        Matrix4ToFloat16Transpose(m_worldMatrix, modelMtx);
        bgfx::setUniform(m_modelUniform, modelMtx);

        // Compute viewProj = view * proj (D3D row-vector convention), then transpose for GLSL
        Matrix4 viewProj = m_viewMatrix * m_projectionMatrix;
        float vpMtx[16];
        Matrix4ToFloat16Transpose(viewProj, vpMtx);
        bgfx::setUniform(m_viewProjUniform, vpMtx);

        // Extract camera position from view matrix
        // View matrix: t = -R * eye  =>  eye = -R^T * t
        const Matrix4& V = m_viewMatrix;
        float tx = V[0][3], ty = V[1][3], tz = V[2][3];
        float camPos[4];
        camPos[0] = -(tx * V[0][0] + ty * V[1][0] + tz * V[2][0]);
        camPos[1] = -(tx * V[0][1] + ty * V[1][1] + tz * V[2][1]);
        camPos[2] = -(tx * V[0][2] + ty * V[1][2] + tz * V[2][2]);
        camPos[3] = 1.0f;
        bgfx::setUniform(m_camPosUniform, camPos);

        m_worldDirty = false;
        m_viewDirty = false;
        m_projectionDirty = false;
    }

    // Ensure default program is set
    if (bgfx::isValid(m_defaultProgram)) {
        // Program is bound per-draw via submit()
    }
}
