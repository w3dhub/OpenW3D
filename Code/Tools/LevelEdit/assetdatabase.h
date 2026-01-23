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
 *                 Project Name : leveledit                                                    *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/assetdatabase.h              $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/29/02 4:31p                                               $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __ASSETDATABASE_H
#define __ASSETDATABASE_H


//////////////////////////////////////////////////////////////////////
//	Forward declarations
//////////////////////////////////////////////////////////////////////
class StringClass;
class FileClass;


//////////////////////////////////////////////////////////////////////
//
//	AssetDatabaseClass
//
//////////////////////////////////////////////////////////////////////
class AssetDatabaseClass
{
public:

	///////////////////////////////////////////////////////////////////
	//	Public data types
	///////////////////////////////////////////////////////////////////
	typedef enum
	{
		UNKNOWN					= -1,
		CHECKED_OUT_TO_ME		= 0,
		CHECKED_OUT,
		NOT_CHECKED_OUT

	} FILE_STATUS;

	///////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	///////////////////////////////////////////////////////////////////
	AssetDatabaseClass  (void)					{}
	virtual ~AssetDatabaseClass (void)		{}

	///////////////////////////////////////////////////////////////////
	//	Public methods
	///////////////////////////////////////////////////////////////////
	
		//
		//	Database methods
		//
		virtual bool				Open_Database (LPCTSTR ini_filename, LPCTSTR username = NULL, LPCTSTR password = NULL) = 0;
		
		//
		//	File manipulation methods
		//
		virtual bool				Add_File (LPCTSTR local_filename, LPCTSTR comment = NULL) = 0;
		virtual bool				Check_In (LPCTSTR local_filename, LPCTSTR comment = NULL) = 0;
		virtual bool				Check_Out (LPCTSTR local_filename, bool get_locally = true) = 0;
		virtual bool				Undo_Check_Out (LPCTSTR local_filename) = 0;
		virtual bool				Get (LPCTSTR local_filename) = 0;
		virtual bool				Get_Subproject (LPCTSTR local_filename) = 0;
		virtual bool				Get_All (LPCTSTR /* dest_path */, LPCTSTR /* search_mask */) { return false; }

		virtual FileClass *		Get_File (LPCTSTR local_filename) = 0;

		//
		//	Extended methods which provide UI
		//
		virtual bool				Check_Out_Ex (LPCTSTR local_filename, HWND parent_wnd) = 0;
		virtual bool				Check_In_Ex (LPCTSTR local_filename, HWND parent_wnd) = 0;

		//
		// Retry methods
		//
		virtual bool				Retry_Check_Out (LPCTSTR local_filename, int attempts = 1, int delay = 250) = 0;
		virtual bool				Retry_Check_In (LPCTSTR local_filename, int attempts = 1, int delay = 250) = 0;
		
		//
		//	File information methods
		//
		
		virtual FILE_STATUS		Get_File_Status (LPCTSTR local_filename, StringClass *checked_out_user_name = NULL) = 0;
		virtual bool				Is_File_Different (LPCTSTR local_filename) = 0;
		virtual bool				Does_File_Exist (LPCTSTR local_filename) = 0;

		//
		// User information
		//
		virtual bool				Is_Read_Only (void) const	{ return true; }

protected:
	
	///////////////////////////////////////////////////////////////////
	//	Protected methods
	///////////////////////////////////////////////////////////////////
	
	///////////////////////////////////////////////////////////////////
	//	Protected member data
	///////////////////////////////////////////////////////////////////	
};


#endif //__ASSETDATABASE_H
