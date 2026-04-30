$input v_texcoord0, v_texcoord1, v_texcoord2, v_texcoord3
$input v_color0, v_color1, v_normal, v_worldPos
$output gl_FragColor

#include <bgfx_shader.sh>

// ============================================================================
// Shader Mode Enums
// ============================================================================

// Blend modes (u_shaderMode.x)
#define BLEND_REPLACE   0
#define BLEND_ADD       1
#define BLEND_MULTIPLY  2
#define BLEND_ALPHA     3

// Fog modes (u_shaderMode.y)
#define FOG_NONE        0
#define FOG_LINEAR      1
#define FOG_EXP         2
#define FOG_EXP2        3

// Primary gradient modes (u_shaderMode.z)
#define PRIGRAD_NONE    0
#define PRIGRAD_MOD     1
#define PRIGRAD_ADD     2

// Secondary gradient modes (u_shaderMode.w)
#define SECGRAD_NONE    0
#define SECGRAD_ADD     1

// ============================================================================
// Shader Mode Uniforms
// ============================================================================

// x=blendMode, y=fogMode, z=priGradient, w=secGradient
uniform vec4 u_shaderMode;

// x=alphaTest, y=texturing, z=lighting, w=alphaTestRef
uniform vec4 u_shaderFlags;

// x=detailColor mode, y=detailAlpha mode
uniform vec4 u_detailFunc;

// ============================================================================
// Material Uniforms
// ============================================================================

uniform vec4 u_diffuseColor;
uniform vec4 u_specularColor;
uniform vec4 u_emissiveColor;
uniform vec4 u_ambientColor;
uniform vec4 u_shininess;
uniform vec4 u_opacity;

// ============================================================================
// Light Environment
// ============================================================================

uniform vec4 u_ambientLight;
uniform vec4 u_lightDir[4];    // xyz=direction, w=type (1=dir, 2=point, 3=spot)
uniform vec4 u_lightDiffuse[4]; // xyz=color, w=enable
uniform vec4 u_lightPos[4];    // xyz=position, w=range
uniform vec4 u_camPos;

// ============================================================================
// Fog
// ============================================================================

uniform vec4 u_fogColor;
uniform vec4 u_fogParams; // x=start, y=end, z=density, w=enable (redundant with fogMode)

// ============================================================================
// Texture Samplers
// ============================================================================

SAMPLER2D(s_tex0, 0);
SAMPLER2D(s_tex1, 1);
SAMPLER2D(s_tex2, 2);
SAMPLER2D(s_tex3, 3);

// ============================================================================
// DX8 Texture Stage Combiner Operations
// ============================================================================

#define COMBOP_SELECTARG1   0
#define COMBOP_SELECTARG2   1
#define COMBOP_MODULATE     2
#define COMBOP_ADD          3
#define COMBOP_ADDSIGNED    4
#define COMBOP_SUBTRACT     5
#define COMBOP_BLEND        6
#define COMBOP_DOTPRODUCT3  7

#define TEXARG_TEXTURE      0
#define TEXARG_CURRENT      1
#define TEXARG_CONSTANT     2
#define TEXARG_DIFFUSE      3
#define TEXARG_SPECULAR     4
#define TEXARG_TFACTOR      5

vec4 GetTextureArg(int argType, vec4 texel, vec4 current, vec4 constant, vec4 vertexColor, vec4 vertexColor1, vec4 tFactor)
{
    if (argType == TEXARG_TEXTURE)  return texel;
    if (argType == TEXARG_CURRENT)  return current;
    if (argType == TEXARG_CONSTANT) return constant;
    if (argType == TEXARG_DIFFUSE)  return vertexColor;
    if (argType == TEXARG_SPECULAR) return vertexColor1;
    if (argType == TEXARG_TFACTOR)  return tFactor;
    return vec4_splat(1.0);
}

// Single combiner operation (combines two args)
vec4 CombinerOp(int op, vec4 arg1, vec4 arg2, vec4 alphaArg1, vec4 alphaArg2)
{
    vec4 result;

    if (op == COMBOP_SELECTARG1)
    {
        result = arg1;
    }
    else if (op == COMBOP_SELECTARG2)
    {
        result = arg2;
    }
    else if (op == COMBOP_MODULATE)
    {
        result = arg1 * arg2;
    }
    else if (op == COMBOP_ADD)
    {
        result = arg1 + arg2;
    }
    else if (op == COMBOP_ADDSIGNED)
    {
        // (arg1 - 0.5) + (arg2 - 0.5) + 0.5 = arg1 + arg2 - 0.5
        result = arg1 + arg2 - vec4_splat(0.5);
    }
    else if (op == COMBOP_SUBTRACT)
    {
        result = arg1 - arg2;
    }
    else if (op == COMBOP_BLEND)
    {
        // arg1 * (1 - arg2.a) + arg2 * arg2.a
        result = arg1 * (1.0 - arg2.aaaa) + arg2 * arg2.aaaa;
    }
    else if (op == COMBOP_DOTPRODUCT3)
    {
        float d = dot(arg1.xyz, arg2.xyz);
        result = vec4(vec3_splat(d), 1.0);
    }
    else
    {
        result = arg1;
    }

    return result;
}

