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
 *                     $Archive:: /Commando/Code/Commando/dlgserversaveload.cpp               $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 2/25/02 4:29p                                               $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "dlgserversaveload.h"
#include "listctrl.h"
#include "renegadedialog.h"
#include "inputconfigmgr.h"
#include "string_ids.h"
#include "rawfile.h"
#include "ini.h"
#include "gamedata.h"
#include "assets.h"
#include "dlgmplanhostoptions.h"
#include "renegadedialogmgr.h"
#include "dlgmpslaveservers.h"
#include <cstdio>


DynamicVectorClass<ServerSettingsClass*> ServerSettingsManagerClass::ServerSettingsList;

bool ServerSaveLoadMenuClass::FromSlaveConfig = false;
const char *DEFAULT_SERVER_SETTINGS_FILE_NAME = "svrcfg_cnc_%04d.ini";


////////////////////////////////////////////////////////////////
//
//	ServerSaveLoadMenuClass
//
////////////////////////////////////////////////////////////////
ServerSaveLoadMenuClass::ServerSaveLoadMenuClass (void)	:
	MenuDialogClass (GetRenegadeDialog(RenegadeDialogID::IDD_MENU_SERVER_SETTINGS_SAVELOAD))
{
}


////////////////////////////////////////////////////////////////
//
//	On_Init_Dialog
//
////////////////////////////////////////////////////////////////
void
ServerSaveLoadMenuClass::On_Init_Dialog (void)
{
	if (FromSlaveConfig) {
		Get_Dlg_Item(IDC_DELETE_BUTTON)->Enable(false);
		Get_Dlg_Item(IDC_SAVE_BUTTON)->Enable(false);
	} else {
		Get_Dlg_Item(IDC_DELETE_BUTTON)->Enable(true);
		Get_Dlg_Item(IDC_SAVE_BUTTON)->Enable(true);
	}

	ListCtrlClass *list_ctrl = (ListCtrlClass *)Get_Dlg_Item (IDC_LIST_CTRL);
	if (list_ctrl != NULL) {

		//
		//	Configure the column
		//
		list_ctrl->Add_Column (L"", 1.0F, Vector3 (1, 1, 1));

		//
		//	Loop over all the configurations
		//
		ServerSettingsManagerClass::Scan();
		int count = ServerSettingsManagerClass::Get_Num_Settings_Files();

		int index;
		for (index = 0; index < count; index ++) {

			//
			//	Get information about this configuration
			//
			ServerSettingsClass *config = ServerSettingsManagerClass::Get_Settings(index);
			Insert_Configuration (config);
		}

		//
		//	Add an entry so the user can add new configurations
		//
		int item_index = list_ctrl->Insert_Entry (index, TRANSLATE(IDS_MENU_EMPTY_SLOT));
		list_ctrl->Set_Curr_Sel (item_index);

		//
		//	Sort the entries
		//
		list_ctrl->Sort (ListSortCallback, 0);
	}

	MenuDialogClass::On_Init_Dialog ();
}


////////////////////////////////////////////////////////////////
//
//	On_Command
//
////////////////////////////////////////////////////////////////
void
ServerSaveLoadMenuClass::On_Command (int ctrl_id, int message_id, unsigned int param)
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

		case IDC_MENU_BACK_BUTTON:
			Next_Dialog();
			break;
	}

	MenuDialogClass::On_Command (ctrl_id, message_id, param);
}



void ServerSaveLoadMenuClass::Next_Dialog(void)
{
	if (FromSlaveConfig) {
	} else {
		START_DIALOG(MPLanHostOptionsMenuClass);
	}
}



////////////////////////////////////////////////////////////////
//
//	Delete_Config
//
////////////////////////////////////////////////////////////////
void
ServerSaveLoadMenuClass::Delete_Config (void)
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
		ServerSettingsClass *config = (ServerSettingsClass *)list_ctrl->Get_Entry_Data (curr_sel, 0);
		if (config != NULL) {

			//
			//	Delete the configuration (if possible)
			//
			if (config->IsCustom && !config->Is_Default_Custom()) {
				ServerSettingsManagerClass::Delete_Configuration(config);
				list_ctrl->Delete_Entry(curr_sel);
				list_ctrl->Sort (ListSortCallback, 0);
			}
		}
	}
}


