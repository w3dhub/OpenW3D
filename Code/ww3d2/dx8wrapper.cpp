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
 *                     $Archive:: /Commando/Code/ww3d2/dx8wrapper.cpp                         $*
 *                                                                                             *
 *              Original Author:: Jani Penttinen                                               *
 *                                                                                             *
 *                      $Author:: Jani_p                                                      $*
 *                                                                                             *
 *                     $Modtime:: 3/12/02 4:27p                                               $*
 *                                                                                             *
 *                    $Revision:: 170                                                         $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   DX8Wrapper::_Update_Texture -- Copies a texture from system memory to video memory        *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

//#define CREATE_DX8_MULTI_THREADED
//#define CREATE_DX8_FPU_PRESERVE
#define WW3D_DEVTYPE D3DDEVTYPE_HAL

#include "dx8wrapper.h"
#include "dx8fvf.h"
#include "dx8vertexbuffer.h"
#include "dx8indexbuffer.h"
#include "dx8renderer.h"
#include "ww3d.h"
#include "camera.h"
#include "wwstring.h"
#include "matrix4.h"
#include "vertmaterial.h"
#include "rddesc.h"
#include "lightenvironment.h"
#include "statistics.h"
#include "registry.h"
#include "boxrobj.h"
#include "pointgr.h"
#include "render2d.h"
#include "sortingrenderer.h"
#include "shattersystem.h"
#include "light.h"
#include "assetmgr.h"
#include "textureloader.h"
#include "missingtexture.h"
#include "thread.h"
#include <stdio.h>
#include <d3dx9core.h>
#include "dxerr_compat.h"
#include "pot.h"
#include "wwprofile.h"
#include "ffactory.h"
#include "dx8caps.h"
#include "formconv.h"
#include "dx8texman.h"
#include "bound.h"
#include "ini.h"
#include "openw3d.h"

const int DEFAULT_RESOLUTION_WIDTH = 800;
const int DEFAULT_RESOLUTION_HEIGHT = 600;
const int DEFAULT_BIT_DEPTH = 32;
const int DEFAULT_TEXTURE_BIT_DEPTH = 16;


/***********************************************************************************
**
** DX8Wrapper Static Variables
**
***********************************************************************************/

static HWND						_Hwnd															= NULL;
bool								DX8Wrapper::IsInitted									= false;
bool								DX8Wrapper::_EnableTriangleDraw						= true;

int								DX8Wrapper::CurRenderDevice							= -1;
int								DX8Wrapper::ResolutionWidth							= DEFAULT_RESOLUTION_WIDTH;
int								DX8Wrapper::ResolutionHeight							= DEFAULT_RESOLUTION_HEIGHT;
int								DX8Wrapper::BitDepth										= DEFAULT_BIT_DEPTH;
int								DX8Wrapper::TextureBitDepth							= DEFAULT_TEXTURE_BIT_DEPTH;
bool								DX8Wrapper::IsWindowed									= false;

D3DMATRIX						DX8Wrapper::old_world;
D3DMATRIX						DX8Wrapper::old_view;
D3DMATRIX						DX8Wrapper::old_prj;

bool								DX8Wrapper::world_identity;
unsigned							DX8Wrapper::RenderStates[256];
unsigned							DX8Wrapper::TextureStageStates[MAX_TEXTURE_STAGES][32];
unsigned							DX8Wrapper::TextureSamplerStates[MAX_TEXTURE_STAGES][14];
IDirect3DBaseTexture9 *		DX8Wrapper::Textures[MAX_TEXTURE_STAGES];
RenderStateStruct				DX8Wrapper::render_state;
unsigned							DX8Wrapper::render_state_changed;

bool								DX8Wrapper::FogEnable									= false;
D3DCOLOR							DX8Wrapper::FogColor										= 0;

IDirect3D9 *					DX8Wrapper::D3DInterface								= NULL;
IDirect3DDevice9 *			DX8Wrapper::D3DDevice									= NULL;
IDirect3DSurface9 *			DX8Wrapper::CurrentRenderTarget						= NULL;
IDirect3DSurface9 *			DX8Wrapper::DefaultRenderTarget						= NULL;
IDirect3DSurface9 *			DX8Wrapper::DefaultDepthBuffer						= NULL;
bool								DX8Wrapper::IsRenderToTexture							= false;

unsigned							DX8Wrapper::matrix_changes								= 0;
unsigned							DX8Wrapper::material_changes							= 0;
unsigned							DX8Wrapper::vertex_buffer_changes					= 0;
unsigned							DX8Wrapper::index_buffer_changes                = 0;
unsigned							DX8Wrapper::light_changes								= 0;
unsigned							DX8Wrapper::texture_changes							= 0;
unsigned							DX8Wrapper::render_state_changes						= 0;
unsigned							DX8Wrapper::texture_stage_state_changes			= 0;
unsigned							DX8Wrapper::sampler_state_changes			= 0;
unsigned							DX8Wrapper::_MainThreadID								= 0;
bool								DX8Wrapper::CurrentDX8LightEnables[4];
bool								DX8Wrapper::IsDeviceLost;
int								DX8Wrapper::ZBias;
float								DX8Wrapper::ZNear;
float								DX8Wrapper::ZFar;
Matrix4							DX8Wrapper::ProjectionMatrix;
int							DX8Wrapper::BaseVertexIndex;

DX8Caps*							DX8Wrapper::CurrentCaps;

D3DADAPTER_IDENTIFIER9		DX8Wrapper::CurrentAdapterIdentifier;

unsigned int DX8Wrapper::FrameCount = 0;

bool								_DX8SingleThreaded										= false;

unsigned							number_of_DX8_calls										= 0;
static unsigned				last_frame_matrix_changes								= 0;
static unsigned				last_frame_material_changes							= 0;
static unsigned				last_frame_vertex_buffer_changes						= 0;
static unsigned				last_frame_index_buffer_changes						= 0;
static unsigned				last_frame_light_changes								= 0;
static unsigned				last_frame_texture_changes								= 0;
static unsigned				last_frame_render_state_changes						= 0;
static unsigned				last_frame_texture_stage_state_changes				= 0;
static unsigned				last_frame_number_of_DX8_calls						= 0;

static D3DDISPLAYMODE DesktopMode;

static D3DPRESENT_PARAMETERS								_PresentParameters;
static DynamicVectorClass<StringClass>					_RenderDeviceNameTable;
static DynamicVectorClass<StringClass>					_RenderDeviceShortNameTable;
static DynamicVectorClass<RenderDeviceDescClass>	_RenderDeviceDescriptionTable;


typedef IDirect3D9* (WINAPI *Direct3DCreate8Type) (UINT SDKVersion);
Direct3DCreate8Type	Direct3DCreate8Ptr = NULL;
HINSTANCE D3D8Lib = NULL;

/***********************************************************************************
**
** DX8Wrapper Implementation
**
***********************************************************************************/

void Log_DX8_ErrorCode(HRESULT res)
{
	const char *error_string = DXGetErrorStringA(res);

	if (error_string != nullptr) {
		WWDEBUG_SAY((error_string));
	}

	WWASSERT(0);
}

void Non_Fatal_Log_DX8_ErrorCode(HRESULT res,const char * file,int line)
{
	const char *error_string = DXGetErrorStringA(res);

	if (error_string != nullptr) {
		WWDEBUG_SAY(("DX8 Error: %s, File: %s, Line: %d\r\n",error_string,file,line));
	}
}



bool DX8Wrapper::Init(void * hwnd, bool lite)
{
	WWASSERT(!IsInitted);

	/*
	** Initialize all variables!
	*/
	_Hwnd = (HWND)hwnd;
	_MainThreadID=ThreadClass::_Get_Current_Thread_ID();
	WWDEBUG_SAY(("DX8Wrapper main thread: 0x%x\n",_MainThreadID));
	CurRenderDevice = -1;
	ResolutionWidth = DEFAULT_RESOLUTION_WIDTH;
	ResolutionHeight = DEFAULT_RESOLUTION_HEIGHT;
	// Initialize Render2DClass Screen Resolution
	Render2DClass::Set_Screen_Resolution( RectClass( 0, 0, ResolutionWidth, ResolutionHeight ) );
	BitDepth = DEFAULT_BIT_DEPTH;
	IsWindowed = false;

	for (int light=0;light<4;++light) CurrentDX8LightEnables[light]=false;

	::ZeroMemory(&old_world, sizeof(D3DMATRIX));
	::ZeroMemory(&old_view, sizeof(D3DMATRIX));
	::ZeroMemory(&old_prj, sizeof(D3DMATRIX));

	//old_vertex_shader; TODO
	//old_sr_shader;
	//current_shader;

	//world_identity;
	//CurrentFogColor;

	D3DInterface = NULL;
	D3DDevice = NULL;

	WWDEBUG_SAY(("Reset DX8Wrapper statistics\n"));
	Reset_Statistics();

	Invalidate_Cached_Render_States();

	if (!lite) {
		D3D8Lib = LoadLibraryA("D3D9.DLL");

		if (D3D8Lib == NULL) return false;

		Direct3DCreate8Ptr = (Direct3DCreate8Type) GetProcAddress(D3D8Lib, "Direct3DCreate9");
		if (Direct3DCreate8Ptr) {

			/*
			** Create the D3D interface object
			*/
			WWDEBUG_SAY(("Create Direct3D8\n"));
			D3DInterface = Direct3DCreate8Ptr(D3D_SDK_VERSION);		// TODO: handle failure cases...
			if (D3DInterface == NULL) {
				return(false);
			}
			IsInitted = true;

			/*
			** Enumerate the available devices
			*/
			WWDEBUG_SAY(("Enumerate devices\n"));
			Enumerate_Devices();
			WWDEBUG_SAY(("DX8Wrapper Init completed\n"));
		}
	}
	return(true);
}

void DX8Wrapper::Shutdown(void)
{
	if (D3DDevice) {
		Set_Render_Target ((IDirect3DSurface9 *)NULL);

		// If in full screen, reset device to windowed mode before releasing it. This is an attempt to
		// fix some random bugs and crashes on some devices.
#if (0)
		if (!IsWindowed) {
			IsWindowed=true;
			ResolutionWidth=DEFAULT_RESOLUTION_WIDTH;
			ResolutionHeight=DEFAULT_RESOLUTION_HEIGHT;
			_PresentParameters.BackBufferWidth = ResolutionWidth;
			_PresentParameters.BackBufferHeight = ResolutionHeight;
			_PresentParameters.BackBufferCount = 1;
			_PresentParameters.MultiSampleType = D3DMULTISAMPLE_NONE;
			_PresentParameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
			_PresentParameters.Windowed = IsWindowed;
			_PresentParameters.EnableAutoDepthStencil = false;
			_PresentParameters.Flags=0;
			_PresentParameters.BackBufferFormat = DesktopMode.Format;
			Reset_Device();
		}
#endif //(0)
		Release_Device();
	}

	if (D3DInterface) {
		D3DInterface->Release();
		D3DInterface=NULL;

	}
	for (int i = 0; i < MAX_TEXTURE_STAGES; i++) {
		if (Textures[i]) {
			Textures[i]->Release();
			Textures[i] = NULL;
		}
	}

	_RenderDeviceNameTable.Delete_All();
	_RenderDeviceShortNameTable.Delete_All();
	_RenderDeviceDescriptionTable.Delete_All();

	if (D3D8Lib) {
		FreeLibrary(D3D8Lib);
		D3D8Lib = NULL;
	}

	IsInitted = false;
}

void DX8Wrapper::Do_Onetime_Device_Dependent_Inits(void)
{
	/*
	** Set Global render states (some of which depend on caps)
	*/
	Compute_Caps(D3DFormat_To_WW3DFormat(_PresentParameters.BackBufferFormat));

   /*
	** Initalize any other subsystems inside of WW3D
	*/
	MissingTexture::_Init();
	TextureClass::_Init_Filters((TextureClass::TextureFilterMode)WW3D::Get_Texture_Filter());
	TheDX8MeshRenderer.Init();
	BoxRenderObjClass::Init();
	VertexMaterialClass::Init();
	PointGroupClass::_Init(); // This needs the VertexMaterialClass to be initted
	ShatterSystem::Init();
	TextureLoader::Init();

	Set_Default_Global_Render_States();
}

inline DWORD F2DW(float f) { return *((unsigned*)&f); }
void DX8Wrapper::Set_Default_Global_Render_States(void)
{
	DX8_THREAD_ASSERT();
	const D3DCAPS9 &caps = Get_Current_Caps()->Get_DX8_Caps();

	Set_DX8_Render_State(D3DRS_RANGEFOGENABLE, (caps.RasterCaps & D3DPRASTERCAPS_FOGRANGE) ? true : false);
	Set_DX8_Render_State(D3DRS_FOGTABLEMODE, D3DFOG_NONE);
	Set_DX8_Render_State(D3DRS_FOGVERTEXMODE, D3DFOG_LINEAR);
	Set_DX8_Render_State(D3DRS_SPECULARMATERIALSOURCE, D3DMCS_MATERIAL);
	Set_DX8_Render_State(D3DRS_COLORVERTEX, true);
	Set_DX8_Render_State(D3DRS_DEPTHBIAS,0);
	Set_DX8_Render_State(D3DRS_SLOPESCALEDEPTHBIAS,0);
	Set_DX8_Texture_Stage_State(1, D3DTSS_BUMPENVLSCALE, F2DW(1.0f));
	Set_DX8_Texture_Stage_State(1, D3DTSS_BUMPENVLOFFSET, F2DW(0.0f));
	Set_DX8_Texture_Stage_State(0, D3DTSS_BUMPENVMAT00,F2DW(1.0f));
	Set_DX8_Texture_Stage_State(0, D3DTSS_BUMPENVMAT01,F2DW(0.0f));
	Set_DX8_Texture_Stage_State(0, D3DTSS_BUMPENVMAT10,F2DW(0.0f));
	Set_DX8_Texture_Stage_State(0, D3DTSS_BUMPENVMAT11,F2DW(1.0f));

//	Set_DX8_Render_State(D3DRS_CULLMODE, D3DCULL_CW);
	// Set dither mode here?
}

void DX8Wrapper::Invalidate_Cached_Render_States(void)
{
	int a;
	for (a=0;a<sizeof(RenderStates)/sizeof(unsigned);++a) {
		RenderStates[a]=0x12345678;
	}
	for (a=0;a<MAX_TEXTURE_STAGES;++a) {
		for (int b=0; b<32;b++) {
			TextureStageStates[a][b]=0x12345678;
		}
        for (int b=0; b<14;b++) {
            TextureSamplerStates[a][b]=0x12345678;
        }
		Textures[a]=NULL;
	}
	ShaderClass::Invalidate();
}

void DX8Wrapper::Do_Onetime_Device_Dependent_Shutdowns(void)
{
	/*
	** Shutdown ww3d systems
	*/
	if (render_state.vertex_buffer) render_state.vertex_buffer->Release_Engine_Ref();
	REF_PTR_RELEASE(render_state.vertex_buffer);
	if (render_state.index_buffer) render_state.index_buffer->Release_Engine_Ref();
	REF_PTR_RELEASE(render_state.index_buffer);
	REF_PTR_RELEASE(render_state.material);
	for (unsigned i=0;i<MAX_TEXTURE_STAGES;++i) REF_PTR_RELEASE(render_state.Textures[i]);

	TextureLoader::Deinit();
	SortingRendererClass::Deinit();
	DynamicVBAccessClass::_Deinit();
	DynamicIBAccessClass::_Deinit();
	ShatterSystem::Shutdown();
	PointGroupClass::_Shutdown();
	VertexMaterialClass::Shutdown();
	BoxRenderObjClass::Shutdown();
	TheDX8MeshRenderer.Shutdown();
	MissingTexture::_Deinit();

	if (CurrentCaps) {
		delete CurrentCaps;
		CurrentCaps=NULL;
	}
}


