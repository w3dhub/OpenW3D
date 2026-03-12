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
 *                 Project Name : commando                                                    *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Commando/singletoninstancekeeper.h                              $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 11/06/01 9:02p                                                $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __SINGLETONINSTANCEKEEPER_H
#define __SINGLETONINSTANCEKEEPER_H


#include "win.h"


//////////////////////////////////////////////////////////////////////
//
//	SingletonInstanceKeeperClass
//
//////////////////////////////////////////////////////////////////////
class SingletonInstanceKeeperClass
{
public:

	///////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	///////////////////////////////////////////////////////////////////
	SingletonInstanceKeeperClass  (void);
	~SingletonInstanceKeeperClass (void);

	///////////////////////////////////////////////////////////////////
	//	Public methods
	///////////////////////////////////////////////////////////////////

	//
	//	Verification methods
	//
	bool		Verify_Safe_To_Execute (void);

	//
	// Configuration methods.
	//
	static void Allow_Multiple_Instances(bool flag);

	//
	//	Static methods
	//
	static const char *	Get_GUID (void)	{ return APP_GUID; }

private:

	///////////////////////////////////////////////////////////////////
	//	Private constants
	///////////////////////////////////////////////////////////////////
	static const char *	APP_GUID;

	///////////////////////////////////////////////////////////////////
	//	Private member data
	///////////////////////////////////////////////////////////////////
	HANDLE	AppMutex;
	HANDLE	AutoPlayMutex;

	static bool AllowMultipleInstances;

};


#endif //__SINGLETONINSTANCEKEEPER_H
