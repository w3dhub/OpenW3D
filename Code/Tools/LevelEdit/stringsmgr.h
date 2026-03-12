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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/stringsmgr.h                 $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 12/04/01 11:37a                                             $*
 *                                                                                             *
 *                    $Revision:: 9                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif


#ifndef __STRINGSMGR_H
#define __STRINGSMGR_H

#include "bittype.h"
#include "widestring.h"


/////////////////////////////////////////////////////////////////////////
//
//	StringsMgrClass
//
/////////////////////////////////////////////////////////////////////////
class StringsMgrClass
{
	public:

		//////////////////////////////////////////////////////////
		//	Public methods
		//////////////////////////////////////////////////////////

		//
		//	Translation database save/load
		//
		static void		Save_Translation_Database (void);
		static void		Save_Translation_Database (const char *full_path);
		static void		Load_Translation_Database (void);
		static void		Create_Database_If_Necessary (void);

		//
		//	Translation database version control
		//
		static bool		Get_Latest_Version (void);
		static bool		Check_Out (void);
		static bool		Check_In (void);
		static bool		Undo_Check_Out (void);

		//
		//	Translation database ID import/export
		//
		static void		Import_Strings (void);
		static void		Import_IDs (void);
		static void		Export_IDs (void);

		//
		//	Translation database import/export
		//
		static void		Export_For_Translation (const char *filename, uint32 lang_id);
		static void		Import_From_Translation (const char *filename, uint32 lang_id);

		//
		//	Editor support
		//
		static void		Edit_Database (HWND parent_wnd);

	protected:

		//////////////////////////////////////////////////////////
		//	Protected methods
		//////////////////////////////////////////////////////////

		static void		Convert_Newline_To_Chars (WideStringClass &string);
		static void		Convert_Chars_To_Newline (WideStringClass &string);
		static void		Apply_Characteristics (WideStringClass &english_string, WideStringClass &translated_string);

	private:

		//////////////////////////////////////////////////////////
		//	Private member data
		//////////////////////////////////////////////////////////
};


#endif //__STRINGSMGR_H
