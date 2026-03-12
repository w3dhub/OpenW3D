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
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Installer                                                    *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Installer/InstallMenuDialog.c $*
 *                                                                                             *
 *                      $Author:: Ian_l                   $*
 *                                                                                             *
 *                     $Modtime:: 12/14/01 11:05a               $*
 *                                                                                             *
 *                    $Revision:: 5                     $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

// Includes.
#include "InstallMenuDialog.h"
#include "AssetMgr.h"
#include "HAnim.h"
#include "MenuBackDrop.h"
#include "Scene.h"
#include "Translator.h"


/***********************************************************************************************
 * InstallMenuDialogClass::InstallMenuDialogClass -- Constructor										  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                *
 *=============================================================================================*/
InstallMenuDialogClass::InstallMenuDialogClass (int resourceid)
	: 	CallbackMenuDialogClass (resourceid)
{
	// If a model has not yet been specified...
	if (Get_BackDrop()->Peek_Model() == NULL) {

		const char		*basemodelname												= "IF_BACK01";
		const char		*baseanimname												= "IF_BACK01.IF_BACK01";
		const unsigned  supplementalmodelcount									= 2;
		const char		*supplementalmodelname [supplementalmodelcount]	= {"IN_EVA", "IN_RENLOGO"};
		const char		*supplementalanimname  [supplementalmodelcount] = {"IN_EVA.IN_EVA", "IN_RENLOGO.IN_RENLOGO"};

		Get_BackDrop()->Set_Model (basemodelname);
		Get_BackDrop()->Set_Animation (baseanimname);

		for (unsigned m = 0; m < supplementalmodelcount; m++) {

			RenderObjClass *supplementalmodel;
			HAnimClass		*supplementalanim;

			supplementalmodel = WW3DAssetManager::Get_Instance()->Create_Render_Obj (supplementalmodelname [m]);
			if (supplementalmodel != NULL) {
				Get_BackDrop()->Peek_Scene()->Add_Render_Object (supplementalmodel);
			}

			supplementalanim = WW3DAssetManager::Get_Instance()->Get_HAnim (supplementalanimname [m]);
			if (supplementalanim != NULL) {
				supplementalmodel->Set_Animation (supplementalanim, 0, RenderObjClass::ANIM_MODE_LOOP);
				REF_PTR_RELEASE (supplementalanim);
			}
			REF_PTR_RELEASE (supplementalmodel);
		}
	}
}


/***********************************************************************************************
 * InstallMenuDialogClass::~InstallMenuDialogClass -- Destructor										  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                *
 *=============================================================================================*/
InstallMenuDialogClass::~InstallMenuDialogClass()
{
}


/***********************************************************************************************
 * InstallMenuDialogClass::On_Command --																		  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/22/01    IML : Created.                                                                *
 *=============================================================================================*/
void InstallMenuDialogClass::On_Command (int ctrl_id, int message_id, DWORD param)
{
	switch (ctrl_id)
	{
		case IDCANCEL:
	  	{
			MessageBoxClass::Do_Dialog (TxWideStringClass (IDS_EXIT_SETUP), TxWideStringClass (IDS_PROMPT_EXIT_SETUP), MessageBoxClass::MESSAGE_BOX_TYPE_YES_NO, this);
			return;
		}

		case IDOK:
		case IDC_BUTTON_BACK:
			break;

		default:
			break;
	}

	MenuDialogClass::On_Command (ctrl_id, message_id, param);
}
