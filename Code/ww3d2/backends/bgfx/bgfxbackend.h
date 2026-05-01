// bgfxbackend.h - OpenW3D render backend

#ifndef BGFXBACKEND_H
#define BGFXBACKEND_H

#include "ww3dbackend.h"
#include "rddesc.h"
#include "matrix4.h"

#include <bgfx/bgfx.h>

#include <array>
#include <vector>
#include <unordered_map>

// Include material mapper for integration
#include "BGFXMaterialMapper.h"

// Include shader key and variant cache
#include "ShaderKey.h"
#include "ShaderVariantCache.h"

// Common DX8 render state values that we track
namespace DX8RenderState
{
    enum {
        FILLMODE = 8,
        BLENDENABLE = 19,
        SRCBLEND = 60,
        DESTBLEND = 61,
        BLENDOP = 62,
        CULLMODE = 22,
        ZENABLE = 7,
        ZWRITEENABLE = 14,
        ALPHABLENDENABLE = 19,
        ALPHATESTENABLE = 24,
        STENCILENABLE = 52,
        LIGHTING = 137,
        DIFFUSEMATERIALSOURCE = 145,
        AMBIENTMATERIALSOURCE = 146,
        EMISSIVEMATERIALSOURCE = 147,
        SPECULARMATERIALSOURCE = 148,
        SHADEMODE = 134,
        COLORVERTEX = 141,

        // Fog states
        FOGENABLE = 27,
        FOGCOLOR = 30,
        FOGSTART = 31,
        FOGEND = 32,
        FOGDENSITY = 33,
        RANGEFOGENABLE = 38,

        // Stencil states
        STENCILFAIL = 53,
        STENCILZFAIL = 54,
        STENCILPASS = 55,
        STENCILFUNC = 56,
        STENCILREF = 57,
        STENCILMASK = 58,
        STENCILWRITEMASK = 59,

        // Alpha test reference and func (D3DRS_ALPHAREF=24, D3DRS_ALPHAFUNC=25)
        // Note: ALPHATESTENABLE in this enum is misnamed (value 24 is actually ALPHAREF in D3D)
        // Kept for compatibility with existing code
        ALPHAREF = 25,

        // Depth bias states
        SLOPESCALEDEPTHBIAS = 175,
        DEPTHBIAS = 195,
    };

    enum FillMode {
        SOLID = 3,
        WIREFRAME = 2,
        POINT = 1,
    };

    enum CullMode {
        NONE = 1,
        CW = 2,
        CCW = 3,
    };

    enum BlendOp {
        ADD = 0,
        SUBTRACT = 1,
        REVSUBTRACT = 2,
        MIN = 3,
        MAX = 4,
    };

    enum BlendFactor {
        ZERO = 0,
        ONE = 1,
        SRCCOLOR = 2,
        INVERTSRCCOLOR = 3,
        SRCALPHA = 4,
        INVERTSRCALPHA = 5,
        DESTALPHA = 6,
        INVERTDESTALPHA = 7,
        DESTCOLOR = 8,
        INVERTDESTCOLOR = 9,
        SRCALPHASAT = 10,
        CONSTANTCOLOR = 11,
        ONEMINUSCONSTANTCOLOR = 12,
        CONSTANTALPHA = 13,
        ONEMINUSCONSTANTALPHA = 14,
    };
}

// Forward declarations
class BGFXTexture;
class BGFXVertexBuffer;
class BGFXIndexBuffer;

// BGFX Backend implementation
class BGFXBackend : public WW3DBackend
{
public:
    BGFXBackend();
    virtual ~BGFXBackend();

    // WW3DBackend interface
    virtual bool Init(void * hwnd, bool lite = false) override;
    virtual void Shutdown() override;

    virtual bool Set_Any_Render_Device() override;
    virtual bool Set_Render_Device(const char * dev_name, int width = -1, int height = -1, int bits = -1, int windowed = -1, bool resize_window = false) override;
    virtual bool Set_Render_Device(int dev = -1, int resx = -1, int resy = -1, int bits = -1, int windowed = -1, bool resize_window = false) override;
    virtual bool Set_Next_Render_Device() override;
    virtual bool Toggle_Windowed() override;
    virtual bool Is_Windowed() override;