bool DX8Wrapper::Create_Device(void)
{
	WWASSERT(D3DDevice == NULL);	// for now, once you've created a device, you're stuck with it!

	D3DCAPS9 caps;
	if (FAILED( D3DInterface->GetDeviceCaps(
		CurRenderDevice,
		WW3D_DEVTYPE,
		&caps))) {
		return false;
	}

	::ZeroMemory(&CurrentAdapterIdentifier, sizeof(D3DADAPTER_IDENTIFIER9));
	if (FAILED( D3DInterface->GetAdapterIdentifier(CurRenderDevice,0/*D3DENUM_WHQL_LEVEL*/,&CurrentAdapterIdentifier))) {
		return false;
	}

	unsigned vertex_processing_type=D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	if (caps.DevCaps&D3DDEVCAPS_HWTRANSFORMANDLIGHT) {
		vertex_processing_type=D3DCREATE_MIXED_VERTEXPROCESSING;
	}

#ifdef CREATE_DX8_MULTI_THREADED
	vertex_processing_type|=D3DCREATE_MULTITHREADED;
	_DX8SingleThreaded=false;
#else
	_DX8SingleThreaded=true;
#endif

#ifdef CREATE_DX8_FPU_PRESERVE
	vertex_processing_type|=D3DCREATE_FPU_PRESERVE;
#endif

	// JANI HACK! Some objects flicker on ATI Radeons. This can be fixed by locking the back buffer before flipping.
	// For this to work the back buffers need to be created as lockable!
	DX8Caps dx8_caps(
		D3DInterface,
		caps,
		D3DFormat_To_WW3DFormat(_PresentParameters.BackBufferFormat),
		CurrentAdapterIdentifier);
	if (dx8_caps.Get_Vendor()==DX8Caps::VENDOR_ATI) {
		_PresentParameters.Flags=D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
	}

	HRESULT hr = D3DInterface->CreateDevice(
		CurRenderDevice,
		WW3D_DEVTYPE,
		_Hwnd,
		vertex_processing_type,
		&_PresentParameters,
		&D3DDevice );

	if (FAILED(hr)) {
		// The device selection may fail because the device lied that it supports 32 bit zbuffer with 16 bit
		// display. This happens at least on Voodoo2.

		if ((_PresentParameters.BackBufferFormat==D3DFMT_R5G6B5 ||
			_PresentParameters.BackBufferFormat==D3DFMT_X1R5G5B5 ||
			_PresentParameters.BackBufferFormat==D3DFMT_A1R5G5B5) &&
			(_PresentParameters.AutoDepthStencilFormat==D3DFMT_D32 ||
			_PresentParameters.AutoDepthStencilFormat==D3DFMT_D24S8 ||
			_PresentParameters.AutoDepthStencilFormat==D3DFMT_D24X8)) {

				_PresentParameters.AutoDepthStencilFormat=D3DFMT_D16;
				hr = D3DInterface->CreateDevice(
					CurRenderDevice,
					WW3D_DEVTYPE,
					_Hwnd,
					vertex_processing_type,
					&_PresentParameters,
					&D3DDevice );
				if (FAILED(hr)) {
					return false;
				}
			}
		}

	/*
	** Initialize all subsystems
	*/
	Do_Onetime_Device_Dependent_Inits();
	return true;
}

bool DX8Wrapper::Reset_Device(void)
{
	WWDEBUG_SAY(("Resetting device.\n"));
	DX8_THREAD_ASSERT();
	if ((IsInitted) && (D3DDevice != NULL)) {
		// Release all non-MANAGED stuff
		WW3D::_Invalidate_Textures();

		Set_Vertex_Buffer (NULL);
		Set_Index_Buffer (NULL, 0);
		DynamicVBAccessClass::_Deinit();
		DynamicIBAccessClass::_Deinit();
		DX8TextureManagerClass::Release_Textures();

		// Reset frame count to reflect the flipping chain being reset by Reset()
		FrameCount = 0;

		DX8CALL(Reset(&_PresentParameters));
		DX8TextureManagerClass::Recreate_Textures();
		Invalidate_Cached_Render_States();
		Set_Default_Global_Render_States();
		WWDEBUG_SAY(("Device reset completed\n"));
		return true;
	}
	WWDEBUG_SAY(("Device reset failed\n"));
	return false;
}

void DX8Wrapper::Release_Device(void)
{
	if (D3DDevice) {
		/*
		** Release the current vertex and index buffers
		*/
		if (render_state.vertex_buffer) render_state.vertex_buffer->Release_Engine_Ref();
		REF_PTR_RELEASE(render_state.vertex_buffer);
		if (render_state.index_buffer) render_state.index_buffer->Release_Engine_Ref();
		REF_PTR_RELEASE(render_state.index_buffer);

		/*
		** Shutdown all subsystems
		*/
		Do_Onetime_Device_Dependent_Shutdowns();

		/*
		** Release the device
		*/
		D3DDevice->Release();
		D3DDevice=NULL;
	}
}

void DX8Wrapper::Enumerate_Devices()
{
	DX8_Assert();

	int adapter_count = D3DInterface->GetAdapterCount();
	for (int adapter_index=0; adapter_index<adapter_count; adapter_index++) {

		D3DADAPTER_IDENTIFIER9 id;
		::ZeroMemory(&id, sizeof(D3DADAPTER_IDENTIFIER9));
		HRESULT res = D3DInterface->GetAdapterIdentifier(adapter_index,0/*D3DENUM_WHQL_LEVEL*/,&id);

		if (res == D3D_OK) {

			/*
			** Set up the render device description
			** TODO: Fill in more fields of the render device description?  (need some lookup tables)
			*/
			RenderDeviceDescClass desc;
			desc.set_device_name(id.Description);
			desc.set_driver_name(id.Driver);

			char buf[64];
			sprintf(buf,"%d.%d.%d.%d", //"%04x.%04x.%04x.%04x",
				HIWORD(id.DriverVersion.HighPart),
				LOWORD(id.DriverVersion.HighPart),
				HIWORD(id.DriverVersion.LowPart),
				LOWORD(id.DriverVersion.LowPart));

			desc.set_driver_version(buf);

			D3DInterface->GetDeviceCaps(adapter_index,WW3D_DEVTYPE,&desc.Caps);
			D3DInterface->GetAdapterIdentifier(adapter_index,0/*D3DENUM_WHQL_LEVEL*/,&desc.AdapterIdentifier);

			DX8Caps dx8caps(D3DInterface,desc.Caps,WW3D_FORMAT_UNKNOWN,desc.AdapterIdentifier);

			/*
			** Enumerate the resolutions
			*/
			desc.reset_resolution_list();
			int mode_count = D3DInterface->GetAdapterModeCount(adapter_index,D3DFMT_X8R8G8B8);
			for (int mode_index=0; mode_index<mode_count; mode_index++) {
				D3DDISPLAYMODE d3dmode;
				::ZeroMemory(&d3dmode, sizeof(D3DDISPLAYMODE));
				HRESULT res = D3DInterface->EnumAdapterModes(adapter_index,D3DFMT_X8R8G8B8,mode_index,&d3dmode);

				if (res == D3D_OK) {
					int bits = 0;
					switch (d3dmode.Format)
					{
						case D3DFMT_R8G8B8:
						case D3DFMT_A8R8G8B8:
						case D3DFMT_X8R8G8B8:		bits = 32; break;

						case D3DFMT_R5G6B5:
						case D3DFMT_X1R5G5B5:		bits = 16; break;
					}

					// Some cards fail in certain modes, DX8Caps keeps list of those.
					if (!dx8caps.Is_Valid_Display_Format(d3dmode.Width,d3dmode.Height,D3DFormat_To_WW3DFormat(d3dmode.Format))) {
						bits=0;
					}

					/*
					** If we recognize the format, add it to the list
					** TODO: should we handle more formats?  will any cards report more than 24 or 16 bit?
					*/
					if (bits != 0) {
						desc.add_resolution(d3dmode.Width,d3dmode.Height,bits);
					}
				}
			}

			// IML: If the device has one or more valid resolutions add it to the device list.
			// NOTE: Testing has shown that there are drivers with zero resolutions.
			if (desc.Enumerate_Resolutions().Count() > 0) {

				/*
				** Set up the device name
				*/
				StringClass device_name(id.Description,true);
				_RenderDeviceNameTable.Add(device_name);
				_RenderDeviceShortNameTable.Add(device_name);	// for now, just add the same name to the "pretty name table"

				/*
				** Add the render device to our table
				*/
				_RenderDeviceDescriptionTable.Add(desc);
			}
		}
	}
}

bool DX8Wrapper::Set_Any_Render_Device(void)
{
	// Then fullscreen
	int dev_number;
	for (dev_number = 0; dev_number < _RenderDeviceNameTable.Count(); dev_number++) {
		if (Set_Render_Device(dev_number,-1,-1,-1,0,false)) {
			return true;
		}
	}

	// Try windowed first
	for (dev_number = 0; dev_number < _RenderDeviceNameTable.Count(); dev_number++) {
		if (Set_Render_Device(dev_number,-1,-1,-1,1,false)) {
			return true;
		}
	}

	return false;
}

bool DX8Wrapper::Set_Render_Device
(
	const char * dev_name,
	int width,
	int height,
	int bits,
	int windowed,
	bool resize_window
)
{
	for ( int dev_number = 0; dev_number < _RenderDeviceNameTable.Count(); dev_number++) {
		if ( strcmp( dev_name, _RenderDeviceNameTable[dev_number]) == 0) {
			return Set_Render_Device( dev_number, width, height, bits, windowed, resize_window );
		}

		if ( strcmp( dev_name, _RenderDeviceShortNameTable[dev_number]) == 0) {
			return Set_Render_Device( dev_number, width, height, bits, windowed, resize_window );
		}
	}
	return false;
}

bool DX8Wrapper::Set_Render_Device(int dev, int width, int height, int bits, int windowed, bool resize_window)
{
	WWASSERT(IsInitted);
	WWASSERT(dev >= -1);
	WWASSERT(dev < _RenderDeviceNameTable.Count());

	/*
	** If user has never selected a render device, start out with device 0
	*/
	if ((CurRenderDevice == -1) && (dev == -1)) {
		CurRenderDevice = 0;
	} else if (dev != -1) {
		CurRenderDevice = dev;
	}

	/*
	** If user doesn't want to change res, set the res variables to match the
	** current resolution
	*/
	if (width != -1)		ResolutionWidth = width;
	if (height != -1)		ResolutionHeight = height;

	// Initialize Render2DClass Screen Resolution
	Render2DClass::Set_Screen_Resolution( RectClass( 0, 0, ResolutionWidth, ResolutionHeight ) );

	if (bits != -1)		BitDepth = bits;
	if (windowed != -1)	IsWindowed = (windowed != 0);

	WWDEBUG_SAY(("Attempting Set_Render_Device: name: %s, width: %d, height: %d, windowed: %d\r\n",
		_RenderDeviceNameTable[CurRenderDevice],ResolutionWidth,ResolutionHeight,(IsWindowed ? 1 : 0)));

	WWASSERT(D3DDevice == NULL);

	/*
	** Initialize values for D3DPRESENT_PARAMETERS members.
	*/
	::ZeroMemory(&_PresentParameters, sizeof(D3DPRESENT_PARAMETERS));

	_PresentParameters.BackBufferWidth = ResolutionWidth;
	_PresentParameters.BackBufferHeight = ResolutionHeight;
	_PresentParameters.BackBufferCount = IsWindowed ? 1 : 2;

	_PresentParameters.MultiSampleType = D3DMULTISAMPLE_NONE;
	_PresentParameters.SwapEffect = IsWindowed ? D3DSWAPEFFECT_DISCARD : D3DSWAPEFFECT_FLIP;		// Shouldn't this be D3DSWAPEFFECT_FLIP?
	_PresentParameters.hDeviceWindow = _Hwnd;
	_PresentParameters.Windowed = IsWindowed;

	_PresentParameters.EnableAutoDepthStencil = true;				// Driver will attempt to match Z-buffer depth
	_PresentParameters.Flags=0;											// We're not going to lock the backbuffer

	_PresentParameters.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	_PresentParameters.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;

	/*
	** Set up the buffer formats.  Several issues here:
	** - if in windowed mode, the backbuffer must use the current display format.
	** - the depth buffer must use
	*/

	::ZeroMemory(&DesktopMode, sizeof(D3DDISPLAYMODE));
	D3DInterface->GetAdapterDisplayMode( CurRenderDevice, &DesktopMode );
	_PresentParameters.BackBufferFormat = DesktopMode.Format;

	if (IsWindowed) {
		WWDEBUG_SAY(("Initializing windowed mode\r\n"));

// 10/23/01 - Denzil - DX window initialization
		/*
		** Enforce a required set of window styles and size if the main window
		** IS NOT A CHILD WINDOW.  :)
		*/
		if ((::GetWindowLong(_Hwnd, GWL_STYLE) & WS_CHILD) == 0) {
			::SetWindowLong(_Hwnd, GWL_STYLE, WS_SYSMENU|WS_CAPTION|WS_MINIMIZEBOX|WS_CLIPCHILDREN);

			// Always resize the window to the desired resolution in windowed mode.
			resize_window = true;
		}
// End Denzil - DX window initialzaion

		// In windowed mode, define the bitdepth from desktop mode (as it can't be changed)
		switch (_PresentParameters.BackBufferFormat) {
		case D3DFMT_X8R8G8B8:
		case D3DFMT_A8R8G8B8:
		case D3DFMT_R8G8B8: BitDepth=32; break;
		case D3DFMT_A4R4G4B4:
		case D3DFMT_A1R5G5B5:
		case D3DFMT_R5G6B5: BitDepth=16; break;
		case D3DFMT_L8:
		case D3DFMT_A8:
		case D3DFMT_P8: BitDepth=8; break;
		default:
			// Unknown backbuffer format probably means the device can't do windowed
			return false;
		}

		WWDEBUG_SAY(("Using buffer format: %d\r\n",_PresentParameters.BackBufferFormat));

		/*
		** Find an appropriate Z buffer
		*/
		if (!Find_Z_Mode(_PresentParameters.BackBufferFormat,_PresentParameters.BackBufferFormat,&_PresentParameters.AutoDepthStencilFormat))
		{
			// If opening 32 bit mode failed, try 16 bit, even if the desktop happens to be 32 bit
			if (BitDepth==32) {
				WWDEBUG_SAY(("Failed to find a 32 bit mode, trying 16 bit\r\n"));
				BitDepth=16;
				_PresentParameters.BackBufferFormat=D3DFMT_R5G6B5;
				if (!Find_Z_Mode(_PresentParameters.BackBufferFormat,_PresentParameters.BackBufferFormat,&_PresentParameters.AutoDepthStencilFormat)) {
					_PresentParameters.AutoDepthStencilFormat=D3DFMT_UNKNOWN;
				}
			}
			else {
				_PresentParameters.AutoDepthStencilFormat=D3DFMT_UNKNOWN;
			}
		}

	} else {

// 10/23/01 - Denzil - DX Window initialization
		// For fullscreen set the window style to WS_POPUP (Recommended in DX docs)
		SetWindowLong(_Hwnd, GWL_STYLE, WS_POPUP);

		// Set fullscreen window position to top left and resize to cover entire display.
		// Recommended in DX docs to prevent other windows on the desktop from attempting
		// to repaint. This also prevents the OS from spending time calculating invalid
		// rects for windows that will never been seen.
		SetWindowPos(_Hwnd, HWND_TOPMOST, 0, 0,
			GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
			SWP_SHOWWINDOW|SWP_NOCOPYBITS);

		// We already resized the window
		resize_window = false;
// End Denzil - DX window initialization

		WWDEBUG_SAY(("Initializing full-screen mode\r\n"));

		/*
		** Try to find a mode that matches the user's desired bit-depth.
		*/
		Find_Color_And_Z_Mode(ResolutionWidth,ResolutionHeight,BitDepth,
			&_PresentParameters.BackBufferFormat,&_PresentParameters.AutoDepthStencilFormat);

	}

#ifdef _WIN32
	// PWG 4/13/2000 - changed so that if you say to resize the window it resizes
	// regardless of whether its windowed or not as OpenGL resizes its self around
	// the caption and edges of the window type you provide, so its important to
	// push the client area to be the size you really want.
	// if ( resize_window && windowed ) {
	if (resize_window) {

		// Get the current dimensions of the 'render area' of the window
		RECT rect = { 0 };
		::GetClientRect (_Hwnd, &rect);

#if(0) // Denzil - DX Window initialization
		// Is the window the correct size for this resolution?
		if ((rect.right-rect.left) != ResolutionWidth ||
			 (rect.bottom-rect.top) != ResolutionHeight) {

			// Calculate what the main window's bounding rectangle should be to
			// accomodate this resolution
			rect.left = 0;
			rect.top = 0;
			rect.right = ResolutionWidth;
			rect.bottom = ResolutionHeight;
			DWORD dwstyle = ::GetWindowLong (_Hwnd, GWL_STYLE);

			AdjustWindowRect (&rect, dwstyle, false);

			// Resize the window to fit this resolution
			::SetWindowPos (_Hwnd,
								 NULL,
								 0,
								 0,
								 rect.right-rect.left,
								 rect.bottom-rect.top,
								 SWP_NOZORDER | SWP_NOMOVE);
		}
#else
		// Adjust the main window's client area to accomodate the resolution
		DWORD dwstyle = ::GetWindowLong(_Hwnd, GWL_STYLE);
		DWORD dwexstyle = ::GetWindowLong(_Hwnd, GWL_EXSTYLE);

		rect.right = ResolutionWidth;
		rect.bottom = ResolutionHeight;

		::AdjustWindowRectEx(&rect, dwstyle, (::GetMenu(_Hwnd) != NULL), dwexstyle);

		::SetWindowPos(_Hwnd, HWND_TOP, 0, 0,
				(rect.right - rect.left), (rect.bottom - rect.top),
				SWP_SHOWWINDOW|SWP_NOCOPYBITS);
#endif
	}
#endif

	/*
	** Time to actually create the device.
	*/
	if (_PresentParameters.AutoDepthStencilFormat==D3DFMT_UNKNOWN) {
		if (BitDepth==32) {
			_PresentParameters.AutoDepthStencilFormat=D3DFMT_D32;
		}
		else {
			_PresentParameters.AutoDepthStencilFormat=D3DFMT_D16;
		}
	}
	return Create_Device();
}

bool DX8Wrapper::Set_Next_Render_Device(void)
{
	int new_dev = (CurRenderDevice + 1) % _RenderDeviceNameTable.Count();
	return Set_Render_Device(new_dev);
}

