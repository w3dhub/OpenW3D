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
 *                 Project Name : wwtranslatedb																  *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwtranslatedb/stringtwiddler.cpp     $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 8/01/01 4:26p                                               $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "stringtwiddler.h"

#include "chunkio.h"
#include "persistfactory.h"
#include "translatedbids.h"
#include "translatedb.h"
#include "tdbcategories.h"


//////////////////////////////////////////////////////////////////////////////
//	Persist factory
//////////////////////////////////////////////////////////////////////////////
SimplePersistFactoryClass<StringTwiddlerClass, CHUNKID_STRING_TWIDDLER> _StringTwiddlerPersistFactory;


//////////////////////////////////////////////////////////////////////////////
//	Constants
//////////////////////////////////////////////////////////////////////////////
enum
{
	CHUNKID_VARIABLES			= 0x07310319,
	CHUNKID_BASE_CLASS,
};

enum
{
	VARID_STRING_ID			= 0x01,
};


///////////////////////////////////////////////////////////////////////
//
//	StringTwiddlerClass
//
///////////////////////////////////////////////////////////////////////
StringTwiddlerClass::StringTwiddlerClass (void)
{
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	StringTwiddlerClass
//
///////////////////////////////////////////////////////////////////////
StringTwiddlerClass::StringTwiddlerClass (const StringTwiddlerClass &src)	:
	TDBObjClass (src)
{
	(*this) = src;
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	~StringTwiddlerClass
//
///////////////////////////////////////////////////////////////////////
StringTwiddlerClass::~StringTwiddlerClass (void)
{
	return ;
}


/////////////////////////////////////////////////////////////////
//
//	operator=
//
/////////////////////////////////////////////////////////////////
const StringTwiddlerClass &
StringTwiddlerClass::operator= (const StringTwiddlerClass &src)
{
	TDBObjClass::operator= (src);
	StringList = src.StringList;
	return *this;
}


///////////////////////////////////////////////////////////////////////
//
//	Get_Factory
//
///////////////////////////////////////////////////////////////////////
const PersistFactoryClass &
StringTwiddlerClass::Get_Factory (void) const
{
	return _StringTwiddlerPersistFactory;
}


/////////////////////////////////////////////////////////////////
//
//	Save
//
/////////////////////////////////////////////////////////////////
bool
StringTwiddlerClass::Save (ChunkSaveClass &csave)
{
	csave.Begin_Chunk (CHUNKID_BASE_CLASS);
		TDBObjClass::Save (csave);
	csave.End_Chunk ();

	csave.Begin_Chunk (CHUNKID_VARIABLES);

		//
		//	Write each string ID to its own microchunk
		//
		for (int index = 0; index < StringList.Count (); index ++) {
			int string_id = StringList[index];
			WRITE_MICRO_CHUNK (csave, VARID_STRING_ID, string_id);
		}

	csave.End_Chunk ();

	return true;
}


/////////////////////////////////////////////////////////////////
//
//	Load
//
/////////////////////////////////////////////////////////////////
bool
StringTwiddlerClass::Load (ChunkLoadClass &cload)
{
	while (cload.Open_Chunk ()) {
		switch (cload.Cur_Chunk_ID ()) {

			case CHUNKID_BASE_CLASS:
				TDBObjClass::Load (cload);
				break;

			case CHUNKID_VARIABLES:
				Load_Variables (cload);
				break;
		}

		cload.Close_Chunk ();
	}

	return true;
}


/////////////////////////////////////////////////////////////////
//
//	Load_Variables
//
/////////////////////////////////////////////////////////////////
void
StringTwiddlerClass::Load_Variables (ChunkLoadClass &cload)
{
	while (cload.Open_Micro_Chunk ()) {
		switch (cload.Cur_Micro_Chunk_ID ()) {

			case VARID_STRING_ID:
			{
				//
				//	Read the data from the chunk
				//
				int string_id = 0;
				LOAD_MICRO_CHUNK (cload, string_id);

				//
				//	Store this string ID in our list if its valid
				//
				if (string_id != 0) {
					StringList.Add (string_id);
				}
			}
			break;
		}

		cload.Close_Micro_Chunk ();
	}

	return ;
}


/////////////////////////////////////////////////////////////////
//
//	Get_String
//
/////////////////////////////////////////////////////////////////
const WideStringClass &
StringTwiddlerClass::Get_String (uint32 lang_id)
{
	//
	//	Copy the contents of one of the string objects
	// into our internal data
	//
	Randomize (lang_id);

	return TDBObjClass::Get_String (lang_id);
}


/////////////////////////////////////////////////////////////////
//
//	Randomize
//
/////////////////////////////////////////////////////////////////
void
StringTwiddlerClass::Randomize (int lang_id)
{
	//
	//	Are there any entries in our twiddler list?
	//
	int count = StringList.Count ();
	if (count > 0) {

		//
		//	Randomly pick a string from our list
		//
		int index = (rand () % count);
		TDBObjClass *object = TranslateDBClass::Find_Object (StringList[index]);
		if (object != NULL && object->As_StringTwiddlerClass () == NULL) {

			//
			//	Copy the string contents into ourselves
			//
			Set_String (lang_id, object->Get_String ());
			EnglishString	= object->Get_English_String ();
			SoundID			= object->Get_Sound_ID ();
			AnimationName	= object->Get_Animation_Name ();
		}
	}

	return ;
}
