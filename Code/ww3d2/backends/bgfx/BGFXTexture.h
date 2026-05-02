// BGFXTexture.h - OpenW3D render backend

#ifndef BGFXTEXTURE_H
#define BGFXTEXTURE_H

#include "bgfx/bgfx.h"
#include "vector3.h"

// Texture format enumeration matching common WW3D usage
enum class BGFXTextureFormat
{
    RGBA8,
    RGB8,
    BGRA8,
    RGBA16F,
    RGBA32F,
    BC1,    // DXT1
    BC2,    // DXT3
    BC3,    // DXT5
    R8,
    R16F,
    Depth16,
    Depth24,
    Depth32F,
};

// Sampler state flags
struct BGFXSamplerState
{
    enum Filter
    {
        Point = 0,
        Linear = 1,
        Anisotropic = 2,
    };

    enum AddressMode
    {
        Wrap = 0,
        Clamp = 1,
        Mirror = 2,
        Border = 3,
    };

    Filter MinFilter;
    Filter MagFilter;
    Filter MipFilter;
    AddressMode AddressU;
    AddressMode AddressV;
    AddressMode AddressW;
    int MaxAnisotropy;
    Vector3 BorderColor;
    float MipLODBias;

    BGFXSamplerState()
        : MinFilter(Linear)
        , MagFilter(Linear)
        , MipFilter(Linear)
        , AddressU(Wrap)
        , AddressV(Wrap)
        , AddressW(Wrap)
        , MaxAnisotropy(1)
        , BorderColor(0.0f, 0.0f, 0.0f)
        , MipLODBias(0.0f)
    {}

    // Convert to bgfx sampler flags
    uint32_t To_BGFX_Flags() const;
};

// BGFXTexture - wraps a bgfx texture
class BGFXTexture
{
public:
    BGFXTexture();
    ~BGFXTexture();

    BGFXTexture(const BGFXTexture&) = delete;
    BGFXTexture& operator=(const BGFXTexture&) = delete;
    BGFXTexture(BGFXTexture&&) = delete;
    BGFXTexture& operator=(BGFXTexture&&) = delete;

    // Create from raw pixel data
    bool Create(int width, int height, BGFXTextureFormat format, void* data, int mipCount = 1);

    // Create from raw pixel data with explicit bgfx format
    bool Create(int width, int height, bgfx::TextureFormat::Enum format, void* data, int mipCount = 1, uint64_t flags = BGFX_TEXTURE_NONE);

    // Create as render target
    bool Create_Render_Target(int width, int height, bgfx::TextureFormat::Enum format, uint64_t flags = BGFX_TEXTURE_RT_WRITE_ONLY);

    // Create depth buffer
    bool Create_Depth(int width, int height, bgfx::TextureFormat::Enum format = bgfx::TextureFormat::D24S8, uint64_t flags = BGFX_TEXTURE_RT_WRITE_ONLY);

    // Destroy the texture
    void Destroy();

    // Check if texture is valid
    bool Is_Valid() const { return bgfx::isValid(m_handle); }

    // Get the bgfx handle
    bgfx::TextureHandle Get_Handle() const { return m_handle; }

    // Get dimensions
    int Get_Width() const { return m_width; }
    int Get_Height() const { return m_height; }

    // Get format
    bgfx::TextureFormat::Enum Get_Format() const { return m_format; }

    // Update texture data (for dynamic textures)
    void Update(int x, int y, int width, int height, void* data);

    // Generate mipmaps
    void Generate_Mips();

    // Set sampler state
    void Set_Sampler_State(const BGFXSamplerState& state);
    BGFXSamplerState Get_Sampler_State() const { return m_samplerState; }

    // Get current sampler flags
    uint32_t Get_Sampler_Flags() const { return m_samplerState.To_BGFX_Flags(); }

    // Static helper: Convert WW3D surface format to bgfx texture format
    static bgfx::TextureFormat::Enum WW3D_To_BGFX_Format(int ww3dFormat);

    // Static helper: Get bytes per pixel for a format
    static int Get_Bytes_Per_Pixel(bgfx::TextureFormat::Enum format);

    // Static helper: Get total data size for a texture (handles compressed formats)
    static uint32_t Get_Texture_Data_Size(int width, int height, bgfx::TextureFormat::Enum format);

private:
    bgfx::TextureHandle m_handle;
    int m_width;
    int m_height;
    bgfx::TextureFormat::Enum m_format;
    int m_mipCount;
    BGFXSamplerState m_samplerState;
    uint64_t m_flags;
};

#endif // BGFXTEXTURE_H