bool DX8Wrapper::Toggle_Windowed(void)
{
#ifdef WW3D_DX8
	// State OK?
	assert (IsInitted);
	if (IsInitted) {

		// Get information about the current render device's resolutions
		const RenderDeviceDescClass &render_device = Get_Render_Device_Desc ();
		const DynamicVectorClass<ResolutionDescClass> &resolutions = render_device.Enumerate_Resolutions ();

		// Loop through all the resolutions supported by the current device.
		// If we aren't currently running under one of these resolutions,
		// then we should probably		 to the closest resolution before
		// toggling the windowed state.
		int curr_res = -1;
		for (int res = 0;
		     (res < resolutions.Count ()) && (curr_res == -1);
			  res ++) {

			// Is this the resolution we are looking for?
			if ((resolutions[res].Width == ResolutionWidth) &&
				 (resolutions[res].Height == ResolutionHeight) &&
				 (resolutions[res].BitDepth == BitDepth)) {
				curr_res = res;
			}
		}

		if (curr_res == -1) {

			// We don't match any of the standard resolutions,
			// so set the first resolution and toggle the windowed state.
			return Set_Device_Resolution (resolutions[0].Width,
								 resolutions[0].Height,
								 resolutions[0].BitDepth,
								 !IsWindowed, true);
		} else {

			// Toggle the windowed state
			return Set_Device_Resolution (-1, -1, -1, !IsWindowed, true);
		}
	}
#endif //WW3D_DX8

	return false;
}

void DX8Wrapper::Set_Swap_Interval(int swap)
{
	switch (swap) {
		case 0: _PresentParameters.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE; break;
		case 1: _PresentParameters.PresentationInterval = D3DPRESENT_INTERVAL_ONE ; break;
		case 2: _PresentParameters.PresentationInterval = D3DPRESENT_INTERVAL_TWO; break;
		case 3: _PresentParameters.PresentationInterval = D3DPRESENT_INTERVAL_THREE; break;
		default: _PresentParameters.PresentationInterval = D3DPRESENT_INTERVAL_ONE ; break;
	}

	Reset_Device();
}

int DX8Wrapper::Get_Swap_Interval(void)
{
	return _PresentParameters.PresentationInterval;
}

int DX8Wrapper::Get_Render_Device_Count(void)
{
	return _RenderDeviceNameTable.Count();

}
int DX8Wrapper::Get_Render_Device(void)
{
	assert(IsInitted);
	return CurRenderDevice;
}

const RenderDeviceDescClass & DX8Wrapper::Get_Render_Device_Desc(int deviceidx)
{
	WWASSERT(IsInitted);

	if ((deviceidx == -1) && (CurRenderDevice == -1)) {
		CurRenderDevice = 0;
	}

	// if the device index is -1 then we want the current device
	if (deviceidx == -1) {
		WWASSERT(CurRenderDevice >= 0);
		WWASSERT(CurRenderDevice < _RenderDeviceNameTable.Count());
		return _RenderDeviceDescriptionTable[CurRenderDevice];
	}

	// We can only ask for multiple device information if the devices
	// have been detected.
	WWASSERT(deviceidx >= 0);
	WWASSERT(deviceidx < _RenderDeviceNameTable.Count());
	return _RenderDeviceDescriptionTable[deviceidx];
}

const char * DX8Wrapper::Get_Render_Device_Name(int device_index)
{
	device_index = device_index % _RenderDeviceShortNameTable.Count();
	return _RenderDeviceShortNameTable[device_index];
}

bool DX8Wrapper::Set_Device_Resolution(int width,int height,int bits,int windowed, bool resize_window)
{
	if (D3DDevice != NULL) {

		if (width != -1) {
			_PresentParameters.BackBufferWidth = ResolutionWidth = width;
		}
		if (height != -1) {
			_PresentParameters.BackBufferHeight = ResolutionHeight = height;
		}
// FIXME TODO: support changing windowed status and changing the bit depth
		return Reset_Device();
	} else {
		return false;
	}
}

void DX8Wrapper::Get_Device_Resolution(int & set_w,int & set_h,int & set_bits,bool & set_windowed)
{
	WWASSERT(IsInitted);

	set_w = ResolutionWidth;
	set_h = ResolutionHeight;
	set_bits = BitDepth;
	set_windowed = IsWindowed;

	return ;
}

void DX8Wrapper::Get_Render_Target_Resolution(int & set_w,int & set_h,int & set_bits,bool & set_windowed)
{
	WWASSERT(IsInitted);

	if (CurrentRenderTarget != NULL) {
		D3DSURFACE_DESC info;
		CurrentRenderTarget->GetDesc (&info);

		set_w				= info.Width;
		set_h				= info.Height;
		set_bits			= BitDepth;		// should we get the actual bit depth of the target?
		set_windowed	= IsWindowed;	// this doesn't really make sense for render targets (shouldn't matter)...

	} else {
		Get_Device_Resolution (set_w, set_h, set_bits, set_windowed);
	}

	return ;
}

bool DX8Wrapper::Registry_Save_Render_Device( const char * sub_key )
{
	int	width, height, depth;
	bool	windowed;
	Get_Device_Resolution(width, height, depth, windowed);
	return Registry_Save_Render_Device(sub_key, CurRenderDevice, ResolutionWidth, ResolutionHeight, BitDepth, IsWindowed, TextureBitDepth);
}

bool DX8Wrapper::Registry_Save_Render_Device( const char *sub_key, int device, int width, int height, int depth, bool windowed, int texture_depth)
{
	INIClass ini(W3D_CONF_FILE);

	ini.Put_String(W3D_SECTION_RENDER, VALUE_INI_RENDER_DEVICE_NAME, _RenderDeviceShortNameTable[device]);
	ini.Put_Int(W3D_SECTION_RENDER, VALUE_INI_RENDER_DEVICE_WIDTH, width);
	ini.Put_Int(W3D_SECTION_RENDER, VALUE_INI_RENDER_DEVICE_HEIGHT, height);
	ini.Put_Int(W3D_SECTION_RENDER, VALUE_INI_RENDER_DEVICE_DEPTH, depth);
	ini.Put_Bool(W3D_SECTION_RENDER, VALUE_INI_RENDER_DEVICE_WINDOWED, windowed != 0);
	ini.Put_Int(W3D_SECTION_RENDER, VALUE_INI_RENDER_DEVICE_TEXTURE_DEPTH, texture_depth);
	
	return ini.Save(W3D_CONF_FILE) != 0;
}

bool DX8Wrapper::Registry_Load_Render_Device( const char * sub_key, bool resize_window )
{
	char	name[ 200 ];
	int	width,height,depth,windowed;

	if (	Registry_Load_Render_Device(	sub_key,
													name,
													sizeof(name),
													width,
													height,
													depth,
													windowed,
													TextureBitDepth) &&
			(*name != 0))
	{
		WWDEBUG_SAY(( "Device %s (%d X %d) %d bit windowed:%d\n", name,width,height,depth,windowed));

		if (TextureBitDepth==16 || TextureBitDepth==32) {
//			WWDEBUG_SAY(( "Texture depth %d\n", TextureBitDepth));
		} else {
			WWDEBUG_SAY(( "Invalid texture depth %d, switching to 16 bits\n", TextureBitDepth));
			TextureBitDepth=16;
		}


//		_RenderDeviceDescriptionTable.


		if ( Set_Render_Device( name, width,height,depth,windowed, resize_window ) != true) {
			if (depth==16) depth=32;
			else depth=16;
			if ( Set_Render_Device( name, width,height,depth,windowed, resize_window ) == true) {
				return true;
			}
			if (depth==16) depth=32;
			else depth=16;
			// we'll test resolutions down, so if start is 640, increase to begin with...
			if (width==640) {
				width=1024;
				height=768;
			}
			while (true) {
				if (width>2048) {
					width=2048;
					height=1536;
				}
				else if (width>1920) {
					width=1920;
					height=1440;
				}
				else if (width>1600) {
					width=1600;
					height=1200;
				}
				else if (width>1280) {
					width=1280;
					height=1024;
				}
				else if (width>1024) {
					width=1024;
					height=768;
				}
				else if (width>800) {
					width=800;
					height=600;
				}
				else if (width!=640) {
					width=640;
					height=480;
				}
				else {
					return Set_Any_Render_Device();
				}
				for (int i=0;i<2;++i) {
					if ( Set_Render_Device( name, width,height,depth,windowed, resize_window ) == true) {
						return true;
					}
					if (depth==16) depth=32;
					else depth=16;
				}
			}
		}

		return true;
	}

	WWDEBUG_SAY(( "Error getting Registry\n" ));

	return Set_Any_Render_Device();
}

bool DX8Wrapper::Registry_Load_Render_Device( const char * sub_key, char *device, int device_len, int &width, int &height, int &depth, int &windowed, int &texture_depth)
{
	INIClass ini(W3D_CONF_FILE);

	if (ini.Is_Present(W3D_SECTION_RENDER)) {
		ini.Get_String(W3D_SECTION_RENDER, VALUE_INI_RENDER_DEVICE_NAME, "", device, device_len);

		width =	ini.Get_Int(W3D_SECTION_RENDER, VALUE_INI_RENDER_DEVICE_WIDTH, -1);
		height = ini.Get_Int(W3D_SECTION_RENDER, VALUE_INI_RENDER_DEVICE_HEIGHT, -1);
		depth =	ini.Get_Int(W3D_SECTION_RENDER, VALUE_INI_RENDER_DEVICE_DEPTH, -1);
		windowed = ini.Get_Bool(W3D_SECTION_RENDER, VALUE_INI_RENDER_DEVICE_WINDOWED, false);
		texture_depth = ini.Get_Int(W3D_SECTION_RENDER, VALUE_INI_RENDER_DEVICE_TEXTURE_DEPTH, -1);
		return true;
	}
	*device='\0';
	width=-1;
	height=-1;
	depth=-1;
	windowed=-1;
	texture_depth=-1;
	return false;
}


bool DX8Wrapper::Find_Color_And_Z_Mode(int resx,int resy,int bitdepth,D3DFORMAT * set_colorbuffer,D3DFORMAT * set_zmode)
{
	static D3DFORMAT _formats16[] =
	{
		D3DFMT_R5G6B5,
		D3DFMT_X1R5G5B5,
		D3DFMT_A1R5G5B5
	};

	static D3DFORMAT _formats32[] =
	{
		D3DFMT_A8R8G8B8,
		D3DFMT_X8R8G8B8,
		D3DFMT_R8G8B8,
	};

	/*
	** Select the table that we're going to use to search for a valid backbuffer format
	*/
	D3DFORMAT * format_table = NULL;
	int format_count = 0;

	if (BitDepth == 16) {
		format_table = _formats16;
		format_count = sizeof(_formats16) / sizeof(D3DFORMAT);
	} else {
		format_table = _formats32;
		format_count = sizeof(_formats32) / sizeof(D3DFORMAT);
	}

	/*
	** now search for a valid format
	*/
	bool found = false;
	unsigned int mode = 0;
	int format_index;

	for (format_index=0; format_index < format_count; format_index++) {
		found |= Find_Color_Mode(format_table[format_index],resx,resy,&mode);
		if (found) break;
	}

	if (!found) {
		return false;
	} else {
		*set_colorbuffer = format_table[format_index];
	}

	/*
	** We found a backbuffer format, now find a zbuffer format
	*/
	return Find_Z_Mode(*set_colorbuffer,*set_colorbuffer, set_zmode);
};


// find the resolution mode with at least resx,resy with the highest supported
// refresh rate
bool DX8Wrapper::Find_Color_Mode(D3DFORMAT colorbuffer, int resx, int resy, UINT *mode)
{
	UINT i,j,modemax;
	UINT rx,ry;
	D3DDISPLAYMODE dmode;
	::ZeroMemory(&dmode, sizeof(D3DDISPLAYMODE));

	rx=(unsigned int) resx;
	ry=(unsigned int) resy;

	bool found=false;

	modemax=D3DInterface->GetAdapterModeCount(D3DADAPTER_DEFAULT, D3DFMT_X8R8G8B8);

	i=0;

	while (i<modemax && !found)
	{
		D3DInterface->EnumAdapterModes(D3DADAPTER_DEFAULT, D3DFMT_X8R8G8B8, i, &dmode);
		if (dmode.Width==rx && dmode.Height==ry && dmode.Format==colorbuffer) {
			WWDEBUG_SAY(("Found valid color mode.  Width = %d Height = %d Format = %d\r\n",dmode.Width,dmode.Height,dmode.Format));
			found=true;
		}
		i++;
	}

	i--; // this is the first valid mode

	// no match
	if (!found) {
		WWDEBUG_SAY(("Failed to find a valid color mode\r\n"));
		return false;
	}

	// go to the highest refresh rate in this mode
	bool stillok=true;

	j=i;
	while (j<modemax && stillok)
	{
		D3DInterface->EnumAdapterModes(D3DADAPTER_DEFAULT, D3DFMT_X8R8G8B8, j, &dmode);
		if (dmode.Width==rx && dmode.Height==ry && dmode.Format==colorbuffer)
			stillok=true; else stillok=false;
		j++;
	}

	if (stillok==false) *mode=j-2;
	else *mode=i;

	return true;
}

// Helper function to find a Z buffer mode for the colorbuffer
// Will look for greatest Z precision
bool DX8Wrapper::Find_Z_Mode(D3DFORMAT colorbuffer,D3DFORMAT backbuffer, D3DFORMAT *zmode)
{
	if (Test_Z_Mode(colorbuffer,backbuffer,D3DFMT_D32))
	{
		*zmode=D3DFMT_D32;
		WWDEBUG_SAY(("Found zbuffer mode D3DFMT_D32\r\n"));
		return true;
	}

	if (Test_Z_Mode(colorbuffer,backbuffer,D3DFMT_D24S8))
	{
		*zmode=D3DFMT_D24S8;
		WWDEBUG_SAY(("Found zbuffer mode D3DFMT_D24S8\r\n"));
		return true;
	}

	if (Test_Z_Mode(colorbuffer,backbuffer,D3DFMT_D24X8))
	{
		*zmode=D3DFMT_D24X8;
		WWDEBUG_SAY(("Found zbuffer mode D3DFMT_D24X8\r\n"));
		return true;
	}

	if (Test_Z_Mode(colorbuffer,backbuffer,D3DFMT_D24X4S4))
	{
		*zmode=D3DFMT_D24X4S4;
		WWDEBUG_SAY(("Found zbuffer mode D3DFMT_D24X4S4\r\n"));
		return true;
	}

	if (Test_Z_Mode(colorbuffer,backbuffer,D3DFMT_D16))
	{
		*zmode=D3DFMT_D16;
		WWDEBUG_SAY(("Found zbuffer mode D3DFMT_D16\r\n"));
		return true;
	}

	if (Test_Z_Mode(colorbuffer,backbuffer,D3DFMT_D15S1))
	{
		*zmode=D3DFMT_D15S1;
		WWDEBUG_SAY(("Found zbuffer mode D3DFMT_D15S1\r\n"));
		return true;
	}

	// can't find a match
	WWDEBUG_SAY(("Failed to find a valid zbuffer mode\r\n"));
	return false;
}

bool DX8Wrapper::Test_Z_Mode(D3DFORMAT colorbuffer,D3DFORMAT backbuffer, D3DFORMAT zmode)
{
	// See if we have this mode first
	if (FAILED(D3DInterface->CheckDeviceFormat(D3DADAPTER_DEFAULT,WW3D_DEVTYPE,
		colorbuffer,D3DUSAGE_DEPTHSTENCIL,D3DRTYPE_SURFACE,zmode)))
	{
		WWDEBUG_SAY(("CheckDeviceFormat failed.  Colorbuffer format = %d  Zbufferformat = %d\r\n",colorbuffer,zmode));
		return false;
	}

	// Then see if it matches the color buffer
	if(FAILED(D3DInterface->CheckDepthStencilMatch(D3DADAPTER_DEFAULT, WW3D_DEVTYPE,
		colorbuffer,backbuffer,zmode)))
	{
		WWDEBUG_SAY(("CheckDepthStencilMatch failed.  Colorbuffer format = %d  Backbuffer format = %d Zbufferformat = %d\r\n",colorbuffer,backbuffer,zmode));
		return false;
	}
	return true;
}


void DX8Wrapper::Reset_Statistics()
{
	matrix_changes	= 0;
	material_changes = 0;
	vertex_buffer_changes = 0;
	index_buffer_changes = 0;
	light_changes = 0;
	texture_changes = 0;
	render_state_changes =0;
	texture_stage_state_changes =0;

	number_of_DX8_calls = 0;
	last_frame_matrix_changes = 0;
	last_frame_material_changes = 0;
	last_frame_vertex_buffer_changes = 0;
	last_frame_index_buffer_changes = 0;
	last_frame_light_changes = 0;
	last_frame_texture_changes = 0;
	last_frame_render_state_changes = 0;
	last_frame_texture_stage_state_changes = 0;
	last_frame_number_of_DX8_calls = 0;
}

void DX8Wrapper::Begin_Statistics()
{
	matrix_changes=0;
	material_changes=0;
	vertex_buffer_changes=0;
	index_buffer_changes=0;
	light_changes=0;
	texture_changes = 0;
	render_state_changes =0;
	texture_stage_state_changes =0;
	number_of_DX8_calls=0;
}

