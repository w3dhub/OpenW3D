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
 * BGFXVertexBuffer.cpp - implementation of bgfx vertex buffer wrapper.                    *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "backends/bgfx/BGFXVertexBuffer.h"
#include "ww3dformat.h"

BGFXVertexBuffer::BGFXVertexBuffer()
    : m_handle(BGFX_INVALID_HANDLE)
    , m_vertexCount(0)
    , m_ownsData(false)
{
}

BGFXVertexBuffer::~BGFXVertexBuffer()
{
    Destroy();
}

bool BGFXVertexBuffer::Create(void* data, int vertexCount, WW3DFormat format, uint64_t flags)
{
    bgfx::VertexLayout layout = Make_Layout(format);
    return Create(data, vertexCount, layout, flags);
}

bool BGFXVertexBuffer::Create(void* data, int vertexCount, const bgfx::VertexLayout& layout, uint64_t flags)
{
    Destroy();

    if (!data || vertexCount <= 0) {
        return false;
    }

    m_layout = layout;
    m_vertexCount = vertexCount;

    const uint32_t stride = layout.getStride();
    const uint32_t size = vertexCount * stride;

    const bgfx::Memory* mem = bgfx::copy(data, size);
    m_handle = bgfx::createVertexBuffer(mem, layout, flags);

    return bgfx::isValid(m_handle);
}

void BGFXVertexBuffer::Destroy()
{
    if (bgfx::isValid(m_handle)) {
        bgfx::destroy(m_handle);
        m_handle = BGFX_INVALID_HANDLE;
    }
    m_vertexCount = 0;
    m_ownsData = false;
}

bgfx::VertexLayout BGFXVertexBuffer::Make_Layout(WW3DFormat format)
{
    bgfx::VertexLayout layout;
    layout.begin();

    // Position is always present
    layout.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float);

    // Normal
    if (format == WW3D_FORMAT_XYZNV1 || format == WW3D_FORMAT_XYZNDV1 ||
        format == WW3D_FORMAT_XYZNV2 || format == WW3D_FORMAT_XYZNDV2 ||
        format == WW3D_FORMAT_XYZNDUV1 || format == WW3D_FORMAT_XYZNDUV2) {
        layout.add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float);
    }

    // Diffuse color
    if (format == WW3D_FORMAT_XYZDUV1 || format == WW3D_FORMAT_XYZDUV2 ||
        format == WW3D_FORMAT_XYZNDUV1 || format == WW3D_FORMAT_XYZNDUV2) {
        layout.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true);
    }

    // Texture coordinates
    if (format == WW3D_FORMAT_XYZUV1 || format == WW3D_FORMAT_XYZDUV1 ||
        format == WW3D_FORMAT_XYZNDUV1 || format == WW3D_FORMAT_XYZNV1) {
        layout.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float);
    }

    if (format == WW3D_FORMAT_XYZUV2 || format == WW3D_FORMAT_XYZDUV2 ||
        format == WW3D_FORMAT_XYZNDUV2 || format == WW3D_FORMAT_XYZNV2) {
        layout.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float);
        layout.add(bgfx::Attrib::TexCoord1, 2, bgfx::AttribType::Float);
    }

    layout.end();
    return layout;
}

int BGFXVertexBuffer::Get_Stride(WW3DFormat format)
{
    bgfx::VertexLayout layout = Make_Layout(format);
    return layout.getStride();
}