    virtual int Get_Render_Device_Count() override;
    virtual int Get_Render_Device() override;
    virtual const RenderDeviceDescClass & Get_Render_Device_Desc(int deviceidx) override;
    virtual const char * Get_Render_Device_Name(int device_index) override;
    virtual bool Set_Device_Resolution(int width = -1, int height = -1, int bits = -1, int windowed = -1, bool resize_window = false) override;
    virtual void Get_Device_Resolution(int & set_w, int & set_h, int & set_bits, bool & set_windowed) override;
    virtual void Get_Render_Target_Resolution(int & set_w, int & set_h, int & set_bits, bool & set_windowed) override;
    virtual int Get_Device_Resolution_Width() override;
    virtual int Get_Device_Resolution_Height() override;

    virtual bool Registry_Save_Render_Device(const char * sub_key) override;
    virtual bool Registry_Load_Render_Device(const char * sub_key, bool resize_window) override;
    virtual bool Registry_Save_Render_Device(const char *sub_key, int device, int width, int height, int depth, bool windowed, int texture_depth) override;
    virtual bool Registry_Load_Render_Device(const char * sub_key, char *device, int device_len, int &width, int &height, int &depth, int &windowed, int &texture_depth) override;

    virtual void Begin_Scene() override;
    virtual void End_Scene(bool flip_frame = true) override;
    virtual void Flip_To_Primary() override;

    virtual void Clear(bool clear_color, bool clear_z_stencil, const Vector3 &color, float z = 1.0f, unsigned int stencil = 0) override;

    virtual void Set_Swap_Interval(int swap) override;
    virtual int Get_Swap_Interval() override;

    virtual void Set_Texture_Bitdepth(int depth) override;
    virtual int Get_Texture_Bitdepth() override;

    virtual void Set_Viewport(const void* viewport) override;
    void Set_Viewport(int x, int y, int width, int height, float minZ = 0.0f, float maxZ = 1.0f);
    virtual void Set_DX8_Render_State(int state, unsigned value) override;
    virtual void Set_Light_Environment(const void* env) override;
    virtual void* _Get_DX8_Front_Buffer() override;

    // Backend dispatch overrides from WW3DBackend
    virtual bool Wants_Deferred_State_Apply() const override { return true; }
    virtual void Apply_Shader_State(const ShaderClass& shader) override;
    virtual void Apply_Texture_State(unsigned stage, TextureClass* texture) override;
    virtual void Invalidate_Texture_Cache_Entry(TextureClass* texture) override;
    virtual void Apply_Material_State(const VertexMaterialClass* material) override;
    virtual void Apply_Light_Environment_State(LightEnvironmentClass* env) override;
    virtual void Set_DX8_Light(int index, const void* light) override;
    virtual void Set_Transform_World(const Matrix4& m) override;
    virtual void Set_Transform_View(const Matrix4& m) override;
    virtual void Set_Transform_Projection(const Matrix4& m) override;
    virtual void Set_Vertex_Buffer(const VertexBufferClass* vb) override;
    virtual void Set_Index_Buffer(const IndexBufferClass* ib, unsigned short index_base_offset) override;
    virtual void Draw_Indexed_Triangles(unsigned short start_index, unsigned short polygon_count, unsigned short min_vertex_index, unsigned short vertex_count) override;
    virtual void Draw_Indexed_Strip(unsigned short start_index, unsigned short index_count, unsigned short min_vertex_index, unsigned short vertex_count) override;
    virtual void Commit_Render_State() override;

    // Texture management
    bgfx::TextureHandle Create_System_Texture(int width, int height, bgfx::TextureFormat::Enum format, uint64_t flags = BGFX_TEXTURE_NONE);
    void Destroy_Texture(bgfx::TextureHandle handle);
    void Set_Texture(int stage, bgfx::TextureHandle handle, uint32_t flags = UINT32_MAX, uint8_t mip = 0);

    // Vertex buffer management
    bgfx::VertexBufferHandle Create_Vertex_Buffer(void* data, int vertexCount, const bgfx::VertexLayout& layout, uint64_t flags = BGFX_BUFFER_NONE);
    void Destroy_Vertex_Buffer(bgfx::VertexBufferHandle handle);
    void Set_Vertex_Buffer(int stream, bgfx::VertexBufferHandle handle, uint32_t startVertex = 0, uint32_t numVertices = UINT32_MAX);

    // Index buffer management
    bgfx::IndexBufferHandle Create_Index_Buffer(void* data, int indexCount, bool index32bit = false, uint64_t flags = BGFX_BUFFER_NONE);
    void Destroy_Index_Buffer(bgfx::IndexBufferHandle handle);
    void Set_Index_Buffer(bgfx::IndexBufferHandle handle, int startIndex = 0, int numIndices = INT32_MAX);