void DX8Wrapper::End_Statistics()
{
	last_frame_matrix_changes=matrix_changes;
	last_frame_material_changes=material_changes;
	last_frame_vertex_buffer_changes=vertex_buffer_changes;
	last_frame_index_buffer_changes=index_buffer_changes;
	last_frame_light_changes=light_changes;
	last_frame_texture_changes = texture_changes;
	last_frame_render_state_changes = render_state_changes;
	last_frame_texture_stage_state_changes = texture_stage_state_changes;
	last_frame_number_of_DX8_calls=number_of_DX8_calls;
}

unsigned DX8Wrapper::Get_Last_Frame_Matrix_Changes()			{ return last_frame_matrix_changes; }
unsigned DX8Wrapper::Get_Last_Frame_Material_Changes()		{ return last_frame_material_changes; }
unsigned DX8Wrapper::Get_Last_Frame_Vertex_Buffer_Changes()	{ return last_frame_vertex_buffer_changes; }
unsigned DX8Wrapper::Get_Last_Frame_Index_Buffer_Changes()	{ return last_frame_index_buffer_changes; }
unsigned DX8Wrapper::Get_Last_Frame_Light_Changes()			{ return last_frame_light_changes; }
unsigned DX8Wrapper::Get_Last_Frame_Texture_Changes()			{ return last_frame_texture_changes; }
unsigned DX8Wrapper::Get_Last_Frame_Render_State_Changes()	{ return last_frame_render_state_changes; }
unsigned DX8Wrapper::Get_Last_Frame_Texture_Stage_State_Changes()	{ return last_frame_texture_stage_state_changes; }
unsigned DX8Wrapper::Get_Last_Frame_DX8_Calls()					{ return last_frame_number_of_DX8_calls; }
unsigned int DX8Wrapper::Get_FrameCount(void) {return FrameCount;}

void DX8_Assert()
{
	WWASSERT(DX8Wrapper::_Get_D3D8());
	DX8_THREAD_ASSERT();
}

void DX8Wrapper::Begin_Scene(void)
{
	DX8_THREAD_ASSERT();
	DX8CALL(BeginScene());
}

void DX8Wrapper::End_Scene(bool flip_frames)
{
	DX8_THREAD_ASSERT();
	DX8CALL(EndScene());

	if (flip_frames) {
		// JANI HACK! Some objects flicker on ATI Radeons. This can be fixed by locking the back buffer before flipping.
		// Remember that for this to work the back buffers need to have been create as lockable!
		if (Get_Current_Caps()->Get_Vendor()==DX8Caps::VENDOR_ATI) {
			IDirect3DSurface9 * bb;
			DX8CALL(GetBackBuffer(0,0,D3DBACKBUFFER_TYPE_MONO,&bb));
			D3DLOCKED_RECT rect;
			HRESULT res=bb->LockRect(
					&rect,
					NULL,
					D3DLOCK_READONLY);
			if (res==D3D_OK) {
				bb->UnlockRect();
				bb->Release();
			}
		}

		DX8_Assert();
		HRESULT hr;
		{
			WWPROFILE("DX8Device::Present()");
			hr=_Get_D3D_Device8()->Present(NULL, NULL, NULL, NULL);
		}

		number_of_DX8_calls++;

		if (SUCCEEDED(hr)) {
			IsDeviceLost=false;
			FrameCount++;
		}
		else {
			IsDeviceLost=true;
		}

		// If the device was lost we need to check for cooperative level and possibly reset the device
		if (hr==D3DERR_DEVICELOST) {
			hr=_Get_D3D_Device8()->TestCooperativeLevel();
			if (hr==D3DERR_DEVICENOTRESET) {
				Reset_Device();
			}
			else {
				// Sleep it not active
				ThreadClass::Sleep_Ms(200);
			}
		}
		else {
			DX8_ErrorCode(hr);
		}
	}

	// Each frame, release all of the buffers and textures.
	Set_Vertex_Buffer(NULL);
	Set_Index_Buffer(NULL,0);
	for (unsigned i=0;i<MAX_TEXTURE_STAGES;++i) Set_Texture(i,NULL);
	Set_Material(NULL);
}


void DX8Wrapper::Flip_To_Primary(void)
{
	// If we are fullscreen and the current frame is odd then we need
	// to force a page flip to ensure that the first buffer in the flipping
	// chain is the one visible.
	if (!IsWindowed) {
		DX8_Assert();

		int numBuffers = (_PresentParameters.BackBufferCount + 1);
		int visibleBuffer = (FrameCount % numBuffers);
		int flipCount = ((numBuffers - visibleBuffer) % numBuffers);
		int resetAttempts = 0;

		while ((flipCount > 0) && (resetAttempts < 3)) {
			HRESULT hr = _Get_D3D_Device8()->TestCooperativeLevel();

			if (FAILED(hr)) {
				WWDEBUG_SAY(("TestCooperativeLevel Failed!\n"));

				if (D3DERR_DEVICELOST == hr) {
					IsDeviceLost=true;
					WWDEBUG_SAY(("DEVICELOST: Cannot flip to primary.\n"));
					return;
				}
				IsDeviceLost=false;

				if (D3DERR_DEVICENOTRESET == hr) {
					WWDEBUG_SAY(("DEVICENOTRESET\n"));
					Reset_Device();
					resetAttempts++;
				}
			} else {
				WWDEBUG_SAY(("Flipping: %ld\n", FrameCount));
				hr = _Get_D3D_Device8()->Present(NULL, NULL, NULL, NULL);

				if (SUCCEEDED(hr)) {
					IsDeviceLost=false;
					FrameCount++;
					WWDEBUG_SAY(("Flip to primary succeeded %ld\n", FrameCount));
				}
				else {
					IsDeviceLost=true;
				}
			}

			--flipCount;
		}
	}
}


void DX8Wrapper::Clear(bool clear_color, bool clear_z_stencil, const Vector3 &color, float z, unsigned int stencil)
{
	DX8_THREAD_ASSERT();
	// If we try to clear a stencil buffer which is not there, the entire call will fail
	bool has_stencil = (	_PresentParameters.AutoDepthStencilFormat == D3DFMT_D15S1 ||
								_PresentParameters.AutoDepthStencilFormat == D3DFMT_D24S8 ||
								_PresentParameters.AutoDepthStencilFormat == D3DFMT_D24X4S4);

	DWORD flags = 0;
	if (clear_color) flags |= D3DCLEAR_TARGET;
	if (clear_z_stencil) flags |= D3DCLEAR_ZBUFFER;
	if (clear_z_stencil && has_stencil) flags |= D3DCLEAR_STENCIL;
	if (flags)
	{
		DX8CALL(Clear(0, NULL, flags, Convert_Color(color,0.0f), z, stencil));
	}
}

void DX8Wrapper::Set_Viewport(CONST D3DVIEWPORT9* pViewport)
{
	DX8_THREAD_ASSERT();
	DX8CALL(SetViewport(pViewport));
}

// ----------------------------------------------------------------------------
//
// Set vertex buffer. A reference to previous vertex buffer is released and
// this one is assigned the current vertex buffer. The DX8 vertex buffer will
// actually be set in Apply() which is called by Draw_Indexed_Triangles().
//
// ----------------------------------------------------------------------------

void DX8Wrapper::Set_Vertex_Buffer(const VertexBufferClass* vb)
{
	render_state.vba_offset=0;
	render_state.vba_count=0;
	if (render_state.vertex_buffer) {
		render_state.vertex_buffer->Release_Engine_Ref();
	}
	REF_PTR_SET(render_state.vertex_buffer,const_cast<VertexBufferClass*>(vb));
	if (vb) {
		vb->Add_Engine_Ref();
		render_state.vertex_buffer_type=vb->Type();
	}
	else {
		render_state.index_buffer_type=BUFFER_TYPE_INVALID;
	}
	render_state_changed|=VERTEX_BUFFER_CHANGED;
}

// ----------------------------------------------------------------------------
//
// Set index buffer. A reference to previous index buffer is released and
// this one is assigned the current index buffer. The DX8 index buffer will
// actually be set in Apply() which is called by Draw_Indexed_Triangles().
//
// ----------------------------------------------------------------------------

void DX8Wrapper::Set_Index_Buffer(const IndexBufferClass* ib,unsigned short index_base_offset)
{
	render_state.iba_offset=0;
	if (render_state.index_buffer) {
		render_state.index_buffer->Release_Engine_Ref();
	}
	REF_PTR_SET(render_state.index_buffer,const_cast<IndexBufferClass*>(ib));
	render_state.index_base_offset=index_base_offset;
	if (ib) {
		ib->Add_Engine_Ref();
		render_state.index_buffer_type=ib->Type();
	}
	else {
		render_state.index_buffer_type=BUFFER_TYPE_INVALID;
	}
	render_state_changed|=INDEX_BUFFER_CHANGED;
}

// ----------------------------------------------------------------------------
//
// Set vertex buffer using dynamic access object.
//
// ----------------------------------------------------------------------------

void DX8Wrapper::Set_Vertex_Buffer(const DynamicVBAccessClass& vba_)
{
	if (render_state.vertex_buffer) render_state.vertex_buffer->Release_Engine_Ref();

	DynamicVBAccessClass& vba=const_cast<DynamicVBAccessClass&>(vba_);
	render_state.vertex_buffer_type=vba.Get_Type();
	render_state.vba_offset=vba.VertexBufferOffset;
	render_state.vba_count=vba.Get_Vertex_Count();
	REF_PTR_SET(render_state.vertex_buffer,vba.VertexBuffer);
	render_state.vertex_buffer->Add_Engine_Ref();
	render_state_changed|=VERTEX_BUFFER_CHANGED;
	render_state_changed|=INDEX_BUFFER_CHANGED;		// vba_offset changes so index buffer needs to be reset as well.
}

// ----------------------------------------------------------------------------
//
// Set index buffer using dynamic access object.
//
// ----------------------------------------------------------------------------

void DX8Wrapper::Set_Index_Buffer(const DynamicIBAccessClass& iba_,unsigned short index_base_offset)
{
	if (render_state.index_buffer) render_state.index_buffer->Release_Engine_Ref();

	DynamicIBAccessClass& iba=const_cast<DynamicIBAccessClass&>(iba_);
	render_state.index_base_offset=index_base_offset;
	render_state.index_buffer_type=iba.Get_Type();
	render_state.iba_offset=iba.IndexBufferOffset;
	REF_PTR_SET(render_state.index_buffer,iba.IndexBuffer);
	render_state.index_buffer->Add_Engine_Ref();
	render_state_changed|=INDEX_BUFFER_CHANGED;
}

// ----------------------------------------------------------------------------
//
// Private function for the special case of rendering polygons from sorting
// index and vertex buffers.
//
// ----------------------------------------------------------------------------

void DX8Wrapper::Draw_Sorting_IB_VB(
	unsigned primitive_type,
	unsigned short start_index,
	unsigned short polygon_count,
	unsigned short min_vertex_index,
	unsigned short vertex_count)
{
	WWASSERT(render_state.vertex_buffer_type==BUFFER_TYPE_SORTING || render_state.vertex_buffer_type==BUFFER_TYPE_DYNAMIC_SORTING);
	WWASSERT(render_state.index_buffer_type==BUFFER_TYPE_SORTING || render_state.index_buffer_type==BUFFER_TYPE_DYNAMIC_SORTING);

	// Fill dynamic vertex buffer with sorting vertex buffer vertices
	DynamicVBAccessClass dyn_vb_access(BUFFER_TYPE_DYNAMIC_DX8,dynamic_fvf_type,vertex_count);
	{
		DynamicVBAccessClass::WriteLockClass lock(&dyn_vb_access);
		VertexFormatXYZNDUV2* src = static_cast<SortingVertexBufferClass*>(render_state.vertex_buffer)->VertexBuffer;
		VertexFormatXYZNDUV2* dest= lock.Get_Formatted_Vertex_Array();
		src += render_state.vba_offset + render_state.index_base_offset + min_vertex_index;
		unsigned  size = dyn_vb_access.FVF_Info().Get_FVF_Size()*vertex_count/sizeof(unsigned);
		unsigned *dest_u =(unsigned*) dest;
		unsigned *src_u = (unsigned*) src;

		for (unsigned i=0;i<size;++i) {
			*dest_u++=*src_u++;
		}
	}

	DX8CALL(SetStreamSource(
		0,
		static_cast<DX8VertexBufferClass*>(dyn_vb_access.VertexBuffer)->Get_DX8_Vertex_Buffer(),
		0,
		dyn_vb_access.FVF_Info().Get_FVF_Size()));
	// Release the programmable shader
	// DX8CALL(SetVertexShader(NULL));
	// Set the fixed-function vertex format
	DX8CALL(SetFVF(dyn_vb_access.FVF_Info().Get_FVF()));
	DX8_RECORD_VERTEX_BUFFER_CHANGE();

	unsigned index_count=0;
	switch (primitive_type) {
	case D3DPT_TRIANGLELIST: index_count=polygon_count*3; break;
	case D3DPT_TRIANGLESTRIP: index_count=polygon_count+2; break;
	case D3DPT_TRIANGLEFAN: index_count=polygon_count+2; break;
	default: WWASSERT(0); break; // Unsupported primitive type
	}

	// Fill dynamic index buffer with sorting index buffer vertices
	DynamicIBAccessClass dyn_ib_access(BUFFER_TYPE_DYNAMIC_DX8,index_count);
	{
		DynamicIBAccessClass::WriteLockClass lock(&dyn_ib_access);
		unsigned short* dest=lock.Get_Index_Array();
		unsigned short* src=NULL;
		src=static_cast<SortingIndexBufferClass*>(render_state.index_buffer)->index_buffer;
		src+=render_state.iba_offset+start_index;

		for (unsigned short i=0;i<index_count;++i) {
			unsigned short index=*src++;
			index-=min_vertex_index;
			WWASSERT(index<vertex_count);
			*dest++=index;
		}
	}

	DX8CALL(SetIndices(
		static_cast<DX8IndexBufferClass*>(dyn_ib_access.IndexBuffer)->Get_DX8_Index_Buffer()));
	BaseVertexIndex = dyn_vb_access.VertexBufferOffset;
	DX8_RECORD_INDEX_BUFFER_CHANGE();

	DX8CALL(DrawIndexedPrimitive(
		D3DPT_TRIANGLELIST,
		BaseVertexIndex,
		0,		// start vertex
		vertex_count,
		dyn_ib_access.IndexBufferOffset,
		polygon_count));

	DX8_RECORD_RENDER(polygon_count,vertex_count,render_state.shader);
}

// ----------------------------------------------------------------------------
//
//
//
// ----------------------------------------------------------------------------

void DX8Wrapper::Draw(
	unsigned primitive_type,
	unsigned short start_index,
	unsigned short polygon_count,
	unsigned short min_vertex_index,
	unsigned short vertex_count)
{
	DX8_THREAD_ASSERT();
	SNAPSHOT_SAY(("DX8 - draw\n"));

	Apply_Render_State_Changes();

	// Debug feature to disable triangle drawing...
	if (!_Is_Triangle_Draw_Enabled()) return;

#ifdef MESH_RENDER_SNAPSHOT_ENABLED
	if (WW3D::Is_Snapshot_Activated()) {
		DWORD passes=0;
		SNAPSHOT_SAY(("ValidateDevice: "));
		HRESULT res=D3DDevice->ValidateDevice(&passes);
		switch (res) {
		case D3D_OK:
			SNAPSHOT_SAY(("OK\n"));
			break;

		case D3DERR_CONFLICTINGTEXTUREFILTER:
			SNAPSHOT_SAY(("D3DERR_CONFLICTINGTEXTUREFILTER\n"));
			break;
		case D3DERR_CONFLICTINGTEXTUREPALETTE:
			SNAPSHOT_SAY(("D3DERR_CONFLICTINGTEXTUREPALETTE\n"));
			break;
		case D3DERR_DEVICELOST:
			SNAPSHOT_SAY(("D3DERR_DEVICELOST\n"));
			break;
		case D3DERR_TOOMANYOPERATIONS:
			SNAPSHOT_SAY(("D3DERR_TOOMANYOPERATIONS\n"));
			break;
		case D3DERR_UNSUPPORTEDALPHAARG:
			SNAPSHOT_SAY(("D3DERR_UNSUPPORTEDALPHAARG\n"));
			break;
		case D3DERR_UNSUPPORTEDALPHAOPERATION:
			SNAPSHOT_SAY(("D3DERR_UNSUPPORTEDALPHAOPERATION\n"));
			break;
		case D3DERR_UNSUPPORTEDCOLORARG:
			SNAPSHOT_SAY(("D3DERR_UNSUPPORTEDCOLORARG\n"));
			break;
		case D3DERR_UNSUPPORTEDCOLOROPERATION:
			SNAPSHOT_SAY(("D3DERR_UNSUPPORTEDCOLOROPERATION\n"));
			break;
		case D3DERR_UNSUPPORTEDFACTORVALUE:
			SNAPSHOT_SAY(("D3DERR_UNSUPPORTEDFACTORVALUE\n"));
			break;
		case D3DERR_UNSUPPORTEDTEXTUREFILTER:
			SNAPSHOT_SAY(("D3DERR_UNSUPPORTEDTEXTUREFILTER\n"));
			break;
		case D3DERR_WRONGTEXTUREFORMAT:
			SNAPSHOT_SAY(("D3DERR_WRONGTEXTUREFORMAT\n"));
			break;
		default:
			SNAPSHOT_SAY(("UNKNOWN Error\n"));
			break;
		}
	}
#endif	// MESH_RENDER_SHAPSHOT_ENABLED


	SNAPSHOT_SAY(("DX8 - draw %d polygons (%d vertices)\n",polygon_count,vertex_count));

	if (vertex_count<3) {
		min_vertex_index=0;
		switch (render_state.vertex_buffer_type) {
		case BUFFER_TYPE_DX8:
		case BUFFER_TYPE_SORTING:
			vertex_count=render_state.vertex_buffer->Get_Vertex_Count()-render_state.index_base_offset-render_state.vba_offset-min_vertex_index;
			break;
		case BUFFER_TYPE_DYNAMIC_DX8:
		case BUFFER_TYPE_DYNAMIC_SORTING:
			vertex_count=render_state.vba_count;
			break;
		}
	}

	switch (render_state.vertex_buffer_type) {
	case BUFFER_TYPE_DX8:
	case BUFFER_TYPE_DYNAMIC_DX8:
		switch (render_state.index_buffer_type) {
		case BUFFER_TYPE_DX8:
		case BUFFER_TYPE_DYNAMIC_DX8:
			{
				DX8_RECORD_RENDER(polygon_count,vertex_count,render_state.shader);
				DX8CALL(DrawIndexedPrimitive(
					(D3DPRIMITIVETYPE)primitive_type,
					BaseVertexIndex,
					min_vertex_index,
					vertex_count,
					start_index+render_state.iba_offset,
					polygon_count));
			}
			break;
		case BUFFER_TYPE_SORTING:
		case BUFFER_TYPE_DYNAMIC_SORTING:
			WWASSERT_PRINT(0,"VB and IB must of same type (sorting or dx8)");
			break;
		case BUFFER_TYPE_INVALID:
			WWASSERT(0);
			break;
		}
		break;
	case BUFFER_TYPE_SORTING:
	case BUFFER_TYPE_DYNAMIC_SORTING:
		switch (render_state.index_buffer_type) {
		case BUFFER_TYPE_DX8:
		case BUFFER_TYPE_DYNAMIC_DX8:
			WWASSERT_PRINT(0,"VB and IB must of same type (sorting or dx8)");
			break;
		case BUFFER_TYPE_SORTING:
		case BUFFER_TYPE_DYNAMIC_SORTING:
			Draw_Sorting_IB_VB(primitive_type,start_index,polygon_count,min_vertex_index,vertex_count);
			break;
		case BUFFER_TYPE_INVALID:
			WWASSERT(0);
			break;
		}
		break;
	case BUFFER_TYPE_INVALID:
		WWASSERT(0);
		break;
	}
}

