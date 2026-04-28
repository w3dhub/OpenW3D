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
 * BGFXIndexBuffer.cpp - implementation of bgfx index buffer wrapper.                      *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

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
