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
 *                     $Archive:: /Commando/Code/Commando/dlgmpslaveservers.cpp               $*
 *                                                                                             *
 *                       Author:: Steve Tall                                                   *
 *                                                                                             *
 *                     $Modtime:: 2/11/02 11:03a                                              $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"
#include "dlgmpslaveservers.h"
#include "menudialog.h"
#include "_globals.h"
#include "slavemaster.h"
#include "dlgserversaveload.h"
#include "rawfile.h"
#include "ini.h"
#include "assets.h"



unsigned long SlaveServerDialogClass::EnableIDs[MAX_SLAVES] = {
	IDC_SLAVE_ENABLE1,
	IDC_SLAVE_ENABLE2,
	IDC_SLAVE_ENABLE3,
	IDC_SLAVE_ENABLE4,
	IDC_SLAVE_ENABLE5,
	IDC_SLAVE_ENABLE6,
	IDC_SLAVE_ENABLE7
};


unsigned long SlaveServerDialogClass::NickIDs[MAX_SLAVES] = {
	IDC_NICK_EDIT1,
	IDC_NICK_EDIT2,
	IDC_NICK_EDIT3,
	IDC_NICK_EDIT4,
	IDC_NICK_EDIT5,
	IDC_NICK_EDIT6,
	IDC_NICK_EDIT7
};

unsigned long SlaveServerDialogClass::PassIDs[MAX_SLAVES] = {
	IDC_PASS_EDIT1,
	IDC_PASS_EDIT2,
	IDC_PASS_EDIT3,
	IDC_PASS_EDIT4,
	IDC_PASS_EDIT5,
	IDC_PASS_EDIT6,
	IDC_PASS_EDIT7
};


unsigned long SlaveServerDialogClass::SerialIDs[MAX_SLAVES] = {
	IDC_SERIAL_EDIT1,
	IDC_SERIAL_EDIT2,
	IDC_SERIAL_EDIT3,
	IDC_SERIAL_EDIT4,
	IDC_SERIAL_EDIT5,
	IDC_SERIAL_EDIT6,
	IDC_SERIAL_EDIT7
};



unsigned long SlaveServerDialogClass::PortIDs[MAX_SLAVES] = {
	IDC_PORT_EDIT1,
	IDC_PORT_EDIT2,
	IDC_PORT_EDIT3,
	IDC_PORT_EDIT4,
	IDC_PORT_EDIT5,
	IDC_PORT_EDIT6,
	IDC_PORT_EDIT7
};



unsigned long SlaveServerDialogClass::SettingsButtons[MAX_SLAVES] = {
	IDC_SLAVE1_SETTINGS,
	IDC_SLAVE2_SETTINGS,
	IDC_SLAVE3_SETTINGS,
	IDC_SLAVE4_SETTINGS,
	IDC_SLAVE5_SETTINGS,
	IDC_SLAVE6_SETTINGS,
	IDC_SLAVE7_SETTINGS
};


char SlaveServerDialogClass::ServerSettingsFileNames[MAX_SLAVES][MAX_PATH];


int SlaveServerDialogClass::SlaveNumber = 0;
SlaveServerDialogClass *SlaveServerDialogClass::Instance = NULL;



////////////////////////////////////////////////////////////////
//
//	CheatOptionsMenuClass
//
////////////////////////////////////////////////////////////////
SlaveServerDialogClass::SlaveServerDialogClass(void) :
	MenuDialogClass(IDD_MP_WOL_SLAVESERVER)
{
	Instance = this;
}


void
SlaveServerDialogClass::On_Init_Dialog (void)
{
	/*
	** Init the slave master.
	*/
	SlaveMaster.Load();

	int num_slaves = SlaveMaster.Get_Num_Slaves();

	char nick[64];
	char serial[64];
	char pass[64];
	bool enable;
	unsigned short port;
	char settings_file_name[MAX_PATH];
	int bw;

	/*
	** Set up the dialog entries for each slave.
	*/
	for (int i=0 ; i<MAX_SLAVES ; i++) {

		nick[0] = 0;
		serial[0] = 0;
		enable = false;
		port = 0;
		settings_file_name[0] = 0;
		bw = 0;

		if (i < num_slaves) {
			SlaveServerClass *slave = SlaveMaster.Get_Slave(i);
			if (slave != NULL) {
				slave->Get(enable, nick, serial, port, settings_file_name, bw, pass);
			}
		}
		Check_Dlg_Button (EnableIDs[i], enable);

		WideStringClass tempnick(nick, true);
		Set_Dlg_Item_Text(NickIDs[i], tempnick.Peek_Buffer());

		WideStringClass temppass(pass, true);
		Set_Dlg_Item_Text(PassIDs[i], temppass.Peek_Buffer());

		WideStringClass tempser(serial, true);
		Set_Dlg_Item_Text(SerialIDs[i], tempser.Peek_Buffer());

		char temp[32];
		_itoa((unsigned long)port, temp, 10);
		WideStringClass tempport(temp, true);
		Set_Dlg_Item_Text(PortIDs[i], tempport.Peek_Buffer());

		strcpy(ServerSettingsFileNames[i], settings_file_name);
		Set_Slave_Button(i);
	}

	MenuDialogClass::On_Init_Dialog ();
}


