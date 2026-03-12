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
 *                 Project Name : Commmando																	  *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/commando/vendor.h         $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/10/02 5:29p                                                $*
 *                                                                                             *
 *                    $Revision:: 7                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __VENDOR_H
#define __VENDOR_H

#include "building.h"
#include "basecontroller.h"
#include "characterclasssettings.h"


////////////////////////////////////////////////////////////////
//	Forward declarations
////////////////////////////////////////////////////////////////
class SoldierGameObj;
class PurchaseSettingsDefClass;


////////////////////////////////////////////////////////////////
//
//	VendorClass
//
////////////////////////////////////////////////////////////////
class VendorClass
{
public:

	////////////////////////////////////////////////////////////////
	//	Public constants
	////////////////////////////////////////////////////////////////
	typedef enum
	{
		PERR_UNKNOWN				= -1,
		PERR_SUCCESS				= 0,
		PERR_OPERATION_PENDING,
		PERR_NO_FUNDS,
		PERR_NO_FACTORY,
		PERR_NOT_IN_STOCK

	}	PURCHASE_ERROR;

	typedef enum
	{
		TYPE_CHARACTER				= 0,
		TYPE_VEHICLE,
		TYPE_ENLISTED_CHARACTER,
		TYPE_BEACON,
		TYPE_SUPPLY,
		TYPE_SECRET_CHARACTER,
		TYPE_SECRET_VEHICLE,

	}	PURCHASE_TYPE;

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////

	//
	//	Purchase support
	//
	static PURCHASE_ERROR	Purchase_Item (SoldierGameObj *player, PURCHASE_TYPE type, int item_index, int alt_skin_index = -1, bool is_from_server = true);

private:

	////////////////////////////////////////////////////////////////
	//	Private methods
	////////////////////////////////////////////////////////////////
	static PURCHASE_ERROR	Purchase_Vehicle (BaseControllerClass *base, SoldierGameObj *player, int cost, int definition_id);
	static PURCHASE_ERROR	Purchase_Powerup (BaseControllerClass *base, SoldierGameObj *player, int cost, int definition_id);
	static PURCHASE_ERROR	Purchase_Character (BaseControllerClass *base, SoldierGameObj *player, int cost, int definition_id);

	static void					Get_Merchandise_Information (SoldierGameObj *player, PURCHASE_TYPE type, int item_index, int alt_skin_index, int &cost, int &definition);
	static void					Grant_Supplies (SoldierGameObj *player);
};


#endif //__VENDOR_H

