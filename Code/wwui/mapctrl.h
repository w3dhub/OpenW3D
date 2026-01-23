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
 *                 Project Name : Combat																		  *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwui/mapctrl.h           $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 11/26/01 10:44a                                             $*
 *                                                                                             *
 *                    $Revision:: 7                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __MAP_CTRL_H
#define __MAP_CTRL_H

#include "dialogcontrol.h"
#include "vector3.h"
#include "vector2i.h"
#include "vector.h"
#include "render2dsentence.h"
#include "bittype.h"


////////////////////////////////////////////////////////////////
//
//	MapMarkerClass
//
////////////////////////////////////////////////////////////////
class MapMarkerClass : public DialogControlClass
{
public:

	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	MapMarkerClass (void)	:
		Position (0, 0, 0),
		UVRect (0, 0, 0, 0),
		Color (0xFFFFFFFF),
		UserData (0)						{}

	virtual ~MapMarkerClass (void)	{}

	////////////////////////////////////////////////////////////////
	//	Public operators
	////////////////////////////////////////////////////////////////
	bool					operator== (const MapMarkerClass &/* src*/) const	{ return false; }
	bool					operator!= (const MapMarkerClass &/* src*/)	const	{ return true; }

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////


	//
	//	Accessors
	//
	const Vector3 &	Get_Position (void) const				{ return Position; }
	const unichar_t *		Get_Name (void) const					{ return Name; }
	const RectClass &	Get_Rect (void) const					{ return UVRect; }
	uintptr_t				Get_User_Data (void)	const				{ return UserData; }
	uint32				Get_Color (void) const					{ return Color; }
	
	void					Set_Position (const Vector3 &pos)	{ Position = pos; }
	void					Set_Name (const unichar_t *name)			{ Name = name; }
	void					Set_Rect (const RectClass &rect)		{ UVRect = rect; }
	void					Set_User_Data (uintptr_t data)			{ UserData = data; }
	void					Set_Color (uint32 color)				{ Color = color; }

protected:

	////////////////////////////////////////////////////////////////
	//	Protected member data
	////////////////////////////////////////////////////////////////
	Vector3				Position;
	WideStringClass	Name;
	RectClass			UVRect;
	uintptr_t				UserData;
	uint32				Color;
};


////////////////////////////////////////////////////////////////
//
//	MapCtrlClass
//
////////////////////////////////////////////////////////////////
class MapCtrlClass : public DialogControlClass
{
public:

	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	MapCtrlClass (void);
	virtual ~MapCtrlClass (void);

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////

	//
	//	From DialogControlClass
	//
	void				Render (void) override;

	//
	//	Map configuration
	//
	void				Set_Map_Texture (const char *filename);
	void				Set_Marker_Texture (const char *filename);

	//
	//	Zoom support
	//
	float				Get_Zoom (void) const	{ return Zoom; }
	void				Set_Zoom (float factor)	{ Zoom = factor; Set_Dirty (); }

	//
	//	Map centering and scale.
	//
	//		Note:	The scale means "how many pixels (of map) per meter (of world space)".
	//				This value can differ in the X and Y axes.
	//
	//				The map center is the pixel position (x,y) where world space (0, 0, 0) is.
	//
	const Vector2 &Get_Map_Scale (void) const					{ return MapScale; }
	void				Set_Map_Scale (const Vector2 &scale)	{ MapScale = scale; Set_Dirty (); }

	const Vector2 &Get_Map_Center (void) const				{ return MapCenterPoint; }
	void				Set_Map_Center (const Vector2 &pos)		{ MapCenterPoint = pos; Set_Dirty (); }

	//
	//	Marker control
	//
	int				Add_Marker (const unichar_t *name, const Vector3 &pos, const RectClass &uv_rect, int color = 0xFFFFFFFF);
	uintptr_t			Get_Marker_Data (int index);
	void				Set_Marker_Data (int index, uintptr_t user_data);
	void				Remove_Marker (int index);

	//
	//	View support
	//
	void				Center_View_About_Marker (int marker_index);

	//
	//	Cloud support
	//
	void				Initialize_Cloud (int cells_x, int cells_y);
	void				Reset_Cloud (void);
	void				Set_Cloud_Cell (int cell_x, int cell_y, bool is_visible);


protected:

	////////////////////////////////////////////////////////////////
	//	Protected methods
	////////////////////////////////////////////////////////////////
	void				On_LButton_Down (const Vector2 &mouse_pos) override;
	void				On_LButton_Up (const Vector2 &mouse_pos) override;
	void				On_Mouse_Move (const Vector2 &mouse_pos) override;
	void				On_Set_Cursor (const Vector2 &mouse_pos) override;
	void				On_Set_Focus (void) override;
	void				On_Kill_Focus (DialogControlClass *focus) override;
	void				On_Frame_Update (void) override;
	void				Update_Client_Rect (void) override;

	void				Create_Control_Renderers (void);
	void				Create_Text_Renderers (void);
	void				Create_Cloud_Renderer (void);
	
	int				Marker_From_Pos (const Vector2 &mouse_pos);
	void				Clamp_Scroll_Pos (void);	
	Vector3			Position_To_Coord (const Vector2 &mouse_pos);

	bool				Is_Cell_Shrouded (int cell_x, int cell_y);
	void				Free_Cloud_Data (void);

	void				Update_Pulse (void);


	////////////////////////////////////////////////////////////////
	//	Protected member data
	////////////////////////////////////////////////////////////////
	Render2DSentenceClass 	TextRenderer;
	Render2DClass				ControlRenderer;
	Render2DClass			ButtonRenderer;
	Render2DClass			MapRenderer;
	Render2DClass			MapOverlayRenderer;
	Render2DClass			CloudRenderer;
	Render2DClass			EdgeRenderer;
	Render2DClass			IconRenderer;
	float						Zoom;
	Vector2					ScrollPos;
	Vector2					MapSize;
	Vector2					IconTextureSize;
	bool						IsDragging;
	bool						IsZoomingIn;
	bool						IsZoomingOut;
	bool						IsUsingOverlay;
	Vector2					InitialMousePos;
	Vector2					InitialScrollPos;
	RectClass				ZoomInButtonRect;
	RectClass				ZoomOutButtonRect;
	float						PulseDirection;
	float						OverlayOpacity;

	Vector2					MapCenterPoint;
	Vector2					MapScale;

	uint32 *					CloudVector;
	Vector2i					CloudSize;

	DynamicVectorClass<MapMarkerClass>	MarkerList;
};


////////////////////////////////////////////////////////////////
//	Is_Cell_Shrouded
////////////////////////////////////////////////////////////////
inline bool
MapCtrlClass::Is_Cell_Shrouded (int cell_x, int cell_y)
{
	bool retval = true;

	//
	//	Is this cell in bounds?
	//
	if (	cell_x >= 0 && cell_x < CloudSize.I &&
			cell_y >= 0 && cell_y < CloudSize.J)
	{
		int bit_offset	= (cell_y * CloudSize.I) + cell_x;
		int index		= bit_offset / 32;
		int bit			= (bit_offset - (index * 32)) + 1;

		//
		//	Check this bit
		//
		retval = ((CloudVector[index] & (1 << bit)) != 0);
	}

	return retval;
}


#endif //__MAP_CTRL_H
