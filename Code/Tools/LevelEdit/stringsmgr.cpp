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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/stringsmgr.cpp               $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 2/12/02 6:16p                                               $*
 *                                                                                             *
 *                    $Revision:: 14                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "StdAfx.h"
#include "stringsmgr.h"
#include "FileLocations.h"
#include "filemgr.h"
#include "assetdatabase.h"
#include "chunkio.h"
#include "rawfile.h"
#include "saveload.h"
#include "translatedb.h"
#include "stringlibrarydialog.h"
#include "excel.h"
#include "AudibleSound.h"
#include "definitionmgr.h"
#include "definitionclassids.h"
#include "Utils.h"


/////////////////////////////////////////////////////////////////////////
//	Local constants
/////////////////////////////////////////////////////////////////////////
enum
{
	COL_CATEGORY_NAME		= 0,
	COL_STRING_ID,
	COL_SOUND_FILENAME,
	COL_ENGLISH_TEXT,
	COL_TRANSLATED_TEXT,
	COL_COMMENTS,
	COL_ERRORS,
	COL_SOUND_PRESET_NAME,
};


/////////////////////////////////////////////////////////////////////////
//
//	Create_Database_If_Necessary
//
/////////////////////////////////////////////////////////////////////////
void
StringsMgrClass::Create_Database_If_Necessary (void)
{
	FileMgrClass *file_mgr			= ::Get_File_Mgr ();
	AssetDatabaseClass &asset_db	= file_mgr->Get_Database_Interface ();
	
	//
	//	Determine where the file should exist locally
	//
	CString filename = ::Get_File_Mgr ()->Make_Full_Path (STRINGS_DB_PATH);

	//
	//	Check to see if the file exists in VSS
	//
	if (asset_db.Does_File_Exist (filename) == false) {
		
		//
		//	Save a copy of the database to disk and add it to VSS
		//
		Save_Translation_Database ();
		asset_db.Add_File (filename);
	} else {
		
		//
		//	The file exists in VSS, so update our local copy
		//
		Get_Latest_Version ();
	}
	
	return ;
}


/////////////////////////////////////////////////////////////////////////
//
//	Save_Translation_Database
//
/////////////////////////////////////////////////////////////////////////
void
StringsMgrClass::Save_Translation_Database (void)
{
	CString filename = ::Get_File_Mgr ()->Make_Full_Path (STRINGS_DB_PATH);
	Save_Translation_Database (filename);
	return ;
}


