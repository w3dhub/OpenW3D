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
 *                     $Archive:: /Commando/Code/commando/purchaserequestevent.h                           $* 
 *                                                                                             * 
 *                      $Author:: Patrick                                                     $* 
 *                                                                                             * 
 *                     $Modtime:: 1/05/02 5:45p                                               $* 
 *                                                                                             * 
 *                    $Revision:: 5                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef __PURCHASEREQUESTEVENT_H__
#define __PURCHASEREQUESTEVENT_H__

#include "netevent.h"
#include "netclassids.h"
#include "vendor.h"


//-----------------------------------------------------------------------------
//
// A C->S mirrored object for C&C mode purchase requests
//
class	cPurchaseRequestEvent : public cNetEvent
{
public:
   cPurchaseRequestEvent(void);

	void						Init(VendorClass::PURCHASE_TYPE type, int item_index, int alt_skin_index);

	virtual void			Export_Creation(BitStreamClass &packet) override;
	virtual void			Import_Creation(BitStreamClass &packet) override;

	virtual uint32			Get_Network_Class_ID(void) const override				{return NETCLASSID_PURCHASEREQUESTEVENT;}

private:

	virtual void			Act(void) override;

	int		SenderId;
	int		PurchaseType;
	int		ItemIndex;
	int		AltSkinIndex;
};

//-----------------------------------------------------------------------------

#endif	// __PURCHASEREQUESTEVENT_H__
