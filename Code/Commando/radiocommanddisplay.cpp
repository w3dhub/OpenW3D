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
 *                 Project Name : commando                                                    *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Commando/radiocommanddisplay.cpp                              $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 3/15/02 3:26p                                                 $*
 *                                                                                             *
 *                    $Revision:: 5                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "radiocommanddisplay.h"
#include "assetmgr.h"
#include "texture.h"
#include "font3d.h"
#include "objectives.h"
#include "translatedb.h"
#include "WWAudio.h"
#include "globalsettings.h"
#include "scene.h"
#include "evasettings.h"
#include "CNCModeSettings.h"
#include "string_ids.h"
#include "rendobj.h"
#include "input.h"
#include "dinput.h"
#include "timemgr.h"
#include "gametype.h"

#include "combat.h"
#include "vehicle.h"
#include "soldier.h"


////////////////////////////////////////////////////////////////
//	Static member initialization
////////////////////////////////////////////////////////////////
float					RadioCommandDisplayClass::DisplayTimer		= 0.0F;
bool					RadioCommandDisplayClass::IsDisplayed		= false;
TextWindowClass *	RadioCommandDisplayClass::TextWindow		= NULL;



////////////////////////////////////////////////////////////////
//
//	Shutdown
//
////////////////////////////////////////////////////////////////
void
RadioCommandDisplayClass::Shutdown (void)
{
	if (TextWindow != NULL) {
		delete TextWindow;
		TextWindow = NULL;
	}

	IsDisplayed		= false;
	DisplayTimer	= 0.0F;
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Initialize
//
////////////////////////////////////////////////////////////////
void
RadioCommandDisplayClass::Initialize (void)
{
	//
	//	Start fresh
	//
	if (TextWindow != NULL) {
		delete TextWindow;
		TextWindow = NULL;
	}

	//
	//	Allocate the new text window
	//
	TextWindow = new TextWindowClass;

	//
	//	Use the same dimensions as the mission objectives in single player
	//
	EvaSettingsDefClass *settings = EvaSettingsDefClass::Get_Instance ();

	//
	//	Local constants
	//
	const char *BACKDROP_TGA = "hud_6x4_Messages.tga";

	//
	//	Configure the backdrop for the text window
	//
	TextWindow->Set_Backdrop (BACKDROP_TGA, settings->Get_Objectives_Screen_Rect (),
		settings->Get_Objectives_Texture_Size (), settings->Get_Objectives_Endcap_Rect (),
		settings->Get_Objectives_Fadeout_Rect (), settings->Get_Objectives_Background_Rect ());

	//
	//	Configure the area where text can be displayed in the window
	//
	TextWindow->Set_Text_Area (settings->Get_Objectives_Text_Rect ());

	//
	//	Configure the columns
	//
	TextWindow->Add_Column (TRANSLATE (IDS_MENU_RADIO_KEY), 0.3F, Vector3 (0.75F, 1.0F, 0.75F));
	TextWindow->Add_Column (TRANSLATE (IDS_MENU_RADIO_KEY_MESSAGE), 0.7F, Vector3 (0.75F, 1.0F, 0.75F));
	TextWindow->Display_Columns (true);
	DisplayTimer = 0.0F;
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Update
//
////////////////////////////////////////////////////////////////
void
RadioCommandDisplayClass::Update (DISPLAY_TYPE type)
{
	CNCModeSettingsDef *cnc_def = CNCModeSettingsDef::Get_Instance ();
	if (TextWindow == NULL || cnc_def == NULL) {
		return ;
	}

	//
	//	Start fresh
	//
	TextWindow->Delete_All_Items ();

	//
	//	Local constants
	//
	const int COMMAND_COUNT	= 10;

	//
	//	Determine what the access key to display these radio commands is.
	//
	int range_start_index = 0;
	WideStringClass access_key;
	if (type == DISPLAY_CMDS_01) {
		access_key = TRANSLATE (IDS_MENU_TEXT892);
	} else if (type == DISPLAY_CMDS_02) {
		access_key = TRANSLATE (IDS_MENU_TEXT893);
		range_start_index = 10;
	} else {
		access_key.Format (U_CHAR("%s + %s"), TRANSLATE (IDS_MENU_TEXT892), TRANSLATE (IDS_MENU_TEXT893));
		range_start_index = 20;
	}

	//
	//	Add all the radio commands to the text window
	//
	for (int index = 0; index < COMMAND_COUNT; index ++){
		
		int key_id = index + 1;
		if (key_id >= 10) {
			key_id -= 10;
		}

		WideStringClass command_key (0, true);
		command_key.Format (U_CHAR("%s + %d"), access_key, key_id);
		
		//
		//	Insert an item for this radio command
		//
		int item_index = TextWindow->Insert_Item (index, command_key);
		if (item_index >= 0) {

			//
			//	Lookup the message that this command displays
			//
			int cmd_index	= range_start_index + index;
			int text_id		= cnc_def->Get_Radio_Command (cmd_index);

			//
			//	Display the message
			//
			TextWindow->Set_Item_Text (item_index, 1, TRANSLATE (text_id));
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Display
//
////////////////////////////////////////////////////////////////
void
RadioCommandDisplayClass::Display (bool onoff, DISPLAY_TYPE type)
{
	//
	//	Display or hide the window
	//
	IsDisplayed	= onoff;
	TextWindow->Display (onoff);
	
	//
	//	Update the data in the window as necessary
	//
	if (onoff) {
		Update (type);
	}

	//
	//	Play the 'EVA displayed' sound effect
	//
	if (IsDisplayed) {
		int sound_id = GlobalSettingsDef::Get_Global_Settings ()->Get_EVA_Objectives_Sound_ID ();
		if (sound_id != 0) {
			WWAudioClass::Get_Instance ()->Create_Instant_Sound (sound_id, Matrix3D (1));
		}
	}
	
	return ;	
}


////////////////////////////////////////////////////////////////
//
//	Render
//
////////////////////////////////////////////////////////////////
void
RadioCommandDisplayClass::Render (void)
{
	//
	//	Don't do this for true single player
	//
	if (IS_MISSION) {
		return ;
	}

	Check_Keys ();

	//
	//	Only render if the window is being displayed
	//
	if (IsDisplayed) {
		TextWindow->Render ();
	}
	
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Check_Keys
//
////////////////////////////////////////////////////////////////
void
RadioCommandDisplayClass::Check_Keys (void)
{
	//
	//	Local constants
	//
	const float TIMER_DURATION	= 2.0F;

	// ignore left control if star is in a flying vehicle
	bool ignore_lctrl = ( COMBAT_STAR && COMBAT_STAR->As_SoldierGameObj() && COMBAT_STAR->As_SoldierGameObj()->Get_Vehicle() && COMBAT_STAR->As_SoldierGameObj()->Get_Vehicle()->Is_Aircraft() );

	//
	//	Determine which keys (if any) are down
	//
	bool is_key_down[DISPLAY_MAX] = { 0 };
	if ( ignore_lctrl ) {
		is_key_down[DISPLAY_CMDS_01] = Input::Is_Button_Down (DIK_RCONTROL);
		is_key_down[DISPLAY_CMDS_02] = Input::Is_Button_Down (DIK_RMENU);
	} else {
		is_key_down[DISPLAY_CMDS_01] = Input::Is_Button_Down (DIK_LCONTROL) || Input::Is_Button_Down (DIK_RCONTROL);
		is_key_down[DISPLAY_CMDS_02] = Input::Is_Button_Down (DIK_LMENU) || Input::Is_Button_Down (DIK_RMENU);
	}
	is_key_down[DISPLAY_CMDS_03] = is_key_down[DISPLAY_CMDS_01] && is_key_down[DISPLAY_CMDS_02];

	//
	//	Are any of the buttons being held?
	//
	bool is_any_key_down = is_key_down[DISPLAY_CMDS_01] || is_key_down[DISPLAY_CMDS_02];
	if (is_any_key_down) {

		//
		//	Do we need to display the UI or trigger the timer?
		//
		if (IsDisplayed == false) {
			if (DisplayTimer == 0.0F) {
				DisplayTimer = TIMER_DURATION;
			} else {

				//
				//	Is it time to display the UI yet?
				//
				DisplayTimer -= TimeManager::Get_Frame_Seconds ();
				if (DisplayTimer <= 0.0F) {
					
					//
					//	Determine which page to display
					//
					for (int index = DISPLAY_MAX - 1; index >= 0; index --) {
						if (is_key_down[index]) {
							Display (true, (DISPLAY_TYPE)index);
							DisplayTimer = 0.0F;
							break;
						}
					}					
				}				
			}
		}

	} else {

		//
		//	Turn off the UI if the key is no longer being held
		//
		if (IsDisplayed) {
			Display (false);			
		}

		DisplayTimer = 0.0F;
	}
	
	return ;
}
