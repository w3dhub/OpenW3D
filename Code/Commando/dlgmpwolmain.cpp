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
 *                     $Archive:: /Commando/Code/Commando/dlgmpwolmain.cpp      $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/11/02 8:57p                                               $*
 *                                                                                             *
 *                    $Revision:: 46                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "dlgmpwolmain.h"
#include "renegadedialog.h"
#include "shortcutbarctrl.h"
#include "renegadedialogmgr.h"
#include "translatedb.h"
#include "string_ids.h"
#include "dialogmgr.h"
#include "DlgMessageBox.h"
#include "DlgDownload.h"
#include "DlgQuickmatch.h"
#include "dlgmpwolgamelist.h"
#include "mpsettingsmgr.h"
#include "dlgsidebarhelp.h"
#include "WOLLoginProfile.h"
#include "bandwidthcheck.h"
#include "AutoStart.h"

////////////////////////////////////////////////////////////////
//	Static member initialization
////////////////////////////////////////////////////////////////
MPWolMainMenuClass *	MPWolMainMenuClass::_TheInstance	= NULL;


////////////////////////////////////////////////////////////////
//
//	MPWolMainMenuClass
//
////////////////////////////////////////////////////////////////
MPWolMainMenuClass::MPWolMainMenuClass (void)	:
	IsSidebarHelpPending (true),
	mPendingCmd(-1),
	MenuDialogClass (GetRenegadeDialog(RenegadeDialogID::IDD_MP_WOL_MAIN))
{
	_TheInstance = this;
	return ;
}


