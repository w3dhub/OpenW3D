// dx8stubs.cpp — Stub implementations for DX8Wrapper and related functions
// when building without DX9/D3D backend (Linux/BGFX/NullBackend builds).
//
// These stubs allow the shared ww3d2 code to link without a D3D9 implementation.
// The actual rendering is handled by WW3DBackend (BGFX or NullBackend).

#include "ww3d_platform.h"

#ifndef _WIN32

#include "dx8wrapper.h"
#include "dx8caps.h"
#include "dx8fvf.h"
#include "texture.h"
#include "dx8vertexbuffer.h"
#include "dx8indexbuffer.h"
#include "dx8texman.h"
#include "rddesc.h"
#include "formconv.h"
#include "shader.h"
#include "lightenvironment.h"

// ---- DX8Wrapper stubs ----

// Static data
static bool IsInitted = false;
static bool IsDeviceLost = false;
static unsigned _MainThreadID = 0;
static D3DCAPS9 CurrentCaps;
static D3DADAPTER_IDENTIFIER9 CurrentAdapterIdentifier;
static D3DLIGHT9 CurrentLights[4];
static bool LightEnabled[4] = {false, false, false, false};
static ShaderClass CurrentShader;
static RenderStateStruct CurrentRenderState;
static unsigned _DX8Calls = 0;

void DX8Wrapper::Init(void * hwnd, bool lite) {}
void DX8Wrapper::Shutdown() {}
void DX8Wrapper::Do_Onetime_Device_Dependent_Inits() {}
void DX8Wrapper::Do_Onetime_Device_Dependent_Shutdowns() {}
bool DX8Wrapper::Is_Initted() { return IsInitted; }
bool DX8Wrapper::Is_Device_Lost() { return IsDeviceLost; }

void DX8Wrapper::Begin_Scene() {}
void DX8Wrapper::End_Scene(bool flip_frame) {}
void DX8Wrapper::Flip_To_Primary() {}

void DX8Wrapper::Clear(bool clear_color, bool clear_z_stencil, const Vector3 &color, float z, unsigned stencil) {}

void DX8Wrapper::Set_Viewport(CONST D3DVIEWPORT9* pViewport) {}
void DX8Wrapper::Set_Vertex_Buffer(const VertexBufferClass* vb) {}
void DX8Wrapper::Set_Vertex_Buffer(const DynamicVBAccessClass& vba) {}
void DX8Wrapper::Set_Index_Buffer(const IndexBufferClass* ib, unsigned short index_base_offset) {}
void DX8Wrapper::Set_Index_Buffer(const DynamicIBAccessClass& iba, unsigned short index_base_offset) {}
void DX8Wrapper::Set_Index_Buffer_Index_Offset(unsigned offset) {}

void DX8Wrapper::Get_Render_State(RenderStateStruct& state) { state = CurrentRenderState; }
void DX8Wrapper::Set_Render_State(const RenderStateStruct& state) { CurrentRenderState = state; }
void DX8Wrapper::Release_Render_State() {}

void DX8Wrapper::Set_DX8_Material(const D3DMATERIAL9* mat) {}
void DX8Wrapper::Set_Gamma(float gamma, float bright, float contrast, bool calibrate, bool uselimit) {}
void DX8Wrapper::Set_DX8_ZBias(int zbias) {}
void DX8Wrapper::Set_Pseudo_ZBias(int zbias) {}
void DX8Wrapper::Set_Projection_Transform_With_Z_Bias(const Matrix4& matrix, float znear, float zfar) {}
void DX8Wrapper::Set_Transform(D3DTRANSFORMSTATETYPE transform, const Matrix4& m) {}
void DX8Wrapper::Set_Transform(D3DTRANSFORMSTATETYPE transform, const Matrix3D& m) {}
void DX8Wrapper::Get_Transform(D3DTRANSFORMSTATETYPE transform, Matrix4& m) { m.Make_Identity(); }
void DX8Wrapper::Set_World_Identity() {}
void DX8Wrapper::Set_View_Identity() {}
bool DX8Wrapper::Is_World_Identity() { return true; }
bool DX8Wrapper::Is_View_Identity() { return true; }

void DX8Wrapper::_Set_DX8_Transform(D3DTRANSFORMSTATETYPE transform, const Matrix4& m) {}
void DX8Wrapper::_Set_DX8_Transform(D3DTRANSFORMSTATETYPE transform, const Matrix3D& m) {}
void DX8Wrapper::_Get_DX8_Transform(D3DTRANSFORMSTATETYPE transform, Matrix4& m) { m.Make_Identity(); }

void DX8Wrapper::Set_DX8_Light(int index, D3DLIGHT9* light) {}
void DX8Wrapper::Set_DX8_Render_State(D3DRENDERSTATETYPE state, unsigned value) {}
void DX8Wrapper::Set_DX8_Texture_Stage_State(unsigned stage, D3DTEXTURESTAGESTATETYPE state, unsigned value) {}
void DX8Wrapper::Set_DX8_N_Patch_Mode(float segments) {}
void DX8Wrapper::Set_DX8_Texture_Sampler_State(unsigned sampler, D3DSAMPLERSTATETYPE state, unsigned value) {}
void DX8Wrapper::Set_DX8_Texture(unsigned int stage, IDirect3DBaseTexture9* texture) {}
void DX8Wrapper::Set_Light_Environment(LightEnvironmentClass* light_env) {}
void DX8Wrapper::Set_Fog(bool enable, const Vector3 &color, float start, float end) {}

