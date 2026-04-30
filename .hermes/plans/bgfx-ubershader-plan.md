# Phase 2: BGFX Ubershader Layer — Implementation Plan

> **For Hermes:** Use subagent-driven-development skill to implement this plan task-by-task.

**Goal:** Build a programmable ubershader system that replaces the fixed-function DX8 shader presets with a single vertex/fragment shader pair controlled by uniforms, enabling all ~22 ShaderClass presets to render correctly through BGFX.

**Architecture:** One vertex shader (`vs_uber.sc`) + one fragment shader (`fs_uber.sc`) that handles all rendering modes via uniforms driven by ShaderClass bits. A `ShaderKey` struct extracts relevant bits from ShaderClass to select shader variants. A variant cache manages compiled bgfx programs. Multi-texture support via dynamic stage loops in the fragment shader.

**Tech Stack:** C++20, BGFX shaderc, GLSL/HLSL via bgfx, DX8 ShaderClass mapping

---

## Current State (what we have)
- ✅ PR 1+2+3 compile: `ninja ww3d2` and `ninja ww3d2e` pass
- ✅ Basic single-texture shader exists: `vs_mesh.sc` / `fs_mesh.sc`
- ✅ Per-pixel lighting (4 lights), fog, alpha test working
- ✅ Backend abstraction with `WW3DBackend` interface
- ✅ Texture bridging, vertex/index buffer management done
- ✅ Material uniforms (diffuse, specular, emissive, ambient, shininess)

## What's Missing (the gap)
The current shader only handles one preset mode. The DX8 engine uses ~22 ShaderClass presets with different:
- **Blend modes**: opaque, additive, alpha, multiplicative, screen
- **Texture stage combiners**: modulate, add, selectarg1/2, dotproduct3, bumpenvmap
- **Post-detail functions**: 9 color ops + 4 alpha ops (detail maps)
- **Gradient modes**: primary (modulate/add/bump/dot3), secondary (enable/disable)
- **Depth states**: 8 compare modes + write enable/disable
- **Alpha test**: on/off with reference value
- **Fog**: 4 modes (disable, linear, scale fragment, white)

---

## Task Breakdown

### Task 1: ShaderKey and Shader Variant Cache

**Objective:** Create a `ShaderKey` struct that extracts relevant bits from ShaderClass and a variant cache for compiled bgfx programs.

**Files:**
- Create: `Code/ww3d2/backends/bgfx/ShaderKey.h`
- Create: `Code/ww3d2/backends/bgfx/ShaderVariantCache.h`
- Modify: `Code/ww3d2/backends/bgfx/bgfxbackend.h` (add includes, forward declare)
- Modify: `Code/ww3d2/backends/bgfx/bgfxbackend.cpp` (add includes)

**Implementation:**

`ShaderKey.h`:
```cpp
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
```

`ShaderVariantCache.h`:
```cpp
#pragma once
#include "ShaderKey.h"
#include <bgfx/bgfx.h>
#include <unordered_map>
#include <string>

class ShaderVariantCache {
public:
    struct ShaderVariant {
        bgfx::ProgramHandle program = BGFX_INVALID_HANDLE;
        bgfx::ShaderHandle vs = BGFX_INVALID_HANDLE;
        bgfx::ShaderHandle fs = BGFX_INVALID_HANDLE;
        uint32_t uses = 0;
    };

    const ShaderVariant* GetOrCreate(const ShaderKey& key);
    void Clear();

private:
    std::unordered_map<ShaderKey, ShaderVariant> m_cache;
    ShaderVariant LoadOrCreateProgram(const ShaderKey& key);
    bgfx::ShaderHandle LoadShader(const std::string& path);
    std::string GetShaderPath(const ShaderKey& key, bool vertex);
};
```

**Step 1: Create ShaderKey.h** — Write the file exactly as shown above.

**Step 2: Create ShaderVariantCache.h** — Write the file exactly as shown above.

**Step 3: Modify bgfxbackend.h** — Add `#include "ShaderKey.h"` and `#include "ShaderVariantCache.h"` near other includes. Add member `ShaderVariantCache m_shaderCache;` to BGFXBackend class.

**Step 4: Verify compilation** — Run on Windows: `ssh ... "cd E:\OpenW3D\build-bgfx && ninja ww3d2"` — expected: builds successfully.

