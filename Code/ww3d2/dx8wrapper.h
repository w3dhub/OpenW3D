#ifndef DX8_WRAPPER_H
#define DX8_WRAPPER_H

// Cross-platform stub for non-Win32 (Linux dedicated server).
// The full Direct3D9 implementation is in dx8wrapper_win32.h (Win32 only).
// Stub provides no-op implementations so game logic compiles without D3D9.

#ifndef _WIN32

#include "always.h"
#include "matrix4.h"
#include "vector4.h"
#include "lightenvironment.h"
#include "wwstring.h"

const unsigned MAX_TEXTURE_STAGES = 2;

const unsigned D3DTS_VIEW       = 2;
const unsigned D3DTS_PROJECTION  = 3;
const unsigned D3DTS_WORLD      = 0;
const unsigned D3DFILL_WIREFRAME = 2;
const unsigned D3DFILL_SOLID    = 3;
const unsigned D3DFILL_POINT   = 1;
const unsigned D3DRS_FILLMODE  = 8;

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

    // Stub: no rendering calls on server
    static void Set_Material(const void *) {}
    static void Set_Shader(const void *) {}
    static void Set_Texture(unsigned, const void *) {}
    static void Set_Alpha(float, unsigned) {}
    static void Set_Transform(int, const Matrix3D &) {}
    static void Set_Light_Environment(const void *) {}
    static void Set_Vertex_Buffer(const void *, unsigned = 0) {}
    static void Set_Index_Buffer(const void *, unsigned = 0) {}
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

#else
// ============================================================
// WINDOWS: include the real Direct3D9 header
// ============================================================
#include "dx8wrapper_win32.h"
#endif // _WIN32

#endif // DX8_WRAPPER_H
