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
 *                     $Archive:: /Commando/Code/Installer/CopyDialog.cpp $* 
 *                                                                                             * 
 *                      $Author:: Ian_l                   $* 
 *                                                                                             * 
 *                     $Modtime:: 1/12/02 7:32p                 $* 
 *                                                                                             * 
 *                    $Revision:: 15                    $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

// Includes.
#include "CopyDialog.h"
#include "AssetMgr.h"
#include "Camera.h"
#include "DialogControl.h"
#include "DialogMgr.h"
#include "DialogText.h"
#include "ErrorHandler.h"
#include "Hanim.h"
#include "Installer.h"
#include "Light.h"
#include "MenuBackdrop.h"
#include "Resource.h"
#include "Scene.h"
#include "Translator.h"
#include "ViewerCtrl.h"
#include "Ww3D.h"
#include "WWAudio.h"


// Defines.
#define MODEL_COUNT			13 
#define MODEL_DISPLAY_TIME	22000		// Display time for each model (in milliseconds).


// Static data.
static const char	*_ModelNames [MODEL_COUNT] = {"C_AG_GDI_MG",
																"C_AG_GDI_MGO",
																"C_AG_NOD_MG",
																"C_AG_GDI_LOCKE",
																"C_AG_NOD_SAKU",
																"IGM_PIST",
																"IGM_SNIP",
																"IGM_C4-R",
																"VDC_GDI_HUMVEE",
																"V_GDI_MEDTNK",
																"V_NOD_TURRET",
																"V_GDI_ORCA",
																"ENC_GPWR"
															  };

	
static const char	*_AnimNames [MODEL_COUNT]  = {"S_A_HUMAN.H_A_A0A1",
																"S_A_HUMAN.H_A_A0B1",
																"S_A_HUMAN.H_A_A0A1",
																"S_A_HUMAN.H_A_A0B1",
																"S_B_HUMAN.H_B_A0A0_13",
																"",
																"",
																"",
																"",
																"",
																"",
																"V_GDI_ORCA.V_GDI_ORCA",
																""
															  };

static const int _ModelHeadings [MODEL_COUNT] =	{IDS_GDI_SOLDIER_HEADING,
																 IDS_GDI_OFFICER_HEADING,
																 IDS_NOD_SOLDIER_HEADING,
																 IDS_LOCKE_HEADING,
																 IDS_SAKURA_HEADING,
																 IDS_PISTOL_HEADING,
																 IDS_SNIPER_RIFLE_HEADING,
																 IDS_REMOTE_C4_HEADING,
																 IDS_GDI_HUMM_VEE_HEADING,
																 IDS_GDI_MEDIUM_TANK_HEADING,
																 IDS_NOD_TURRET_HEADING,
																 IDS_GDI_ORCA_HEADING,
																 IDS_GDI_POWER_PLANT_HEADING
																};

static const int _ModelDescriptions [MODEL_COUNT] = {IDS_GDI_SOLDIER_DESCRIPTION,
																	  IDS_GDI_OFFICER_DESCRIPTION,
																	  IDS_NOD_SOLDIER_DESCRIPTION,
																	  IDS_LOCKE_DESCRIPTION,
																	  IDS_SAKURA_DESCRIPTION,
																	  IDS_PISTOL_DESCRIPTION,
																	  IDS_SNIPER_RIFLE_DESCRIPTION,
																	  IDS_REMOTE_C4_DESCRIPTION,
																	  IDS_GDI_HUMM_VEE_DESCRIPTION,
																	  IDS_GDI_MEDIUM_TANK_DESCRIPTION,
																	  IDS_NOD_TURRET_DESCRIPTION,
																	  IDS_GDI_ORCA_DESCRIPTION,
																	  IDS_GDI_POWER_PLANT_DESCRIPTION
																	 };

																	  
