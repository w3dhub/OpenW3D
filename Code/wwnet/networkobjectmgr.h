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
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando                                                     *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwnet/networkobjectmgr.h                     $*
 *                                                                                             *
 *                      $Author:: Tom_s                                                       $*
 *                                                                                             *
 *                     $Modtime:: 11/24/01 10:47a                                             $*
 *                                                                                             *
 *                    $Revision:: 14                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef	__NETWORKOBJECTMGR_H
#define	__NETWORKOBJECTMGR_H

#include "vector.h"


////////////////////////////////////////////////////////////////
//	Forward declarations
////////////////////////////////////////////////////////////////
class NetworkObjectClass;


////////////////////////////////////////////////////////////////
//	ID Ranges
////////////////////////////////////////////////////////////////
enum
{
	NETID_DYNAMIC_OBJECT_MIN	= 1500000000,	// 600M dynamic
	NETID_DYNAMIC_OBJECT_MAX	= 2100000000,
	NETID_STATIC_OBJECT_MIN		= 2100000001,  // 10M static
	NETID_STATIC_OBJECT_MAX		= 2110000000,
	NETID_CLIENT_OBJECT_MIN		= 2110000001,  // 100K per client, 128 clients
	//NETID_CLIENT_OBJECT_MAX		= 2113100000,
	NETID_CLIENT_OBJECT_MAX		= 2122800001,

	//
	// Note: INT_MAX = 2147483647
	//
};


////////////////////////////////////////////////////////////////
//
//	NetworkObjectMgrClass
//
//	Container for network objects.  Used for sending and receiving
// object state updates.
//
////////////////////////////////////////////////////////////////
class NetworkObjectMgrClass
{
public:

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////

	//
	//	Object registration
	//
	static void							Register_Object (NetworkObjectClass *object);
	static void							Unregister_Object (NetworkObjectClass *object);

	//
	//	Delete registration support
	//
	static void							Register_Object_For_Deletion (NetworkObjectClass *object);
	static void							Set_Is_Level_Loading (bool onoff)	{ _IsLevelLoading = onoff; }

	//
	//	Timestep
	//
	static void							Think (void);

	//
	//	Deletion support
	//
	static void							Set_All_Delete_Pending (void);//TSS092301
	static void							Delete_Pending (void);
	static void							Delete_Client_Objects (int client_id);
	static void							Restore_Dirty_Bits (int client_id);

	//
	//	Object enumeration
	//
	static int							Get_Object_Count (void)	{ return _ObjectList.Count (); }
	static NetworkObjectClass *	Get_Object (int index)	{ return _ObjectList[index]; }
	static int							Get_Pending_Object_Count (void)	{ return _DeletePendingList.Count (); }

	//
	//	Object lookup
	//
	static NetworkObjectClass *	Find_Object (int object_id);

	//
	//	ID access
	//
	static int							Get_New_Dynamic_ID (void);
	static int							Get_Current_Dynamic_ID (void);
	static void							Set_New_Dynamic_ID (int id);

	static void							Init_New_Client_ID (int client_id);
	static int							Get_New_Client_ID (void);

	static void							Reset_Import_State_Counts(void);

private:

	////////////////////////////////////////////////////////////////
	//	Private methods
	////////////////////////////////////////////////////////////////
	static bool							Find_Object (int id_to_find, int *index);

	////////////////////////////////////////////////////////////////
	//	Private tyepdefs
	////////////////////////////////////////////////////////////////
	typedef DynamicVectorClass<NetworkObjectClass *>	OBJECT_LIST;

	////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////
	static OBJECT_LIST	_ObjectList;
	static OBJECT_LIST	_DeletePendingList;
	static int			_NewDynamicID;
	static int			_NewClientID;
	static bool			_IsLevelLoading;
};


#endif	// __NETWORKOBJECTMGR_H
