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
 *                 Project Name : Commando                                                     *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/commando/dlgcncpurchasemenu.h                $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/10/02 4:35p                                               $*
 *                                                                                             *
 *                    $Revision:: 14                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __DLGCNCPURCHASEMENU_H
#define __DLGCNCPURCHASEMENU_H

#include "menudialog.h"
#include "playerterminal.h"
#include "purchasesettings.h"


////////////////////////////////////////////////////////////////
//	Forward declarations
////////////////////////////////////////////////////////////////
class PurchaseSettingsDefClass;
class MerchandiseCtrlClass;


////////////////////////////////////////////////////////////////
//
//	CNCPurchaseMenuClass
//
////////////////////////////////////////////////////////////////
class CNCPurchaseMenuClass : public MenuDialogClass
{
public:
	
	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////	
	CNCPurchaseMenuClass (void);

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////
	
	//
	//	Inherited
	//
	void		On_Init_Dialog (void) override;
	void		On_Command (int ctrl_id, int mesage_id, DWORD param) override;
	void		On_Frame_Update (void) override;
	void		On_Merchandise_Selected (MerchandiseCtrlClass *ctrl, int ctrl_id) override;
	void		On_Merchandise_DblClk (MerchandiseCtrlClass *ctrl, int ctrl_id) override;
	bool		On_Key_Down (uint32 key_id, uint32 key_data) override;
	void		Render (void) override;

	//
	//	Configuration
	//
	void		Set_Definition (PurchaseSettingsDefClass *definition)	{ Definition = definition; }
	void		Set_Team (PlayerTerminalClass::TYPE team)					{ Team = team; }
	void		Set_Type (PurchaseSettingsDefClass::TYPE type)			{ PurchaseType = type; }

	void		Enable_Production (bool onoff)	{ IsProductionDisabled = !onoff; }
	bool		Is_Production_Enabled (void)		{ return IsProductionDisabled; }

	void		Set_Cost_Scaling_Factor (float factor)	{ CostScalingFactor = factor; }
	float		Get_Cost_Scaling_Factor (void) const	{ return CostScalingFactor; }

	void		Enable_Multiple_Purchases (bool onoff)			{ EnableMultiplePurchases = onoff; }
	bool		Are_Multiple_Purchases_Enabled (void) const	{ return EnableMultiplePurchases; }

private:

	////////////////////////////////////////////////////////////////
	//	Private methods
	////////////////////////////////////////////////////////////////	
	void		Add_Item_To_Shopping_Cart (int ctrl_index);
	void		Clear_Shopping_Cart (void);
	void		Purchase (void);
	void		Purchase_Item (int ctrl_id);
	bool		Is_Definition_OK (DefinitionClass *definition);
	bool		Verify_Vehicle_Purchase (void);

	void		Update_Enabled_Status (void);
	void		Update_Building_Health (void);
	void		Configure_Building_Icons (void);

	////////////////////////////////////////////////////////////////
	//	Private structs
	////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////
	PlayerTerminalClass::TYPE				Team;
	PurchaseSettingsDefClass::TYPE		PurchaseType;
	PurchaseSettingsDefClass *				Definition;
	DynamicVectorClass<int>					ShoppingList;
	int											TotalCost;
	bool											IsProductionDisabled;
	bool											EnableMultiplePurchases;
	float											CostScalingFactor;
	float											HealthUpdateTimer;
	float											EnabledStateUpdateTimer;
};


#endif //__DLGCNCPURCHASEMENU_H