**Step 5: Commit**
```bash
git add Code/ww3d2/backends/bgfx/ShaderKey.h Code/ww3d2/backends/bgfx/ShaderVariantCache.h Code/ww3d2/backends/bgfx/bgfxbackend.h Code/ww3d2/backends/bgfx/bgfxbackend.cpp
git commit -m "feat(bgfx): add ShaderKey and variant cache infrastructure"
```

---

### Task 2: ShaderClass → ShaderKey Translation

**Objective:** Write the translation function that maps ShaderClass bitfield to ShaderKey. This is the critical mapping between DX8 fixed-function state and ubershader uniforms.

**Files:**
- Modify: `Code/ww3d2/backends/bgfx/ShaderKey.h` (add translation function)

**Implementation** — Add to bottom of `ShaderKey.h`:

```cpp
inline ShaderKey ShaderKey_From_DX8(const ShaderClass& shader) {
    ShaderKey key = {};

    // Depth
    key.depthCompare = static_cast<uint8_t>(shader.Get_Depth_Compare());
    key.depthWrite   = (shader.Get_Depth_Mask() == ShaderClass::DEPTH_WRITE_ENABLE) ? 1 : 0;

    // Blend mode mapping (src+dst blend combo → enum)
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
    else if (srcBlend == ShaderClass::SRCBLEND_ONE_MINUS_SRC_COLOR && dstBlend == ShaderClass::DSTBLEND_SRC_COLOR)
        key.blendMode = 4; // screen
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
```

**Step 1: Add translation function** to ShaderKey.h as shown above.

**Step 2: Verify compilation** — `ninja ww3d2`

**Step 3: Commit**
```bash
git add Code/ww3d2/backends/bgfx/ShaderKey.h
git commit -m "feat(bgfx): add ShaderClass → ShaderKey translation function"
```

---

### Task 3: Write the Ubershader Vertex Shader (vs_uber.sc)

**Objective:** Create the ubershader vertex shader that handles all FVF layout variants, transforms, and passes data to the fragment shader.

**Files:**
- Create: `Code/ww3d2/backends/bgfx/shaders/source/vs_uber.sc`
- Modify: `Code/ww3d2/backends/bgfx/shaders/varying.def.sc` (add new varying declarations)

**Implementation:**

`vs_uber.sc`:
```glsl
$input a_position, a_normal, a_texcoord0, a_color0
$input a_texcoord1, a_texcoord2, a_texcoord3, a_color1
$output v_texcoord0, v_texcoord1, v_texcoord2, v_texcoord3
$output v_color0, v_color1, v_normal, v_worldPos

#include <bgfx_shader.sh>

uniform vec4 u_model[4];
uniform vec4 u_viewProj[4];
uniform vec4 u_camPos;
uniform vec4 u_lightingEnable;

void main()
{
    vec4 pos = vec4(a_position, 1.0);
    vec4 worldPos = vec4(
        dot(u_model[0], pos),
        dot(u_model[1], pos),
        dot(u_model[2], pos),
        dot(u_model[3], pos)
    );
    v_worldPos = worldPos.xyz;

    vec4 normal = vec4(a_normal, 0.0);
    v_normal = vec3(
        dot(u_model[0], normal),
        dot(u_model[1], normal),
        dot(u_model[2], normal)
    );

    v_texcoord0 = a_texcoord0;
    v_texcoord1 = a_texcoord1;
    v_texcoord2 = a_texcoord2;
    v_texcoord3 = a_texcoord3;

    v_color0 = a_color0;
    v_color1 = a_color1;

    vec4 viewProjPos = vec4(
        dot(u_viewProj[0], worldPos),
        dot(u_viewProj[1], worldPos),
        dot(u_viewProj[2], worldPos),
        dot(u_viewProj[3], worldPos)
    );
    gl_Position = viewProjPos;
}
```

`varying.def.sc` — append:
```
vec2 v_texcoord1 : TEXCOORD1;
vec2 v_texcoord2 : TEXCOORD2;
vec2 v_texcoord3 : TEXCOORD3;
vec4 v_color1 : COLOR1;
```

**Step 1: Create vs_uber.sc** as shown above.

**Step 2: Modify varying.def.sc** — add new varying declarations.

**Step 3: Compile shader** — Run shaderc to verify:
```bash
# On Windows, from E:\OpenW3D\external\bgfx\tools\bin\windows
shaderc.exe -f E:\OpenW3D\Code\ww3d2\backends\bgfx\shaders\source\vs_uber.sc -o E:\OpenW3D\Code\ww3d2\backends\bgfx\shaders\d3d11\vs_uber.bin --type v --platform windows -p s_5_0 --varyingdef E:\OpenW3D\Code\ww3d2\backends\bgfx\shaders\varying.def.sc
```
Expected: no errors, outputs `.bin` file.

