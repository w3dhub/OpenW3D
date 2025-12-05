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
 *                     $Archive:: /Commando/Code/Commando/dlgmainmenu.cpp       $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 2/12/02 1:28p                                               $*
 *                                                                                             *
 *                    $Revision:: 29                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "dlgmainmenu.h"
#include "assetmgr.h"
#include "renegadedialog.h"
#include "rendobj.h"
#include "hanim.h"
#include "gameinitmgr.h"
#include "mainmenutransition.h"
#include "menubackdrop.h"
#include "scene.h"
#include "renegadedialog.h"
#include "mesh.h"
#include "meshgeometry.h"
#include "dialogmgr.h"
#include "gameinitmgr.h"
#include "debug.h"
#include "dialogcontrol.h"
#include "specialbuilds.h"
#include "buildnum.h"
#include "campaign.h"
#include "gamedata.h"
#include "imagectrl.h"
#include "init.h"
#include "registry.h"
#include "_globals.h"
#include "dialogtests.h"
#include "DlgWOLWait.h"
#include "nicenum.h"
#include "DlgMessageBox.h"
#include "translatedb.h"
#include "string_ids.h"
#include "gamespyadmin.h"

////////////////////////////////////////////////////////////////
//	Static member initialization
////////////////////////////////////////////////////////////////
MainMenuDialogClass *	MainMenuDialogClass::_TheInstance	= NULL;
bool MainMenuDialogClass::Animated = true;

