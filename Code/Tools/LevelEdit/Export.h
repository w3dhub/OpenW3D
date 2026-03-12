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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/Export.h                     $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/31/02 1:57p                                               $*
 *                                                                                             *
 *                    $Revision:: 16                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __EXPORT_H
#define __EXPORT_H

#include "vector.h"
#include "listtypes.h"
#include "wwstring.h"
#include "editormixfile.h"


//////////////////////////////////////////////////////////////////////
//
//	ExporterClass
//
//////////////////////////////////////////////////////////////////////
class ExporterClass
{
	public:

		//////////////////////////////////////////////////////////
		//	Public constructors/destructors
		//////////////////////////////////////////////////////////
		ExporterClass (void)	:
			DontGetVersionsFromVSS (false)	{}
		virtual ~ExporterClass (void)			{}

		//////////////////////////////////////////////////////////
		//	Public Methods
		//////////////////////////////////////////////////////////
		void		Export_Level (LPCTSTR filename);
		void		Export_Always_Files (LPCTSTR path);
		void		Export_Database_Mix (LPCTSTR path);

		void		Export_Package (LPCTSTR path);
		void		Export_Level_Only (LPCTSTR path);

		//
		//	VSS Support
		//
		void		Dont_Get_Latest_Versions (bool onoff)	{ DontGetVersionsFromVSS = onoff; }

	protected:

		//////////////////////////////////////////////////////////
		//	Protected methods
		//////////////////////////////////////////////////////////
		void		Process_Include_Files (const STRING_LIST &search_list);
		bool		Build_File_List (const STRING_LIST &search_list, STRING_LIST &file_list);
		bool		Export_Include_Files (const STRING_LIST &file_list);
		bool		Export_Definition_Databases (bool use_temp_strings_library);
		bool		Export_Level_File (void);
		bool		Export_Level_Data (void);
		void		Export_Dependency_File (void);

		bool		Delete_File (LPCTSTR filename);
		bool		Clean_Directory (LPCTSTR local_dir);

		void		Delete_Temp_Directory (void);
		void		Make_Temp_Directory (void);

		void		Pre_Level_Export (DynamicVectorClass<NodeClass *> &node_list);
		void		Post_Level_Export (DynamicVectorClass<NodeClass *> &node_list);

		void		Find_Files (const char *search_mask, STRING_LIST &file_list);

		void		Add_Files_To_Mod_Package (LPCTSTR full_path);

	private:

		//////////////////////////////////////////////////////////
		//	Private member data
		//////////////////////////////////////////////////////////
		CString									Path;
		CString									FilenameBase;
		CString									TempDirectory;

		DynamicVectorClass<StringClass>	m_AssetList;

		EditorMixFileCreator					MixFileCreator;

		bool										DontGetVersionsFromVSS;
};

#endif //__EXPORT_H
