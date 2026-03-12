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
 *                 Project Name : WWSaveLoad                                                   *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwnet/networkobjectfactorymgr.h              $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 5/18/01 2:35p                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __NETWORK_OBJECT_FACTORY_MGR_H
#define __NETWORK_OBJECT_FACTORY_MGR_H

#include "always.h"
#include "bittype.h"

//////////////////////////////////////////////////////////////////////////////////
//	Forward declarations
//////////////////////////////////////////////////////////////////////////////////
class NetworkObjectFactoryClass;

//////////////////////////////////////////////////////////////////////////////////
//
//	NetworkObjectFactoryMgrClass
//
//////////////////////////////////////////////////////////////////////////////////
class NetworkObjectFactoryMgrClass
{
public:

	/////////////////////////////////////////////////////////////////////
	//	Public methods
	/////////////////////////////////////////////////////////////////////
	static NetworkObjectFactoryClass *	Find_Factory (uint32 class_id);
	static void									Register_Factory (NetworkObjectFactoryClass *factory);
	static void									Unregister_Factory (NetworkObjectFactoryClass *factory);

	// Factory enumeration
	static NetworkObjectFactoryClass *	Get_First (void);
	static NetworkObjectFactoryClass *	Get_Next (NetworkObjectFactoryClass *current);

private:

	/////////////////////////////////////////////////////////////////////
	//	Private methods
	/////////////////////////////////////////////////////////////////////
	static void									Link_Factory (NetworkObjectFactoryClass *factory);
	static void									Unlink_Factory (NetworkObjectFactoryClass *factory);

	/////////////////////////////////////////////////////////////////////
	//	Static member data
	/////////////////////////////////////////////////////////////////////
	static NetworkObjectFactoryClass *	_FactoryListHead;
};


#endif //__NETWORK_OBJECT_FACTORY_MGR_H
