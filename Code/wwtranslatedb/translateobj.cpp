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
 *                     $Archive:: /Commando/Code/wwtranslatedb/translateobj.cpp      $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 9/13/01 9:04a                                               $*
 *                                                                                             *
 *                    $Revision:: 9                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "translateobj.h"

#include "chunkio.h"
#include "persistfactory.h"
#include "translatedbids.h"
#include "translatedb.h"
#include "tdbcategories.h"


//////////////////////////////////////////////////////////////////////////////
//	Persist factory
//////////////////////////////////////////////////////////////////////////////
SimplePersistFactoryClass<TDBObjClass, CHUNKID_TRANSLATE_OBJ> _TranslateObjPersistFactory;


//////////////////////////////////////////////////////////////////////////////
//	Constants
//////////////////////////////////////////////////////////////////////////////
enum
{
	CHUNKID_VARIABLES			= 0x06141108,
	CHUNKID_BASE_CLASS,
	CHUNKID_ENGLISH_STRING,
	CHUNKID_TRANSLATED_STRING
};

enum
{
	VARID_ID							= 0x01,
	VARID_ID_DESC,
	VARID_STRING,
	VARID_ENGLISH_STRING,
	VARID_SOUND_ID,
	VARID_ANIMATION_NAME,
	VARID_CATEGORY_ID
};


