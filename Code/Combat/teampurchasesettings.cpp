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
 *                 Project Name : combat                                                       *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Combat/teampurchasesettings.cpp              $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 10/23/01 3:22p                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "teampurchasesettings.h"
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
DECLARE_FORCE_LINK (TeamPurchaseSettings)


///////////////////////////////////////////////////////////////////////////////////////////
//	Factories
///////////////////////////////////////////////////////////////////////////////////////////
SimplePersistFactoryClass<TeamPurchaseSettingsDefClass, CHUNKID_GLOBAL_SETTINGS_DEF_TEAM_PURCHASE>	_TeamPurchaseDefPersistFactory;
DECLARE_DEFINITION_FACTORY(TeamPurchaseSettingsDefClass, CLASSID_GLOBAL_SETTINGS_DEF_TEAM_PURCHASE, "Team Purchase Settings") _TeamPurchaseDefDefFactory;


///////////////////////////////////////////////////////////////////////////////////////////
//	Save/load constants
///////////////////////////////////////////////////////////////////////////////////////////
enum
{
	CHUNKID_PARENT				=	0x10231215,
	CHUNKID_VARIABLES,

	VARID_TEAM					= 1,
	VARID_DEFINITION,
	VARID_TEXTURE_NAME,
	VARID_NAME,
	VARID_INDEX,
	VARID_BEACON_DEFINITION,
	VARID_BEACON_NAME,
	VARID_BEACON_TEXTURE_NAME,
	VARID_BEACON_COST,
	VARID_SUPPLY_NAME,
	VARID_SUPPLY_TEXTURE_NAME
};


//////////////////////////////////////////////////////////////////////
//	Static member initialization
//////////////////////////////////////////////////////////////////////
TeamPurchaseSettingsDefClass *		TeamPurchaseSettingsDefClass::DefinitionArray[TEAM_COUNT] = { 0 };

