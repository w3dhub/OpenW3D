// BGFXMaterialMapper.h - OpenW3D render backend

#ifndef BGFXMATERIALMAPPER_H
#define BGFXMATERIALMAPPER_H

#include "bgfx/bgfx.h"
#include "vector3.h"
#include "shader.h"
#include "vertmaterial.h"
#include "meshmatdesc.h"

// bgfx uniform names for material properties
namespace BGFXMaterialUniforms
{
    const char* const DIFFUSE_COLOR = "u_diffuseColor";
    const char* const SPECULAR_COLOR = "u_specularColor";
    const char* const EMISSIVE_COLOR = "u_emissiveColor";
    const char* const AMBIENT_COLOR = "u_ambientColor";
    const char* const SHININESS = "u_shininess";
    const char* const OPACITY = "u_opacity";
    const char* const LIGHTING_ENABLE = "u_lightingEnable";
}

// Material color structure matching shader expectations
struct MaterialColors
{
    Vector3 Diffuse;
    float Alpha;
    Vector3 Specular;
    float Shininess;
    Vector3 Emissive;
    float Pad1;  // Alignment padding
    Vector3 Ambient;
    float Pad2;

    MaterialColors()
        : Diffuse(1.0f, 1.0f, 1.0f)
        , Alpha(1.0f)
        , Specular(0.0f, 0.0f, 0.0f)
        , Shininess(0.0f)
        , Emissive(0.0f, 0.0f, 0.0f)
        , Pad1(0.0f)
        , Ambient(0.0f, 0.0f, 0.0f)
        , Pad2(0.0f)
    {}
};

// BGFXMaterialMapper - helper class for mapping materials to bgfx
class BGFXMaterialMapper
{
public:
    // Map ShaderClass blend modes to bgfx state
    static uint64_t Shader_Blend_To_BGFX(ShaderClass::SrcBlendFuncType src, ShaderClass::DstBlendFuncType dst)
    {
        uint64_t state = BGFX_STATE_DEFAULT;
        
        // Source blend
        uint64_t srcBlend = Src_Blend_To_BGFX(src);
        
        // Destination blend
        uint64_t dstBlend = Dst_Blend_To_BGFX(dst);
        
        state |= BGFX_STATE_BLEND_FUNC(srcBlend, dstBlend);
        return state;
    }

    static uint64_t Src_Blend_To_BGFX(ShaderClass::SrcBlendFuncType src)
    {
        switch (src) {
            case ShaderClass::SRCBLEND_ZERO:           return BGFX_STATE_BLEND_ZERO;
            case ShaderClass::SRCBLEND_ONE:            return BGFX_STATE_BLEND_ONE;
            case ShaderClass::SRCBLEND_SRC_ALPHA:      return BGFX_STATE_BLEND_SRC_ALPHA;
            case ShaderClass::SRCBLEND_ONE_MINUS_SRC_ALPHA: return BGFX_STATE_BLEND_INV_SRC_ALPHA;
            default:                                  return BGFX_STATE_BLEND_ONE;
        }
    }

    static uint64_t Dst_Blend_To_BGFX(ShaderClass::DstBlendFuncType dst)
    {
        switch (dst) {
            case ShaderClass::DSTBLEND_ZERO:                   return BGFX_STATE_BLEND_ZERO;
            case ShaderClass::DSTBLEND_ONE:                    return BGFX_STATE_BLEND_ONE;
            case ShaderClass::DSTBLEND_SRC_ALPHA:              return BGFX_STATE_BLEND_SRC_ALPHA;
            case ShaderClass::DSTBLEND_ONE_MINUS_SRC_ALPHA:    return BGFX_STATE_BLEND_INV_SRC_ALPHA;
            case ShaderClass::DSTBLEND_SRC_COLOR:             return BGFX_STATE_BLEND_SRC_COLOR;
            case ShaderClass::DSTBLEND_ONE_MINUS_SRC_COLOR:   return BGFX_STATE_BLEND_INV_SRC_COLOR;
            default:                                          return BGFX_STATE_BLEND_ZERO;
        }
    }

    // Map ShaderClass cull mode to bgfx state
    static uint64_t Shader_Cull_To_BGFX(ShaderClass::CullModeType cull)
    {
        switch (cull) {
            case ShaderClass::CULL_MODE_DISABLE:  return BGFX_STATE_NONE;
            case ShaderClass::CULL_MODE_ENABLE:   return BGFX_STATE_CULL_CCW;
            default:                              return BGFX_STATE_CULL_CCW;
        }
    }

