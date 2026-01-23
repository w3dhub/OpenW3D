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
 *                     $Archive:: /Commando/Code/wwui/mapctrl.cpp           $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/10/02 6:39p                                               $*
 *                                                                                             *
 *                    $Revision:: 15                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "mapctrl.h"

#include "assetmgr.h"
#include "refcount.h"
#include "font3d.h"
#include "mousemgr.h"
#include "ww3d.h"
#include "dialogmgr.h"
#include "dialogbase.h"
#include "stylemgr.h"
#include "ffactory.h"
#include <algorithm>


////////////////////////////////////////////////////////////////
//	Local constants
////////////////////////////////////////////////////////////////
static const float ZOOM_RATE				=	2.0F;
static const float MARKER_WIDTH			= 16.0F;
static const float MARKER_HEIGHT			= 16.0F;

static const RectClass	ZoomInUVRect	(1, 33, 25, 57);
static const RectClass	ZoomOutUVRect	(26, 33, 53, 57);
static const Vector2		ButtonTextureSize (64, 64);

static const RectClass	EdgeTopUVRect		(1,	2,		31,	30);
static const RectClass	EdgeRightUVRect	(34,	1,		62,	31);
static const RectClass	EdgeLeftUVRect		(2,	31,	30,	63);
static const RectClass	EdgeBottomUVRect	(31,	34,	63,	62);


