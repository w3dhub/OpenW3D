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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/PresetExport.cpp     $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 4/02/01 11:32a                                              $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "StdAfx.h"

#include "presetexport.h"
#include "preset.h"
#include "presetmgr.h"
#include "textfile.h"
#include "Utils.h"
#include "parameter.h"
#include "simpleparameter.h"
#include "presetslibform.h"


///////////////////////////////////////////////////////////////////////
//
//	Import
//
///////////////////////////////////////////////////////////////////////
void
PresetExportClass::Import (const char *filename)
{
	//
	//	Try to open the log file
	//
	TextFileClass file (filename);
	if (file.Open ()) {
		
		//
		//	Lookup the class ID in the header of the text file
		//
		bool is_valid_file = false;
		int class_id = Find_Header (file);
		if (class_id != 0) {

			//
			//	Read the column headers from disk
			//
			DynamicVectorClass<StringClass> column_headers;
			if (	Read_Column_Headers (file, column_headers) &&
					Validate_Columns (class_id, column_headers))
			{
				//
				//	Try to check-out the preset database we will be modifying
				//
				bool should_undo_on_err = false;
				if (PresetMgrClass::Check_Out_Database (class_id, &should_undo_on_err)) {

					//
					//	Turn off immediate check in mode so we can make bulk changes to the
					// preset library
					//
					bool old_check_in_mode = PresetMgrClass::Get_Immediate_Check_In_Mode ();
					PresetMgrClass::Set_Immediate_Check_In_Mode (false);

					//
					//	Import the new settings
					//
					if (Import_Presets (file, class_id, column_headers)) {
						is_valid_file = true;
				
						//
						//	Save the preset changes to disk
						//
						::Get_Presets_Form ()->Save_Global_Presets (class_id);
						if (old_check_in_mode) {
							PresetMgrClass::Check_In_Presets ();
						}
					}

					//
					//	Restore the old check-in mode
					//
					PresetMgrClass::Set_Immediate_Check_In_Mode (old_check_in_mode);
				}
			}			
		}

		//
		//	Warn the user that the file did not appear to be valid
		//	
		if (is_valid_file == false) {			
			CString message;
			message.Format ("%s does not appear to be a valid preset export text file or there was an error parsing the data.\nDue to the severe side-effects of improperly importing preset settings, all changes were ignored.", filename);
			::MessageBox (::AfxGetMainWnd ()->m_hWnd, message, "Invalid Data", MB_ICONERROR | MB_OK);
		}

		//
		//	Close the file
		//
		file.Close ();
	}

	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Read_Column_Headers
//
///////////////////////////////////////////////////////////////////////
bool
PresetExportClass::Read_Column_Headers
(
	TextFileClass &							file,
	DynamicVectorClass<StringClass> &	column_headers
)
{
	bool retval = false;	

	//
	//	Keep reading lines of text from the file until we've
	// found the column headers
	//
	StringClass line;
	while (file.Read_Line (line)) {
		
		//
		//	Check to see if this is the column header
		//
		const char * COL_HEADER_TEXT	= "Name\tPreset ID\t";
		int header_len						= ::lstrlen (COL_HEADER_TEXT);
		if (::strnicmp (line, COL_HEADER_TEXT, header_len) == 0) {
			
			//
			//	Build a list from the column header string
			//
			CString *temp_string_list = NULL;
			int count = ::Build_List_From_String (line, "\t", &temp_string_list);
			if (count > 0) {
				
				//
				//	Add the entries from our temp list to the list the
				// caller supplied.  Also check for NULL entries
				//
				for (int index = 0; index < count; index ++) {
					if (temp_string_list[index].IsEmpty () == false) {
						column_headers.Add ((const char *)temp_string_list[index]);
					}
				}

				SAFE_DELETE_ARRAY (temp_string_list);
				retval = true;
			}
			break;
		}
	}
	
	return retval;
}


///////////////////////////////////////////////////////////////////////
//
//	Validate_Columns
//
///////////////////////////////////////////////////////////////////////
bool
PresetExportClass::Validate_Columns
(
	int											class_id,
	DynamicVectorClass<StringClass> &	column_headers
)
{
	bool retval = false;

	//
	//	Lookup the first preset of the given class
	//
	PresetClass *first_preset = PresetMgrClass::Get_First (class_id, PresetMgrClass::ID_CLASS);
	if (first_preset != NULL) {

		//
		//	Assume success from here on out
		//
		retval = true;
		
		//
		//	Try to match each column header with a parameter of the class
		//
		for (int col_index = 2; col_index < column_headers.Count (); col_index ++) {
			
			//
			//	Loop over all the parameters for this definition type
			//
			DefinitionClass *definition	= first_preset->Get_Definition ();
			int count							= definition->Get_Parameter_Count ();
			bool found							= false;
			for (int index = 0; !found && index < count; index ++) {
				ParameterClass *parameter = definition->Lock_Parameter (index);

				//
				//	Is this the parameter we are looking for?
				//
				if (::lstrcmpi (column_headers[col_index], parameter->Get_Name ()) == 0) {
					
					//
					//	Is this a parameter type that we are likely to have exported?
					//
					if (Can_Export_Parameter (parameter)) {
						found = true;
					}
				}

				definition->Unlock_Parameter (index);
			}

			//
			//	Kick out of the loop if we can't match even one of the presets
			//
			if (found == false) {
				retval = false;
				break;
			}
		}
	}	

	return retval;
}


///////////////////////////////////////////////////////////////////////
//
//	Import_Presets
//
///////////////////////////////////////////////////////////////////////
bool
PresetExportClass::Import_Presets
(
	TextFileClass &							file,
	int											class_id,
	DynamicVectorClass<StringClass> &	column_headers
)
{
	bool retval	= true;

	//
	//	Keep reading preset values until we've exhausted the file's contents
	//
	StringClass line;
	while (retval && file.Read_Line (line)) {

		//
		//	Build a list of values from the string
		//
		CString *value_list = NULL;
		int count = ::Build_List_From_String (line, "\t", &value_list);

		//
		//	Exit early if the count of values does not match the count
		// of expected values.
		//
		if (count != column_headers.Count ()) {
			retval = false;
		} else {

			//
			//	Get the ID of the preset
			//
			int preset_id = ::atoi (value_list[1]);
			PresetClass *preset = NULL;
			
			if (preset_id == 0) {

				//
				//	Create a new preset...
				//
				preset = PresetMgrClass::Create_Preset (class_id, value_list[0], false);
				if (preset != NULL) {

					//
					//	Add this preset to the framework
					//
					::Get_Presets_Form ()->Add_Preset (preset);
					PresetMgrClass::Add_Preset (preset);
					DefinitionMgrClass::Register_Definition (preset->Get_Definition ());
				}

			} else {

				//
				//	Lookup the preset based on its ID
				//
				preset = PresetMgrClass::Find_Preset (preset_id);
			}

			//
			//	Did we find the preset and does its name match?
			//
			if (preset != NULL && ::lstrcmpi (preset->Get_Name (), value_list[0]) == 0) {

				//
				//	Import the remainder of the parameter values
				//
				for (int index = 2; retval && index < count; index ++) {
					ParameterClass *parameter = Find_Parameter (preset, column_headers[index]);
					if (parameter != NULL) {

						//
						//	Store the change in the preset
						//
						retval = Import_Setting (preset, parameter, value_list[index]);
					} else {
						retval = false;
					}
				}

				//
				//	Register this preset for updating...
				//
				if (retval) {
					PresetMgrClass::Add_Dirty_Preset (preset_id);
				}
			}
		}
		
		SAFE_DELETE_ARRAY (value_list);
	}

	//
	//	Force the imported presets to be sorted
	//
	::Get_Presets_Form ()->Sort_Items (class_id);

	return retval;
}


///////////////////////////////////////////////////////////////////////
//
//	Import_Setting
//
///////////////////////////////////////////////////////////////////////
bool
PresetExportClass::Import_Setting
(
	PresetClass *			/* preset */,
	ParameterClass *		parameter,
	const char *			value
)
{
	bool retval = true;

	//
	//	Handle each type differently
	//
	switch (parameter->Get_Type ())
	{
		case ParameterClass::TYPE_INT:
		{
			//
			//	Convert the string to its native type
			//
			int param_value = 0;
			::sscanf (value, "%d", &param_value);
			
			//
			//	Pass the new value onto the parameter
			//
			((IntParameterClass *)parameter)->Set_Value (param_value);
		}
		break;

		case ParameterClass::TYPE_FLOAT:
		{
			//
			//	Convert the string to its native type
			//
			float param_value = 0;
			::sscanf (value, "%f", &param_value);
			
			//
			//	Pass the new value onto the parameter
			//
			((FloatParameterClass *)parameter)->Set_Value (param_value);
		}
		break;

		case ParameterClass::TYPE_FILENAME:
		case ParameterClass::TYPE_SOUND_FILENAME:
		case ParameterClass::TYPE_STRING:
		{
			//
			//	Pass the new value onto the parameter
			//
			((StringParameterClass *)parameter)->Set_String (value);
		}
		break;

		case ParameterClass::TYPE_VECTOR3:
		{
			//
			//	Convert the string to its native type
			//
			Vector3 param_value (0, 0, 0);
			::sscanf (value, "\"%f, %f, %f\"", &param_value.X, &param_value.Y, &param_value.Z);
			
			//
			//	Pass the new value onto the parameter
			//
			((Vector3ParameterClass *)parameter)->Set_Value (param_value);
		}
		break;

		case ParameterClass::TYPE_BOOL:
		{
			//
			//	Convert the string to its native type
			//
			int param_value = 0;
			::sscanf (value, "%d", &param_value);
			
			//
			//	Pass the new value onto the parameter
			//
			((BoolParameterClass *)parameter)->Set_Value (bool(param_value == 1));
		}
		break;

		case ParameterClass::TYPE_ENUM:
		{
			//
			//	Convert the string to its native type
			//
			int param_value = 0;
			::sscanf (value, "%d", &param_value);
			
			//
			//	Pass the new value onto the parameter
			//
			((EnumParameterClass *)parameter)->Set_Selected_Value (param_value);
		}
		break;

		case ParameterClass::TYPE_ANGLE:
		{
			//
			//	Convert the string to its native type
			//
			float param_value = 0;
			::sscanf (value, "%f", &param_value);
			
			//
			//	Pass the new value onto the parameter
			//
			((AngleParameterClass *)parameter)->Set_Value (param_value);
		}
		break;

		default:
			retval = false;
			break;
	}
	
	return retval;
}


///////////////////////////////////////////////////////////////////////
//
//	Find_Parameter
//
///////////////////////////////////////////////////////////////////////
ParameterClass *
PresetExportClass::Find_Parameter (PresetClass *preset, const char *parameter_name)
{
	ParameterClass *parameter = NULL;

	DefinitionClass *definition	= preset->Get_Definition ();
	int count							= definition->Get_Parameter_Count ();
	bool found							= false;
	for (int index = 0; !found && index < count; index ++) {
		ParameterClass *curr_parameter = definition->Lock_Parameter (index);

		//
		//	Is this the parameter we are looking for?
		//
		if (	::lstrcmpi (parameter_name, curr_parameter->Get_Name ()) == 0 &&
				Can_Export_Parameter (curr_parameter))
		{
			parameter = curr_parameter;
			found = true;
		}
	}
	
	return parameter;
}


///////////////////////////////////////////////////////////////////////
//
//	Find_Header
//
///////////////////////////////////////////////////////////////////////
int
PresetExportClass::Find_Header (TextFileClass &file)
{
	int class_id = 0;
	StringClass line;

	//
	//	Keep reading lines of text from the file until we've
	// found the header
	//
	while (file.Read_Line (line)) {
		
		//
		//	Check to see if this is the header
		//
		const char * HEADER_TEXT	= "Level Edit Preset Export.  Class ID: ";
		int header_len					= ::lstrlen (HEADER_TEXT);
		if (::strnicmp (line, HEADER_TEXT, header_len) == 0) {
			
			//
			//	Convert the remaining characters to a class ID
			//
			class_id = ::atoi (((const char *)line) + header_len);
			break;
		}
	}

	return class_id;
}


///////////////////////////////////////////////////////////////////////
//
//	Export
//
///////////////////////////////////////////////////////////////////////
void
PresetExportClass::Export (uint32 class_id, const char *filename)
{
	//
	//	Try to open the log file
	//
	TextFileClass file (filename);
	if (file.Open (RawFileClass::WRITE)) {

		//
		//	Write a header to the file
		//
		StringClass header;
		header.Format ("Level Edit Preset Export.  Class ID: %d", class_id);
		file.Write_Line (header);
		file.Write_Line ("");
		file.Write_Line ("");

		//
		//	Write a tab-delimited list of the parameter names we are going to
		// export
		//
		Write_Column_Headers (class_id, file);

		//
		//	Export each preset
		//
		PresetClass *preset = NULL;
		for (	preset = PresetMgrClass::Get_First (class_id, PresetMgrClass::ID_CLASS);
				preset != NULL;
				preset = PresetMgrClass::Get_Next (preset, class_id, PresetMgrClass::ID_CLASS))
		{
			Export_Preset (file, preset);
		}

		//
		//	Close the file
		//
		file.Close ();
	}

	
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Can_Export_Parameter
//
///////////////////////////////////////////////////////////////////////
bool
PresetExportClass::Can_Export_Parameter (ParameterClass *parameter)
{
	bool retval = false;

	//
	//	We can only export simple parameter types
	//
	if (	parameter->Get_Type () == ParameterClass::TYPE_INT ||
			parameter->Get_Type () == ParameterClass::TYPE_FLOAT ||
			parameter->Get_Type () == ParameterClass::TYPE_STRING ||
			parameter->Get_Type () == ParameterClass::TYPE_VECTOR3 ||
			parameter->Get_Type () == ParameterClass::TYPE_BOOL ||
			parameter->Get_Type () == ParameterClass::TYPE_FILENAME ||
			parameter->Get_Type () == ParameterClass::TYPE_ENUM ||
			parameter->Get_Type () == ParameterClass::TYPE_SOUND_FILENAME ||
			parameter->Get_Type () == ParameterClass::TYPE_ANGLE )
	{
		retval = true;
	}

	return retval;
}


///////////////////////////////////////////////////////////////////////
//
//	Write_Column_Headers
//
///////////////////////////////////////////////////////////////////////
void
PresetExportClass::Write_Column_Headers (uint32 class_id, TextFileClass &file)
{
	//
	//	Write the column headers
	//
	PresetClass *first_preset = PresetMgrClass::Get_First (class_id, PresetMgrClass::ID_CLASS);
	if (first_preset != NULL) {
		StringClass column_headers = "Name\tPreset ID\t";
		
		//
		//	Loop over all the parameters for this definition type
		//
		DefinitionClass *definition = first_preset->Get_Definition ();
		int count = definition->Get_Parameter_Count ();
		for (int index = 0; index < count; index ++) {
			ParameterClass *parameter = definition->Lock_Parameter (index);

			//
			//	Check to ensure we can export this type of parameter
			//
			if (Can_Export_Parameter (parameter)) {
				column_headers += parameter->Get_Name ();
				column_headers += "\t";
			}

			definition->Unlock_Parameter (index);
		}

		//
		//	Write the column headers to the file
		//
		file.Write_Line (column_headers);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Export_Preset
//
///////////////////////////////////////////////////////////////////////
void
PresetExportClass::Export_Preset (TextFileClass &file, PresetClass *preset)
{
	//
	//	Start the entry by filling in the preset's name and ID
	//
	StringClass settings = preset->Get_Name ();
	settings += "\t";

	StringClass id_string;
	id_string.Format ("%d", preset->Get_ID ());
	settings += id_string;
	settings += "\t";

	//
	//	Loop over all the parameters for this definition type
	//
	DefinitionClass *definition = preset->Get_Definition ();
	int count = definition->Get_Parameter_Count ();
	for (int index = 0; index < count; index ++) {
		ParameterClass *parameter = definition->Lock_Parameter (index);

		//
		//	Handle each type differently
		//
		switch (parameter->Get_Type ())
		{
			case ParameterClass::TYPE_INT:
			{
				//
				//	Format the parameter value into a string
				//
				int param_value = ((IntParameterClass *)parameter)->Get_Value ();
				StringClass string_value;
				string_value.Format ("%d", param_value);

				//
				//	Add this string to the end of the entry
				//
				settings += string_value;
				settings += "\t";
			}
			break;

			case ParameterClass::TYPE_FLOAT:
			{
				//
				//	Format the parameter value into a string
				//
				float param_value = ((FloatParameterClass *)parameter)->Get_Value ();
				StringClass string_value;
				string_value.Format ("%f", param_value);

				//
				//	Add this string to the end of the entry
				//
				settings += string_value;
				settings += "\t";
			}
			break;

			case ParameterClass::TYPE_FILENAME:
			case ParameterClass::TYPE_SOUND_FILENAME:
			case ParameterClass::TYPE_STRING:
			{
				//
				//	Format the parameter value into a string
				//
				const char *param_value = ((StringParameterClass *)parameter)->Get_String ();

				//
				//	Add this string to the end of the entry
				//
				settings += param_value;
				settings += "\t";
			}
			break;

			case ParameterClass::TYPE_VECTOR3:
			{
				//
				//	Format the parameter value into a string
				//
				const Vector3 &param_value = ((Vector3ParameterClass *)parameter)->Get_Value ();
				StringClass string_value;
				string_value.Format ("\"%f, %f, %f\"", param_value.X, param_value.Y, param_value.Z);

				//
				//	Add this string to the end of the entry
				//
				settings += string_value;
				settings += "\t";
			}
			break;

			case ParameterClass::TYPE_BOOL:
			{
				//
				//	Format the parameter value into a string
				//
				int param_value = ((BoolParameterClass *)parameter)->Get_Value ();
				StringClass string_value;
				string_value.Format ("%d", param_value);

				//
				//	Add this string to the end of the entry
				//
				settings += string_value;
				settings += "\t";
			}
			break;

			case ParameterClass::TYPE_ENUM:
			{
				//
				//	Format the parameter value into a string
				//
				int param_value = ((EnumParameterClass *)parameter)->Get_Selected_Value ();
				StringClass string_value;
				string_value.Format ("%d", param_value);

				//
				//	Add this string to the end of the entry
				//
				settings += string_value;
				settings += "\t";
			}
			break;

			case ParameterClass::TYPE_ANGLE:
			{
				//
				//	Format the parameter value into a string
				//
				float param_value = ((AngleParameterClass *)parameter)->Get_Value ();
				StringClass string_value;
				string_value.Format ("%f", param_value);

				//
				//	Add this string to the end of the entry
				//
				settings += string_value;
				settings += "\t";
			}
			break;
		}

		definition->Unlock_Parameter (index);
	}

	//
	//	Write the settings to the file
	//
	file.Write_Line (settings);

	return ;
}
