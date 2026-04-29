// BGFXVertexBuffer.h - OpenW3D render backend

#ifndef BGFXVERTEXBUFFER_H
#define BGFXVERTEXBUFFER_H

#include "bgfx/bgfx.h"
#include "ww3dformat.h"

// BGFXVertexBuffer - wraps a bgfx vertex buffer with WW3D format support
class BGFXVertexBuffer
{
public:
    BGFXVertexBuffer();
    ~BGFXVertexBuffer();

    // Create from raw data with a specific WW3D format
    bool Create(void* data, int vertexCount, WW3DFormat format, uint64_t flags = BGFX_BUFFER_NONE);

    // Create from raw data with a bgfx vertex layout
    bool Create(void* data, int vertexCount, const bgfx::VertexLayout& layout, uint64_t flags = BGFX_BUFFER_NONE);

    // Destroy the buffer
    void Destroy();

    // Check if buffer is valid
    bool Is_Valid() const { return bgfx::isValid(m_handle); }

    // Get the bgfx handle
    bgfx::VertexBufferHandle Get_Handle() const { return m_handle; }

    // Get vertex count
    int Get_Vertex_Count() const { return m_vertexCount; }

    // Get vertex layout
    const bgfx::VertexLayout& Get_Layout() const { return m_layout; }

    // Static helper: Create bgfx vertex layout from WW3D FVF format
    static bgfx::VertexLayout Make_Layout(WW3DFormat format);

    // Static helper: Get stride (bytes per vertex) for a WW3D format
    static int Get_Stride(WW3DFormat format);

private:
    bgfx::VertexBufferHandle m_handle;
    bgfx::VertexLayout m_layout;
    int m_vertexCount;
};

#endif // BGFXVERTEXBUFFER_H
