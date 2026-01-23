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
 *                     $Archive:: /Commando/Code/wwnet/networkobjectfactory.h                 $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 5/18/01 3:49p                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __NETWORK_OBJECT_FACTORY_H
#define __NETWORK_OBJECT_FACTORY_H

#include "always.h"
#include "bittype.h"
#include "wwpacket.h"


//////////////////////////////////////////////////////////////////////////////////
// Forward declarations
//////////////////////////////////////////////////////////////////////////////////
class NetworkObjectClass;


//////////////////////////////////////////////////////////////////////////////////
//
//	NetworkObjectFactoryClass 
//
//	These factories act as virtual constructors for object network objects.  They
//	are responsible for creating new network objects for a particular class of objects.
//
//////////////////////////////////////////////////////////////////////////////////
class NetworkObjectFactoryClass
{
public:

	//////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////////
	NetworkObjectFactoryClass (void);
	virtual ~NetworkObjectFactoryClass (void);

	//////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////
	virtual NetworkObjectClass *	Create (cPacket &packet) const = 0;
	virtual void						Prep_Packet (NetworkObjectClass */* object */, cPacket &/* packet */) const {};
	virtual uint32						Get_Class_ID (void) const = 0;

protected:
	
	//////////////////////////////////////////////////////////////
	//	Protected member data
	//////////////////////////////////////////////////////////////
	NetworkObjectFactoryClass *	NextFactory;
	NetworkObjectFactoryClass *	PrevFactory;

	
	//////////////////////////////////////////////////////////////
	//	Friends
	//////////////////////////////////////////////////////////////
	friend class NetworkObjectFactoryMgrClass;
};


//////////////////////////////////////////////////////////////////////////////////
//
//	SimpleNetworkObjectFactoryClass 
//
//	Template class to automate the creation of simple network object factories.
//
//////////////////////////////////////////////////////////////////////////////////
template<class T, int class_id>
class SimpleNetworkObjectFactoryClass : public NetworkObjectFactoryClass
{
public:

	//////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////////
	SimpleNetworkObjectFactoryClass (void)	{}

	//////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////
	virtual NetworkObjectClass *	Create (cPacket &packet) const override;
	virtual uint32						Get_Class_ID (void) const override;
};

template<class T, int class_id>
inline NetworkObjectClass *
SimpleNetworkObjectFactoryClass<T, class_id>::Create (cPacket & /*packet*/) const
{
	return new T;
}

template<class T, int class_id>
inline uint32
SimpleNetworkObjectFactoryClass<T, class_id>::Get_Class_ID (void) const
{
	return class_id;
}

#define DECLARE_NETWORKOBJECT_FACTORY(_class, _id)					\
SimpleNetworkObjectFactoryClass<_class, _id> _class ## Factory


#endif //__NETWORK_OBJECT_FACTORY_H