    // Map ShaderClass depth compare to bgfx state
    static uint64_t Shader_DepthCompare_To_BGFX(ShaderClass::DepthCompareType cmp)
    {
        switch (cmp) {
            case ShaderClass::PASS_NEVER:    return BGFX_STATE_DEPTH_TEST_NEVER;
            case ShaderClass::PASS_LESS:     return BGFX_STATE_DEPTH_TEST_LESS;
            case ShaderClass::PASS_EQUAL:    return BGFX_STATE_DEPTH_TEST_EQUAL;
            case ShaderClass::PASS_LEQUAL:   return BGFX_STATE_DEPTH_TEST_LEQUAL;
            case ShaderClass::PASS_GREATER:  return BGFX_STATE_DEPTH_TEST_GREATER;
            case ShaderClass::PASS_NOTEQUAL: return BGFX_STATE_DEPTH_TEST_NOTEQUAL;
            case ShaderClass::PASS_GEQUAL:   return BGFX_STATE_DEPTH_TEST_GEQUAL;
            case ShaderClass::PASS_ALWAYS:   return BGFX_STATE_DEPTH_TEST_ALWAYS;
            default:                         return BGFX_STATE_DEPTH_TEST_LEQUAL;
        }
    }

    // Map ShaderClass to full bgfx state flags
    static uint64_t Shader_To_BGFX_State(const ShaderClass& shader)
    {
        uint64_t state = BGFX_STATE_DEFAULT;

        // Depth test setting
        state |= Shader_DepthCompare_To_BGFX(shader.Get_Depth_Compare());

        // Depth write
        if (shader.Get_Depth_Mask() == ShaderClass::DEPTH_WRITE_DISABLE) {
            // Don't add WRITE_Z
        } else {
            state |= BGFX_STATE_WRITE_Z;
        }

        // Cull mode
        state |= Shader_Cull_To_BGFX(shader.Get_Cull_Mode());

        // Blending
        state |= Shader_Blend_To_BGFX(shader.Get_Src_Blend_Func(), shader.Get_Dst_Blend_Func());

        return state;
    }

    // Extract material colors from VertexMaterialClass
    static MaterialColors Get_Material_Colors(const VertexMaterialClass* mat)
    {
        MaterialColors colors;

        if (mat) {
            mat->Get_Diffuse(&colors.Diffuse);
            colors.Alpha = mat->Get_Opacity();
            mat->Get_Specular(&colors.Specular);
            colors.Shininess = mat->Get_Shininess();
            mat->Get_Emissive(&colors.Emissive);
            mat->Get_Ambient(&colors.Ambient);
        }

        return colors;
    }

    // Create bgfx render state from ShaderClass
    static uint64_t Make_Render_State(const ShaderClass& shader, bool lightingEnabled)
    {
        uint64_t state = Shader_To_BGFX_State(shader);

        // Add point/line/wireframe based on fill mode if needed
        // For now, default to triangle strips

        return state;
    }

    // Determine if shader uses alpha blending
    static bool Uses_Alpha_Blending(const ShaderClass& shader)
    {
        ShaderClass::DstBlendFuncType dst = shader.Get_Dst_Blend_Func();
        ShaderClass::SrcBlendFuncType src = shader.Get_Src_Blend_Func();

        return (dst == ShaderClass::DSTBLEND_SRC_ALPHA || 
                dst == ShaderClass::DSTBLEND_ONE_MINUS_SRC_ALPHA ||
                src == ShaderClass::SRCBLEND_SRC_ALPHA ||
                src == ShaderClass::SRCBLEND_ONE_MINUS_SRC_ALPHA);
    }

    // Determine static sort category based on shader
    static ShaderClass::StaticSortCategoryType Get_Sort_Category(const ShaderClass& shader)
    {
        if (shader.Get_Alpha_Test() != ShaderClass::ALPHATEST_DISABLE) {
            return ShaderClass::SSCAT_ALPHA_TEST;
        }

        ShaderClass::DstBlendFuncType dst = shader.Get_Dst_Blend_Func();
        ShaderClass::SrcBlendFuncType src = shader.Get_Src_Blend_Func();
        if (dst == ShaderClass::DSTBLEND_ONE && src == ShaderClass::SRCBLEND_ONE) {
            return ShaderClass::SSCAT_ADDITIVE;
        }
        if (dst == ShaderClass::DSTBLEND_SRC_ALPHA || dst == ShaderClass::DSTBLEND_ONE) {
            return ShaderClass::SSCAT_ADDITIVE;
        }

        return ShaderClass::SSCAT_OTHER;
    }
};

#endif // BGFXMATERIALMAPPER_H
