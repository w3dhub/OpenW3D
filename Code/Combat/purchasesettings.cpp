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
 *                     $Archive:: /Commando/Code/Combat/purchasesettings.cpp                  $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/05/02 6:04p                                               $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "purchasesettings.h"
#include "combatchunkid.h"
#include "wwhack.h"
#include "persistfactory.h"
#include "definitionfactory.h"
#include "simpledefinitionfactory.h"
#include "debug.h"
#include "translatedb.h"


//////////////////////////////////////////////////////////////////////
//	Force links
//////////////////////////////////////////////////////////////////////
DECLARE_FORCE_LINK (PurchaseSettings)


///////////////////////////////////////////////////////////////////////////////////////////
//	Factories
///////////////////////////////////////////////////////////////////////////////////////////
SimplePersistFactoryClass<PurchaseSettingsDefClass, CHUNKID_GLOBAL_SETTINGS_DEF_PURCHASE>	_PurchaseDefPersistFactory;
DECLARE_DEFINITION_FACTORY(PurchaseSettingsDefClass, CLASSID_GLOBAL_SETTINGS_DEF_PURCHASE, "Purchase Settings") _PurchaseDefDefFactory;


///////////////////////////////////////////////////////////////////////////////////////////
//	Save/load constants
///////////////////////////////////////////////////////////////////////////////////////////
enum
{
	CHUNKID_PARENT				=	0x08071203,
	CHUNKID_VARIABLES,

	VARID_TEAM					= 1,
	VARID_TYPE,
	XXX_VARID_ROW,
	XXX_VARID_COL,
	VARID_COST,
	VARID_DEFINITION,
	VARID_TEXTURE_NAME,
	VARID_NAME,
	VARID_INDEX,
	VARID_ALT_INDEX,
	VARID_ALT_TEXTURE_NAME,
	VARID_ALT_DEFINITION
};


//////////////////////////////////////////////////////////////////////
//	Static member initialization
//////////////////////////////////////////////////////////////////////
PurchaseSettingsDefClass *		PurchaseSettingsDefClass::DefinitionArray[TYPE_COUNT][TEAM_COUNT]	= { 0 };