////////////////////////////////////////////////////////////////
//
//	Load_Config
//
////////////////////////////////////////////////////////////////
void
ServerSaveLoadMenuClass::Load_Config (void)
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
		ServerSettingsClass *config = (ServerSettingsClass *)list_ctrl->Get_Entry_Data(curr_sel, 0);
		if (config != NULL) {

			if (FromSlaveConfig) {
				//
				// Inform the slave settings dialog of the choice.
				//
				SlaveServerDialogClass::Set_Slave_Settings(&config->RawFileName);
			} else {

				//
				//	Load this configuration
				//
				ServerSettingsManagerClass::Load_Settings(config);
			}

			End_Dialog ();
			Next_Dialog();
		}
	}
}


////////////////////////////////////////////////////////////////
//
//	Save_Config
//
////////////////////////////////////////////////////////////////
void
ServerSaveLoadMenuClass::Save_Config (bool prompt)
{
	ListCtrlClass *list_ctrl = (ListCtrlClass *)Get_Dlg_Item (IDC_LIST_CTRL);
	if (list_ctrl == NULL) {
		return ;
	}

	//
	//	Get the current selection
	//
	int curr_sel = list_ctrl->Get_Curr_Sel();
	if (curr_sel != -1) {

		//
		//	Get the configuration object associated with this entry
		//
		ServerSettingsClass *config = (ServerSettingsClass *)list_ctrl->Get_Entry_Data(curr_sel, 0);
		if (config != NULL) {

			//
			//	We can only save custom configurations...
			//
			if (config->IsCustom) {
				WideStringClass outMsg;

				if (The_Game()->Is_Valid_Settings(outMsg, true)) {

					//
					//	Get the new display name for this configuration
					//
					const wchar_t *display_name = Get_Dlg_Item_Text (IDC_NAME_EDIT);
					if (display_name[0] != 0) {

						//
						//	Display a message to the user asking if they really want to do this...
						//
						if (prompt) {
							DlgMsgBox::DoDialog (IDS_MENU_CONTROLS_OVERWRITE_PROMPT_TITLE, IDS_MENU_CONTROLS_OVERWRITE_PROMPT_MSG, DlgMsgBox::YesNo, this);
						} else {
							//
							// Just save it.
							//
							Save_Now();
						}
					} else {

						//
						//	Let the user know they can't save a configuration without a name
						//
						DlgMsgBox::DoDialog(IDS_MENU_CANT_SAVE_CONFIG, IDS_MENU_CONFIG_NEEDS_NAME, DlgMsgBox::Okay, NULL, 0);
					}
				} else {
#if(0) // Denzil
					//
					//	Let the user know the settings are not correct
					//
					RenegadeDialogMgrClass::Do_Simple_Dialog(GetRenegadeDialog(RenegadeDialogID::IDD_MP_INVALID_SERVER_CONFIG));
#else
					WideStringClass errorMsg(0, true);
					errorMsg.Format(L"%s\n\n%s", TRANSLATE(IDS_MENU_TEXT330), (const wchar_t*)outMsg);
					DlgMsgBox::DoDialog(TRANSLATE(IDS_MENU_TEXT329), errorMsg);
#endif
				}
			}

		} else {

			//
			//	Get the new display name for this configuration
			//
			const wchar_t *display_name = Get_Dlg_Item_Text (IDC_NAME_EDIT);
			if (display_name[0] != 0) {

				//
				//	Add a new configuration
				//
				WideStringClass name(display_name, true);
				config = ServerSettingsManagerClass::Add_Configuration(&name);
				if (config != NULL) {
					Insert_Configuration(config);
					ServerSettingsManagerClass::Save_Configuration(config);
					list_ctrl->Sort (ListSortCallback, 0);
				}
			} else {

				//
				//	Let the user know they can't save a configuration without a name
				//
				DlgMsgBox::DoDialog (IDS_MENU_CANT_SAVE_CONFIG, IDS_MENU_CONFIG_NEEDS_NAME, DlgMsgBox::Okay, NULL, 0);
			}
		}
	}
}


