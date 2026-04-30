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
