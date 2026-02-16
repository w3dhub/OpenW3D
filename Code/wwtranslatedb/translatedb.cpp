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
 *                     $Archive:: /Commando/Code/wwtranslatedb/translatedb.cpp                $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/28/02 4:05p                                                $*
 *                                                                                             *
 *                    $Revision:: 18                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "translatedb.h"

#include <string.h>

#include "persist.h"
#include "persistfactory.h"
#include "chunkio.h"
#include "translatedbids.h"
#include "rawfile.h"
#include "textfile.h"
#include "tdbcategories.h"
#include "definition.h"
#include "definitionmgr.h"
#include "definitionclassids.h"
#include <algorithm>


///////////////////////////////////////////////////////////////////////
// Local prototypes
///////////////////////////////////////////////////////////////////////
static int Build_List_From_String (const char *buffer, const char *delimiter, StringClass **string_list);


///////////////////////////////////////////////////////////////////////
// Global singleton instance
///////////////////////////////////////////////////////////////////////
TranslateDBClass _TheTranslateDB;


///////////////////////////////////////////////////////////////////////
// Static member initialization
///////////////////////////////////////////////////////////////////////
TDB_OBJ_LIST						TranslateDBClass::m_ObjectList;
HashTemplateClass<StringClass,TDBObjClass*> TranslateDBClass::m_ObjectHash;
TDB_CATEGORY_LIST					TranslateDBClass::m_CategoryList;
uint32								TranslateDBClass::m_VersionNumber			= 100;
uint32								TranslateDBClass::m_LanguageID				= TranslateDBClass::LANGID_ENGLISH;
bool									TranslateDBClass::IsSingleLanguageExport	= false;
TranslateDBClass::FILTER_OPT	TranslateDBClass::FilterType					= FILTER_DISABLED;
uint32								TranslateDBClass::FilterCategoryID			= 0xFFFFFFFF;


///////////////////////////////////////////////////////////////////////
//	Constants
///////////////////////////////////////////////////////////////////////
enum
{
	CHUNKID_VARIABLES			= 0x07141200,
	CHUNKID_OBJECTS,
	CHUNKID_CATEGORIES
};


enum
{
	VARID_VERSION_NUMBER		= 0x01,
	VARID_LANGUAGE_ID
};

const unichar_t *	STRING_NOT_FOUND				= U_CHAR("TDBERR");
const char *	ENGLISH_STRING_NOT_FOUND	= "TDBERR";

///////////////////////////////////////////////////////////////////////
//
//	Chunk_ID
//
///////////////////////////////////////////////////////////////////////
uint32
TranslateDBClass::Chunk_ID (void) const
{
	return CHUNKID_TRANSLATE_DB;
}


