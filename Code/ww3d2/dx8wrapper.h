#ifndef DX8_WRAPPER_H
#define DX8_WRAPPER_H

// Cross-platform stub for non-Direct3D9 builds.
// The full Direct3D9 implementation is in dx8wrapper_win32.h (Win32 with DX9 only).
// Stub provides no-op implementations so game logic compiles without D3D9.

#if defined(WW3D_DX9_BACKEND) && defined(_WIN32)
// ============================================================
// WINDOWS with DX9: include the real Direct3D9 header
// ============================================================
#include "dx8wrapper_win32.h"

#else
// ============================================================
// All other cases (Linux, Windows without DX9, etc.): use stubs
// ============================================================

#include "always.h"
#include "matrix4.h"
#include "vector4.h"
#include "lightenvironment.h"
#include "wwstring.h"
#include "shader.h"

const unsigned MAX_TEXTURE_STAGES = 2;

// Buffer types for dynamic vertex/index buffers
enum {
    BUFFER_TYPE_DX8,
    BUFFER_TYPE_SORTING,
    BUFFER_TYPE_DYNAMIC_DX8,
    BUFFER_TYPE_DYNAMIC_SORTING,
    BUFFER_TYPE_INVALID
};

// Stub D3D constants for non-DX9 builds (used by ww3d.cpp, etc.)
// On MinGW, d3d9.h / d3d9types.h defines these as preprocessor macros.
// Use #define instead of enum so #ifndef guards work properly.
#ifndef D3DFILL_POINT
#define D3DFILL_POINT 1
#define D3DFILL_WIREFRAME 2
#define D3DFILL_SOLID 3
#endif

#ifndef D3DTS_WORLD
#define D3DTS_WORLD 0
#define D3DTS_VIEW 1
#define D3DTS_PROJECTION 2
#endif

#ifndef D3DRS_FILLMODE
#define D3DRS_FILLMODE 2
#define D3DRS_AMBIENT 87
#define D3DRS_SLOPESCALEDEPTHBIAS 233
#define D3DRS_DEPTHBIAS 234
#endif

#ifndef _d3d9TYPES_H_
struct D3DVIEWPORT9 {
    int X, Y;
    int Width, Height;
    float MinZ, MaxZ;
};
#endif

// =====================================================================
// STUB: DX8Wrapper
// =====================================================================
class DX8Wrapper {
public:
    // Convert color (Vector3+alpha → ARGB) — used by backgroundmgr, lightsolve
    static unsigned Convert_Color(const Vector3 &color, float alpha) {
        unsigned r = static_cast<unsigned>(color.X * 255.0f);
        unsigned g = static_cast<unsigned>(color.Y * 255.0f);
        unsigned b = static_cast<unsigned>(color.Z * 255.0f);
        unsigned a = static_cast<unsigned>(alpha * 255.0f);
        return (a << 24) | (r << 16) | (g << 8) | b;
    }
    // Convert color (Vector4 → ARGB)
    static unsigned Convert_Color(const Vector4 &color) {
        unsigned r = static_cast<unsigned>(color.X * 255.0f);
        unsigned g = static_cast<unsigned>(color.Y * 255.0f);
        unsigned b = static_cast<unsigned>(color.Z * 255.0f);
        unsigned a = static_cast<unsigned>(color.W * 255.0f);
        return (a << 24) | (r << 16) | (g << 8) | b;
    }
    // Convert color with clamping (Vector4 → ARGB)
    static unsigned Convert_Color_Clamp(const Vector4 &color) {
        float r = color.X < 0.0f ? 0.0f : (color.X > 1.0f ? 1.0f : color.X);
        float g = color.Y < 0.0f ? 0.0f : (color.Y > 1.0f ? 1.0f : color.Y);
        float b = color.Z < 0.0f ? 0.0f : (color.Z > 1.0f ? 1.0f : color.Z);
        float a = color.W < 0.0f ? 0.0f : (color.W > 1.0f ? 1.0f : color.W);
        unsigned R = static_cast<unsigned>(r * 255.0f);
        unsigned G = static_cast<unsigned>(g * 255.0f);
        unsigned B = static_cast<unsigned>(b * 255.0f);
        unsigned A = static_cast<unsigned>(a * 255.0f);
        return (A << 24) | (R << 16) | (G << 8) | B;
    }

    // Stub: no rendering calls on server
    static void Clear(bool, bool, const Vector3 &) {}
    static void Set_Material(const void *) {}
    static void Set_Shader(const ShaderClass &) {}
    static void Set_Texture(unsigned, const void *) {}
    static void Set_Alpha(float, unsigned) {}
    static void Set_Transform(int, const Matrix3D &) {}
    static void Set_Viewport(void *) {}
    static void Set_Projection_Transform_With_Z_Bias(const Matrix4 &, float, float) {}
    static void Set_Light_Environment(const void *) {}
    static void Set_Vertex_Buffer(const void *, unsigned = 0) {}
    static void Set_Index_Buffer(const void *, unsigned = 0) {}
    static void Set_Index_Buffer_Index_Offset(unsigned) {}
    static void Draw_Strip(int, int, int, int) {}
    static void Draw_Triangles(int, int, int, int) {}
    static void Get_Transform(int, Matrix3D &) {}
    static void Set_DX8_Render_State(int, unsigned) {}
    static void Set_Pseudo_ZBias(float) {}
    static void Set_DX8_ZBias(float) {}
    static void *Get_DX8_Back_Buffer(unsigned = 0) { return nullptr; }
    static void Get_DX8_Texture_Stage_State_Value_Name(StringClass &, int, unsigned) {}
    static void Get_DX8_Render_State_Value_Name(StringClass &, int, unsigned) {}
    static void Get_DX8_Texture_Sampler_State_Value_Name(StringClass &, int, unsigned) {}
};

// Stub DX8TextureManagerClass for non-Windows builds
class DX8TextureManagerClass {
public:
    static void Shutdown() {}
};

// d3d9types.h is included via CMake when DX9 backend is enabled

#endif // WW3D_DX9_BACKEND && _WIN32

#endif // DX8_WRAPPER_H
