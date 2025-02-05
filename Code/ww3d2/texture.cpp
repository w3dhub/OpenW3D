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
 *                     $Archive:: /Commando/Code/ww3d2/texture.cpp                            $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 1/09/02 2:57p                                               $*
 *                                                                                             *
 *                    $Revision:: 83                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   FileListTextureClass::Load_Frame_Surface -- Load source texture                           *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "texture.h"

#include <d3d8.h>
#include <stdio.h>
#include <D3dx8core.h>
#include "dx8wrapper.h"
#include "targa.h"
#include <nstrdup.h>
#include "w3d_file.h"
#include "assetmgr.h"
#include "formconv.h"
#include "textureloader.h"
#include "missingtexture.h"
#include "ffactory.h"
#include "dx8caps.h"
#include "dx8texman.h"
#include "meshmatdesc.h"
#include "texturethumbnail.h"

const unsigned DEFAULT_INACTIVATION_TIME=20000;

/*
** Definitions of static members:
*/

static unsigned unused_texture_id;

unsigned _MinTextureFilters[MAX_TEXTURE_STAGES][TextureClass::FILTER_TYPE_COUNT];
unsigned _MagTextureFilters[MAX_TEXTURE_STAGES][TextureClass::FILTER_TYPE_COUNT];
unsigned _MipMapFilters[MAX_TEXTURE_STAGES][TextureClass::FILTER_TYPE_COUNT];

// ----------------------------------------------------------------------------

static int Calculate_Texture_Memory_Usage(const TextureClass* texture,int red_factor=0)
{
	// Set performance statistics

	int size=0;
	IDirect3DTexture8* d3d_texture=const_cast<TextureClass*>(texture)->Peek_DX8_Texture();
	if (!d3d_texture) return 0;
	for (unsigned i=red_factor;i<d3d_texture->GetLevelCount();++i) {
		D3DSURFACE_DESC desc;
		DX8_ErrorCode(d3d_texture->GetLevelDesc(i,&desc));
		size+=desc.Size;
	}
	return size;
}

/*************************************************************************
**                             TextureClass
*************************************************************************/

TextureClass::TextureClass(unsigned width, unsigned height, WW3DFormat format, MipCountType mip_level_count, PoolType pool,bool rendertarget)
	:
	D3DTexture(NULL),
	texture_id(unused_texture_id++),
	Initialized(true),
	TextureMinFilter(FILTER_TYPE_DEFAULT),
	TextureMagFilter(FILTER_TYPE_DEFAULT),
	MipMapFilter((mip_level_count!=MIP_LEVELS_1) ? FILTER_TYPE_DEFAULT : FILTER_TYPE_NONE),
	UAddressMode(TEXTURE_ADDRESS_REPEAT),
	VAddressMode(TEXTURE_ADDRESS_REPEAT),
	MipLevelCount(mip_level_count),
	Pool(pool),
	Dirty(false),
	IsLightmap(false),
	IsProcedural(true),
	Name(""),
	TextureFormat(format),
	IsCompressionAllowed(false),
	TextureLoadTask(NULL),
	ThumbnailLoadTask(NULL),
	Width(width),
	Height(height),
	InactivationTime(0),		// Don't inactivate!
	ExtendedInactivationTime(0),
	LastInactivationSyncTime(0)
{
	switch (format) {
	case WW3D_FORMAT_DXT1:
	case WW3D_FORMAT_DXT2:
	case WW3D_FORMAT_DXT3:
	case WW3D_FORMAT_DXT4:
	case WW3D_FORMAT_DXT5:
		IsCompressionAllowed=true;
		break;
	default:
		break;
	}
	D3DPOOL d3dpool=(D3DPOOL) 0;
	switch(pool)
	{
	case POOL_DEFAULT:
		d3dpool=D3DPOOL_DEFAULT;
		break;
	case POOL_MANAGED:
		d3dpool=D3DPOOL_MANAGED;
		break;
	case POOL_SYSTEMMEM:
		d3dpool=D3DPOOL_SYSTEMMEM;
		break;
	default:
		WWASSERT(0);
	}
	D3DTexture = DX8Wrapper::_Create_DX8_Texture(width, height, format, mip_level_count,d3dpool,rendertarget);
	if (pool==POOL_DEFAULT)
	{
		Dirty=true;
		DX8TextureTrackerClass *track=new
		DX8TextureTrackerClass(width, height, format, mip_level_count,rendertarget,
		this);
		DX8TextureManagerClass::Add(track);
	}
	LastAccessed=WW3D::Get_Sync_Time();
}

// ----------------------------------------------------------------------------