///////////////////////////////////////////////////////////////////////
//
//	Initialize
//
///////////////////////////////////////////////////////////////////////
void
TranslateDBClass::Initialize (void)
{
	m_ObjectList.Set_Growth_Step (1000);
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Shutdown
//
///////////////////////////////////////////////////////////////////////
void
TranslateDBClass::Shutdown (void)
{
	Free_Objects ();
	Free_Categories ();
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Free_Categories
//
///////////////////////////////////////////////////////////////////////
void
TranslateDBClass::Free_Categories (void)
{
	//
	//	Loop over and free all the translation categories
	//
	for (int index = 0; index < m_CategoryList.Count (); index ++) {			
		TDBCategoryClass *category = m_CategoryList[index];
		if (category != NULL) {
			delete category;
		}
	}
	
	m_CategoryList.Delete_All ();
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Free_Objects
//
///////////////////////////////////////////////////////////////////////
void
TranslateDBClass::Free_Objects (void)
{
	//
	//	Loop over and free all the translation objects
	//
	for (int index = 0; index < m_ObjectList.Count (); index ++) {			
		TDBObjClass *translate_obj = m_ObjectList[index];
		if (translate_obj != NULL) {
			delete translate_obj;
		}
	}
	m_ObjectList.Delete_All ();

	// Remove the stuff from the hash table as well..
	m_ObjectHash.Remove_All();

	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Contains_Data
//
///////////////////////////////////////////////////////////////////////
bool
TranslateDBClass::Contains_Data (void) const
{
	return true;
}


///////////////////////////////////////////////////////////////////////
//
//	Save
//
///////////////////////////////////////////////////////////////////////
bool
TranslateDBClass::Save (ChunkSaveClass &csave)
{
	bool retval = true;

	csave.Begin_Chunk (CHUNKID_VARIABLES);
		WRITE_MICRO_CHUNK (csave, VARID_VERSION_NUMBER, m_VersionNumber);
		WRITE_MICRO_CHUNK (csave, VARID_LANGUAGE_ID,		m_LanguageID);
	csave.End_Chunk ();

	csave.Begin_Chunk (CHUNKID_CATEGORIES);

		//
		//	Loop over and save all the translation categories
		//
		int index;
		for (index = 0; index < m_CategoryList.Count (); index ++) {			
			TDBCategoryClass *category = m_CategoryList[index];

			//
			//	Save this category
			//
			if (category != NULL) {
				csave.Begin_Chunk (category->Get_Factory ().Chunk_ID ());
					category->Get_Factory ().Save (csave, category);
				csave.End_Chunk ();
			}
		}

	csave.End_Chunk ();
	
	csave.Begin_Chunk (CHUNKID_OBJECTS);

		//
		//	Loop over and save all the translation objects
		//
		for (index = 0; index < m_ObjectList.Count (); index ++) {			
			TDBObjClass *translate_obj = m_ObjectList[index];

			//
			//	Save this translation object
			//
			if (translate_obj != NULL) {
				
				//
				//	Check to ensure this category isn't filtered out...
				//
				bool is_equal = (translate_obj->Get_Category_ID () == FilterCategoryID);
				if (	FilterType == FILTER_DISABLED ||
						(FilterType == FILTER_IF_EQUAL && is_equal == false) ||
						(FilterType == FILTER_IF_NOT_EQUAL && is_equal))
				{
					csave.Begin_Chunk (translate_obj->Get_Factory ().Chunk_ID ());
					translate_obj->Get_Factory ().Save (csave, translate_obj);
					csave.End_Chunk ();
				}
			}
		}

	csave.End_Chunk ();

	return retval;
}


///////////////////////////////////////////////////////////////////////
//
//	Load
//
///////////////////////////////////////////////////////////////////////
bool
TranslateDBClass::Load (ChunkLoadClass &cload)
{
	Free_Objects ();
	Free_Categories ();

	bool retval = true;	
	while (cload.Open_Chunk ()) {
		switch (cload.Cur_Chunk_ID ()) {
			
			//
			//	Load all the presets from this chunk
			//
			case CHUNKID_VARIABLES:
				retval &= Load_Variables (cload);
				break;

			case CHUNKID_OBJECTS:
				retval &= Load_Objects (cload);
				break;

			case CHUNKID_CATEGORIES:
				retval &= Load_Categories (cload);
				break;
		}

		cload.Close_Chunk ();
	}

	Validate_Data ();
	return retval;
}


///////////////////////////////////////////////////////////////////////
//
//	Load_Categories
//
///////////////////////////////////////////////////////////////////////
bool
TranslateDBClass::Load_Categories (ChunkLoadClass &cload)
{
	bool retval = true;

	while (cload.Open_Chunk ()) {

		//
		//	Load this object from the chunk (if possible)
		//
		PersistFactoryClass *factory = SaveLoadSystemClass::Find_Persist_Factory (cload.Cur_Chunk_ID ());
		if (factory != NULL) {
			TDBCategoryClass *category = (TDBCategoryClass *)factory->Load (cload);
			if (category != NULL) {
				Add_Category (category, false);
			}
		}

		cload.Close_Chunk ();
	}

	return retval;
}


///////////////////////////////////////////////////////////////////////
//
//	Load_Objects
//
///////////////////////////////////////////////////////////////////////
bool
TranslateDBClass::Load_Objects (ChunkLoadClass &cload)
{
	bool retval = true;

	while (cload.Open_Chunk ()) {

		//
		//	Load this object from the chunk (if possible)
		//
		PersistFactoryClass *factory = SaveLoadSystemClass::Find_Persist_Factory (cload.Cur_Chunk_ID ());
		if (factory != NULL) {
			TDBObjClass *translate_obj = (TDBObjClass *)factory->Load (cload);
			if (translate_obj != NULL) {
				Add_Object (translate_obj);
			}
		}

		cload.Close_Chunk ();
	}

	return retval;
}


///////////////////////////////////////////////////////////////////////
//
//	Load_Variables
//
///////////////////////////////////////////////////////////////////////
bool
TranslateDBClass::Load_Variables (ChunkLoadClass &cload)
{
	bool retval = true;

	while (cload.Open_Micro_Chunk ()) {
		switch (cload.Cur_Micro_Chunk_ID ()) {

			READ_MICRO_CHUNK (cload, VARID_VERSION_NUMBER,	m_VersionNumber);
			READ_MICRO_CHUNK (cload, VARID_LANGUAGE_ID,		m_LanguageID);
		}
			
		cload.Close_Micro_Chunk ();
	}

	return retval;
}


///////////////////////////////////////////////////////////////////////
//
//	Validate_Data
//
///////////////////////////////////////////////////////////////////////
void
TranslateDBClass::Validate_Data (void)
{
	if (m_CategoryList.Count () == 0) {

		//
		//	Create the default category
		//
		TDBCategoryClass *category = new TDBCategoryClass;
		category->Set_Name ("Default");
		m_CategoryList.Add (category);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Export_Table
//
///////////////////////////////////////////////////////////////////////
void
TranslateDBClass::Export_Table (const char *filename)
{
	//
	//	Create the file
	//
    TextFileClass file_obj;
    file_obj.Open(filename, FileClass::WRITE);
    if (file_obj.Is_Open()) {

		//
		//	Loop over all the translation objects and write a tab delimited
		// entry for each one
		//
		for (int index = 0; index < m_ObjectList.Count (); index ++) {
			TDBObjClass *object = m_ObjectList[index];
			if (object != NULL && object->As_StringTwiddlerClass () == NULL) {
				
			StringClass english_string = object->Get_English_String ();

			const size_t length = english_string.Get_Length ();
			for (size_t index = 0; index < length; index ++) {
				if (english_string[static_cast<int>(index)] == '\n') {
					english_string[static_cast<int>(index)] = ' ';
				}
			}

				//
				//	Lookup the string's category
				//
				StringClass category_name;
				TDBCategoryClass *category = Find_Category (object->Get_Category_ID ());
				if (category != NULL) {
					category_name = category->Get_Name ();
				}

				//
				//	Lookup the string's sound preset
				//
				StringClass sound_preset_name;							
				DefinitionClass *definition = DefinitionMgrClass::Find_Definition (object->Get_Sound_ID (), false);
				if (definition != NULL) {
					sound_preset_name = definition->Get_Name ();
				}

				//
				//	Write a tab delimited entry for this object
				//
				StringClass text_entry;
				text_entry = category_name;
				text_entry += "\t";
				text_entry += object->Get_ID_Desc ();
				text_entry += "\t";
				text_entry += english_string;
				text_entry += "\t";
				text_entry += sound_preset_name;
				file_obj.Write_Line (text_entry);
			}
		}
	}
	
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Export_C_Header
//
///////////////////////////////////////////////////////////////////////
void
TranslateDBClass::Export_C_Header (const char *filename)
{
	//
	//	Create the file
	//
    TextFileClass file_obj;
    file_obj.Open(filename, FileClass::WRITE);
    if (file_obj.Is_Open()) {

		//
		//	Wtite the 'C' style header framework
		//
		file_obj.Write_Line ("#if defined(_MSC_VER)");
		file_obj.Write_Line ("#pragma once");
		file_obj.Write_Line ("#endif");
		file_obj.Write_Line ("");

		file_obj.Write_Line ("#ifndef __STRING_IDS_H");
		file_obj.Write_Line ("#define __STRING_IDS_H");
		file_obj.Write_Line ("");

		StringClass version_line;
		version_line.Format ("#define STRINGS_VER		%d", m_VersionNumber);
		file_obj.Write_Line (version_line);

		file_obj.Write_Line ("");
		file_obj.Write_Line ("// TRANSLATEDB: Begin ID Block");

		//
		//	Loop over all the translation objects and write a #define to the
		// header file for each one...
		//
		for (int index = 0; index < m_ObjectList.Count (); index ++) {
			TDBObjClass *object = m_ObjectList[index];
			if (object != NULL) {
				
				//
				//	Write a #define for this object's ID
				//
				StringClass id_entry;
				id_entry.Format ("#define %s		%d", (const char *)object->Get_ID_Desc (), object->Get_ID ());				
				file_obj.Write_Line (id_entry);
			}
		}

		file_obj.Write_Line ("// TRANSLATEDB: End ID Block");
		file_obj.Write_Line ("");
		file_obj.Write_Line ("#endif //__STRING_IDS_H");
	}
	
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Import_C_Header
//
///////////////////////////////////////////////////////////////////////
void
TranslateDBClass::Import_C_Header (const char *filename)
{
	//
	//	Create the file
	//
    TextFileClass file_obj;
    file_obj.Open(filename, FileClass::READ);
    if (file_obj.Is_Open()) {

		StringClass line;
		bool found_id_block = false;

		//
		//	Look for the start of the ID block
		//
		while (found_id_block == false && file_obj.Read_Line (line)) {
			found_id_block = (line.Compare_No_Case ("// TRANSLATEDB: Begin ID Block") == 0);
		}

		if (found_id_block) {
			
			//
			//	Read each ID define from the header file
			//
			bool found_end_block = false;
			while (found_end_block == false && file_obj.Read_Line (line)) {

				if (::strnicmp (line, "#define ", 8) == 0) {
					
					//
					//	Break the #define into its parts
					//
					int word_breaks[4] = { 0 };
					int curr_break = 0;

					//
					//	Find out where each word begins and ends
					//
			const size_t count = line.Get_Length ();
			for (size_t index = 8; curr_break < 4 && index < count; index ++) { 

				bool is_whitespace = (line[static_cast<int>(index)] == ' ' || line[static_cast<int>(index)] == '\t');
				
				if (is_whitespace == (curr_break & 1) || index == count - 1) {
					word_breaks[curr_break ++] = static_cast<int>(index);
				}
			}

					//
					//	Did we encounter the right number of words?
					//
					if (curr_break == 4) {
						
						//
						//	Isolate the words
						//
						char id_desc_text[64]	= { 0 };
						char id_text[64]			= { 0 };						
						::strncpy (id_desc_text,	line.Peek_Buffer () + word_breaks[0], word_breaks[1] - word_breaks[0]);
						::strncpy (id_text,			line.Peek_Buffer () + word_breaks[2], word_breaks[3] - word_breaks[2]);

						//
						//	Convert the ID string to a number
						//
						uint32 id = ::atoi (id_text);

						//
						//	Do we already have this object?
						//
						TDBObjClass *object = Find_Object (id);
						if (object == NULL) {
							object = Find_Object (id_desc_text);
							if (object == NULL) {

								//
								//	If we didn't already have this object, then create
								// a new object and add it to the system
								//
								object = new TDBObjClass;
								object->Set_ID_Desc (id_desc_text);
								Add_Object (object);
							}
						}
					}

				} else {
					
					//
					//	Check for block-end conditions
					//
					found_end_block = (line.Compare_No_Case ("// TRANSLATEDB: End ID Block") == 0);
				}
			}
        }
	}
	
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Get_Object_Count
//
///////////////////////////////////////////////////////////////////////
int
TranslateDBClass::Get_Object_Count (void)
{
	return m_ObjectList.Count ();
}


///////////////////////////////////////////////////////////////////////
//
//	Get_Object
//
///////////////////////////////////////////////////////////////////////
TDBObjClass *
TranslateDBClass::Get_Object (int index)
{
	TDBObjClass *object = NULL;
	
	WWASSERT (index >= 0 && index < m_ObjectList.Count ());
	if (index >= 0 && index < m_ObjectList.Count ()) {
		object = m_ObjectList[index];
	}

	return object;
}


///////////////////////////////////////////////////////////////////////
//
//	Find_Unique_ID
//
///////////////////////////////////////////////////////////////////////
uint32
TranslateDBClass::Find_Unique_ID (void)
{
	uint32 new_id = ID_MIN + m_ObjectList.Count ();

	//
	//	Return the 'id' of the first empty slot in the list
	//
	for (int index = 0; index < m_ObjectList.Count (); index ++) {
		if (m_ObjectList[index] == NULL) {
			new_id = (ID_MIN + index);
			break;
		}
	}

	return new_id;
}


///////////////////////////////////////////////////////////////////////
//
//	Add_Category
//
///////////////////////////////////////////////////////////////////////
TDBCategoryClass *
TranslateDBClass::Add_Category (const char *name)
{
	//
	//	Create the new category
	//
	TDBCategoryClass *category = new TDBCategoryClass;
	category->Set_Name (name);

	//
	//	Add the category to our list and return a pointer to the caller
	//
	Add_Category (category);
	return category;
}


///////////////////////////////////////////////////////////////////////
//
//	Add_Category
//
///////////////////////////////////////////////////////////////////////
bool
TranslateDBClass::Add_Category (TDBCategoryClass *new_category, bool assign_id)
{
	bool retval = false;

	WWASSERT (new_category != NULL);
	if (new_category != NULL) {

		//
		//	Assign this category an ID (if necessary)
		//
		if (assign_id && new_category->Get_ID () == 0) {
			uint32 new_id = 1;
			for (int index = 0; index < m_CategoryList.Count (); index ++) {
				uint32 curr_id = m_CategoryList[index]->Get_ID ();
				new_id = std::max (curr_id + 1, new_id);
			}
			new_category->Set_ID (new_id);
		}

		//
		//	Add the new category to our list
		//
		m_CategoryList.Add (new_category);
		retval = true;
	}

	return retval;
}


///////////////////////////////////////////////////////////////////////
//
//	Remove_Category
//
///////////////////////////////////////////////////////////////////////
bool
TranslateDBClass::Remove_Category (int index)
{
	bool retval = false;

	//
	//	Make sure this index is in our range
	//
	WWASSERT (index >= 0 && index < m_CategoryList.Count ());
	if (index >= 0 && index < m_CategoryList.Count ()) {
		
		//
		//	Free the object that was in this slot
		//
		TDBCategoryClass *category = m_CategoryList[index];
		if (category != NULL) {

			//
			//	Remove all objects from the category
			//
			int category_id = category->Get_ID ();			
			for (	TDBObjClass *object = Get_First_Object (category_id);
					object != NULL;
					object = Get_Next_Object (category_id, object))
			{
				object->Set_Category_ID (CATEGORY_DEFAULT);
			}

			delete category;
		}

		//
		//	Remove the entry from our list
		//
		m_CategoryList.Delete (index);
		retval = true;
	}

	return retval;
}


///////////////////////////////////////////////////////////////////////
//
//	Add_Object
//
///////////////////////////////////////////////////////////////////////
bool
TranslateDBClass::Add_Object (TDBObjClass *new_obj)
{
	bool retval = false;

	WWASSERT (new_obj != NULL);
	if (new_obj != NULL) {

		//
		//	Try to find an unused ID for the object (if necessary)
		//		
		if (new_obj->Get_ID () < ID_MIN) {
			new_obj->Set_ID (Find_Unique_ID ());
		}

		//
		//	Determine where in the list this object should
		// be inserted
		//
		int new_id		= new_obj->Get_ID ();
		int obj_index	= new_id - ID_MIN;

		//
		//	Grow the list up to the number of elements we need
		//
		while (m_ObjectList.Count () <= obj_index) {
			m_ObjectList.Add (NULL);
		}
		
		//
		//	Add the object to our list
		//
		m_ObjectList[obj_index] = new_obj;

		// Insert object to the hash table as well...
		StringClass lower_case_name(new_obj->Get_ID_Desc(),true);
        lower_case_name.To_Lower();
		m_ObjectHash.Insert(lower_case_name,new_obj);

		retval = true;
	}

	return retval;
}


///////////////////////////////////////////////////////////////////////
//
//	Remove_Object
//
///////////////////////////////////////////////////////////////////////
bool
TranslateDBClass::Remove_Object (int index)
{
	bool retval = false;

	//
	//	Make sure this index is in our range
	//
	WWASSERT (index >= 0 && index < m_ObjectList.Count ());
	if (index >= 0 && index < m_ObjectList.Count ()) {
		
		//
		//	Free the object that was in this slot
		//
		TDBObjClass *object = m_ObjectList[index];
		if (object != NULL) {
			// Remove the object from the hash table
			StringClass lower_case_name(object->Get_ID_Desc(),true);
            lower_case_name.To_Lower();
			m_ObjectHash.Remove(lower_case_name);
			delete object;
		}

		//
		//	Remove the pointer from the list and re-assign IDs
		//
		m_ObjectList[index] = NULL;
		retval = true;
	}

	return retval;
}


//////////////////////////////////////////////////////////////
//
//	Remove_All
//
//////////////////////////////////////////////////////////////
void
TranslateDBClass::Remove_All (void)
{
	Free_Objects ();
	return ;
}


//////////////////////////////////////////////////////////////
//
//	Get_Version_Number
//
//////////////////////////////////////////////////////////////
uint32
TranslateDBClass::Get_Version_Number (void)
{
	return m_VersionNumber;
}


//////////////////////////////////////////////////////////////
//
//	Update_Version
//
//////////////////////////////////////////////////////////////
void
TranslateDBClass::Update_Version (void)
{
	m_VersionNumber ++;
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Get_Category_Count
//
///////////////////////////////////////////////////////////////////////
int
TranslateDBClass::Get_Category_Count (void)
{
	return m_CategoryList.Count ();
}


///////////////////////////////////////////////////////////////////////
//
//	Get_Category
//
///////////////////////////////////////////////////////////////////////
TDBCategoryClass *
TranslateDBClass::Get_Category (int index)
{
	TDBCategoryClass *category = NULL;
	
	WWASSERT (index >= 0 && index < m_CategoryList.Count ());
	if (index >= 0 && index < m_CategoryList.Count ()) {
		category = m_CategoryList[index];
	}

	return category;
}


///////////////////////////////////////////////////////////////////////
//
//	Find_Category
//
///////////////////////////////////////////////////////////////////////
TDBCategoryClass *
TranslateDBClass::Find_Category (uint32 id)
{
	TDBCategoryClass *category = NULL;

	//
	//	Loop over all the categories until we've found a matching ID 
	//
	for (int index = 0; index < m_CategoryList.Count ();	index ++) {
		if (m_CategoryList[index]->Get_ID () == id) {
			category = m_CategoryList[index];
			break;
		}
	}

	return category;
}


///////////////////////////////////////////////////////////////////////
//
//	Find_Category
//
///////////////////////////////////////////////////////////////////////
TDBCategoryClass *
TranslateDBClass::Find_Category (const char *name)
{
	TDBCategoryClass *category = NULL;

	//
	//	Loop over all the categories until we've found a matching name
	//
	for (int index = 0; index < m_CategoryList.Count ();	index ++) {
		if (m_CategoryList[index]->Get_Name ().Compare_No_Case (name) == 0) {
			category = m_CategoryList[index];
			break;
		}
	}

	return category;
}


///////////////////////////////////////////////////////////////////////
//
//	Get_First_Object
//
///////////////////////////////////////////////////////////////////////
TDBObjClass *
TranslateDBClass::Get_First_Object (uint32 category_id)
{
	TDBObjClass *object = NULL;

	//
	//	Loop over the objects we know about and return the first
	// one that matches the given category ID
	//
	for (int index = 0; index < m_ObjectList.Count (); index ++) {
		TDBObjClass *curr_obj = m_ObjectList[index];
		if (curr_obj != NULL) {

			//
			//	Does this object belong to the category we are enumerating?
			//
			if (curr_obj->Get_Category_ID () == category_id) {
				object = curr_obj;
				break;
			}
		}
	}

	return object;
}


///////////////////////////////////////////////////////////////////////
//
//	Get_Next_Object
//
///////////////////////////////////////////////////////////////////////
TDBObjClass *
TranslateDBClass::Get_Next_Object (uint32 category_id, TDBObjClass *curr_obj)
{
	//
	//	Sanity check
	//
	if (curr_obj == NULL) {
		return NULL;
	}

	TDBObjClass *object = NULL;

	//
	//	Determine where in the list to start looking
	//
	int start_index = (curr_obj->Get_ID () - ID_MIN) + 1;

	//
	//	Loop over the objects we know about and return the first
	// one that matches the given category ID
	//	
	for (int index = start_index; index < m_ObjectList.Count (); index ++) {
		TDBObjClass *check_obj = m_ObjectList[index];
		if (check_obj != NULL) {

			//
			//	Does this object belong to the category we are enumerating?
			//
			if (check_obj->Get_Category_ID () == category_id) {
				object = check_obj;
				break;
			}
		}
	}

	return object;
}


/////////////////////////////////////////////////////////////////////////////
//
// Convert_Chars_To_Newline
//
/////////////////////////////////////////////////////////////////////////////
void
Convert_Chars_To_Newline (StringClass &string)
{
	StringClass retval;
	
	const size_t count = string.Get_Length ();

	//
	//	Copy characters between the strings
	//
	for (size_t index = 0; index < count; index ++) {

		if (index + 1 < count && string[static_cast<int>(index)] == '\\' && string[static_cast<int>(index + 1)] == 'n') {
			retval += '\n';
			index ++;
		} else {
			retval += string[static_cast<int>(index)];
		}
	}

	string = retval;
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Import_Strings
//
///////////////////////////////////////////////////////////////////////
void
TranslateDBClass::Import_Strings (const char *filename)
{
	//
	//	Open the file
	//
    TextFileClass file_obj;
    file_obj.Open(filename, FileClass::READ);
    if (file_obj.Is_Open()) {

		//
		//	Keep reading data from the file until we've reached the end
		// of the file or the data becomes corrupt
		//
		bool keep_going = true;
		StringClass line;
		while (keep_going && file_obj.Read_Line (line)) {
			
			//
			//	Convert the string to an array of values
			//
			StringClass *value_array = NULL;
			int value_count = Build_List_From_String (line, "\t", &value_array);

			//
			//	We can't keep going unless we've got at least 3 columns of information
			//
			if (value_count >= 3) {
				
				//
				//	Lookup the category this entry belongs to
				//
				TDBCategoryClass *category = Find_Category (value_array[0]);

				//
				//	Create the entry
				//
				TDBObjClass *new_obj = new TDBObjClass;
				new_obj->Set_Category_ID (category ? category->Get_ID () : 0);
				new_obj->Set_ID_Desc (value_array[1]);
				
				//
				//	Set the english string
				//
				StringClass english_string = value_array[2];
				::Convert_Chars_To_Newline (english_string);
				new_obj->Set_English_String (english_string);

				//
				//	If necessary, lookup the sound definition that goes with this new entry
				// and plug in its ID.
				//
				if (value_count >= 4) {
					DefinitionClass *definition = DefinitionMgrClass::Find_Typed_Definition (value_array[3], CLASSID_SOUND, false);
					if (definition != NULL) {
						new_obj->Set_Sound_ID (definition->Get_ID ());
					}
				}

				//
				//	Add this object to the database
				//
				Add_Object (new_obj);
			}

			//
			// Cleanup
			//
			if (value_array != NULL) {
				delete [] value_array;
				value_array = NULL;
			}
		}
	}

	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Build_List_From_String
//
///////////////////////////////////////////////////////////////////////
int Build_List_From_String
(
	const char *	buffer,
	const char *	delimiter,
	StringClass **	string_list
)
{
	int count = 0;

	WWASSERT (buffer != NULL);
	WWASSERT (delimiter != NULL);
	WWASSERT (string_list != NULL);
	if ((buffer != NULL) &&
		 (delimiter != NULL) &&
		 (string_list != NULL))
	{
		const size_t delim_len = ::strlen (delimiter);

		//
		// Determine how many entries there will be in the list
		//
		const char *entry;
		for (entry = buffer;
			  (entry != NULL) && (entry[1] != 0);
			  entry = ::strstr (entry, delimiter))
		{
			
			//
			// Move past the current delimiter (if necessary)
			//
			if ((::strnicmp (entry, delimiter, delim_len) == 0) && (count > 0)) {
				entry += delim_len;
			}

			// Increment the count of entries
			count ++;
		}
	
		if (count > 0) {

			//
			// Allocate enough StringClass objects to hold all the strings in the list
			//
			(*string_list) = new StringClass[count];
		
			//
			// Parse the string and pull out its entries.
			//
			count = 0;
			for (entry = buffer;
				  (entry != NULL) && (entry[1] != 0);
				  entry = ::strstr (entry, delimiter))
			{
				
				//
				// Move past the current delimiter (if necessary)
				//
				if ((::strnicmp (entry, delimiter, delim_len) == 0) && (count > 0)) {
					entry += delim_len;
				}

				//
				// Copy this entry into its own string
				//
				StringClass entry_string = entry;
				char *delim_start = (char *)::strstr (entry_string, delimiter);				
				if (delim_start != NULL) {
					delim_start[0] = 0;
				}

				//
				// Add this entry to our list
				//
				if ((entry_string.Get_Length () > 0) || (count == 0)) {
					(*string_list)[count++] = entry_string;
				}
			}

		} else if (delim_len > 0) {
			count = 1;
			(*string_list) = new StringClass[count];
			(*string_list)[0] = buffer;
		}
				
	}

	//
	// Return the number of entries in our list
	//
	return count;
}


///////////////////////////////////////////////////////////////////////
//
//	Set_Export_Filter
//
///////////////////////////////////////////////////////////////////////
void
TranslateDBClass::Set_Export_Filter (FILTER_OPT filter, uint32 category_id)
{
	FilterType			= filter;
	FilterCategoryID	= category_id;
	return ;
}