// ============================================================================
// Detail Map Operations
// ============================================================================

#define DETAIL_NONE       0
#define DETAIL_SCALE      1
#define DETAIL_SCALE2X    2
#define DETAIL_SCALE2XADD 3
#define DETAIL_ADD        4
#define DETAIL_BLEND      5
#define DETAIL_BLENDALPHA 6

vec3 DetailColorOp(int op, vec3 baseColor, vec3 detailColor, float detailAlpha)
{
    if (op == DETAIL_NONE)
    {
        return baseColor;
    }
    else if (op == DETAIL_SCALE)
    {
        // baseColor * detailColor (scaled darkening)
        return baseColor * detailColor;
    }
    else if (op == DETAIL_SCALE2X)
    {
        // 2 * baseColor * detailColor - baseColor (scaled to center)
        return baseColor * detailColor * vec3_splat(2.0) - baseColor;
    }
    else if (op == DETAIL_SCALE2XADD)
    {
        // 2 * baseColor * detailColor - baseColor + detailColor
        return baseColor * detailColor * vec3_splat(2.0) - baseColor + detailColor;
    }
    else if (op == DETAIL_ADD)
    {
        return baseColor + detailColor;
    }
    else if (op == DETAIL_BLEND)
    {
        return mix(baseColor, detailColor, detailAlpha);
    }
    else if (op == DETAIL_BLENDALPHA)
    {
        // Blend based on detail alpha, but also modulate by base alpha
        return mix(baseColor, detailColor, detailAlpha * baseColor.r);
    }

    return baseColor;
}

float DetailAlphaOp(int op, float baseAlpha, float detailAlpha)
{
    if (op == DETAIL_NONE)
    {
        return baseAlpha;
    }
    else if (op == DETAIL_SCALE)
    {
        return baseAlpha * detailAlpha;
    }
    else if (op == DETAIL_SCALE2X)
    {
        return baseAlpha * detailAlpha * 2.0 - baseAlpha;
    }
    else if (op == DETAIL_SCALE2XADD)
    {
        return baseAlpha * detailAlpha * 2.0 - baseAlpha + detailAlpha;
    }
    else if (op == DETAIL_ADD)
    {
        return baseAlpha + detailAlpha;
    }
    else if (op == DETAIL_BLEND)
    {
        return mix(baseAlpha, detailAlpha, 0.5);
    }
    else if (op == DETAIL_BLENDALPHA)
    {
        return mix(baseAlpha, detailAlpha, baseAlpha);
    }

    return baseAlpha;
}

// ============================================================================
// Lighting Calculation
// ============================================================================

vec3 CalculateLighting(vec3 worldPos, vec3 normal, vec3 diffuseColor, vec3 specularColor, float shininess)
{
    vec3 result = vec3_splat(0.0);

    if (u_shaderFlags.z < 0.5)
    {
        // Lighting disabled, return diffuse color
        return diffuseColor;
    }

    // Ambient term
    vec3 ambient = u_ambientLight.xyz * diffuseColor;
    result += ambient;

    vec3 viewDir = normalize(u_camPos.xyz - worldPos);

    for (int i = 0; i < 4; i++)
    {
        // Light enabled if diffuse alpha (w) > 0
        if (u_lightDiffuse[i].w < 0.5) continue;

        vec3 lightColor = u_lightDiffuse[i].xyz;
        vec3 lightDir;
        float attenuation = 1.0;
        int lightType = int(u_lightDir[i].w + 0.5);

        if (lightType == 2) // Point light
        {
            vec3 toLight = u_lightPos[i].xyz - worldPos;
            float dist = length(toLight);
            lightDir = normalize(toLight);
            float range = u_lightPos[i].w;
            if (dist > range) continue;
            attenuation = 1.0 - (dist / range);
            attenuation = max(attenuation, 0.0);
            attenuation = pow(attenuation, 2.0);
        }
        else if (lightType == 3) // Spot light
        {
            vec3 toLight = u_lightPos[i].xyz - worldPos;
            float dist = length(toLight);
            lightDir = normalize(toLight);
            float range = u_lightPos[i].w;
            if (dist > range) continue;
            attenuation = 1.0 - (dist / range);
            attenuation = max(attenuation, 0.0);
            attenuation = pow(attenuation, 2.0);
        }
        else // Directional (type 1) or default
        {
            lightDir = -normalize(u_lightDir[i].xyz);
        }

        // Diffuse
        float NdotL = max(dot(normal, lightDir), 0.0);
        vec3 diff = diffuseColor * lightColor * NdotL;

        // Specular (Blinn-Phong)
        vec3 halfDir = normalize(lightDir + viewDir);
        float NdotH = max(dot(normal, halfDir), 0.0);
        float specFactor = pow(NdotH, shininess);
        vec3 spec = specularColor * lightColor * specFactor;

        result += (diff + spec) * attenuation;
    }

    return result;
}

