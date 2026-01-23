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
 *                     $Archive:: /Commando/Code/Combat/encyclopediamgr.cpp                   $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/16/02 12:07p                                              $*
 *                                                                                             *
 *                    $Revision:: 11                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "encyclopediamgr.h"
#include "chunkio.h"
#include "saveload.h"
#include "ini.h"
#include "inisup.h"
#include "assets.h"
#include "damageablegameobj.h"
#include "globalsettings.h"
#include "soldier.h"
#include "gametype.h"
#include <algorithm>


///////////////////////////////////////////////////////////////////////
// Global singleton instance
///////////////////////////////////////////////////////////////////////
EncyclopediaMgrClass _TheEncyclopediaMgrSaveLoadSubsystem;


////////////////////////////////////////////////////////////////
//	Local constants
////////////////////////////////////////////////////////////////
enum
{
	CHUNKID_VARIABLES		= 0x09221214,
	CHUNKID_TYPE_DATA
};

enum
{
	VARID_TEXTURE_NAME		= 0x01,
	VARID_CENTER_POINT,
	VARID_SCALE,
};


const char *ENCY_INI_FILENAMES[EncyclopediaMgrClass::TYPE_COUNT] =
{
	"characters.ini",
	"weapons.ini",
	"vehicles.ini",
	"buildings.ini",
};


///////////////////////////////////////////////////////////////////////
// Static member initialization
///////////////////////////////////////////////////////////////////////
BooleanVectorClass	EncyclopediaMgrClass::KnownObjectVector[TYPE_COUNT];
BooleanVectorClass	EncyclopediaMgrClass::CopyOfKnownObjectVector[TYPE_COUNT];


