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
 *                     $Archive:: /Commando/Code/commando/dlgcncpurchasemainmenu.h            $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/03/02 11:40a                                              $*
 *                                                                                             *
 *                    $Revision:: 9                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __DLGCNCPURCHASEMAINMENU_H
#define __DLGCNCPURCHASEMAINMENU_H


#include "menudialog.h"
#include "purchasesettings.h"


////////////////////////////////////////////////////////////////
//	Forward declarations
////////////////////////////////////////////////////////////////
class MPChatChildDialogClass;


////////////////////////////////////////////////////////////////
//
//	CNCPurchaseMainMenuClass
//
////////////////////////////////////////////////////////////////
class CNCPurchaseMainMenuClass : public MenuDialogClass
{
public:
	
	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////	
	CNCPurchaseMainMenuClass (void);
	~CNCPurchaseMainMenuClass (void);

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////
	
	//
	//	Inherited
	//
	void		On_Init_Dialog (void) override;
	void		On_Destroy (void) override;
	void		On_Command (int ctrl_id, int mesage_id, DWORD param) override;
	void		On_Merchandise_Selected (MerchandiseCtrlClass *ctrl, int ctrl_id) override;
	void		On_Merchandise_DblClk (MerchandiseCtrlClass *ctrl, int ctrl_id) override;
	bool		On_Key_Down (uint32 key_id, uint32 key_data) override;
	void		On_Frame_Update (void) override;
	void		Render (void) override;

	//
	//	Configuration
	//
	void		Set_Team (PurchaseSettingsDefClass::TEAM team)	{ Team = team; }

	//
	// C&C Easter Eggs
	//
	static void		Enable_Secrets(bool onoff)						{ SecretsEnabled = onoff; }
	static bool		Are_Secrets_Enabled(void)						{ return SecretsEnabled; }

private:

	////////////////////////////////////////////////////////////////
	//	Private methods
	////////////////////////////////////////////////////////////////
	void		Do_Purchase_Screen (PurchaseSettingsDefClass::TYPE type);
	void		Configure_Purchase_Controls (void);
	void		Purchase (void);
	void		Purchase_Item (int ctrl_id);
	void		Refresh_Button_States (void);
	void		Refresh_Beacon_State (void);
	void		Refresh_Message_Log (void);
	void		Clear_Selections (void);

	////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////
	PurchaseSettingsDefClass::TEAM	Team;
	MPChatChildDialogClass *			ChatModule;
	float										RefreshTimer;
	int										MessageLogLength;
	static bool								SecretsEnabled;
};



#endif //__DLGCNCPURCHASEMAINMENU_H

