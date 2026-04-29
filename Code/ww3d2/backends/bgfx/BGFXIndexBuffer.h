// BGFXIndexBuffer.h - OpenW3D render backend

#ifndef BGFXINDEXBUFFER_H
#define BGFXINDEXBUFFER_H

#include "bgfx/bgfx.h"

// BGFXIndexBuffer - wraps a bgfx index buffer
class BGFXIndexBuffer
{
public:
    BGFXIndexBuffer();
    ~BGFXIndexBuffer();

    // Create from raw data
    bool Create(void* data, int indexCount, bool index32bit = false, uint64_t flags = BGFX_BUFFER_NONE);

    // Destroy the buffer
    void Destroy();

    // Check if buffer is valid
    bool Is_Valid() const { return bgfx::isValid(m_handle); }

    // Get the bgfx handle
    bgfx::IndexBufferHandle Get_Handle() const { return m_handle; }

    // Get index count
    int Get_Index_Count() const { return m_indexCount; }

    // Check if using 32-bit indices
    bool Is_32Bit() const { return m_index32bit; }

private:
    bgfx::IndexBufferHandle m_handle;
    int m_indexCount;
    bool m_index32bit;
};

#endif // BGFXINDEXBUFFER_H
