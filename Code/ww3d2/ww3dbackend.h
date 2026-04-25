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
 *                 Project Name : WW3D                                                         *
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 *                                                                                             *
 *  Render backend abstraction layer.                                                           *
 *                                                                                             *
 *  This header defines the WW3DBackend interface: a pure-virtual interface that backends      *
 *  (DX8, DX9, DX12, Vulkan, Null) must implement. It is backend-agnostic — no D3D or      *
 *  Vulkan types appear in this interface.                                                    *
 *                                                                                             *
 *  Each backend is a thin implementation layer that translates between this abstract              *
 *  interface and the underlying graphics API.                                                 *
 *                                                                                             *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#ifndef WW3DBACKEND_H
#define WW3DBACKEND_H

class Vector3;
class SceneClass;

/**
 * BackendDeviceCapabilities
 *
 * A backend-agnostic description of a single GPU adapter. This struct contains only
 * plain C++ types — no D3D or Vulkan types. Backends populate it from their
 * respective GPU enumeration APIs.
 *
 * Used by: WW3DBackend::Enumerate_Devices()
 */
struct BackendDeviceCapabilities
{
    int  AdapterIndex;          // Backend-specific adapter index
    char DeviceName[256];       // Human-readable GPU name (e.g. "NVIDIA RTX 3080")
    char DriverName[256];       // Driver/runtime name
    char DriverVersion[64];     // Driver version string
    int  VendorId;              // PCI vendor ID (e.g. 0x10DE for NVIDIA, 0x8086 for Intel)
    int  DeviceId;              // PCI device ID
    int  SubSystemId;           // Subsystem ID
    int  RevisionId;            // Revision ID
    int  DedicatedVideoMemory;  // Approximate VRAM in bytes (-1 if unknown)
    int  DedicatedSystemMemory; // Approximate dedicated system RAM in bytes (-1 if unknown)
    int  SharedSystemMemory;   // Approximate shared system RAM in bytes (-1 if unknown)
    bool IsHardware;            // True if this is a physical GPU, false if software/render-only
    bool IsExternal;           // True if this is an external GPU (e.g. eGPU)
};

/**
 * BackendResolution
 *
 * A single resolution mode supported by a GPU adapter.
 */
struct BackendResolution
{
    int Width;
    int Height;
    int RefreshRateHz;   // Vertical refresh rate in Hz (0 = default/unknown)
};

/**
 * WW3DBackend
 *
 * Pure-virtual interface for the render backend. All rendering in WW3D is routed
 * through this interface. Each backend (DX8, DX9, DX12, Vulkan, Null) implements
 * this interface independently.
 *
 * Key design principles:
 * - No D3D or Vulkan types in this interface
 * - No texture/surface factory types — those belong in a separate abstraction
 * - Device enumeration uses only BackendDeviceCapabilities (plain C++ structs)
 * - Presentation and scene framing are separate from rendering API calls
 */
class WW3DBackend
{
public:
    virtual ~WW3DBackend() = default;

    //========================================================================
    // Device enumeration
    //========================================================================

    /**
     * Enumerate all render devices available through this backend.
     *
     * @param devices Output array of BackendDeviceCapabilities, one per GPU.
     *                Backends populate this from their respective GPU enumeration API.
     * @return Number of devices found (size of devices array).
     *
     * Backend implementations:
     * - DX8/DX9: call Direct3D()->GetAdapterIdentifier() for each adapter
     * - DX12: call D3D12CreateDevice() enumerate adapters via DXGI
     * - Vulkan: call vkEnumeratePhysicalDevices()
     * - Null: return 1 with a single "Null Device" entry
     */
    virtual int Enumerate_Devices(BackendDeviceCapabilities* devices, int max_devices) = 0;

    //========================================================================
    // Initialization
    //========================================================================

    /**
     * Initialize the backend for the given window.
     *
     * @param hwnd Platform window handle (HWND on Windows, xcb_window_t* on Linux/X11, etc.)
     * @param lite If true, initialize with minimal features (for tools/editors)
     * @return true if initialization succeeded
     */
    virtual bool Init(void* hwnd, bool lite = false) = 0;

    /** Release all backend resources. Called on shutdown. */
    virtual void Shutdown() = 0;

    //========================================================================
    // Device selection
    //========================================================================

    /** Use the first available hardware device. */
    virtual bool Set_Any_Render_Device() = 0;

    /**
     * Select a render device by name.
     *
     * @param dev_name Device name string (matched against BackendDeviceCapabilities::DeviceName)
     * @return true if a matching device was found and selected
     */
    virtual bool Set_Render_Device(const char* dev_name) = 0;