//////////////////////////////////////////////////////////////////////
//
//	TeamPurchaseSettingsDefClass
//
//////////////////////////////////////////////////////////////////////
TeamPurchaseSettingsDefClass::TeamPurchaseSettingsDefClass (void)	:
	Team (TEAM_GDI),
	BeaconNameID (0),
	BeaconDefinitionID (0),
	BeaconCost (0),
	SupplyNameID (0)
{
	//
	//	Initialize the lists
	//
	::memset (DefinitionList,	0, sizeof (DefinitionList));
	::memset (NameList,			0, sizeof (NameList));

	//
	//	Configure the enum parameters for the editable system
	//
	#ifdef PARAM_EDITING_ON
		EnumParameterClass *param1 = new EnumParameterClass ((int *)&Team);
		param1->Set_Name ("Team");
		param1->Add_Value ("GDI",			TEAM_GDI);
		param1->Add_Value ("NOD",			TEAM_NOD);
		GENERIC_EDITABLE_PARAM (TeamPurchaseSettingsDefClass, param1)
	#endif //PARAM_EDITING_ON

	//
	//	Add the beacon parameters
	//
	NAMED_EDITABLE_PARAM (TeamPurchaseSettingsDefClass, ParameterClass::TYPE_STRINGSDB_ID,	BeaconNameID, "Beacon Name");
	NAMED_EDITABLE_PARAM (TeamPurchaseSettingsDefClass, ParameterClass::TYPE_STRING,			BeaconTextureName, "Beacon Texture");
	NAMED_EDITABLE_PARAM (TeamPurchaseSettingsDefClass, ParameterClass::TYPE_INT,				BeaconCost, "Beacon Cost");	

	#ifdef PARAM_EDITING_ON
		GenericDefParameterClass *param = new GenericDefParameterClass (&BeaconDefinitionID);
		param->Set_Class_ID (CLASSID_GAME_OBJECTS);
		param->Set_Name ("Beacon Object");
		GENERIC_EDITABLE_PARAM (TeamPurchaseSettingsDefClass, param)
	#endif //PARAM_EDITING_ON

	//
	//	Add the supply parameters
	//
	NAMED_EDITABLE_PARAM (TeamPurchaseSettingsDefClass, ParameterClass::TYPE_STRINGSDB_ID,	SupplyNameID,			"Supply Name");
	NAMED_EDITABLE_PARAM (TeamPurchaseSettingsDefClass, ParameterClass::TYPE_STRING,			SupplyTextureName,	"Supply Texture");

	//
	//	Add the enlisted parameters
	//
	for (int index = 0; index < MAX_ENTRIES; index ++) {
		
		//
		//	Add a separator for this entry
		//
		StringClass name;
		name.Format ("Enlisted %d", index + 1);
		PARAM_SEPARATOR (TeamPurchaseSettingsDefClass, (const char *)name);

		//
		//	Add fields for the texture and object
		//
		NAMED_EDITABLE_PARAM (TeamPurchaseSettingsDefClass, ParameterClass::TYPE_STRINGSDB_ID,	NameList[index], "Name");
		NAMED_EDITABLE_PARAM (TeamPurchaseSettingsDefClass, ParameterClass::TYPE_STRING,			TextureList[index], "Texture");

		#ifdef PARAM_EDITING_ON
			GenericDefParameterClass *param = new GenericDefParameterClass (&(DefinitionList[index]));
			param->Set_Class_ID (CLASSID_GAME_OBJECTS);
			param->Set_Name ("Object");
			GENERIC_EDITABLE_PARAM(TeamPurchaseSettingsDefClass, param)
		#endif //PARAM_EDITING_ON

	}

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	~TeamPurchaseSettingsDefClass
//
//////////////////////////////////////////////////////////////////////
TeamPurchaseSettingsDefClass::~TeamPurchaseSettingsDefClass (void)
{
	//
	//	Remove this entry from the static array
	//
	if (Team < TEAM_COUNT) {
		DefinitionArray[Team] = NULL;
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////////////////////
//
//	Get_Class_ID
//
///////////////////////////////////////////////////////////////////////////////////////////
uint32
TeamPurchaseSettingsDefClass::Get_Class_ID (void) const
{ 
	return CLASSID_GLOBAL_SETTINGS_DEF_TEAM_PURCHASE; 
}


///////////////////////////////////////////////////////////////////////////////////////////
//
//	Get_Factory
//
///////////////////////////////////////////////////////////////////////////////////////////
const PersistFactoryClass &
TeamPurchaseSettingsDefClass::Get_Factory (void) const
{ 
	return _TeamPurchaseDefPersistFactory; 
}


///////////////////////////////////////////////////////////////////////////////////////////
//
//	Create
//
///////////////////////////////////////////////////////////////////////////////////////////
PersistClass *
TeamPurchaseSettingsDefClass::Create (void) const 
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
TeamPurchaseSettingsDefClass::Save (ChunkSaveClass &csave)
{
	csave.Begin_Chunk (CHUNKID_PARENT);
		DefinitionClass::Save (csave);
	csave.End_Chunk ();

	csave.Begin_Chunk (CHUNKID_VARIABLES);

		//
		//	Save the simple variables
		//
		WRITE_MICRO_CHUNK (csave, VARID_TEAM,									Team);
		WRITE_MICRO_CHUNK (csave, VARID_BEACON_DEFINITION,					BeaconDefinitionID);
		WRITE_MICRO_CHUNK (csave, VARID_BEACON_NAME,							BeaconNameID);
		WRITE_MICRO_CHUNK (csave, VARID_BEACON_COST,							BeaconCost);		
		WRITE_MICRO_CHUNK_WWSTRING (csave, VARID_BEACON_TEXTURE_NAME,	BeaconTextureName);
		WRITE_MICRO_CHUNK (csave, VARID_SUPPLY_NAME,							SupplyNameID);
		WRITE_MICRO_CHUNK_WWSTRING (csave, VARID_SUPPLY_TEXTURE_NAME,	SupplyTextureName);

		//
		//	Save the lists
		//
		for (int index = 0; index < MAX_ENTRIES; index ++) {
			WRITE_MICRO_CHUNK (csave, VARID_INDEX,							index);
			WRITE_MICRO_CHUNK (csave, VARID_DEFINITION,					DefinitionList[index]);
			WRITE_MICRO_CHUNK (csave, VARID_NAME,							NameList[index]);			
			WRITE_MICRO_CHUNK_WWSTRING (csave, VARID_TEXTURE_NAME,	TextureList[index]);			
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
TeamPurchaseSettingsDefClass::Load (ChunkLoadClass &cload)
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
TeamPurchaseSettingsDefClass::Load_Variables (ChunkLoadClass &cload)
{
	int entry_index = 0;

	while (cload.Open_Micro_Chunk ()) {
		switch(cload.Cur_Micro_Chunk_ID ()) {
			
			//
			//	Read the type and team information
			//
			READ_MICRO_CHUNK (cload, VARID_TEAM,	Team);
			READ_MICRO_CHUNK (cload, VARID_BEACON_DEFINITION,					BeaconDefinitionID);
			READ_MICRO_CHUNK (cload, VARID_BEACON_NAME,							BeaconNameID);			
			READ_MICRO_CHUNK (cload, VARID_BEACON_COST,							BeaconCost);		
			READ_MICRO_CHUNK_WWSTRING (cload, VARID_BEACON_TEXTURE_NAME,	BeaconTextureName);
			READ_MICRO_CHUNK (cload, VARID_SUPPLY_NAME,							SupplyNameID);
			READ_MICRO_CHUNK_WWSTRING (cload, VARID_SUPPLY_TEXTURE_NAME,	SupplyTextureName);

			//
			//	Read the current list entry
			//
			READ_MICRO_CHUNK (cload, VARID_INDEX,	entry_index);
																						
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

			default:
				Debug_Say (("Unhandled Micro Chunk:%d File:%s Line:%d\r\n", cload.Cur_Micro_Chunk_ID (), __FILE__, __LINE__));
				break;
		}

		cload.Close_Micro_Chunk();
	}

	//
	//	Add this definition to the static array
	//
	if (Team < TEAM_COUNT) {
		DefinitionArray[Team] = this;
	}
	return ;
}


///////////////////////////////////////////////////////////////////////////////////////////
//
//	Get_Enlisted_Name
//
///////////////////////////////////////////////////////////////////////////////////////////
const unichar_t *
TeamPurchaseSettingsDefClass::Get_Enlisted_Name (int index)
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
//	Get_Definition
//
///////////////////////////////////////////////////////////////////////////////////////////
TeamPurchaseSettingsDefClass *
TeamPurchaseSettingsDefClass::Get_Definition (TEAM team)
{
	return DefinitionArray[team];
}

