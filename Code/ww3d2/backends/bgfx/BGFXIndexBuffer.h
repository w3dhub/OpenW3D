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
 * BGFXIndexBuffer - WW3D index buffer implementation using bgfx.                           *
 *                                                                                             *
 * Supports 16-bit and 32-bit indices.                                                       *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

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
