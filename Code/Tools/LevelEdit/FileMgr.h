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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/FileMgr.h                    $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/29/02 11:48a                                              $*
 *                                                                                             *
 *                    $Revision:: 17                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __FILEMGR_H
#define __FILEMGR_H


#include "vector.h"
#include "bittype.h"
#include "listtypes.h"


///////////////////////////////////////////////////////////////////////
//	Forward declarations
///////////////////////////////////////////////////////////////////////
class RenderObjClass;
class PresetClass;
class AssetDatabaseClass;
class NodeClass;


///////////////////////////////////////////////////////////////////////
//	Constants
///////////////////////////////////////////////////////////////////////
extern const char *SPECIAL_USER_FOLDER;
extern const char *PROXY_TESTS_FOLDER;


///////////////////////////////////////////////////////////////////////////
//
// FileInfoStruct
//
///////////////////////////////////////////////////////////////////////////
struct FileInfoStruct
{
	public:

		//////////////////////////////////////////////////////////
		//	Public constructors/destructors
		//////////////////////////////////////////////////////////
		FileInfoStruct (void)
			:	m_SubdirImportant (false) { }

		FileInfoStruct (const CString &filename, bool subdir_important)
			:	m_Filename (filename),
				m_SubdirImportant (subdir_important) { }

		//////////////////////////////////////////////////////////
		//	Public operators
		//////////////////////////////////////////////////////////		
		bool			operator== (const FileInfoStruct &/* src*/) { return false; }
		bool			operator!= (const FileInfoStruct &/* src*/) { return true; }

		//////////////////////////////////////////////////////////
		//	Public fields
		//////////////////////////////////////////////////////////
		CString		m_Filename;
		bool			m_SubdirImportant;
};

typedef DynamicVectorClass<FileInfoStruct>	FILE_LIST;

///////////////////////////////////////////////////////////////////////////
//
// LevelFileStruct
//
//	Struct  definition for a level editor file.  This is used by the manager
//	to represent a file.
//
struct LevelFileStruct
{
	public:

		//////////////////////////////////////////////////////////
		//	Public constructors/destructors
		//////////////////////////////////////////////////////////
		LevelFileStruct (void)
			:	m_SubdirImportant (false),
				m_RefCount (1L) { }

		virtual ~LevelFileStruct (void) { }


		//////////////////////////////////////////////////////////
		//	Public fields
		//////////////////////////////////////////////////////////
		CString				m_Filename;
		ULONG					m_RefCount;
		bool					m_SubdirImportant;
};


///////////////////////////////////////////////////////////////////////////
//
// FileMgrClass
//
//	Class definition for a level editor file manager which controls:
//		- Versioning
//		- Reference counting
//		- VSS integration
//
///////////////////////////////////////////////////////////////////////////
class FileMgrClass
{
	public:

		//////////////////////////////////////////////////////////
		//	Public constructors/destructors
		//////////////////////////////////////////////////////////
		FileMgrClass (void);
		virtual ~FileMgrClass (void) { Free_Data (); }

		//////////////////////////////////////////////////////////
		//	Public Methods
		//////////////////////////////////////////////////////////

		//
		//	Initialization routines
		//
		void						Initialize (void);


		//
		//	Enumeration routines
		//
		int						Get_File_Count (void) const { return m_FileList.Count (); }
		CString					Get_File_Name (int index) const { return m_FileList[index]->m_Filename; }
		bool						Is_File_Path_Important (int index) const { return m_FileList[index]->m_SubdirImportant; }

		//
		// Management
		//
		void						Update (NodeClass *node, bool add_node = true);
		void						Update (PresetClass *preset, bool add_node = true);
		void						Add_Asset (LPCTSTR filename);
		void						Remove_Asset (LPCTSTR filename);
		int						Add_File (LPCTSTR filename, bool subdir_important = false);
		bool						Remove_File (int prop_index);
		int						Find_File (LPCTSTR filename) const;
		void						Build_File_List (LPCTSTR start_path, RenderObjClass *prender_obj, FILE_LIST &file_list);
		void						Find_Files (LPCTSTR path, LPCTSTR wildcard, DynamicVectorClass<CString> &file_list, bool brecurse);

