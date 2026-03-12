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
 *                 Project Name : LevelEdit                                                    *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/DefinitionUtils.cpp          $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 8/24/01 12:15p                                              $*
 *                                                                                             *
 *                    $Revision:: 11                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "StdAfx.h"
#include "definition.h"
#include "definitionmgr.h"
#include "definitionfactorymgr.h"
#include "definitionfactory.h"
#include "phys.h"
#include "Utils.h"
#include "physicalgameobj.h"
#include "tiledefinition.h"
#include "combatchunkid.h"
#include "dialogue.h"
#include "soldier.h"
#include "presetmgr.h"


///////////////////////////////////////////////////////////////////////
//
//	Create_Physics_Definition
//
///////////////////////////////////////////////////////////////////////
DefinitionClass *
Create_Physics_Definition (LPCTSTR base_class_name, bool is_temp)
{
	DefinitionClass *definition = NULL;

	//
	//	Loop over all the factories, until we've found one that matches
	// the base we are looking for
	//
	DefinitionFactoryClass *factory = NULL;
	for (	factory = DefinitionFactoryMgrClass::Get_First (CLASSID_PHYSICS);
			factory != NULL && definition == NULL;
			factory = DefinitionFactoryMgrClass::Get_Next (factory, CLASSID_PHYSICS))
	{
		//
		//	Instantiate a definition
		//
		definition = factory->Create ();
		if (definition != NULL) {

			//
			//	Is this the definition 'type' we want?
			//
			if (((PhysDefClass *)definition)->Is_Type (base_class_name) == false) {
				SAFE_DELETE (definition);
			} else {

				if (is_temp) {
					definition->Set_ID (::Get_Next_Temp_ID ());
				} else {
					definition->Set_ID (DefinitionMgrClass::Get_New_ID (definition->Get_Class_ID ()));
				}
			}
		}
	}

	return definition;
}


///////////////////////////////////////////////////////////////////////
//
//	Create_Definition
//
///////////////////////////////////////////////////////////////////////
DefinitionClass *
Create_Definition (int class_id, bool is_temp)
{
	DefinitionClass *definition = NULL;

	//
	//	Attempt to find a factory for this definition
	//
	DefinitionFactoryClass *factory = DefinitionFactoryMgrClass::Find_Factory (class_id);
	if (factory != NULL) {

		//
		//	Create a new instance of the definition
		//
		definition = factory->Create ();
		ASSERT (definition != NULL);

		//
		//	Give the definition a new id
		//
		if (is_temp) {
			definition->Set_ID (::Get_Next_Temp_ID ());
		} else {
			definition->Set_ID (DefinitionMgrClass::Get_New_ID (definition->Get_Class_ID ()));
		}
	}

	return definition;
}