    /**
     * Select a render device by adapter index.
     *
     * @param adapter_index Adapter index returned by Enumerate_Devices()
     * @return true if the index is valid
     */
    virtual bool Set_Render_Device(int adapter_index) = 0;

    /** Advance to the next available render device. Wraps from last to first. */
    virtual bool Set_Next_Render_Device() = 0;

    /** Toggle between windowed and fullscreen mode. */
    virtual bool Toggle_Windowed() = 0;

    /** @return true if currently in windowed mode */
    virtual bool Is_Windowed() = 0;

    //========================================================================
    // Device properties
    //========================================================================

    /** @return Number of available render devices */
    virtual int Get_Render_Device_Count() = 0;

    /** @return Index of the currently selected render device */
    virtual int Get_Render_Device() = 0;

    /**
     * Get full capabilities for a specific device.
     *
     * @param adapter_index Device index (0 to Get_Render_Device_Count()-1)
     * @return BackendDeviceCapabilities for that device, or a default-constructed struct if invalid
     */
    virtual const BackendDeviceCapabilities& Get_Render_Device_Desc(int adapter_index) = 0;

    /** @return Device name string, or "Unknown" if invalid */
    virtual const char* Get_Render_Device_Name(int adapter_index) = 0;

    //========================================================================
    // Resolution
    //========================================================================

    /**
     * Set the display resolution.
     *
     * @param width  Horizontal resolution in pixels (-1 = keep current)
     * @param height Vertical resolution in pixels (-1 = keep current)
     * @param bits   Color depth in bits per pixel (-1 = keep current)
     * @param windowed Windowed (1) or fullscreen (0); -1 = keep current
     * @param resize_window If true and in windowed mode, resize the window to match
     * @return true if the resolution was successfully set
     */
    virtual bool Set_Device_Resolution(int width = -1, int height = -1, int bits = -1, int windowed = -1, bool resize_window = false) = 0;

    /** @return Current device resolution (runtime-queried, may differ from what was requested) */
    virtual void Get_Device_Resolution(int& width, int& height, int& bits, bool& windowed) = 0;

    /** @return Resolution of the actual render target (may be a different surface than the display) */
    virtual void Get_Render_Target_Resolution(int& width, int& height, int& bits, bool& windowed) = 0;

    /** @return Current display width in pixels */
    virtual int Get_Device_Resolution_Width() = 0;

    /** @return Current display height in pixels */
    virtual int Get_Device_Resolution_Height() = 0;

    //========================================================================
    // Registry / persistence
    //========================================================================

    /** Save current render device to registry (platform-specific config store). */
    virtual bool Registry_Save_Render_Device(const char* sub_key) = 0;

    /** Load render device from registry. @param resize_window If true, apply saved resolution too. */
    virtual bool Registry_Load_Render_Device(const char* sub_key, bool resize_window) = 0;

    //========================================================================
    // Scene framing
    //========================================================================

    /** Begin a new rendered frame. Allocates command buffer(s) for this frame. */
    virtual void Begin_Scene() = 0;

    /**
     * End the current frame and queue it for presentation.
     *
     * @param flip_frame If true, present/swap the front and back buffers
     */
    virtual void End_Scene(bool flip_frame = true) = 0;

    /** Force the back buffer to the primary display immediately (no v-sync). */
    virtual void Flip_To_Primary() = 0;

    //========================================================================
    // Framebuffer clear
    //========================================================================

    /**
     * Clear the render target and/or depth-stencil buffer.
     *
     * @param clear_color  If true, fill the render target with the given color
     * @param clear_z_stencil If true, clear the depth and/or stencil buffer
     * @param color RGB color to fill the render target with
     * @param z Depth value to fill the depth buffer with (0.0 to 1.0)
     * @param stencil Stencil value to fill the stencil buffer with
     */
    virtual void Clear(bool clear_color, bool clear_z_stencil, const Vector3& color, float z = 1.0f, unsigned int stencil = 0) = 0;

    //========================================================================
    // VSync
    //========================================================================

    /** Set the vertical sync interval. 0 = immediate/no vsync, 1 = one refresh interval, etc. */
    virtual void Set_Swap_Interval(int swap_interval) = 0;

    /** @return Current vsync interval (0 = off, 1 = on, etc.) */
    virtual int Get_Swap_Interval() = 0;

    //========================================================================
    // Texture depth
    //========================================================================

    /**
     * Set the preferred texture color depth.
     *
     * @param depth Color depth in bits per pixel (16, 24, or 32)
     */
    virtual void Set_Texture_Bitdepth(int depth) = 0;

    /** @return Current texture bit depth setting */
    virtual int Get_Texture_Bitdepth() = 0;

protected:
    // Protected constructor — must be constructed via WW3D::Set_Backend()
    WW3DBackend() = default;
};

#endif // WW3DBACKEND_H
