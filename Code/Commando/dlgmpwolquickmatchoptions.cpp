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

#ifdef QUICKMATCH_OPTIONS
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Combat																		  *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/commando/dlgmpwolquickmatchoptions.cpp   $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 12/14/01 5:10p                                              $*
 *                                                                                             *
 *                    $Revision:: 17                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "dlgmpwolquickmatchoptions.h"
#include "dlgmpwolbuddies.h"
#include "dlgmpwolchat.h"
#include "renegadedialogmgr.h"
#include "comboboxctrl.h"
#include "sliderctrl.h"
#include "shortcutbarctrl.h"
#include "translatedb.h"
#include "string_ids.h"
#include "gamedata.h"
#include "mpsettingsmgr.h"

////////////////////////////////////////////////////////////////
//
//	MPWolQuickMatchOptionsMenuClass
//
////////////////////////////////////////////////////////////////
MPWolQuickMatchOptionsMenuClass::MPWolQuickMatchOptionsMenuClass (void)	:
	MenuDialogClass (IDD_MP_WOL_QUICKMATCH_OPTIONS)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Init_Dialog
//
////////////////////////////////////////////////////////////////
void
MPWolQuickMatchOptionsMenuClass::On_Init_Dialog (void)
{
	WideStringClass entry_text;

	//
	//	Configure the shortcut bar
	//
	ShortcutBarCtrlClass* bar = (ShortcutBarCtrlClass*)Get_Dlg_Item(IDC_SHORTCUT_BAR);

	if (bar) {		
		bar->Add_Button(IDC_MP_SHORTCUT_BUDDIES, TRANSLATE(IDS_MP_SHORTCUT_BUDDIES));
		bar->Add_Button (IDC_MP_SHORTCUT_INTERNET_OPTIONS, TRANSLATE (IDS_INTERNET_OPTIONS));
		bar->Add_Button(IDC_MP_SHORTCUT_CHAT, TRANSLATE(IDS_MP_SHORTCUT_CHAT));
		bar->Add_Button(IDC_MP_SHORTCUT_NEWS, TRANSLATE(IDS_MP_SHORTCUT_NEWS));
		bar->Add_Button(IDC_MP_SHORTCUT_CLANS, TRANSLATE(IDS_MP_SHORTCUT_CLANS));
		bar->Add_Button(IDC_MP_SHORTCUT_RANKINGS, TRANSLATE(IDS_MP_SHORTCUT_RANKINGS));
		bar->Add_Button(IDC_MP_SHORTCUT_NET_STATUS, TRANSLATE(IDS_MP_SHORTCUT_NET_STATUS));
	}

	//	Configure the sliders
	/*
	((SliderCtrlClass*)Get_Dlg_Item(IDC_DM_SLIDER))->Set_Range(0, 10);
	const char* mode = cGameData::Get_Game_Type_Name(cGameData::GAME_TYPE_DEATHMATCH);
	int pref = MPSettingsMgrClass::Get_QuickMatch_Mode_Preference(mode);
	((SliderCtrlClass*)Get_Dlg_Item(IDC_DM_SLIDER))->Set_Pos(pref);
	Update_Slider_Text(IDC_DM_SLIDER);
	
	((SliderCtrlClass*)Get_Dlg_Item(IDC_TDM_SLIDER))->Set_Range(0, 10);
	mode = cGameData::Get_Game_Type_Name(cGameData::GAME_TYPE_TEAM_DEATHMATCH);
	pref = MPSettingsMgrClass::Get_QuickMatch_Mode_Preference(mode);
	((SliderCtrlClass*)Get_Dlg_Item(IDC_TDM_SLIDER))->Set_Pos(pref);
	Update_Slider_Text(IDC_TDM_SLIDER);

	((SliderCtrlClass*)Get_Dlg_Item(IDC_CNC_SLIDER))->Set_Range(0, 10);
	//const char * mode = cGameData::Get_Game_Type_Name(cGameData::GAME_TYPE_CNC);
	const char * mode = cGameDataCnc::Get_Game_Type_Name();
	int pref = MPSettingsMgrClass::Get_QuickMatch_Mode_Preference(mode);
	((SliderCtrlClass*)Get_Dlg_Item(IDC_CNC_SLIDER))->Set_Pos(pref);
	Update_Slider_Text(IDC_CNC_SLIDER);
	*/

	Check_Dlg_Button(IDC_REMEMBER_CHECK, false);

	MenuDialogClass::On_Init_Dialog();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Command
//
////////////////////////////////////////////////////////////////
void
MPWolQuickMatchOptionsMenuClass::On_Command (int ctrl_id, int message_id, unsigned int param)
{
	if (ctrl_id == IDOK) {
		if (Save_Settings()) {
			End_Dialog();
		}
	}

	MenuDialogClass::On_Command (ctrl_id, message_id, param);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_SliderCtrl_Pos_Changed
//
////////////////////////////////////////////////////////////////
void	
MPWolQuickMatchOptionsMenuClass::On_SliderCtrl_Pos_Changed
(
	SliderCtrlClass *	slider_ctrl,
	int					ctrl_id,
	int					new_pos
)
{
	Update_Slider_Text (ctrl_id);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Update_Slider_Text
//
////////////////////////////////////////////////////////////////
void	
MPWolQuickMatchOptionsMenuClass::Update_Slider_Text(int ctrl_id)
{
	WideStringClass new_string;

	int new_pos = ((SliderCtrlClass*)Get_Dlg_Item(ctrl_id))->Get_Pos();
	
	//	Determine which string to use for the given position
	if (new_pos <= 2) {
		new_string = TRANSLATE (IDS_MP_QM_UNDESIRABLE);
	} else if (new_pos <= 5) {
		new_string = TRANSLATE (IDS_MP_QM_DESIRABLE);
	} else if (new_pos <= 7) {
		new_string = TRANSLATE (IDS_MP_QM_VERY_DESIRABLE);
	} else {
		new_string = TRANSLATE (IDS_MP_QM_EXTREMELY_DESIRABLE);
	}

	//	Now update the associated text
	switch (ctrl_id)
	{
		case IDC_DM_SLIDER:
			Set_Dlg_Item_Text (IDC_DM_STATIC, new_string);
			break;

		case IDC_TDM_SLIDER:
			Set_Dlg_Item_Text (IDC_TDM_STATIC, new_string);
			break;

		case IDC_CTF_SLIDER:
			Set_Dlg_Item_Text (IDC_CTF_STATIC, new_string);
			break;

		case IDC_CNC_SLIDER:
			Set_Dlg_Item_Text (IDC_CNC_STATIC, new_string);
			break;
	}

	return ;
}


bool MPWolQuickMatchOptionsMenuClass::Save_Settings(void)
{
	int sum = 0;

	/*
	int pref = ((SliderCtrlClass*)Get_Dlg_Item(IDC_DM_SLIDER))->Get_Pos();
	const char* mode = cGameData::Get_Game_Type_Name(cGameData::GAME_TYPE_DEATHMATCH);
	MPSettingsMgrClass::Set_QuickMatch_Mode_Preference(mode, pref);
	sum += pref;
	
	pref = ((SliderCtrlClass*)Get_Dlg_Item(IDC_TDM_SLIDER))->Get_Pos();
	mode = cGameData::Get_Game_Type_Name(cGameData::GAME_TYPE_TEAM_DEATHMATCH);
	MPSettingsMgrClass::Set_QuickMatch_Mode_Preference(mode, pref);
	sum += pref;

	int pref = ((SliderCtrlClass*)Get_Dlg_Item(IDC_CNC_SLIDER))->Get_Pos();
	//const char * mode = cGameData::Get_Game_Type_Name(cGameData::GAME_TYPE_CNC);
	const char * mode = cGameDataCnc::Get_Game_Type_Name();
	MPSettingsMgrClass::Set_QuickMatch_Mode_Preference(mode, pref);
	sum += pref;
	*/

	if (Is_Dlg_Button_Checked(IDC_REMEMBER_CHECK)) {
		MPSettingsMgrClass::Save_QuickMatch_Mode_Preferences();
	}

	if (sum == 0) {
		DlgMsgBox::DoDialog(TRANSLATE (IDS_MENU_INVALID_QM_SETTINGS_TITLE), TRANSLATE (IDS_MENU_INVALID_QM_SETTINGS));
		return false;
	}

	return true;
}

#endif // QUICKMATCH_OPTIONS