// ----------------------------------------------------------------------------
//
//
//
// ----------------------------------------------------------------------------

void DX8Wrapper::Draw_Triangles(
	unsigned buffer_type,
	unsigned short start_index,
	unsigned short polygon_count,
	unsigned short min_vertex_index,
	unsigned short vertex_count)
{
	if (buffer_type==BUFFER_TYPE_SORTING || buffer_type==BUFFER_TYPE_DYNAMIC_SORTING) {
		SortingRendererClass::Insert_Triangles(start_index,polygon_count,min_vertex_index,vertex_count);
	}
	else {
		Draw(D3DPT_TRIANGLELIST,start_index,polygon_count,min_vertex_index,vertex_count);
	}
}

// ----------------------------------------------------------------------------
//
//
//
// ----------------------------------------------------------------------------

void DX8Wrapper::Draw_Triangles(
	unsigned short start_index,
	unsigned short polygon_count,
	unsigned short min_vertex_index,
	unsigned short vertex_count)
{
	Draw(D3DPT_TRIANGLELIST,start_index,polygon_count,min_vertex_index,vertex_count);
}

// ----------------------------------------------------------------------------
//
//
//
// ----------------------------------------------------------------------------

void DX8Wrapper::Draw_Strip(
	unsigned short start_index,
	unsigned short polygon_count,
	unsigned short min_vertex_index,
	unsigned short vertex_count)
{
	Draw(D3DPT_TRIANGLESTRIP,start_index,polygon_count,min_vertex_index,vertex_count);
}

// ----------------------------------------------------------------------------
//
//
//
// ----------------------------------------------------------------------------

void DX8Wrapper::Apply_Render_State_Changes()
{
	if (!render_state_changed) return;
	if (render_state_changed&SHADER_CHANGED) {
		SNAPSHOT_SAY(("DX8 - apply shader\n"));
		render_state.shader.Apply();
	}

	unsigned mask=TEXTURE0_CHANGED;
	for (unsigned i=0;i<MAX_TEXTURE_STAGES;++i,mask<<=1) {
		if (render_state_changed&mask) {
			SNAPSHOT_SAY(("DX8 - apply texture %d (%s)\n",i,render_state.Textures[i] ? static_cast<const char *>(render_state.Textures[i]->Get_Full_Path()) : "NULL"));
			if (render_state.Textures[i]) render_state.Textures[i]->Apply(i);
			else TextureClass::Apply_Null(i);
		}
		else {
			SNAPSHOT_SAY(("DX8 - texture %d not changed (%s)\n",i,render_state.Textures[i] ? static_cast<const char *>(render_state.Textures[i]->Get_Full_Path()) : "NULL"));
		}
	}

	if (render_state_changed&MATERIAL_CHANGED) {
		SNAPSHOT_SAY(("DX8 - apply material\n"));
		VertexMaterialClass* material=const_cast<VertexMaterialClass*>(render_state.material);
		if (material) {
			material->Apply();
		}
		else VertexMaterialClass::Apply_Null();
	}

	if (render_state_changed&LIGHTS_CHANGED)
	{
		unsigned mask=LIGHT0_CHANGED;
		for (unsigned index=0;index<4;++index,mask<<=1) {
			if (render_state_changed&mask) {
				SNAPSHOT_SAY(("DX8 - apply light %d\n",index));
				if (render_state.LightEnable[index]) {
					Set_DX8_Light(index,&render_state.Lights[index]);
				}
				else {
					Set_DX8_Light(index,NULL);
				}
			}
		}
	}

	if (render_state_changed&WORLD_CHANGED) {
		SNAPSHOT_SAY(("DX8 - apply world matrix\n"));
		_Set_DX8_Transform(D3DTS_WORLD,render_state.world);
	}
	if (render_state_changed&VIEW_CHANGED) {
		SNAPSHOT_SAY(("DX8 - apply view matrix\n"));
		_Set_DX8_Transform(D3DTS_VIEW,render_state.view);
	}
	if (render_state_changed&VERTEX_BUFFER_CHANGED) {
		SNAPSHOT_SAY(("DX8 - apply vb change\n"));
		if (render_state.vertex_buffer) {
			switch (render_state.vertex_buffer_type) {//->Type()) {
			case BUFFER_TYPE_DX8:
			case BUFFER_TYPE_DYNAMIC_DX8:
				DX8CALL(SetStreamSource(
					0,
					static_cast<DX8VertexBufferClass*>(render_state.vertex_buffer)->Get_DX8_Vertex_Buffer(),
					0,
					render_state.vertex_buffer->FVF_Info().Get_FVF_Size()));
				DX8_RECORD_VERTEX_BUFFER_CHANGE();
				// DX8CALL(SetVertexShader(NULL));
				DX8CALL(SetFVF(render_state.vertex_buffer->FVF_Info().Get_FVF()));
				break;
			case BUFFER_TYPE_SORTING:
			case BUFFER_TYPE_DYNAMIC_SORTING:
				break;
			default:
				WWASSERT(0);
			}
		} else {
			DX8CALL(SetStreamSource(0,NULL,0,0));
			DX8_RECORD_VERTEX_BUFFER_CHANGE();
		}
	}
	if (render_state_changed&INDEX_BUFFER_CHANGED) {
		SNAPSHOT_SAY(("DX8 - apply ib change\n"));
		if (render_state.index_buffer) {
			switch (render_state.index_buffer_type) {//->Type()) {
			case BUFFER_TYPE_DX8:
			case BUFFER_TYPE_DYNAMIC_DX8:
				DX8CALL(SetIndices(
					static_cast<DX8IndexBufferClass*>(render_state.index_buffer)->Get_DX8_Index_Buffer()));
				BaseVertexIndex = render_state.index_base_offset+render_state.vba_offset;
				DX8_RECORD_INDEX_BUFFER_CHANGE();
				break;
			case BUFFER_TYPE_SORTING:
			case BUFFER_TYPE_DYNAMIC_SORTING:
				break;
			default:
				WWASSERT(0);
			}
		}
		else {
			DX8CALL(SetIndices(NULL));
			BaseVertexIndex = 0;
			DX8_RECORD_INDEX_BUFFER_CHANGE();
		}
	}

	render_state_changed&=((unsigned)WORLD_IDENTITY|(unsigned)VIEW_IDENTITY);
}

IDirect3DTexture9 * DX8Wrapper::_Create_DX8_Texture(
	unsigned int width,
	unsigned int height,
	WW3DFormat format,
	TextureClass::MipCountType mip_level_count,
	D3DPOOL pool,
	bool rendertarget)
{
	DX8_THREAD_ASSERT();
	DX8_Assert();
	IDirect3DTexture9 *texture = NULL;

	// Paletted textures not supported!
	WWASSERT(format!=WW3D_FORMAT_P8);

	// NOTE: If 'format' is not supported as a texture format, this function will find the closest
	// format that is supported and use that instead.

	// Render target may return NOTAVAILABLE, in
	// which case we return NULL.
	if (rendertarget) {
		HRESULT ret=D3DXCreateTexture(
			DX8Wrapper::_Get_D3D_Device8(),
			width,
			height,
			mip_level_count,
			D3DUSAGE_RENDERTARGET,
			WW3DFormat_To_D3DFormat(format),
			pool,
			&texture);

		if (ret==D3DERR_NOTAVAILABLE) {
			Non_Fatal_Log_DX8_ErrorCode(ret,__FILE__,__LINE__);
			return NULL;
		}

		// If ran out of texture ram, try invalidating some textures and mesh cache.
		if (ret==D3DERR_OUTOFVIDEOMEMORY) {
			WWDEBUG_SAY(("Error: Out of memory while creating render target. Trying to release assets...\n"));
			// Free all textures that haven't been used in the last 5 seconds
			TextureClass::Invalidate_Old_Unused_Textures(5000);

			// Invalidate the mesh cache
			WW3D::_Invalidate_Mesh_Cache();

			ret=D3DXCreateTexture(
				DX8Wrapper::_Get_D3D_Device8(),
				width,
				height,
				mip_level_count,
				D3DUSAGE_RENDERTARGET,
				WW3DFormat_To_D3DFormat(format),
				pool,
				&texture);

			if (SUCCEEDED(ret)) {
				WWDEBUG_SAY(("...Render target creation succesful.\n"));
			}
			else {
				WWDEBUG_SAY(("...Render target creation failed.\n"));
			}
			if (ret==D3DERR_OUTOFVIDEOMEMORY) {
				Non_Fatal_Log_DX8_ErrorCode(ret,__FILE__,__LINE__);
				return NULL;
			}
		}

		DX8_ErrorCode(ret);
		// Just return the texture, no reduction
		// allowed for render targets.
		return texture;
	}

	// We should never run out of video memory when allocating a non-rendertarget texture.
	// However, it seems to happen sometimes when there are a lot of textures in memory and so
	// if it happens we'll release assets and try again (anything is better than crashing).
	unsigned ret=D3DXCreateTexture(
		DX8Wrapper::_Get_D3D_Device8(),
		width,
		height,
		mip_level_count,
		0,
		WW3DFormat_To_D3DFormat(format),
		pool,
		&texture);

	// If ran out of texture ram, try invalidating some textures and mesh cache.
	if (ret==D3DERR_OUTOFVIDEOMEMORY) {
		WWDEBUG_SAY(("Error: Out of memory while creating texture. Trying to release assets...\n"));
		// Free all textures that haven't been used in the last 5 seconds
		TextureClass::Invalidate_Old_Unused_Textures(5000);

		// Invalidate the mesh cache
		WW3D::_Invalidate_Mesh_Cache();

		ret=D3DXCreateTexture(
			DX8Wrapper::_Get_D3D_Device8(),
			width,
			height,
			mip_level_count,
			0,
			WW3DFormat_To_D3DFormat(format),
			pool,
			&texture);
		if (SUCCEEDED(ret)) {
			WWDEBUG_SAY(("...Texture creation succesful.\n"));
		}
		else {
			StringClass format_name(0,true);
			Get_WW3D_Format_Name(format, format_name);
			WWDEBUG_SAY(("...Texture creation failed. (%d x %d, format: %s, mips: %d\n",width,height,format_name,mip_level_count));
		}

	}
	DX8_ErrorCode(ret);

	return texture;
}

IDirect3DTexture9 * DX8Wrapper::_Create_DX8_Texture(
	const char *filename,
	TextureClass::MipCountType mip_level_count)
{
	DX8_THREAD_ASSERT();
	DX8_Assert();
	IDirect3DTexture9 *texture = NULL;

	// NOTE: If the original image format is not supported as a texture format, it will
	// automatically be converted to an appropriate format.
	// NOTE: It is possible to get the size and format of the original image file from this
	// function as well, so if we later want to second-guess D3DX's format conversion decisions
	// we can do so after this function is called..
	unsigned result = D3DXCreateTextureFromFileExA(
		_Get_D3D_Device8(),
		filename,
		D3DX_DEFAULT,
		D3DX_DEFAULT,
		mip_level_count,//create_mipmaps ? 0 : 1,
		0,
		D3DFMT_UNKNOWN,
		D3DPOOL_MANAGED,
		D3DX_FILTER_BOX,
		D3DX_FILTER_BOX,
		0,
		NULL,
		NULL,
		&texture);

	if (result != D3D_OK) {
		return MissingTexture::_Get_Missing_Texture();
	}

	// Make sure texture wasn't paletted!
	D3DSURFACE_DESC desc;
	texture->GetLevelDesc(0,&desc);
	if (desc.Format==D3DFMT_P8) {
		texture->Release();
		return MissingTexture::_Get_Missing_Texture();
	}
	return texture;
}

IDirect3DTexture9 * DX8Wrapper::_Create_DX8_Texture(
	IDirect3DSurface9 *surface,
	TextureClass::MipCountType mip_level_count)
{
	DX8_THREAD_ASSERT();
	DX8_Assert();
	IDirect3DTexture9 *texture = NULL;

	D3DSURFACE_DESC surface_desc;
	::ZeroMemory(&surface_desc, sizeof(D3DSURFACE_DESC));
	surface->GetDesc(&surface_desc);

	// This function will create a texture with a different (but similar) format if the surface is
	// not in a supported texture format.
	WW3DFormat format=D3DFormat_To_WW3DFormat(surface_desc.Format);
	texture = _Create_DX8_Texture(surface_desc.Width, surface_desc.Height, format, mip_level_count);

	// Copy the surface to the texture
	IDirect3DSurface9 *tex_surface = NULL;
	texture->GetSurfaceLevel(0, &tex_surface);
	DX8_ErrorCode(D3DXLoadSurfaceFromSurface(tex_surface, NULL, NULL, surface, NULL, NULL, D3DX_FILTER_BOX, 0));
	tex_surface->Release();

	// Create mipmaps if needed
	if (mip_level_count!=TextureClass::MIP_LEVELS_1) {
		DX8_ErrorCode(D3DXFilterTexture(texture, NULL, 0, D3DX_FILTER_BOX));
	}

	return texture;

}

IDirect3DSurface9 * DX8Wrapper::_Create_DX8_Surface(unsigned int width, unsigned int height, D3DPOOL pool, WW3DFormat format)
{
	DX8_THREAD_ASSERT();
	DX8_Assert();

	IDirect3DSurface9 *surface = NULL;

	// Paletted surfaces not supported!
	WWASSERT(format!=WW3D_FORMAT_P8);

	DX8CALL(CreateOffscreenPlainSurface(width, height, WW3DFormat_To_D3DFormat(format), pool, &surface, NULL));

	return surface;
}

IDirect3DSurface9 * DX8Wrapper::_Create_DX8_Surface(const char *filename_)
{
	DX8_THREAD_ASSERT();
	DX8_Assert();

	// Note: Since there is no "D3DXCreateSurfaceFromFile" and no "GetSurfaceInfoFromFile" (the
	// latter is supposed to be added to D3DX in a future version), we create a texture from the
	// file (w/o mipmaps), check that its surface is equal to the original file data (which it
	// will not be if the file is not in a texture-supported format or size). If so, copy its
	// surface (we might be able to just get its surface and add a ref to it but I'm not sure so
	// I'm not going to risk it) and release the texture. If not, create a surface according to
	// the file data and use D3DXLoadSurfaceFromFile. This is a horrible hack, but it saves us
	// having to write file loaders. Will fix this when D3DX provides us with the right functions.
	// Create a surface the size of the file image data
	IDirect3DSurface9 *surface = NULL;

	{
		file_auto_ptr myfile(_TheFileFactory,filename_);
		// If file not found, create a surface with missing texture in it
		if (!myfile->Is_Available()) {
			return MissingTexture::_Create_Missing_Surface();
		}
	}

	StringClass filename_string(filename_,true);
	surface=TextureLoader::Load_Surface_Immediate(
		filename_string,
		WW3D_FORMAT_UNKNOWN,
		true);
	return surface;
}


/***********************************************************************************************
 * DX8Wrapper::_Update_Texture -- Copies a texture from system memory to video memory          *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/26/2001  hy : Created.                                                                  *
 *=============================================================================================*/