////////////////////////////////////////////////////////////////
//
//	MapCtrlClass
//
////////////////////////////////////////////////////////////////
MapCtrlClass::MapCtrlClass (void)	:
	Zoom (1.0F),
	ScrollPos (0, 0),
	MapSize (0, 0),
	IsDragging (false),
	IsZoomingIn (false),
	IsZoomingOut (false),
	IsUsingOverlay (false),
	InitialMousePos (0, 0),
	InitialScrollPos (0, 0),
	ZoomInButtonRect (0, 0, 0, 0),
	ZoomOutButtonRect (0, 0, 0, 0),
	MapCenterPoint (0, 0),
	MapScale (1, 1),
	CloudVector (NULL),
	CloudSize (0, 0),
	OverlayOpacity (1.0F),
	PulseDirection (-1.0F)
{
	//
	//	Configure the renderers
	//
	StyleMgrClass::Assign_Font (&TextRenderer, StyleMgrClass::FONT_CONTROLS);
	StyleMgrClass::Configure_Renderer (&ControlRenderer);
	StyleMgrClass::Configure_Renderer (&ButtonRenderer);	
	StyleMgrClass::Configure_Renderer (&MapRenderer);
	StyleMgrClass::Configure_Renderer (&MapOverlayRenderer);	
	StyleMgrClass::Configure_Renderer (&CloudRenderer);
	StyleMgrClass::Configure_Renderer (&EdgeRenderer);	
	StyleMgrClass::Configure_Renderer (&IconRenderer);

	TextureClass *texture = WW3DAssetManager::Get_Instance ()->Get_Texture ("map_edges.tga", TextureClass::MIP_LEVELS_1);
	EdgeRenderer.Set_Texture (texture);
	REF_PTR_RELEASE (texture);

	texture = WW3DAssetManager::Get_Instance ()->Get_Texture ("mapicons.tga", TextureClass::MIP_LEVELS_1);
	ButtonRenderer.Set_Texture (texture);
	REF_PTR_RELEASE (texture);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	~MapCtrlClass
//
////////////////////////////////////////////////////////////////
MapCtrlClass::~MapCtrlClass (void)
{
	Free_Cloud_Data ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Create_Text_Renderers
//
////////////////////////////////////////////////////////////////
void
MapCtrlClass::Create_Text_Renderers (void)
{
	//
	//	Start fresh
	//
	TextRenderer.Reset ();

	//
	//	Draw the text
	//
	//StyleMgrClass::Render_Text (Title, &TextRenderer, TextRect, true, true);

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Create_Control_Renderers
//
////////////////////////////////////////////////////////////////
void
MapCtrlClass::Create_Control_Renderers (void)
{
	Render2DClass &renderer = ControlRenderer;

	//
	//	Configure this renderer
	//
	renderer.Reset ();
	renderer.Enable_Texturing (false);

	//
	//	Determine which color to draw the outline in
	//
	int color	= StyleMgrClass::Get_Line_Color ();
	int bkcolor	= StyleMgrClass::Get_Bk_Color ();
	if (IsEnabled == false) {
		color		= StyleMgrClass::Get_Disabled_Line_Color ();
		bkcolor	= StyleMgrClass::Get_Disabled_Bk_Color ();
	}

	//
	//	Draw the control's outline
	//
	renderer.Add_Outline (Rect, 1.0F, color);
	renderer.Add_Outline (ZoomInButtonRect, 1.0F, color);
	renderer.Add_Outline (ZoomOutButtonRect, 1.0F, color);

	//
	//	Shrink the rectangle
	//
	RectClass rect = Rect;
	rect.Right	-= 1;
	rect.Bottom -= 1;

	//
	//	Calculate what the UV rectangle should be given the
	// current scroll and zoom factors.
	//
	RectClass map_uv_rect (0, 0, 1, 1);

	//
	//	Calculate the center of the image
	//
	Vector2 center = (MapSize / 2) + ScrollPos;

	//
	//	Calculate the 'zoomed' rectangle (in pixels)
	//
	float width				= Rect.Width ();
	float height			= Rect.Height ();
	map_uv_rect.Left		= center.X - ((width / 2) / Zoom);
	map_uv_rect.Top		= center.Y - ((height / 2) / Zoom);
	map_uv_rect.Right		= center.X + ((width / 2) / Zoom);
	map_uv_rect.Bottom	= center.Y + ((height / 2) / Zoom);

	//
	//	Render any markers that are in view
	//
	IconRenderer.Reset ();
	for (int index = 0; index < MarkerList.Count (); index ++) {		
		const MapMarkerClass &marker = MarkerList[index];

		//
		//	Calculate what pixel position this marker is located at.
		//
		Vector2 map_pos;
		map_pos.X = MapCenterPoint.X + (marker.Get_Position ().X * MapScale.X);
		map_pos.Y = MapCenterPoint.Y - (marker.Get_Position ().Y * MapScale.Y);

		//
		//	Is this marker currently visible?
		//
		if (map_uv_rect.Contains (map_pos)) {
			
			//
			//	Calculate what percentage of the visible map the marker's position is
			//
			Vector2 percent;
			percent.X = (map_pos.X - map_uv_rect.Left) / map_uv_rect.Width ();
			percent.Y = (map_pos.Y - map_uv_rect.Top) / map_uv_rect.Height ();

			//
			//	Convert the perctange into a screen position
			//
			Vector2 screen_pos;
			screen_pos.X = rect.Left + (percent.X * rect.Width ());
			screen_pos.Y = rect.Top + (percent.Y * rect.Height ());

			//
			//	Build a screen rectangle we'll render into
			//
			RectClass screen_rect;
			screen_rect.Left		= int(screen_pos.X - (marker.Get_Rect ().Width () / 2));
			screen_rect.Top		= int(screen_pos.Y - (marker.Get_Rect ().Height () / 2));
			screen_rect.Right		= int(screen_rect.Left + marker.Get_Rect ().Width ());
			screen_rect.Bottom	= int(screen_rect.Top + marker.Get_Rect ().Height ());

			//
			//	Convert the pixel coordinates into normalized UVs
			//
			RectClass marker_uv_rect = marker.Get_Rect ();
			marker_uv_rect.Inverse_Scale (Vector2 (IconTextureSize.X, IconTextureSize.Y));

			//
			//	Clip the rectangle as necessary
			//
			RectClass clipped_rect;
			clipped_rect.Left		= std::max (screen_rect.Left, Rect.Left);
			clipped_rect.Right	= std::min (screen_rect.Right, Rect.Right);
			clipped_rect.Top		= std::max (screen_rect.Top, Rect.Top);
			clipped_rect.Bottom	= std::min (screen_rect.Bottom, Rect.Bottom);

			//
			//	Clip the texture to the specified area
			//
			RectClass clipped_uv_rect;
			float clip_percent		= ((clipped_rect.Left - screen_rect.Left) / screen_rect.Width ());
			clipped_uv_rect.Left		= marker_uv_rect.Left + (marker_uv_rect.Width () * clip_percent);

			clip_percent				= ((clipped_rect.Right - screen_rect.Left) / screen_rect.Width ());
			clipped_uv_rect.Right	= marker_uv_rect.Left + (marker_uv_rect.Width () * clip_percent);

			clip_percent				= ((clipped_rect.Top - screen_rect.Top) / screen_rect.Height ());
			clipped_uv_rect.Top		= marker_uv_rect.Top + (marker_uv_rect.Height () * clip_percent);

			clip_percent				= ((clipped_rect.Bottom - screen_rect.Top) / screen_rect.Height ());
			clipped_uv_rect.Bottom	= marker_uv_rect.Top + (marker_uv_rect.Height () * clip_percent);

			//
			//	Use the clipped rectangles to render
			//
			screen_rect		= clipped_rect;
			marker_uv_rect	= clipped_uv_rect;

			//
			//	Render the marker
			//
			IconRenderer.Add_Quad (screen_rect, marker_uv_rect, marker.Get_Color ());
		}
	}

	//
	//	Render the buttons
	//
	ButtonRenderer.Reset ();

	RectClass temp_rect1;
	temp_rect1.Left		= int(ZoomInButtonRect.Center ().X - (ZoomInUVRect.Width () / 2));
	temp_rect1.Top			= int(ZoomInButtonRect.Center ().Y - (ZoomInUVRect.Height () / 2));
	temp_rect1.Right		= int(temp_rect1.Left + ZoomInUVRect.Width ());
	temp_rect1.Bottom		= int(temp_rect1.Top + ZoomInUVRect.Height ());
	
	RectClass temp_rect2;
	temp_rect2.Left		= int(ZoomOutButtonRect.Center ().X - (ZoomInUVRect.Width () / 2));
	temp_rect2.Top			= int(ZoomOutButtonRect.Center ().Y - (ZoomInUVRect.Height () / 2));
	temp_rect2.Right		= int(temp_rect2.Left + ZoomInUVRect.Width ());
	temp_rect2.Bottom		= int(temp_rect2.Top + ZoomInUVRect.Height ());

	RectClass temp_uv_rect1 = ZoomInUVRect;
	RectClass temp_uv_rect2 = ZoomOutUVRect;
	temp_uv_rect1.Inverse_Scale (Vector2 (ButtonTextureSize.X, ButtonTextureSize.Y));
	temp_uv_rect2.Inverse_Scale (Vector2 (ButtonTextureSize.X, ButtonTextureSize.Y));
	ButtonRenderer.Add_Quad (temp_rect1, temp_uv_rect1);
	ButtonRenderer.Add_Quad (temp_rect2, temp_uv_rect2);

	//
	//	Normalize the map UVs
	//
	map_uv_rect.Inverse_Scale (Vector2 (MapSize.X , MapSize.Y));
	
	//
	//	Render the map
	//
	MapRenderer.Reset ();
	MapOverlayRenderer.Reset ();
	MapRenderer.Enable_Alpha (false);
	MapOverlayRenderer.Enable_Alpha (true);
	MapRenderer.Add_Quad (rect, map_uv_rect);
	MapOverlayRenderer.Add_Quad (rect, map_uv_rect);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Create_Cloud_Renderer
//
////////////////////////////////////////////////////////////////
void
MapCtrlClass::Create_Cloud_Renderer (void)
{
	CloudRenderer.Reset ();
	EdgeRenderer.Reset ();
	CloudRenderer.Enable_Texturing (false);

	//
	//	Calculate the dimensions of the cloud at this zoom factor
	//
	float cloud_width		= (MapSize.X / CloudSize.I) * Zoom;
	float cloud_height	= (MapSize.Y / CloudSize.J) * Zoom;

	Vector2 center = (MapSize / 2) - ScrollPos;

	float delta_x = ((MapSize.X * Zoom) - Rect.Width ()) / 2;
	float delta_y = ((MapSize.Y * Zoom) - Rect.Height ()) / 2;

	float cloud_x_pos = Rect.Left - (delta_x + (ScrollPos.X * Zoom));
	float cloud_y_pos = Rect.Top  - (delta_y + (ScrollPos.Y * Zoom));

	//
	//	Loop over all the cells
	//
	for (int cell_y = 0; cell_y < CloudSize.J; cell_y ++) {
		
		//
		//	Reset the x position
		//
		cloud_x_pos = Rect.Left - (delta_x + (ScrollPos.X * Zoom));

		for (int cell_x = 0; cell_x < CloudSize.I; cell_x ++) {
		
			//
			//	Build the rectangle for this cloud section
			//
			RectClass cloud_rect;
			cloud_rect.Left	= cloud_x_pos;
			cloud_rect.Top		= cloud_y_pos;
			cloud_rect.Right	= cloud_rect.Left + cloud_width;
			cloud_rect.Bottom	= cloud_rect.Top + cloud_height;
		
			//
			//	Clip the rectangle
			//
			cloud_rect.Left	= std::max (cloud_rect.Left, Rect.Left);
			cloud_rect.Top		= std::max (cloud_rect.Top, Rect.Top);
			cloud_rect.Right	= std::min (cloud_rect.Right, Rect.Right);
			cloud_rect.Bottom	= std::min (cloud_rect.Bottom, Rect.Bottom);
			
			//
			//	Don't render anything if this cell is completely clipped.
			//
			if (cloud_rect.Width () > 0 && cloud_rect.Height () > 0) {

				//
				//	Is this cell clouded?
				//
				if (Is_Cell_Shrouded (cell_x, cell_y)) {

					//
					//	Render a black square in this cell
					//
					CloudRenderer.Add_Quad (cloud_rect, RGB_TO_INT32 (0, 0, 0));

				} else {

					RectClass delta_rect;
					delta_rect.Left	= ((cloud_rect.Left - cloud_x_pos) / cloud_width);
					delta_rect.Top		= ((cloud_rect.Top - cloud_y_pos) / cloud_height);
					delta_rect.Right	= ((cloud_rect.Right - (cloud_x_pos + cloud_width)) / cloud_width);
					delta_rect.Bottom	= ((cloud_rect.Bottom - (cloud_y_pos + cloud_height)) / cloud_height);


					if (Is_Cell_Shrouded (cell_x - 1, cell_y)) {
						RectClass uv_rect = EdgeLeftUVRect;

						//
						//	Clip the uv-rectangle to fit the color rectangle
						//
						uv_rect.Left	+= uv_rect.Width () * delta_rect.Left;
						uv_rect.Right	+= uv_rect.Width () * delta_rect.Right;
						uv_rect.Top		+= uv_rect.Height () * delta_rect.Top;
						uv_rect.Bottom	+= uv_rect.Height () * delta_rect.Bottom;

						uv_rect.Inverse_Scale (Vector2 (64.0F, 64.0F));
						EdgeRenderer.Add_Quad (cloud_rect, uv_rect);
					} 

					if (Is_Cell_Shrouded (cell_x + 1, cell_y)) {
						RectClass uv_rect = EdgeRightUVRect;

						//
						//	Clip the uv-rectangle to fit the color rectangle
						//
						uv_rect.Left	+= uv_rect.Width () * delta_rect.Left;
						uv_rect.Right	+= uv_rect.Width () * delta_rect.Right;
						uv_rect.Top		+= uv_rect.Height () * delta_rect.Top;
						uv_rect.Bottom	+= uv_rect.Height () * delta_rect.Bottom;

						uv_rect.Inverse_Scale (Vector2 (64.0F, 64.0F));
						EdgeRenderer.Add_Quad (cloud_rect, uv_rect);
					} 

					if (Is_Cell_Shrouded (cell_x, cell_y - 1)) {
						RectClass uv_rect = EdgeTopUVRect;

						//
						//	Clip the uv-rectangle to fit the color rectangle
						//
						uv_rect.Left	+= uv_rect.Width () * delta_rect.Left;
						uv_rect.Right	+= uv_rect.Width () * delta_rect.Right;
						uv_rect.Top		+= uv_rect.Height () * delta_rect.Top;
						uv_rect.Bottom	+= uv_rect.Height () * delta_rect.Bottom;

						uv_rect.Inverse_Scale (Vector2 (64.0F, 64.0F));
						EdgeRenderer.Add_Quad (cloud_rect, uv_rect);
					} 

					if (Is_Cell_Shrouded (cell_x, cell_y + 1)) {
						RectClass uv_rect = EdgeBottomUVRect;

						//
						//	Clip the uv-rectangle to fit the color rectangle
						//
						uv_rect.Left	+= uv_rect.Width () * delta_rect.Left;
						uv_rect.Right	+= uv_rect.Width () * delta_rect.Right;
						uv_rect.Top		+= uv_rect.Height () * delta_rect.Top;
						uv_rect.Bottom	+= uv_rect.Height () * delta_rect.Bottom;

						uv_rect.Inverse_Scale (Vector2 (64.0F, 64.0F));
						EdgeRenderer.Add_Quad (cloud_rect, uv_rect);
					}
				}
			}

			//
			//	Advance one column
			//
			cloud_x_pos += cloud_width;
		}

		//
		//	Advance one row
		//
		cloud_y_pos += cloud_height;
	}
	
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Set_Cursor
//
////////////////////////////////////////////////////////////////
void
MapCtrlClass::On_Set_Cursor (const Vector2 &mouse_pos)
{
	//
	//	Change the mouse cursor
	//
	if (	ZoomInButtonRect.Contains (mouse_pos) ||
			ZoomOutButtonRect.Contains (mouse_pos) ||
			(::GetAsyncKeyState (VK_CONTROL) < 0) ||
			Marker_From_Pos (mouse_pos) != -1)
	{
		MouseMgrClass::Set_Cursor (MouseMgrClass::CURSOR_ACTION);
	} else {
		MouseMgrClass::Set_Cursor (MouseMgrClass::CURSOR_PAN_UP);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Update_Client_Rect
//
////////////////////////////////////////////////////////////////
void
MapCtrlClass::Update_Client_Rect (void)
{
	//
	//	Set the client area
	//
	ClientRect = Rect;

	//
	//	Build the zoom button rectangles
	//
	ZoomOutButtonRect;	
	ZoomOutButtonRect.Left		= int(Rect.Right	- (ZoomInUVRect.Width () + 2));
	ZoomOutButtonRect.Top		= int(Rect.Bottom	- (ZoomInUVRect.Height () + 2));
	ZoomOutButtonRect.Right		= int(Rect.Right);
	ZoomOutButtonRect.Bottom	= int(Rect.Bottom);

	ZoomInButtonRect				= ZoomOutButtonRect;	
	ZoomInButtonRect.Left		= int(ZoomInButtonRect.Left - (ZoomInUVRect.Width () + 2));
	ZoomInButtonRect.Right		= int(ZoomInButtonRect.Right - (ZoomInUVRect.Width () + 2));

	Set_Dirty ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Render
//
////////////////////////////////////////////////////////////////
void
MapCtrlClass::Render (void)
{
	Update_Pulse ();

	//
	//	Recreate the renderers (if necessary)
	//
	if (IsDirty) {
		Create_Cloud_Renderer ();
		Create_Control_Renderers ();
		Create_Text_Renderers ();
	}

	//
	//	Render the background and text for the current state
	//	
	TextRenderer.Render ();
	MapRenderer.Render ();	
	if (IsUsingOverlay) {
		MapOverlayRenderer.Render ();
	}
	IconRenderer.Render ();
	CloudRenderer.Render ();
	EdgeRenderer.Render ();
	ButtonRenderer.Render ();
	ControlRenderer.Render ();	

	DialogControlClass::Render ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_LButton_Down
//
////////////////////////////////////////////////////////////////
void
MapCtrlClass::On_LButton_Down (const Vector2 &mouse_pos)
{
	IsDragging		= false;
	IsZoomingIn		= false;
	IsZoomingOut	= false;

	if (ZoomInButtonRect.Contains (mouse_pos)) {
		IsZoomingIn = true;
		Set_Capture ();
	} else if (ZoomOutButtonRect.Contains (mouse_pos)) {
		IsZoomingOut = true;
		Set_Capture ();
	} else {
		
		//
		//	If the user held the control as they clicked, then
		//	notify the advise sinks that a position is being requested
		//
		if (::GetAsyncKeyState (VK_CONTROL) < 0) {						
			Vector3 world_pos = Position_To_Coord (mouse_pos);
			ADVISE_NOTIFY (On_MapCtrl_Pos_Clicked (this, Get_ID (), world_pos));
		} else {
			
			//
			//	Begin panning
			//
			IsDragging = true;
			Set_Capture ();
			
			InitialMousePos	= mouse_pos;
			InitialScrollPos	= ScrollPos;
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_LButton_Up
//
////////////////////////////////////////////////////////////////
void
MapCtrlClass::On_LButton_Up (const Vector2 &/* mouse_pos */)
{
	Release_Capture ();

	IsDragging		= false;
	IsZoomingIn		= false;
	IsZoomingOut	= false;
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Set_Map_Texture
//
////////////////////////////////////////////////////////////////
void
MapCtrlClass::Set_Map_Texture (const char *filename)
{
	//
	//	Load the texture
	//
	TextureClass *texture = WW3DAssetManager::Get_Instance ()->Get_Texture (filename, TextureClass::MIP_LEVELS_1);
	if (texture != NULL) {

		//
		//	Get the dimensions of the texture
		//
		MapSize.X = texture->Get_Width();
		MapSize.Y = texture->Get_Height();
		texture->Set_U_Addr_Mode (TextureClass::TEXTURE_ADDRESS_CLAMP);
		texture->Set_V_Addr_Mode (TextureClass::TEXTURE_ADDRESS_CLAMP);

		//
		//	Set the texture for the map
		//
		MapRenderer.Set_Texture (texture);
		REF_PTR_RELEASE (texture);

		//
		//	Force a repaint
		//
		Set_Dirty ();
	}

	//
	//	Try to find a texture to use as an overlay...
	//
	StringClass overlay_texture_name = filename;
	const size_t len = overlay_texture_name.Get_Length ();
	overlay_texture_name.Erase (static_cast<int>(len - 4), 4);
	overlay_texture_name += "a.tga";

	//
	//	Check to see if the overlay exists
	//
	IsUsingOverlay = false;
	FileClass *file = _TheFileFactory->Get_File (overlay_texture_name);
	if (file != NULL) {
		
		bool is_valid = true;

		//
		//	If the file doesn't exist as a TGA try to find it as a DDS
		//
		if (file->Is_Available () == false) {
			is_valid = false;
			_TheFileFactory->Return_File (file);
			const size_t overlay_len = overlay_texture_name.Get_Length ();
			overlay_texture_name.Erase (static_cast<int>(overlay_len - 3), 3);
			overlay_texture_name += "dds";
			file = _TheFileFactory->Get_File (overlay_texture_name);
			if (file != NULL && file->Is_Available ()) {
				is_valid = true;
			}
		}

		if (is_valid) {
			texture = WW3DAssetManager::Get_Instance ()->Get_Texture (overlay_texture_name, TextureClass::MIP_LEVELS_1);

			//
			//	Configure the overlay renderer
			//
			if (texture != NULL) {
				texture->Set_U_Addr_Mode (TextureClass::TEXTURE_ADDRESS_CLAMP);
				texture->Set_V_Addr_Mode (TextureClass::TEXTURE_ADDRESS_CLAMP);
				MapOverlayRenderer.Set_Texture (texture);
				REF_PTR_RELEASE (texture);
				IsUsingOverlay = true;
			}
		}

		if (file != NULL) {
			_TheFileFactory->Return_File (file);
		}
	}	

	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Set_Focus
//
////////////////////////////////////////////////////////////////
void
MapCtrlClass::On_Set_Focus (void)
{
	DialogControlClass::On_Set_Focus ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Kill_Focus
//
////////////////////////////////////////////////////////////////
void
MapCtrlClass::On_Kill_Focus (DialogControlClass *focus)
{
	IsDragging		= false;
	IsZoomingIn		= false;
	IsZoomingOut	= false;

	DialogControlClass::On_Kill_Focus (focus);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Mouse_Move
//
////////////////////////////////////////////////////////////////
void
MapCtrlClass::On_Mouse_Move (const Vector2 &mouse_pos)
{
	//
	//	Is the user "dragging" inside the control
	//
	if (HasFocus) {
	
		if (IsDragging) {

			Vector2 delta = (InitialMousePos - mouse_pos);

			//
			//	Update the scroll position
			//
			ScrollPos.X = InitialScrollPos.X + (delta.X / Zoom);
			ScrollPos.Y = InitialScrollPos.Y + (delta.Y / Zoom);
			Clamp_Scroll_Pos ();

			//
			//	Force a repaint
			//
			Set_Dirty ();
		}
	}

	//
	//	Notify the parent
	//
	if (IsDragging == false) {
		int index = Marker_From_Pos (mouse_pos);
		ADVISE_NOTIFY (On_MapCtrl_Marker_Hilighted (this, Get_ID (), index));
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Frame_Update
//
////////////////////////////////////////////////////////////////
void
MapCtrlClass::On_Frame_Update (void)
{
	if (HasFocus) {
		
		if (IsZoomingIn) {
			
			//
			//	Zoom-In
			//
			Zoom += (DialogMgrClass::Get_Frame_Time () / 1000.F) * ZOOM_RATE;
			Zoom = WWMath::Clamp (Zoom, 0.5F, 1.5F);		
			Set_Dirty ();

		} else if (IsZoomingOut) {
			
			//
			//	Zoom-Out
			//
			Zoom -= (DialogMgrClass::Get_Frame_Time () / 1000.0F) * ZOOM_RATE;
			Zoom = WWMath::Clamp (Zoom, 0.5F, 1.5F);
			Set_Dirty ();
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Add_Marker
//
////////////////////////////////////////////////////////////////
int
MapCtrlClass::Add_Marker
(
	const unichar_t *		name,
	const Vector3 &	pos,
	const RectClass &	uv_rect,
	int					color
)
{
	//
	//	Add this marker to our list
	//
	MapMarkerClass marker;
	marker.Set_Name (name);
	marker.Set_Position (pos);
	marker.Set_Rect (uv_rect);
	marker.Set_Color (color);
	MarkerList.Add (marker);

	//
	//	Return the new marker's index
	//
	return (MarkerList.Count () - 1);
}


////////////////////////////////////////////////////////////////
//
//	Get_Marker_Data
//
////////////////////////////////////////////////////////////////
uintptr_t
MapCtrlClass::Get_Marker_Data (int index)
{
	return MarkerList[index].Get_User_Data ();
}


////////////////////////////////////////////////////////////////
//
//	Set_Marker_Data
//
////////////////////////////////////////////////////////////////
void
MapCtrlClass::Set_Marker_Data (int index, uintptr_t user_data)
{
	MarkerList[index].Set_User_Data (user_data);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Remove_Marker
//
////////////////////////////////////////////////////////////////
void
MapCtrlClass::Remove_Marker (int index)
{
	MarkerList.Delete (index);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Set_Marker_Texture
//
////////////////////////////////////////////////////////////////
void
MapCtrlClass::Set_Marker_Texture (const char *filename)
{
	//
	//	Load the texture
	//
	TextureClass *texture = WW3DAssetManager::Get_Instance ()->Get_Texture (filename, TextureClass::MIP_LEVELS_1);
	if (texture != NULL) {

		//
		//	Get the dimensions of the texture
		//
//		SurfaceClass::SurfaceDescription surface_desc;
//		texture->Get_Level_Description (surface_desc);
//		IconTextureSize.X = surface_desc.Width;
//		IconTextureSize.Y = surface_desc.Height;
		IconTextureSize.X = texture->Get_Width ();
		IconTextureSize.Y = texture->Get_Height ();

		//
		//	Set the texture for the map
		//
		IconRenderer.Set_Texture (texture);
		REF_PTR_RELEASE (texture);

		//
		//	Force a repaint
		//
		Set_Dirty ();
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Center_View_About_Marker
//
////////////////////////////////////////////////////////////////
void
MapCtrlClass::Center_View_About_Marker (int marker_index)
{
	const MapMarkerClass &marker = MarkerList[marker_index];

	//
	//	Calculate what pixel position this marker is located at.
	//
	Vector2 map_pos;
	map_pos.X = MapCenterPoint.X + (marker.Get_Position ().X * MapScale.X);
	map_pos.Y = MapCenterPoint.Y - (marker.Get_Position ().Y * MapScale.Y);

	//
	//	Now calculate what scroll offset we'd need to be centered
	// about this position
	//
	ScrollPos = map_pos - (MapSize * 0.5F); 
	Clamp_Scroll_Pos ();

	//
	//	Force a repaint
	//
	Set_Dirty ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Marker_From_Pos
//
////////////////////////////////////////////////////////////////
int
MapCtrlClass::Marker_From_Pos (const Vector2 &mouse_pos)
{
	int retval = -1;

	Vector2 center = (MapSize / 2) + ScrollPos;

	//
	//	Loop over all the markers in our list
	//
	for (int index = 0; index < MarkerList.Count (); index ++) {		
		const MapMarkerClass &marker = MarkerList[index];

		//
		//	Calculate what texel position this marker is located at.
		//
		Vector2 map_pos;
		map_pos.X = MapCenterPoint.X + (marker.Get_Position ().X * MapScale.X);
		map_pos.Y = MapCenterPoint.Y - (marker.Get_Position ().Y * MapScale.Y);

		//
		//	Get the screen position of this objective
		//
		Vector2 screen_pos = Rect.Center () + ((map_pos - center) * Zoom);
		if (Rect.Contains (screen_pos)) {
			
			//
			//	Build a screen rectangle of the marker
			//
			RectClass screen_rect;			
			screen_rect.Left		= screen_pos.X - (marker.Get_Rect ().Width () / 2);
			screen_rect.Top		= screen_pos.Y - (marker.Get_Rect ().Height () / 2);
			screen_rect.Right		= screen_rect.Left + marker.Get_Rect ().Width ();
			screen_rect.Bottom	= screen_rect.Top + marker.Get_Rect ().Height ();

			//
			//	Is the mouse over this marker?
			//
			if (screen_rect.Contains (mouse_pos)) {
				retval = index;
				break;
			}
		}				
	}	

	return retval;
}


////////////////////////////////////////////////////////////////
//
//	Clamp_Scroll_Pos
//
////////////////////////////////////////////////////////////////
void
MapCtrlClass::Clamp_Scroll_Pos (void)
{
	float width		= Rect.Width ();
	float height	= Rect.Height ();

	//
	//	Determine what our scroll offset should be
	// at this zoom factor
	//	
	float max_x_offset	= WWMath::Clamp (MapSize.X - width, 0, MapSize.X);
	float max_y_offset	= WWMath::Clamp (MapSize.Y - height, 0, MapSize.Y);

	float offset_x			= ScrollPos.X / Zoom;
	float offset_y			= ScrollPos.Y / Zoom;
	offset_x					= WWMath::Clamp (offset_x, -max_x_offset, max_x_offset);
	offset_y					= WWMath::Clamp (offset_y, -max_y_offset, max_y_offset);

	//
	//	Re-adjust our scroll position so it doesn't go off the page
	//
	ScrollPos.X				= offset_x * Zoom;
	ScrollPos.Y				= offset_y * Zoom;
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Initialize_Cloud
//
////////////////////////////////////////////////////////////////
void
MapCtrlClass::Initialize_Cloud (int cells_x, int cells_y)
{
	Free_Cloud_Data ();

	CloudSize.I = cells_x;
	CloudSize.J = cells_y;

	//
	//	Allocate a bit vector large enough to hold the cells
	//
	CloudVector = new uint32[((CloudSize.I * CloudSize.J) / sizeof (uint32)) + 1];
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Reset_Cloud
//
////////////////////////////////////////////////////////////////
void
MapCtrlClass::Reset_Cloud (void)
{
	::memset (CloudVector, 0xFF, sizeof (uint32) * ((CloudSize.I * CloudSize.J) / sizeof (uint32)) + 1);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Set_Cloud_Cell
//
////////////////////////////////////////////////////////////////
void
MapCtrlClass::Set_Cloud_Cell (int cell_x, int cell_y, bool is_visible)
{
	if (CloudVector == NULL) {
		return ;
	}

	//
	//	Calculate where in our bit-vector the cell lies
	//
	int bit_offset	= (cell_y * CloudSize.I) + cell_x;
	int index		= bit_offset / 32;
	int bit			= (bit_offset - (index * 32)) + 1;

	//
	//	Set (or clear) the bit
	//
	if (is_visible) {
		CloudVector[index] &= ~(1 << bit);
	} else {
		CloudVector[index] |= (1 << bit);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Free_Cloud_Data
//
////////////////////////////////////////////////////////////////
void
MapCtrlClass::Free_Cloud_Data (void)
{
	if (CloudVector != NULL) {
		delete CloudVector;
		CloudVector = NULL;
	}

	CloudSize.Set (0, 0);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Position_To_Coord
//
////////////////////////////////////////////////////////////////
Vector3
MapCtrlClass::Position_To_Coord (const Vector2 &mouse_pos)
{
	Vector2 percent;
	percent.X = (mouse_pos.X - Rect.Left) / Rect.Width ();
	percent.Y = (mouse_pos.Y - Rect.Top) / Rect.Height ();	

	//
	//	Calculate the center of the image
	//
	RectClass map_uv_rect (0, 0, 1, 1);
	Vector2 center = (MapSize / 2) + ScrollPos;

	//
	//	Calculate the 'zoomed' rectangle (in pixels)
	//
	float width				= Rect.Width ();
	float height			= Rect.Height ();
	map_uv_rect.Left		= center.X - ((width / 2) / Zoom);
	map_uv_rect.Top		= center.Y - ((height / 2) / Zoom);
	map_uv_rect.Right		= center.X + ((width / 2) / Zoom);
	map_uv_rect.Bottom	= center.Y + ((height / 2) / Zoom);

	Vector2 map_pos;
	map_pos.X = map_uv_rect.Left + (map_uv_rect.Width () * percent.X);
	map_pos.Y = map_uv_rect.Top + (map_uv_rect.Height () * percent.Y);

	//
	//	Now convert the map position into a world-space position
	//
	Vector3 result;
	result.X = (map_pos.X - MapCenterPoint.X) / MapScale.X;
	result.Y = (MapCenterPoint.Y - map_pos.Y) / MapScale.Y;
	result.Z = 0;

	return result;
}


////////////////////////////////////////////////////////////////
//
//	Update_Pulse
//
////////////////////////////////////////////////////////////////
void
MapCtrlClass::Update_Pulse (void)
{
	if (IsUsingOverlay == false) {
		return ;
	}

	const float PULSE_RATE	= 0.7F;
	const float MIN_OPACITY	= 0.5F;
	const float MAX_OPACITY	= 1.0F;

	//
	//	Pulse the bar
	//
	float delta = PULSE_RATE * (DialogMgrClass::Get_Frame_Time () / 1000.0F);
	OverlayOpacity += PulseDirection * delta;

	//
	//	Clamp the opacity
	//
	if (OverlayOpacity <= MIN_OPACITY) {
		OverlayOpacity			= MIN_OPACITY;
		PulseDirection		= 1.0F;
	} else if (OverlayOpacity >= MAX_OPACITY) {
		OverlayOpacity			= MAX_OPACITY;
		PulseDirection		= -1.0F;			
	}

	int overlay_color = VRGBA_TO_INT32 (Vector4 (1.0F, 1.0F, 1.0F, OverlayOpacity));

	//
	//	Update the color vector array
	//
	DynamicVectorClass<unsigned int> &color_array = MapOverlayRenderer.Get_Color_Array ();
	for (int index = 0; index < color_array.Count (); index ++) {
		color_array[index] = overlay_color;
	}

	return ;
}