TextureClass::TextureClass(
	const char *name,
	const char *full_path,
	MipCountType mip_level_count,
	WW3DFormat texture_format,
	bool allow_compression)
	:
	D3DTexture(NULL),
	texture_id(unused_texture_id++),
	Initialized(false),
	TextureMinFilter(FILTER_TYPE_DEFAULT),
	TextureMagFilter(FILTER_TYPE_DEFAULT),
	MipMapFilter((mip_level_count!=MIP_LEVELS_1) ? FILTER_TYPE_DEFAULT : FILTER_TYPE_NONE),
	UAddressMode(TEXTURE_ADDRESS_REPEAT),
	VAddressMode(TEXTURE_ADDRESS_REPEAT),
	MipLevelCount(mip_level_count),
	Pool(POOL_MANAGED),
	Dirty(false),
	IsLightmap(false),
	IsProcedural(false),
	TextureFormat(texture_format),
	IsCompressionAllowed(allow_compression),
	TextureLoadTask(NULL),
	ThumbnailLoadTask(NULL),
	Width(0),
	Height(0),
	InactivationTime(DEFAULT_INACTIVATION_TIME),		// Default inactivation time 30 seconds
	ExtendedInactivationTime(0),
	LastInactivationSyncTime(0)
{
	switch (TextureFormat) {
	case WW3D_FORMAT_DXT1:
	case WW3D_FORMAT_DXT2:
	case WW3D_FORMAT_DXT3:
	case WW3D_FORMAT_DXT4:
	case WW3D_FORMAT_DXT5:
		IsCompressionAllowed=true;
		break;
	case WW3D_FORMAT_U8V8:		// Bumpmap
	case WW3D_FORMAT_L6V5U5:	// Bumpmap
	case WW3D_FORMAT_X8L8V8U8:	// Bumpmap
		// If requesting bumpmap format that isn't available we'll just return the surface in whatever color
		// format the texture file is in. (This is illegal case, the format support should always be queried
		// before creating a bump texture!)
		if (!DX8Wrapper::Is_Initted() || !DX8Wrapper::Get_Current_Caps()->Support_Texture_Format(TextureFormat)) {
			TextureFormat=WW3D_FORMAT_UNKNOWN;
		}
		// If bump format is valid, make sure compression is not allowed so that we don't even attempt to load
		// from a compressed file (quality isn't good enough for bump map). Also disable mipmapping.
		else {
			IsCompressionAllowed=false;
			MipLevelCount=MIP_LEVELS_1;
			MipMapFilter=FILTER_TYPE_NONE;
		}
		break;

	default:
		break;
	}

	WWASSERT_PRINT(name && name[0], "TextureClass CTor: NULL or empty texture name\n");
	int len=strlen(name);
	for (int i=0;i<len;++i) {
		if (name[i]=='+') {
			IsLightmap=true;

			// Set bilinear filtering for lightmaps (they are very stretched and
			// low detail so we don't care for anisotropic or trilinear filtering...)
			TextureMinFilter=FILTER_TYPE_FAST;
			TextureMagFilter=FILTER_TYPE_FAST;
			if (mip_level_count!=MIP_LEVELS_1) MipMapFilter=FILTER_TYPE_FAST;
			break;
		}
	}
	Set_Texture_Name(name);
	Set_Full_Path(full_path);
	WWASSERT(name[0]!='\0');
	if (!WW3D::Is_Texturing_Enabled()) {
		Initialized=true;
		D3DTexture=0;
	}

	// Find original size from the thumbnail (but don't create thumbnail texture yet!)
	ThumbnailClass* thumb=NULL;
	ThumbnailManagerClass* thumb_man=ThumbnailManagerClass::Peek_List().Head();
	while (thumb_man) {
		thumb=thumb_man->Peek_Thumbnail_Instance(Get_Full_Path());
		if (thumb) {
			Width=thumb->Get_Original_Texture_Width();
			Height=thumb->Get_Original_Texture_Height();
			break;
		}
		thumb_man=thumb_man->Succ();
	}

	LastAccessed=WW3D::Get_Sync_Time();

	// If the thumbnails are not enabled, init the texture at this point to avoid stalling when the
	// mesh is rendered.
	if (!WW3D::Get_Thumbnail_Enabled()) {
		if (TextureLoader::Is_DX8_Thread()) {
			Init();
		}
	}
}

// ----------------------------------------------------------------------------

TextureClass::TextureClass(SurfaceClass *surface, MipCountType mip_level_count)
	:
	D3DTexture(NULL),
	texture_id(unused_texture_id++),
	Initialized(true),
	TextureMinFilter(FILTER_TYPE_DEFAULT),
	TextureMagFilter(FILTER_TYPE_DEFAULT),
	MipMapFilter((mip_level_count!=MIP_LEVELS_1) ? FILTER_TYPE_DEFAULT : FILTER_TYPE_NONE),
	UAddressMode(TEXTURE_ADDRESS_REPEAT),
	VAddressMode(TEXTURE_ADDRESS_REPEAT),
	MipLevelCount(mip_level_count),
	Pool(POOL_MANAGED),
	Dirty(false),
	IsLightmap(false),
	Name(""),
	IsProcedural(true),
	TextureFormat(surface->Get_Surface_Format()),
	IsCompressionAllowed(false),
	TextureLoadTask(NULL),
	ThumbnailLoadTask(NULL),
	Width(0),
	Height(0),
	InactivationTime(0),		// Don't inactivate
	ExtendedInactivationTime(0),
	LastInactivationSyncTime(0)
{
	SurfaceClass::SurfaceDescription sd;
	surface->Get_Description(sd);
	Width=sd.Width;
	Height=sd.Height;
	switch (sd.Format) {
	case WW3D_FORMAT_DXT1:
	case WW3D_FORMAT_DXT2:
	case WW3D_FORMAT_DXT3:
	case WW3D_FORMAT_DXT4:
	case WW3D_FORMAT_DXT5:
		IsCompressionAllowed=true;
		break;
	default:
		break;
	}

	D3DTexture = DX8Wrapper::_Create_DX8_Texture(surface->Peek_D3D_Surface(), mip_level_count);
	LastAccessed=WW3D::Get_Sync_Time();
}

