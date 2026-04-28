/*
**	Command & Conquer Renegade(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : ww3d                                                         *
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * BGFXVertexBuffer - WW3D vertex buffer implementation using bgfx.                          *
 *                                                                                             *
 * Maps all WW3D vertex formats (FVF codes) to bgfx vertex layouts.                          *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

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
    bool m_ownsData;
};

#endif // BGFXVERTEXBUFFER_H
