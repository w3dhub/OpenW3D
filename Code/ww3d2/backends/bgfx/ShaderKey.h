#pragma once
#include "shader.h"
#include <cstddef>
#include <functional>

struct ShaderKey {
    uint8_t depthCompare : 3;     // PASS_NEVER..PASS_ALWAYS
    uint8_t depthWrite   : 1;     // 0=disable, 1=enable
    uint8_t blendMode    : 4;     // 0=opaque, 1=additive, 2=alpha, 3=mult, 4=screen
    uint8_t alphaTest    : 1;     // 0=off, 1=on
    uint8_t fogMode      : 2;     // 0=off, 1=linear, 2=exp, 3=exp2
    uint8_t texturing    : 1;     // 0=solid color, 1=texture
    uint8_t priGradient  : 3;     // 0=off, 1=modulate, 2=add, 3=bump, 4=bumplum, 5=dot3
    uint8_t secGradient  : 1;     // 0=off, 1=on
    uint8_t lighting     : 1;     // 0=unlit, 1=lit
    uint8_t texStageCount: 3;     // 0-8 active texture stages
    uint8_t detailColor  : 4;     // DETAILCOLOR_* function
    uint8_t detailAlpha  : 3;     // DETAILALPHA_* function

    bool operator==(const ShaderKey& o) const {
        return std::memcmp(this, &o, sizeof(ShaderKey)) == 0;
    }
    bool operator!=(const ShaderKey& o) const { return !(*this == o); }
};

namespace std {
    template<> struct hash<ShaderKey> {
        size_t operator()(const ShaderKey& k) const {
            // Pack into a 64-bit value for hashing
            const uint8_t* p = reinterpret_cast<const uint8_t*>(&k);
            size_t h = 0;
            for (size_t i = 0; i < sizeof(ShaderKey); ++i)
                h = h * 31 + p[i];
            return h;
        }
    };
}

inline ShaderKey ShaderKey_From_DX8(const ShaderClass& shader) {
    ShaderKey key = {};

    // Depth
    key.depthCompare = static_cast<uint8_t>(shader.Get_Depth_Compare());
    key.depthWrite   = (shader.Get_Depth_Mask() == ShaderClass::DEPTH_WRITE_ENABLE) ? 1 : 0;

    // Blend mode mapping (src+dst blend combo -> enum)
    auto srcBlend = shader.Get_Src_Blend_Func();
    auto dstBlend = shader.Get_Dst_Blend_Func();
    if (srcBlend == ShaderClass::SRCBLEND_ONE && dstBlend == ShaderClass::DSTBLEND_ZERO)
        key.blendMode = 0; // opaque
    else if (srcBlend == ShaderClass::SRCBLEND_ONE && dstBlend == ShaderClass::DSTBLEND_ONE)
        key.blendMode = 1; // additive
    else if (srcBlend == ShaderClass::SRCBLEND_SRC_ALPHA && dstBlend == ShaderClass::DSTBLEND_ONE_MINUS_SRC_ALPHA)
        key.blendMode = 2; // alpha
    else if (srcBlend == ShaderClass::SRCBLEND_ZERO && dstBlend == ShaderClass::DSTBLEND_SRC_COLOR)
        key.blendMode = 3; // multiplicative
    else if (srcBlend == ShaderClass::SRCBLEND_SRC_ALPHA && dstBlend == ShaderClass::DSTBLEND_ONE)
        key.blendMode = 4; // screen-like (additive with alpha)
    else
        key.blendMode = 5; // other/custom

    // Alpha test
    key.alphaTest = (shader.Get_Alpha_Test() != ShaderClass::ALPHATEST_DISABLE) ? 1 : 0;

    // Fog
    switch (shader.Get_Fog_Func()) {
        case ShaderClass::FOG_DISABLE:    key.fogMode = 0; break;
        case ShaderClass::FOG_ENABLE:     key.fogMode = 1; break;
        case ShaderClass::FOG_SCALE_FRAGMENT: key.fogMode = 2; break;
        case ShaderClass::FOG_WHITE:      key.fogMode = 3; break;
        default: key.fogMode = 0; break;
    }

    // Texturing
    key.texturing = (shader.Get_Texturing() != ShaderClass::TEXTURING_DISABLE) ? 1 : 0;

    // Primary gradient
    key.priGradient = static_cast<uint8_t>(shader.Get_Primary_Gradient());

    // Secondary gradient
    key.secGradient = (shader.Get_Secondary_Gradient() != ShaderClass::SECONDARY_GRADIENT_DISABLE) ? 1 : 0;

    // Post-detail functions (detail maps)
    key.detailColor = static_cast<uint8_t>(shader.Get_Post_Detail_Color_Func());
    key.detailAlpha = static_cast<uint8_t>(shader.Get_Post_Detail_Alpha_Func());

    // Lighting: infer from gradient — if gradient is modulate, lighting is active
    key.lighting = (key.priGradient == ShaderClass::GRADIENT_MODULATE ||
                    key.priGradient == ShaderClass::GRADIENT_ADD) ? 1 : 0;

    // Texture stages — start with 1, will be set by texture binding code
    key.texStageCount = key.texturing ? 1 : 0;

    return key;
}