void DX8Wrapper::_Update_Texture(TextureClass *system, TextureClass *video)
{
	WWASSERT(system);
	WWASSERT(video);
	WWASSERT(system->Pool==TextureClass::POOL_SYSTEMMEM);
	WWASSERT(video->Pool==TextureClass::POOL_DEFAULT);
	DX8CALL(UpdateTexture(system->D3DTexture,video->D3DTexture));
}

void DX8Wrapper::Compute_Caps(WW3DFormat display_format)
{
	DX8_THREAD_ASSERT();
	DX8_Assert();
	delete CurrentCaps;
	CurrentCaps=new DX8Caps(_Get_D3D8(),D3DDevice,display_format,Get_Current_Adapter_Identifier());
}

void DX8Wrapper::Set_Light(unsigned index,const LightClass &light)
{
	D3DLIGHT9 dlight;
	Vector3 temp;
	memset(&dlight,0,sizeof(D3DLIGHT9));

	switch (light.Get_Type())
	{
	case LightClass::POINT:
		{
			dlight.Type=D3DLIGHT_POINT;
		}
		break;
	case LightClass::DIRECTIONAL:
		{
			dlight.Type=D3DLIGHT_DIRECTIONAL;
		}
		break;
	case LightClass::SPOT:
		{
			dlight.Type=D3DLIGHT_SPOT;
		}
		break;
	}

	light.Get_Diffuse(&temp);
	temp*=light.Get_Intensity();
	dlight.Diffuse.r=temp.X;
	dlight.Diffuse.g=temp.Y;
	dlight.Diffuse.b=temp.Z;
	dlight.Diffuse.a=1.0f;

	light.Get_Specular(&temp);
	temp*=light.Get_Intensity();
	dlight.Specular.r=temp.X;
	dlight.Specular.g=temp.Y;
	dlight.Specular.b=temp.Z;
	dlight.Specular.a=1.0f;

	light.Get_Ambient(&temp);
	temp*=light.Get_Intensity();
	dlight.Ambient.r=temp.X;
	dlight.Ambient.g=temp.Y;
	dlight.Ambient.b=temp.Z;
	dlight.Ambient.a=1.0f;

	temp=light.Get_Position();
	dlight.Position=*(D3DVECTOR*) &temp;

	light.Get_Spot_Direction(temp);
	dlight.Direction=*(D3DVECTOR*) &temp;

	dlight.Range=light.Get_Attenuation_Range();
	dlight.Falloff=light.Get_Spot_Exponent();
	dlight.Theta=light.Get_Spot_Angle();
	dlight.Phi=light.Get_Spot_Angle();

	// Inverse linear light 1/(1+D)
	double a,b;
	light.Get_Far_Attenuation_Range(a,b);
	dlight.Attenuation0=1.0f;
	if (fabs(a-b)<1e-5)
		// if the attenuation range is too small assume uniform with cutoff
		dlight.Attenuation1=0.0f;
	else
		// this will cause the light to drop to half intensity at the first far attenuation
		dlight.Attenuation1=(float) 1.0/a;
	dlight.Attenuation2=0.0f;

	Set_Light(index,&dlight);
}

// ----------------------------------------------------------------------------
//
// Set the light environment. This is a lighting model which used up to four
// directional lights to produce the lighting.
//
// ----------------------------------------------------------------------------

void DX8Wrapper::Set_Light_Environment(LightEnvironmentClass* light_env)
{
	if (light_env) {

		int light_count = light_env->Get_Light_Count();
		Set_DX8_Render_State(D3DRS_AMBIENT,Convert_Color(light_env->Get_Equivalent_Ambient(),0.0f));

		D3DLIGHT9 light;
		::ZeroMemory(&light, sizeof(D3DLIGHT9));
		light.Type=D3DLIGHT_DIRECTIONAL;
		int l;

		for (l=0;l<light_count;++l) {
			(Vector3&)light.Diffuse=light_env->Get_Light_Diffuse(l);
			Vector3 dir=-light_env->Get_Light_Direction(l);
			light.Direction=(const D3DVECTOR&)(dir);
			Set_Light(l,&light);
		}

		for (;l<4;++l) {
			Set_Light(l,NULL);
		}
	}
/*	else {
		for (int l=0;l<4;++l) {
			Set_Light(l,NULL);
		}
	}
*/
}

IDirect3DSurface9 * DX8Wrapper::_Get_DX8_Front_Buffer()
{
	DX8_THREAD_ASSERT();
	D3DDISPLAYMODE mode;

	DX8CALL(GetDisplayMode(0,&mode));

	IDirect3DSurface9 * fb=NULL;

	DX8CALL(CreateOffscreenPlainSurface(mode.Width,mode.Height,D3DFMT_A8R8G8B8,D3DPOOL_SCRATCH,&fb,NULL));

	DX8CALL(GetFrontBufferData(0,fb));
	return fb;
}

SurfaceClass * DX8Wrapper::_Get_DX8_Back_Buffer(unsigned int num)
{
	DX8_THREAD_ASSERT();

	IDirect3DSurface9 * bb;
	SurfaceClass *surf=NULL;
	DX8CALL(GetBackBuffer(0,num,D3DBACKBUFFER_TYPE_MONO,&bb));
	if (bb)
	{
		surf=NEW_REF(SurfaceClass,(bb));
		bb->Release();
	}

	return surf;
}


TextureClass *
DX8Wrapper::Create_Render_Target (int width, int height, WW3DFormat format)
{
	DX8_THREAD_ASSERT();
	DX8_Assert();
	number_of_DX8_calls++;

	// Use the current display format if format isn't specified
	if (format==WW3D_FORMAT_UNKNOWN) {
		D3DDISPLAYMODE mode;
		DX8CALL(GetDisplayMode(0,&mode));
		format=D3DFormat_To_WW3DFormat(mode.Format);
	}

	// If render target format isn't supported return NULL
	if (!Get_Current_Caps()->Support_Render_To_Texture_Format(format)) {
		WWDEBUG_SAY(("DX8Wrapper - Render target format is not supported\r\n"));
		return NULL;
	}

	//
	//	Note: We're going to force the width and height to be powers of two and equal
	//
	const D3DCAPS9& dx8caps=Get_Current_Caps()->Get_DX8_Caps();
	float poweroftwosize = width;
	if (height > 0 && height < width) {
		poweroftwosize = height;
	}
	poweroftwosize = ::Find_POT (poweroftwosize);

	if (poweroftwosize>dx8caps.MaxTextureWidth) {
		poweroftwosize=dx8caps.MaxTextureWidth;
	}
	if (poweroftwosize>dx8caps.MaxTextureHeight) {
		poweroftwosize=dx8caps.MaxTextureHeight;
	}

	width = height = poweroftwosize;

	//
	//	Attempt to create the render target
	//
	TextureClass * tex = NEW_REF(TextureClass,(width,height,format,TextureClass::MIP_LEVELS_1,TextureClass::POOL_DEFAULT,true));

	// 3dfx drivers are lying in the CheckDeviceFormat call and claiming
	// that they support render targets!
	if (tex->Peek_DX8_Texture() == NULL) {
		WWDEBUG_SAY(("DX8Wrapper - Render target creation failed!\r\n"));
		REF_PTR_RELEASE(tex);
	}

	return tex;
}


void
DX8Wrapper::Set_Render_Target (TextureClass * texture)
{
	WWASSERT(texture != NULL);
	IDirect3DSurface9 * d3d_surf = texture->Get_D3D_Surface_Level();
	WWASSERT(d3d_surf != NULL);
	Set_Render_Target(d3d_surf);
	d3d_surf->Release();

	IsRenderToTexture = true;
}

void
DX8Wrapper::Set_Render_Target(IDirect3DSwapChain9 *swap_chain)
{
	DX8_THREAD_ASSERT();
	WWASSERT (swap_chain != NULL);

	//
	//	Get the back buffer for the swap chain
	//
	LPDIRECT3DSURFACE9 render_target = NULL;
	swap_chain->GetBackBuffer (0, D3DBACKBUFFER_TYPE_MONO, &render_target);

	//
	//	Set this back buffer as the render targer
	//
	Set_Render_Target (render_target, true);

	//
	//	Release our hold on the back buffer
	//
	if (render_target != NULL) {
		render_target->Release ();
		render_target = NULL;
	}

	IsRenderToTexture = false;

	return ;
}

void
DX8Wrapper::Set_Render_Target(IDirect3DSurface9 *render_target, bool use_default_depth_buffer)
{
	DX8_THREAD_ASSERT();
	DX8_Assert();

	//
	//	Should we restore the default render target set a new one?
	//
	if (render_target == NULL || render_target == DefaultRenderTarget) {
		// If there is currently a custom render target, default must NOT be NULL.
		if (CurrentRenderTarget) {
			WWASSERT(DefaultRenderTarget!=NULL);
		}

		//
		//	Restore the default render target
		//
		if (DefaultRenderTarget != NULL) {
			DX8CALL(SetRenderTarget (0,DefaultRenderTarget));
			DX8CALL(SetDepthStencilSurface(DefaultDepthBuffer));
			DefaultRenderTarget->Release ();
			DefaultRenderTarget = NULL;
			if (DefaultDepthBuffer) {
				DefaultDepthBuffer->Release ();
				DefaultDepthBuffer = NULL;
			}
		}

		//
		//	Release our hold on the "current" render target
		//
		if (CurrentRenderTarget != NULL) {
			CurrentRenderTarget->Release ();
			CurrentRenderTarget = NULL;
		}

	} else if (render_target != CurrentRenderTarget) {
		WWASSERT(DefaultRenderTarget==NULL);

		//
		//	We'll need the depth buffer later...
		//
		if (DefaultDepthBuffer == NULL) {
//		IDirect3DSurface9 *depth_buffer = NULL;
			DX8CALL(GetDepthStencilSurface (&DefaultDepthBuffer));
		}

		//
		//	Get a pointer to the default render target (if necessary)
		//
		if (DefaultRenderTarget == NULL) {
			DX8CALL(GetRenderTarget (0,&DefaultRenderTarget));
		}

		//
		//	Release our hold on the old "current" render target
		//
		if (CurrentRenderTarget != NULL) {
			CurrentRenderTarget->Release ();
			CurrentRenderTarget = NULL;
		}

		//
		//	Keep a copy of the current render target (for housekeeping)
		//
		CurrentRenderTarget = render_target;
		WWASSERT (CurrentRenderTarget != NULL);
		if (CurrentRenderTarget != NULL) {
			CurrentRenderTarget->AddRef ();

			//
			//	Switch render targets
			//
			if (use_default_depth_buffer) {
				DX8CALL(SetRenderTarget (0, CurrentRenderTarget));
				DX8CALL(SetDepthStencilSurface(DefaultDepthBuffer));
			} else {
				DX8CALL(SetRenderTarget (0, CurrentRenderTarget));
				DX8CALL(SetDepthStencilSurface(NULL));
			}
		}
	}

	//
	//	Free our hold on the depth buffer
	//
//	if (depth_buffer != NULL) {
//		depth_buffer->Release ();
//		depth_buffer = NULL;
//	}

	IsRenderToTexture = false;
	return ;
}


IDirect3DSwapChain9 *
DX8Wrapper::Create_Additional_Swap_Chain (HWND render_window)
{
	DX8_Assert();

	//
	//	Configure the presentation parameters for a windowed render target
	//
	D3DPRESENT_PARAMETERS params				= { 0 };
	params.BackBufferFormat						= _PresentParameters.BackBufferFormat;
	params.BackBufferCount						= 1;
	params.MultiSampleType						= D3DMULTISAMPLE_NONE;
	params.SwapEffect								= D3DSWAPEFFECT_FLIP; /* or D3DSWAPEFFECT_COPY?  */
	params.hDeviceWindow							= render_window;
	params.Windowed								= true;
	params.EnableAutoDepthStencil				= true;
	params.AutoDepthStencilFormat				= _PresentParameters.AutoDepthStencilFormat;
	params.Flags									= 0;
	params.FullScreen_RefreshRateInHz			= D3DPRESENT_RATE_DEFAULT;
	params.PresentationInterval					= D3DPRESENT_INTERVAL_DEFAULT;

	//
	//	Create the swap chain
	//
	IDirect3DSwapChain9 *swap_chain = NULL;
	DX8CALL(CreateAdditionalSwapChain(&params, &swap_chain));
	return swap_chain;
}

void DX8Wrapper::Flush_DX8_Resource_Manager()
{
	DX8_Assert();
	DX8CALL(EvictManagedResources());
}

unsigned int DX8Wrapper::Get_Free_Texture_RAM()
{
	DX8_Assert();
	number_of_DX8_calls++;
	return DX8Wrapper::_Get_D3D_Device8()->GetAvailableTextureMem();
}

// Converts a linear gamma ramp to one that is controlled by:
// Gamma - controls the curvature of the middle of the curve
// Bright - controls the minimum value of the curve
// Contrast - controls the difference between the maximum and the minimum of the curve
void DX8Wrapper::Set_Gamma(float gamma,float bright,float contrast,bool calibrate,bool uselimit)
{
	gamma=Bound(gamma,0.6f,6.0f);
	bright=Bound(bright,-0.5f,0.5f);
	contrast=Bound(contrast,0.5f,2.0f);
	float oo_gamma=1.0f/gamma;

	DX8_Assert();
	number_of_DX8_calls++;

	DWORD flag=(calibrate?D3DSGR_CALIBRATE:D3DSGR_NO_CALIBRATION);

	D3DGAMMARAMP ramp;
	float			 limit;	

	// IML: I'm not really sure what the intent of the 'limit' variable is. It does not produce useful results for my purposes.
	if (uselimit) {
		limit=(contrast-1)/2*contrast;
	} else {
		limit = 0.0f;
	}

	// HY - arrived at this equation after much trial and error.
	for (int i=0; i<256; i++) {
		float in,out;
		in=i/256.0f;
		float x=in-limit;
		x=Bound(x,0.0f,1.0f);
		x=powf(x,oo_gamma);
		out=contrast*x+bright;
		out=Bound(out,0.0f,1.0f);
		ramp.red[i]=(WORD) (out*65535);
		ramp.green[i]=(WORD) (out*65535);
		ramp.blue[i]=(WORD) (out*65535);
	}

	if (Get_Current_Caps()->Support_Gamma())	{
		DX8Wrapper::_Get_D3D_Device8()->SetGammaRamp(0,flag,&ramp);
	} else {
		HWND hwnd = GetDesktopWindow();
		HDC hdc = GetDC(hwnd);
		if (hdc)
		{
			SetDeviceGammaRamp (hdc, &ramp);
			ReleaseDC (hwnd, hdc);
		}
	}
}

