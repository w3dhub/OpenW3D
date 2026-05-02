#include "ShaderVariantCache.h"
#include <bx/file.h>
#include <cstdio>
#include <cstdlib>

// Release callback for bgfx::makeRef — frees memory allocated with malloc
static void ReleaseShaderMemory(void* _ptr, void* _userData)
{
    free(_ptr);
    (void)_userData;
}

bgfx::ShaderHandle ShaderVariantCache::LoadShader(const std::string& path)
{
    bx::FileReader reader;
    if (!bx::open(&reader, path.c_str()))
    {
        return BGFX_INVALID_HANDLE;
    }

    const uint32_t size = static_cast<uint32_t>(bx::getSize(&reader));
    if (size == 0)
    {
        bx::close(&reader);
        return BGFX_INVALID_HANDLE;
    }

    // Allocate buffer that will persist until bgfx releases it
    void* buffer = malloc(size);
    if (!buffer)
    {
        bx::close(&reader);
        return BGFX_INVALID_HANDLE;
    }

    bx::read(&reader, buffer, size, bx::ErrorAssert{});
    bx::close(&reader);

    // Create bgfx memory reference with release callback
    const bgfx::Memory* mem = bgfx::makeRef(buffer, size, ReleaseShaderMemory, nullptr);
    if (!mem || !mem->data)
    {
        free(buffer);
        return BGFX_INVALID_HANDLE;
    }

    bgfx::ShaderHandle handle = bgfx::createShader(mem);
    if (!bgfx::isValid(handle))
    {
        return BGFX_INVALID_HANDLE;
    }

    return handle;
}

std::string ShaderVariantCache::GetShaderPath(const ShaderKey& key, bool vertex)
{
    if (vertex)
    {
        return "shaders/d3d11/vs_uber.bin";
    }
    else
    {
        return "shaders/d3d11/fs_uber.bin";
    }
}

const ShaderVariantCache::ShaderVariant* ShaderVariantCache::GetOrCreate(const ShaderKey& key)
{
    auto it = m_cache.find(key);
    if (it != m_cache.end())
    {
        it->second.uses++;
        return &it->second;
    }

    // Create new variant
    ShaderVariant variant = LoadOrCreateProgram(key);
    if (!bgfx::isValid(variant.program))
    {
        return nullptr;
    }

    variant.uses = 1;
    auto result = m_cache.emplace(key, std::move(variant));
    return &result.first->second;
}

ShaderVariantCache::ShaderVariant ShaderVariantCache::LoadOrCreateProgram(const ShaderKey& key)
{
    ShaderVariant variant;

    std::string vsPath = GetShaderPath(key, true);
    std::string fsPath = GetShaderPath(key, false);

    variant.vs = LoadShader(vsPath);
    if (!bgfx::isValid(variant.vs))
    {
        return variant;
    }

    variant.fs = LoadShader(fsPath);
    if (!bgfx::isValid(variant.fs))
    {
        bgfx::destroy(variant.vs);
        variant.vs = BGFX_INVALID_HANDLE;
        return variant;
    }

    variant.program = bgfx::createProgram(variant.vs, variant.fs, true /* destroy shaders when program is destroyed */);
    if (!bgfx::isValid(variant.program))
    {
        variant.vs = BGFX_INVALID_HANDLE;
        variant.fs = BGFX_INVALID_HANDLE;
        return variant;
    }

    // Since createProgram was called with _destroyShaders=true,
    // bgfx owns the shader handles now — clear them from our struct
    // so we don't double-destroy in Clear().
    variant.vs = BGFX_INVALID_HANDLE;
    variant.fs = BGFX_INVALID_HANDLE;

    return variant;
}

void ShaderVariantCache::Clear()
{
    for (auto& pair : m_cache)
    {
        ShaderVariant& variant = pair.second;
        if (bgfx::isValid(variant.program))
        {
            bgfx::destroy(variant.program);
        }
        if (bgfx::isValid(variant.vs))
        {
            bgfx::destroy(variant.vs);
        }
        if (bgfx::isValid(variant.fs))
        {
            bgfx::destroy(variant.fs);
        }
    }
    m_cache.clear();
}
