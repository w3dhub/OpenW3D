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
 *                     $Archive:: /Commando/Code/Combat/objectivesviewer.cpp       $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/23/02 3:31p                                               $*
 *                                                                                             *
 *                    $Revision:: 9                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "objectivesviewer.h"
#include "assetmgr.h"
#include "texture.h"
#include "font3d.h"
#include "objectives.h"
#include "translatedb.h"
#include "WWAudio.h"
#include "globalsettings.h"
#include "scene.h"
#include "evasettings.h"
#include "rendobj.h"
#include "string_ids.h"


////////////////////////////////////////////////////////////////
//	Constants
////////////////////////////////////////////////////////////////
static const char *FONT_NAME						= "FONT6X8.TGA";
static const char *HEADER_FONT_NAME				= "FONT8X8.TGA";

static const char *BACKGROUND_MODEL_NAME		= "FRAME_1BIG";
static const char *UPPER_LEFT_BONE_NAME		= "BONE00";
static const char *LOWER_RIGHT_BONE_NAME		= "BONE01";


////////////////////////////////////////////////////////////////
//
//	ObjectivesViewerClass
//
////////////////////////////////////////////////////////////////
ObjectivesViewerClass::ObjectivesViewerClass (void) :
	IsDisplayed (false),
	TextWindow (NULL)
{	
	return ;
}


////////////////////////////////////////////////////////////////
//
//	~ObjectivesViewerClass
//
////////////////////////////////////////////////////////////////
ObjectivesViewerClass::~ObjectivesViewerClass (void)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Shutdown
//
////////////////////////////////////////////////////////////////
void
ObjectivesViewerClass::Shutdown (void)
{
	if (TextWindow != NULL) {
		delete TextWindow;
		TextWindow = NULL;
	}

	IsDisplayed		= false;
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Initialize
//
////////////////////////////////////////////////////////////////
void
ObjectivesViewerClass::Initialize (void)
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

	EvaSettingsDefClass *settings = EvaSettingsDefClass::Get_Instance ();

	//
	//	Configure the backdrop for the text window
	//
	TextWindow->Set_Backdrop ("hud_6x4_Messages.tga", settings->Get_Objectives_Screen_Rect (),
		settings->Get_Objectives_Texture_Size (), settings->Get_Objectives_Endcap_Rect (),
		settings->Get_Objectives_Fadeout_Rect (), settings->Get_Objectives_Background_Rect ());

	//
	//	Configure the area where text can be displayed in the window
	//
	TextWindow->Set_Text_Area (settings->Get_Objectives_Text_Rect ());

	//
	//	Configure the fonts for the window
	//
	TextWindow->Set_Heading_Font (HEADER_FONT_NAME);
	TextWindow->Set_Text_Font (FONT_NAME);
	
	//
	//	Configure the columns
	//
	TextWindow->Add_Column (TRANSLATE (IDS_MENU_MISSION_OBJECTIVE), 0.8F, Vector3 (0.75F, 1.0F, 0.75F));
	TextWindow->Add_Column (TRANSLATE (IDS_OBJ_STATUS), 0.2F, Vector3 (0.75F, 1.0F, 0.75F));
	TextWindow->Display_Columns (true);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Update
//
////////////////////////////////////////////////////////////////
void
ObjectivesViewerClass::Update (void)
{
	if (TextWindow == NULL) {
		return ;
	}

	int objective_count = ObjectiveManager::ObjectiveList.Count ();

	//
	//	Build a temporary list of objectives that we can sort
	//
	SimpleDynVecClass<Objective *> sorted_list;
	int index;
	for (index = 0; index < objective_count; index ++){
		Objective *objective = ObjectiveManager::ObjectiveList[index];
		
		//
		//	Filter out hidden objectives (if necessary)
		//
		if (	ObjectiveManager::Is_Objective_Debug_Mode_Enabled() == true ||
				(objective->Status != ObjectiveManager::STATUS_HIDDEN && objective->Status != ObjectiveManager::STATUS_ACCOMPLISHED))
		{
			sorted_list.Add (objective);
		}
	}
	
	objective_count = sorted_list.Count ();

	//
	//	Sort the objectives
	//
	if (objective_count > 0) {
		::qsort (&sorted_list[0], objective_count, sizeof (Objective *), fnCompareObjectivesCallback);
	}


	//
	//	Start fresh
	//
	TextWindow->Delete_All_Items ();

	//
	//	Add all the objectives to the text window
	//
	for (index = 0; index < objective_count; index ++){
		Objective *objective = sorted_list[index];
		
		const unichar_t *text				= TranslateDBClass::Get_String (objective->ShortDescriptionID);
		const unichar_t *status_text	= objective->Status_To_Name ();
		
		//
		//	Insert an item for this objective
		//
		int item_index = TextWindow->Insert_Item (index, text);
		if (item_index >= 0) {

			//
			//	Configure the entry
			//
			TextWindow->Set_Item_Text (item_index, COL_STATUS, status_text);
			TextWindow->Set_Item_Color (item_index, COL_DESC, objective->Type_To_Color ());
			TextWindow->Set_Item_Color (item_index, COL_STATUS, objective->Status_To_Color ());
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
ObjectivesViewerClass::Display (bool onoff)
{
	IsDisplayed	= onoff;
	TextWindow->Display (onoff);

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
//	Page_Down
//
////////////////////////////////////////////////////////////////
void
ObjectivesViewerClass::Page_Down (void)
{
	if (TextWindow == NULL) {
		return ;
	}

	//
	//	Page down the window
	//
	TextWindow->Page_Down ();

	//
	// Close the window if there is nothing left to display
	//
	if (TextWindow->Get_Display_Count () == 0) {
		Display (false);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Render
//
////////////////////////////////////////////////////////////////
void
ObjectivesViewerClass::Render (void)
{
	if (IsDisplayed == false) {
		return ;
	}

	TextWindow->Render ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	fnCompareObjectivesCallback
//
////////////////////////////////////////////////////////////////
int __cdecl
ObjectivesViewerClass::fnCompareObjectivesCallback
(
	const void *elem1,
	const void *elem2
)
{
   WWASSERT (elem1 != NULL);
   WWASSERT (elem2 != NULL);
   Objective *objective1 = *((Objective **)elem1);
   Objective *objective2 = *((Objective **)elem2);	

	//
	//	Sort the objectives based on type
	//
	int result = 0;
	if (objective1->Type < objective2->Type) {
		result = -1;
	} else if (objective1->Type > objective2->Type) {
		result = 1;
	} else {
		
		//
		//	Sort alphabetically if the types are the same
		//
		const unichar_t *text1 = TranslateDBClass::Get_String (objective1->ShortDescriptionID);
		const unichar_t *text2 = TranslateDBClass::Get_String (objective2->ShortDescriptionID);
		result = ::u_strcasecmp (text1, text2, U_COMPARE_CODE_POINT_ORDER);
	}

   return result;
}
