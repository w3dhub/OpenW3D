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
 *                     $Archive:: /Commando/Code/commando/dlgcharacteroptions.cpp         $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/07/02 10:56a                                              $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "renegadedialog.h"
#include "dlgcharacteroptions.h"
#include "listctrl.h"
#include "translatedb.h"
#include "string_ids.h"
#include "skinpackagemgr.h"
#include "skinpackage.h"
#include "comboboxctrl.h"
#include "purchasesettings.h"
#include "viewerctrl.h"
#include "physicalgameobj.h"
#include "assets.h"
#include "savegame.h"


////////////////////////////////////////////////////////////////
//
//	CharacterOptionsMenuClass
//
////////////////////////////////////////////////////////////////
CharacterOptionsMenuClass::CharacterOptionsMenuClass (void)	:
	UnloadDefs (false),
	MenuDialogClass (GetRenegadeDialog(RenegadeDialogID::IDD_OPTIONS_CHARACTER))
{
	SkinPackageMgrClass::Reset_List ();
	SkinPackageMgrClass::Build_List ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	~CharacterOptionsMenuClass
//
////////////////////////////////////////////////////////////////
CharacterOptionsMenuClass::~CharacterOptionsMenuClass (void)
{
	SkinPackageMgrClass::Reset_List ();	

	//
	//	Unload the definitions as necessary
	//
	if (UnloadDefs) {
		DefinitionMgrClass::Free_Definitions ();
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Init_Dialog
//
////////////////////////////////////////////////////////////////
void
CharacterOptionsMenuClass::On_Init_Dialog (void)
{	
	Ensure_Definitions_Are_Loaded ();

	ComboBoxCtrlClass *combo_box = (ComboBoxCtrlClass *)Get_Dlg_Item (IDC_PACKAGE_COMBO);
	if (combo_box != NULL) {		

		const StringClass &curr_package_name = SkinPackageMgrClass::Get_Current_Package ().Get_Name ();

		//
		//	Add an entry for each package to the combobox
		//
		int count = SkinPackageMgrClass::Get_Package_Count ();
		for (int index = 0; index < count; index ++) {
			
			//
			//	Add this entry to the combobox
			//
			const StringClass &name = SkinPackageMgrClass::Get_Package (index)->Get_Name ();
			WideStringClass wide_name;
			wide_name.Convert_From (name);
			int item_index = combo_box->Add_String (wide_name);
			
			//
			//	Select this entry if its the active package
			//
			if (name.Compare_No_Case (curr_package_name) == 0) {
				combo_box->Set_Curr_Sel (item_index);
			}
		}

		//
		//	Ensure that at least the first entry is selected
		//
		int curr_sel = combo_box->Get_Curr_Sel ();
		if (curr_sel == -1) {
			curr_sel = 0;
			combo_box->Set_Curr_Sel (curr_sel);
		}
	}

	//
	//	Fill in the list of characters that can be overridden
	//
	Fill_Character_Combobox ();

	MenuDialogClass::On_Init_Dialog ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Command
//
////////////////////////////////////////////////////////////////
void
CharacterOptionsMenuClass::On_Command (int ctrl_id, int message_id, unsigned int param)
{
	MenuDialogClass::On_Command (ctrl_id, message_id, param);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Fill_Character_Combobox
//
////////////////////////////////////////////////////////////////
void
CharacterOptionsMenuClass::Fill_Character_Combobox (void)
{
	PurchaseSettingsDefClass *settings1 = PurchaseSettingsDefClass::Find_Definition (PurchaseSettingsDefClass::TYPE_CLASSES, PurchaseSettingsDefClass::TEAM_GDI);
	PurchaseSettingsDefClass *settings2 = PurchaseSettingsDefClass::Find_Definition (PurchaseSettingsDefClass::TYPE_CLASSES, PurchaseSettingsDefClass::TEAM_NOD);
	if (settings1 == NULL || settings2 == NULL) {
		return ;
	}

	ComboBoxCtrlClass *combo_box = (ComboBoxCtrlClass *)Get_Dlg_Item (IDC_CHARACTER_COMBO);
	if (combo_box == NULL) {
		return ;
	}

	//
	//	Fill in the GDI characters
	//
	const int MAX_ENTRIES = 10;
	int index;
	for (index = 0; index < MAX_ENTRIES; index ++) {
		
		int item_index = combo_box->Add_String (settings1->Get_Name (index));
		if (item_index != -1) {
			combo_box->Set_Item_Data (item_index, settings1->Get_Definition (index));
		}
	}

	//
	//	Fill in the NOD characters
	//
	for (index = 0; index < MAX_ENTRIES; index ++) {
		
		int item_index = combo_box->Add_String (settings2->Get_Name (index));
		if (item_index != -1) {
			combo_box->Set_Item_Data (item_index, settings2->Get_Definition (index));
		}
	}

	//
	//	Select the first entry by default
	//
	combo_box->Set_Curr_Sel (0);
	On_Character_Selected (0);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_ComboBoxCtrl_Sel_Change
//
////////////////////////////////////////////////////////////////
void
CharacterOptionsMenuClass::On_ComboBoxCtrl_Sel_Change
(
	ComboBoxCtrlClass *	combo_ctrl,
	int						ctrl_id,
	int						old_sel,
	int						new_sel
)
{
	if (ctrl_id == IDC_PACKAGE_COMBO) {
		
		//
		//	Select the current package and populate the skin list control
		//
		SkinPackageMgrClass::Set_Current_Package (new_sel);
		
	} else if (ctrl_id == IDC_CHARACTER_COMBO) {
		On_Character_Selected (new_sel);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Character_Selected
//
////////////////////////////////////////////////////////////////
void
CharacterOptionsMenuClass::On_Character_Selected (int index)
{
	ComboBoxCtrlClass *combo_box = (ComboBoxCtrlClass *)Get_Dlg_Item (IDC_CHARACTER_COMBO);
	if (combo_box == NULL) {
		return ;
	}
	
	//
	//	Display the model for this purchase
	//
	uint32 definition_id	= combo_box->Get_Item_Data (index);	
	Display_Default_Model (definition_id);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Display_Default_Model
//
////////////////////////////////////////////////////////////////
void
CharacterOptionsMenuClass::Display_Default_Model (int definition_id)
{
	//
	//	Lookup the definition that goes with this character
	//	
	DefinitionClass *definition = DefinitionMgrClass::Find_Definition (definition_id);
	if (definition != NULL) {
		PhysicalGameObjDef *phys_game_obj_def = static_cast<PhysicalGameObjDef *> (definition);
		
		//
		//	Now, lookup the physics definition that this game object uses
		//
		DefinitionClass *phys_def = DefinitionMgrClass::Find_Definition (phys_game_obj_def->Get_Phys_Def_ID ());
		if (phys_def != NULL) {
			
			//
			//	Display the model that this physics definition requires
			//
			const char *model_name = ((PhysDefClass *)phys_def)->Get_Model_Name ();
			Display_Model (model_name);
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Display_Model
//
////////////////////////////////////////////////////////////////
void
CharacterOptionsMenuClass::Display_Model (const char *model_name)
{
	ViewerCtrlClass *viewer_ctrl = (ViewerCtrlClass *)Get_Dlg_Item (IDC_MODEL_VIEWER);
	if (viewer_ctrl == NULL) {
		return ;
	}

	//
	//	Create and display model
	//
	RenderObjClass *model = ::Create_Render_Obj_From_Filename (model_name);
	viewer_ctrl->Set_Model (model);
	viewer_ctrl->Set_Animation ("S_A_HUMAN.H_A_A0A0");
	viewer_ctrl->Set_Interface_Mode (ViewerCtrlClass::Z_ROTATION, 45.0F);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Ensure_Definitions_Are_Loaded
//
////////////////////////////////////////////////////////////////
void
CharacterOptionsMenuClass::Ensure_Definitions_Are_Loaded (void)
{
	if (DefinitionMgrClass::Get_First () != NULL) {
		return ;
	}

	//
	//	Force load the definitions as necessary
	//
	SaveGameManager::Load_Definitions ();
	UnloadDefs = true;
	return ;
}