const D3DLIGHT9& DX8Wrapper::Peek_Light(unsigned index) { return CurrentLights[0]; }
bool DX8Wrapper::Is_Light_Enabled(unsigned index) { return LightEnabled[index % 4]; }

void DX8Wrapper::Set_Shader(const ShaderClass& shader) { CurrentShader = shader; }
void DX8Wrapper::Get_Shader(ShaderClass& shader) { shader = CurrentShader; }
void DX8Wrapper::Set_Texture(unsigned stage, TextureClass* texture) {}
void DX8Wrapper::Set_Material(const VertexMaterialClass* material) {}
void DX8Wrapper::Set_Light(unsigned index, const D3DLIGHT9* light) {}
void DX8Wrapper::Set_Light(unsigned index, const LightClass &light) {}

void DX8Wrapper::Apply_Render_State_Changes() {}

void DX8Wrapper::Draw_Triangles(unsigned flags, unsigned min_vertex_index, unsigned num_vertices, unsigned start_index, unsigned num_triangles) {}
void DX8Wrapper::Draw_Triangles(unsigned flags, unsigned min_vertex_index, unsigned num_vertices, unsigned start_index, unsigned num_triangles, unsigned pass_count) {}
void DX8Wrapper::Draw_Strip(unsigned flags, unsigned min_vertex_index, unsigned num_vertices, unsigned start_index, unsigned num_triangles) {}
void DX8Wrapper::Draw_Sorting_IB_VB(unsigned flags, unsigned min_vertex_index, unsigned num_vertices, unsigned start_index, unsigned num_triangles) {}

void DX8Wrapper::Flush_DX8_Resource_Manager() {}
unsigned int DX8Wrapper::Get_Free_Texture_RAM() { return 64 * 1024 * 1024; } // Report 64MB

unsigned DX8Wrapper::Get_Current_Adapter_Identifier() { return 0; }

IDirect3DTexture9* DX8Wrapper::_Create_DX8_Texture(unsigned width, unsigned height, unsigned levels, D3DPOOL pool, D3DFORMAT format) { return nullptr; }
IDirect3DTexture9* DX8Wrapper::_Create_DX8_Texture(const char *filename, TextureClass::MipCountType mip_level_count) { return nullptr; }
IDirect3DTexture9* DX8Wrapper::_Create_DX8_Texture(IDirect3DSurface9 *surface, TextureClass::MipCountType mip_level_count) { return nullptr; }
IDirect3DSurface9* DX8Wrapper::_Create_DX8_Surface(unsigned int width, unsigned int height, D3DPOOL pool, WW3DFormat format) { return nullptr; }
IDirect3DSurface9* DX8Wrapper::_Create_DX8_Surface(const char *filename) { return nullptr; }
IDirect3DSurface9* DX8Wrapper::_Get_DX8_Front_Buffer() { return nullptr; }
SurfaceClass* DX8Wrapper::_Get_DX8_Back_Buffer(unsigned int num) { return nullptr; }

void DX8Wrapper::_Copy_DX8_Rects(IDirect3DSurface9* src, IDirect3DSurface9* dst) {}
void DX8Wrapper::_Read_Texture(IDirect3DSurface9* surface, void* data, unsigned pitch) {}
void DX8Wrapper::_Update_Texture(TextureClass *system, TextureClass *video) {}

void DX8Wrapper::Begin_Statistics() {}
void DX8Wrapper::End_Statistics() {}
unsigned DX8Wrapper::Get_Last_Frame_Matrix_Changes() { return 0; }
unsigned DX8Wrapper::Get_Last_Frame_Material_Changes() { return 0; }
unsigned DX8Wrapper::Get_Last_Frame_Vertex_Buffer_Changes() { return 0; }
unsigned DX8Wrapper::Get_Last_Frame_Index_Buffer_Changes() { return 0; }
unsigned DX8Wrapper::Get_Last_Frame_Light_Changes() { return 0; }
unsigned DX8Wrapper::Get_Last_Frame_Texture_Changes() { return 0; }
unsigned DX8Wrapper::Get_Last_Frame_Render_State_Changes() { return 0; }

void DX8Wrapper::Set_Clip_Plane(unsigned index, const float* plane) {}
void DX8Wrapper::Set_Swap_Interval(int swap) {}
int DX8Wrapper::Get_Swap_Interval() { return 0; }
void DX8Wrapper::Set_Texture_Bitdepth(int depth) {}
int DX8Wrapper::Get_Texture_Bitdepth() { return 32; }

// DX8Caps stubs
class DX8CapsInternal {
public:
    D3DCAPS9 caps;
    DX8CapsInternal() { memset(&caps, 0, sizeof(caps)); }
    bool fog_allowed;
    DX8CapsInternal() : fog_allowed(true) {}
};

