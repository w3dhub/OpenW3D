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
 *                     $Archive:: /Commando/Code/commando/dlgcontrolsaveload.cpp       $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/09/02 11:43a                                              $*
 *                                                                                             *
 *                    $Revision:: 8                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "dlgcontrolsaveload.h"
#include "resource.h"
#include "listctrl.h"
#include "dialogresource.h"
#include "inputconfigmgr.h"
#include "string_ids.h"
#include "translatedb.h"

////////////////////////////////////////////////////////////////
//	Local constants
////////////////////////////////////////////////////////////////
enum
{
	MBEVENT_DELETE_PROMPT	= 1,
	MBEVENT_SAVE_PROMPT
};


////////////////////////////////////////////////////////////////
//
//	ControlSaveLoadMenuClass
//
////////////////////////////////////////////////////////////////
ControlSaveLoadMenuClass::ControlSaveLoadMenuClass (void)	:
	MenuDialogClass (IDD_MENU_CONTROL_SAVELOAD)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Init_Dialog
//
////////////////////////////////////////////////////////////////
void
ControlSaveLoadMenuClass::On_Init_Dialog (void)
{
	ListCtrlClass *list_ctrl = (ListCtrlClass *)Get_Dlg_Item (IDC_LIST_CTRL);
	if (list_ctrl != NULL) {

		//
		//	Configure the column
		//
		list_ctrl->Add_Column (L"", 1.0F, Vector3 (1, 1, 1));		
		
		//
		//	Loop over all the configurations
		//
		int count = InputConfigMgrClass::Get_Configuration_Count ();
		int index;
		for (index = 0; index < count; index ++) {
			
			//
			//	Get information about this configuration
			//
			InputConfigClass config;
			InputConfigMgrClass::Get_Configuration (index, config);

			Insert_Configuration (config);
		}

		//
		//	Add an entry so the user can add new configurations
		//
		int item_index = list_ctrl->Insert_Entry (index, TRANSLATE (IDS_MENU_EMPTY_SLOT));
		list_ctrl->Set_Curr_Sel (item_index);

		//
		//	Sort the entries
		//
		list_ctrl->Sort (ListSortCallback, 0);
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
ControlSaveLoadMenuClass::On_Command (int ctrl_id, int message_id, DWORD param)
{
	switch (ctrl_id)
	{
		case IDC_DELETE_BUTTON:
			Delete_Config ();
			break;

		case IDC_SAVE_BUTTON:
			Save_Config (true);
			break;

		case IDC_LOAD_BUTTON:
			Load_Config ();
			break;
	}

	MenuDialogClass::On_Command (ctrl_id, message_id, param);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Delete_Config
//
////////////////////////////////////////////////////////////////
void
ControlSaveLoadMenuClass::Delete_Config (void)
{
	ListCtrlClass *list_ctrl = (ListCtrlClass *)Get_Dlg_Item (IDC_LIST_CTRL);
	if (list_ctrl == NULL) {
		return ;
	}

	//
	//	Get the current selection
	//
	int curr_sel = list_ctrl->Get_Curr_Sel ();
	if (curr_sel != -1) {

		//
		//	Get the configuration object associated with this entry
		//
		InputConfigClass *config = (InputConfigClass *)list_ctrl->Get_Entry_Data (curr_sel, 0);
		if (config != NULL) {

			//
			//	Delete the configuration (if possible)
			//
			if (config->Is_Custom ()) {

				//
				//	Build a confirmation message to display to the user
				//
				WideStringClass message;
				message.Format (TRANSLATE (IDS_MENU_DELETE_SAVE_MSG),
							list_ctrl->Get_Entry_Text (curr_sel, 0));

				//
				//	Display a message to the user asking if they really want to do this...
				//
				DlgMsgBox::DoDialog (TRANSLATE (IDS_MENU_DELETE_SAVE_TITLE), message, DlgMsgBox::YesNo,
								this, MBEVENT_DELETE_PROMPT);
			}
		}		
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	HandleNotification
//
////////////////////////////////////////////////////////////////
void
ControlSaveLoadMenuClass::HandleNotification (DlgMsgBoxEvent &event)
{
	if (event.Get_User_Data () == MBEVENT_DELETE_PROMPT) {
		
		//
		//	The user has confirmed the delete, so delete the configuration
		//
		if (event.Event () == DlgMsgBoxEvent::Yes) {

			ListCtrlClass *list_ctrl = (ListCtrlClass *)Get_Dlg_Item (IDC_LIST_CTRL);
			if (list_ctrl != NULL) {

				//
				//	Get the current selection
				//
				int curr_sel = list_ctrl->Get_Curr_Sel ();
				if (curr_sel != -1) {

					//
					//	Get the configuration object associated with this entry
					//
					InputConfigClass *config = (InputConfigClass *)list_ctrl->Get_Entry_Data (curr_sel, 0);
					if (config != NULL) {
						InputConfigMgrClass::Delete_Configuration (config->Get_Filename ());
						list_ctrl->Delete_Entry (curr_sel);
					}
				}
			}
		}

	} else if (event.Get_User_Data () == MBEVENT_SAVE_PROMPT) {

		//
		//	The user has confirmed the overwrite, so save the configuration
		//
		if (event.Event () == DlgMsgBoxEvent::Yes) {
			Save_Config (false);
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Load_Config
//
////////////////////////////////////////////////////////////////
void
ControlSaveLoadMenuClass::Load_Config (void)
{
	ListCtrlClass *list_ctrl = (ListCtrlClass *)Get_Dlg_Item (IDC_LIST_CTRL);
	if (list_ctrl == NULL) {
		return ;
	}

	//
	//	Get the current selection
	//
	int curr_sel = list_ctrl->Get_Curr_Sel ();
	if (curr_sel != -1) {
		
		//
		//	Get the configuration object associated with this entry
		//
		InputConfigClass *config = (InputConfigClass *)list_ctrl->Get_Entry_Data (curr_sel, 0);
		if (config != NULL) {
			
			//
			//	Load this configuration
			//
			InputConfigMgrClass::Load_Configuration (*config);
			End_Dialog ();
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Save_Config
//
////////////////////////////////////////////////////////////////
void
ControlSaveLoadMenuClass::Save_Config (bool prompt)
{
	ListCtrlClass *list_ctrl = (ListCtrlClass *)Get_Dlg_Item (IDC_LIST_CTRL);
	if (list_ctrl == NULL) {
		return ;
	}

	//
	//	Get the current selection
	//
	int curr_sel = list_ctrl->Get_Curr_Sel ();
	if (curr_sel != -1) {
		
		//
		//	Get the configuration object associated with this entry
		//
		InputConfigClass *config = (InputConfigClass *)list_ctrl->Get_Entry_Data (curr_sel, 0);
		if (config != NULL) {
			
			//
			//	We can only save custom configurations...
			//
			if (config->Is_Custom ()) {

				//
				//	Get the new display name for this configuration
				//
				const WCHAR *display_name = Get_Dlg_Item_Text (IDC_NAME_EDIT);			
				if (display_name[0] != 0) {

					//
					//	Display a message to the user asking if they really want to do this...
					//
					bool save_file = true;
					if (prompt) {
						DlgMsgBox::DoDialog (IDS_MENU_CONTROLS_OVERWRITE_PROMPT_TITLE, IDS_MENU_CONTROLS_OVERWRITE_PROMPT_MSG,
											DlgMsgBox::YesNo, this, MBEVENT_SAVE_PROMPT);

						save_file = false;
					}

					if (save_file) {

						//
						//	Update the name of this configuration
						//
						config->Set_Display_Name (display_name);
						list_ctrl->Set_Entry_Text (curr_sel, 0, display_name);
						
						//
						//	Save the configuration
						//
						InputConfigMgrClass::Save_Configuration (*config);
						End_Dialog ();
					}

				} else {
					
					//
					//	Let the user know they can't save a configuration without a name
					//
					DlgMsgBox::DoDialog (IDS_MENU_CANT_SAVE_CONFIG, IDS_MENU_CONFIG_NEEDS_NAME, DlgMsgBox::Okay, NULL, 0);
				}
			}

		} else {

			//
			//	Get the new display name for this configuration
			//
			const WCHAR *display_name = Get_Dlg_Item_Text (IDC_NAME_EDIT);
			if (display_name[0] != 0) {

				//
				//	Add a new configuration
				//
				InputConfigMgrClass::Add_Configuration (display_name);
				End_Dialog ();

			} else {

				//
				//	Let the user know they can't save a configuration without a name
				//
				DlgMsgBox::DoDialog (IDS_MENU_CANT_SAVE_CONFIG, IDS_MENU_CONFIG_NEEDS_NAME, DlgMsgBox::Okay, NULL, 0);
			}
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_ListCtrl_Delete_Entry
//
////////////////////////////////////////////////////////////////
void
ControlSaveLoadMenuClass::On_ListCtrl_Delete_Entry
(
	ListCtrlClass *list_ctrl,
	int				ctrl_id,
	int				item_index
)
{
	//
	//	Remove the data we associated with this entry
	//
	InputConfigClass *config = (InputConfigClass *)list_ctrl->Get_Entry_Data (item_index, 0);
	list_ctrl->Set_Entry_Data (item_index, 0, NULL);

	//
	//	Free the config object
	//
	if (config != NULL) {
		delete config;
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Insert_Configuration
//
////////////////////////////////////////////////////////////////
int
ControlSaveLoadMenuClass::Insert_Configuration (const InputConfigClass &config)
{
	ListCtrlClass *list_ctrl = (ListCtrlClass *)Get_Dlg_Item (IDC_LIST_CTRL);
	if (list_ctrl == NULL) {
		return -1;
	}

	//
	//	Add an entry for this configuration to the list
	//
	int item_index = list_ctrl->Insert_Entry (0xFFFF, config.Get_Display_Name ());
	if (item_index != -1) {

		//
		//	Make a copy of the config object and store it with the entry
		//
		InputConfigClass *local_copy = new InputConfigClass (config);
		list_ctrl->Set_Entry_Data (item_index, 0, (DWORD)local_copy);

		//
		//	Change the color of this configuration if the user cannot edit it
		//
		if (config.Is_Custom () == false) {
			list_ctrl->Set_Entry_Color (item_index, 0, Vector3 (1.0F, 1.0F, 1.0F));
		}
	}

	return item_index;
}


////////////////////////////////////////////////////////////////
//
//	On_ListCtrl_Sel_Change
//
////////////////////////////////////////////////////////////////
void
ControlSaveLoadMenuClass::On_ListCtrl_Sel_Change
(
	ListCtrlClass *	list_ctrl,
	int					ctrl_id,
	int					old_index,
	int					new_index
)
{
	bool enable_edit = true;

	//
	//	Remove the data we associated with this entry
	//
	InputConfigClass *config = (InputConfigClass *)list_ctrl->Get_Entry_Data (new_index, 0);
	if (config != NULL) {
		
		//
		//	We want to disable the edit control if the user can't edit this entry
		//
		if (config->Is_Custom () == false) {
			enable_edit = false;
		}

		//
		//	Put the name of the currently selected configuration in the control
		//
		Set_Dlg_Item_Text (IDC_NAME_EDIT, config->Get_Display_Name ());
	} else {

		//
		//	Clear the name of the current configuration
		//
		Set_Dlg_Item_Text (IDC_NAME_EDIT, L"");
	}

	//
	//	Fix the enable state of the edit control
	//
	Enable_Dlg_Item (IDC_NAME_EDIT, enable_edit);	
	return ;
}


////////////////////////////////////////////////////////////////
//
//	ListSortCallback
//
////////////////////////////////////////////////////////////////
int CALLBACK
ControlSaveLoadMenuClass::ListSortCallback
(
	ListCtrlClass *	list_ctrl,
	int					item_index1,
	int					item_index2,
	uint32				user_param
)
{
	int retval = 0;

	if (list_ctrl->Get_Entry_Data (item_index1, 0) == NULL) {
		retval = 1;
	} else if (list_ctrl->Get_Entry_Data (item_index2, 0) == NULL) {
		retval = -1;
	} else {

		InputConfigClass *config1 = (InputConfigClass *)list_ctrl->Get_Entry_Data (item_index1, 0);
		InputConfigClass *config2 = (InputConfigClass *)list_ctrl->Get_Entry_Data (item_index2, 0);

		//
		//	Sort based on the type of configuration
		//
		if (config1->Is_Default () && config2->Is_Default () == false) {
			retval = -1;
		} else if (config1->Is_Default () == false && config2->Is_Default ()) {
			retval = 1;
		} else if (config1->Is_Custom () && config2->Is_Custom () == false) {
			retval = -1;
		} else if (config1->Is_Custom () == false && config2->Is_Custom ()) {
			retval = 1;
		} else {
			
			//
			//	Sort based on the names
			//
			retval = ::wcsicmp (config1->Get_Display_Name (), config2->Get_Display_Name ());
		}	
	}

	return retval;
}


////////////////////////////////////////////////////////////////
//
//	On_EditCtrl_Enter_Pressed
//
////////////////////////////////////////////////////////////////
void
ControlSaveLoadMenuClass::On_EditCtrl_Enter_Pressed (EditCtrlClass *edit_ctrl, int ctrl_id)
{
	if (ctrl_id == IDC_NAME_EDIT) {
		Save_Config (true);
	}

	return ;
}