/***********************************************************************************************
 * CopyDialogClass::CopyDialogClass --																			  *
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
CopyDialogClass::CopyDialogClass()
	: InstallMenuDialogClass (IDD_DIALOG_COPY),
	  SupplementalModel (NULL),
	  ProgressBarModel (NULL),
	  ProgressBarAnim (NULL),
	  CurrentModel (0),
	  CountdownTimer (NULL),
	  FlashTimer (NULL),
	  FileSystemErrorPopup (NULL),
	  AddedModels (false)
{
}


/***********************************************************************************************
 * CopyDialogClass::On_Init_Dialog --																			  *
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
void CopyDialogClass::On_Init_Dialog (void)
{
	WideStringClass sourcepath;

	ViewerCtrlClass *viewercontrol = Get_Dlg_Item (IDC_COPY_VIEWER)->As_ViewerCtrlClass();
	
	// Disable the OK button until copying is complete.
	Enable_Dlg_Item (IDOK, false);

	// Configure the W3D elements.
	viewercontrol->Set_Background_Visible (false);
	Add_Models();

	// Start the copy thread.
	CopyThread = new CopyThreadClass (_Installer.Get_Total_Size (false));
	CopyThread->Execute();

	CountdownTimer = new CDTimerClass <SafeTimerClass> (MODEL_DISPLAY_TIME);

	InstallMenuDialogClass::On_Init_Dialog();
}


/***********************************************************************************************
 * CopyDialogClass::On_Activate --																				  *
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
void CopyDialogClass::On_Activate (bool onoff)
{
	if (onoff) {
		AIL_set_preference (DIG_DS_MIX_FRAGMENT_CNT, 64);
		AIL_serve();
	} else {
		AIL_set_preference (DIG_DS_MIX_FRAGMENT_CNT, 8);
	}

	InstallMenuDialogClass::On_Activate (onoff);
}


/***********************************************************************************************
 * CopyDialogClass::On_Frame_Update --																			  *
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
void CopyDialogClass::On_Frame_Update (void)
{
	WideStringClass  statusmessage;
	DialogTextClass *statustext1, *statustext2;

	// Set both status lines with the status message. By default, use the small text for display.
	CopyThread->Get_Status_Message (statusmessage);
	statustext1 = Get_Dlg_Item (IDC_COPY_STATUS1)->As_DialogTextClass();
	statustext2 = Get_Dlg_Item (IDC_COPY_STATUS2)->As_DialogTextClass();

	// Update the copy status.
	switch (CopyThread->Get_Status()) {

		case CopyThreadClass::STATUS_OK:
		{
			float	fraction, framecount;
			char  percentagestring [12];

			Add_Models();

 			// Advance to next model if timer has 'gone off'.
			if (CountdownTimer->Value() == 0) {
				CurrentModel = (CurrentModel + 1) % MODEL_COUNT;
				Set_Model (CurrentModel);	
		 		delete CountdownTimer;
				CountdownTimer = new CDTimerClass <SafeTimerClass> (MODEL_DISPLAY_TIME);
			}
			
			// Update the progress bar.
			statustext1->Set_Text (statusmessage);
			statustext2->Set_Text (L"");
			framecount = ProgressBarModel->Peek_Animation()->Get_Num_Frames();
			fraction = CopyThread->Get_Fraction_Complete();
			ProgressBarModel->Set_Animation (ProgressBarAnim, fraction * (framecount - 1), RenderObjClass::ANIM_MODE_MANUAL);
			percentagestring [sizeof (percentagestring) - 1] = '\0';
			_snprintf (percentagestring, sizeof (percentagestring) - 1, "%u%%", (unsigned) (fraction * 100.0f));
			Set_Dlg_Item_Text (IDC_COPY_PERCENTAGE, WideStringClass (percentagestring));
			break;
		}

		case CopyThreadClass::STATUS_ERROR:
			FileSystemErrorPopup = MessageBoxClass::Create_Dialog (TxWideStringClass (IDS_APPLICATION_ERROR), TxWideStringClass (IDS_FILE_SYSTEM_ERROR), MessageBoxClass::MESSAGE_BOX_TYPE_RETRY_QUIT, this);
			CountdownTimer->Stop();
			break;

		case CopyThreadClass::STATUS_SUCCESS:

			Remove_Models();
			
 			// Advance to next model if timer has 'gone off'.
			if (CountdownTimer->Value() == 0) {
				CurrentModel = (CurrentModel + 1) % MODEL_COUNT;
				Set_Model (CurrentModel);	
		 		delete CountdownTimer;
				CountdownTimer = new CDTimerClass <SafeTimerClass> (MODEL_DISPLAY_TIME);
			}

			if (FlashTimer == NULL) {
				
				FlashTimer = new TTimerClass <SafeTimerClass>;

				// Allow the user to continue by enabling the OK button.
				Enable_Dlg_Item (IDOK, true);
			}

			// Flash the status message in large text.
			statustext1->Set_Text (L"");
			Set_Dlg_Item_Text (IDC_COPY_PERCENTAGE, L"");
			if (((FlashTimer->Value() / 1000) % 3) == 0) {
				statustext2->Set_Text (statusmessage);
			} else {
				statustext2->Set_Text (L"");
			}
			break;

		case CopyThreadClass::STATUS_ABORTED:

			Remove_Models();

			// Use the large text to display the status message.
			statustext1->Set_Text (L"");
			statustext2->Set_Text (statusmessage);

			// Cancel this dialog if the copy thread has terminated.
			if (!CopyThread->Is_Running()) {
				
				MenuDialogClass::On_Command (IDCANCEL, 0, 0);
			
				// NOTE: Must return because this object may have already been deleted.
				return;
			}
			break;

		case CopyThreadClass::STATUS_FAILURE:
		{
			WideStringClass errormessage;

			// A fatal error has occurred.
			CopyThread->Get_Error_Message (errormessage);
			throw (errormessage);	
		}

		default:
			break;
	}

	InstallMenuDialogClass::On_Frame_Update();
}


/***********************************************************************************************
 * CopyDialogClass::Add_Models --																				  *
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
void CopyDialogClass::Add_Models()
{
	if (!AddedModels) {

		const char *supplementalmodelname = "IN_COPYGIZMO";
		const char *supplementalanimname  = "IN_COPYGIZMO.IN_COPYGIZMO";
		const char *progressbarmodelname  = "IN_PROGRESS";
		const char *progressbaranimname	 = "IN_PROGRESS.IN_PROGRESS";

		HAnimClass *supplementalanim;

		SupplementalModel = WW3DAssetManager::Get_Instance()->Create_Render_Obj (supplementalmodelname);
		if (SupplementalModel != NULL) {
			Get_BackDrop()->Peek_Scene()->Add_Render_Object (SupplementalModel);

			supplementalanim = WW3DAssetManager::Get_Instance()->Get_HAnim (supplementalanimname);
			if (supplementalanim != NULL) {
				SupplementalModel->Set_Animation (supplementalanim, 0, RenderObjClass::ANIM_MODE_LOOP);
				REF_PTR_RELEASE (supplementalanim);
			}
		}

		ProgressBarModel = WW3DAssetManager::Get_Instance()->Create_Render_Obj (progressbarmodelname);
		if (ProgressBarModel != NULL) {
			Get_BackDrop()->Peek_Scene()->Add_Render_Object (ProgressBarModel);
			ProgressBarAnim = WW3DAssetManager::Get_Instance()->Get_HAnim (progressbaranimname);
			if (ProgressBarAnim != NULL) {
				ProgressBarModel->Set_Animation (ProgressBarAnim, 0, RenderObjClass::ANIM_MODE_MANUAL);
			}
		}

		Set_Model (CurrentModel);
		
		AddedModels = true;	
	}
}


/***********************************************************************************************
 * CopyDialogClass::Set_Model --																					  *	
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
void CopyDialogClass::Set_Model (unsigned modelindex)
{
	const float	rotationrate = 45.0f;	

	// Set the appropriate model, animation and interface for the viewer.
	
	ViewerCtrlClass *viewercontrol = Get_Dlg_Item (IDC_COPY_VIEWER)->As_ViewerCtrlClass();

	Set_Dlg_Item_Text (IDC_COPY_HEADER, TxWideStringClass (_ModelHeadings [modelindex]));
	Set_Dlg_Item_Text (IDC_COPY_DESCRIPTION, TxWideStringClass (_ModelDescriptions [modelindex]));

	viewercontrol->Set_Model (_ModelNames [modelindex]);
	viewercontrol->Set_Animation (_AnimNames [modelindex]);
	viewercontrol->Set_Interface_Mode (ViewerCtrlClass::Z_ROTATION, rotationrate);
		
	//	Traverse the model...
	for (int index = 0; index < viewercontrol->Peek_Model()->Get_Num_Sub_Objects(); index++) {

		RenderObjClass *sub_obj = viewercontrol->Peek_Model()->Get_Sub_Object (index);

		// Hide muzzle flashes - they are not appropriate in this context.
		if (::strstr (sub_obj->Get_Name(), "MUZZLEFLASH") != NULL)  {
			sub_obj->Set_Hidden (true);
		}

		// Hide flares - the rendering system has not been initialized.
		if (sub_obj->Class_ID() == RenderObjClass::CLASSID_DAZZLE) {
			sub_obj->Set_Hidden (true);
		}

		REF_PTR_RELEASE (sub_obj);
	}
}


/***********************************************************************************************
 * CopyDialogClass::Remove_Models --																			  *	
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
void CopyDialogClass::Remove_Models()
{
	if (AddedModels) {
		REF_PTR_RELEASE (ProgressBarAnim);
		ProgressBarModel->Remove();
		REF_PTR_RELEASE (ProgressBarModel);
		SupplementalModel->Remove();
		REF_PTR_RELEASE (SupplementalModel);

		AddedModels = false;
	}
}


/***********************************************************************************************
 * CopyDialogClass::End_Dialog --																				  *
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
void CopyDialogClass::End_Dialog (void)
{
	Remove_Models();

	// Wait for copy thread to terminate (in case it hasn't finished yet).
	while (CopyThread->Is_Running()) {
		Sleep (50);
	}
	delete CopyThread;

	AIL_set_preference (DIG_DS_MIX_FRAGMENT_CNT, 8);

	if (CountdownTimer != NULL) delete CountdownTimer;
	if (FlashTimer != NULL) delete FlashTimer;

	InstallMenuDialogClass::End_Dialog();
}


/***********************************************************************************************
 * CopyDialogClass::Callback --																					  *
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
void CopyDialogClass::Callback (int id, PopupDialogClass *popup)
{
 	if (popup == FileSystemErrorPopup) {

 		switch (id) {

 			case IDC_BUTTON_RETRY:
 				CopyThread->Set_Status (CopyThreadClass::STATUS_RETRY);
 				REF_PTR_RELEASE (popup);
				CountdownTimer->Start();
				break;

 			case IDC_BUTTON_QUIT:
 				CopyThread->Set_Status (CopyThreadClass::STATUS_QUIT);
 				REF_PTR_RELEASE (popup);
				CountdownTimer->Start();
				break;
				
 			default:
 				break;
		}

	} else {
			
		switch (id) {
		
			case IDC_BUTTON_YES:
				CopyThread->Set_Abort (true);
				CountdownTimer->Start();
				break;

			case IDC_BUTTON_NO:
			case IDCANCEL:
			case IDOK:
				CopyThread->Set_Abort (false);
				CountdownTimer->Start();
				break;

			default:
				break;
		}
	}

	// NOTE: Do not call base callback - this function replaces base callback functionality.
}


/***********************************************************************************************
 * CopyDialogClass::On_Command --																				  *
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
void CopyDialogClass::On_Command (int ctrl_id, int message_id, DWORD param)
{
	switch (ctrl_id) {

		case IDOK:

			// If the OK button has been disabled throw away this message.
			if (!Is_Dlg_Item_Enabled (IDOK)) return;
			break;

		case IDCANCEL:

			// If the copy process has already finished then cancel immediately.
			if (!CopyThread->Is_Running()) {
				MenuDialogClass::On_Command (IDCANCEL, 0, 0);
				return;
			} else {
			
				// Can the user cancel?
				// NOTE: User will not be able to cancel if the copying process is uninterruptable because,
				//			for example, it is updating the registry.
				if (!CopyThread->Can_Abort (true)) {
					CopyThread->Set_Abort (false);
					return;
				}
				CountdownTimer->Stop();
			}

 		default:
			break;
	}
	
	InstallMenuDialogClass::On_Command (ctrl_id, message_id, param);
}