DX8Caps::DX8Caps() {}
DX8Caps::~DX8Caps() {}
bool DX8Caps::Is_Fog_Allowed() { return true; }
const D3DCAPS9& DX8Caps::Get_DX8_Caps() { static D3DCAPS9 c; memset(&c, 0, sizeof(c)); return c; }
bool DX8Caps::Support_DXTC() { return false; }
bool DX8Caps::Support_Texture_Swizzle() { return false; }
unsigned int DX8Caps::Get_Texture_Op_Caps() { return 0; }

// DX8_ErrorCode stubs
void Log_DX8_ErrorCode(HRESULT res) {}
void DX8_ErrorCode(HRESULT res) {}

// Formconv stubs — these convert between D3DFORMAT and WW3DFormat
#ifdef _WIN32
// Real implementations only on Windows
#else
static WW3DFormat D3DFormatToWW3FormatArray[D3DFMT_X8L8V8U8 + 1];

D3DFORMAT WW3DFormat_To_D3DFormat(WW3DFormat ww3d_format) {
    switch (ww3d_format) {
        case WW3D_FORMAT_R8G8B8: return D3DFMT_R8G8B8;
        case WW3D_FORMAT_A8R8G8B8: return D3DFMT_A8R8G8B8;
        case WW3D_FORMAT_X8R8G8B8: return D3DFMT_X8R8G8B8;
        case WW3D_FORMAT_R5G6B5: return D3DFMT_R5G6B5;
        case WW3D_FORMAT_DXT1: return D3DFMT_DXT1;
        case WW3D_FORMAT_DXT2: return D3DFMT_DXT2;
        case WW3D_FORMAT_DXT3: return D3DFMT_DXT3;
        case WW3D_FORMAT_DXT4: return D3DFMT_DXT4;
        case WW3D_FORMAT_DXT5: return D3DFMT_DXT5;
        case WW3D_FORMAT_L8: return D3DFMT_L8;
        case WW3D_FORMAT_A8L8: return D3DFMT_A8L8;
        case WW3D_FORMAT_A4L4: return D3DFMT_A4L4;
        case WW3D_FORMAT_A8: return D3DFMT_A8;
        default: return D3DFMT_UNKNOWN;
    }
}

WW3DFormat D3DFormat_To_WW3DFormat(D3DFORMAT d3d_format) {
    switch (d3d_format) {
        case D3DFMT_R8G8B8: return WW3D_FORMAT_R8G8B8;
        case D3DFMT_A8R8G8B8: return WW3D_FORMAT_A8R8G8B8;
        case D3DFMT_X8R8G8B8: return WW3D_FORMAT_X8R8G8B8;
        case D3DFMT_R5G6B5: return WW3D_FORMAT_R5G6B5;
        case D3DFMT_DXT1: return WW3D_FORMAT_DXT1;
        case D3DFMT_DXT2: return WW3D_FORMAT_DXT2;
        case D3DFMT_DXT3: return WW3D_FORMAT_DXT3;
        case D3DFMT_DXT4: return WW3D_FORMAT_DXT4;
        case D3DFMT_DXT5: return WW3D_FORMAT_DXT5;
        case D3DFMT_L8: return WW3D_FORMAT_L8;
        case D3DFMT_A8L8: return WW3D_FORMAT_A8L8;
        case D3DFMT_A4L4: return WW3D_FORMAT_A4L4;
        case D3DFMT_A8: return WW3D_FORMAT_A8;
        default: return WW3D_FORMAT_UNKNOWN;
    }
}

void Init_D3D_To_WW3_Conversion() {
    memset(D3DFormatToWW3FormatArray, 0, sizeof(D3DFormatToWW3FormatArray));
    D3DFormatToWW3FormatArray[D3DFMT_R8G8B8] = WW3D_FORMAT_R8G8B8;
    D3DFormatToWW3FormatArray[D3DFMT_A8R8G8B8] = WW3D_FORMAT_A8R8G8B8;
    D3DFormatToWW3FormatArray[D3DFMT_X8R8G8B8] = WW3D_FORMAT_X8R8G8B8;
    D3DFormatToWW3FormatArray[D3DFMT_R5G6B5] = WW3D_FORMAT_R5G6B5;
    D3DFormatToWW3FormatArray[D3DFMT_DXT1] = WW3D_FORMAT_DXT1;
    D3DFormatToWW3FormatArray[D3DFMT_DXT3] = WW3D_FORMAT_DXT3;
    D3DFormatToWW3FormatArray[D3DFMT_DXT5] = WW3D_FORMAT_DXT5;
    D3DFormatToWW3FormatArray[D3DFMT_L8] = WW3D_FORMAT_L8;
    D3DFormatToWW3FormatArray[D3DFMT_A8L8] = WW3D_FORMAT_A8L8;
    D3DFormatToWW3FormatArray[D3DFMT_A4L4] = WW3D_FORMAT_A4L4;
    D3DFormatToWW3FormatArray[D3DFMT_A8] = WW3D_FORMAT_A8;
}
#endif

// SortingRenderer stub
// Provided by nullbackend.cpp stubs or similar

#endif // !_WIN32