void
SlaveServerDialogClass::On_Command (int ctrl_id, int message_id, DWORD param)
{
	switch (ctrl_id) {
		case IDC_MENU_BACK_BUTTON:
		{
			SlaveMaster.Reset();
			char *nick = "";
			char *pass = "";
			char *serial = "";
			bool enable = false;

			for (int i=0 ; i<MAX_SLAVES ; i++) {
				enable = Is_Dlg_Button_Checked(EnableIDs[i]);

				WideStringClass tempnick(Get_Dlg_Item_Text(NickIDs[i]));
				StringClass newnick;
				tempnick.Convert_To(newnick);
				nick = newnick.Peek_Buffer();

				WideStringClass temppass(Get_Dlg_Item_Text(PassIDs[i]));
				StringClass newpass;
				temppass.Convert_To(newpass);
				pass = newpass.Peek_Buffer();

				WideStringClass tempser(Get_Dlg_Item_Text(SerialIDs[i]));
				StringClass newser;
				tempser.Convert_To(newser);
				serial = newser.Peek_Buffer();

				WideStringClass tempport(Get_Dlg_Item_Text(PortIDs[i]));
				StringClass newport;
				tempport.Convert_To(newport);
				unsigned long port = atoi(newport.Peek_Buffer());
				if (port > 0xffff) {
					port = 0xffff;
				}

				SlaveMaster.Add_Slave(enable, nick, serial, (unsigned short)port, ServerSettingsFileNames[i], 0xffffffff, pass);
			}
			SlaveMaster.Save();
		}
		break;

		case IDC_SLAVE1_SETTINGS:
			Load_Settings(1);
			break;

		case IDC_SLAVE2_SETTINGS:
			Load_Settings(2);
			break;

		case IDC_SLAVE3_SETTINGS:
			Load_Settings(3);
			break;

		case IDC_SLAVE4_SETTINGS:
			Load_Settings(4);
			break;

		case IDC_SLAVE5_SETTINGS:
			Load_Settings(5);
			break;

		case IDC_SLAVE6_SETTINGS:
			Load_Settings(6);
			break;

		case IDC_SLAVE7_SETTINGS:
			Load_Settings(7);
			break;

	}

	MenuDialogClass::On_Command (ctrl_id, message_id, param);
}




void SlaveServerDialogClass::Load_Settings(int slave_number)
{
	SlaveNumber = slave_number;
	ServerSaveLoadMenuClass::Set_From_Slave_Config(true);
	START_DIALOG(ServerSaveLoadMenuClass);
}


void SlaveServerDialogClass::Set_Slave_Settings(StringClass *file_name)
{
	if (SlaveNumber) {
		int slave = SlaveNumber-1;
		strcpy(ServerSettingsFileNames[slave], file_name->Peek_Buffer());
		Set_Slave_Button(slave);
	}
}



void SlaveServerDialogClass::Set_Slave_Button(int slavenum)
{
	char char_description[256];
	WideStringClass description;
	char file_name[MAX_PATH];

	sprintf(file_name, "data\\%s", ServerSettingsFileNames[slavenum]);
	RawFileClass file(file_name);

	if (file.Is_Available()) {

		INIClass *ini = Get_INI(ServerSettingsFileNames[slavenum]);

		if (ini) {
			description = ini->Get_Wide_String(description, "Settings", "wConfigName", L"");
			int size = description.Get_Length();
			if (size == 0) {
				size = ini->Get_String("Settings", "bConfigName", "", char_description, sizeof(char_description));
				if (size != 0) {
					description = char_description;
				} else {
					description = TRANSLATE(IDS_SERVER_SAVELOAD_DEFAULT);
					size = 1;
				}
			}
			//int size = ini->Get_String("Settings", "ConfigName", "Custom C&C Server Settings", description, sizeof(description));

			if (size) {
				Instance->Set_Dlg_Item_Text(SettingsButtons[slavenum], WideStringClass(description, true));
			}
		}
	}
}



////////////////////////////////////////////////////////////////
//
//	On_Destroy
//
////////////////////////////////////////////////////////////////
void
SlaveServerDialogClass::On_Destroy (void)
{
	MenuDialogClass::On_Destroy ();
}