**Step 4: Commit**
```bash
git add Code/ww3d2/backends/bgfx/shaders/source/vs_uber.sc Code/ww3d2/backends/bgfx/shaders/varying.def.sc
git commit -m "feat(bgfx): add ubershader vertex shader with multi-texcoord support"
```

---

### Task 4: Write the Ubershader Fragment Shader (fs_uber.sc)

**Objective:** Create the fragment shader that handles all blend modes, texture stage combiners, fog, alpha test, lighting, and detail maps — the core of the ubershader.

**Files:**
- Create: `Code/ww3d2/backends/bgfx/shaders/source/fs_uber.sc`

**Implementation:**

`fs_uber.sc`:
```glsl
$input v_texcoord0, v_texcoord1, v_texcoord2, v_texcoord3
$input v_color0, v_color1, v_normal, v_worldPos
$output gl_FragColor

#include <bgfx_shader.sh>

// Shader mode uniforms
uniform vec4 u_shaderMode;  // x=blendMode, y=fogMode, z=priGradient, w=secGradient
uniform vec4 u_shaderFlags; // x=alphaTest, y=texturing, z=lighting, w=alphaTestRef
uniform vec4 u_detailFunc;  // x=detailColor func, y=detailAlpha func

// Material
uniform vec4 u_diffuseColor;
uniform vec4 u_specularColor;
uniform vec4 u_emissiveColor;
uniform vec4 u_ambientColor;
uniform vec4 u_shininess;
uniform vec4 u_opacity;

// Light environment
uniform vec4 u_ambientLight;
uniform vec4 u_lightDir[4];
uniform vec4 u_lightDiffuse[4];
uniform vec4 u_lightPos[4];
uniform vec4 u_camPos;

// Fog
uniform vec4 u_fogColor;
uniform vec4 u_fogParams; // x=start, y=end, z=density, w=unused

// Samplers — up to 4 texture stages
SAMPLER2D(s_tex0, 0);
SAMPLER2D(s_tex1, 1);
SAMPLER2D(s_tex2, 2);
SAMPLER2D(s_tex3, 3);

// Helper: DX8 texture stage combiner
vec3 CombinerOp(int op, vec3 arg1, vec3 arg2) {
    switch (op) {
        case 0: return arg1;                                    // SELECTARG1
        case 1: return arg2;                                    // SELECTARG2
        case 2: return arg1 * arg2;                             // MODULATE
        case 3: return arg1 * arg2 * 2.0;                       // MODULATE2X
        case 4: return arg1 * arg2 * 4.0;                       // MODULATE4X
        case 5: return arg1 + arg2;                             // ADD
        case 6: return arg1 + arg2 - 0.5;                       // ADDSIGNED
        case 7: return (arg1 + arg2 - 0.5) * 2.0;              // ADDSIGNED2X
        case 8: return arg1 - arg2;                             // SUBTRACT
        case 9: return arg1 + arg2 - arg1 * arg2;              // ADDSMOOTH
        case 10: return dot(arg1.rgb, arg2.rgb);               // DOTPRODUCT3 (returns scalar)
        default: return arg1;
    }
}

// Helper: Detail color function
vec3 DetailColorOp(int op, vec3 local, vec3 other, float localAlpha) {
    switch (op) {
        case 0: return local;                                          // DISABLE
        case 1: return other;                                          // DETAIL
        case 2: return local * other;                                  // SCALE
        case 3: return local + other - local * other;                  // INVSCALE
        case 4: return local + other;                                  // ADD
        case 5: return local - other;                                  // SUB
        case 6: return other - local;                                  // SUBR
        case 7: return localAlpha * local + (1.0 - localAlpha) * other;// BLEND
        case 8: return other.a * local + (1.0 - other.a) * other;     // DETAILBLEND
        default: return local;
    }
}

vec3 CalculateLighting(vec3 worldPos, vec3 normal, vec3 diffuseColor, vec3 specularColor, float shininess)
{
    vec3 result = vec3_splat(0.0);
    if (u_shaderFlags.z < 0.5) return diffuseColor;

    vec3 ambient = u_ambientLight.xyz * diffuseColor;
    result += ambient;

    vec3 viewDir = normalize(u_camPos.xyz - worldPos);

    for (int i = 0; i < 4; i++) {
        if (u_lightDiffuse[i].w < 0.5) continue;

        vec3 lightColor = u_lightDiffuse[i].xyz;
        vec3 lightDir;
        float attenuation = 1.0;
        int lightType = int(u_lightDir[i].w + 0.5);

        if (lightType == 2) {
            vec3 toLight = u_lightPos[i].xyz - worldPos;
            float dist = length(toLight);
            lightDir = normalize(toLight);
            float range = u_lightPos[i].w;
            if (dist > range) continue;
            attenuation = 1.0 - (dist / range);
            attenuation = max(attenuation, 0.0);
            attenuation = pow(attenuation, 2.0);
        } else {
            lightDir = -normalize(u_lightDir[i].xyz);
        }

        float NdotL = max(dot(normal, lightDir), 0.0);
        vec3 diff = diffuseColor * lightColor * NdotL;

        vec3 halfDir = normalize(lightDir + viewDir);
        float NdotH = max(dot(normal, halfDir), 0.0);
        float specFactor = pow(NdotH, shininess);
        vec3 spec = specularColor * lightColor * specFactor;

        result += (diff + spec) * attenuation;
    }
    return result;
}

void main()
{
    int blendMode  = int(u_shaderMode.x + 0.5);
    int fogMode    = int(u_shaderMode.y + 0.5);
    int priGrad    = int(u_shaderMode.z + 0.5);
    int secGrad    = int(u_shaderMode.w + 0.5);
    bool alphaTest = u_shaderFlags.x > 0.5;
    bool texturing = u_shaderFlags.y > 0.5;
    bool lighting  = u_shaderFlags.z > 0.5;
    float alphaRef = u_shaderFlags.w;

    int detailColorOp = int(u_detailFunc.x + 0.5);
    int detailAlphaOp = int(u_detailFunc.y + 0.5);

    // Base color from vertex
    vec4 baseColor = v_color0;

    // Texture sampling
    vec4 texColor = vec4(1.0);
    if (texturing) {
        texColor = texture2D(s_tex0, v_texcoord0);

        // Multi-stage: apply detail texture if enabled
        if (detailColorOp > 0) {
            vec4 detailTex = texture2D(s_tex1, v_texcoord1 * 4.0); // detail usually tiled
            texColor.rgb = DetailColorOp(detailColorOp, texColor.rgb, detailTex.rgb, texColor.a);
            texColor.a = DetailColorOp(detailAlphaOp, texColor.a, detailTex.a, texColor.a);
        }
    }

    // Primary gradient (texture + vertex color combination)
    vec4 diffuse;
    switch (priGrad) {
        case 0: // DISABLE (decal — texture replaces)
            diffuse = texturing ? texColor : vec4(1.0);
            break;
        case 1: // MODULATE
            diffuse = texturing ? texColor * baseColor : baseColor;
            break;
        case 2: // ADD
            diffuse = texturing ? texColor + baseColor : baseColor;
            break;
        case 3: // BUMPENVMAP — handled separately (needs tangent space)
        case 4: // BUMPENVMAPLUMINANCE
        case 5: // DOTPRODUCT3
            diffuse = texturing ? texColor : baseColor; // fallback
            break;
        default:
            diffuse = texturing ? texColor : baseColor;
            break;
    }

    diffuse *= u_diffuseColor;

    // Alpha test
    if (alphaTest && diffuse.a < alphaRef) {
        discard;
    }

    // Lighting
    vec3 litColor = CalculateLighting(v_worldPos, normalize(v_normal), diffuse.rgb, u_specularColor.rgb, u_shininess.x);

    // Emissive
    vec3 finalColor = litColor + u_emissiveColor.rgb;

    // Secondary gradient
    if (secGrad > 0) {
        vec4 specGrad = v_color1;
        finalColor += specGrad.rgb;
    }

    // Apply opacity
    float alpha = diffuse.a * u_opacity.x;

    // Fog
    if (fogMode > 0) {
        float dist = length(u_camPos.xyz - v_worldPos);
        float fogFactor;

        if (fogMode == 1) { // Linear
            fogFactor = (u_fogParams.y - dist) / (u_fogParams.y - u_fogParams.x);
        } else if (fogMode == 2) { // Exp
            fogFactor = exp(-u_fogParams.z * dist);
        } else { // Exp2
            float d = u_fogParams.z * dist;
            fogFactor = exp(-d * d);
        }
        fogFactor = clamp(fogFactor, 0.0, 1.0);

        if (fogMode == 3) { // White fog (replace)
            finalColor = mix(u_fogColor.rgb, finalColor, fogFactor);
        } else {
            finalColor = mix(u_fogColor.rgb, finalColor, fogFactor);
        }
    }

    gl_FragColor = vec4(finalColor, alpha);
}
```