//////////////////////////////////////////////////////////////////////
//
//	Initialize
//
//////////////////////////////////////////////////////////////////////
void
EncyclopediaMgrClass::Initialize (void)
{
	//
	//	Initialize the bit vector for each type
	//
	for (int index = 0; index < TYPE_COUNT; index ++) {
		Build_Bit_Vector ((TYPE)index);
	}

	Store_Data ();
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Shutdown
//
//////////////////////////////////////////////////////////////////////
void
EncyclopediaMgrClass::Shutdown (void)
{
	//
	//	Reset the bit vectors
	//
	for (int index = 0; index < TYPE_COUNT; index ++) {
		KnownObjectVector[index].Clear ();
	}

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Build_Bit_Vector
//
//////////////////////////////////////////////////////////////////////
void
EncyclopediaMgrClass::Build_Bit_Vector (TYPE type)
{
	int max_id = 0;

	//
	//	Get the INI file which contains the data for this type
	//
	INIClass *ini_file = ::Get_INI (ENCY_INI_FILENAMES[type]);
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
			int id = ini_file->Get_Int (section->Section, "ID");
			max_id = std::max (max_id, id);
		}

		// ST - 9/6/2001 11:48PM
		delete ini_file;
	}

	//
	//	Ensure the bit vector is large enough to hold this data
	//
	if (KnownObjectVector[type].Length () < max_id + 1) {
		KnownObjectVector[type].Resize (max_id + 1);
	}

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Reveal_Object
//
//////////////////////////////////////////////////////////////////////
bool
EncyclopediaMgrClass::Reveal_Object (TYPE type, int object_id)
{
	bool retval = false;

	//
	//	Don't set the data if the object ID is outside our range
	//
	//if (object_id < KnownObjectVector[type].Length ()) {
	if (IS_MISSION && object_id < KnownObjectVector[type].Length ()) {
		retval = (KnownObjectVector[type][object_id] != true);
		KnownObjectVector[type][object_id] = true;
	}
	
	return retval;
}


//////////////////////////////////////////////////////////////////////
//
//	Is_Object_Revealed
//
//////////////////////////////////////////////////////////////////////
bool
EncyclopediaMgrClass::Is_Object_Revealed (TYPE type, int object_id)
{
	bool retval = false;

	//
	//	Lookup the bit for this object...
	//
	if (object_id < KnownObjectVector[type].Length ()) {
		retval = KnownObjectVector[type][object_id];
	}

	return retval;
}


//////////////////////////////////////////////////////////////////////
//
//	Reveal_Objects
//
//////////////////////////////////////////////////////////////////////
void
EncyclopediaMgrClass::Reveal_Objects (TYPE type)
{
	KnownObjectVector[type].Set ();
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Reveal_All_Objects
//
//////////////////////////////////////////////////////////////////////
void
EncyclopediaMgrClass::Reveal_All_Objects (void)
{
	//
	//	Simply set all bits to true in each cateogry
	//
	for (int index = 0; index < TYPE_COUNT; index ++) {
		KnownObjectVector[index].Set ();
	}

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Hide_Objects
//
//////////////////////////////////////////////////////////////////////
void
EncyclopediaMgrClass::Hide_Objects (TYPE type)
{
	KnownObjectVector[type].Reset ();
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Hide_All_Objects
//
//////////////////////////////////////////////////////////////////////
void
EncyclopediaMgrClass::Hide_All_Objects (void)
{
	//
	//	Simply set all bits to false in each cateogry
	//
	for (int index = 0; index < TYPE_COUNT; index ++) {
		KnownObjectVector[index].Reset ();
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Save
//
////////////////////////////////////////////////////////////////
bool
EncyclopediaMgrClass::Save (ChunkSaveClass &csave)
{
	//
	//	Loop over and save the bit vector for each type to disk
	//
	for (int index = 0; index < TYPE_COUNT; index ++) {
		csave.Begin_Chunk (CHUNKID_TYPE_DATA);
			const VectorClass<unsigned char> &bit_vector = KnownObjectVector[index].Get_Bit_Array();
			if (bit_vector.Length () > 0) {
				csave.Write (&bit_vector[0], sizeof (bit_vector[0]) * bit_vector.Length ());
			}
		csave.End_Chunk ();
	}

	//
	//	Write the variables
	//
	csave.Begin_Chunk (CHUNKID_VARIABLES);
	csave.End_Chunk ();
	return true;
}


////////////////////////////////////////////////////////////////
//
//	Load
//
////////////////////////////////////////////////////////////////
bool
EncyclopediaMgrClass::Load (ChunkLoadClass &cload)
{
	//
	//	Start clean...
	//
	Hide_All_Objects ();

	int type_index = 0;

	while (cload.Open_Chunk ()) {
		switch (cload.Cur_Chunk_ID ()) {

			//
			//	Load all the variables from this chunk
			//
			case CHUNKID_VARIABLES:
				Load_Variables (cload);
				break;

			case CHUNKID_TYPE_DATA:
			{
				//
				//	Read the bit vector from its chunk
				//
				int size = cload.Cur_Chunk_Length ();
				
				int vector_size = std::max (size, KnownObjectVector[type_index].Get_Bit_Array().Length ());
				//unsigned char *bit_vector = new unsigned char[vector_size];

				//
				//	Initialize the known object array from the saved bit vector
				//
				int bitcount = vector_size * 8;
				KnownObjectVector[type_index].Init (bitcount);

				//
				//	Now read the data straight into the vector
				//
				const VectorClass<unsigned char> &bit_array = KnownObjectVector[type_index].Get_Bit_Array ();
				cload.Read (const_cast<unsigned char*>(&bit_array[0]), size);

				//
				//	Advance to the next entry
				//
				type_index ++;
				break;
			}
		}

		cload.Close_Chunk ();
	}

	return true;
}


////////////////////////////////////////////////////////////////
//
//	Load_Variables
//
////////////////////////////////////////////////////////////////
void
EncyclopediaMgrClass::Load_Variables (ChunkLoadClass &/* cload */)
{
	/*while (cload.Open_Micro_Chunk ()) {
		switch (cload.Cur_Micro_Chunk_ID ()) {

			//READ_MICRO_CHUNK_WWSTRING (cload, VARID_TEXTURE_NAME,	MapTextureName);
			//READ_MICRO_CHUNK (cload, VARID_CENTER_POINT,				MapCenterPoint);
			//READ_MICRO_CHUNK (cload, VARID_SCALE,						MapScale);
			default:
				break;
		}

		cload.Close_Micro_Chunk ();
	}*/

	//SaveLoadSystemClass::Register_Post_Load_Callback (this);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Reveal_Object
//
////////////////////////////////////////////////////////////////
bool
EncyclopediaMgrClass::Reveal_Object (DamageableGameObj *game_obj)
{
	if (game_obj == NULL) {
		return false;
	}

	if (!IS_MISSION) {
		// TSS092601 - we don't want to display datalink updated msg in MP.
		return false;
	}

	//
	//	Get information about this entry
	//
	EncyclopediaMgrClass::TYPE	type	= game_obj->Get_Definition ().Get_Encyclopedia_Type ();
	int id									= game_obj->Get_Definition ().Get_Encyclopedia_ID ();

	//
	//	If this is an expected type, then reveal the object to the user
	//
	bool retval = false;
	if (type != TYPE_UNKNOWN) {

		//
		//	Display some text and play a sound if the player hasn't gotten
		// this entry yet.
		//
		if (Is_Object_Revealed (type, id) == false) {
			Display_Event_UI ();
		}

		retval = Reveal_Object (type, id);		
	}

	return retval;
}


////////////////////////////////////////////////////////////////
//
//	Display_Event_UI
//
////////////////////////////////////////////////////////////////
void
EncyclopediaMgrClass::Display_Event_UI (void)
{
	//
	//	Get the current encyclopedia event string and sound ID
	//
	int strings_id = GlobalSettingsDef::Get_Global_Settings ()->Get_Encyclopedia_Event_String_ID ();
	if (strings_id != 0) {

		//
		//	Display the text and play the sound
		//
		SoldierGameObj::Say_Dynamic_Dialogue( strings_id, NULL );
	}

	return ;
}



////////////////////////////////////////////////////////////////
//
//	Store_Data
//
////////////////////////////////////////////////////////////////
void
EncyclopediaMgrClass::Store_Data (void)
{
	for (int index = 0; index < TYPE_COUNT; index ++) {
		CopyOfKnownObjectVector[index] = KnownObjectVector[index];
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	Restore_Data
//
////////////////////////////////////////////////////////////////
void
EncyclopediaMgrClass::Restore_Data (void)
{
	for (int index = 0; index < TYPE_COUNT; index ++) {
		KnownObjectVector[index] = CopyOfKnownObjectVector[index];
	}
	
	return ;
}
