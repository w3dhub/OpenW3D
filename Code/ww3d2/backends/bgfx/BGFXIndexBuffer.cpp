// BGFXIndexBuffer.cpp - OpenW3D render backend

#include "backends/bgfx/BGFXIndexBuffer.h"

BGFXIndexBuffer::BGFXIndexBuffer()
    : m_handle(BGFX_INVALID_HANDLE)
    , m_indexCount(0)
    , m_index32bit(false)
{
}

BGFXIndexBuffer::~BGFXIndexBuffer()
{
    Destroy();
}

bool BGFXIndexBuffer::Create(void* data, int indexCount, bool index32bit, uint64_t flags)
{
    Destroy();

    if (!data || indexCount <= 0) {
        return false;
    }

    m_indexCount = indexCount;
    m_index32bit = index32bit;

    const uint32_t indexSize = index32bit ? 4 : 2;
    const uint32_t size = indexCount * indexSize;

    const bgfx::Memory* mem = bgfx::copy(data, size);

    if (index32bit) {
        m_handle = bgfx::createIndexBuffer(mem, flags | BGFX_BUFFER_INDEX32);
    } else {
        m_handle = bgfx::createIndexBuffer(mem, flags);
    }

    return bgfx::isValid(m_handle);
}

void BGFXIndexBuffer::Destroy()
{
    if (bgfx::isValid(m_handle)) {
        bgfx::destroy(m_handle);
        m_handle = BGFX_INVALID_HANDLE;
    }
    m_indexCount = 0;
    m_index32bit = false;
}
