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
 *                     $Archive:: /Commando/Code/wwui/mousemgr.cpp               $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 10/22/01 4:49p                                             $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "mousemgr.h"
#include "screencursor.h"
#include "assetmgr.h"
#include "texture.h"


////////////////////////////////////////////////////////////////
//	Local constants
////////////////////////////////////////////////////////////////
static const char * TEXTURE_NAME[MouseMgrClass::CURSOR_COUNT] =
{
	"cursor_arrow.tga",
	"cursor_text.tga",
	"cursor_action.tga",
	"cursor_busy.tga",
	"cursor_pan_up.tga",
	"cursor_rotate.tga"
};


static Vector2 HOTSPOTS[MouseMgrClass::CURSOR_COUNT] =
{
	Vector2 (0, 0),
	Vector2 (15, 15),
	Vector2 (7, 1),
	Vector2 (15, 15),
	Vector2 (15, 17),
	Vector2 (15, 15)
};


////////////////////////////////////////////////////////////////
//	Local constants
////////////////////////////////////////////////////////////////
TextureClass *						MouseMgrClass::Textures[CURSOR_COUNT]	= { 0 };
ScreenCursorClass *				MouseMgrClass::MouseCursor					= NULL;
MouseMgrClass::CURSOR_TYPE		MouseMgrClass::CursorType					= MouseMgrClass::CURSOR_ARROW;
unsigned int MouseMgrClass::CursorWaitCount = 0;

////////////////////////////////////////////////////////////////
//
//	Initialize
//
////////////////////////////////////////////////////////////////
void
MouseMgrClass::Initialize (void)
{
	//
	//	Load each mouse cursor texture
	//
	for (int index = 0; index < CURSOR_COUNT; index ++) {
		Textures[index] = WW3DAssetManager::Get_Instance()->Get_Texture (TEXTURE_NAME[index], TextureClass::MIP_LEVELS_1);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Shutdown
//
////////////////////////////////////////////////////////////////
void
MouseMgrClass::Shutdown (void)
{
	//
	//	Free each mouse cursor texture
	//
	for (int index = 0; index < CURSOR_COUNT; index ++) {
		REF_PTR_RELEASE (Textures[index]);
	}

	//
	//	This will free the cursor object
	//
	Show_Cursor (false);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Show_Cursor
//
////////////////////////////////////////////////////////////////
void
MouseMgrClass::Show_Cursor (bool onoff)
{
	if (MouseCursor == NULL && onoff) {

		//
		//	Create the mouse cursor
		//
		MouseCursor = new ScreenCursorClass;
		MouseCursor->Set_Texture (Textures[CursorType]);
		MouseCursor->Set_Hotspot (HOTSPOTS[CursorType]);
		CursorWaitCount = 0;

	} else if (MouseCursor != NULL && onoff == false) {

		//
		//	Free the mouse cursor
		//
		delete MouseCursor;
		MouseCursor = NULL;
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Set_Cursor
//
////////////////////////////////////////////////////////////////
void
MouseMgrClass::Set_Cursor (CURSOR_TYPE type)
{
	if (CursorType != type && MouseCursor != NULL && CursorWaitCount == 0) {
		MouseCursor->Set_Texture (Textures[type]);
		MouseCursor->Set_Hotspot (HOTSPOTS[type]);
		CursorType = type;
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Begin_Wait_Cursor
//
////////////////////////////////////////////////////////////////
void
MouseMgrClass::Begin_Wait_Cursor(void)
{
	if (CursorWaitCount == 0) {
		Set_Cursor(CURSOR_BUSY);
	}

	++CursorWaitCount;
}


////////////////////////////////////////////////////////////////
//
//	End_Wait_Cursor
//
////////////////////////////////////////////////////////////////
void
MouseMgrClass::End_Wait_Cursor(void)
{
	assert(CursorWaitCount > 0 && "End_Wait_Cursor() mismatch");

	if (CursorWaitCount > 0) {
		--CursorWaitCount;

		if (CursorWaitCount == 0) {
			Set_Cursor(CURSOR_ARROW);
		}
	}
}


////////////////////////////////////////////////////////////////
//
//	Render
//
////////////////////////////////////////////////////////////////
void
MouseMgrClass::Render (void)
{
	if (MouseCursor != NULL) {
		MouseCursor->Render ();
	}

	return ;
}