		//
		// File base path management
		//
		void						Set_Base_Path (LPCTSTR base);
		LPCTSTR					Get_Base_Path (void) const { return m_BasePath; }

		//
		//	Preset library information
		//
		void						Get_Preset_Library_Path (uint32 class_id, bool is_temp, CString &path);

		//
		// Texture methods
		//
		bool						Update_Texture_Cache (LPCTSTR start_path, RenderObjClass *prender_obj);
		int						Build_Texture_List (LPCTSTR start_path, RenderObjClass *prender_obj, DynamicVectorClass<CString> &texture_list, bool brecurse = true);
		const CString &		Get_Texture_Cache_Path (void) const { return m_TextureCachePath; }

		//
		// File information
		//
		bool						Does_File_Exist (LPCTSTR filename, bool bupdate_from_vss = false);
		CString					Make_Full_Path (LPCTSTR rel_path);
		CString					Make_Relative_Path (LPCTSTR full_path);
		bool						Is_Path_Valid (LPCTSTR path);
		bool						Is_Empty_Path (LPCTSTR path);
		bool						Is_Path_In_Asset_Tree (LPCTSTR path);
		bool						Determine_File_Location (LPCTSTR full_path, CString &real_location);

		//
		//	Include file methods
		//
		void									Build_Global_Include_List (void);
		bool									Update_Global_Include_File_List (void);
		DynamicVectorClass<CString> &	Get_Global_Include_File_List (void)			{ return m_GlobalIncludeFiles; }
		DynamicVectorClass<CString> &	Get_Include_File_List (void)					{ return m_IncludeFiles; }
		void									Build_Include_List (LPCTSTR ini_filename, DynamicVectorClass<CString> &list);

		//
		//	VSS integration methods
		//
		bool						Initialize_VSS (LPCTSTR ini_file_path, LPCTSTR username = NULL, LPCTSTR password = NULL);
		int						Build_Update_List (DynamicVectorClass<LevelFileStruct *> &update_list);
		bool						Update_Files (DynamicVectorClass<LevelFileStruct *> *pupdate_list = NULL);
		bool						Update_File (int index);
		bool						Update_File (LPCTSTR filename);
		bool						Update_All_Files (LPCTSTR dest_path, LPCTSTR search_mask);
		bool						Update_Model (LPCTSTR local_path, LPCTSTR new_filename);
		bool						Add_Files_To_Database (LPCTSTR filename);
		bool						Add_File_To_Database (LPCTSTR filename);
		bool						Is_VSS_Integration_Enabled (void) { return m_bVSSInitialized; }
		bool						Is_File_In_VSS (LPCTSTR filename);
		bool						Get_Subproject (LPCTSTR path);
		AssetDatabaseClass &	Get_Database_Interface (void) { return *m_DatabaseInterface; }
		void						Set_Read_Only_VSS (bool bread_only = true) { m_bReadOnlyVSS = bread_only; }
		bool						Is_VSS_Read_Only (void) { return m_bReadOnlyVSS; }
		bool						Is_Special_User (void)	{ return m_IsSpecialUser; }
		bool						Update_Asset_Tree (void);

		void						Build_Dependency_List (LPCTSTR filename, UniqueListClass<CString> &list);

		//
		//	Static member data
		//
		static bool				_bAutoUpdateOn;

	protected:
		
		//////////////////////////////////////////////////////////
		//	Protected methods
		//////////////////////////////////////////////////////////
		void						Free_Data (void);

		CDialog *				m_pUpdateDlg;
		CString					m_CurrentFile;
		void						Add_Files_To_Database (void);		

		friend UINT				fnUpdateVSSThread (LPVOID pParam);

	private:

		//////////////////////////////////////////////////////////
		//	Private member data
		//////////////////////////////////////////////////////////
		DynamicVectorClass<LevelFileStruct *>	m_FileList;
		CString											m_BasePath;
		CString											m_TextureCachePath;
		AssetDatabaseClass *							m_DatabaseInterface;
		bool												m_bVSSInitialized;
		bool												m_bReadOnlyVSS;
		bool												m_IsSpecialUser;

		DynamicVectorClass<CString>				m_GlobalIncludeFiles;
		DynamicVectorClass<CString>				m_IncludeFiles;
};


#endif //__FILEMGR_H