const char* DX8Wrapper::Get_DX8_Render_State_Name(D3DRENDERSTATETYPE state)
{
	switch (state) {
	case D3DRS_ZENABLE                       : return "D3DRS_ZENABLE";
	case D3DRS_FILLMODE                      : return "D3DRS_FILLMODE";
	case D3DRS_SHADEMODE                     : return "D3DRS_SHADEMODE";
	case D3DRS_ZWRITEENABLE                  : return "D3DRS_ZWRITEENABLE";
	case D3DRS_ALPHATESTENABLE               : return "D3DRS_ALPHATESTENABLE";
	case D3DRS_LASTPIXEL                     : return "D3DRS_LASTPIXEL";
	case D3DRS_SRCBLEND                      : return "D3DRS_SRCBLEND";
	case D3DRS_DESTBLEND                     : return "D3DRS_DESTBLEND";
	case D3DRS_CULLMODE                      : return "D3DRS_CULLMODE";
	case D3DRS_ZFUNC                         : return "D3DRS_ZFUNC";
	case D3DRS_ALPHAREF                      : return "D3DRS_ALPHAREF";
	case D3DRS_ALPHAFUNC                     : return "D3DRS_ALPHAFUNC";
	case D3DRS_DITHERENABLE                  : return "D3DRS_DITHERENABLE";
	case D3DRS_ALPHABLENDENABLE              : return "D3DRS_ALPHABLENDENABLE";
	case D3DRS_FOGENABLE                     : return "D3DRS_FOGENABLE";
	case D3DRS_SPECULARENABLE                : return "D3DRS_SPECULARENABLE";
	case D3DRS_FOGCOLOR                      : return "D3DRS_FOGCOLOR";
	case D3DRS_FOGTABLEMODE                  : return "D3DRS_FOGTABLEMODE";
	case D3DRS_FOGSTART                      : return "D3DRS_FOGSTART";
	case D3DRS_FOGEND                        : return "D3DRS_FOGEND";
	case D3DRS_FOGDENSITY                    : return "D3DRS_FOGDENSITY";
	case D3DRS_RANGEFOGENABLE                : return "D3DRS_RANGEFOGENABLE";
	case D3DRS_STENCILENABLE                 : return "D3DRS_STENCILENABLE";
	case D3DRS_STENCILFAIL                   : return "D3DRS_STENCILFAIL";
	case D3DRS_STENCILZFAIL                  : return "D3DRS_STENCILZFAIL";
	case D3DRS_STENCILPASS                   : return "D3DRS_STENCILPASS";
	case D3DRS_STENCILFUNC                   : return "D3DRS_STENCILFUNC";
	case D3DRS_STENCILREF                    : return "D3DRS_STENCILREF";
	case D3DRS_STENCILMASK                   : return "D3DRS_STENCILMASK";
	case D3DRS_STENCILWRITEMASK              : return "D3DRS_STENCILWRITEMASK";
	case D3DRS_TEXTUREFACTOR                 : return "D3DRS_TEXTUREFACTOR";
	case D3DRS_WRAP0                         : return "D3DRS_WRAP0";
	case D3DRS_WRAP1                         : return "D3DRS_WRAP1";
	case D3DRS_WRAP2                         : return "D3DRS_WRAP2";
	case D3DRS_WRAP3                         : return "D3DRS_WRAP3";
	case D3DRS_WRAP4                         : return "D3DRS_WRAP4";
	case D3DRS_WRAP5                         : return "D3DRS_WRAP5";
	case D3DRS_WRAP6                         : return "D3DRS_WRAP6";
	case D3DRS_WRAP7                         : return "D3DRS_WRAP7";
	case D3DRS_CLIPPING                      : return "D3DRS_CLIPPING";
	case D3DRS_LIGHTING                      : return "D3DRS_LIGHTING";
	case D3DRS_AMBIENT                       : return "D3DRS_AMBIENT";
	case D3DRS_FOGVERTEXMODE                 : return "D3DRS_FOGVERTEXMODE";
	case D3DRS_COLORVERTEX                   : return "D3DRS_COLORVERTEX";
	case D3DRS_LOCALVIEWER                   : return "D3DRS_LOCALVIEWER";
	case D3DRS_NORMALIZENORMALS              : return "D3DRS_NORMALIZENORMALS";
	case D3DRS_DIFFUSEMATERIALSOURCE         : return "D3DRS_DIFFUSEMATERIALSOURCE";
	case D3DRS_SPECULARMATERIALSOURCE        : return "D3DRS_SPECULARMATERIALSOURCE";
	case D3DRS_AMBIENTMATERIALSOURCE         : return "D3DRS_AMBIENTMATERIALSOURCE";
	case D3DRS_EMISSIVEMATERIALSOURCE        : return "D3DRS_EMISSIVEMATERIALSOURCE";
	case D3DRS_VERTEXBLEND                   : return "D3DRS_VERTEXBLEND";
	case D3DRS_CLIPPLANEENABLE               : return "D3DRS_CLIPPLANEENABLE";
	case D3DRS_POINTSIZE                     : return "D3DRS_POINTSIZE";
	case D3DRS_POINTSIZE_MIN                 : return "D3DRS_POINTSIZE_MIN";
	case D3DRS_POINTSPRITEENABLE             : return "D3DRS_POINTSPRITEENABLE";
	case D3DRS_POINTSCALEENABLE              : return "D3DRS_POINTSCALEENABLE";
	case D3DRS_POINTSCALE_A                  : return "D3DRS_POINTSCALE_A";
	case D3DRS_POINTSCALE_B                  : return "D3DRS_POINTSCALE_B";
	case D3DRS_POINTSCALE_C                  : return "D3DRS_POINTSCALE_C";
	case D3DRS_MULTISAMPLEANTIALIAS          : return "D3DRS_MULTISAMPLEANTIALIAS";
	case D3DRS_MULTISAMPLEMASK               : return "D3DRS_MULTISAMPLEMASK";
	case D3DRS_PATCHEDGESTYLE                : return "D3DRS_PATCHEDGESTYLE";
	case D3DRS_DEBUGMONITORTOKEN             : return "D3DRS_DEBUGMONITORTOKEN";
	case D3DRS_POINTSIZE_MAX                 : return "D3DRS_POINTSIZE_MAX";
	case D3DRS_INDEXEDVERTEXBLENDENABLE      : return "D3DRS_INDEXEDVERTEXBLENDENABLE";
	case D3DRS_COLORWRITEENABLE              : return "D3DRS_COLORWRITEENABLE";
	case D3DRS_TWEENFACTOR                   : return "D3DRS_TWEENFACTOR";
	case D3DRS_BLENDOP                       : return "D3DRS_BLENDOP";
	case D3DRS_POSITIONDEGREE                : return "D3DRS_POSITIONDEGREE";
	case D3DRS_NORMALDEGREE                  : return "D3DRS_NORMALDEGREE";
	case D3DRS_SCISSORTESTENABLE             : return "D3DRS_SCISSORTESTENABLE";
	case D3DRS_SLOPESCALEDEPTHBIAS           : return "D3DRS_SLOPESCALEDEPTHBIAS";
	case D3DRS_ANTIALIASEDLINEENABLE         : return "D3DRS_ANTIALIASEDLINEENABLE";
	case D3DRS_MINTESSELLATIONLEVEL          : return "D3DRS_MINTESSELLATIONLEVEL";
	case D3DRS_MAXTESSELLATIONLEVEL          : return "D3DRS_MAXTESSELLATIONLEVEL";
	case D3DRS_ADAPTIVETESS_X                : return "D3DRS_ADAPTIVETESS_X";
	case D3DRS_ADAPTIVETESS_Y                : return "D3DRS_ADAPTIVETESS_Y";
	case D3DRS_ADAPTIVETESS_Z                : return "D3DRS_ADAPTIVETESS_Z";
	case D3DRS_ADAPTIVETESS_W                : return "D3DRS_ADAPTIVETESS_W";
	case D3DRS_ENABLEADAPTIVETESSELLATION    : return "D3DRS_ENABLEADAPTIVETESSELLATION";
	case D3DRS_TWOSIDEDSTENCILMODE           : return "D3DRS_TWOSIDEDSTENCILMODE";
	case D3DRS_CCW_STENCILFAIL               : return "D3DRS_CCW_STENCILFAIL";
	case D3DRS_CCW_STENCILZFAIL              : return "D3DRS_CCW_STENCILZFAIL";
	case D3DRS_CCW_STENCILPASS               : return "D3DRS_CCW_STENCILPASS";
	case D3DRS_CCW_STENCILFUNC               : return "D3DRS_CCW_STENCILFUNC";
	case D3DRS_COLORWRITEENABLE1             : return "D3DRS_COLORWRITEENABLE1";
	case D3DRS_COLORWRITEENABLE2             : return "D3DRS_COLORWRITEENABLE2";
	case D3DRS_COLORWRITEENABLE3             : return "D3DRS_COLORWRITEENABLE3";
	case D3DRS_BLENDFACTOR                   : return "D3DRS_BLENDFACTOR";
	case D3DRS_SRGBWRITEENABLE               : return "D3DRS_SRGBWRITEENABLE";
	case D3DRS_DEPTHBIAS                     : return "D3DRS_DEPTHBIAS";
	case D3DRS_WRAP8                         : return "D3DRS_WRAP8";
	case D3DRS_WRAP9                         : return "D3DRS_WRAP9";
	case D3DRS_WRAP10                        : return "D3DRS_WRAP10";
	case D3DRS_WRAP11                        : return "D3DRS_WRAP11";
	case D3DRS_WRAP12                        : return "D3DRS_WRAP12";
	case D3DRS_WRAP13                        : return "D3DRS_WRAP13";
	case D3DRS_WRAP14                        : return "D3DRS_WRAP14";
	case D3DRS_WRAP15                        : return "D3DRS_WRAP15";
	case D3DRS_SEPARATEALPHABLENDENABLE      : return "D3DRS_SEPARATEALPHABLENDENABLE";
	case D3DRS_SRCBLENDALPHA                 : return "D3DRS_SRCBLENDALPHA";
	case D3DRS_DESTBLENDALPHA                : return "D3DRS_DESTBLENDALPHA";
	case D3DRS_BLENDOPALPHA                  : return "D3DRS_BLENDOPALPHA";
	default											  : return "UNKNOWN";
	}
}

const char* DX8Wrapper::Get_DX8_Texture_Stage_State_Name(D3DTEXTURESTAGESTATETYPE state)
{
	switch (state) {
	case D3DTSS_COLOROP                   : return "D3DTSS_COLOROP";
	case D3DTSS_COLORARG1                 : return "D3DTSS_COLORARG1";
	case D3DTSS_COLORARG2                 : return "D3DTSS_COLORARG2";
	case D3DTSS_ALPHAOP                   : return "D3DTSS_ALPHAOP";
	case D3DTSS_ALPHAARG1                 : return "D3DTSS_ALPHAARG1";
	case D3DTSS_ALPHAARG2                 : return "D3DTSS_ALPHAARG2";
	case D3DTSS_BUMPENVMAT00              : return "D3DTSS_BUMPENVMAT00";
	case D3DTSS_BUMPENVMAT01              : return "D3DTSS_BUMPENVMAT01";
	case D3DTSS_BUMPENVMAT10              : return "D3DTSS_BUMPENVMAT10";
	case D3DTSS_BUMPENVMAT11              : return "D3DTSS_BUMPENVMAT11";
	case D3DTSS_TEXCOORDINDEX             : return "D3DTSS_TEXCOORDINDEX";
	case D3DTSS_BUMPENVLSCALE             : return "D3DTSS_BUMPENVLSCALE";
	case D3DTSS_BUMPENVLOFFSET            : return "D3DTSS_BUMPENVLOFFSET";
	case D3DTSS_TEXTURETRANSFORMFLAGS     : return "D3DTSS_TEXTURETRANSFORMFLAGS";
	case D3DTSS_COLORARG0                 : return "D3DTSS_COLORARG0";
	case D3DTSS_ALPHAARG0                 : return "D3DTSS_ALPHAARG0";
	case D3DTSS_RESULTARG                 : return "D3DTSS_RESULTARG";
	case D3DTSS_CONSTANT                  : return "D3DTSS_CONSTANT";
	default										  : return "UNKNOWN";
	}
}

const char* DX8Wrapper::Get_DX8_Texture_Sampler_State_Name(D3DSAMPLERSTATETYPE state)
{
    switch (state) {
    case D3DSAMP_ADDRESSU:          return "D3DSAMP_ADDRESSU";
    case D3DSAMP_ADDRESSV:          return "D3DSAMP_ADDRESSV";
    case D3DSAMP_ADDRESSW:          return "D3DSAMP_ADDRESSW";
    case D3DSAMP_BORDERCOLOR:       return "D3DSAMP_BORDERCOLOR";
    case D3DSAMP_MAGFILTER:         return "D3DSAMP_MAGFILTER";
    case D3DSAMP_MINFILTER:         return "D3DSAMP_MINFILTER";
    case D3DSAMP_MIPFILTER:         return "D3DSAMP_MIPFILTER";
    case D3DSAMP_MIPMAPLODBIAS:     return "D3DSAMP_MIPMAPLODBIAS";
    case D3DSAMP_MAXMIPLEVEL:       return "D3DSAMP_MAXMIPLEVEL";
    case D3DSAMP_MAXANISOTROPY:     return "D3DSAMP_MAXANISOTROPY";
    case D3DSAMP_SRGBTEXTURE:       return "D3DSAMP_SRGBTEXTURE";
    case D3DSAMP_ELEMENTINDEX:      return "D3DSAMP_ELEMENTINDEX";
    case D3DSAMP_DMAPOFFSET:        return "D3DSAMP_DMAPOFFSET";
    default:                        return "UNKNOWN";
    }
}

void DX8Wrapper::Get_DX8_Render_State_Value_Name(StringClass& name, D3DRENDERSTATETYPE state, unsigned value)
{
	switch (state) {
	case D3DRS_ZENABLE:
		name=Get_DX8_ZBuffer_Type_Name(value);
		break;

	case D3DRS_FILLMODE:
		name=Get_DX8_Fill_Mode_Name(value);
		break;

	case D3DRS_SHADEMODE:
		name=Get_DX8_Shade_Mode_Name(value);
		break;

	case D3DRS_FOGCOLOR:
	case D3DRS_ALPHAREF:
	case D3DRS_STENCILMASK:
	case D3DRS_STENCILWRITEMASK:
	case D3DRS_TEXTUREFACTOR:
	case D3DRS_AMBIENT:
	case D3DRS_CLIPPLANEENABLE:
	case D3DRS_MULTISAMPLEMASK:
		name.Format("0x%x",value);
		break;

	case D3DRS_ZWRITEENABLE:
	case D3DRS_ALPHATESTENABLE:
	case D3DRS_LASTPIXEL:
	case D3DRS_DITHERENABLE:
	case D3DRS_ALPHABLENDENABLE:
	case D3DRS_FOGENABLE:
	case D3DRS_SPECULARENABLE:
	case D3DRS_STENCILENABLE:
	case D3DRS_RANGEFOGENABLE:
	case D3DRS_CLIPPING:
	case D3DRS_LIGHTING:
	case D3DRS_COLORVERTEX:
	case D3DRS_LOCALVIEWER:
	case D3DRS_NORMALIZENORMALS:
	case D3DRS_POINTSPRITEENABLE:
	case D3DRS_POINTSCALEENABLE:
	case D3DRS_MULTISAMPLEANTIALIAS:
	case D3DRS_INDEXEDVERTEXBLENDENABLE:
		name=value ? "true" : "false";
		break;

	case D3DRS_SRCBLEND:
	case D3DRS_DESTBLEND:
		name=Get_DX8_Blend_Name(value);
		break;

	case D3DRS_CULLMODE:
		name=Get_DX8_Cull_Mode_Name(value);
		break;

	case D3DRS_ZFUNC:
	case D3DRS_ALPHAFUNC:
	case D3DRS_STENCILFUNC:
		name=Get_DX8_Cmp_Func_Name(value);
		break;

	case D3DRS_FOGTABLEMODE:
	case D3DRS_FOGVERTEXMODE:
		name=Get_DX8_Fog_Mode_Name(value);
		break;

	case D3DRS_FOGSTART:
	case D3DRS_FOGEND:
	case D3DRS_FOGDENSITY:
	case D3DRS_POINTSIZE:
	case D3DRS_POINTSIZE_MIN:
	case D3DRS_POINTSCALE_A:
	case D3DRS_POINTSCALE_B:
	case D3DRS_POINTSCALE_C:
	case D3DRS_POINTSIZE_MAX:
	case D3DRS_TWEENFACTOR:
		name.Format("%f",*(float*)&value);
		break;

	case D3DRS_DEPTHBIAS:
	case D3DRS_STENCILREF:
		name.Format("%d",value);
		break;

	case D3DRS_STENCILFAIL:
	case D3DRS_STENCILZFAIL:
	case D3DRS_STENCILPASS:
		name=Get_DX8_Stencil_Op_Name(value);
		break;

	case D3DRS_WRAP0:
	case D3DRS_WRAP1:
	case D3DRS_WRAP2:
	case D3DRS_WRAP3:
	case D3DRS_WRAP4:
	case D3DRS_WRAP5:
	case D3DRS_WRAP6:
	case D3DRS_WRAP7:
		name="0";
		if (value&D3DWRAP_U) name+="|D3DWRAP_U";
		if (value&D3DWRAP_V) name+="|D3DWRAP_V";
		if (value&D3DWRAP_W) name+="|D3DWRAP_W";
		break;

	case D3DRS_DIFFUSEMATERIALSOURCE:
	case D3DRS_SPECULARMATERIALSOURCE:
	case D3DRS_AMBIENTMATERIALSOURCE:
	case D3DRS_EMISSIVEMATERIALSOURCE:
		name=Get_DX8_Material_Source_Name(value);
		break;

	case D3DRS_VERTEXBLEND:
		name=Get_DX8_Vertex_Blend_Flag_Name(value);
		break;

	case D3DRS_PATCHEDGESTYLE:
		name=Get_DX8_Patch_Edge_Style_Name(value);
		break;

	case D3DRS_DEBUGMONITORTOKEN:
		name=Get_DX8_Debug_Monitor_Token_Name(value);
		break;

	case D3DRS_COLORWRITEENABLE:
		name="0";
		if (value&D3DCOLORWRITEENABLE_RED) name+="|D3DCOLORWRITEENABLE_RED";
		if (value&D3DCOLORWRITEENABLE_GREEN) name+="|D3DCOLORWRITEENABLE_GREEN";
		if (value&D3DCOLORWRITEENABLE_BLUE) name+="|D3DCOLORWRITEENABLE_BLUE";
		if (value&D3DCOLORWRITEENABLE_ALPHA) name+="|D3DCOLORWRITEENABLE_ALPHA";
		break;
	case D3DRS_BLENDOP:
		name=Get_DX8_Blend_Op_Name(value);
		break;
	default:
		name.Format("UNKNOWN (%d)",value);
		break;
	}
}

void DX8Wrapper::Get_DX8_Texture_Stage_State_Value_Name(StringClass& name, D3DTEXTURESTAGESTATETYPE state, unsigned value)
{
	switch (state) {
	case D3DTSS_COLOROP:
	case D3DTSS_ALPHAOP:
		name=Get_DX8_Texture_Op_Name(value);
		break;

	case D3DTSS_COLORARG0:
	case D3DTSS_COLORARG1:
	case D3DTSS_COLORARG2:
	case D3DTSS_ALPHAARG0:
	case D3DTSS_ALPHAARG1:
	case D3DTSS_ALPHAARG2:
	case D3DTSS_RESULTARG:
		name=Get_DX8_Texture_Arg_Name(value);
		break;

	// Floating point values
	case D3DTSS_BUMPENVMAT00:
	case D3DTSS_BUMPENVMAT01:
	case D3DTSS_BUMPENVMAT10:
	case D3DTSS_BUMPENVMAT11:
	case D3DTSS_BUMPENVLSCALE:
	case D3DTSS_BUMPENVLOFFSET:
		name.Format("%f",*(float*)&value);
		break;

	case D3DTSS_TEXCOORDINDEX:
		if ((value&0xffff0000)==D3DTSS_TCI_CAMERASPACENORMAL) {
			name.Format("D3DTSS_TCI_CAMERASPACENORMAL|%d",value&0xffff);
		}
		else if ((value&0xffff0000)==D3DTSS_TCI_CAMERASPACEPOSITION) {
			name.Format("D3DTSS_TCI_CAMERASPACEPOSITION|%d",value&0xffff);
		}
		else if ((value&0xffff0000)==D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR) {
			name.Format("D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR|%d",value&0xffff);
		}
		else {
			name.Format("%d",value);
		}
		break;

	default:
		name.Format("UNKNOWN (%d)",value);
		break;
	}
}