    // Draw commands
    void Draw(int numVertices, int numInstances = 1);
    void Draw_Indexed(int numIndices, int numInstances = 1, int startIndex = 0, int startVertex = 0);

    // Shader program management
    bgfx::ShaderHandle Load_Shader(const char* shaderPath);
    bgfx::ProgramHandle Create_Program(const char* vsPath, const char* fsPath);
    bgfx::ProgramHandle Create_Program(bgfx::ShaderHandle vs, bgfx::ShaderHandle fs);
    void Destroy_Shader(bgfx::ShaderHandle handle);
    void Destroy_Program(bgfx::ProgramHandle handle);
    void Set_Program(bgfx::ProgramHandle program);

    // Uniform (constant buffer) management
    bgfx::UniformHandle Create_Uniform(const char* name, bgfx::UniformType::Enum type, uint16_t num = 1);
    void Destroy_Uniform(bgfx::UniformHandle handle);
    void Set_Uniform(bgfx::UniformHandle handle, const void* value, uint16_t num = 1);

    // Scissor rect management
    void Enable_Scissor(bool enable);
    void Set_Scissor(int x, int y, int width, int height);
    bool Is_Scissor_Enabled() const;

    // Frame buffer (render target) management
    bgfx::FrameBufferHandle Create_FrameBuffer(int width, int height, bgfx::TextureFormat::Enum format, uint64_t textureFlags = BGFX_TEXTURE_RT_WRITE_ONLY);
    bgfx::FrameBufferHandle Create_FrameBuffer(int width, int height, bgfx::TextureFormat::Enum colorFormat, bgfx::TextureFormat::Enum depthFormat, uint64_t textureFlags = BGFX_TEXTURE_RT_WRITE_ONLY);
    bgfx::FrameBufferHandle Create_FrameBuffer_Attachment(bgfx::TextureHandle color, bgfx::TextureHandle depth);
    void Destroy_FrameBuffer(bgfx::FrameBufferHandle handle);
    void Set_FrameBuffer(bgfx::FrameBufferHandle handle, int width = -1, int height = -1);
    void Set_FrameBuffer(bgfx::FrameBufferHandle handle, bgfx::TextureHandle color, bgfx::TextureHandle depth);
    void Set_Default_FrameBuffer();

    // Resource cache management
    void Clear_Resource_Caches();

    // Material integration - map ww3d2 material properties to bgfx
    void Set_Material_Colors(const Vector3& diffuse, float alpha, const Vector3& specular, float shininess, const Vector3& emissive, const Vector3& ambient);
    void Set_Material_Uniforms(bgfx::UniformHandle diffuseHandle, bgfx::UniformHandle specularHandle, bgfx::UniformHandle emissiveHandle, const MaterialColors& colors);
    void Set_Texture_Stage(int stage, bgfx::TextureHandle handle, uint32_t flags = UINT32_MAX, uint8_t mip = 0, int uvIndex = 0);
    uint64_t Apply_Shader(const ShaderClass& shader, bool lightingEnabled);
    void Begin_Material_Pass();
    void End_Material_Pass();

    // Renderer info
    const char* Get_Renderer_Name();
    bgfx::RendererType::Enum Get_Renderer_Type();

private:
    static constexpr int MAX_TEXTURE_STAGES = 4;

    void Update_ViewDimensions();
    void Apply_Render_State(int state, unsigned value);

    // Helper: Map DX8 stencil comparison function to bgfx stencil func
    static uint32_t Compare_Stencil_Func(unsigned dx8func);
    // Helper: Map DX8 stencil operation to bgfx stencil op (stencil fail)
    static uint32_t Compare_Stencil_Op(unsigned dx8op);
    // Helper: Map DX8 stencil operation to bgfx stencil op (depth fail)
    static uint32_t Compare_Stencil_Op_Z(unsigned dx8op);
    // Helper: Map DX8 stencil operation to bgfx stencil op (depth pass)
    static uint32_t Compare_Stencil_Op_Pass(unsigned dx8op);

    // Convert from W3D color format (0-1 float) to 0xAABBGGRR in little-endian memory
    static uint32_t Color_To_ABGR(const Vector3& color)
    {
        uint8_t r = static_cast<uint8_t>(color.X * 255.0f);
        uint8_t g = static_cast<uint8_t>(color.Y * 255.0f);
        uint8_t b = static_cast<uint8_t>(color.Z * 255.0f);
        return 0xFF000000 | (r << 16) | (g << 8) | b;
    }

