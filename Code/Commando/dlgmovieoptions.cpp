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
 *                     $Archive:: /Commando/Code/Commando/dlgmovieoptions.cpp       $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/15/02 9:28p                                               $*
 *                                                                                             *
 *                    $Revision:: 7                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "renegadedialog.h"
#include "dlgmovieoptions.h"
#include "listctrl.h"
#include "BINKMovie.h"
#include "registry.h"
#include "translatedb.h"
#include "_globals.h"
#include "string_ids.h"
#include "WWAudio.h"

////////////////////////////////////////////////////////////////
//
//	MovieOptionsMenuClass
//
////////////////////////////////////////////////////////////////
MovieOptionsMenuClass::MovieOptionsMenuClass (void)	:
	IsPlaying (false),
	MenuDialogClass (GetRenegadeDialog(RenegadeDialogID::IDD_OPTIONS_MOVIES))
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Init_Dialog
//
////////////////////////////////////////////////////////////////
void
MovieOptionsMenuClass::On_Init_Dialog (void)
{
	//
	//	Get a pointer to the list control
	//
	ListCtrlClass *list_ctrl = (ListCtrlClass *)Get_Dlg_Item (IDC_LIST_CTRL);
	if (list_ctrl != NULL) {

		//
		//	Configure the list control
		//
		list_ctrl->Add_Column (TRANSLATE (IDS_MOVIE_COL_HEADER), 1.0F, Vector3 (1, 1, 1));

		//
		//	Add the movies to the list...
		//		
		RegistryClass registry (APPLICATION_SUB_KEY_NAME_MOVIES);
		if (registry.Is_Valid ()) {

			const char *INTRO_MOVIE	= "MOVIES\\R_INTRO.BIK";
						
			//
			//	Insert the renegade intro movie by default...
			//
			int item_index = list_ctrl->Insert_Entry (0xFF, TRANSLATE (IDS_INTRO_MOVIE));
			if (item_index != -1) {
				list_ctrl->Set_Entry_Data (item_index, 0, (uintptr_t)new StringClass (INTRO_MOVIE));
			}

			//
			//	Get the list of entries from the registry
			//
			DynamicVectorClass<StringClass> list;
			registry.Get_Value_List (list);

			//
			//	Loop over all the movies in the registry
			//
			for (int index = 0; index < list.Count (); index ++) {
				StringClass string_id_des;
				registry.Get_String (list[index], string_id_des);

				//
				//	Add an entry for this movie
				//
				const wchar_t *wide_desc = TRANSLATE_BY_DESC(string_id_des);
				int item_index = list_ctrl->Insert_Entry (0xFF, wide_desc);
				if (item_index != -1) {
					list_ctrl->Set_Entry_Data (item_index, 0, (uintptr_t)new StringClass (list[index]));
				}
			}
		}
	}

	MenuDialogClass::On_Init_Dialog ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Command
//
////////////////////////////////////////////////////////////////
void
MovieOptionsMenuClass::On_Command (int ctrl_id, int message_id, unsigned int param)
{
	if (IsPlaying) {
		return ;
	}

	//
	//	Play the movie if the user clicked the play button
	//
	if (ctrl_id == IDC_MENU_PLAY_MOVIE_BUTTON) {
		Begin_Play_Movie ();
	}

	MenuDialogClass::On_Command (ctrl_id, message_id, param);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_ListCtrl_Delete_Entry
//
////////////////////////////////////////////////////////////////
void
MovieOptionsMenuClass::On_ListCtrl_Delete_Entry
(
	ListCtrlClass *list_ctrl,
	int				ctrl_id,
	int				item_index
)
{
	if (IsPlaying) {
		return ;
	}

	if (ctrl_id == IDC_LIST_CTRL) {
		
		//
		//	Remove the data we associated with this entry
		//
		StringClass *filename = (StringClass *)list_ctrl->Get_Entry_Data (item_index, 0);
		list_ctrl->Set_Entry_Data (item_index, 0, 0);
		if (filename != NULL) {
			delete filename;
		}		
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_ListCtrl_DblClk
//
////////////////////////////////////////////////////////////////
void
MovieOptionsMenuClass::On_ListCtrl_DblClk
(
	ListCtrlClass *list_ctrl,
	int				ctrl_id,
	int				item_index
)
{
	if (IsPlaying) {
		return ;
	}

	Begin_Play_Movie ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Begin_Play_Movie
//
////////////////////////////////////////////////////////////////
void
MovieOptionsMenuClass::Begin_Play_Movie (void)
{
	//
	//	Get a pointer to the list control
	//
	ListCtrlClass *list_ctrl = (ListCtrlClass *)Get_Dlg_Item (IDC_LIST_CTRL);
	if (list_ctrl == NULL) {
		return ;
	}
	
	//
	//	Get the currently selected entry
	//
	int curr_sel = list_ctrl->Get_Curr_Sel ();
	if (curr_sel != -1) {
		StringClass *filename = (StringClass *)list_ctrl->Get_Entry_Data (curr_sel, 0);
		
		//
		//	Play the movie (if it exists locally)
		//
		if (::GetFileAttributesA (filename->Peek_Buffer ()) != 0xFFFFFFFF) {
			Play_Movie (filename->Peek_Buffer ());
		} else {

			//
			//	Strip any path information off the filename
			//
			StringClass filename_only (filename->Peek_Buffer (), true);
			const char *delimiter = ::strrchr (filename->Peek_Buffer (), '\\');
			if (delimiter != NULL) {
				filename_only = delimiter + 1;
			}

			//
			//	Try to find the CD...
			//
			StringClass cd_path;
			if (CDVerifier.Get_CD_Path (cd_path)) {
				
				//
				//	Build a full-path to the movie on the CD
				//
				StringClass full_path = cd_path;
				const size_t path_length = cd_path.Get_Length ();
					if (path_length == 0 || cd_path[static_cast<int>(path_length - 1)] != '\\') {
						full_path += "\\";
					}
				full_path += filename_only;
				Play_Movie (full_path);
			} else {
				PendingMovieFilename = filename_only;
				CDVerifier.Display_UI (this);
			}
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Play_Movie
//
////////////////////////////////////////////////////////////////
void
MovieOptionsMenuClass::Play_Movie (const char *filename)
{
	WWAudioClass::Get_Instance ()->Temp_Disable_Audio (true);
	
	FontCharsClass* font = StyleMgrClass::Get_Font(StyleMgrClass::FONT_INGAME_SUBTITLE_TXT);

	BINKMovie::Play (filename, "data\\subtitle.ini", font);
	
	if (font) {
		font->Release_Ref();
	}

	IsPlaying = true;
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Render
//
////////////////////////////////////////////////////////////////
void
MovieOptionsMenuClass::Render (void)
{
	//
	//	Render the movie or stop it from playing as necessary
	//
	if (IsPlaying) {
		if (BINKMovie::Is_Complete () == false) {
			BINKMovie::Render ();
		} else {
			WWAudioClass::Get_Instance ()->Temp_Disable_Audio (false);
			BINKMovie::Stop ();
			IsPlaying = false;
		}
	}

	MenuDialogClass::Render ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Frame_Update
//
////////////////////////////////////////////////////////////////
void
MovieOptionsMenuClass::On_Frame_Update (void)
{
	//
	//	Let the movie update (if nececessary)
	//
	BINKMovie::Update ();

	MenuDialogClass::On_Frame_Update ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Key_Down
//
////////////////////////////////////////////////////////////////
bool
MovieOptionsMenuClass::On_Key_Down (uint32 key_id, uint32 key_data)
{
	bool retval = false;
	
	//
	//	Stop playing the movie on any keypress
	//
	if (IsPlaying && key_id == VK_ESCAPE) {
		WWAudioClass::Get_Instance ()->Temp_Disable_Audio (false);
		BINKMovie::Stop ();
		IsPlaying = false;
		retval = true;
	} else if (IsPlaying == false) {
		retval = MenuDialogClass::On_Key_Down (key_id, key_data);
	}

	return retval;
}


////////////////////////////////////////////////////////////////
//
//	HandleNotification
//
////////////////////////////////////////////////////////////////
void
MovieOptionsMenuClass::HandleNotification (CDVerifyEvent &event)
{
	if (event.Event () == CDVerifyEvent::VERIFIED) {

		//
		//	Get the path to the CD...
		//
		StringClass cd_path;
		if (CDVerifier.Get_CD_Path (cd_path)) {
			
			//
			//	Build a full-path to the movie on the CD
			//
			StringClass full_path = cd_path;
			const size_t path_length = cd_path.Get_Length ();
				if (path_length == 0 || cd_path[static_cast<int>(path_length - 1)] != '\\') {
					full_path += "\\";
				}
			full_path += PendingMovieFilename;
			Play_Movie (full_path);
		}		
	}

	PendingMovieFilename = "";
	return ;
}