**Step 1: Create fs_uber.sc** as shown above.

**Step 2: Compile shader** — Run shaderc:
```bash
shaderc.exe -f E:\OpenW3D\Code\ww3d2\backends\bgfx\shaders\source\fs_uber.sc -o E:\OpenW3D\Code\ww3d2\backends\bgfx\shaders\d3d11\fs_uber.bin --type f --platform windows -p s_5_0 --varyingdef E:\OpenW3D\Code\ww3d2\backends\bgfx\shaders\varying.def.sc
```
Expected: no errors, outputs `.bin` file.

**Step 3: Commit**
```bash
git add Code/ww3d2/backends/bgfx/shaders/source/fs_uber.sc
git commit -m "feat(bgfx): add ubershader fragment shader with all preset modes"
```

---

### Task 5: Shader Loading and Program Creation

**Objective:** Implement shader loading (bgfx::createShader from .bin files) and program creation in ShaderVariantCache.

**Files:**
- Modify: `Code/ww3d2/backends/bgfx/ShaderVariantCache.cpp` (create new file)
- Modify: `Code/ww3d2/backends/bgfx/CMakeLists.txt` (add ShaderVariantCache.cpp to sources)

**Implementation:**

`ShaderVariantCache.cpp`:
```cpp
#include "ShaderVariantCache.h"
#include <bgfx/bgfx.h>
#include <bx/file.h>
#include <string>
#include <vector>
#include <iostream>

bgfx::ShaderHandle ShaderVariantCache::LoadShader(const std::string& path) {
    bx::FileReader reader;
    if (!reader.open(path.c_str())) {
        std::cerr << "Failed to load shader: " << path << std::endl;
        return BGFX_INVALID_HANDLE;
    }

    bx::StaticMemoryBlockWriter writer;
    // Read file size
    int64_t size = reader.getSize();
    std::vector<char> data(size);
    reader.read(data.data(), size);
    reader.close();

    return bgfx::createShader(bgfx::makeRef(data.data(), size));
}

std::string ShaderVariantCache::GetShaderPath(const ShaderKey& key, bool vertex) {
    // For Phase 2, we use a single uber shader pair.
    // In Phase 3, this will return different .bin files based on key.
    const char* name = vertex ? "vs_uber" : "fs_uber";
    return "shaders/d3d11/" + std::string(name) + ".bin";
}

const ShaderVariant* ShaderVariantCache::GetOrCreate(const ShaderKey& key) {
    auto it = m_cache.find(key);
    if (it != m_cache.end()) {
        it->second.uses++;
        return &it->second;
    }

    ShaderVariant variant = LoadOrCreateProgram(key);
    if (variant.program.idx == bgfx::kInvalidHandle) {
        return nullptr;
    }

    auto result = m_cache.emplace(key, variant);
    result.first->second.uses++;
    return &result.first->second;
}

ShaderVariant ShaderVariantCache::LoadOrCreateProgram(const ShaderKey& key) {
    ShaderVariant variant;

    std::string vsPath = GetShaderPath(key, true);
    std::string fsPath = GetShaderPath(key, false);

    variant.vs = LoadShader(vsPath);
    variant.fs = LoadShader(fsPath);

    if (variant.vs.idx == bgfx::kInvalidHandle || variant.fs.idx == bgfx::kInvalidHandle) {
        if (variant.vs.idx != bgfx::kInvalidHandle) bgfx::destroy(variant.vs);
        if (variant.fs.idx != bgfx::kInvalidHandle) bgfx::destroy(variant.fs);
        return variant;
    }

    variant.program = bgfx::createProgram(variant.vs, variant.fs, true);
    return variant;
}

void ShaderVariantCache::Clear() {
    for (auto& pair : m_cache) {
        if (bgfx::isValid(pair.second.program)) {
            bgfx::destroy(pair.second.program);
        }
    }
    m_cache.clear();
}
```

