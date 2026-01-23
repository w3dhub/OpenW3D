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
 *                     $Archive:: /Commando/Code/commando/dlgevaviewertab.cpp       $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/07/02 6:12p                                               $*
 *                                                                                             *
 *                    $Revision:: 17                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "dlgevaviewertab.h"

#include "mapctrl.h"
#include "combat.h"
#include "soldier.h"
#include "objectives.h"
#include "translatedb.h"
#include "translateobj.h"
#include "listctrl.h"
#include "dialogtext.h"
#include "assets.h"
#include "ini.h"
#include "inisup.h"
#include "listnode.h"
#include "viewerctrl.h"
#include "rendobj.h"
#include "wheelvehicle.h"
#include "string_ids.h"


////////////////////////////////////////////////////////////////
//
//	EvaViewerTabClass
//
////////////////////////////////////////////////////////////////
EvaViewerTabClass::EvaViewerTabClass (const DialogResource *dialog_resource)	:
	ListCtrl (NULL),
	DescriptionCtrl (NULL),
	AffiliationCtrl (NULL),
	ViewerCtrl (NULL),
	EncyclopediaType (EncyclopediaMgrClass::TYPE_CHARACTER),
	ChildDialogClass (dialog_resource)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Init_Dialog
//
////////////////////////////////////////////////////////////////
void
EvaViewerTabClass::On_Init_Dialog (void)
{
	//
	//	Generate a set of data to display to the user (from an INI)
	//
	Build_Object_List ();

	//
	//	Configure the list control
	//
	ListCtrl->Add_Column (TRANSLATE (IDS_MENU_NAME), 1.0F, Vector3 (1, 1, 1));

	//
	//	Add this data to the list control
	//
	int total_obj_count = 0;
	for (int index = 0; index < ObjectList.Count (); index ++) {
		EvaViewerObjectClass &object = ObjectList[index];

		//
		//	Check to see if this entry should be displayed
		//
		if (EncyclopediaMgrClass::Is_Object_Revealed (EncyclopediaType, object.Get_ID ())) {

			//
			//	Add an entry to the list control for this object
			//
			int item_index = ListCtrl->Insert_Entry (index, object.Get_Name ());
			if (item_index >= 0) {
				ListCtrl->Set_Entry_Data (item_index, 0, (uintptr_t)&object);

				//
				//	Colorize the entry
				//
				Vector3 color (1.0F, 1.0F, 1.0F);
				if (object.Get_Player_Type () == PLAYERTYPE_GDI) {
					color.Set (1.0F, 0.832F, 0.156F);
				} else if (object.Get_Player_Type () == PLAYERTYPE_NOD) {
					color.Set (1.0F, 0.0F, 0.0F);
				}
				ListCtrl->Set_Entry_Color (item_index, 0, color);

				total_obj_count ++;
			}
		}
	}

	//
	//	Now, sort the players by rank
	//
	ListCtrl->Sort (ListSortCallback, 0);

	//
	//	Select and view the first entry by default
	//
	if (total_obj_count > 0) {
		ListCtrl->Set_Curr_Sel (0);
		View_Entry (0);
	} else {
		
		if (AffiliationCtrl != NULL) {
			AffiliationCtrl->Set_Text (U_CHAR(""));
		}		
	}

	ChildDialogClass::On_Init_Dialog ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Destroy_Dialog
//
////////////////////////////////////////////////////////////////
void
EvaViewerTabClass::On_Destroy_Dialog (void)
{
	if (ViewerCtrl != NULL) {
		ViewerCtrl->Set_Model ((RenderObjClass *)NULL);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Build_Object_List
//
////////////////////////////////////////////////////////////////
void
EvaViewerTabClass::Build_Object_List (void)
{
	ObjectList.Delete_All ();

	//
	//	Get the INI file which contains the data for this viewer
	//
	INIClass *ini_file = ::Get_INI (INIFilename);
	if (ini_file != NULL) {

		//
		//	Loop over all the sections in the INI
		//
		List<INISection *> &section_list = ini_file->Get_Section_List ();
		for (	INISection *section = section_list.First ();
				section != NULL;
				section = section->Next_Valid ())
		{
			//
			//	Read this object's data from the INI file
			//
			StringClass name_id (0, true);
			StringClass description_id (0, true);
			StringClass affiliation_id (0, true);
			StringClass model_name (0, true);
			StringClass anim_name (0, true);
			StringClass definition_name (0, true);
			StringClass player_type_str (0, true);
			ini_file->Get_String (name_id,			section->Section, "NameID");
			ini_file->Get_String (description_id,	section->Section, "DescriptionID");
			ini_file->Get_String (affiliation_id,	section->Section, "AffiliationID");
			ini_file->Get_String (model_name,		section->Section, "Model");
			ini_file->Get_String (anim_name,			section->Section, "Anim");			
			ini_file->Get_String (definition_name, section->Section, "DefinitionName");
			ini_file->Get_String (player_type_str,	section->Section, "PlayerType");
			int id			= ini_file->Get_Int (section->Section, "ID");
			float min_dist	= ini_file->Get_Float (section->Section, "MinDist");

			//
			//	Determine the player type
			//
			int player_type = PLAYERTYPE_NEUTRAL;
			if (player_type_str.Compare_No_Case ("GDI") == 0) {
				player_type = PLAYERTYPE_GDI;
			} else if (player_type_str.Compare_No_Case ("NOD") == 0) {
				player_type = PLAYERTYPE_NOD;
			}

			//
			//	Translate the data
			//
			const unichar_t *name				= TRANSLATE_BY_DESC(name_id);
			const unichar_t *description	= TRANSLATE_BY_DESC(description_id);
			const unichar_t *affiliation	= TRANSLATE_BY_DESC(affiliation_id);

			//
			//	Configure an object that will hold this data for us
			//
			EvaViewerObjectClass object;
			object.Set_ID (id);
			object.Set_Name (name);
			object.Set_Description (description);
			object.Set_Affiliation (affiliation);
			object.Set_Model_Name (model_name);
			object.Set_Anim_Name (anim_name);
			object.Set_Definition_Name (definition_name);
			object.Set_Min_Distance (min_dist);
			object.Set_Player_Type (player_type);

			//
			//	Add this object to the list
			//
			ObjectList.Add (object);
		}
		delete ini_file;
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_ListCtrl_Sel_Change
//
////////////////////////////////////////////////////////////////
void
EvaViewerTabClass::On_ListCtrl_Sel_Change
(
	ListCtrlClass *	/* list_ctrl */,
	int					/* ctrl_id */,
	int					/* old_index */,
	int					new_index
)
{
	View_Entry (new_index);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	View_Entry
//
////////////////////////////////////////////////////////////////
void
EvaViewerTabClass::View_Entry (int entry_index)
{
	WideStringClass name;
	WideStringClass description;
	WideStringClass affiliation;
	StringClass model_name;
	StringClass anim_name;
	StringClass	definition_name;
	float min_dist		= 0.0F;

	//
	//	Is there new data to display?
	//
	if (entry_index >= 0) {

		//
		//	Load the data for this selection
		//
		EvaViewerObjectClass *object = (EvaViewerObjectClass *)ListCtrl->Get_Entry_Data (entry_index, 0);
		name			= object->Get_Name ();
		description	= object->Get_Description ();
		affiliation = object->Get_Affiliation ();
		model_name	= object->Get_Model_Name ();
		anim_name	= object->Get_Anim_Name ();
		min_dist		= object->Get_Min_Distance ();
		definition_name = object->Get_Definition_Name ();
	}

	//
	//	Display the data
	//
	if (DescriptionCtrl != NULL) {
		DescriptionCtrl->Set_Text (description);
	}

	if (AffiliationCtrl != NULL) {
		AffiliationCtrl->Set_Text (affiliation);
	}

	if (ViewerCtrl != NULL) {
		ViewerCtrl->Set_Camera_Min_Dist (min_dist);

		//
		//	Special-case vehicles so we can get the wheels to "drop"
		//
		if (EncyclopediaType == EncyclopediaMgrClass::TYPE_VEHICLE) {
			
			RenderObjClass *new_model = NULL;

			//
			//	Try to find the definition that this vehicle is defined by
			//
			DefinitionClass *definition = DefinitionMgrClass::Find_Typed_Definition (definition_name, CLASSID_GAME_OBJECTS);
			if (definition != NULL) {
				
				PhysicalGameObj *game_obj = (PhysicalGameObj *)definition->Create ();
				if (game_obj != NULL) {
					
					//
					//	Is the physics object associated with this game object a vehicle?
					//
					PhysClass *phys_obj = game_obj->Peek_Physical_Object ();
					if (phys_obj != NULL && phys_obj->As_VehiclePhysClass () != NULL) {
						
						//
						//	Drop the wheels on this object and clone its model
						//
						phys_obj->As_VehiclePhysClass ()->Non_Physical_Wheel_Update (0.7F, 0);
						new_model = phys_obj->Peek_Model ()->Clone ();
					}

					//
					//	Delete the game object
					//
					game_obj->Set_Delete_Pending ();
				}
			}

			//
			//	Set the model we dug out of the definition, or
			// simply create the object from its W3D
			//
			if (new_model != NULL) {
				ViewerCtrl->Set_Model (new_model);
			} else {
				ViewerCtrl->Set_Model (model_name);
			}
			
		} else {
			ViewerCtrl->Set_Model (model_name);
		}

		//
		//	Play the animation on the object
		//
		ViewerCtrl->Set_Animation (anim_name);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_ViewerCtrl_Model_Loaded
//
////////////////////////////////////////////////////////////////
void
EvaViewerTabClass::On_ViewerCtrl_Model_Loaded
(
	ViewerCtrlClass *	/* viewer_ctrl */,
	int					/* ctrl_id */,
	RenderObjClass *	model
)
{
	if (model != NULL) {
		Prepare_Model (model);

		//
		//	Start the image rotating again...
		//
		ViewerCtrl->Set_Interface_Mode (ViewerCtrlClass::Z_ROTATION, 45.0F);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Prepare_Model
//
////////////////////////////////////////////////////////////////
void
EvaViewerTabClass::Prepare_Model (RenderObjClass *model)
{
	if (model != NULL) {

		//
		//	Attempt to hide the muzzle flashes
		//
		for (int index = 0; index < model->Get_Num_Sub_Objects(); index ++) {
			RenderObjClass *sub_obj = model->Get_Sub_Object (index);

			//
			//	Hide the muzzle flash (if necessary)
			//
			if (::strstr (sub_obj->Get_Name (),"MUZZLEFLASH") != NULL)  {
				sub_obj->Set_Hidden (true);
			}

			//
			//	Recurse into this sub-object
			//
			Prepare_Model (sub_obj);

			REF_PTR_RELEASE (sub_obj);
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	ListSortCallback
//
////////////////////////////////////////////////////////////////
int
EvaViewerTabClass::ListSortCallback
(
	ListCtrlClass *	list_ctrl,
	int					item_index1,
	int					item_index2,
	uint32				/* user_param */
)
{
	//
	//	Lookup the data associated with these entries
	//
	EvaViewerObjectClass *object1 = (EvaViewerObjectClass *)list_ctrl->Get_Entry_Data (item_index1, 0);
	EvaViewerObjectClass *object2 = (EvaViewerObjectClass *)list_ctrl->Get_Entry_Data (item_index2, 0);

	int player_type1 = object1->Get_Player_Type ();
	int player_type2 = object2->Get_Player_Type ();
	int result = 0;

	//
	//	Sort alphatically if the types are the same
	//
	if (player_type1 == player_type2) {
		result = ::u_strcasecmp (object1->Get_Name (), object2->Get_Name (), U_COMPARE_CODE_POINT_ORDER);
	} else {

		//
		//	Sort by type...
		//
		if (player_type1 == PLAYERTYPE_NEUTRAL) {
			result = -1;
		} else if (player_type2 == PLAYERTYPE_NEUTRAL) {
			result = 1;
		} else if (player_type1 == PLAYERTYPE_GDI) {
			result = -1;
		} else if (player_type2 == PLAYERTYPE_GDI) {
			result = 1;
		}
	}

	return result;
}
