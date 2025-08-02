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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/EditScript.cpp               $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 11/02/00 4:17p                                              $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "StdAfx.h"
#include "editscript.h"
#include "..\..\scripts\ScriptEvents.H"
#include "scripts.h"
#include "chunkio.h"
#include "scriptmgr.h"


enum
{
	CHUNKID_VARIABLES			= 0x10271120,
	CHUNKID_VALUES
};

enum
{
	VARID_NAME					= 0x01,
	VARID_PARAM_DESC,
	VARID_VALUES
};


////////////////////////////////////////////////////////////////
//
//	EditScriptClass
//
////////////////////////////////////////////////////////////////
EditScriptClass::EditScriptClass (void)
{	
	return ;
}


////////////////////////////////////////////////////////////////
//
//	EditScriptClass
//
////////////////////////////////////////////////////////////////
EditScriptClass::EditScriptClass (const EditScriptClass &src)
{
	*this = src;
	return ;
}


////////////////////////////////////////////////////////////////
//
//	~EditScriptClass
//
////////////////////////////////////////////////////////////////
EditScriptClass::~EditScriptClass (void)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	operator=
//
////////////////////////////////////////////////////////////////
const EditScriptClass &
EditScriptClass::operator= (const EditScriptClass &src)
{
	m_Name					= src.m_Name;
	m_ParamValues			= src.m_ParamValues;
	m_ParamDescription	= src.m_ParamDescription;
	return *this;
}


////////////////////////////////////////////////////////////////
//
//	operator==
//
////////////////////////////////////////////////////////////////
bool
EditScriptClass::operator== (const EditScriptClass &src)
{	
	bool retval = true;

	// Are the params the same?
	retval &= bool(m_ParamValues.Count () == src.m_ParamValues.Count ());
	retval &= bool(m_ParamDescription.Compare_No_Case (src.m_ParamDescription) == 0);
	return retval;
}


