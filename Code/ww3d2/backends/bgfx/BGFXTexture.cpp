// BGFXTexture.cpp - OpenW3D render backend

#include "backends/bgfx/BGFXTexture.h"

BGFXTexture::BGFXTexture()
    : m_handle(BGFX_INVALID_HANDLE)
    , m_width(0)
    , m_height(0)
    , m_format(bgfx::TextureFormat::RGBA8)
    , m_mipCount(1)
    , m_flags(BGFX_TEXTURE_NONE)
{
}

BGFXTexture::~BGFXTexture()
{
    Destroy();
}

bool BGFXTexture::Create(int width, int height, BGFXTextureFormat format, void* data, int mipCount)
{
    bgfx::TextureFormat::Enum bgfxFormat = bgfx::TextureFormat::RGBA8;
    switch (format) {
        case BGFXTextureFormat::RGBA8:      bgfxFormat = bgfx::TextureFormat::RGBA8; break;
        case BGFXTextureFormat::RGB8:       bgfxFormat = bgfx::TextureFormat::RGB8; break;
        case BGFXTextureFormat::BGRA8:      bgfxFormat = bgfx::TextureFormat::BGRA8; break;
        case BGFXTextureFormat::RGBA16F:    bgfxFormat = bgfx::TextureFormat::RGBA16F; break;
        case BGFXTextureFormat::RGBA32F:    bgfxFormat = bgfx::TextureFormat::RGBA32F; break;
        case BGFXTextureFormat::BC1:        bgfxFormat = bgfx::TextureFormat::BC1; break;
        case BGFXTextureFormat::BC2:        bgfxFormat = bgfx::TextureFormat::BC2; break;
        case BGFXTextureFormat::BC3:        bgfxFormat = bgfx::TextureFormat::BC3; break;
        case BGFXTextureFormat::R8:         bgfxFormat = bgfx::TextureFormat::R8; break;
        case BGFXTextureFormat::R16F:       bgfxFormat = bgfx::TextureFormat::R16F; break;
        case BGFXTextureFormat::Depth16:    bgfxFormat = bgfx::TextureFormat::D16; break;
        case BGFXTextureFormat::Depth24:    bgfxFormat = bgfx::TextureFormat::D24; break;
        case BGFXTextureFormat::Depth32F:   bgfxFormat = bgfx::TextureFormat::D32F; break;
        default:                            bgfxFormat = bgfx::TextureFormat::RGBA8; break;
    }
    return Create(width, height, bgfxFormat, data, mipCount, BGFX_TEXTURE_NONE);
}

bool BGFXTexture::Create(int width, int height, bgfx::TextureFormat::Enum format, void* data, int mipCount, uint64_t flags)
{
    Destroy();

    if (width <= 0 || height <= 0) {
        return false;
    }

    m_width = width;
    m_height = height;
    m_format = format;
    m_mipCount = mipCount;
    m_flags = flags;

    const bgfx::Memory* mem = nullptr;
    if (data) {
        // Calculate size based on format
        uint32_t size = Get_Texture_Data_Size(width, height, format);
        mem = bgfx::copy(data, size);
    }

    m_handle = bgfx::createTexture2D(
        static_cast<uint16_t>(width),
        static_cast<uint16_t>(height),
        false,  // hasMips - we'll handle mips separately
        1,      // numLayers
        format,
        flags,
        mem
    );

    return bgfx::isValid(m_handle);
}

bool BGFXTexture::Create_Render_Target(int width, int height, bgfx::TextureFormat::Enum format, uint64_t flags)
{
    return Create(width, height, format, nullptr, 1, flags | BGFX_TEXTURE_RT_WRITE_ONLY);
}

bool BGFXTexture::Create_Depth(int width, int height, bgfx::TextureFormat::Enum format, uint64_t flags)
{
    return Create(width, height, format, nullptr, 1, flags | BGFX_TEXTURE_RT_WRITE_ONLY);
}

void BGFXTexture::Destroy()
{
    if (bgfx::isValid(m_handle)) {
        bgfx::destroy(m_handle);
        m_handle = BGFX_INVALID_HANDLE;
    }
    m_width = 0;
    m_height = 0;
    m_mipCount = 1;
}

void BGFXTexture::Update(int x, int y, int width, int height, void* data)
{
    if (!bgfx::isValid(m_handle) || !data) {
        return;
    }

    uint32_t size = Get_Texture_Data_Size(width, height, m_format);
    const bgfx::Memory* mem = bgfx::copy(data, size);

    bgfx::updateTexture2D(
        m_handle,
        0,  // mip level
        0,  // layer
        static_cast<uint16_t>(x),
        static_cast<uint16_t>(y),
        static_cast<uint16_t>(width),
        static_cast<uint16_t>(height),
        mem
    );
}

void BGFXTexture::Generate_Mips()
{
    // bgfx handles mip generation internally for most formats
    // This is a placeholder for explicit mip generation if needed
}

void BGFXTexture::Set_Sampler_State(const BGFXSamplerState& state)
{
    m_samplerState = state;
}

