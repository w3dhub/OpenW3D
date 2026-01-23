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
 *                     $Archive:: /Commando/Code/commando/dlgevaobjectivestab.cpp         $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 11/07/01 12:28p                                             $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "renegadedialog.h"
#include "dlgevaobjectivestab.h"
#include "listctrl.h"
#include "string_ids.h"
#include "translatedb.h"
#include "objectives.h"
#include "imagectrl.h"


////////////////////////////////////////////////////////////////
//	Local constants
////////////////////////////////////////////////////////////////
enum
{
	COL_PRIORITY	= 0,
	COL_TEXT,
	COL_STATUS
};


//
//	This is extremely poor, but I need to get the index
// of the currently displayed POG in the HUD.
//
extern int	CurrentObjectiveIndex;


////////////////////////////////////////////////////////////////
//
//	EvaObjectivesTabClass
//
////////////////////////////////////////////////////////////////
EvaObjectivesTabClass::EvaObjectivesTabClass (void)	:
	ChildDialogClass (GetRenegadeDialog(RenegadeDialogID::IDD_ENCYCLOPEDIA_OBJECTIVES_TAB))
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Init_Dialog
//
////////////////////////////////////////////////////////////////
void
EvaObjectivesTabClass::On_Init_Dialog (void)
{
	//
	//	Get a pointer to the list control
	//
	ListCtrlClass *list_ctrl = (ListCtrlClass *)Get_Dlg_Item (IDC_OBJECTIVES_LIST_CTRL);
	if (list_ctrl != NULL) {

		//
		//	Configure the list control
		//
		list_ctrl->Add_Column (TRANSLATE (IDS_OBJHDR_PRIORITY), 0.25F, Vector3 (1, 1, 1));
		list_ctrl->Add_Column (TRANSLATE (IDS_OBJHDR_OBJECTIVE), 0.5F, Vector3 (1, 1, 1));
		list_ctrl->Add_Column (TRANSLATE (IDS_OBJHDR_STATUS), 0.25F, Vector3 (1, 1, 1));

		Fill_Objectives_List ();
	}

	ChildDialogClass::On_Init_Dialog ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Command
//
////////////////////////////////////////////////////////////////
void
EvaObjectivesTabClass::On_Command (int ctrl_id, int message_id, unsigned int param)
{
	ChildDialogClass::On_Command (ctrl_id, message_id, param);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_ListCtrl_Sel_Change
//
////////////////////////////////////////////////////////////////
void
EvaObjectivesTabClass::On_ListCtrl_Sel_Change (ListCtrlClass * /* list_ctrl */, int ctrl_id, int /* old_index */, int /* new_index */)
{
	if (ctrl_id == IDC_OBJECTIVES_LIST_CTRL) {
		Update_Curr_Objective_Controls ();
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Update_Curr_Objective_Controls
//
////////////////////////////////////////////////////////////////
void
EvaObjectivesTabClass::Update_Curr_Objective_Controls (void)
{
	//
	//	Get a pointer to the list controls
	//
	ListCtrlClass *list_ctrl = (ListCtrlClass *)Get_Dlg_Item (IDC_OBJECTIVES_LIST_CTRL);
	ImageCtrlClass *image_ctrl = (ImageCtrlClass *)Get_Dlg_Item (IDC_IMAGE);
	if (list_ctrl == NULL || image_ctrl == NULL) {
		return ;
	}

	//
	//	Get the current selection from the list control
	//
	int curr_sel = list_ctrl->Get_Curr_Sel ();
	if (curr_sel >= 0) {
		Objective *objective = ObjectiveManager::Get_Objective (list_ctrl->Get_Entry_Data (curr_sel, 0));
		if (objective != NULL) {
			
			//
			//	Put the long description into the edit control
			//
			Set_Dlg_Item_Text (IDC_DESCRIPTION_EDIT, TRANSLATE (objective->LongDescriptionID));

			//
			//	Configure the image ctrl
			//
			image_ctrl->Set_Texture (objective->HUDPogTextureName);
		}		
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Fill_Objectives_List
//
////////////////////////////////////////////////////////////////
void
EvaObjectivesTabClass::Fill_Objectives_List (void)
{
	//
	//	Get a pointer to the list control
	//
	ListCtrlClass *list_ctrl = (ListCtrlClass *)Get_Dlg_Item (IDC_OBJECTIVES_LIST_CTRL);
	if (list_ctrl == NULL) {
		return ;
	}

	//
	//	Loop over all the objectives
	//
	int count = ObjectiveManager::Get_Objective_Count ();
	for (int index = 0; index < count; index ++) {		
		Objective *objective = ObjectiveManager::Get_Objective (index);
		if (objective != NULL) {
			
			//
			//	Don't display hidden objectives (unless you really want to)
			//
			if (	objective->Status != ObjectiveManager::STATUS_HIDDEN ||
					ObjectiveManager::Is_Objective_Debug_Mode_Enabled ())
			{
				//
				//	Add this entry to the list
				//
				int item_index = list_ctrl->Insert_Entry (index, objective->Type_To_Name ());
				if (item_index != -1) {
					WideStringClass text = TRANSLATE (objective->ShortDescriptionID);
					
					//
					//	Strip off the line delimiter (if necessary)
					//
					if (text.Get_Length () > 0 && text[text.Get_Length () - 1] == U_CHAR('\n')) {
						text.Erase (text.Get_Length () - 1, 1);
					}

					list_ctrl->Set_Entry_Text (item_index, COL_TEXT, text);
					list_ctrl->Set_Entry_Text (item_index, COL_STATUS, objective->Status_To_Name ());
					list_ctrl->Set_Entry_Data (item_index, 0, index);

					//
					//	Colorize this entry
					//
					Vector3 color = objective->Type_To_Color ();
					list_ctrl->Set_Entry_Color (item_index, COL_PRIORITY, color);
					list_ctrl->Set_Entry_Color (item_index, COL_TEXT, color);
					list_ctrl->Set_Entry_Color (item_index, COL_STATUS, color);					
				}
			}
		}
	}

	//
	//	Sort the objectives...
	//
	list_ctrl->Sort (ListSortCallback, 0);

	//
	//	Select the first entry by default
	//
	if (count > 0) {
		list_ctrl->Set_Curr_Sel (0);
		Update_Curr_Objective_Controls ();
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	ListSortCallback
//
////////////////////////////////////////////////////////////////
int
EvaObjectivesTabClass::ListSortCallback
(
	ListCtrlClass *	list_ctrl,
	int					item_index1,
	int					item_index2,
	uint32				/* user_param */
)
{
	int count = list_ctrl->Get_Entry_Count ();

	Objective *objective1 = ObjectiveManager::Get_Objective (list_ctrl->Get_Entry_Data (item_index1, 0));
	Objective *objective2 = ObjectiveManager::Get_Objective (list_ctrl->Get_Entry_Data (item_index2, 0));

	int result = 0;

	//
	//	Sort the objective based on status
	//
	if (	objective1->Status != ObjectiveManager::STATUS_ACCOMPLISHED &&
			objective2->Status == ObjectiveManager::STATUS_ACCOMPLISHED)
	{
		result = -1;
	} else if (	objective1->Status == ObjectiveManager::STATUS_ACCOMPLISHED &&
					objective2->Status != ObjectiveManager::STATUS_ACCOMPLISHED)
	{
		result = 1;
	} else {

		if (item_index1 < CurrentObjectiveIndex) {
			item_index1 += count;
		}

		if (item_index2 < CurrentObjectiveIndex) {
			item_index2 += count;
		}

		//
		//	Sort based on relative position to the current objective
		//
		result = (item_index1 - item_index2);
	}
			
	return result;
}