/////////////////////////////////////////////////////////////////////////
//
//	Save_Translation_Database
//
/////////////////////////////////////////////////////////////////////////
void
StringsMgrClass::Save_Translation_Database (const char *full_path)
{
	//
	//	Create the file
	//
	HANDLE file = ::CreateFile (full_path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
							0L, NULL);

	ASSERT (file != INVALID_HANDLE_VALUE);
	if (file != INVALID_HANDLE_VALUE) {

		RawFileClass file_obj;
		file_obj.Attach (file);
		ChunkSaveClass chunk_save (&file_obj);

		//
		//	Save the translation database subsystem
		//		
		SaveLoadSystemClass::Save (chunk_save, _TheTranslateDB);
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////
//
//	Load_Translation_Database
//
/////////////////////////////////////////////////////////////////////////
void
StringsMgrClass::Load_Translation_Database (void)
{
	CString filename = ::Get_File_Mgr ()->Make_Full_Path (STRINGS_DB_PATH);

	//
	//	Open the file
	//
	HANDLE file = ::CreateFile (filename, GENERIC_READ, FILE_SHARE_READ, NULL,
							OPEN_EXISTING, 0L, NULL);

	ASSERT (file != INVALID_HANDLE_VALUE);
	if (file != INVALID_HANDLE_VALUE) {

		RawFileClass file_obj;
		file_obj.Attach (file);
		ChunkLoadClass chunk_load (&file_obj);

		//
		//	Let the save/load system handle the laod
		//
		SaveLoadSystemClass::Load (chunk_load);
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////
//
//	Import_Strings
//
/////////////////////////////////////////////////////////////////////////
void
StringsMgrClass::Import_Strings (void)
{	
	CFileDialog dialog (true, ".txt", "strings.txt",
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER,
		"Text Files (*.txt)|*.txt||", ::AfxGetMainWnd ());

	dialog.m_ofn.lpstrTitle = "Import Strings";

	//
	// Ask the user what file they want to load
	//
	if (dialog.DoModal () == IDOK) {		
		if (Check_Out ()) {

			//
			//	Import the strings and save the database back to disk
			//
			TranslateDBClass::Import_Strings (dialog.GetPathName ());
			Save_Translation_Database ();
			Check_In ();			
		}
	}	

	return ;
}


/////////////////////////////////////////////////////////////////////////
//
//	Import_IDs
//
/////////////////////////////////////////////////////////////////////////
void
StringsMgrClass::Import_IDs (void)
{	
	CFileDialog dialog (true, ".h", "string_ids.h",
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER,
		"C Header Files (*.h)|*.h||", ::AfxGetMainWnd ());

	//
	// Ask the user what file they want to load
	//
	if (dialog.DoModal () == IDOK) {		
		if (Check_Out ()) {

			//
			//	Import the header and save the database back to disk
			//
			TranslateDBClass::Import_C_Header (dialog.GetPathName ());
			Save_Translation_Database ();
			Check_In ();			
		}
	}	

	return ;
}


/////////////////////////////////////////////////////////////////////////
//
//	Export_IDs
//
/////////////////////////////////////////////////////////////////////////
void
StringsMgrClass::Export_IDs (void)
{	
	CFileDialog dialog (false, ".h", "string_ids.h",
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER,
		"C Header Files (*.h)|*.h||", ::AfxGetMainWnd ());

	//
	// Ask the user what file they want to create
	//
	if (dialog.DoModal () == IDOK) {

		//
		//	Check to make sure the destination filename is not read-only
		//
		CString path				= dialog.GetPathName ();
		DWORD file_attributes	= ::GetFileAttributes (path);
		if (file_attributes != 0xFFFFFFFF && file_attributes & FILE_ATTRIBUTE_READONLY) {
			::MessageBox (::AfxGetMainWnd ()->m_hWnd, "File is read-only, export operation can not complete.", "File Error", MB_ICONERROR | MB_ICONEXCLAMATION);
		} else {
			TranslateDBClass::Export_C_Header (path);
		}
	}	

	return ;
}


/////////////////////////////////////////////////////////////////////////
//
//	Get_Latest_Version
//
/////////////////////////////////////////////////////////////////////////
bool
StringsMgrClass::Get_Latest_Version (void)
{
	FileMgrClass *file_mgr			= ::Get_File_Mgr ();
	AssetDatabaseClass &asset_db	= file_mgr->Get_Database_Interface ();
	
	//
	//	Determine where the file should exist locally
	//
	CString filename = ::Get_File_Mgr ()->Make_Full_Path (STRINGS_DB_PATH);
	
	//
	//	Ask VSS to get the latest version of the file for us
	//
	return asset_db.Get (filename);
}


/////////////////////////////////////////////////////////////////////////
//
//	Check_Out
//
/////////////////////////////////////////////////////////////////////////
bool
StringsMgrClass::Check_Out (void)
{
	FileMgrClass *file_mgr			= ::Get_File_Mgr ();
	AssetDatabaseClass &asset_db	= file_mgr->Get_Database_Interface ();
	
	//
	//	Determine where the file should exist locally
	//
	CString filename = ::Get_File_Mgr ()->Make_Full_Path (STRINGS_DB_PATH);
	
	//
	//	Ask VSS to check out the file to us
	//
	return asset_db.Check_Out_Ex (filename, ::AfxGetMainWnd ()->m_hWnd);
}


/////////////////////////////////////////////////////////////////////////
//
//	Check_In
//
/////////////////////////////////////////////////////////////////////////
bool
StringsMgrClass::Check_In (void)
{
	FileMgrClass *file_mgr			= ::Get_File_Mgr ();
	AssetDatabaseClass &asset_db	= file_mgr->Get_Database_Interface ();
	
	//
	//	Determine where the file should exist locally
	//
	CString filename = ::Get_File_Mgr ()->Make_Full_Path (STRINGS_DB_PATH);
	
	//
	//	Ask VSS to check in the file for us
	//
	return asset_db.Check_In_Ex (filename, ::AfxGetMainWnd ()->m_hWnd);
}


/////////////////////////////////////////////////////////////////////////
//
//	Undo_Check_Out
//
/////////////////////////////////////////////////////////////////////////
bool
StringsMgrClass::Undo_Check_Out (void)
{
	FileMgrClass *file_mgr			= ::Get_File_Mgr ();
	AssetDatabaseClass &asset_db	= file_mgr->Get_Database_Interface ();
	bool retval							= false;

	//
	//	Determine where the file should exist locally
	//
	CString filename = ::Get_File_Mgr ()->Make_Full_Path (STRINGS_DB_PATH);

	//
	//	We only undo the checkout if its checked out to us
	//
	if (asset_db.Get_File_Status (filename) == AssetDatabaseClass::CHECKED_OUT_TO_ME) {
		retval = asset_db.Undo_Check_Out (filename);
	}

	return retval;
}


/////////////////////////////////////////////////////////////////////////
//
//	Edit_Database
//
/////////////////////////////////////////////////////////////////////////
void
StringsMgrClass::Edit_Database (HWND parent_wnd)
{
	CWaitCursor wait_cursor;

	//
	//	Check out the strings database from VSS
	//
	if (StringsMgrClass::Check_Out ()) {

		//
		//	Reload the database
		//
		StringsMgrClass::Load_Translation_Database ();
		
		//
		//	Show a dialog to the user so then can edit the strings
		//
		StringLibraryDialogClass dialog (CWnd::FromHandle (parent_wnd));
		if (dialog.DoModal () == IDOK) {
			StringsMgrClass::Save_Translation_Database ();
			StringsMgrClass::Check_In ();
		} else {
			StringsMgrClass::Undo_Check_Out ();
		}
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////
//
//	Export_For_Translation
//
/////////////////////////////////////////////////////////////////////////
void
StringsMgrClass::Export_For_Translation (const char *filename, uint32 lang_id)
{
	CWaitCursor wait_cursor;

	if (ExcelClass::Initialize () == false) {
		::MessageBox (::AfxGetMainWnd ()->m_hWnd, "Cannot Initialize Excel, this feature requires Excel installed on the machine.", "Export Error", MB_ICONERROR | MB_OK);
		return ;
	}

	//
	//	Overwrite any existing files
	//
	if (GetFileAttributes (filename) != 0xFFFFFFFF) {
		::DeleteFile (filename);
	}

	//
	//	Lookup the path of the executable
	//
	char path[MAX_PATH] = { 0 };
	::GetModuleFileName (NULL, path, sizeof (path));

	//
	//	Strip off the filename
	//
	char *filename_portion = ::strrchr (path, '\\');
	if (filename_portion != NULL) {
		filename_portion[0] = 0;
	}

	//
	//	Create the new excel workbook based on this template
	//
	StringClass template_path = static_cast<const char *>(Make_Path (path, "renegade.xlt"));
	ExcelClass::New_Workbook (template_path);

	//
	//	Loop over all the strings in the database
	//
	int count = TranslateDBClass::Get_Object_Count ();
	for (int index = 0; index < count; index ++) {
		TDBObjClass *object = TranslateDBClass::Get_Object (index);
		if (object != NULL && object->As_StringTwiddlerClass () == NULL) {
			
			//
			//	Get the data for this string that we want to export
			//
			const StringClass &string			= object->Get_English_String ();
			const StringClass &string_id		= object->Get_ID_Desc ();
			int sound_preset_id					= object->Get_Sound_ID ();
			int category_id						= object->Get_Category_ID ();

			const WideStringClass &foreign_string	= object->Get_String (lang_id);

			//
			//	Dig out the filename from the sound object
			//
			WideStringClass wide_sound_filename;
			WideStringClass wide_sound_preset_name;
			AudibleSoundDefinitionClass *sound_def = (AudibleSoundDefinitionClass *)DefinitionMgrClass::Find_Definition (sound_preset_id, false);
			if (sound_def != NULL) {
				wide_sound_preset_name.Convert_From (sound_def->Get_Name ());
				CString ascii_filename = ::Get_Filename_From_Path (sound_def->Get_Filename ());
				wide_sound_filename.Convert_From ((const char *)ascii_filename);
			}

			//
			//	Lookup the category name
			//
			WideStringClass category_name;
			TDBCategoryClass *category	= TranslateDBClass::Find_Category (category_id);
			if (category != NULL) {				
				category_name.Convert_From (category->Get_Name ());
			}

			//
			//	Convert the data to wide character format
			//
			WideStringClass wide_string;
			WideStringClass wide_string_id;
			wide_string.Convert_From (string);
			wide_string_id.Convert_From (string_id);

			//
			//	Convert any newline characters to string literals
			//
			Convert_Newline_To_Chars (wide_string);

			//
			//	Put this data into the cells
			//
			ExcelClass::Set_String (index + 1, COL_CATEGORY_NAME,		category_name);
			ExcelClass::Set_String (index + 1, COL_STRING_ID,			wide_string_id);
			ExcelClass::Set_String (index + 1, COL_SOUND_FILENAME,	wide_sound_filename);			
			ExcelClass::Set_String (index + 1, COL_ENGLISH_TEXT,		wide_string);
			ExcelClass::Set_String (index + 1, COL_SOUND_PRESET_NAME,wide_sound_preset_name);

			if (lang_id != TranslateDBClass::LANGID_ENGLISH && object->Contains_Translation (lang_id)) {
				ExcelClass::Set_String (index + 1, COL_TRANSLATED_TEXT, foreign_string);
			}
		}
	}

	//
	//	Save our changes
	//
	ExcelClass::Save_Workbook (filename);

	ExcelClass::Shutdown ();
	return ;
}


/////////////////////////////////////////////////////////////////////////
//
//	Import_From_Translation
//
/////////////////////////////////////////////////////////////////////////
void
StringsMgrClass::Import_From_Translation (const char *filename, uint32 lang_id)
{
	CWaitCursor wait_cursor;

	if (ExcelClass::Initialize () == false) {
		::MessageBox (::AfxGetMainWnd ()->m_hWnd, "Cannot Initialize Excel, this feature requires Excel installed on the machine.", "Export Error", MB_ICONERROR | MB_OK);
		return ;
	}

	if (Check_Out () == false) {
		return ;
	}

	//
	//	Open the workbook
	//
	ExcelClass::Open_Workbook (filename);

	//
	//	Keep reading until we don't have any more data to read
	//
	bool keep_going = true;
	int index;
	for (index = 0; keep_going; index ++) {

		//
		//	Read these fields of data
		//
		WideStringClass category_name;
		WideStringClass string_id;
		WideStringClass english_string;
		WideStringClass string;
		WideStringClass preset_name;
		ExcelClass::Get_String (index + 1, COL_CATEGORY_NAME,		category_name);
		ExcelClass::Get_String (index + 1, COL_STRING_ID,			string_id);		
		ExcelClass::Get_String (index + 1, COL_ENGLISH_TEXT,		english_string);
		ExcelClass::Get_String (index + 1, COL_TRANSLATED_TEXT,	string);
		ExcelClass::Get_String (index + 1, COL_SOUND_PRESET_NAME,preset_name);

		//
		//	Convert any typed newline literals "\n" to characters '\n'
		//
		Convert_Chars_To_Newline (english_string);
		Convert_Chars_To_Newline (string);
		Apply_Characteristics (english_string, string);

		//
		//	Did we find the data?
		//
		if (string_id.Is_Empty ()) {
			keep_going = false;
		} else {
			
			StringClass ascii_string_id;
			string_id.Convert_To (ascii_string_id);

			//
			//	Find or add this object to our database
			//
			TDBObjClass *object = TranslateDBClass::Find_Object (ascii_string_id);
			if ((object == NULL) && (lang_id == TranslateDBClass::LANGID_ENGLISH)) {
				object = new TDBObjClass;
				object->Set_ID_Desc (ascii_string_id);
				TranslateDBClass::Add_Object (object);
			}

			if (object != NULL) {

				//
				//	Set the string for this language
				//
				if (lang_id != TranslateDBClass::LANGID_ENGLISH) {
					object->Set_String (lang_id, string);
				} else {
					object->Set_String (TranslateDBClass::LANGID_ENGLISH, english_string);
				}

				//
				//	Find or add the category
				//
				StringClass ascii_category_name;
				category_name.Convert_To (ascii_category_name);
				TDBCategoryClass *category = TranslateDBClass::Find_Category (ascii_category_name);
				if (category == NULL && ascii_category_name.Get_Length () > 0) {
					category = new TDBCategoryClass;
					category->Set_Name (ascii_category_name);
					TranslateDBClass::Add_Category (category, true);
				}

				//
				//	Set the category
				//
				if (category != NULL) {
					object->Set_Category_ID (category->Get_ID ());
				}

				//
				//	Find the sound preset
				//
				StringClass ascii_preset_name;
				preset_name.Convert_To (ascii_preset_name);
				DefinitionClass *definition = DefinitionMgrClass::Find_Typed_Definition (ascii_preset_name, CLASSID_SOUND, false);
				if (definition != NULL) {
					object->Set_Sound_ID (definition->Get_ID ());
				}
			}
		}
	}

	//
	//	Let the user know how many strings were imported
	//
	CString message;
	message.Format ("Successfully imported %d strings.", index);
	::MessageBox (::AfxGetMainWnd ()->m_hWnd, message, "String Import", MB_ICONEXCLAMATION | MB_OK);

	//
	//	Close Excel and save the changes to the database
	//
	ExcelClass::Shutdown ();
	Save_Translation_Database ();
	Check_In ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Convert_Newline_To_Chars
//
/////////////////////////////////////////////////////////////////////////////
void
StringsMgrClass::Convert_Newline_To_Chars (WideStringClass &string)
{
	WideStringClass retval;

	//
	//	Take a guess as to how large to make the final string
	//
	int count = string.Get_Length ();

	//
	//	Copy characters between the strings
	//
	for (int index = 0; index < count; index ++) {
		
		if (string[index] == U_CHAR('\n')) {
			retval += U_CHAR("\\n");
		} else if (string[index] == U_CHAR('\t')) {
			retval += U_CHAR("\\t");
		} else {
			retval += string[index];
		}
	}

	string = retval;
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Convert_Chars_To_Newline
//
/////////////////////////////////////////////////////////////////////////////
void
StringsMgrClass::Convert_Chars_To_Newline (WideStringClass &string)
{
	WideStringClass retval;
	
	//
	//	Take a guess as to how large to make the final string
	//
	int count = string.Get_Length ();

	//
	//	Copy characters between the strings
	//
	for (int index = 0; index < count; index ++) {
		
		if (index + 1 < count && string[index] == U_CHAR('\\') && string[index + 1] == U_CHAR('n')) {
			retval += U_CHAR('\n');
			index ++;
		} else if (index + 1 < count && string[index] == U_CHAR('\\') && string[index + 1] == U_CHAR('t')) {
			retval += U_CHAR('\t');
			index ++;
		} else {
			retval += string[index];
		}
	}

	string = retval;
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Apply_Characteristics
//
/////////////////////////////////////////////////////////////////////////////
void
StringsMgrClass::Apply_Characteristics
(
	WideStringClass &english_string,
	WideStringClass &translated_string
)
{
	int english_len	= english_string.Get_Length ();
	int trans_len		= translated_string.Get_Length ();
	if (english_len == 0 || trans_len == 0) {
		return ;
	}

	/*if (english_len > 2 && trans_len > 2) {

		//
		//	Check to see if the english string is commented out
		///
		const wchar_t *buffer = english_string;			
		if (buffer[0] == U_CHAR('/') && buffer[1] == U_CHAR('/')) {
			
			//
			//	Do we need to comment out the translated string as well?
			//
			const wchar_t *trans_buffer = translated_string;
			if (trans_buffer[0] != U_CHAR('/') || trans_buffer[1] != U_CHAR('/')) {
				
				//
				//	Prepend the forward slashes
				//
				WideStringClass temp_string = U_CHAR("//");
				temp_string += translated_string;
				translated_string = temp_string;
				trans_len = translated_string.Get_Length ();
			}
		}
	}*/

	//
	//	Concatenate a '\n' onto the end of the translated string, if
	// there's one at the end of the english string
	//
	const unichar_t *buffer = english_string;
	if (buffer[english_len - 1] == U_CHAR('\n')) {
		const unichar_t *trans_buffer = translated_string;
		if (trans_buffer[trans_len - 1] != U_CHAR('\n')) {
			translated_string += U_CHAR("\n");
		}
	}

	return ;
}