// ----------------------------------------------------------------------------

TextureClass::TextureClass(IDirect3DTexture8* d3d_texture)
	:
	D3DTexture(d3d_texture),
	texture_id(unused_texture_id++),
	Initialized(true),
	TextureMinFilter(FILTER_TYPE_DEFAULT),
	TextureMagFilter(FILTER_TYPE_DEFAULT),
	MipMapFilter((d3d_texture->GetLevelCount()!=1) ? FILTER_TYPE_DEFAULT : FILTER_TYPE_NONE),
	UAddressMode(TEXTURE_ADDRESS_REPEAT),
	VAddressMode(TEXTURE_ADDRESS_REPEAT),
	MipLevelCount((MipCountType)d3d_texture->GetLevelCount()),
	Pool(POOL_MANAGED),
	Dirty(false),
	IsLightmap(false),
	Name(""),
	IsProcedural(true),
	IsCompressionAllowed(false),
	TextureLoadTask(NULL),
	ThumbnailLoadTask(NULL),
	Width(0),
	Height(0),
	InactivationTime(0),	// Don't inactivate!
	ExtendedInactivationTime(0),
	LastInactivationSyncTime(0)
{
	D3DTexture->AddRef();
	IDirect3DSurface8* surface;
	DX8_ErrorCode(D3DTexture->GetSurfaceLevel(0,&surface));
	D3DSURFACE_DESC d3d_desc;
	::ZeroMemory(&d3d_desc, sizeof(D3DSURFACE_DESC));
	DX8_ErrorCode(surface->GetDesc(&d3d_desc));
	Width=d3d_desc.Width;
	Height=d3d_desc.Height;
	TextureFormat=D3DFormat_To_WW3DFormat(d3d_desc.Format);
	switch (TextureFormat) {
	case WW3D_FORMAT_DXT1:
	case WW3D_FORMAT_DXT2:
	case WW3D_FORMAT_DXT3:
	case WW3D_FORMAT_DXT4:
	case WW3D_FORMAT_DXT5:
		IsCompressionAllowed=true;
		break;
	default:
		break;
	}

	LastAccessed=WW3D::Get_Sync_Time();
}

// ----------------------------------------------------------------------------

TextureClass::~TextureClass(void)
{
	delete TextureLoadTask;
	TextureLoadTask=NULL;
	delete ThumbnailLoadTask;
	ThumbnailLoadTask=NULL;

	if (D3DTexture) {
		D3DTexture->Release();
		D3DTexture = NULL;
	}
	DX8TextureManagerClass::Remove(this);
}

void TextureClass::Invalidate_Old_Unused_Textures(unsigned invalidation_time_override)
{
	unsigned synctime=WW3D::Get_Sync_Time();
	HashTemplateIterator<StringClass,TextureClass*> ite(WW3DAssetManager::Get_Instance()->Texture_Hash());
	// Loop through all the textures in the manager

	for (ite.First ();!ite.Is_Done();ite.Next ()) {
		TextureClass* tex=ite.Peek_Value();

		// Consider invalidating if texture has been initialized and defines inactivation time
		if (tex->Initialized && tex->InactivationTime) {
			unsigned age=synctime-tex->LastAccessed;

			if (invalidation_time_override) {
				if (age>invalidation_time_override) {
					tex->Invalidate();
					tex->LastInactivationSyncTime=synctime;
				}
			}
			else {
				// Not used in the last n milliseconds?
				if (age>(tex->InactivationTime+tex->ExtendedInactivationTime)) {
					tex->Invalidate();
					tex->LastInactivationSyncTime=synctime;
				}
			}
		}
	}
}


// ----------------------------------------------------------------------------

void TextureClass::Init()
{
	// If the texture has already been initialised we should exit now
	if (Initialized) return;

	// If the texture has recently been inactivated, increase the inactivation time (this texture obviously
	// should not have been inactivated yet).

	if (InactivationTime && LastInactivationSyncTime) {
		if ((WW3D::Get_Sync_Time()-LastInactivationSyncTime)<InactivationTime) {
			ExtendedInactivationTime=3*InactivationTime;
		}
		LastInactivationSyncTime=0;
	}


	if (!D3DTexture) {
		if (!WW3D::Get_Thumbnail_Enabled() || MipLevelCount==MIP_LEVELS_1) {
//		if (MipLevelCount==MIP_LEVELS_1) {
			TextureLoader::Request_Foreground_Loading(this);
		}
		else {
			WW3DFormat format=TextureFormat;
			Load_Locked_Surface();
			TextureFormat=format;
		}
	}

	if (!Initialized) {
		TextureLoader::Request_Background_Loading(this);
	}

	LastAccessed=WW3D::Get_Sync_Time();
}

