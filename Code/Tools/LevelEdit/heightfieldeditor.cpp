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
 *                 Project Name : leveledit                                                    *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/heightfieldeditor.cpp        $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 3/05/02 3:21p                                               $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "StdAfx.h"
#include "heightfieldeditor.h"
#include "SceneEditor.h"
#include "mover.h"
#include "rendobj.h"
#include "phys.h"
#include "editableheightfield.h"
#include "MouseMgr.h"
#include "texture.h"
#include "terrainmaterial.h"
#include "heightfieldpage.h"
#include "textureloader.h"
#include "d3d9types.h"
#include "dx8wrapper.h"
#include "bitmaphandler.h"


//////////////////////////////////////////////////////////////////////
//	Forward declarations
//////////////////////////////////////////////////////////////////////
float				HeightfieldEditorClass::BrushInnerRadius					= 10.0F;
float				HeightfieldEditorClass::BrushOutterRadius					= 20.0F;
float				HeightfieldEditorClass::BrushAmount							= 1.0F;
int				HeightfieldEditorClass::CurrentTextureIndex				= 0;

EditableHeightfieldClass *							HeightfieldEditorClass::CurrentHeightfield = NULL;
HeightfieldEditorClass::EDITING_MODE			HeightfieldEditorClass::Mode = HeightfieldEditorClass::MODE_DEFORM;
DynamicVectorClass<TerrainMaterialClass *>	HeightfieldEditorClass::MaterialList;


//////////////////////////////////////////////////////////////////////
//	Local constants
//////////////////////////////////////////////////////////////////////
static const int	MAX_HEIGHTFIELD_MATERIALS	= 10;