**Step 1: Create ShaderVariantCache.cpp** as shown above.

**Step 2: Modify CMakeLists.txt** — add `ShaderVariantCache.cpp` to the bgfx backend source list in `backends/bgfx/CMakeLists.txt`.

**Step 3: Verify compilation** — `ninja ww3d2`

**Step 4: Commit**
```bash
git add Code/ww3d2/backends/bgfx/ShaderVariantCache.cpp Code/ww3d2/backends/bgfx/CMakeLists.txt
git commit -m "feat(bgfx): implement shader loading and variant cache"
```

---

### Task 6: Apply Shader State in BGFXBackend

**Objective:** Wire up `Apply_Shader_State()` to extract ShaderKey, get/create the program, set all uniforms, and configure bgfx render state (blend, depth, stencil).

**Files:**
- Modify: `Code/ww3d2/backends/bgfx/bgfxbackend.cpp` (Apply_Shader_State implementation)

**Implementation:**

Add/modify `Apply_Shader_State` in bgfxbackend.cpp:
```cpp
void BGFXBackend::Apply_Shader_State(const ShaderClass& shader)
{
    ShaderKey key = ShaderKey_From_DX8(shader);
    const ShaderVariant* variant = m_shaderCache.GetOrCreate(key);
    if (!variant || !bgfx::isValid(variant->program)) {
        // Fallback to default program
        if (bgfx::isValid(m_defaultProgram)) {
            bgfx::setProgram(m_defaultProgram);
        }
        return;
    }

    bgfx::setProgram(variant->program);

    // Set shader uniforms
    float shaderMode[4] = {
        static_cast<float>(key.blendMode),
        static_cast<float>(key.fogMode),
        static_cast<float>(key.priGradient),
        static_cast<float>(key.secGradient)
    };
    bgfx::setUniform(m_shaderModeUniform, shaderMode);

    float shaderFlags[4] = {
        static_cast<float>(key.alphaTest),
        static_cast<float>(key.texturing),
        static_cast<float>(key.lighting),
        key.alphaTest ? (m_alphaTestRef / 255.0f) : 0.0f
    };
    bgfx::setUniform(m_shaderFlagsUniform, shaderFlags);

    float detailFunc[4] = {
        static_cast<float>(key.detailColor),
        static_cast<float>(key.detailAlpha),
        0.0f, 0.0f
    };
    bgfx::setUniform(m_detailFuncUniform, detailFunc);

    // Configure render state from shader bits
    uint64_t state = 0;

    // Depth
    switch (shader.Get_Depth_Compare()) {
        case ShaderClass::PASS_NEVER:    state |= BGFX_STATE_DEPTH_TEST_NEVER; break;
        case ShaderClass::PASS_LESS:     state |= BGFX_STATE_DEPTH_TEST_LESS; break;
        case ShaderClass::PASS_EQUAL:    state |= BGFX_STATE_DEPTH_TEST_EQUAL; break;
        case ShaderClass::PASS_LEQUAL:   state |= BGFX_STATE_DEPTH_TEST_LEQUAL; break;
        case ShaderClass::PASS_GREATER:  state |= BGFX_STATE_DEPTH_TEST_GREATER; break;
        case ShaderClass::PASS_NOTEQUAL: state |= BGFX_STATE_DEPTH_TEST_NOTEQUAL; break;
        case ShaderClass::PASS_GEQUAL:   state |= BGFX_STATE_DEPTH_TEST_GEQUAL; break;
        case ShaderClass::PASS_ALWAYS:   state |= BGFX_STATE_DEPTH_TEST_ALWAYS; break;
    }
    if (shader.Get_Depth_Mask() == ShaderClass::DEPTH_WRITE_ENABLE)
        state |= BGFX_STATE_WRITE_Z;

    // Color write
    if (shader.Get_Color_Mask() != ShaderClass::COLOR_WRITE_ENABLE)
        state |= BGFX_STATE_RGB_WRITE | BGFX_STATE_ALPHA_WRITE; // inverted — skip write

    // Culling
    if (shader.Get_Cull_Mode() == ShaderClass::CULL_MODE_ENABLE)
        state |= BGFX_STATE_CULL_CW;

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
        default:
            state |= BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_ONE, BGFX_STATE_BLEND_ZERO);
            break;
    }

    bgfx::setState(state);
}
```

