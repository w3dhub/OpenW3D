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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/heightfieldeditor.h          $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 3/05/02 11:32a                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __HEIGHTFIELDEDITOR_H
#define __HEIGHTFIELDEDITOR_H


#include "vector.h"


//////////////////////////////////////////////////////////////////////
//	Forward declarations
//////////////////////////////////////////////////////////////////////
class TerrainMaterialClass;
class EditableHeightfieldClass;
class TextureClass;


//////////////////////////////////////////////////////////////////////
//
//	HeightfieldEditorClass
//
//////////////////////////////////////////////////////////////////////
class HeightfieldEditorClass
{
public:

	///////////////////////////////////////////////////////////////////
	//	Public data types
	///////////////////////////////////////////////////////////////////
	typedef enum
	{
		MODE_DEFORM								= 0,
		MODE_DEFORM_SMOOTH,
		MODE_DEFORM_SMOOTH_FOUNDATION,
		MODE_DEFORM_NOISE,
		MODE_QUAD_CUTOUT,
		MODE_TEXTURING,
	}	EDITING_MODE;

	///////////////////////////////////////////////////////////////////
	//	Public methods
	///////////////////////////////////////////////////////////////////

	//
	//	Initialization
	//
	static void				Initialize (void);
	static void				Shutdown (void);

	//
	//	Mode support
	//
	static void				Set_Mode (EDITING_MODE mode);
	static EDITING_MODE	Get_Mode (void)						{ return Mode; }

	//
	//	Frame updating support
	//
	static void			On_Frame_Update (void);
	static void			Render (void);

	//
	//	Current heightfield information
	//
	static EditableHeightfieldClass *	Get_Current_Heightfield (void)	{ return CurrentHeightfield; }
	static void									Set_Current_Heightfield (EditableHeightfieldClass *heightfield);

	//
	//	Current brush configuration
	//
	static void			Set_Brush_Radii (float inner_radius, float outter_radius);
	static void			Set_Brush_Amount (float amount)			{ BrushAmount = amount; }

	static void			Get_Brush_Radii (float &inner_radius, float &outter_radius);
	static float		Get_Brush_Inner_Radius (void)				{ return BrushInnerRadius; }
	static float		Get_Brush_Outter_Radius (void)			{ return BrushOutterRadius; }
	static float		Get_Brush_Amount (void)						{ return BrushAmount; }

	//
	//	Texture configuation
	//
	static void			Set_Current_Texture (int index)	{ CurrentTextureIndex = index; }
	static int			Get_Current_Texture (void)			{ return CurrentTextureIndex; }

	//
	//	Material configuration
	//
	static TerrainMaterialClass *	Get_Material (int index);
	static TerrainMaterialClass *	Peek_Material (int index);
	static void							Set_Material (int index, TerrainMaterialClass *material);

	static void							Load_Materials (EditableHeightfieldClass *heightfield);
	static void							On_Material_Changed (int material_index);

	//
	//	UI support
	//
	static HBITMAP		Create_Texture_Thumbnail (const char *texture_name, int width, int height);
	static HBITMAP		Create_Texture_Thumbnail (TextureClass *texture, int width, int height);

private:

	///////////////////////////////////////////////////////////////////
	//	Private methods
	///////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////
	//	Private member data
	///////////////////////////////////////////////////////////////////
	static float			BrushInnerRadius;
	static float			BrushOutterRadius;
	static float			BrushAmount;
	static EDITING_MODE	Mode;
	static int				CurrentTextureIndex;

	static EditableHeightfieldClass *	CurrentHeightfield;

	static DynamicVectorClass<TerrainMaterialClass *>	MaterialList;
};


#endif //__HEIGHTFIELDEDITOR_H
