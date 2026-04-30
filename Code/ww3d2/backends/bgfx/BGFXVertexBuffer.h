// BGFXVertexBuffer.h - OpenW3D render backend

#ifndef BGFXVERTEXBUFFER_H
#define BGFXVERTEXBUFFER_H

#include "bgfx/bgfx.h"

// Vertex format enumeration for DX8-style FVF vertex layouts
enum WW3DVertexFormat {
    WW3D_FORMAT_XYZ,         // XYZ only
    WW3D_FORMAT_XYZUV1,      // XYZ + 1 UV set
    WW3D_FORMAT_XYZUV2,      // XYZ + 2 UV sets
    WW3D_FORMAT_XYZDUV1,     // XYZ + Diffuse + 1 UV set
    WW3D_FORMAT_XYZDUV2,     // XYZ + Diffuse + 2 UV sets
    WW3D_FORMAT_XYZNV1,      // XYZ + Normal + 1 UV set
    WW3D_FORMAT_XYZNV2,      // XYZ + Normal + 2 UV sets
    WW3D_FORMAT_XYZNDV1,     // XYZ + Normal + Diffuse + 1 UV set
    WW3D_FORMAT_XYZNDV2,     // XYZ + Normal + Diffuse + 2 UV sets
    WW3D_FORMAT_XYZNDUV1,    // XYZ + Normal + Diffuse + 1 UV set (alias)
    WW3D_FORMAT_XYZNDUV2,    // XYZ + Normal + Diffuse + 2 UV sets (alias)
};

// BGFXVertexBuffer - wraps a bgfx vertex buffer with WW3D format support
class BGFXVertexBuffer
{
public:
    BGFXVertexBuffer();
    ~BGFXVertexBuffer();

    // Create from raw data with a specific vertex format
    bool Create(void* data, int vertexCount, WW3DVertexFormat format, uint64_t flags = BGFX_BUFFER_NONE);

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

    // Static helper: Create bgfx vertex layout from vertex format
    static bgfx::VertexLayout Make_Layout(WW3DVertexFormat format);

    // Static helper: Get stride (bytes per vertex) for a vertex format
    static int Get_Stride(WW3DVertexFormat format);

private:
    bgfx::VertexBufferHandle m_handle;
    bgfx::VertexLayout m_layout;
    int m_vertexCount;
};

#endif // BGFXVERTEXBUFFER_H