//////////////////////////////////////////////////////////////////////
//
//	PurchaseSettingsDefClass
//
//////////////////////////////////////////////////////////////////////
PurchaseSettingsDefClass::PurchaseSettingsDefClass (void)	:
	Team (TEAM_GDI),
	Type (TYPE_CLASSES)
{
	//
	//	Initialize the cost and definition lists
	//
	::memset (CostList,						0, sizeof (CostList));
	::memset (DefinitionList,				0, sizeof (DefinitionList));
	::memset (NameList,						0, sizeof (NameList));
	::memset (AlternateDefinitionList,	0, sizeof (AlternateDefinitionList));	

	//
	//	Configure the enum parameters for the editable system
	//
	#ifdef PARAM_EDITING_ON
		EnumParameterClass *param1 = new EnumParameterClass ((int *)&Team);
		param1->Set_Name ("Team");
		param1->Add_Value ("GDI",			TEAM_GDI);
		param1->Add_Value ("NOD",			TEAM_NOD);
		param1->Add_Value ("Mutant GDI",	TEAM_MUTANT_GDI);
		param1->Add_Value ("Mutant NOD",	TEAM_MUTANT_NOD);
		GENERIC_EDITABLE_PARAM (PurchaseSettingsDefClass, param1)

		EnumParameterClass *param2 = new EnumParameterClass ((int *)&Type);
		param2->Set_Name ("Team");
		param2->Add_Value ("Character Classes",	TYPE_CLASSES);
		param2->Add_Value ("Vehicles",				TYPE_VEHICLES);
		param2->Add_Value ("Equipment",				TYPE_EQUIPMENT);
		param2->Add_Value ("Secret Classes",		TYPE_SECRET_CLASSES);
		param2->Add_Value ("Secret Vehicles",		TYPE_SECRET_VEHICLES);
		GENERIC_EDITABLE_PARAM (PurchaseSettingsDefClass, param2)
	#endif //PARAM_EDITING_ON

	//
	//	Configure the editable system
	//
	for (int index = 0; index < MAX_ENTRIES; index ++) {
		
		//
		//	Add a separator for this entry
		//
		StringClass name;
		name.Format ("Entry %d", index + 1);
		PARAM_SEPARATOR (PurchaseSettingsDefClass, (const char *)name);

		//
		//	Add fields for the cost, texture, and object
		//
		NAMED_EDITABLE_PARAM (PurchaseSettingsDefClass, ParameterClass::TYPE_STRINGSDB_ID,	NameList[index], "Name");
		NAMED_EDITABLE_PARAM (PurchaseSettingsDefClass, ParameterClass::TYPE_INT,				CostList[index], "Cost");
		NAMED_EDITABLE_PARAM (PurchaseSettingsDefClass, ParameterClass::TYPE_STRING,			TextureList[index], "Texture");

		#ifdef PARAM_EDITING_ON
			GenericDefParameterClass *param = new GenericDefParameterClass (&(DefinitionList[index]));
			param->Set_Class_ID (CLASSID_GAME_OBJECTS);
			param->Set_Name ("Object");
			GENERIC_EDITABLE_PARAM(PurchaseSettingsDefClass, param)

			//
			//	Insert the alternate textures and definitions
			//
			for (int alt_index = 0; alt_index < MAX_ALTERNATES; alt_index ++) {
				
				name.Format ("Alt Texture %d", alt_index + 1);
				NAMED_EDITABLE_PARAM (PurchaseSettingsDefClass, ParameterClass::TYPE_STRING, AlternateTextureList[index][alt_index], name);

				name.Format ("Alt Object %d", alt_index + 1);		
				GenericDefParameterClass *param = new GenericDefParameterClass (&(AlternateDefinitionList[index][alt_index]));
				param->Set_Class_ID (CLASSID_GAME_OBJECTS);
				param->Set_Name (name);
				GENERIC_EDITABLE_PARAM (PurchaseSettingsDefClass, param)
			}

		#endif //PARAM_EDITING_ON

	}

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	~PurchaseSettingsDefClass
//
//////////////////////////////////////////////////////////////////////
PurchaseSettingsDefClass::~PurchaseSettingsDefClass (void)
{
	//
	//	Remove this entry from the static array
	//
	if (Type < TYPE_COUNT && Team < TEAM_COUNT) {
		DefinitionArray[Type][Team] = NULL;
	}
	return ;
}


///////////////////////////////////////////////////////////////////////////////////////////
//
//	Get_Class_ID
//
///////////////////////////////////////////////////////////////////////////////////////////
uint32
PurchaseSettingsDefClass::Get_Class_ID (void) const
{ 
	return CLASSID_GLOBAL_SETTINGS_DEF_PURCHASE; 
}


///////////////////////////////////////////////////////////////////////////////////////////
//
//	Get_Factory
//
///////////////////////////////////////////////////////////////////////////////////////////
const PersistFactoryClass &
PurchaseSettingsDefClass::Get_Factory (void) const
{ 
	return _PurchaseDefPersistFactory; 
}


///////////////////////////////////////////////////////////////////////////////////////////
//
//	Create
//
///////////////////////////////////////////////////////////////////////////////////////////
PersistClass *
PurchaseSettingsDefClass::Create (void) const 
{
	WWASSERT (0);
	return NULL;
}


///////////////////////////////////////////////////////////////////////////////////////////
//
//	Save
//
///////////////////////////////////////////////////////////////////////////////////////////
bool
PurchaseSettingsDefClass::Save (ChunkSaveClass &csave)
{
	csave.Begin_Chunk (CHUNKID_PARENT);
		DefinitionClass::Save (csave);
	csave.End_Chunk ();

	csave.Begin_Chunk (CHUNKID_VARIABLES);

		//
		//	Save the simple variables
		//
		WRITE_MICRO_CHUNK (csave, VARID_TEAM,	Team);
		WRITE_MICRO_CHUNK (csave, VARID_TYPE,	Type);

		//
		//	Save the lists
		//
		for (int index = 0; index < MAX_ENTRIES; index ++) {
			WRITE_MICRO_CHUNK (csave, VARID_INDEX,							index);
			WRITE_MICRO_CHUNK (csave, VARID_COST,							CostList[index]);
			WRITE_MICRO_CHUNK (csave, VARID_DEFINITION,					DefinitionList[index]);
			WRITE_MICRO_CHUNK (csave, VARID_NAME,							NameList[index]);			
			WRITE_MICRO_CHUNK_WWSTRING (csave, VARID_TEXTURE_NAME,	TextureList[index]);

			for (int alt_index = 0; alt_index < MAX_ALTERNATES; alt_index ++) {
				WRITE_MICRO_CHUNK (csave, VARID_ALT_INDEX,						alt_index);
				WRITE_MICRO_CHUNK_WWSTRING (csave, VARID_ALT_TEXTURE_NAME,	AlternateTextureList[index][alt_index]);
				WRITE_MICRO_CHUNK (csave, VARID_ALT_DEFINITION,		AlternateDefinitionList[index][alt_index]);
			}
		}

	csave.End_Chunk();

	return true;
}


///////////////////////////////////////////////////////////////////////////////////////////
//
//	Load
//
///////////////////////////////////////////////////////////////////////////////////////////
bool
PurchaseSettingsDefClass::Load (ChunkLoadClass &cload)
{
	while (cload.Open_Chunk ()) {
		switch(cload.Cur_Chunk_ID ()) {

			case CHUNKID_PARENT:
				DefinitionClass::Load (cload);
				break;
								
			case CHUNKID_VARIABLES:
				Load_Variables (cload);
				break;

			default:
				Debug_Say(("Unhandled Chunk:%d File:%s Line:%d\r\n",cload.Cur_Chunk_ID(),__FILE__,__LINE__));
				break;

		}

		cload.Close_Chunk();
	}

	return true;
}


///////////////////////////////////////////////////////////////////////////////////////////
//
//	Load_Variables
//
///////////////////////////////////////////////////////////////////////////////////////////
void
PurchaseSettingsDefClass::Load_Variables (ChunkLoadClass &cload)
{
	int entry_index	= 0;
	int alt_index		= 0;

	while (cload.Open_Micro_Chunk ()) {
		switch(cload.Cur_Micro_Chunk_ID ()) {
			
			//
			//	Read the type and team information
			//
			READ_MICRO_CHUNK (cload, VARID_TEAM,	Team);
			READ_MICRO_CHUNK (cload, VARID_TYPE,	Type);

			//
			//	Read the current list entry
			//
			READ_MICRO_CHUNK (cload, VARID_INDEX,		entry_index);
			READ_MICRO_CHUNK (cload, VARID_ALT_INDEX,	alt_index);			
																						
			case VARID_COST:
				if (entry_index >= 0 && entry_index < MAX_ENTRIES) {
					LOAD_MICRO_CHUNK (cload, CostList[entry_index]);
				}
				break;

			case VARID_DEFINITION:
				if (entry_index >= 0 && entry_index < MAX_ENTRIES) {
					LOAD_MICRO_CHUNK (cload, DefinitionList[entry_index]);
				}
				break;

			case VARID_NAME:
				if (entry_index >= 0 && entry_index < MAX_ENTRIES) {
					LOAD_MICRO_CHUNK (cload, NameList[entry_index]);			
				}
				break;
			
			case VARID_TEXTURE_NAME:
				if (entry_index >= 0 && entry_index < MAX_ENTRIES) {
					LOAD_MICRO_CHUNK_WWSTRING (cload, TextureList[entry_index]);
				}
				break;

			case VARID_ALT_DEFINITION:
				if (entry_index >= 0 && entry_index < MAX_ENTRIES && alt_index >= 0 && alt_index < MAX_ALTERNATES) {
					LOAD_MICRO_CHUNK (cload, AlternateDefinitionList[entry_index][alt_index]);
				}
				break;

			case VARID_ALT_TEXTURE_NAME:
				if (entry_index >= 0 && entry_index < MAX_ENTRIES && alt_index >= 0 && alt_index < MAX_ALTERNATES) {
					LOAD_MICRO_CHUNK_WWSTRING (cload, AlternateTextureList[entry_index][alt_index]);
				}
				break;

			default:
				Debug_Say (("Unhandled Micro Chunk:%d File:%s Line:%d\r\n", cload.Cur_Micro_Chunk_ID (), __FILE__, __LINE__));
				break;
		}

		cload.Close_Micro_Chunk();
	}

	//
	//	Add this definition to the static array
	//
	if (Type < TYPE_COUNT && Team < TEAM_COUNT) {
		DefinitionArray[Type][Team] = this;
	}
	return ;
}


///////////////////////////////////////////////////////////////////////////////////////////
//
//	Get_Name
//
///////////////////////////////////////////////////////////////////////////////////////////
const unichar_t *
PurchaseSettingsDefClass::Get_Name (int index)
{
	const unichar_t *retval = NULL;

	//
	//	Return the translated string...
	//
	if (index >= 0 && index < MAX_ENTRIES && NameList[index] != 0) {
		retval = TRANSLATE (NameList[index]);
	}

	return retval;
}


///////////////////////////////////////////////////////////////////////////////////////////
//
//	Find_Definition
//
///////////////////////////////////////////////////////////////////////////////////////////
PurchaseSettingsDefClass *
PurchaseSettingsDefClass::Find_Definition (TYPE type, TEAM team)
{
	PurchaseSettingsDefClass *retval = DefinitionArray[type][team];

	if (retval == NULL) {

		//
		// If the type is "secret classes" try the base classes
		//
		if (type == TYPE_SECRET_CLASSES) {
			type = TYPE_CLASSES;
		}
		if (type == TYPE_SECRET_VEHICLES) {
			type = TYPE_VEHICLES;
		}

		//
		//	Return the "base" definition if there was no definition
		// for one of the mutant types
		//
		if (team == TEAM_MUTANT_GDI) {
			retval = DefinitionArray[type][TEAM_GDI];
		} else if (team == TEAM_MUTANT_NOD) {
			retval = DefinitionArray[type][TEAM_NOD];
		}
	}

	return retval;
}