///////////////////////////////////////////////////////////////////////
//
//	TDBObjClass
//
///////////////////////////////////////////////////////////////////////
TDBObjClass::TDBObjClass (void)	:
	ID (0),
	SoundID (uint32(-1)),
	CategoryID (CATEGORY_DEFAULT)
{
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	TDBObjClass
//
///////////////////////////////////////////////////////////////////////
TDBObjClass::TDBObjClass (const TDBObjClass &src)	:
	ID (0),
	SoundID (uint32(-1)),
	CategoryID (CATEGORY_DEFAULT)
{
	(*this) = src;
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	~TDBObjClass
//
///////////////////////////////////////////////////////////////////////
TDBObjClass::~TDBObjClass (void)
{
	return ;
}


/////////////////////////////////////////////////////////////////
//
//	operator=
//
/////////////////////////////////////////////////////////////////
const TDBObjClass &	
TDBObjClass::operator= (const TDBObjClass &src)
{
	TranslatedStrings	= src.TranslatedStrings;
	EnglishString		= src.EnglishString;
	IDDesc				= src.IDDesc;
	ID						= src.ID;
	SoundID				= src.SoundID;
	AnimationName		= src.AnimationName;
	CategoryID			= src.CategoryID;

	return *this;
}


///////////////////////////////////////////////////////////////////////
//
//	Get_Factory
//
///////////////////////////////////////////////////////////////////////
const PersistFactoryClass &
TDBObjClass::Get_Factory (void) const
{
	return _TranslateObjPersistFactory;
}


/////////////////////////////////////////////////////////////////
//
//	Save
//
/////////////////////////////////////////////////////////////////
bool
TDBObjClass::Save (ChunkSaveClass &csave)
{
	csave.Begin_Chunk (CHUNKID_BASE_CLASS);
		PersistClass::Save (csave);
	csave.End_Chunk ();	

	csave.Begin_Chunk (CHUNKID_VARIABLES);
		Save_Variables (csave);
	csave.End_Chunk ();

	//
	//	Write the english string to its own chunk
	//
	WRITE_WWSTRING_CHUNK (csave, CHUNKID_ENGLISH_STRING, EnglishString);

	//
	//	Are we saving all the translations or just the current one?
	//
	if (TranslateDBClass::Is_Single_Language_Export_Enabled ()) {
		
		//
		//	If we don't have the requested language, save the english
		//
		int lang_id = TranslateDBClass::Get_Current_Language ();
		if (lang_id >= TranslatedStrings.Count ()) {
			lang_id = TranslateDBClass::LANGID_ENGLISH;
		}
		
		//
		//	Save the string
		//
		const WideStringClass &string = TranslatedStrings[lang_id];
		WRITE_WIDESTRING_CHUNK (csave, CHUNKID_TRANSLATED_STRING, string);

	} else {
	
		//
		//	Write each translated string to its own chunk
		//
		for (int index = 0; index < TranslatedStrings.Count (); index ++) {
			const WideStringClass &string = TranslatedStrings[index];
			WRITE_WIDESTRING_CHUNK(csave, CHUNKID_TRANSLATED_STRING, string);;
		}
	}

	return true;
}


/////////////////////////////////////////////////////////////////
//
//	Load
//
/////////////////////////////////////////////////////////////////
bool
TDBObjClass::Load (ChunkLoadClass &cload)
{
	while (cload.Open_Chunk ()) {		
		switch (cload.Cur_Chunk_ID ()) {

			READ_WWSTRING_CHUNK (cload, CHUNKID_ENGLISH_STRING, EnglishString);

			case CHUNKID_TRANSLATED_STRING:
			{
				//
				//	Load the translated string from its chunk
				//
				WideStringClass string;
				cload.Read (string.Get_Buffer((cload.Cur_Chunk_Length () + 1) / 2), cload.Cur_Chunk_Length ());

				//
				//	Add the translated string to our list
				//
				TranslatedStrings.Add (string);
			}
			break;
			
			case CHUNKID_BASE_CLASS:
				PersistClass::Load (cload);
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
//	Save_Variables
//
/////////////////////////////////////////////////////////////////
void
TDBObjClass::Save_Variables (ChunkSaveClass &csave)
{	
	//
	//	Save each variable to its own micro chunk
	//
	WRITE_MICRO_CHUNK					(csave, VARID_ID,					ID);
	WRITE_MICRO_CHUNK					(csave, VARID_SOUND_ID,			SoundID);	
	WRITE_MICRO_CHUNK					(csave, VARID_CATEGORY_ID,		CategoryID);
	WRITE_MICRO_CHUNK_WWSTRING		(csave, VARID_ID_DESC,			IDDesc);
	WRITE_MICRO_CHUNK_WWSTRING		(csave, VARID_ANIMATION_NAME,	AnimationName);	
	return ;
}


/////////////////////////////////////////////////////////////////
//
//	Load_Variables
//
/////////////////////////////////////////////////////////////////
void
TDBObjClass::Load_Variables (ChunkLoadClass &cload)
{	
	while (cload.Open_Micro_Chunk ()) {
		switch (cload.Cur_Micro_Chunk_ID ()) {			
			READ_MICRO_CHUNK					(cload, VARID_ID,					ID);
			READ_MICRO_CHUNK					(cload, VARID_SOUND_ID,			SoundID);	
			READ_MICRO_CHUNK					(cload, VARID_CATEGORY_ID,		CategoryID);
			READ_MICRO_CHUNK_WWSTRING		(cload, VARID_ID_DESC,			IDDesc);
			READ_MICRO_CHUNK_WWSTRING		(cload, VARID_ENGLISH_STRING,	EnglishString);
			READ_MICRO_CHUNK_WWSTRING		(cload, VARID_ANIMATION_NAME,	AnimationName);

			case VARID_STRING:
			{
				//
				//	Load the translated string from its chunk
				//
				WideStringClass string;
				cload.Read (string.Get_Buffer((cload.Cur_Micro_Chunk_Length () + 1) / 2), cload.Cur_Micro_Chunk_Length ());

				//
				//	Add the translated string to our list
				//
				TranslatedStrings.Add (string);
			}
			break;
		}

		cload.Close_Micro_Chunk ();
	}

	return ;
}


/////////////////////////////////////////////////////////////////
//
//	Set_English_String
//
/////////////////////////////////////////////////////////////////
void
TDBObjClass::Set_English_String (const char *string)
{
	EnglishString = string;

	//
	//	Grow the translated string array out large enough so
	// we can index into the english string
	//
	while (TranslatedStrings.Count () <= TranslateDBClass::LANGID_ENGLISH) {
		TranslatedStrings.Add (WideStringClass (U_CHAR("")));
	}

	TranslatedStrings[TranslateDBClass::LANGID_ENGLISH].Convert_From (string);
	return ;
}


/////////////////////////////////////////////////////////////////
//
//	Set_ID
//
/////////////////////////////////////////////////////////////////
void
TDBObjClass::Set_ID (uint32 id)
{
	ID = id;
	return ;
}


/////////////////////////////////////////////////////////////////
//
//	Set_ID_Desc
//
/////////////////////////////////////////////////////////////////
void
TDBObjClass::Set_ID_Desc (const char *desc)
{
	IDDesc = desc;
	return ;
}


/////////////////////////////////////////////////////////////////
//
//	Get_String
//
/////////////////////////////////////////////////////////////////
const WideStringClass &
TDBObjClass::Get_String (void)
{
	//
	//	Lookup the translated string based on the current language
	//
	return Get_String (TranslateDBClass::Get_Current_Language ());
}


/////////////////////////////////////////////////////////////////
//
//	Get_String
//
/////////////////////////////////////////////////////////////////
const WideStringClass &
TDBObjClass::Get_String (uint32 lang_id)
{
	//
	//	Default to english on error
	//
	if (lang_id >= (uint32)TranslatedStrings.Count ()) {
		lang_id = TranslateDBClass::LANGID_ENGLISH;
	}

	return TranslatedStrings[lang_id];
}


/////////////////////////////////////////////////////////////////
//
//	Set_String
//
/////////////////////////////////////////////////////////////////
void
TDBObjClass::Set_String (uint32 lang_id, const unichar_t *string)
{
	//
	//	Grow the translated strings array until we have enough
	// to cover the requested language
	//
	while ((uint32)TranslatedStrings.Count () <= lang_id) {
		TranslatedStrings.Add (WideStringClass (U_CHAR("")));
	}

	//
	//	Set the string
	//
	TranslatedStrings[lang_id] = string;

	//
	//	Keep a copy of the english string (if necessary)
	//
	if (lang_id == TranslateDBClass::LANGID_ENGLISH) {
		TranslatedStrings[lang_id].Convert_To (EnglishString);
	}

	return ;
}


/////////////////////////////////////////////////////////////////
//
//	Contains_Translation
//
/////////////////////////////////////////////////////////////////
bool
TDBObjClass::Contains_Translation (uint32 lang_id)
{
	bool retval = false;

	//
	//	Simply check to see if the string contains any data
	//
	if (TranslatedStrings.Count () > (int)lang_id) {
		retval = (TranslatedStrings[lang_id].Get_Length () > 0);
	}
	
	return retval;
}
