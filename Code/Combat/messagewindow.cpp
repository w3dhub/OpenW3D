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
 *                     $Archive:: /Commando/Code/Combat/messagewindow.cpp        $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/07/02 10:43a                                              $*
 *                                                                                             *
 *                    $Revision:: 19                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "messagewindow.h"
#include "assetmgr.h"
#include "texture.h"
#include "font3d.h"
#include "objectives.h"
#include "translatedb.h"
#include "scene.h"
#include "timemgr.h"
#include "assets.h"
#include "globalsettings.h"
#include "smartgameobj.h"
#include "evasettings.h"
#include "soldier.h"
#include "dynamicspeechanim.h"
#include "combat.h"
#include "camera.h"
#include "scene.h"
#include "ww3d.h"
#include "wwmemlog.h"

////////////////////////////////////////////////////////////////
//	Constants
////////////////////////////////////////////////////////////////
static const char *FONT_NAME						= "FONT6X8.TGA";
static const char *HEADER_FONT_NAME				= "FONT8X8.TGA";



////////////////////////////////////////////////////////////////
//
//	MessageWindowClass
//
////////////////////////////////////////////////////////////////
MessageWindowClass::MessageWindowClass (void) :
	TextWindow (NULL),
	DecayTime (6000),
	Scene (NULL),
	Camera (NULL),
	HeadModel (NULL),
	IsRectangleDirty (true)
{	
	MessageLog.Set_Growth_Step (1000);
	MessageLogColor.Set_Growth_Step (1000);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	~MessageWindowClass
//
////////////////////////////////////////////////////////////////
MessageWindowClass::~MessageWindowClass (void)
{
	REF_PTR_RELEASE (Scene);
	REF_PTR_RELEASE (Camera);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Shutdown
//
////////////////////////////////////////////////////////////////
void
MessageWindowClass::Shutdown (void)
{
	if (TextWindow != NULL) {
		delete TextWindow;
		TextWindow = NULL;
	}

	if (HeadModel != NULL) {
		HeadModel->Remove ();
	}
	REF_PTR_RELEASE (HeadModel);

	Clear_Log ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Initialize
//
////////////////////////////////////////////////////////////////
void
MessageWindowClass::Initialize (void)
{
	WWMEMLOG(MEM_GAMEDATA);
	//
	//	Allocate the new text window
	//
	TextWindow = new TextWindowClass;

	EvaSettingsDefClass *settings = EvaSettingsDefClass::Get_Instance ();

	//
	//	Configure the backdrop for the text window
	//
	CurrentRect = settings->Get_Messages_Screen_Rect ();
	CurrentRect.Bottom = CurrentRect.Top + 1;
	TextWindow->Set_Backdrop ("hud_6x4_Messages.tga", CurrentRect,
		settings->Get_Messages_Texture_Size (), settings->Get_Messages_Endcap_Rect (),
		settings->Get_Messages_Fadeout_Rect (), settings->Get_Messages_Background_Rect ());
	
	//
	//	Configure the area where text can be displayed in the window
	//
	RectClass text_rect = settings->Get_Messages_Text_Rect ();
	text_rect.Bottom = text_rect.Top + 1;
	TextWindow->Set_Text_Area (text_rect);

	//
	//	Configure the fonts for the window
	//
	TextWindow->Set_Heading_Font (HEADER_FONT_NAME);
	TextWindow->Set_Text_Font (FONT_NAME);
	
	//
	//	Configure the columns
	//
	TextWindow->Add_Column (U_CHAR(""), 1.0F, Vector3 (1, 1, 1));
	TextWindow->Display_Columns (false);

	//
	//	Configure the icon renderer
	//
	//IconRenderer.Reset ();
	//IconRenderer.Set_Hidden (true);
	//IconRenderer.Set_Coordinate_Range (Render2DClass::Get_Screen_Resolution ());

	//
	//	Create a scene to use for the head model
	//
	Scene = new SimpleSceneClass;
	Scene->Set_Ambient_Light(Vector3(1,1,1));

	//
	//	Create a camera to use in the "head-scene"
	//
	Camera = new CameraClass ();
	Camera->Set_Position (Vector3 (0, 0, 2.0F));	
	Camera->Set_View_Plane (DEG_TO_RADF (107.0), DEG_TO_RADF (74.0));
	Camera->Set_Clip_Planes (0.1F, 100.0F);
	Camera->Set_Projection_Type (CameraClass::ORTHO);

	//
	//	Lookup the head model we'll use for conversations
	//
	//HeadModel = WW3DAssetManager::Get_Instance ()->Create_Render_Obj ("CX_HAVOC_HEAD");
	if (HeadModel != NULL) {
		HeadModel->Set_Hidden (true);

		//
		//	Position the head model
		//
		Matrix3D tm (1);
		tm.Rotate_Z (DEG_TO_RADF (90));
		tm.Rotate_X (DEG_TO_RADF (90));		
		tm.Set_Translation (Vector3 (-1.22F, 0.53F, 0));
		HeadModel->Set_Transform (tm);

		//
		//	Add the head model to the scene
		//
		Scene->Add_Render_Object (HeadModel);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Render
//
////////////////////////////////////////////////////////////////
void
MessageWindowClass::Render (void)
{
	WWMEMLOG(MEM_GAMEDATA);

	//
	//	Update the window rectangle as necessary
	//
	if (IsRectangleDirty) {
		Update_Window_Rectangle ();
	}

	if (TextWindow != NULL) {
		TextWindow->Render ();
	}
	
	//IconRenderer.Render ();

	WW3D::Render (Scene, Camera);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Frame_Update
//
////////////////////////////////////////////////////////////////
void
MessageWindowClass::On_Frame_Update (void)
{
	WWMEMLOG(MEM_GAMEDATA);
	if (TextWindow == NULL || TextWindow->Is_Displayed () == false) {
		return ;
	}

	if (TextWindow->Get_Item_Count () == 0) {
		TextWindow->Display (false);
		//IconRenderer.Set_Hidden (true);
		if (HeadModel != NULL) {
			HeadModel->Set_Hidden (true);
		}
		Reset_Current_Rect ();
	} else {
		int items_to_remove = TextWindow->Get_Item_Count () - TextWindow->Get_Display_Count ();
		
		//
		//	Remove messages from the top of the list if we have more then we
		// can display
		//
		int index;
		for (index = 0; index < items_to_remove; index ++) {
			TextWindow->Delete_Item (0);
		}

		//
		//	Decay the entries
		//
		for (index = TextWindow->Get_Item_Count () - 1; index >= 0; index --) {
			
			//
			//	Decay the time remaining on this message
			//
			int time_left = (int)TextWindow->Get_Item_Data (index);
			time_left -= TimeManager::Get_Frame_Ticks ();
			TextWindow->Set_Item_Data (index, (uintptr_t)time_left);

			//
			//	Should we remove this message?
			//
			if (time_left <= 0) {
				TextWindow->Delete_Item (index);
			}
		}
	}
	
	TextWindow->On_Frame_Update ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Add_Message
//
////////////////////////////////////////////////////////////////
void
MessageWindowClass::Add_Message
(
	const WideStringClass &	message,
	const Vector3 &			color,
	SmartGameObj *				game_obj,
	float							decay_time
)
{
	WWMEMLOG(MEM_GAMEDATA);
	if (TextWindow == NULL) {
		return ;
	}

	//
	//	Determine how long we should display the message for...
	//
	int decay_time_in_ms = DecayTime;
	if (decay_time != 0) {
		decay_time_in_ms = int(decay_time * 1000);
	}

	//
	//	Add the message to the end of the list
	//
	if (message.Get_Length () > 0) {
		int item_index = TextWindow->Insert_Item (TextWindow->Get_Item_Count (), message);
		if (item_index != -1) {		
			TextWindow->Set_Item_Color (item_index, 0, color);
			TextWindow->Set_Item_Data (item_index, decay_time_in_ms);		
		}
	}

	//
	//	Resize the window as necessary
	//
	Update_Window_Rectangle ();

	//
	//	Make sure the window is displayed
	//
	if (message.Get_Length () > 0) {
		TextWindow->Display (true);
	}

	//
	//	Get the name of the icon file we'll use
	//
	StringClass filename("hud_6x4_icon_default.tga",true);
	if (game_obj != NULL) {
		Strip_Path_From_Filename (filename, game_obj->Get_Info_Icon_Texture_Filename ());

		//
		//	Try to play an animation on the head model
		//
		if (HeadModel != NULL) {
			SoldierGameObj *soldier = game_obj->As_SoldierGameObj ();
			if (soldier != NULL) {
				DynamicSpeechAnimClass *speech_anim = soldier->Get_Facial_Anim ();

				if (speech_anim != NULL) {
					HeadModel->Set_Animation( speech_anim, 0, RenderObjClass::ANIM_MODE_ONCE );			
				}
			}	
		}
	}


	//
	//	Configure the icon renderer
	//
	//IconRenderer.Reset ();
	//IconRenderer.Set_Hidden (false);

	if (HeadModel != NULL) {
		HeadModel->Set_Hidden (false);
	}

	//
	//	Log the message
	//
	MessageLog.Add (message);
	MessageLogColor.Add (color);

	//
	//	Calculate the display rectangle of the icon
	//
	/*const HUDGlobalSettingsDef *hud_settings = HUDGlobalSettingsDef::Get_Instance ();
	RectClass icon_rect (Vector2 (0, 0), hud_settings->TargetIconSize);
	icon_rect += Render2DClass::Get_Screen_Resolution().Upper_Left () + settings->Get_Messages_Icon_Position ();*/

	//
	//	Build the icon mesh
	//
	//IconRenderer.Set_Texture ( filename );
	//IconRenderer.Add_Quad (icon_rect, 0xFFFFFFFF);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Update_Window_Rectangle
//
////////////////////////////////////////////////////////////////
void
MessageWindowClass::Update_Window_Rectangle (void)
{
	//
	//	Get the dimensions of the text window
	//
	EvaSettingsDefClass *settings		= EvaSettingsDefClass::Get_Instance ();

	if (settings == NULL || TextWindow == NULL) {
		return;
	}

	WWASSERT(settings != NULL);
	RectClass screen_rect				= settings->Get_Messages_Screen_Rect ();
	RectClass text_rect					= settings->Get_Messages_Text_Rect ();
	float top_border						= text_rect.Top - screen_rect.Top;
	float bottom_border					= screen_rect.Bottom - text_rect.Bottom;
	float vert_border						= top_border + bottom_border;

	//
	//	Resize the area (if needs be)
	//
	WWASSERT(TextWindow != NULL);
	float total_height = TextWindow->Get_Total_Display_Height ();
	if (IsRectangleDirty || (total_height > (CurrentRect.Height () - vert_border))) {

		CurrentRect.Bottom = WWMath::Clamp (CurrentRect.Top + total_height + vert_border, 0, screen_rect.Bottom);

		//
		//	Resize the backdrop area
		//
		TextWindow->Set_Backdrop ("hud_6x4_Messages.tga", CurrentRect,
			settings->Get_Messages_Texture_Size (), settings->Get_Messages_Endcap_Rect (),
			settings->Get_Messages_Fadeout_Rect (), settings->Get_Messages_Background_Rect ());

		//
		//	Resize the text area
		//
		text_rect.Top		= CurrentRect.Top + top_border;
		text_rect.Bottom	= CurrentRect.Bottom - bottom_border;
		TextWindow->Set_Text_Area (text_rect);		
	}
	
	IsRectangleDirty = false;
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Clear
//
////////////////////////////////////////////////////////////////
void
MessageWindowClass::Clear (void)
{
	if (TextWindow != NULL) {
		TextWindow->Delete_All_Items ();
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Force_Display
//
////////////////////////////////////////////////////////////////
void
MessageWindowClass::Force_Display (bool onoff)
{
	if (TextWindow != NULL) {
		TextWindow->Display (onoff);
	}

	//IconRenderer.Set_Hidden (!onoff);


	if (HeadModel != NULL) {
		HeadModel->Set_Hidden (!onoff);
	}

	Reset_Current_Rect ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Has_Data
//
////////////////////////////////////////////////////////////////
bool
MessageWindowClass::Has_Data (void) const
{
	return (TextWindow != NULL && TextWindow->Get_Item_Count () > 0);
}


////////////////////////////////////////////////////////////////
//
//	Reset_Current_Rect
//
////////////////////////////////////////////////////////////////
void
MessageWindowClass::Reset_Current_Rect (void)
{
	EvaSettingsDefClass *settings = EvaSettingsDefClass::Get_Instance ();

	//
	//	Shrink the rectangle so its only 1 pixel high (this will force
	// it to grow to the right size later on).
	//
	CurrentRect				= settings->Get_Messages_Screen_Rect ();
	CurrentRect.Bottom	= CurrentRect.Top + 1;	
	return ;
}