**Step 1: Add new uniform handles** to bgfxbackend.h:
```cpp
bgfx::UniformHandle m_shaderModeUniform;
bgfx::UniformHandle m_shaderFlagsUniform;
bgfx::UniformHandle m_detailFuncUniform;
```

**Step 2: Initialize uniforms** in Init() — add:
```cpp
m_shaderModeUniform = bgfx::createUniform("u_shaderMode", bgfx::UniformType::Vec4);
m_shaderFlagsUniform = bgfx::createUniform("u_shaderFlags", bgfx::UniformType::Vec4);
m_detailFuncUniform = bgfx::createUniform("u_detailFunc", bgfx::UniformType::Vec4);
```

**Step 3: Implement Apply_Shader_State** as shown above.

**Step 4: Wire up in Draw call** — in `Draw_IB_VB` or similar, call:
```cpp
const ShaderClass& currentShader = WW3D::Get_Backend()->GetCurrentShader();
Apply_Shader_State(currentShader);
```

**Step 5: Verify compilation** — `ninja ww3d2`

**Step 6: Commit**
```bash
git add Code/ww3d2/backends/bgfx/bgfxbackend.cpp Code/ww3d2/backends/bgfx/bgfxbackend.h
git commit -m "feat(bgfx): wire Apply_Shader_State with ubershader and render state"
```