////////////////////////////////////////////////////////////////
//
//	MainMenuDialogClass
//
////////////////////////////////////////////////////////////////
MainMenuDialogClass::MainMenuDialogClass (void)	:
	MenuDialogClass (GetRenegadeDialog(RenegadeDialogID::IDD_MENU_MAIN)),
	TitleTransModel (NULL),
	LogoModel (NULL),
	GizmoModel (NULL),
	IsStartingPractice (false)
{
	LogoModel			= WW3DAssetManager::Get_Instance ()->Create_Render_Obj ("IF_RENLOGO");
	TitleTransModel	= WW3DAssetManager::Get_Instance ()->Create_Render_Obj ("IF_TITLETRANS");
	GizmoModel			= WW3DAssetManager::Get_Instance ()->Create_Render_Obj ("IF_EVAGIZMO");

	RegistryClass reg(APPLICATION_SUB_KEY_NAME_OPTIONS);
	if (reg.Get_Int("DisableMenuAnim", 0) != 0) {
		Animated = false;
	}


	if (TitleTransModel != NULL && GizmoModel != NULL && Animated) {

		//
		//	Play the gizmo animation
		//
		HAnimClass *gizmo_anim = WW3DAssetManager::Get_Instance ()->Get_HAnim ("IF_EVAGIZMO.IF_EVAGIZMO");
		if (gizmo_anim != NULL) {
			GizmoModel->Set_Animation (gizmo_anim, 0.0F, RenderObjClass::ANIM_MODE_LOOP);
			REF_PTR_RELEASE (gizmo_anim);
		}

		//
		//	Add the gizmo to the transition model
		//
		TitleTransModel->Add_Sub_Object_To_Bone (GizmoModel, "IF_GIZMOBONE");
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	~MainMenuDialogClass
//
////////////////////////////////////////////////////////////////
MainMenuDialogClass::~MainMenuDialogClass (void)
{
	if (TitleTransModel != NULL) {
		TitleTransModel->Remove ();
	}

	if (LogoModel != NULL) {
		LogoModel->Remove ();
	}

	REF_PTR_RELEASE (LogoModel);
	REF_PTR_RELEASE (TitleTransModel);
	REF_PTR_RELEASE (GizmoModel);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Menu_Activate
//
////////////////////////////////////////////////////////////////
void
MainMenuDialogClass::On_Menu_Activate (bool onoff)
{
	if (TitleTransModel != NULL) {
	
		//
		//	Either add or remove the logo from the scene
		//
		if (onoff) {
	
			// Put the logo pack into the scene when reactivated.
			if (LogoModel && LogoModel->Peek_Scene() == NULL) {
				if (MenuBackDropClass *backdrop = Get_BackDrop()) {
					backdrop->Peek_Scene()->Add_Render_Object(LogoModel);
				}
			}

			//
			//	Force-shutdown any interfaces that are running... (this could
			// happen if the user navigates through the multiplay menus and then
			// returns to the main menu).
			//
			GameInitMgrClass::Shutdown ();
		} else {

			//
			//	Remove the logo from the screen
			//
			if (LogoModel != NULL) {
				LogoModel->Remove ();
			}
		}
	}

	MenuDialogClass::On_Menu_Activate (onoff);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Init_Dialog
//
////////////////////////////////////////////////////////////////
void
MainMenuDialogClass::On_Init_Dialog (void)
{
	Update_Version_Number ();

#if defined(BETACLIENT) || defined(FREEDEDICATEDSERVER) || defined(MULTIPLAYERDEMO)
	Get_Dlg_Item(IDC_MENU_START_SP_GAME_BUTTON)->Enable(false);
	Get_Dlg_Item(IDC_MENU_START_PRACTICE_GAME_BUTTON)->Enable(false);
#endif

#ifndef BETACLIENT
	if (Get_Dlg_Item (IDC_BETA_TEST_TEXT) != NULL) {
		Get_Dlg_Item (IDC_BETA_TEST_TEXT)->Show (false);
	}
#endif

	ImageCtrlClass *image_ctrl = (ImageCtrlClass *)Get_Dlg_Item (IDC_IMAGE);
	if (image_ctrl != NULL) {
		image_ctrl->Set_Texture ("ESRB_RATING.TGA");
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Transition_In
//
////////////////////////////////////////////////////////////////
DialogTransitionClass *
MainMenuDialogClass::Get_Transition_In (DialogBaseClass *prev_dlg)
{
	MainMenuTransitionClass *transition = NULL;

	//
	//	Add the transition model to the scene
	//
	if (TitleTransModel != NULL && TitleTransModel->Peek_Scene () == NULL) {
		if (MenuBackDropClass *backdrop = Get_BackDrop ()) {
			backdrop->Peek_Scene ()->Add_Render_Object (TitleTransModel);
		}
	}

	//
	//	Add the logo to the screen
	//
	if (LogoModel != NULL && LogoModel->Peek_Scene () == NULL) {
		if (MenuBackDropClass *backdrop = Get_BackDrop ()) {
			backdrop->Peek_Scene ()->Add_Render_Object (LogoModel);
		}
	}

	//
	//	We only want to transition between menu dialogs
	//
	if (prev_dlg == NULL ||
			(prev_dlg != QuitVerificationDialogClass::Get_Instance () &&
			 prev_dlg != DlgWOLWait::Get_Instance ()))
	{
		transition = new MainMenuTransitionClass;
		transition->Set_Model (TitleTransModel);
		if (MenuBackDropClass *backdrop = Get_BackDrop ()) {
			transition->Set_Camera (backdrop->Peek_Camera ());
		}
		transition->Set_Type (DialogTransitionClass::SCREEN_IN);
		transition->Set_Dialogs (this, prev_dlg);

		//
		//	Don't do the transition if something is missing
		//
		if (transition->Is_Valid () == false) {
			REF_PTR_RELEASE (transition);
		}
	}

	return transition;
}


////////////////////////////////////////////////////////////////
//
//	Get_Transition_Out
//
////////////////////////////////////////////////////////////////
DialogTransitionClass *
MainMenuDialogClass::Get_Transition_Out (DialogBaseClass *next_dlg)
{
	MainMenuTransitionClass *transition = NULL;

	//
	//	We only want to transition between menu dialogs
	//
	if (	IsStartingPractice == false &&
			(next_dlg == NULL || next_dlg->As_MenuDialogClass () != NULL))
	{
		transition = new MainMenuTransitionClass;
		transition->Set_Model (TitleTransModel);
		if (MenuBackDropClass *backdrop = Get_BackDrop ()) {
			transition->Set_Camera (backdrop->Peek_Camera ());
		}
		transition->Set_Type (DialogTransitionClass::SCREEN_OUT);
		transition->Set_Dialogs (this, next_dlg);

		//
		//	Don't do the transition if something is missing
		//
		if (transition->Is_Valid () == false) {
			REF_PTR_RELEASE (transition);
		}
	}

	return transition;
}


////////////////////////////////////////////////////////////////
//
//	Choose_Skirmish_Map
//
////////////////////////////////////////////////////////////////
StringClass
MainMenuDialogClass::Choose_Skirmish_Map (void)
{
	DynamicVectorClass<StringClass>	map_list;
	WIN32_FIND_DATAA find_info	= { 0 };
	BOOL keep_going				= true;
	HANDLE file_find				= NULL;
	StringClass file_filter;

	//
	// Look for any skirmish maps.
	//
	file_filter.Format("data\\skirmish*.mix");
	keep_going = true;
	for (file_find = ::FindFirstFileA (file_filter, &find_info);
		 (file_find != INVALID_HANDLE_VALUE) && keep_going;
		  keep_going = ::FindNextFileA (file_find, &find_info))
	{
		map_list.Add (find_info.cFileName);
	}

	if (file_find != INVALID_HANDLE_VALUE) {
		::FindClose (file_find);
	}

	if (map_list.Count() == 0) {
		//
		// No skirmish maps found. Look for a C&C map.
		//
		file_filter.Format("data\\c&c_*.mix");
		keep_going = true;
		for (file_find = ::FindFirstFileA (file_filter, &find_info);
			 (file_find != INVALID_HANDLE_VALUE) && keep_going;
			  keep_going = ::FindNextFileA (file_find, &find_info))
		{
			map_list.Add (find_info.cFileName);
		}

		if (file_find != INVALID_HANDLE_VALUE) {
			::FindClose (file_find);
		}
	}

	StringClass mapname;
	if (map_list.Count() > 0) {
		int choice = rand() % map_list.Count();
		mapname = map_list[choice];
	}

	return mapname;
}


////////////////////////////////////////////////////////////////
//
//	On_Command
//
////////////////////////////////////////////////////////////////
void
MainMenuDialogClass::On_Command (int ctrl_id, int message_id, unsigned int param)
{
	bool allow_default = true;

	switch (ctrl_id)
	{
		case IDCANCEL:
			ctrl_id = IDC_MENU_QUIT_BUTTON;
			break;

		case IDC_MENU_START_PRACTICE_GAME_BUTTON:
		{
			StringClass mapname = Choose_Skirmish_Map();

			if (!mapname.Is_Empty()) {
				IsStartingPractice = true;
				const int SKIRMISH_LOAD_MENU_NUMBER	= 96;
				CampaignManager::Select_Backdrop_Number(SKIRMISH_LOAD_MENU_NUMBER);
				GameInitMgrClass::Initialize_Skirmish();

				//
				// We will cycle on the same map until they get tired of practicing.
				//
				WWASSERT(The_Game() != NULL);
				The_Game()->Set_Map_Cycle(0, mapname);

				GameInitMgrClass::Start_Game(mapname, -1, 0);
				IsStartingPractice = false;
			}
			break;
		}

		case IDC_MENU_MP_LAN_GAME_BUTTON:
			
			//
			// Clear any gamespyadmin flags
			//
			cGameSpyAdmin::Reset();

			if (cNicEnum::Get_Num_Nics() > 0) {
				GameInitMgrClass::Initialize_LAN ();
			} else {
				DlgMsgBox::DoDialog(
					TRANSLATE(IDS_MP_UNABLE_INITIALIZE_LAN), 
					TRANSLATE(IDS_MP_NO_LAN_IP_ADDRESSES_FOUND));
				allow_default = false;
			}
			break;

		case IDC_MENU_MP_INTERNET_GAME_BUTTON:
			START_DIALOG (InternetMainDialogClass);
			allow_default = false;
			break;

		default:
			break;
	}

	if (allow_default) {
		MenuDialogClass::On_Command (ctrl_id, message_id, param);
	}
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Display
//
////////////////////////////////////////////////////////////////
void
MainMenuDialogClass::Display (void)
{
	//
	//	Create the dialog if necessary, otherwise simply bring it to the front
	//
	if (_TheInstance == NULL) {

		//
		//	Create the dialog
		//
		MainMenuDialogClass *dialog = new MainMenuDialogClass;

		//
		//	Create the backdrop if necessary
		//
		if (Animated) {

			if (MenuBackDropClass *backdrop = dialog->Get_BackDrop ()) {
				if (backdrop->Peek_Model () == NULL) {
					backdrop->Set_Model ("IF_BACK01");
					backdrop->Set_Animation ("IF_BACK01.IF_BACK01");

					/*RenderObjClass *model = WW3DAssetManager::Get_Instance ()->Create_Render_Obj ("IF_RENLOGO");
					if (model != NULL) {
						backdrop->Peek_Scene ()->Add_Render_Object(model);
					}*/
				}
			}
		}

		//
		//	Start the dialog
		//
		dialog->Start_Dialog ();
		REF_PTR_RELEASE (dialog);

	} else {
		if (_TheInstance->Is_Active_Menu () == false) {
			DialogMgrClass::Rollback (_TheInstance);
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Update_Version_Number
//
////////////////////////////////////////////////////////////////
void
MainMenuDialogClass::Update_Version_Number (void)
{
	//
	// Version 1.0 by default
	//
	unsigned int version_major = 1;
	unsigned int version_minor = 0;
	Get_Version_Number(&version_major, &version_minor);

	//
	// Put the version string into the dialog
	//
	WideStringClass version_string;
	// Add build number temporarily. Will probably be removed for shipping.
	WideStringClass build_number(BuildInfoClass::Get_Build_Number_String(), true);
	WideStringClass build_initials(BuildInfoClass::Get_Builder_Initials(), true);
	WideStringClass build_date(BuildInfoClass::Get_Build_Date_String(), true);
	version_string.Format (L"v%d.%.3d %s-%s %s", (version_major >> 16), (version_major & 0xFFFF), build_initials, build_number, build_date);
	Set_Dlg_Item_Text (IDC_VERSION_STATIC, version_string);
}
























				//TRANSLATE_ME
				//const wchar_t * title	= L"Unable to initialize LAN";
				//IDS_MP_UNABLE_INITIALIZE_LAN
				//const wchar_t * text	= L"No LAN IP addresses found.";
				//IDS_MP_NO_LAN_IP_ADDRESSES_FOUND

				//DlgMsgBox::DoDialog(title, text);

/*
#ifdef MULTIPLAYERDEMO
			START_DIALOG (GameSpyMainDialogClass);
#else
			START_DIALOG (InternetMainDialogClass);
#endif
*/