void TextureClass::Invalidate()
{
	if (TextureLoadTask) {
		return;
	}
	if (ThumbnailLoadTask) {
		return;
	}

	// Don't invalidate procedural textures
	if (IsProcedural) {
		return;
	}

	if (D3DTexture) {
		D3DTexture->Release();
		D3DTexture = NULL;
	}

	Initialized=false;

	LastAccessed=WW3D::Get_Sync_Time();
}

// ----------------------------------------------------------------------------

void TextureClass::Load_Locked_Surface()
{
	if (D3DTexture) D3DTexture->Release();
	D3DTexture=0;
	TextureLoader::Request_Thumbnail(this);
	Initialized=false;
}

// ----------------------------------------------------------------------------

bool TextureClass::Is_Missing_Texture()
{
	bool flag = false;
	IDirect3DTexture8 *missing_texture = MissingTexture::_Get_Missing_Texture();

	if(D3DTexture == missing_texture)
		flag = true;

	if(missing_texture) {
		missing_texture->Release();
	}

	return flag;
}

// ----------------------------------------------------------------------------

void TextureClass::Set_Texture_Name(const char * name)
{
	Name=name;
}

// ----------------------------------------------------------------------------

unsigned int TextureClass::Get_Mip_Level_Count(void)
{
	if (!D3DTexture) {
		WWASSERT_PRINT(0, "Get_Mip_Level_Count: D3DTexture is NULL!\n");
		return 0;
	}

	return D3DTexture->GetLevelCount();
}

// ----------------------------------------------------------------------------

SurfaceClass *TextureClass::Get_Surface_Level(unsigned int level)
{
	if (!D3DTexture) {
		WWASSERT_PRINT(0, "Get_Surface_Level: D3DTexture is NULL!\n");
		return 0;
	}

	IDirect3DSurface8 *d3d_surface = NULL;
	DX8_ErrorCode(D3DTexture->GetSurfaceLevel(level, &d3d_surface));
	SurfaceClass *surface = new SurfaceClass(d3d_surface);
	d3d_surface->Release();
	return surface;
}

// ----------------------------------------------------------------------------

IDirect3DSurface8 *TextureClass::Get_D3D_Surface_Level(unsigned int level)
{
	if (!D3DTexture) {
		WWASSERT_PRINT(0, "Get_D3D_Surface_Level: D3DTexture is NULL!\n");
		return 0;
	}

	IDirect3DSurface8 *d3d_surface = NULL;
	DX8_ErrorCode(D3DTexture->GetSurfaceLevel(level, &d3d_surface));
	return d3d_surface;
}

// ----------------------------------------------------------------------------

unsigned int TextureClass::Get_Priority(void)
{
	if (!D3DTexture) {
		WWASSERT_PRINT(0, "Get_Priority: D3DTexture is NULL!\n");
		return 0;
	}

	return D3DTexture->GetPriority();
}

// ----------------------------------------------------------------------------

unsigned int TextureClass::Set_Priority(unsigned int priority)
{
	if (!D3DTexture) {
		WWASSERT_PRINT(0, "Set_Priority: D3DTexture is NULL!\n");
		return 0;
	}

	return D3DTexture->SetPriority(priority);
}

// ----------------------------------------------------------------------------

void TextureClass::Set_Mip_Mapping(FilterType mipmap)
{
	if (mipmap != FILTER_TYPE_NONE && Get_Mip_Level_Count() <= 1) {
		WWASSERT_PRINT(0, "Trying to enable MipMapping on texture w/o Mip levels!\n");
		return;
	}
	MipMapFilter=mipmap;
}

unsigned TextureClass::Get_Reduction() const
{
	if (MipLevelCount==MIP_LEVELS_1) return 0;

	int reduction=WW3D::Get_Texture_Reduction();
	if (MipLevelCount && reduction>MipLevelCount) {
		reduction=MipLevelCount;
	}
	return reduction;
}

// ----------------------------------------------------------------------------

