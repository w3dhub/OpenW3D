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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/ScriptMgr.h                  $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 10/27/99 3:57p                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __SCRIPT_MGR_H
#define __SCRIPT_MGR_H

#include "vector.h"
#include "listtypes.h"


// Forward declarations
class EditScriptClass;


//////////////////////////////////////////////////////////////////////////
//
//	ScriptMgrClass
//
//////////////////////////////////////////////////////////////////////////
class ScriptMgrClass
{
public:

	//////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////////
	ScriptMgrClass (void)				{ }
	virtual ~ScriptMgrClass (void);

	//////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////

	static void					Initialize (void);
	static void					Shutdown (void);

	static int					Get_Count (void)			{ return _ScriptList.Count (); }
	static EditScriptClass *Get_Script (int index)	{ return _ScriptList[index]; }
	static EditScriptClass *Find_Script (LPCTSTR name);

protected:

	//////////////////////////////////////////////////////////////
	//	Protected methods
	//////////////////////////////////////////////////////////////

private:

	/////////////////////////////////////////////////////////////////////
	//	Private methods
	/////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////
	//	Static member data
	/////////////////////////////////////////////////////////////////////
	static SCRIPT_LIST _ScriptList;
};


#endif //__SCRIPT_MGR_H