////////////////////////////////////////////////////////////////
//
//	Set_Composite_Values
//
////////////////////////////////////////////////////////////////
void
EditScriptClass::Set_Composite_Values (LPCTSTR values)
{
	// Build a list of values from the composite string
	CString *value_list = NULL;
	int count = ::Build_List_From_String (values, ",", &value_list);

	// Pass these values onto our list
	for (int index = 0; (index < m_ParamValues.Count ()) && (index < count); index ++) {
		m_ParamValues[index].value = value_list[index];
	}

	SAFE_DELETE_ARRAY (value_list);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Build_Composite_String
//
////////////////////////////////////////////////////////////////
CString
EditScriptClass::Build_Composite_String (void) const
{
	CString composite_string;

	// Build a composite string out of the values for each param
	int count = m_ParamValues.Count ();
	for (int index = 0; index < count; index ++) {
		const PARAM_VALUE &value = m_ParamValues[index];
		composite_string += value.value;
		
		// Make sure the entries are comma delimited
		if (index != (count - 1)) {
			composite_string += ",";
		}
	}

	// Return a string object containing the values
	return composite_string;
}


////////////////////////////////////////////////////////////////
//
//	Update_Data
//
////////////////////////////////////////////////////////////////
void
EditScriptClass::Update_Data (void)
{
	// Start with a clean list of values
	m_ParamValues.Delete_All ();

	// Build a list of parameters from the description string
	CString *param_list = NULL;
	int count = ::Build_List_From_String (m_ParamDescription, ",", &param_list);

	// Loop through the paramter descriptions
	for (int index = 0; index < count; index ++) {
		CString param_desc = param_list[index];
		
		CString name = param_desc;
		CString def_value;
		CString type;
		
		// Look for a default value and a type specifier
		int def_index = param_desc.Find ('=');
		int type_index = param_desc.Find (':');
		
		// Was there a type specifier?
		if (type_index != -1) {
			name = param_desc.Left (type_index);
			type = &(((LPCTSTR)param_desc)[type_index+1]);
			type.TrimLeft ();
			type.TrimRight ();
			param_desc = name;
		}

		// Was there a default value?
		if (def_index != -1) {
			name = param_desc.Left (def_index);
			def_value = &(((LPCTSTR)param_desc)[def_index+1]);
		}

		//
		// Add this parameter to our list
		//
		PARAM_VALUE value;
		value.name	= name;
		value.value	= def_value;
		value.type	= String_To_Type (type);
		m_ParamValues.Add (value);
	}
	
	SAFE_DELETE_ARRAY (param_list);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	String_To_Type
//
////////////////////////////////////////////////////////////////
PARAM_TYPES
EditScriptClass::String_To_Type (LPCTSTR type_name)
{
	// Look through all the known specifiers and see if any match
	PARAM_TYPES type = PARAM_TYPE_INT;
	bool found = false;
	for (int type_index = 0; (type_index < PARAM_TYPE_COUNT) && (found == false); type_index ++) {
		
		// Is this the type specifier we were looking for?
		if (::lstrcmpi (type_name, PARAM_TYPE_STRINGS[type_index]) == 0) {
			type = (PARAM_TYPES)type_index;
			found = true;
		}
	}

	// Return the type specifier ID
	return type;
}


////////////////////////////////////////////////////////////////
//
//	Create_Script
//
////////////////////////////////////////////////////////////////
ScriptClass *
EditScriptClass::Create_Script (void)
{
	//
	//	Create the new script
	//
	ScriptClass	*script = ScriptManager::Create_Script (m_Name);
	if (script != NULL) {
		
		//
		//	Convert our params to a string and pass them
		// onto the script
		//
		CString params = Build_Composite_String ();
		script->Set_Parameters_String (params);
	}

	return script;
}


/////////////////////////////////////////////////////////////////
//
//	Save
//
/////////////////////////////////////////////////////////////////
bool
EditScriptClass::Save (ChunkSaveClass &csave)
{	
	csave.Begin_Chunk (CHUNKID_VARIABLES);
		
		WRITE_MICRO_CHUNK_WWSTRING (csave, VARID_NAME, m_Name);
		//WRITE_MICRO_CHUNK_WWSTRING (csave, VARID_PARAM_DESC, m_ParamDescription);
		
		//StringClass values = (LPCTSTR)Build_Composite_String ();
		//WRITE_MICRO_CHUNK_WWSTRING (csave, VARID_VALUES, values);

	csave.End_Chunk ();		

	csave.Begin_Chunk (CHUNKID_VALUES);
		StringClass values = (LPCTSTR)Build_Composite_String ();
		csave.Write ((const char *)values, values.Get_Length () + 1);
	csave.End_Chunk ();

	return true;
}


/////////////////////////////////////////////////////////////////
//
//	Load
//
/////////////////////////////////////////////////////////////////
bool
EditScriptClass::Load (ChunkLoadClass &cload)
{
	bool retval = true;

	while (cload.Open_Chunk ()) {		
		switch (cload.Cur_Chunk_ID ()) 
		{
			case CHUNKID_VARIABLES:
				retval &= Load_Variables (cload);
				break;

			case CHUNKID_VALUES:
			{
				//
				//	Read the parameter value string from the chunk
				//
				StringClass values;
				int len = cload.Cur_Chunk_Length ();
				cload.Read (values.Get_Buffer (len), len);

				//
				//	Store the parameter values in our internal data structures
				//
				Set_Composite_Values (values);
			}
			break;
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
EditScriptClass::Load_Variables (ChunkLoadClass &cload)
{
	StringClass values;

	//
	//	Loop through all the microchunks that define the variables
	//
	while (cload.Open_Micro_Chunk ()) {
		switch (cload.Cur_Micro_Chunk_ID ()) {
			
			READ_MICRO_CHUNK_WWSTRING (cload, VARID_NAME,			m_Name);
			READ_MICRO_CHUNK_WWSTRING (cload, VARID_VALUES,			values);
		}

		cload.Close_Micro_Chunk ();
	}

	//
	//	Make sure we use the current version of the parameter list
	//
	Lookup_Param_Description ();	
	Set_Composite_Values (values);
	return true;
}


///////////////////////////////////////////////////////////////////////
//
//	Lookup_Param_Description
//
///////////////////////////////////////////////////////////////////////
void
EditScriptClass::Lookup_Param_Description (void)
{
	//
	//	Try to find the default instance of the script
	//
	EditScriptClass *current_version = ScriptMgrClass::Find_Script (m_Name);
	if (current_version != NULL) {
		m_ParamDescription = current_version->m_ParamDescription;
	} else {
		m_ParamDescription = "";
	}

	Update_Data ();
	return ;
}