// ============================================================================
// Fog Calculation
// ============================================================================

float CalculateFogFactor(float dist)
{
    if (u_shaderMode.y < 1.5) // FOG_NONE
    {
        return 1.0;
    }
    else if (u_shaderMode.y < 2.5) // FOG_LINEAR
    {
        float fogFactor = (u_fogParams.y - dist) / (u_fogParams.y - u_fogParams.x);
        return clamp(fogFactor, 0.0, 1.0);
    }
    else if (u_shaderMode.y < 3.5) // FOG_EXP
    {
        float fogDensity = u_fogParams.z;
        return exp(-dist * fogDensity);
    }
    else // FOG_EXP2
    {
        float fogDensity = u_fogParams.z;
        float d = dist * fogDensity;
        return exp(-(d * d));
    }

    return 1.0;
}

// ============================================================================
// Main Fragment Shader
// ============================================================================

void main()
{
    // =========================================================================
    // Stage 1: Texture Sampling & Detail Maps
    // =========================================================================

    vec4 texColor0 = vec4_splat(1.0);
    vec4 texColor1 = vec4_splat(1.0);
    vec4 texColor2 = vec4_splat(1.0);
    vec4 texColor3 = vec4_splat(1.0);

    // Sample textures based on texturing flag
    if (u_shaderFlags.y > 0.5)
    {
        texColor0 = texture2D(s_tex0, v_texcoord0);
    }

    // Sample secondary texture (detail map, lightmap, etc.)
    if (u_shaderFlags.y > 0.5)
    {
        texColor1 = texture2D(s_tex1, v_texcoord1);
    }

    // Sample third texture (only if texturing enabled)
    if (u_shaderFlags.y > 0.5)
    {
        texColor2 = texture2D(s_tex2, v_texcoord2);
        texColor3 = texture2D(s_tex3, v_texcoord3);
    }

    // =========================================================================
    // Stage 2: Apply Detail Maps (color and alpha)
    // =========================================================================

    vec3 baseColor = texColor0.rgb;
    float baseAlpha = texColor0.a;

    int detailColorMode = int(u_detailFunc.x + 0.5);
    int detailAlphaMode = int(u_detailFunc.y + 0.5);

    baseColor = DetailColorOp(detailColorMode, baseColor, texColor1.rgb, texColor1.a);
    baseAlpha = DetailAlphaOp(detailAlphaMode, baseAlpha, texColor1.a);

    // =========================================================================
    // Stage 3: DX8 Texture Combiner (texture * vertex color)
    // =========================================================================

    // Simple modulate: texColor0 * v_color0
    vec4 combinedColor = texColor0 * v_color0;

    // Use vertex color alpha
    combinedColor.a = texColor0.a * v_color0.a;

    // =========================================================================
    // Stage 4: Apply Material Colors
    // =========================================================================

    vec4 diffuse = combinedColor * u_diffuseColor;

    // Override alpha with computed value
    diffuse.a = combinedColor.a * u_opacity.x;

    // =========================================================================
    // Stage 5: Alpha Test
    // =========================================================================

    if (u_shaderFlags.x > 0.5) // Alpha test enabled
    {
        if (diffuse.a < u_shaderFlags.w)
        {
            discard;
        }
    }

    // =========================================================================
    // Stage 6: Lighting
    // =========================================================================

    vec3 normal = normalize(v_normal);
    vec3 litColor = CalculateLighting(v_worldPos, normal, diffuse.rgb, u_specularColor.rgb, u_shininess.x);

    // =========================================================================
    // Stage 7: Primary Gradient
    // =========================================================================

    int priGradMode = int(u_shaderMode.z + 0.5);

    if (priGradMode == PRIGRAD_NONE)
    {
        // Keep litColor as is
    }
    else if (priGradMode == PRIGRAD_MOD)
    {
        // Modulate: litColor * diffuseColor
        litColor *= u_diffuseColor.rgb;
    }
    else if (priGradMode == PRIGRAD_ADD)
    {
        // Add: litColor + diffuseColor
        litColor += u_diffuseColor.rgb;
    }

    // =========================================================================
    // Stage 8: Emissive
    // =========================================================================

    litColor += u_emissiveColor.rgb;

    // =========================================================================
    // Stage 9: Secondary Gradient (Specular Add)
    // =========================================================================

    int secGradMode = int(u_shaderMode.w + 0.5);

    if (secGradMode == SECGRAD_ADD)
    {
        litColor += u_specularColor.rgb;
    }

    // =========================================================================
    // Stage 10: Fog
    // =========================================================================

    vec3 finalColor = litColor;
    float fogFactor = CalculateFogFactor(length(u_camPos.xyz - v_worldPos));
    finalColor = mix(u_fogColor.rgb, finalColor, fogFactor);

    // =========================================================================
    // Stage 11: Output
    // =========================================================================

    float alpha = diffuse.a;

    gl_FragColor = vec4(finalColor, alpha);
}