///////////////////////////////////////////////////////////////////////
//
//	Copy_Definition
//
///////////////////////////////////////////////////////////////////////
void
Copy_Definition (DefinitionClass *src_def, DefinitionClass *dest_def, bool is_temp)
{
	SANITY_CHECK(src_def != NULL && dest_def != NULL) {
		return ;
	}

	//
	//	Make sure the definitions are of the same type.
	//
	uint32 class_id1 = dest_def->Get_Class_ID ();
	uint32 class_id2 = src_def->Get_Class_ID ();
	if (class_id1 == class_id2) {

		//
		//	Loop over all the parameters contained in the definitions
		//
		int count = dest_def->Get_Parameter_Count ();
		for (int index = 0; index < count; index ++) {
			ParameterClass *dest_param	= dest_def->Lock_Parameter (index);
			ParameterClass *src_param	= src_def->Lock_Parameter (index);

			if ((dest_param != NULL) && (src_param != NULL)) {

				//
				//	Is this parameter type a 'phys-def' parameter?
				///If it is, we need to make sure we don't copy the phys-def ID value,
				// we need to copy the properties of the phys-definition that the parameter
				// points to.
				//
				if (dest_param->Get_Type () == ParameterClass::TYPE_MODELDEFINITIONID) {

					int dest_def_id	= ((ModelDefParameterClass *)dest_param)->Get_Value ();
					int src_def_id		= ((ModelDefParameterClass *)src_param)->Get_Value ();
					DefinitionClass *dest_phys_def	= DefinitionMgrClass::Find_Definition (dest_def_id, false);
					DefinitionClass *src_phys_def		= DefinitionMgrClass::Find_Definition (src_def_id, false);

					//
					//	Create a new phys-def for the destination param
					//
					if (dest_phys_def == NULL && src_phys_def != NULL) {
						dest_phys_def = ::Create_Definition (src_phys_def->Get_Class_ID (), is_temp);
						if (dest_phys_def != NULL) {
							((ModelDefParameterClass *)dest_param)->Set_Value (dest_phys_def->Get_ID ());
							DefinitionMgrClass::Register_Definition (dest_phys_def);
						}
					}

					//
					//	If both the src and dest params point to valid physics-definitions,
					//	then, copy the properties from the src to the dest (recursion)
					//
					if (dest_phys_def != NULL && src_phys_def != NULL) {
						::Copy_Definition (src_phys_def, dest_phys_def, is_temp);
					}

				} else {

					//
					//	Copy the parameter value
					//
					dest_param->Copy_Value (*src_param);
				}
			}

			dest_def->Unlock_Parameter (index);
			src_def->Unlock_Parameter (index);
		}

		//
		//	Should we copy the dialogue list as well?
		//
		if (class_id1 == CLASSID_GAME_OBJECT_DEF_SOLDIER) {

			SoldierGameObjDef *src_soldier	= reinterpret_cast<SoldierGameObjDef *> (src_def);
			SoldierGameObjDef *dest_soldier	= reinterpret_cast<SoldierGameObjDef *> (dest_def);

			DialogueClass *src_list		= src_soldier->Get_Dialog_List ();
			DialogueClass *dest_list	= dest_soldier->Get_Dialog_List ();

			//
			//	Copy the settings from the base to the derived...
			//
			for (int index = 0; index < DIALOG_MAX; index ++) {
				dest_list[index] = src_list[index];
			}
		}
	}

	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Build_Embedded_Definition_List
//
///////////////////////////////////////////////////////////////////////
void
Build_Embedded_Definition_List (DEFINITION_LIST &list, DefinitionClass *parent)
{
	SANITY_CHECK(parent != NULL) {
		return ;
	}

	//
	//	Loop over all the parameters of this definition
	//
	int count = parent->Get_Parameter_Count ();
	for (int index = 0; index < count; index ++) {
		ParameterClass *param = parent->Lock_Parameter (index);

		//
		//	If this is the paramter type we are looking for, get the
		// definition pointer from it and return the pointer to the caller
		//
		if (param != NULL ) {

			if (param->Get_Type () == ParameterClass::TYPE_MODELDEFINITIONID) {
				int def_id = ((ModelDefParameterClass *)param)->Get_Value ();
				DefinitionClass *definition = DefinitionMgrClass::Find_Definition (def_id, false);
				if (definition != NULL) {
					list.Add (definition);

					//
					//	Now recurse into the model definition
					//
					Build_Embedded_Definition_List (list, definition);
				}

			} else if (param->Get_Type () == ParameterClass::TYPE_PHYSDEFINITIONID) {
				int def_id = ((PhysDefParameterClass *)param)->Get_Value ();
				DefinitionClass *definition = DefinitionMgrClass::Find_Definition (def_id, false);
				if (definition != NULL) {
					list.Add (definition);
				}
			}
		}

		parent->Unlock_Parameter (index);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Fix_Embedded_Definition_IDs
//
///////////////////////////////////////////////////////////////////////
void
Fix_Embedded_Definition_IDs (DefinitionClass *parent)
{
	SANITY_CHECK(parent != NULL) {
		return ;
	}

	//
	//	Loop over all the parameters of this definition
	//
	int count = parent->Get_Parameter_Count ();
	for (int index = 0; index < count; index ++) {
		ParameterClass *param = parent->Lock_Parameter (index);

		//
		//	If this is the paramter type we are looking for, get the
		// definition pointer from it and return the pointer to the caller
		//
		if (param != NULL ) {

			if (param->Get_Type () == ParameterClass::TYPE_MODELDEFINITIONID) {
				int def_id = ((ModelDefParameterClass *)param)->Get_Value ();
				DefinitionClass *definition = DefinitionMgrClass::Find_Definition (def_id, false);
				if (definition != NULL) {

					//
					//	Give this definition a new ID if necessary
					//
					if (definition->Get_ID () >= TEMP_DEF_ID_START) {
						definition->Set_ID (DefinitionMgrClass::Get_New_ID (definition->Get_Class_ID ()));
						((ModelDefParameterClass *)param)->Set_Value (definition->Get_ID ());
					}

					//
					//	Now recurse into the model definition
					//
					Fix_Embedded_Definition_IDs (definition);
				}

			} else if (param->Get_Type () == ParameterClass::TYPE_PHYSDEFINITIONID) {

				int def_id = ((PhysDefParameterClass *)param)->Get_Value ();
				DefinitionClass *definition = DefinitionMgrClass::Find_Definition (def_id, false);
				if (definition != NULL) {

					//
					//	Give this definition a new ID if necessary
					//
					if (definition->Get_ID () >= TEMP_DEF_ID_START) {
						definition->Set_ID (DefinitionMgrClass::Get_New_ID (definition->Get_Class_ID ()));
						((PhysDefParameterClass *)param)->Set_Value (definition->Get_ID ());
					}
				}
			}
		}

		parent->Unlock_Parameter (index);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Find_Physics_Definition
//
///////////////////////////////////////////////////////////////////////
DefinitionClass *
Find_Physics_Definition (DefinitionClass *parent)
{
	SANITY_CHECK(parent != NULL) {
		return NULL;
	}

	DefinitionClass *definition = NULL;

	//
	//	Loop over all the parameters of this definition
	//
	int count = parent->Get_Parameter_Count ();
	for (int index = 0; index < count && definition == NULL; index ++) {
		ParameterClass *param = parent->Lock_Parameter (index);

		//
		//	If this is the paramter type we are looking for, get the
		// definition pointer from it and return the pointer to the caller
		//
		if (param != NULL && param->Get_Type () == ParameterClass::TYPE_MODELDEFINITIONID) {
			int def_id = ((ModelDefParameterClass *)param)->Get_Value ();
			definition = DefinitionMgrClass::Find_Definition (def_id, false);
		}

		parent->Unlock_Parameter (index);
	}

	return definition;
}


///////////////////////////////////////////////////////////////////////
//
//	Get_Phys_Obj_From_Definition
//
///////////////////////////////////////////////////////////////////////
PhysClass *
Get_Phys_Obj_From_Definition (DefinitionClass *definition)
{
	SANITY_CHECK (definition != NULL) {
		return NULL;
	}

	PhysClass *retval = NULL;

	//
	//	What type of object will this definition create?
	//
	int class_id = definition->Get_Class_ID ();
	switch (::SuperClassID_From_ClassID (class_id))
	{
		/*case CLASSID_TERRAIN:
		{
			//
			//	Create the terrain and pull out its physics object
			//
			TerrainNodeClass *node = (TerrainNodeClass *)definition->Create ();
			if (node != NULL) {
				MEMBER_ADD (retval, node->Peek_Physics_Obj ());
				MEMBER_RELEASE (node);
			}
		}
		break;*/

		case CLASSID_TILE:
		{
			//
			//	Create the tile and pull out its physics object
			//
			int phys_def_id = ((TileDefinitionClass *)definition)->Get_Phys_Def_ID ();
			if (phys_def_id != 0) {
				DefinitionClass *phys_def = DefinitionMgrClass::Find_Definition (phys_def_id, false);
				if (phys_def != NULL) {
					retval = (PhysClass *)phys_def->Create ();
				}
			}
		}
		break;

		case CLASSID_GAME_OBJECTS:
		{
			//
			//	Create the game object and pull out its physics object
			//
			BaseGameObj *game_obj = (BaseGameObj *)definition->Create ();
			if (game_obj != NULL) {
				PhysicalGameObj *phys_game_obj = game_obj->As_PhysicalGameObj ();
				if (phys_game_obj != NULL) {
					MEMBER_ADD (retval, phys_game_obj->Peek_Physical_Object ());
				}

				game_obj->Set_Delete_Pending ();
			}
		}
		break;
	}

	return retval;
}