void TextureClass::Apply(unsigned int stage)
{
	if (!Initialized) {
		Init();
	}
	LastAccessed=WW3D::Get_Sync_Time();

	DX8_RECORD_TEXTURE(this);

	// Set texture itself
	if (WW3D::Is_Texturing_Enabled()) {
		DX8Wrapper::Set_DX8_Texture(stage, D3DTexture);
	} else {
		DX8Wrapper::Set_DX8_Texture(stage, NULL);
	}

	DX8Wrapper::Set_DX8_Texture_Stage_State(stage,D3DTSS_MINFILTER,_MinTextureFilters[stage][TextureMinFilter]);
	DX8Wrapper::Set_DX8_Texture_Stage_State(stage,D3DTSS_MAGFILTER,_MagTextureFilters[stage][TextureMagFilter]);
	DX8Wrapper::Set_DX8_Texture_Stage_State(stage,D3DTSS_MIPFILTER,_MipMapFilters[stage][MipMapFilter]);

	switch (Get_U_Addr_Mode()) {

		case TEXTURE_ADDRESS_REPEAT:
			DX8Wrapper::Set_DX8_Texture_Stage_State(stage, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
			break;

		case TEXTURE_ADDRESS_CLAMP:
			DX8Wrapper::Set_DX8_Texture_Stage_State(stage, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
			break;

	}

	switch (Get_V_Addr_Mode()) {

		case TEXTURE_ADDRESS_REPEAT:
			DX8Wrapper::Set_DX8_Texture_Stage_State(stage, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
			break;

		case TEXTURE_ADDRESS_CLAMP:
			DX8Wrapper::Set_DX8_Texture_Stage_State(stage, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
			break;

	}
}

// ----------------------------------------------------------------------------

void TextureClass::Apply_Null(unsigned int stage)
{
	// This function sets the render states for a "NULL" texture
	DX8Wrapper::Set_DX8_Texture(stage, NULL);
}

// ----------------------------------------------------------------------------

void TextureClass::Apply_New_Surface(IDirect3DTexture8* d3d_texture,bool initialized)
{
	if (D3DTexture) D3DTexture->Release();
	D3DTexture=d3d_texture;//TextureLoadTask->Peek_D3D_Texture();
	D3DTexture->AddRef();
	if (initialized) Initialized=true;

	WWASSERT(D3DTexture);
	IDirect3DSurface8* surface;
	DX8_ErrorCode(D3DTexture->GetSurfaceLevel(0,&surface));
	D3DSURFACE_DESC d3d_desc;
	::ZeroMemory(&d3d_desc, sizeof(D3DSURFACE_DESC));
	DX8_ErrorCode(surface->GetDesc(&d3d_desc));
	if (initialized) {
		TextureFormat=D3DFormat_To_WW3DFormat(d3d_desc.Format);
		Width=d3d_desc.Width;
		Height=d3d_desc.Height;
	}
	surface->Release();
}

// ----------------------------------------------------------------------------

unsigned TextureClass::Get_Texture_Memory_Usage() const
{
	if (!Initialized) return Calculate_Texture_Memory_Usage(this,0);
	return Calculate_Texture_Memory_Usage(this,0);
}

// ----------------------------------------------------------------------------

int TextureClass::_Get_Total_Locked_Surface_Size()
{
	int total_locked_surface_size=0;

	HashTemplateIterator<StringClass,TextureClass*> ite(WW3DAssetManager::Get_Instance()->Texture_Hash());
	// Loop through all the textures in the manager
	for (ite.First ();!ite.Is_Done();ite.Next ()) {

		// Get the current texture
		TextureClass* tex=ite.Peek_Value();
		if (!tex->Initialized) {
			total_locked_surface_size+=tex->Get_Texture_Memory_Usage();
		}
	}
	return total_locked_surface_size;
}

// ----------------------------------------------------------------------------

int TextureClass::_Get_Total_Texture_Size()
{
	int total_texture_size=0;

	HashTemplateIterator<StringClass,TextureClass*> ite(WW3DAssetManager::Get_Instance()->Texture_Hash());
	// Loop through all the textures in the manager
	for (ite.First ();!ite.Is_Done();ite.Next ()) {
		// Get the current texture
		TextureClass* tex=ite.Peek_Value();
		total_texture_size+=tex->Get_Texture_Memory_Usage();
	}
	return total_texture_size;
}

// ----------------------------------------------------------------------------

int TextureClass::_Get_Total_Lightmap_Texture_Size()
{
	int total_texture_size=0;

	HashTemplateIterator<StringClass,TextureClass*> ite(WW3DAssetManager::Get_Instance()->Texture_Hash());
	// Loop through all the textures in the manager
	for (ite.First ();!ite.Is_Done();ite.Next ()) {
		// Get the current texture
		TextureClass* tex=ite.Peek_Value();
		if (tex->Is_Lightmap()) {
			total_texture_size+=tex->Get_Texture_Memory_Usage();
		}
	}
	return total_texture_size;
}

// ----------------------------------------------------------------------------

int TextureClass::_Get_Total_Procedural_Texture_Size()
{
	int total_texture_size=0;

	HashTemplateIterator<StringClass,TextureClass*> ite(WW3DAssetManager::Get_Instance()->Texture_Hash());
	// Loop through all the textures in the manager
	for (ite.First ();!ite.Is_Done();ite.Next ()) {
		// Get the current texture
		TextureClass* tex=ite.Peek_Value();
		if (tex->Is_Procedural()) {
			total_texture_size+=tex->Get_Texture_Memory_Usage();
		}
	}
	return total_texture_size;
}

// ----------------------------------------------------------------------------

int TextureClass::_Get_Total_Texture_Count()
{
	int texture_count=0;

	HashTemplateIterator<StringClass,TextureClass*> ite(WW3DAssetManager::Get_Instance()->Texture_Hash());
	// Loop through all the textures in the manager
	for (ite.First ();!ite.Is_Done();ite.Next ()) {
		texture_count++;
	}

	return texture_count;
}

// ----------------------------------------------------------------------------

int TextureClass::_Get_Total_Lightmap_Texture_Count()
{
	int texture_count=0;

	HashTemplateIterator<StringClass,TextureClass*> ite(WW3DAssetManager::Get_Instance()->Texture_Hash());
	// Loop through all the textures in the manager
	for (ite.First ();!ite.Is_Done();ite.Next ()) {
		if (ite.Peek_Value()->Is_Lightmap()) {
			texture_count++;
		}
	}

	return texture_count;
}

// ----------------------------------------------------------------------------

int TextureClass::_Get_Total_Procedural_Texture_Count()
{
	int texture_count=0;

	HashTemplateIterator<StringClass,TextureClass*> ite(WW3DAssetManager::Get_Instance()->Texture_Hash());
	// Loop through all the textures in the manager
	for (ite.First ();!ite.Is_Done();ite.Next ()) {
		if (ite.Peek_Value()->Is_Procedural()) {
			texture_count++;
		}
	}

	return texture_count;
}

// ----------------------------------------------------------------------------

int TextureClass::_Get_Total_Locked_Surface_Count()
{
	int texture_count=0;

	HashTemplateIterator<StringClass,TextureClass*> ite(WW3DAssetManager::Get_Instance()->Texture_Hash());
	// Loop through all the textures in the manager
	for (ite.First ();!ite.Is_Done();ite.Next ()) {
		// Get the current texture
		TextureClass* tex=ite.Peek_Value();
		if (!tex->Initialized) {
			texture_count++;
		}
	}

	return texture_count;
}

void TextureClass::_Init_Filters(TextureClass::TextureFilterMode filter_type)
{
	const D3DCAPS8& dx8caps=DX8Wrapper::Get_Current_Caps()->Get_DX8_Caps();

	_MinTextureFilters[0][FILTER_TYPE_NONE]=D3DTEXF_POINT;
	_MagTextureFilters[0][FILTER_TYPE_NONE]=D3DTEXF_POINT;
	_MipMapFilters[0][FILTER_TYPE_NONE]=D3DTEXF_NONE;

	_MinTextureFilters[0][FILTER_TYPE_FAST]=D3DTEXF_LINEAR;
	_MagTextureFilters[0][FILTER_TYPE_FAST]=D3DTEXF_LINEAR;
	_MipMapFilters[0][FILTER_TYPE_FAST]=D3DTEXF_POINT;

	_MagTextureFilters[0][FILTER_TYPE_BEST]=D3DTEXF_POINT;
	_MinTextureFilters[0][FILTER_TYPE_BEST]=D3DTEXF_POINT;
	_MipMapFilters[0][FILTER_TYPE_BEST]=D3DTEXF_POINT;

	if (dx8caps.TextureFilterCaps&D3DPTFILTERCAPS_MAGFLINEAR) _MagTextureFilters[0][FILTER_TYPE_BEST]=D3DTEXF_LINEAR;
	if (dx8caps.TextureFilterCaps&D3DPTFILTERCAPS_MINFLINEAR) _MinTextureFilters[0][FILTER_TYPE_BEST]=D3DTEXF_LINEAR;

	// Set anisotropic filtering only if requested and available
	if (filter_type==TextureClass::TEXTURE_FILTER_ANISOTROPIC) {
		if (dx8caps.TextureFilterCaps&D3DPTFILTERCAPS_MAGFANISOTROPIC) _MagTextureFilters[0][FILTER_TYPE_BEST]=D3DTEXF_ANISOTROPIC;
		if (dx8caps.TextureFilterCaps&D3DPTFILTERCAPS_MINFANISOTROPIC) _MinTextureFilters[0][FILTER_TYPE_BEST]=D3DTEXF_ANISOTROPIC;
	}

	// Set linear mip filter only if requested trilinear or anisotropic, and linear available
	if (filter_type==TextureClass::TEXTURE_FILTER_ANISOTROPIC || filter_type==TextureClass::TEXTURE_FILTER_TRILINEAR) {
		if (dx8caps.TextureFilterCaps&D3DPTFILTERCAPS_MIPFLINEAR) _MipMapFilters[0][FILTER_TYPE_BEST]=D3DTEXF_LINEAR;
	}

	// For stages above zero, set best filter to the same as the stage zero, except if anisotropic
	int i;
	for (i=1;i<MAX_TEXTURE_STAGES;++i) {
/*		_MinTextureFilters[i][FILTER_TYPE_NONE]=D3DTEXF_POINT;
		_MagTextureFilters[i][FILTER_TYPE_NONE]=D3DTEXF_POINT;
		_MipMapFilters[i][FILTER_TYPE_NONE]=D3DTEXF_NONE;

		_MinTextureFilters[i][FILTER_TYPE_FAST]=D3DTEXF_LINEAR;
		_MagTextureFilters[i][FILTER_TYPE_FAST]=D3DTEXF_LINEAR;
		_MipMapFilters[i][FILTER_TYPE_FAST]=D3DTEXF_POINT;

		_MagTextureFilters[i][FILTER_TYPE_BEST]=D3DTEXF_LINEAR;
		_MinTextureFilters[i][FILTER_TYPE_BEST]=D3DTEXF_LINEAR;
		_MipMapFilters[i][FILTER_TYPE_BEST]=D3DTEXF_POINT;
*/
		_MinTextureFilters[i][FILTER_TYPE_NONE]=_MinTextureFilters[i-1][FILTER_TYPE_NONE];
		_MagTextureFilters[i][FILTER_TYPE_NONE]=_MagTextureFilters[i-1][FILTER_TYPE_NONE];
		_MipMapFilters[i][FILTER_TYPE_NONE]=_MipMapFilters[i-1][FILTER_TYPE_NONE];

		_MinTextureFilters[i][FILTER_TYPE_FAST]=_MinTextureFilters[i-1][FILTER_TYPE_FAST];
		_MagTextureFilters[i][FILTER_TYPE_FAST]=_MagTextureFilters[i-1][FILTER_TYPE_FAST];
		_MipMapFilters[i][FILTER_TYPE_FAST]=_MipMapFilters[i-1][FILTER_TYPE_FAST];

		if (_MagTextureFilters[i-1][FILTER_TYPE_BEST]==D3DTEXF_ANISOTROPIC) {
			_MagTextureFilters[i][FILTER_TYPE_BEST]=D3DTEXF_LINEAR;
		}
		else {
			_MagTextureFilters[i][FILTER_TYPE_BEST]=_MagTextureFilters[i-1][FILTER_TYPE_BEST];
		}

		if (_MinTextureFilters[i-1][FILTER_TYPE_BEST]==D3DTEXF_ANISOTROPIC) {
			_MinTextureFilters[i][FILTER_TYPE_BEST]=D3DTEXF_LINEAR;
		}
		else {
			_MinTextureFilters[i][FILTER_TYPE_BEST]=_MinTextureFilters[i-1][FILTER_TYPE_BEST];
		}
		_MipMapFilters[i][FILTER_TYPE_BEST]=_MipMapFilters[i-1][FILTER_TYPE_BEST];


	}

	// Set default to best. The level of best filter mode is controlled by the input parameter.
	for (i=0;i<MAX_TEXTURE_STAGES;++i) {
		_MinTextureFilters[i][FILTER_TYPE_DEFAULT]=_MinTextureFilters[i][FILTER_TYPE_BEST];
		_MagTextureFilters[i][FILTER_TYPE_DEFAULT]=_MagTextureFilters[i][FILTER_TYPE_BEST];
		_MipMapFilters[i][FILTER_TYPE_DEFAULT]=_MipMapFilters[i][FILTER_TYPE_BEST];

		DX8Wrapper::Set_DX8_Texture_Stage_State(i,D3DTSS_MAXANISOTROPY,2);
	}

}

void TextureClass::_Set_Default_Min_Filter(FilterType filter)
{
	for (int i=0;i<MAX_TEXTURE_STAGES;++i) {
		_MinTextureFilters[i][FILTER_TYPE_DEFAULT]=_MinTextureFilters[i][filter];
	}
}

void TextureClass::_Set_Default_Mag_Filter(FilterType filter)
{
	for (int i=0;i<MAX_TEXTURE_STAGES;++i) {
		_MagTextureFilters[i][FILTER_TYPE_DEFAULT]=_MagTextureFilters[i][filter];
	}
}

void TextureClass::_Set_Default_Mip_Filter(FilterType filter)
{
	for (int i=0;i<MAX_TEXTURE_STAGES;++i) {
		_MipMapFilters[i][FILTER_TYPE_DEFAULT]=_MipMapFilters[i][filter];
	}
}

// Utility functions
TextureClass *Load_Texture(ChunkLoadClass & cload)
{
	// Assume failure
	TextureClass *newtex = NULL;

	char name[256];
	if (cload.Open_Chunk () && (cload.Cur_Chunk_ID () == W3D_CHUNK_TEXTURE)) {

		W3dTextureInfoStruct texinfo;
		bool hastexinfo = false;

		/*
		** Read in the texture filename, and a possible texture info structure.
		*/
		while (cload.Open_Chunk()) {
			switch (cload.Cur_Chunk_ID()) {
				case W3D_CHUNK_TEXTURE_NAME:
					cload.Read(&name,cload.Cur_Chunk_Length());
					break;

				case W3D_CHUNK_TEXTURE_INFO:
					cload.Read(&texinfo,sizeof(W3dTextureInfoStruct));
					hastexinfo = true;
					break;
			};
			cload.Close_Chunk();
		}
		cload.Close_Chunk();

		/*
		** Get the texture from the asset manager
		*/
		if (hastexinfo) {

			TextureClass::MipCountType mipcount;

			bool no_lod = ((texinfo.Attributes & W3DTEXTURE_NO_LOD) == W3DTEXTURE_NO_LOD);

			if (no_lod) {
				mipcount = TextureClass::MIP_LEVELS_1;
			} else {
				switch (texinfo.Attributes & W3DTEXTURE_MIP_LEVELS_MASK) {

					case W3DTEXTURE_MIP_LEVELS_ALL:
						mipcount = TextureClass::MIP_LEVELS_ALL;
						break;

					case W3DTEXTURE_MIP_LEVELS_2:
						mipcount = TextureClass::MIP_LEVELS_2;
						break;

					case W3DTEXTURE_MIP_LEVELS_3:
						mipcount = TextureClass::MIP_LEVELS_3;
						break;

					case W3DTEXTURE_MIP_LEVELS_4:
						mipcount = TextureClass::MIP_LEVELS_4;
						break;

					default:
						WWASSERT (false);
						mipcount = TextureClass::MIP_LEVELS_ALL;
						break;
				}
			}

			WW3DFormat format=WW3D_FORMAT_UNKNOWN;

			switch (texinfo.Attributes & W3DTEXTURE_TYPE_MASK) {

				case W3DTEXTURE_TYPE_COLORMAP:
					// Do nothing.
					break;

				case W3DTEXTURE_TYPE_BUMPMAP:
				{
					if (DX8Wrapper::Is_Initted() && DX8Wrapper::Get_Current_Caps()->Support_Bump_Envmap()) {
						// No mipmaps to bumpmap for now
						mipcount=TextureClass::MIP_LEVELS_1;

						if (DX8Wrapper::Get_Current_Caps()->Support_Texture_Format(WW3D_FORMAT_U8V8)) format=WW3D_FORMAT_U8V8;
						else if (DX8Wrapper::Get_Current_Caps()->Support_Texture_Format(WW3D_FORMAT_X8L8V8U8)) format=WW3D_FORMAT_X8L8V8U8;
						else if (DX8Wrapper::Get_Current_Caps()->Support_Texture_Format(WW3D_FORMAT_L6V5U5)) format=WW3D_FORMAT_L6V5U5;
					}
					break;
				}

				default:
					WWASSERT (false);
					break;
			}

			newtex = WW3DAssetManager::Get_Instance()->Get_Texture (name, mipcount, format);

			if (no_lod) {
				newtex->Set_Mip_Mapping(TextureClass::FILTER_TYPE_NONE);
			}
			bool u_clamp = ((texinfo.Attributes & W3DTEXTURE_CLAMP_U) != 0);
			newtex->Set_U_Addr_Mode(u_clamp ? TextureClass::TEXTURE_ADDRESS_CLAMP : TextureClass::TEXTURE_ADDRESS_REPEAT);
			bool v_clamp = ((texinfo.Attributes & W3DTEXTURE_CLAMP_V) != 0);
			newtex->Set_V_Addr_Mode(v_clamp ? TextureClass::TEXTURE_ADDRESS_CLAMP : TextureClass::TEXTURE_ADDRESS_REPEAT);

		} else {
			newtex = WW3DAssetManager::Get_Instance()->Get_Texture(name);
		}

		WWASSERT(newtex);
	}

	// Return a pointer to the new texture
	return newtex;
}

// Utility function used by Save_Texture
void setup_texture_attributes(TextureClass * tex, W3dTextureInfoStruct * texinfo)
{
	texinfo->Attributes = 0;

	if (tex->Get_Mip_Mapping() == TextureClass::FILTER_TYPE_NONE) texinfo->Attributes |= W3DTEXTURE_NO_LOD;
	if (tex->Get_U_Addr_Mode() == TextureClass::TEXTURE_ADDRESS_CLAMP) texinfo->Attributes |= W3DTEXTURE_CLAMP_U;
	if (tex->Get_V_Addr_Mode() == TextureClass::TEXTURE_ADDRESS_CLAMP) texinfo->Attributes |= W3DTEXTURE_CLAMP_V;
}


void Save_Texture(TextureClass * texture,ChunkSaveClass & csave)
{
	const char * filename;
	W3dTextureInfoStruct texinfo;
	memset(&texinfo,0,sizeof(texinfo));

	filename = texture->Get_Full_Path();

	setup_texture_attributes(texture, &texinfo);

	csave.Begin_Chunk(W3D_CHUNK_TEXTURE_NAME);
	csave.Write(filename,strlen(filename)+1);
	csave.End_Chunk();

	if ((texinfo.Attributes != 0) || (texinfo.AnimType != 0) || (texinfo.FrameCount != 0)) {
		csave.Begin_Chunk(W3D_CHUNK_TEXTURE_INFO);
		csave.Write(&texinfo, sizeof(texinfo));
		csave.End_Chunk();
	}
}