////////////////////////////////////////////////////////////////
//
//	On_ListCtrl_Delete_Entry
//
////////////////////////////////////////////////////////////////
void
ServerSaveLoadMenuClass::On_ListCtrl_Delete_Entry
(
	ListCtrlClass *list_ctrl,
	int				ctrl_id,
	int				item_index
)
{
	//
	//	Remove the data we associated with this entry
	//
	ServerSettingsClass *config = (ServerSettingsClass *)list_ctrl->Get_Entry_Data (item_index, 0);
	list_ctrl->Set_Entry_Data (item_index, 0, 0);

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
ServerSaveLoadMenuClass::Insert_Configuration (ServerSettingsClass *config)
{
	ListCtrlClass *list_ctrl = (ListCtrlClass *)Get_Dlg_Item (IDC_LIST_CTRL);
	if (list_ctrl == NULL) {
		return -1;
	}

	//
	//	Add an entry for this configuration to the list
	//
	int item_index = list_ctrl->Insert_Entry (0xFFFF, config->ConfigName);
	if (item_index != -1) {

		//
		//	Make a copy of the config object and store it with the entry
		//
		ServerSettingsClass *local_copy = new ServerSettingsClass(config);
		list_ctrl->Set_Entry_Data (item_index, 0, (uintptr_t)local_copy);

		//
		//	Change the color of this configuration if the user cannot edit it
		//
		if (!config->IsCustom) {
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
ServerSaveLoadMenuClass::On_ListCtrl_Sel_Change
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
	ServerSettingsClass *config = (ServerSettingsClass *)list_ctrl->Get_Entry_Data(new_index, 0);
	if (config != NULL) {

		//
		//	We want to disable the edit control if the user can't edit this entry
		//
		if (!config->IsCustom || config->Is_Default_Custom()) {
			enable_edit = false;
		}

		//
		//	Put the name of the currently selected configuration in the control
		//
		Set_Dlg_Item_Text (IDC_NAME_EDIT, config->ConfigName);
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
ServerSaveLoadMenuClass::ListSortCallback
(
	ListCtrlClass *	list_ctrl,
	int					item_index1,
	int					item_index2,
	uint32				user_param
)
{

	ServerSettingsClass *config1 = (ServerSettingsClass *)list_ctrl->Get_Entry_Data (item_index1, 0);
	ServerSettingsClass *config2 = (ServerSettingsClass *)list_ctrl->Get_Entry_Data (item_index2, 0);

	if (config1 == NULL) {
		return(1);
	} else if (config2 == NULL) {
		return(-1);
	} else {

		//
		//	Sort based on the type of configuration
		//
		if (!config1->IsCustom && config2->IsCustom) {
			return(-1);
		}
		if (config1->IsCustom && !config2->IsCustom) {
			return(1);
		}

		if (config1->Is_Default_Custom() && !config2->Is_Default_Custom()) {
			return(-1);
		}

		if (!config1->Is_Default_Custom() && config2->Is_Default_Custom()) {
			return(1);
		}
	}

	return(wcsicmp(config1->ConfigName, config2->ConfigName));
}


////////////////////////////////////////////////////////////////
//
//	On_EditCtrl_Enter_Pressed
//
////////////////////////////////////////////////////////////////
void
ServerSaveLoadMenuClass::On_EditCtrl_Enter_Pressed (EditCtrlClass *edit_ctrl, int ctrl_id)
{
	if (ctrl_id == IDC_NAME_EDIT) {
		Save_Config (true);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	HandleNotification
//
////////////////////////////////////////////////////////////////
void
ServerSaveLoadMenuClass::HandleNotification (DlgMsgBoxEvent &event)
{
	//
	//	The user has confirmed the overwrite, so save the configuration
	//
	if (event.Event () == DlgMsgBoxEvent::Yes) {
		Save_Now();
	}
}



////////////////////////////////////////////////////////////////
//
//	Save_Now
//
////////////////////////////////////////////////////////////////
void ServerSaveLoadMenuClass::Save_Now(void)
{

	ListCtrlClass *list_ctrl = (ListCtrlClass *)Get_Dlg_Item (IDC_LIST_CTRL);
	if (list_ctrl == NULL) {
		return ;
	}

	//
	//	Get the current selection
	//
	int curr_sel = list_ctrl->Get_Curr_Sel();
	if (curr_sel != -1) {

		//
		//	Get the configuration object associated with this entry
		//
		ServerSettingsClass *config = (ServerSettingsClass *)list_ctrl->Get_Entry_Data(curr_sel, 0);

		if (config != NULL) {
			//
			//	We can only save custom configurations...
			//
			if (config->IsCustom) {
				WideStringClass outMsg;

				if (The_Game()->Is_Valid_Settings(outMsg, true)) {
					//
					//	Get the new display name for this configuration
					//
					const wchar_t *display_name = Get_Dlg_Item_Text (IDC_NAME_EDIT);

					if (display_name[0] != 0) {
						//
						//	Update the name of this configuration
						//
						config->ConfigName = display_name;
						list_ctrl->Set_Entry_Text (curr_sel, 0, display_name);

						//
						//	Save the configuration
						//
						ServerSettingsManagerClass::Save_Configuration(config);
						End_Dialog ();
						Next_Dialog();
					}
				}
			}
		}
	}
}









/***********************************************************************************************
 * ServerSettingsClass::ServerSettingsClass -- Class constructor                               *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    File name of config file                                                          *
 *           Name of config file                                                               *
 *           File number                                                                       *
 *                                                                                             *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/17/2001 5:11PM ST : Created                                                            *
 *=============================================================================================*/
ServerSettingsClass::ServerSettingsClass(const char *filename, const wchar_t *configname, int file_number)
{
	ConfigName = configname;	//"Default C&C Server Settings";
	RawFileName = filename;		//"svrcfg_cnc.ini"
	FileNumber = file_number;
	IsCustom = true;
}


/***********************************************************************************************
 * ServerSettingsClass::ServerSettingsClass -- Class copy constructor                          *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Other class                                                                       *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/17/2001 5:11PM ST : Created                                                            *
 *=============================================================================================*/
ServerSettingsClass::ServerSettingsClass(ServerSettingsClass *other)
{
	ConfigName = other->ConfigName;
	RawFileName = other->RawFileName;
	FileNumber = other->FileNumber;
	IsCustom = other->IsCustom;
}


/***********************************************************************************************
 * ServerSettingsManagerClass::Get_Settings -- Get settings by index                           *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Index                                                                             *
 *                                                                                             *
 * OUTPUT:   Settings                                                                          *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/17/2001 5:12PM ST : Created                                                            *
 *=============================================================================================*/
ServerSettingsClass *ServerSettingsManagerClass::Get_Settings(int index)
{
	WWASSERT(index >= 0 && index < ServerSettingsList.Count());
	return(ServerSettingsList[index]);
}



/***********************************************************************************************
 * ServerSettingsManagerClass::Scan -- Scan for server settings files on the disk              *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/17/2001 5:13PM ST : Created                                                            *
 *=============================================================================================*/
void ServerSettingsManagerClass::Scan(void)
{
	char file_name[MAX_PATH];
	char whole_file_name[MAX_PATH];
	//char description[256];
	WideStringClass description(128, true);
	StringClass char_description(128, true);

	/*
	** Clear out old configs.
	*/
	Clear_Settings_List();

	/*
	** Add in the default as the first entry.
	*/
	ServerSettingsList.Add(new ServerSettingsClass("def_svrcfg_cnc.ini", TRANSLATE(IDS_SERVER_SAVELOAD_DEFAULT), 0));
	ServerSettingsList[0]->IsCustom = false;

	/*
	** Add in the custom default as the second entry.
	*/
	ServerSettingsList.Add(new ServerSettingsClass("svrcfg_cnc.ini", TRANSLATE(IDS_SERVER_SAVELOAD_CUSTOM_DEFAULT), 1));
	ServerSettingsList[1]->IsCustom = true;

	for (int i=2 ; i<MAX_SETTINGS_FILES ; i++) {

		sprintf(file_name, DEFAULT_SERVER_SETTINGS_FILE_NAME, i);
		sprintf(whole_file_name, "data\\%s", file_name);
		RawFileClass file(whole_file_name);
		if (file.Is_Available()) {
			INIClass *ini = Get_INI(file_name);
			if (ini) {
				description = ini->Get_Wide_String(description, "Settings", "wConfigName", L"");	//(unsigned short *)TRANSLATE(IDS_SERVER_SAVELOAD_DEFAULT));
				if (description.Get_Length()) {
					ServerSettingsList.Add(new ServerSettingsClass(file_name, description.Peek_Buffer(), i));
				} else {
					StringClass defaultstr(TRANSLATE(IDS_SERVER_SAVELOAD_DEFAULT), true);
					ini->Get_String("Settings", "bConfigName", defaultstr.Peek_Buffer(), char_description.Peek_Buffer(), 128);
					ServerSettingsList.Add(new ServerSettingsClass(file_name, WideStringClass(char_description, true).Peek_Buffer(), i));
				}
			}
		}
	}
}




/***********************************************************************************************
 * ServerSettingsManagerClass::Clear_Settings_List -- Reset the settings list                  *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/17/2001 5:13PM ST : Created                                                            *
 *=============================================================================================*/
void ServerSettingsManagerClass::Clear_Settings_List(void)
{
	for (int i=0 ; i<ServerSettingsList.Count() ; i++) {
		delete ServerSettingsList[i];
	}
	ServerSettingsList.Reset_Active();
}



/***********************************************************************************************
 * ServerSettingsManagerClass::Load_Settings -- Load settings into the game                    *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Settings                                                                          *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/17/2001 5:14PM ST : Created                                                            *
 *=============================================================================================*/
void ServerSettingsManagerClass::Load_Settings(ServerSettingsClass *settings)
{
	WWASSERT(settings != NULL);
	WWASSERT(The_Game() != NULL);

	if (settings && The_Game()) {
		char filename[MAX_PATH];
		sprintf(filename, "data\\%s", settings->RawFileName.Peek_Buffer());
		RawFileClass file(filename);
		if (file.Is_Available()) {
			The_Game()->Set_Ini_Filename(settings->RawFileName);
			The_Game()->Load_From_Server_Config();
			settings->ConfigName = The_Game()->Get_Settings_Description();
		}
	}
}


/***********************************************************************************************
 * ServerSettingsManagerClass::Delete_Configuration -- Delete a settings file from disk        *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Settings                                                                          *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/17/2001 5:14PM ST : Created                                                            *
 *=============================================================================================*/
void ServerSettingsManagerClass::Delete_Configuration(ServerSettingsClass *settings)
{
	if (!settings->Is_Default()) {
		char filename[MAX_PATH];
		sprintf(filename, "data\\%s", settings->RawFileName.Peek_Buffer());
		DeleteFileA(filename);
		for (int i=0 ; i<ServerSettingsList.Count() ; i++) {
			if (strcmp(settings->RawFileName, ServerSettingsList[i]->RawFileName) == 0) {
				delete ServerSettingsList[i];
				ServerSettingsList.Delete(i);
				break;
			}
		}
	}
}



/***********************************************************************************************
 * ServerSettingsManagerClass::Save_Configuration -- Save settings from the game               *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Settings                                                                          *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/17/2001 5:14PM ST : Created                                                            *
 *=============================================================================================*/
void ServerSettingsManagerClass::Save_Configuration(ServerSettingsClass *settings)
{
	WWASSERT(settings != NULL);
	WWASSERT(The_Game() != NULL);

	if (settings && The_Game()) {
		char filename[MAX_PATH];
		sprintf(filename, "data\\%s", settings->RawFileName.Peek_Buffer());
		RawFileClass file(filename);
		if (!file.Is_Available()) {
			file.Create();
			WWASSERT(file.Is_Available());
		}
		if (file.Is_Available()) {
			//StringClass string(128, true);
			//settings->ConfigName.Convert_To(string);
			The_Game()->Set_Settings_Description(settings->ConfigName);
			The_Game()->Set_Ini_Filename(settings->RawFileName);
			The_Game()->Save_To_Server_Config();
		}
	}
}


/***********************************************************************************************
 * ServerSettingsManagerClass::Add_Configuration -- Create a new settings file entry           *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Settings name                                                                     *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/17/2001 5:15PM ST : Created                                                            *
 *=============================================================================================*/
ServerSettingsClass *ServerSettingsManagerClass::Add_Configuration(WideStringClass *display_name)
{
	WWASSERT(display_name != NULL);
	WWASSERT(The_Game());

	if (display_name != NULL) {
		if (The_Game()) {

			/*
			** Find an unused filename.
			*/
			char population[MAX_SETTINGS_FILES];
			memset(population, 0, sizeof(population));
			char filename[MAX_PATH];
			int file_number = -1;
			int i;

			for (i=0 ; i<ServerSettingsList.Count() ; i++) {
				ServerSettingsClass *settings = ServerSettingsList[i];
				if (settings) {
					WWASSERT(settings->FileNumber >= 0);
					WWASSERT(settings->FileNumber < MAX_SETTINGS_FILES);
					population[settings->FileNumber] = 1;
				}
			}

			for (i=0 ; i<MAX_SETTINGS_FILES ; i++) {
				if (population[i] == 0) {
					sprintf(filename, DEFAULT_SERVER_SETTINGS_FILE_NAME, i);
					file_number = i;
					break;
				}
			}

			if (file_number != -1) {
				ServerSettingsClass *new_settings = new ServerSettingsClass(filename, display_name->Peek_Buffer(), file_number);
				ServerSettingsList.Add(new_settings);
				return(new_settings);
			}
		}
	}
	return(NULL);
}