void DX8Wrapper::Get_DX8_Texture_Sampler_State_Value_Name(StringClass& name, D3DSAMPLERSTATETYPE state, unsigned value)
{
	switch (state) {
	case D3DSAMP_ADDRESSU:
	case D3DSAMP_ADDRESSV:
	case D3DSAMP_ADDRESSW:
		name=Get_DX8_Texture_Address_Name((D3DTEXTUREADDRESS)value);
		break;
	case D3DSAMP_BORDERCOLOR:
		name.Format("%x",value);
		break;
	case D3DSAMP_MAGFILTER:
	case D3DSAMP_MINFILTER:
	case D3DSAMP_MIPFILTER:
		name=Get_DX8_Texture_Filter_Name((D3DTEXTUREFILTERTYPE)value);
		break;
	case D3DSAMP_MIPMAPLODBIAS:
		name.Format("%f",*(float*)&value);
		break;
	case D3DSAMP_MAXMIPLEVEL:
	case D3DSAMP_MAXANISOTROPY:
	case D3DSAMP_SRGBTEXTURE:
	case D3DSAMP_ELEMENTINDEX:
		name.Format("%d",value);
		break;
	default:
		name.Format("UNKNOWN (%d)",value);
		break;
	}
}

const char* DX8Wrapper::Get_DX8_Texture_Op_Name(unsigned value)
{
	switch (value) {
	case D3DTOP_DISABLE                      : return "D3DTOP_DISABLE";
	case D3DTOP_SELECTARG1                   : return "D3DTOP_SELECTARG1";
	case D3DTOP_SELECTARG2                   : return "D3DTOP_SELECTARG2";
	case D3DTOP_MODULATE                     : return "D3DTOP_MODULATE";
	case D3DTOP_MODULATE2X                   : return "D3DTOP_MODULATE2X";
	case D3DTOP_MODULATE4X                   : return "D3DTOP_MODULATE4X";
	case D3DTOP_ADD                          : return "D3DTOP_ADD";
	case D3DTOP_ADDSIGNED                    : return "D3DTOP_ADDSIGNED";
	case D3DTOP_ADDSIGNED2X                  : return "D3DTOP_ADDSIGNED2X";
	case D3DTOP_SUBTRACT                     : return "D3DTOP_SUBTRACT";
	case D3DTOP_ADDSMOOTH                    : return "D3DTOP_ADDSMOOTH";
	case D3DTOP_BLENDDIFFUSEALPHA            : return "D3DTOP_BLENDDIFFUSEALPHA";
	case D3DTOP_BLENDTEXTUREALPHA            : return "D3DTOP_BLENDTEXTUREALPHA";
	case D3DTOP_BLENDFACTORALPHA             : return "D3DTOP_BLENDFACTORALPHA";
	case D3DTOP_BLENDTEXTUREALPHAPM          : return "D3DTOP_BLENDTEXTUREALPHAPM";
	case D3DTOP_BLENDCURRENTALPHA            : return "D3DTOP_BLENDCURRENTALPHA";
	case D3DTOP_PREMODULATE                  : return "D3DTOP_PREMODULATE";
	case D3DTOP_MODULATEALPHA_ADDCOLOR       : return "D3DTOP_MODULATEALPHA_ADDCOLOR";
	case D3DTOP_MODULATECOLOR_ADDALPHA       : return "D3DTOP_MODULATECOLOR_ADDALPHA";
	case D3DTOP_MODULATEINVALPHA_ADDCOLOR    : return "D3DTOP_MODULATEINVALPHA_ADDCOLOR";
	case D3DTOP_MODULATEINVCOLOR_ADDALPHA    : return "D3DTOP_MODULATEINVCOLOR_ADDALPHA";
	case D3DTOP_BUMPENVMAP                   : return "D3DTOP_BUMPENVMAP";
	case D3DTOP_BUMPENVMAPLUMINANCE          : return "D3DTOP_BUMPENVMAPLUMINANCE";
	case D3DTOP_DOTPRODUCT3                  : return "D3DTOP_DOTPRODUCT3";
	case D3DTOP_MULTIPLYADD                  : return "D3DTOP_MULTIPLYADD";
	case D3DTOP_LERP                         : return "D3DTOP_LERP";
	default										     : return "UNKNOWN";
	}
}

const char* DX8Wrapper::Get_DX8_Texture_Arg_Name(unsigned value)
{
	switch (value) {
	case D3DTA_CURRENT			: return "D3DTA_CURRENT";
	case D3DTA_DIFFUSE			: return "D3DTA_DIFFUSE";
	case D3DTA_SELECTMASK		: return "D3DTA_SELECTMASK";
	case D3DTA_SPECULAR			: return "D3DTA_SPECULAR";
	case D3DTA_TEMP				: return "D3DTA_TEMP";
	case D3DTA_TEXTURE			: return "D3DTA_TEXTURE";
	case D3DTA_TFACTOR			: return "D3DTA_TFACTOR";
	case D3DTA_ALPHAREPLICATE	: return "D3DTA_ALPHAREPLICATE";
	case D3DTA_COMPLEMENT		: return "D3DTA_COMPLEMENT";
	default					      : return "UNKNOWN";
	}
}

const char* DX8Wrapper::Get_DX8_Texture_Filter_Name(unsigned value)
{
	switch (value) {
	case D3DTEXF_NONE				: return "D3DTEXF_NONE";
	case D3DTEXF_POINT			: return "D3DTEXF_POINT";
	case D3DTEXF_LINEAR			: return "D3DTEXF_LINEAR";
	case D3DTEXF_ANISOTROPIC	: return "D3DTEXF_ANISOTROPIC";
	case D3DTEXF_PYRAMIDALQUAD	: return "D3DTEXF_PYRAMIDALQUAD";
	case D3DTEXF_GAUSSIANQUAD	: return "D3DTEXF_GAUSSIANQUAD";
	default					      : return "UNKNOWN";
	}
}

const char* DX8Wrapper::Get_DX8_Texture_Address_Name(unsigned value)
{
	switch (value) {
	case D3DTADDRESS_WRAP		: return "D3DTADDRESS_WRAP";
	case D3DTADDRESS_MIRROR		: return "D3DTADDRESS_MIRROR";
	case D3DTADDRESS_CLAMP		: return "D3DTADDRESS_CLAMP";
	case D3DTADDRESS_BORDER		: return "D3DTADDRESS_BORDER";
	case D3DTADDRESS_MIRRORONCE: return "D3DTADDRESS_MIRRORONCE";
	default					      : return "UNKNOWN";
	}
}

const char* DX8Wrapper::Get_DX8_Texture_Transform_Flag_Name(unsigned value)
{
	switch (value) {
	case D3DTTFF_DISABLE			: return "D3DTTFF_DISABLE";
	case D3DTTFF_COUNT1			: return "D3DTTFF_COUNT1";
	case D3DTTFF_COUNT2			: return "D3DTTFF_COUNT2";
	case D3DTTFF_COUNT3			: return "D3DTTFF_COUNT3";
	case D3DTTFF_COUNT4			: return "D3DTTFF_COUNT4";
	case D3DTTFF_PROJECTED		: return "D3DTTFF_PROJECTED";
	default					      : return "UNKNOWN";
	}
}

const char* DX8Wrapper::Get_DX8_ZBuffer_Type_Name(unsigned value)
{
	switch (value) {
	case D3DZB_FALSE				: return "D3DZB_FALSE";
	case D3DZB_TRUE				: return "D3DZB_TRUE";
	case D3DZB_USEW				: return "D3DZB_USEW";
	default					      : return "UNKNOWN";
	}
}

const char* DX8Wrapper::Get_DX8_Fill_Mode_Name(unsigned value)
{
	switch (value) {
	case D3DFILL_POINT			: return "D3DFILL_POINT";
	case D3DFILL_WIREFRAME		: return "D3DFILL_WIREFRAME";
	case D3DFILL_SOLID			: return "D3DFILL_SOLID";
	default					      : return "UNKNOWN";
	}
}

const char* DX8Wrapper::Get_DX8_Shade_Mode_Name(unsigned value)
{
	switch (value) {
	case D3DSHADE_FLAT			: return "D3DSHADE_FLAT";
	case D3DSHADE_GOURAUD		: return "D3DSHADE_GOURAUD";
	case D3DSHADE_PHONG			: return "D3DSHADE_PHONG";
	default							: return "UNKNOWN";
	}
}

const char* DX8Wrapper::Get_DX8_Blend_Name(unsigned value)
{
	switch (value) {
	case D3DBLEND_ZERO                : return "D3DBLEND_ZERO";
	case D3DBLEND_ONE                 : return "D3DBLEND_ONE";
	case D3DBLEND_SRCCOLOR            : return "D3DBLEND_SRCCOLOR";
	case D3DBLEND_INVSRCCOLOR         : return "D3DBLEND_INVSRCCOLOR";
	case D3DBLEND_SRCALPHA            : return "D3DBLEND_SRCALPHA";
	case D3DBLEND_INVSRCALPHA         : return "D3DBLEND_INVSRCALPHA";
	case D3DBLEND_DESTALPHA           : return "D3DBLEND_DESTALPHA";
	case D3DBLEND_INVDESTALPHA        : return "D3DBLEND_INVDESTALPHA";
	case D3DBLEND_DESTCOLOR           : return "D3DBLEND_DESTCOLOR";
	case D3DBLEND_INVDESTCOLOR        : return "D3DBLEND_INVDESTCOLOR";
	case D3DBLEND_SRCALPHASAT         : return "D3DBLEND_SRCALPHASAT";
	case D3DBLEND_BOTHSRCALPHA        : return "D3DBLEND_BOTHSRCALPHA";
	case D3DBLEND_BOTHINVSRCALPHA     : return "D3DBLEND_BOTHINVSRCALPHA";
	default									 : return "UNKNOWN";
	}
}

const char* DX8Wrapper::Get_DX8_Cull_Mode_Name(unsigned value)
{
	switch (value) {
	case D3DCULL_NONE				: return "D3DCULL_NONE";
	case D3DCULL_CW				: return "D3DCULL_CW";
	case D3DCULL_CCW				: return "D3DCULL_CCW";
	default							: return "UNKNOWN";
	}
}

const char* DX8Wrapper::Get_DX8_Cmp_Func_Name(unsigned value)
{
	switch (value) {
	case D3DCMP_NEVER          : return "D3DCMP_NEVER";
	case D3DCMP_LESS           : return "D3DCMP_LESS";
	case D3DCMP_EQUAL          : return "D3DCMP_EQUAL";
	case D3DCMP_LESSEQUAL      : return "D3DCMP_LESSEQUAL";
	case D3DCMP_GREATER        : return "D3DCMP_GREATER";
	case D3DCMP_NOTEQUAL       : return "D3DCMP_NOTEQUAL";
	case D3DCMP_GREATEREQUAL   : return "D3DCMP_GREATEREQUAL";
	case D3DCMP_ALWAYS         : return "D3DCMP_ALWAYS";
	default							: return "UNKNOWN";
	}
}

const char* DX8Wrapper::Get_DX8_Fog_Mode_Name(unsigned value)
{
	switch (value) {
	case D3DFOG_NONE				: return "D3DFOG_NONE";
	case D3DFOG_EXP				: return "D3DFOG_EXP";
	case D3DFOG_EXP2				: return "D3DFOG_EXP2";
	case D3DFOG_LINEAR			: return "D3DFOG_LINEAR";
	default							: return "UNKNOWN";
	}
}

const char* DX8Wrapper::Get_DX8_Stencil_Op_Name(unsigned value)
{
	switch (value) {
	case D3DSTENCILOP_KEEP		: return "D3DSTENCILOP_KEEP";
	case D3DSTENCILOP_ZERO		: return "D3DSTENCILOP_ZERO";
	case D3DSTENCILOP_REPLACE	: return "D3DSTENCILOP_REPLACE";
	case D3DSTENCILOP_INCRSAT	: return "D3DSTENCILOP_INCRSAT";
	case D3DSTENCILOP_DECRSAT	: return "D3DSTENCILOP_DECRSAT";
	case D3DSTENCILOP_INVERT	: return "D3DSTENCILOP_INVERT";
	case D3DSTENCILOP_INCR		: return "D3DSTENCILOP_INCR";
	case D3DSTENCILOP_DECR		: return "D3DSTENCILOP_DECR";
	default							: return "UNKNOWN";
	}
}

const char* DX8Wrapper::Get_DX8_Material_Source_Name(unsigned value)
{
	switch (value) {
	case D3DMCS_MATERIAL			: return "D3DMCS_MATERIAL";
	case D3DMCS_COLOR1			: return "D3DMCS_COLOR1";
	case D3DMCS_COLOR2			: return "D3DMCS_COLOR2";
	default							: return "UNKNOWN";
	}
}

const char* DX8Wrapper::Get_DX8_Vertex_Blend_Flag_Name(unsigned value)
{
	switch (value) {
	case D3DVBF_DISABLE			: return "D3DVBF_DISABLE";
	case D3DVBF_1WEIGHTS			: return "D3DVBF_1WEIGHTS";
	case D3DVBF_2WEIGHTS			: return "D3DVBF_2WEIGHTS";
	case D3DVBF_3WEIGHTS			: return "D3DVBF_3WEIGHTS";
	case D3DVBF_TWEENING			: return "D3DVBF_TWEENING";
	case D3DVBF_0WEIGHTS			: return "D3DVBF_0WEIGHTS";
	default							: return "UNKNOWN";
	}
}

const char* DX8Wrapper::Get_DX8_Patch_Edge_Style_Name(unsigned value)
{
	switch (value) {
	case D3DPATCHEDGE_DISCRETE	: return "D3DPATCHEDGE_DISCRETE";
	case D3DPATCHEDGE_CONTINUOUS:return "D3DPATCHEDGE_CONTINUOUS";
	default							: return "UNKNOWN";
	}
}

const char* DX8Wrapper::Get_DX8_Debug_Monitor_Token_Name(unsigned value)
{
	switch (value) {
	case D3DDMT_ENABLE			: return "D3DDMT_ENABLE";
	case D3DDMT_DISABLE			: return "D3DDMT_DISABLE";
	default							: return "UNKNOWN";
	}
}

const char* DX8Wrapper::Get_DX8_Blend_Op_Name(unsigned value)
{
	switch (value) {
	case D3DBLENDOP_ADD			: return "D3DBLENDOP_ADD";
	case D3DBLENDOP_SUBTRACT	: return "D3DBLENDOP_SUBTRACT";
	case D3DBLENDOP_REVSUBTRACT: return "D3DBLENDOP_REVSUBTRACT";
	case D3DBLENDOP_MIN			: return "D3DBLENDOP_MIN";
	case D3DBLENDOP_MAX			: return "D3DBLENDOP_MAX";
	default							: return "UNKNOWN";
	}
}

void DX8Wrapper::Set_DX8_ZBias(int zbias)
{
	if (zbias==ZBias) return;
	if (zbias>15) zbias=15;
	if (zbias<0) zbias=0;
	ZBias=zbias;

	if (!Get_Current_Caps()->Support_ZBias() && ZNear!=ZFar) {
		Matrix4 tmp=ProjectionMatrix;
		float tmp_zbias=ZBias;
		tmp_zbias*=(1.0f/16.0f);
		tmp_zbias*=1.0f / (ZFar - ZNear);
		tmp[2][2]-=tmp_zbias*tmp[3][2];
		DX8CALL(SetTransform(D3DTS_PROJECTION,(D3DMATRIX*)&tmp));
	}
	else {
		//float ZBias_float = zbias / 8.0f;
		Set_DX8_Render_State (D3DRS_DEPTHBIAS, ZBias);
		Set_DX8_Render_State (D3DRS_SLOPESCALEDEPTHBIAS, ZBias);
	}
}

void DX8Wrapper::Set_DX8_Render_State(D3DRENDERSTATETYPE state, unsigned value)
{
	// Can't monitor state changes because setShader call to GERD may change the states!
	if (RenderStates[state]==value) return;

#ifdef MESH_RENDER_SNAPSHOT_ENABLED
	if (WW3D::Is_Snapshot_Activated()) {
		StringClass value_name(0,true);
		Get_DX8_Render_State_Value_Name(value_name,state,value);
		SNAPSHOT_SAY(("DX8 - SetRenderState(state: %s, value: %s)\n",
			Get_DX8_Render_State_Name(state),
			value_name));
	}
#endif

	RenderStates[state]=value;
	DX8CALL(SetRenderState( state, value ));
	DX8_RECORD_RENDER_STATE_CHANGE();
}
