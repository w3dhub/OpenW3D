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
 *                     $Archive:: /Commando/Code/Commando/dlgserversaveload.h                 $*
 *                                                                                             *
 *                       Author:: Steve T                                                      *
 *                                                                                             *
 *                     $Modtime:: 12/17/01 1:14p                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef _DLG_SERVER_SAVELOAD_H
#define _DLG_SERVER_SAVELOAD_H


#include "menudialog.h"
#include "dlgmessagebox.h"
#include "translatedb.h"
#include "string_ids.h"


////////////////////////////////////////////////////////////////
//	Forward declarations
////////////////////////////////////////////////////////////////
class ServerSettingsClass;


////////////////////////////////////////////////////////////////
//
//	ControlSaveLoadMenuClass
//
////////////////////////////////////////////////////////////////
class ServerSaveLoadMenuClass : public MenuDialogClass, public Observer<DlgMsgBoxEvent>
{
public:

	//////////////////////////////////////////////////////////////
	//	Public constructors/destructor
	//////////////////////////////////////////////////////////////
	ServerSaveLoadMenuClass (void);

	//////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////

	//
	//	Inherited
	//
	void		On_Init_Dialog (void);
	void		On_Command (int ctrl_id, int message_id, DWORD param);
	void		On_ListCtrl_Delete_Entry (ListCtrlClass *list_ctrl, int ctrl_id, int item_index);
	void		On_ListCtrl_Sel_Change (ListCtrlClass *list_ctrl, int	ctrl_id, int old_index, int	new_index);
	void		On_EditCtrl_Enter_Pressed (EditCtrlClass *edit_ctrl, int ctrl_id);
	void		Save_Now(void);

	static void Set_From_Slave_Config(bool set) {FromSlaveConfig = set;}
	static void Next_Dialog(void);

private:

	////////////////////////////////////////////////////////////////
	//	Static methods
	////////////////////////////////////////////////////////////////
	static int CALLBACK ListSortCallback (ListCtrlClass *list_ctrl, int item_index1, int item_index2, uint32 user_param);

	//////////////////////////////////////////////////////////////
	//	Private methods
	//////////////////////////////////////////////////////////////
	void		Delete_Config (void);
	void		Load_Config (void);
	void		Save_Config (bool prompt);
	int		Insert_Configuration (ServerSettingsClass *config);
	void		HandleNotification (DlgMsgBoxEvent &event);

	static	bool FromSlaveConfig;

	bool		YesNo;
};





#define MAX_SETTINGS_FILES 500



class ServerSettingsClass
{
	public:

		ServerSettingsClass(const char *filename = "svrcfg_cnc.ini", const wchar_t *configname = TRANSLATE(IDS_SERVER_SAVELOAD_DEFAULT), int file_number = 0);
		ServerSettingsClass(ServerSettingsClass *other);
		bool Is_Default(void) {return(FileNumber == 0);}
		bool Is_Default_Custom(void) {return(FileNumber == 1);}

		StringClass			RawFileName;
		WideStringClass	ConfigName;


		int			FileNumber;
		bool			IsCustom;
};


class ServerSettingsManagerClass
{
	public:
		static void Scan(void);
		static int Get_Num_Settings_Files(void) {return(ServerSettingsList.Count());}
		static ServerSettingsClass *Get_Settings(int index);
		static void Load_Settings(ServerSettingsClass *settings);
		static void Delete_Configuration(ServerSettingsClass *settings);
		static void Save_Configuration(ServerSettingsClass *settings);
		static ServerSettingsClass *Add_Configuration(WideStringClass *display_name);


	private:
		static DynamicVectorClass<ServerSettingsClass*> ServerSettingsList;
		static void Clear_Settings_List(void);

};


#endif //_DLG_CONTROL_SAVELOAD_H