uint32_t BGFXSamplerState::To_BGFX_Flags() const
{
    uint32_t flags = 0;

    // Min filter
    switch (MinFilter) {
        case Point:        flags |= BGFX_SAMPLER_MIN_POINT; break;
        case Linear:       flags |= BGFX_SAMPLER_MIN_ANISOTROPIC; break;  // Use anisotropic for linear
        case Anisotropic:  flags |= BGFX_SAMPLER_MIN_ANISOTROPIC; break;
    }

    // Mag filter
    switch (MagFilter) {
        case Point:        flags |= BGFX_SAMPLER_MAG_POINT; break;
        case Linear:       break;  // Linear is default
        case Anisotropic:  break;  // Linear is default for mag
    }

    // Mip filter
    switch (MipFilter) {
        case Point:        flags |= BGFX_SAMPLER_MIP_POINT; break;
        case Linear:       break;  // Linear mip is default
        case Anisotropic:  break;  // Linear mip is default
    }

    // Address modes
    switch (AddressU) {
        case Wrap:     break;  // Wrap is default (no flag needed)
        case Clamp:    flags |= BGFX_SAMPLER_U_CLAMP; break;
        case Mirror:   flags |= BGFX_SAMPLER_U_MIRROR; break;
        case Border:   flags |= BGFX_SAMPLER_U_BORDER; break;
    }

    switch (AddressV) {
        case Wrap:     break;  // Wrap is default (no flag needed)
        case Clamp:    flags |= BGFX_SAMPLER_V_CLAMP; break;
        case Mirror:   flags |= BGFX_SAMPLER_V_MIRROR; break;
        case Border:   flags |= BGFX_SAMPLER_V_BORDER; break;
    }

    switch (AddressW) {
        case Wrap:     break;  // Wrap is default (no flag needed)
        case Clamp:    flags |= BGFX_SAMPLER_W_CLAMP; break;
        case Mirror:   flags |= BGFX_SAMPLER_W_MIRROR; break;
        case Border:   flags |= BGFX_SAMPLER_W_BORDER; break;
    }

    return flags;
}

bgfx::TextureFormat::Enum BGFXTexture::WW3D_To_BGFX_Format(int ww3dFormat)
{
    // Map common WW3D surface formats to bgfx
    // These are placeholder mappings - adjust based on actual WW3D format values
    switch (ww3dFormat) {
        case 0:  // WW3D_FORMAT_RGBA8
            return bgfx::TextureFormat::RGBA8;
        case 1:  // WW3D_FORMAT_RGB8
            return bgfx::TextureFormat::RGB8;
        case 2:  // WW3D_FORMAT_BGRA8
            return bgfx::TextureFormat::BGRA8;
        case 3:  // WW3D_FORMAT_DXT1
            return bgfx::TextureFormat::BC1;
        case 4:  // WW3D_FORMAT_DXT3
            return bgfx::TextureFormat::BC2;
        case 5:  // WW3D_FORMAT_DXT5
            return bgfx::TextureFormat::BC3;
        default:
            return bgfx::TextureFormat::RGBA8;
    }
}

int BGFXTexture::Get_Bytes_Per_Pixel(bgfx::TextureFormat::Enum format)
{
    switch (format) {
        case bgfx::TextureFormat::R8:       return 1;
        case bgfx::TextureFormat::RG8:      return 2;
        case bgfx::TextureFormat::RGB8:     return 3;
        case bgfx::TextureFormat::RGBA8:    return 4;
        case bgfx::TextureFormat::BGRA8:    return 4;
        case bgfx::TextureFormat::R16F:     return 2;
        case bgfx::TextureFormat::RG16F:    return 4;
        case bgfx::TextureFormat::RGBA16F:  return 8;
        case bgfx::TextureFormat::R32F:     return 4;
        case bgfx::TextureFormat::RG32F:    return 8;
        case bgfx::TextureFormat::RGBA32F:  return 16;
        case bgfx::TextureFormat::D16:      return 2;
        case bgfx::TextureFormat::D24:      return 3;
        case bgfx::TextureFormat::D24S8:    return 4;
        case bgfx::TextureFormat::D32F:     return 4;
        case bgfx::TextureFormat::BC1:      return 0;  // Compressed
        case bgfx::TextureFormat::BC2:      return 0;  // Compressed
        case bgfx::TextureFormat::BC3:      return 0;  // Compressed
        default:                            return 4;
    }
}

uint32_t BGFXTexture::Get_Texture_Data_Size(int width, int height, bgfx::TextureFormat::Enum format)
{
    switch (format) {
        case bgfx::TextureFormat::BC1:
            return ((width + 3) / 4) * ((height + 3) / 4) * 8;
        case bgfx::TextureFormat::BC2:
        case bgfx::TextureFormat::BC3:
            return ((width + 3) / 4) * ((height + 3) / 4) * 16;
        default:
            return width * height * Get_Bytes_Per_Pixel(format);
    }
}