//////////////////////////////////////////////////////////////////////
//
//	Initialize
//
//////////////////////////////////////////////////////////////////////
void
HeightfieldEditorClass::Initialize (void)
{
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Shutdown
//
//////////////////////////////////////////////////////////////////////
void
HeightfieldEditorClass::Shutdown (void)
{
	//
	//	Free the list of materials
	//
	for (int index = 0; index < MaterialList.Count (); index ++) {
		REF_PTR_RELEASE (MaterialList[index]);
	}

	MaterialList.Delete_All ();
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Brush_Radii
//
//////////////////////////////////////////////////////////////////////
void
HeightfieldEditorClass::Set_Brush_Radii (float inner_radius, float outter_radius)
{
	BrushInnerRadius	= inner_radius;
	BrushOutterRadius	= outter_radius;
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Get_Brush_Radii
//
//////////////////////////////////////////////////////////////////////
void
HeightfieldEditorClass::Get_Brush_Radii (float &inner_radius, float &outter_radius)
{
	inner_radius	= BrushInnerRadius;
	outter_radius	= BrushOutterRadius;
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Mode
//
//////////////////////////////////////////////////////////////////////
void
HeightfieldEditorClass::Set_Mode (EDITING_MODE mode)
{
	Mode = mode;
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	On_Frame_Update
//
//////////////////////////////////////////////////////////////////////
void
HeightfieldEditorClass::On_Frame_Update (void)
{
	//
	//	Don't do any processing if we're not in heightfield editing mode
	//
	if (::Get_Mouse_Mgr ()->Get_Mouse_Mode () != MouseMgrClass::MODE_HEIGHTFIELD_EDIT) {
		return ;
	}

	//
	//	Get a ray from the current view through the mouse cursor position
	//
	Vector3 v_start;
	Vector3 v_end;
	MoverClass::Get_Mouse_Ray (1000.0F, v_start, v_end);

	//
	//	Cast a ray into the world and see what we've hit
	//
	CastResultStruct res;
	res.ComputeContactPoint = true;
	PhysClass *phys_obj = MoverClass::Cast_Ray (res, v_start, v_end);
	if (phys_obj != NULL) {

		//
		//	For right now, add a debug box to show where the operation will occur
		//
		PhysicsSceneClass::Get_Instance ()->Add_Debug_AABox (AABoxClass (res.ContactPoint + Vector3 (0, 0, 2.0F), Vector3 (1.0F, 1.0F, 1.0F)), Vector3 (1, 0, 0));

		//
		//	Check to see if this is a heightfield
		//
		RenderObjClass *model = phys_obj->Peek_Model ();
		if (model != NULL && model->Class_ID () == RenderObjClass::CLASSID_RENEGADE_TERRAIN && CurrentHeightfield != NULL) {
			
			EditableHeightfieldClass *heightfield = CurrentHeightfield;

			//
			//	Is the left button or right button down?
			//
			if (::Get_Mouse_Mgr ()->Is_LButton_Down ()) {
				
				//
				//	Determine what operation to perform
				//
				if (Mode == MODE_DEFORM) {
					heightfield->Deform_Heightfield (res.ContactPoint, BrushAmount, BrushInnerRadius, BrushOutterRadius);
				} else if (Mode == MODE_DEFORM_SMOOTH) {
					heightfield->Smooth_Heightfield (res.ContactPoint, BrushAmount, BrushInnerRadius, BrushOutterRadius);
				} else if (Mode == MODE_DEFORM_SMOOTH_FOUNDATION) {
					heightfield->Smooth_Foundation_Heightfield (res.ContactPoint, BrushAmount, BrushInnerRadius, BrushOutterRadius);
				} else if (Mode == MODE_QUAD_CUTOUT) {
					heightfield->Cutout_Heightfield (res.ContactPoint, BrushOutterRadius, true);
				} else if (Mode == MODE_TEXTURING) {
					heightfield->Paint_Heightfield (res.ContactPoint, CurrentTextureIndex, BrushAmount, BrushInnerRadius, BrushOutterRadius);
				}

			} else if (::Get_Mouse_Mgr ()->Is_RButton_Down ()) {

				//
				//	Determine what operation to perform
				//
				if (Mode == MODE_DEFORM) {
					heightfield->Deform_Heightfield (res.ContactPoint, -BrushAmount, BrushInnerRadius, BrushOutterRadius);
				} else if (Mode == MODE_QUAD_CUTOUT) {
					heightfield->Cutout_Heightfield (res.ContactPoint, BrushOutterRadius, false);
				}
			}
		}
	}
	
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Render
//
//////////////////////////////////////////////////////////////////////
void
HeightfieldEditorClass::Render (void)
{
	
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Create_Texture_Thumbnail
//
//////////////////////////////////////////////////////////////////////
HBITMAP
HeightfieldEditorClass::Create_Texture_Thumbnail (TextureClass *texture, int width, int height)
{
	if (texture == NULL) {
		return NULL;
	}

	//
	//	Create the thumbnail
	//
	StringClass filename = texture->Get_Full_Path ();
	HBITMAP thumbnail = Create_Texture_Thumbnail (filename, width, height);
	
	//
	//	Release our hold on the texture
	//
	REF_PTR_RELEASE (texture);

	return thumbnail;
}


//////////////////////////////////////////////////////////////////////
//
//	Create_Texture_Thumbnail
//
//////////////////////////////////////////////////////////////////////
HBITMAP
HeightfieldEditorClass::Create_Texture_Thumbnail (const char *filename, int width, int height)
{
	if (filename == NULL) {
		return NULL;
	}

	//
	//	Load the d3d surface for this texture
	//	
	IDirect3DSurface9 *d3d_surface = TextureLoader::Load_Surface_Immediate (filename, WW3D_FORMAT_R8G8B8, false);

	//
	//	Get information about the texture
	//
	D3DSURFACE_DESC sd;
	::ZeroMemory (&sd, sizeof(sd));
	DX8_ErrorCode (d3d_surface->GetDesc (&sd));

	//
	//	Get the texture's bits so we can copy tthem
	//
	D3DLOCKED_RECT lock_rect;	
	::ZeroMemory (&lock_rect, sizeof (D3DLOCKED_RECT));
	DX8_ErrorCode (d3d_surface->LockRect (&lock_rect, 0, 0));
	int src_pitch		= lock_rect.Pitch;
	uint8 *src_bits	= (unsigned char *)lock_rect.pBits;

	//
	// Set-up the fields of the BITMAPINFOHEADER
	//	Note: Top-down DIBs use negative height in Win32.
	//
	BITMAPINFOHEADER bitmap_info = { 0 };
	bitmap_info.biSize				= sizeof (BITMAPINFOHEADER);
	bitmap_info.biWidth				= width;
	bitmap_info.biHeight				= -height;
	bitmap_info.biPlanes				= 1;
	bitmap_info.biBitCount			= 24;
	bitmap_info.biCompression		= BI_RGB;
	bitmap_info.biSizeImage			= ((width * height) * 3);
	bitmap_info.biXPelsPerMeter	= 0;
	bitmap_info.biYPelsPerMeter	= 0;
	bitmap_info.biClrUsed			= 0;
	bitmap_info.biClrImportant		= 0;

	//
	// Create a bitmap that we can access the bits directly of
	//
	uint8 *dest_bits	= NULL;
	HDC screen_dc		= ::GetDC (NULL);
	HBITMAP bitmap		= ::CreateDIBSection (screen_dc, (const BITMAPINFO *)&bitmap_info,
														DIB_RGB_COLORS, (void **)&dest_bits, NULL, 0L);	

	int dest_pitch = (((width * 3) + 3) & ~3);

	//
	//	Scale the texture to the size requested
	//
	BitmapHandlerClass::Copy_Image (dest_bits, width, height, dest_pitch, WW3D_FORMAT_R8G8B8,
								src_bits, sd.Width, sd.Height, src_pitch, WW3D_FORMAT_R8G8B8,
								NULL, 0, false);

	//
	//	Free the surface
	//
	DX8_ErrorCode (d3d_surface->UnlockRect ());
	d3d_surface->Release ();
	d3d_surface = NULL;

	return bitmap;
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Material
//
//////////////////////////////////////////////////////////////////////
void
HeightfieldEditorClass::Set_Material (int index, TerrainMaterialClass *material)
{	
	//
	//	Add enough entries to the array until we've got a slot for "index"
	//
	while (index >= MaterialList.Count ()) {
		MaterialList.Add (NULL);
	}

	//
	//	Add this material to its slot
	//
	REF_PTR_SET (MaterialList[index], material);
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Peek_Material
//
//////////////////////////////////////////////////////////////////////
TerrainMaterialClass *
HeightfieldEditorClass::Peek_Material (int index)
{	
	TerrainMaterialClass *retval = NULL;

	//
	//	Simply return the material in this slot (if it exists)
	//
	if (index >= 0 && index < MaterialList.Count ()) {
		retval = MaterialList[index];
	}

	return retval;
}


//////////////////////////////////////////////////////////////////////
//
//	Get_Material
//
//////////////////////////////////////////////////////////////////////
TerrainMaterialClass *
HeightfieldEditorClass::Get_Material (int index)
{	
	//
	//	Simply add a reference to the material and return its pointer
	//
	TerrainMaterialClass *retval = Peek_Material (index);
	if (retval != NULL) {
		retval->Add_Ref ();
	}

	return retval;
}


//////////////////////////////////////////////////////////////////////
//
//	Load_Materials
//
//////////////////////////////////////////////////////////////////////
void
HeightfieldEditorClass::Load_Materials (EditableHeightfieldClass *heightfield)
{
	if (heightfield == NULL) {
		return ;
	}

	//
	//	Simply loop over and store all the materials
	//
	for (int index = 0; index < MAX_HEIGHTFIELD_MATERIALS; index ++) {
		TerrainMaterialClass *material = heightfield->Peek_Material (index);		
		Set_Material (index, material);

		//
		//	Update the UI
		//
		HeightfieldPageClass::Get_Instance ()->Update_Material_Button (index);
	}
		
	CurrentHeightfield = heightfield;
	return ;	
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Current_Heightfield
//
//////////////////////////////////////////////////////////////////////
void
HeightfieldEditorClass::Set_Current_Heightfield (EditableHeightfieldClass *heightfield)
{
	CurrentHeightfield = heightfield;
	Load_Materials (CurrentHeightfield);
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	On_Material_Changed
//
//////////////////////////////////////////////////////////////////////
void
HeightfieldEditorClass::On_Material_Changed (int material_index)
{
	//
	//	Update the material panel UI
	//
	HeightfieldPageClass::Get_Instance ()->Update_Material_Button (material_index);

	//
	//	Apply these changes to the heightfield itself
	//
	if (CurrentHeightfield != NULL) {
		CurrentHeightfield->Set_Material (material_index, MaterialList[material_index]);
		CurrentHeightfield->On_Material_Changed (material_index);
	}
	
	return ;
}