    // Window/Surface
    void* m_hwnd;
    void* m_nativeDisplay;
    int m_width;
    int m_height;
    int m_bitDepth;
    bool m_windowed;

    // Device enumeration
    std::vector<std::string> m_deviceNames;
    int m_currentDevice;

    // Swap/VSync
    int m_swapInterval;
    int m_textureBitDepth;

    // DX8 render state cache - maps state enum to value
    std::unordered_map<int, unsigned> m_render_state;

    // Internal bgfx state
    bgfx::TextureHandle m_systemTexture;
    bgfx::FrameBufferHandle m_frameBuffer;
    bgfx::ProgramHandle m_currentProgram;
    bgfx::TextureHandle m_whiteTexture;
    bool m_initialized;

    // Current bgfx state for tracking changes
    uint64_t m_currentState;
    uint32_t m_currentColor;

    // Viewport state
    int m_viewportX;
    int m_viewportY;
    int m_viewportWidth;
    int m_viewportHeight;
    float m_viewportMinZ;
    float m_viewportMaxZ;

    // Scissor state
    bool m_scissorEnabled;
    int m_scissorX;
    int m_scissorY;
    int m_scissorWidth;
    int m_scissorHeight;

    // Active framebuffer (0 = default/backbuffer)
    bgfx::FrameBufferHandle m_activeFrameBuffer;

    // Sampler + texture stage state
    std::array<bgfx::UniformHandle, MAX_TEXTURE_STAGES> m_samplerUniforms;
    std::array<bgfx::TextureHandle, MAX_TEXTURE_STAGES> m_boundTextures;
    std::array<uint32_t, MAX_TEXTURE_STAGES> m_textureFlags;
    std::array<uint8_t, MAX_TEXTURE_STAGES> m_textureMipLevels;
    std::array<int, MAX_TEXTURE_STAGES> m_textureUvIndices;
    int m_activeTextureStages = 0;

    // Material/light uniforms
    bgfx::UniformHandle m_diffuseUniform;
    bgfx::UniformHandle m_specularUniform;
    bgfx::UniformHandle m_emissiveUniform;
    bgfx::UniformHandle m_ambientUniform;
    bgfx::UniformHandle m_opacityUniform;
    bgfx::UniformHandle m_lightingEnableUniform;
    bgfx::UniformHandle m_alphaTestUniform;

    // Light environment uniforms
    bgfx::UniformHandle m_ambientLightUniform;
    bgfx::UniformHandle m_lightDirUniform;
    bgfx::UniformHandle m_lightDiffuseUniform;
    bgfx::UniformHandle m_lightPosUniform;

    // Transform uniforms
    bgfx::UniformHandle m_modelUniform;
    bgfx::UniformHandle m_viewProjUniform;
    bgfx::UniformHandle m_camPosUniform;

    // Fog uniforms
    bgfx::UniformHandle m_fogColorUniform;
    bgfx::UniformHandle m_fogParamsUniform;

    // Ubershader uniforms
    bgfx::UniformHandle m_shaderModeUniform;
    bgfx::UniformHandle m_shaderFlagsUniform;
    bgfx::UniformHandle m_detailFuncUniform;

    // Cached render state from DX8Wrapper dispatch
    Matrix4 m_worldMatrix;
    Matrix4 m_viewMatrix;
    Matrix4 m_projectionMatrix;
    bool m_worldDirty;
    bool m_viewDirty;
    bool m_projectionDirty;

    // Cached W3D objects for bridging
    const VertexBufferClass* m_currentVB;
    const IndexBufferClass* m_currentIB;
    unsigned short m_currentIBOffset;
    const VertexMaterialClass* m_currentMaterial;
    LightEnvironmentClass* m_currentLightEnv;

    // Default shader program for mesh rendering
    bgfx::ProgramHandle m_defaultProgram;

    // Resource caches (W3D object -> bgfx handle).
    // NOTE: These caches do not evict entries when W3D objects are destroyed.
    // If a destroyed object is reallocated at the same address, a stale handle
    // may be reused. A proper fix requires invalidation on object destruction.
    std::unordered_map<const void*, bgfx::VertexBufferHandle> m_vbCache;
    std::unordered_map<const void*, bgfx::IndexBufferHandle> m_ibCache;
    std::unordered_map<const void*, bgfx::TextureHandle> m_textureCache;

    // Shader variant cache
    ShaderVariantCache m_shaderCache;
};

#endif // BGFXBACKEND_H
