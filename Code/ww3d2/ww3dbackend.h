/*
**	Command & Conquer Renegade(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
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
 * WW3DBackend interface - minimal device-level abstraction for render backends.              *
 *                                                                                             *
 * Concrete backends (DX8, BGFX, null) implement this interface. DX8Wrapper stays          *
 * standalone and is NOT part of this interface chain.                                        *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef WW3DBACKEND_H
#define WW3DBACKEND_H

#include "vector3.h"

class SceneClass;
class RenderDeviceDescClass;

// Forward declarations for render state methods
class ShaderClass;
class TextureClass;
class VertexMaterialClass;
class LightEnvironmentClass;
class VertexBufferClass;
class IndexBufferClass;
class DynamicVBAccessClass;
class DynamicIBAccessClass;
class Matrix3D;
class Matrix4;

class WW3DBackend
{
public:
    virtual ~WW3DBackend() {}

    // Initialization
    virtual bool Init(void * hwnd, bool lite = false) = 0;
    virtual void Shutdown() = 0;

    // Device enumeration
    virtual bool Set_Any_Render_Device() = 0;
    virtual bool Set_Render_Device(const char * dev_name, int width = -1, int height = -1, int bits = -1, int windowed = -1, bool resize_window = false) = 0;
    virtual bool Set_Render_Device(int dev = -1, int resx = -1, int resy = -1, int bits = -1, int windowed = -1, bool resize_window = false) = 0;
    virtual bool Set_Next_Render_Device() = 0;
    virtual bool Toggle_Windowed() = 0;
    virtual bool Is_Windowed() = 0;

    virtual int Get_Render_Device_Count() = 0;
    virtual int Get_Render_Device() = 0;
    virtual const RenderDeviceDescClass & Get_Render_Device_Desc(int deviceidx) = 0;
    virtual const char * Get_Render_Device_Name(int device_index) = 0;
    virtual bool Set_Device_Resolution(int width = -1, int height = -1, int bits = -1, int windowed = -1, bool resize_window = false) = 0;
    virtual void Get_Device_Resolution(int & set_w, int & set_h, int & set_bits, bool & set_windowed) = 0;
    virtual void Get_Render_Target_Resolution(int & set_w, int & set_h, int & set_bits, bool & set_windowed) = 0;
    virtual int Get_Device_Resolution_Width() = 0;
    virtual int Get_Device_Resolution_Height() = 0;

    // Registry
    virtual bool Registry_Save_Render_Device(const char * sub_key) = 0;
    virtual bool Registry_Load_Render_Device(const char * sub_key, bool resize_window) = 0;
    virtual bool Registry_Save_Render_Device(const char *sub_key, int device, int width, int height, int depth, bool windowed, int texture_depth) = 0;
    virtual bool Registry_Load_Render_Device(const char * sub_key, char *device, int device_len, int &width, int &height, int &depth, int &windowed, int &texture_depth) = 0;

    // Scene
    virtual void Begin_Scene() = 0;
    virtual void End_Scene(bool flip_frame = true) = 0;
    virtual void Flip_To_Primary() = 0;

    // Clear
    virtual void Clear(bool clear_color, bool clear_z_stencil, const Vector3 &color, float z = 1.0f, unsigned int stencil = 0) = 0;

    // VSync
    virtual void Set_Swap_Interval(int swap) = 0;
    virtual int Get_Swap_Interval() = 0;

    // Texture depth
    virtual void Set_Texture_Bitdepth(int depth) = 0;
    virtual int Get_Texture_Bitdepth() = 0;

    // Render state
    virtual void Set_Viewport(const void* viewport) = 0;
    virtual void Set_DX8_Render_State(int state, unsigned value) = 0;
    virtual void Set_Light_Environment(const void* env) = 0;
    virtual void* _Get_DX8_Front_Buffer() = 0;

    // =========================================================================
    // Extended render state — backends that support modern APIs implement these
    // DX8Backend can leave these as no-ops since it uses the existing DX8 path
    // =========================================================================

    // Transform state
    virtual void Set_Transform_World(const Matrix4& m) { (void)m; }
    virtual void Set_Transform_View(const Matrix4& m) { (void)m; }
    virtual void Set_Transform_Projection(const Matrix4& m) { (void)m; }

    // Shader / material / texture state (deferred application)
    virtual void Apply_Shader_State(const ShaderClass& shader) { (void)shader; }
    virtual void Apply_Texture_State(unsigned stage, TextureClass* texture) { (void)stage; (void)texture; }
    virtual void Apply_Material_State(const VertexMaterialClass* material) { (void)material; }
    virtual void Apply_Light_Environment_State(LightEnvironmentClass* env) { (void)env; }

    // Buffer binding
    virtual void Set_Vertex_Buffer(const VertexBufferClass* vb) { (void)vb; }
    virtual void Set_Index_Buffer(const IndexBufferClass* ib, unsigned short index_base_offset) { (void)ib; (void)index_base_offset; }

    // Draw submission
    virtual void Draw_Indexed_Triangles(unsigned short start_index, unsigned short polygon_count, unsigned short min_vertex_index, unsigned short vertex_count) { (void)start_index; (void)polygon_count; (void)min_vertex_index; (void)vertex_count; }
    virtual void Draw_Indexed_Strip(unsigned short start_index, unsigned short index_count, unsigned short min_vertex_index, unsigned short vertex_count) { (void)start_index; (void)index_count; (void)min_vertex_index; (void)vertex_count; }

    // Finalize state application (called after all individual state pieces are set)
    virtual void Commit_Render_State() {}

    // Returns true if this backend wants DX8Wrapper to delegate Apply_Render_State_Changes
    virtual bool Wants_Deferred_State_Apply() const { return false; }
};

#endif // WW3DBACKEND_H