---

### Task 7: Multi-Texture Stage Support

**Objective:** Track active texture stages and update ShaderKey's texStageCount. Bind multiple texture samplers per stage.

**Files:**
- Modify: `Code/ww3d2/backends/bgfx/bgfxbackend.cpp` (texture stage tracking)
- Modify: `Code/ww3d2/backends/bgfx/BGFXTexture.cpp` (multi-stage binding)

**Implementation:**

Add to bgfxbackend.h private members:
```cpp
int m_activeTextureStages = 0;
std::array<bgfx::TextureHandle, 8> m_boundTextures;
std::array<bgfx::UniformHandle, 8> m_samplerUniforms;
```

In texture binding code (Set_Texture or similar):
```cpp
void BGFXBackend::Set_Texture_Stage(int stage, bgfx::TextureHandle tex, bgfx::UniformHandle sampler) {
    if (stage >= 0 && stage < 8) {
        m_boundTextures[stage] = tex;
        m_samplerUniforms[stage] = sampler;
        m_activeTextureStages = std::max(m_activeTextureStages, stage + 1);
    }
}
```

In Apply_Shader_State or before draw:
```cpp
for (int i = 0; i < m_activeTextureStages; ++i) {
    if (bgfx::isValid(m_boundTextures[i])) {
        bgfx::setTexture(static_cast<uint8_t>(i), m_samplerUniforms[i], m_boundTextures[i]);
    }
}
```

**Step 1: Add texture stage tracking** to bgfxbackend.h.

**Step 2: Modify texture binding** to use stage-aware binding.

**Step 3: Update Apply_Shader_State** to bind all active stages.

**Step 4: Verify compilation** — `ninja ww3d2`

**Step 5: Commit**
```bash
git add Code/ww3d2/backends/bgfx/bgfxbackend.cpp Code/ww3d2/backends/bgfx/bgfxbackend.h Code/ww3d2/backends/bgfx/BGFXTexture.cpp
git commit -m "feat(bgfx): add multi-texture stage support to ubershader"
```

---

### Task 8: Shader Compilation Pipeline (CMake)

**Objective:** Set up CMake to compile `.sc` shaders to `.bin` files for all target renderers (s_5_0 for D3D11, spirv for Vulkan, 120 for GL).

**Files:**
- Modify: `Code/ww3d2/backends/bgfx/CMakeLists.txt`

**Implementation:**

