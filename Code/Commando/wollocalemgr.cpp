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
 *                 Project Name : commando                                                     *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Commando/wollocalemgr.cpp                    $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/19/02 2:25p                                               $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "wollocalemgr.h"
#include "WOLLoginProfile.h"
#include "WOLLogonMgr.h"
#include "dlgmpwollocalenag.h"
#include "comboboxctrl.h"
#include "mpsettingsmgr.h"
#include "string_ids.h"
#include "translatedb.h"


//////////////////////////////////////////////////////////////////////
//	Local constants
//////////////////////////////////////////////////////////////////////
static const int LOC_STRING_ARRAY[] =
{
	IDS_LOCALE_UNKNOWN,
	IDS_LOCALE_OTHER,
	IDS_LOCALE_USA,
	IDS_LOCALE_CANADA,
	IDS_LOCALE_UK,
	IDS_LOCALE_GERMANY,
	IDS_LOCALE_FRANCE,
	IDS_LOCALE_SPAIN,
	IDS_LOCALE_NETHERLANDS,
	IDS_LOCALE_BELGIUM,
	IDS_LOCALE_AUSTRIA,
	IDS_LOCALE_SWITZERLAND,
	IDS_LOCALE_ITALY,
	IDS_LOCALE_DENMARK,
	IDS_LOCALE_SWEDEN,
	IDS_LOCALE_NORWAY,
	IDS_LOCALE_FINLAND,
	IDS_LOCALE_ISRAEL,
	IDS_LOCALE_SOUTH_AFRICA,
	IDS_LOCALE_JAPAN,
	IDS_LOCALE_SOUTH_KOREA,
	IDS_LOCALE_CHINA,
	IDS_LOCALE_SINGAPORE,
	IDS_LOCALE_TAIWAN,
	IDS_LOCALE_MALAYSIA,
	IDS_LOCALE_AUSTRALIA,
	IDS_LOCALE_NEW_ZEALAND,
	IDS_LOCALE_BRAZIL,
	IDS_LOCALE_THAILAND,
	IDS_LOCALE_ARGENTINA,
	IDS_LOCALE_PHILIPPINES,
	IDS_LOCALE_GREECE,
	IDS_LOCALE_IRELAND,
	IDS_LOCALE_POLAND,
	IDS_LOCALE_PORTUGAL,
	IDS_LOCALE_MEXICO,
	IDS_LOCALE_RUSSIA,
	IDS_LOCALE_TURKEY
};

static const int LOC_COUNT = sizeof (LOC_STRING_ARRAY) / sizeof (LOC_STRING_ARRAY[0]);


//////////////////////////////////////////////////////////////////////
//
//	Get_Locale
//
//////////////////////////////////////////////////////////////////////
WOL::Locale WolLocaleMgrClass::Get_Locale(const unichar_t *login_name)
{
	WOL::Locale locale = WOL::LOC_UNKNOWN;

	// Get the login information for this name
	LoginProfile* profile = LoginProfile::Get(login_name, false);

	if (profile) {
		locale = profile->GetLocale();
		profile->Release_Ref();
	}

	return locale;
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Locale
//
//////////////////////////////////////////////////////////////////////
void WolLocaleMgrClass::Set_Locale(const unichar_t *login_name, WOL::Locale locale)
{
	if (WOL::LOC_UNKNOWN != locale) {
		LoginProfile* profile = LoginProfile::Get(login_name, false);
		
		if (profile) {
			profile->SetLocale(locale);
			profile->SaveSettings();
			profile->Release_Ref();
		}
	}
}


//////////////////////////////////////////////////////////////////////
//
//	Get_Current_Locale
//
//////////////////////////////////////////////////////////////////////
WOL::Locale WolLocaleMgrClass::Get_Current_Locale(void)
{
	WideStringClass loginName(0, true);
	WOLLogonMgr::GetLoginName(loginName);
	return Get_Locale(loginName);
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Current_Locale
//
//////////////////////////////////////////////////////////////////////
void WolLocaleMgrClass::Set_Current_Locale(WOL::Locale locale)
{
	WideStringClass loginName(0, true);
	WOLLogonMgr::GetLoginName(loginName);
	Set_Locale(loginName, locale);
}


//////////////////////////////////////////////////////////////////////
//
//	Get_Locale_String
//
//////////////////////////////////////////////////////////////////////
const unichar_t* WolLocaleMgrClass::Get_Locale_String(WOL::Locale locale)
{
	WWASSERT(locale < LOC_COUNT && "Invalid locale");
	return TRANSLATE(LOC_STRING_ARRAY[locale]);
}


//////////////////////////////////////////////////////////////////////
//
//	Configure_Locale_Combobox
//
//////////////////////////////////////////////////////////////////////
void WolLocaleMgrClass::Configure_Locale_Combobox(ComboBoxCtrlClass *ctrl)
{
	if (ctrl) {
		ctrl->Reset_Content();

		//	Add an entry for each locale to the combobox
		for (int index = 0; index < LOC_COUNT; ++index) {
			ctrl->Add_String(TRANSLATE(LOC_STRING_ARRAY[index]));
		}

		// Get the last login
		WideStringClass wide_nickname(64, true);
		const char* nickname = MPSettingsMgrClass::Get_Last_Login();
		wide_nickname.Convert_From(nickname);
	
		//	Select this locale in the combobox
		int locale = Get_Locale(wide_nickname);
		ctrl->Set_Curr_Sel(locale);
	}
}


//////////////////////////////////////////////////////////////////////
//
//	Display_Nag_Dialog
//
//////////////////////////////////////////////////////////////////////
void WolLocaleMgrClass::Display_Nag_Dialog(void)
{
	WOL::Locale locale = Get_Current_Locale();

	if (WOL::LOC_UNKNOWN == locale) {
		START_DIALOG(MPWolLocaleNagDialogClass);
	}
}
