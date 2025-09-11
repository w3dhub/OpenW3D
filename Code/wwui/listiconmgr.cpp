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
 *                 Project Name : wwui																			  *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwui/listiconmgr.cpp         $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 10/26/01 2:48p                                              $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "listiconmgr.h"
#include "stylemgr.h"
#include "assetmgr.h"
#include "texture.h"
#include "render2d.h"


////////////////////////////////////////////////////////////////
//
//	ListIconMgrClass
//
////////////////////////////////////////////////////////////////
ListIconMgrClass::ListIconMgrClass (void)	:
	IconWidth (16),
	IconHeight (16)
{
	return ;
}

////////////////////////////////////////////////////////////////
//
//	Add_Icon
//
////////////////////////////////////////////////////////////////
void
ListIconMgrClass::Add_Icon (const char *texture_name)
{
	//
	//	Make sure this texture isn't already in our lists
	//
	int index = Find_Texture (texture_name);
	if (index == -1) {

		//
		//	Load the texture
		//
		TextureClass *texture = WW3DAssetManager::Get_Instance ()->Get_Texture (texture_name, TextureClass::MIP_LEVELS_1);	
		if (texture != NULL) {

			//
			//	Create a new renderer and assign it the texture
			//
			Render2DClass *renderer = new Render2DClass;
			StyleMgrClass::Configure_Renderer (renderer);
			renderer->Set_Texture (texture);
			renderer->Enable_Alpha (true);

			//
			//	Add this renderer to our list
			//
			Renderers.Add (renderer);
			TextureNames.Add (texture_name);

			//
			//	Release our hold on the texture
			//
			REF_PTR_RELEASE (texture);
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Remove_Icon
//
////////////////////////////////////////////////////////////////
void
ListIconMgrClass::Remove_Icon (const char *texture_name)
{
	//
	//	Try to find the texture in our list
	//
	int index = Find_Texture (texture_name);
	if (index >= 0) {

		//
		//	Remove this entry from out lists
		//
		delete Renderers[index];
		Renderers.Delete (index);
		TextureNames.Delete (index);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Reset_Icons
//
////////////////////////////////////////////////////////////////
void
ListIconMgrClass::Reset_Icons (void)
{
	//
	//	First, free each renderer
	//
	for (int index = 0; index < Renderers.Count (); index ++) {
		delete Renderers[index];
	}

	//
	//	Now, reset the lists
	//
	Renderers.Delete_All ();
	TextureNames.Delete_All ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Render_Icon
//
////////////////////////////////////////////////////////////////
void
ListIconMgrClass::Render_Icon (const RectClass &clip_rect, const char *texture_name)
{
	//
	//	Try to find the texture in our list
	//
	int index = Find_Texture (texture_name);
	if (index >= 0) {

		float icon_size_x = IconWidth * StyleMgrClass::Get_X_Scale ();
		float icon_size_y = IconHeight * StyleMgrClass::Get_Y_Scale ();

		//
		//	Render the texture left-aligned and v-centered in the clip rectangle
		//
		RectClass rect;
		rect.Left			= clip_rect.Left;
		rect.Right			= clip_rect.Left + icon_size_x;
		rect.Top				= int (clip_rect.Center ().Y - (icon_size_y * 0.5F));
		rect.Bottom			= int (clip_rect.Center ().Y + (icon_size_y * 0.5F));
		Renderers[index]->Add_Quad (rect);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Render_Icons
//
////////////////////////////////////////////////////////////////
void
ListIconMgrClass::Render_Icons (void)
{
	for (int index = 0; index < Renderers.Count (); index ++) {
		Renderers[index]->Render ();
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Find_Texture
//
////////////////////////////////////////////////////////////////
int
ListIconMgrClass::Find_Texture (const char *texture_name)
{
	int retval = -1;

	//
	//	Try to find a texture with this name in our list
	//
	for (int index = 0; index < TextureNames.Count (); index ++) {
		if (::openw3d::stricmp (texture_name, TextureNames[index]) == 0) {
			retval = index;
			break;
		}
	}

	return retval;
}


////////////////////////////////////////////////////////////////
//
//	Reset_Renderers
//
////////////////////////////////////////////////////////////////
void
ListIconMgrClass::Reset_Renderers (void)
{
	for (int index = 0; index < Renderers.Count (); index ++) {
		Renderers[index]->Reset ();
	}

	return ;
}