Add to CMakeLists.txt:
```cmake
# Shader compilation
set(BGFX_SHADER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
set(BGFX_SHADER_SRC "${BGFX_SHADER_DIR}/source")
set(BGFX_SHADER_OUT "${BGFX_SHADER_DIR}/d3d11")

# Find shaderc
find_program(SHADERC_EXECUTABLE NAMES shaderc HINTS "${BGFX_DIR}/tools/bin/windows")

if(SHADERC_EXECUTABLE)
    file(GLOB BGFX_VS_SOURCES "${BGFX_SHADER_SRC}/vs_*.sc")
    file(GLOB BGFX_FS_SOURCES "${BGFX_SHADER_SRC}/fs_*.sc")

    foreach(VS_SOURCE ${BGFX_VS_SOURCES})
        get_filename_component(VS_NAME ${VS_SOURCE} NAME_WE)
        set(VS_OUTPUT "${BGFX_SHADER_OUT}/${VS_NAME}.bin")
        add_custom_command(
            OUTPUT ${VS_OUTPUT}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${BGFX_SHADER_OUT}
            COMMAND ${SHADERC_EXECUTABLE}
                -f ${VS_SOURCE}
                -o ${VS_OUTPUT}
                --type v
                --platform windows
                -p s_5_0
                --varyingdef ${BGFX_SHADER_DIR}/varying.def.sc
            DEPENDS ${VS_SOURCE} ${BGFX_SHADER_DIR}/varying.def.sc
            COMMENT "Compiling vertex shader ${VS_NAME}"
        )
        list(APPEND BGFX_SHADER_OUTPUTS ${VS_OUTPUT})
    endforeach()

    foreach(FS_SOURCE ${BGFX_FS_SOURCES})
        get_filename_component(FS_NAME ${FS_SOURCE} NAME_WE)
        set(FS_OUTPUT "${BGFX_SHADER_OUT}/${FS_NAME}.bin")
        add_custom_command(
            OUTPUT ${FS_OUTPUT}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${BGFX_SHADER_OUT}
            COMMAND ${SHADERC_EXECUTABLE}
                -f ${FS_SOURCE}
                -o ${FS_OUTPUT}
                --type f
                --platform windows
                -p s_5_0
                --varyingdef ${BGFX_SHADER_DIR}/varying.def.sc
            DEPENDS ${FS_SOURCE} ${BGFX_SHADER_DIR}/varying.def.sc
            COMMENT "Compiling fragment shader ${FS_NAME}"
        )
        list(APPEND BGFX_SHADER_OUTPUTS ${FS_OUTPUT})
    endforeach()

    add_custom_target(bgfx_shaders DEPENDS ${BGFX_SHADER_OUTPUTS})
    add_dependencies(ww3d2 bgfx_shaders)
endif()
```

**Step 1: Modify CMakeLists.txt** as shown above.

**Step 2: Test build** — `cmake .. -GNinja -DENABLE_BGFX_BACKEND=ON -DWANT_DX9=ON` then `ninja bgfx_shaders`

**Step 3: Verify .bin files created** — check `shaders/d3d11/vs_uber.bin` and `shaders/d3d11/fs_uber.bin` exist.

**Step 4: Commit**
```bash
git add Code/ww3d2/backends/bgfx/CMakeLists.txt
git commit -m "build(bgfx): add shaderc compilation pipeline for ubershaders"
```

---

## Verification Checklist

After all tasks complete, verify:
- [ ] `ninja ww3d2` compiles without errors
- [ ] `ninja ww3d2e` compiles without errors
- [ ] Shader `.bin` files generated in `shaders/d3d11/`
- [ ] All 22 ShaderClass presets map to valid ShaderKey combinations
- [ ] Blend modes work: opaque, additive, alpha, multiplicative, screen
- [ ] Fog modes work: linear, exp, exp2
- [ ] Alpha test with reference value works
- [ ] Texture stages bind correctly (1-4 stages)
- [ ] Detail maps work (post-detail color/alpha functions)
- [ ] No shader permutation explosion (single vs_uber + fs_uber pair)

## Execution Order

1. **Task 1**: ShaderKey + variant cache (infrastructure)
2. **Task 2**: ShaderClass → ShaderKey translation (mapping)
3. **Task 3**: Vertex ubershader (vs_uber.sc)
4. **Task 4**: Fragment ubershader (fs_uber.sc) — **CORE**
5. **Task 5**: Shader loading (runtime)
6. **Task 6**: Apply shader state (wiring)
7. **Task 7**: Multi-texture support (completeness)
8. **Task 8**: CMake shader compilation (build system)

Tasks 1-2 can be batched (no file conflicts). Tasks 3-4 are independent (different files). Tasks 5-6 depend on 3-4. Task 7 depends on 6. Task 8 is independent.

## Risks and Mitigations

1. **Shader compilation errors**: bgfx shaderc has limited GLSL support. Test each shader independently before integration.
2. **Performance**: Dynamic branching in fragment shader may hurt performance on older GPUs. Profile early.
3. **Missing preset coverage**: Some presets (bump mapping, environment maps) need additional work beyond Phase 2. Document as known gaps.
4. **Texture format compatibility**: DXT compression must work with bgfx samplers. Already handled in PR 3.