////////////////////////////////////////////////////////////////
//
//	~MPWolMainMenuClass
//
////////////////////////////////////////////////////////////////
MPWolMainMenuClass::~MPWolMainMenuClass (void)
{
	_TheInstance = NULL;
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Init_Dialog
//
////////////////////////////////////////////////////////////////
void
MPWolMainMenuClass::On_Init_Dialog (void)
{
	//
	//	Configure the shortcut bar
	//
	ShortcutBarCtrlClass *bar = (ShortcutBarCtrlClass *)Get_Dlg_Item (IDC_SHORTCUT_BAR);

	if (bar != NULL) {
		bar->Add_Button(IDC_MP_SHORTCUT_BUDDIES, TRANSLATE(IDS_MP_SHORTCUT_BUDDIES));
		bar->Add_Button(IDC_MP_SHORTCUT_INTERNET_OPTIONS, TRANSLATE(IDS_INTERNET_OPTIONS));

		#ifdef QUICKMATCH_OPTIONS
		bar->Add_Button(IDC_MP_SHORTCUT_QUICKMATCH_OPTIONS, TRANSLATE(IDS_MENU_TEXT364));
		#endif

		bar->Add_Button(IDC_MP_SHORTCUT_NEWS, TRANSLATE(IDS_MP_SHORTCUT_NEWS));
		bar->Add_Button(IDC_MP_SHORTCUT_CLANS, TRANSLATE(IDS_MP_SHORTCUT_CLANS));
		bar->Add_Button(IDC_MP_SHORTCUT_RANKINGS, TRANSLATE(IDS_MP_SHORTCUT_RANKINGS));
		bar->Add_Button(IDC_MP_SHORTCUT_ACCOUNT, TRANSLATE(IDS_MP_SHORTCUT_ACCOUNT));
		bar->Add_Button(IDC_MP_SHORTCUT_NET_STATUS, TRANSLATE(IDS_MP_SHORTCUT_NET_STATUS));

		//
		//	Force the bar to be displayed
		//
		if (MPSettingsMgrClass::Is_Sidebar_Help_Displayed ()) {
			bar->Display (true);
		}
	}

	MenuDialogClass::On_Init_Dialog ();

	bool wolValid = CheckWOLVersion();

	if (!wolValid) {
		DlgMsgBox::DoDialog(IDS_WOL_ERROR, IDS_WOL_WRONGVERSION);
		End_Dialog();
	}
}


// Check the version of the Westwood online component
bool MPWolMainMenuClass::CheckWOLVersion(void)
{
	RefPtr<WWOnline::Session> wolSession = WWOnline::Session::GetInstance(false);

	if (!wolSession.IsValid()) {
		return false;
	}

	// WOLAPI version 1.19.3 or better required.
	const LONG minVersion = MAKELONG(19,1);
	const LONG minBuild = MAKELONG(0,3);

	unsigned int wolVersion = 0;
	unsigned int wolBuild = 0;
	wolSession->GetChatObject()->GetVersion(&wolVersion);

	WideStringClass wolText(255, true);

	if (BandwidthCheckerClass::Get_Reported_Upstream_Bandwidth()) {
		WideStringClass conn(BandwidthCheckerClass::Get_Bandwidth_As_String(), true);
		wolText.Format(TRANSLATE(IDS_MENU_CONNECTION_SPEED_FORMAT), conn);
	}

	WideStringClass string(0, true);
	string.Format(U_CHAR("WOLAPI V%u.%u"), HIWORD(wolVersion), LOWORD(wolVersion));
	wolText += string;

	char buildString[32] = {0};
	LPCSTR value = (LPCSTR)(&buildString[0]);
	HRESULT hr = wolSession->GetChatObject()->GetAttributeValue("BuildNumber", &value);

	if (SUCCEEDED(hr)) {
		wolBuild = atol(buildString);
		string.Format(U_CHAR(".%u"), HIWORD(wolBuild));
		wolText += string;
	}

	Set_Dlg_Item_Text(IDC_WOLAPI_VER, wolText);

	// Are we below the minimum version
	if (wolVersion < minVersion) {
		return false;
	}

	// If the version is the same then are we below the minimum build
	if ((wolVersion == minVersion) && (wolBuild < minBuild)) {
		return false;
	}

	return true;
}


////////////////////////////////////////////////////////////////
//
//	On_Frame_Update
//
////////////////////////////////////////////////////////////////
void
MPWolMainMenuClass::On_Frame_Update (void)
{
	if (IsSidebarHelpPending) {
		IsSidebarHelpPending = false;

		//
		//	Show the sidebar help (if necessary)
		//
		if (MPSettingsMgrClass::Is_Sidebar_Help_Displayed ()) {
			START_DIALOG (SidebarHelpDialogClass);
		}
	}

	MenuDialogClass::On_Frame_Update ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Command
//
////////////////////////////////////////////////////////////////
void
MPWolMainMenuClass::On_Command (int ctrl_id, int message_id, unsigned int param)
{
	switch (ctrl_id)
	{
		case IDC_MP_WOL_QUICK_MATCH_BUTTON:
		case IDC_MP_WOL_ADVANCED_LISTINGS_BUTTON:  // intentional fall through
		case IDC_MP_SHORTCUT_CHAT:                 // intentional fall through
		case IDC_MP_SHORTCUT_BUDDIES:              // intentional fall through
			mPendingCmd = ctrl_id;
			WOLLogonMgr::Logon(this);
			return;
			break;
	}

	MenuDialogClass::On_Command (ctrl_id, message_id, param);
	return ;
}


void MPWolMainMenuClass::HandleNotification(WOLLogonAction& action)
{
	if (WOLLOGON_SUCCESS == action) {
		switch (mPendingCmd) {
			case IDC_MP_WOL_ADVANCED_LISTINGS_BUTTON:
				MPWolGameListMenuClass::DoDialog();
				break;

			case IDC_MP_WOL_QUICK_MATCH_BUTTON:
				DlgQuickMatch::DoDialog();
				break;

			case -1:
				break;

			default:
				MenuDialogClass::On_Command(mPendingCmd, 0, 0);
				break;
		}
	} else if (WOLLOGON_PATCHREQUIRED == action) {

		// If a patch is required, ask the user if they want to download it now.
		DlgMsgBox::DoDialog(TRANSLATE(IDS_WOL_PATCHREQUIRED),
			TRANSLATE(IDS_WOL_DOWNLOADPROMPT), DlgMsgBox::YesNo, this);
	}

	mPendingCmd = -1;
}


void MPWolMainMenuClass::HandleNotification(DlgMsgBoxEvent& event)
{
	if (DlgMsgBoxEvent::Yes == event.Event()) {
		RefPtr<WWOnline::Session> wolSession = WWOnline::Session::GetInstance(false);

		WWOnline::DownloadList& patchList = wolSession->GetPatchDownloadList();
		bool quiet = AutoRestart.Is_Active();

		DlgDownload::DoDialog(TRANSLATE(IDS_WOL_DOWNLOAD), patchList, quiet);
	}
}


////////////////////////////////////////////////////////////////
//
//	Display
//
////////////////////////////////////////////////////////////////
void
MPWolMainMenuClass::Display (void)
{
	//
	//	Create the dialog if necessary, otherwise simply bring it to the front
	//
	if (_TheInstance == NULL) {
		START_DIALOG (MPWolMainMenuClass);
	} else {
		if (_TheInstance->Is_Active_Menu () == false) {
			DialogMgrClass::Rollback (_TheInstance);
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Activate
//
////////////////////////////////////////////////////////////////
void
MPWolMainMenuClass::On_Activate(bool onoff)
{
	MenuDialogClass::On_Activate(onoff);

	if (onoff) {
		Update_Login_Profile();
		WOLLogonMgr::Logoff();
	}
}


////////////////////////////////////////////////////////////////
//
//	On_Last_Menu_Ending
//
////////////////////////////////////////////////////////////////
void
MPWolMainMenuClass::On_Last_Menu_Ending (void)
{
	RenegadeDialogMgrClass::Goto_Location (RenegadeDialogMgrClass::LOC_MAIN_MENU);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Update_Login_Profile
//
////////////////////////////////////////////////////////////////
void
MPWolMainMenuClass::Update_Login_Profile(void)
{
	WideStringClass lastlogin(64, true);
	lastlogin = MPSettingsMgrClass::Get_Last_Login();

	LoginProfile* profile = LoginProfile::Get(lastlogin);
	ShowProfileRanking(this, profile);
	
	if (profile) {
		profile->Release_Ref();
	}
}
