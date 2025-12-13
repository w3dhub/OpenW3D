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
 *                     $Archive:: /Commando/Code/Commando/renegadeplayerterminal.cpp          $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 12/17/01 2:18p                                              $*
 *                                                                                             *
 *                    $Revision:: 5                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "renegadeplayerterminal.h"
#include "renegadedialog.h"
#include "dlgcncpurchasemainmenu.h"
#include "soldier.h"
#include "playertype.h"
#include "renegadedialogmgr.h"


//////////////////////////////////////////////////////////////////////
//	Global variables
//////////////////////////////////////////////////////////////////////
static RenegadePlayerTerminalClass	_ThePlayerTerminal;


//////////////////////////////////////////////////////////////////////
//
//	RenegadePlayerTerminalClass
//
//////////////////////////////////////////////////////////////////////
RenegadePlayerTerminalClass::RenegadePlayerTerminalClass (void)
{
	_TheInstance = this;
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	~RenegadePlayerTerminalClass
//
//////////////////////////////////////////////////////////////////////
RenegadePlayerTerminalClass::~RenegadePlayerTerminalClass (void)
{
	_TheInstance = NULL;
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Show_Terminal_Dialog
//
//////////////////////////////////////////////////////////////////////
void
RenegadePlayerTerminalClass::Show_Terminal_Dialog (PurchaseSettingsDefClass::TEAM team)
{
	//
	//	Configure the team setting for the dialog
	//
	CNCPurchaseMainMenuClass *dialog = new CNCPurchaseMainMenuClass;
	dialog->Set_Team (team);

	//
	//	Show the dialog
	//
	dialog->Start_Dialog ();
	REF_PTR_RELEASE (dialog);
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Display_Terminal
//
//////////////////////////////////////////////////////////////////////
void
RenegadePlayerTerminalClass::Display_Terminal (SoldierGameObj *player, TYPE type)
{
	//if (player == NULL) {
	if (player == NULL || player->Get_Player_Data() == NULL) {
		return ;
	}
	
	//
	//	Check to ensure this player can access the given terminal
	//
	if (player->Get_Player_Type () == PLAYERTYPE_GDI && type == TYPE_GDI) {
		Show_Terminal_Dialog (PurchaseSettingsDefClass::TEAM_GDI);
	} else if (player->Get_Player_Type () == PLAYERTYPE_NOD && type == TYPE_NOD) {
		Show_Terminal_Dialog (PurchaseSettingsDefClass::TEAM_NOD);
	} else if (type == TYPE_MUTANT) {
		
		//
		//	Determine what player terminal to display -- GDI mutant or NOD mutant
		//
		if (player->Get_Player_Type () == PLAYERTYPE_GDI) {
			Show_Terminal_Dialog (PurchaseSettingsDefClass::TEAM_MUTANT_GDI);
		} else {
			Show_Terminal_Dialog (PurchaseSettingsDefClass::TEAM_MUTANT_NOD);
		}

	} else {
		
		//
		//	Display a dialog to the user telling them they don't have access to this terminal.
		//
		RenegadeDialogMgrClass::Do_Simple_Dialog (GetRenegadeDialog(RenegadeDialogID::IDD_CNC_PURCHASE_ACCESS_DENIED));
	}

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Display_Default_Terminal_For_Player
//
//////////////////////////////////////////////////////////////////////
void
RenegadePlayerTerminalClass::Display_Default_Terminal_For_Player (SoldierGameObj *player)
{
	//if (player == NULL) {
	if (player == NULL || player->Get_Player_Data() == NULL) {
		return ;
	}

	if (player->Get_Player_Type () == PLAYERTYPE_GDI) {
		Show_Terminal_Dialog (PurchaseSettingsDefClass::TEAM_GDI);
	} else if (player->Get_Player_Type () == PLAYERTYPE_NOD) {
		Show_Terminal_Dialog (PurchaseSettingsDefClass::TEAM_NOD);
	}

	return ;
}
