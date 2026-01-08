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

/******************************************************************************
*
* NAME
*     $Archive: /Commando/Code/Commando/DlgMPConnect.cpp $
*
* DESCRIPTION
*     Dialog to inform user that we are connecting to a game host.
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*     $Author: Tom_s $
*
* VERSION INFO
*     $Revision: 12 $
*     $Modtime: 2/25/02 11:29a $
*
******************************************************************************/

#include "DlgMPConnect.h"
#include "renegadedialog.h"
#include "gamedata.h"
#include "gameinitmgr.h"
#include "campaign.h"
#include "cnetwork.h"
#include <wwdebug/wwdebug.h>
#include "dlgmainmenu.h"
#include "gamespyadmin.h"
#include "specialbuilds.h"
#include "dialogtests.h"
#include "dialogmgr.h"
#include "gamemode.h"
#include "langmode.h"
#include "wolgmode.h"

/******************************************************************************
*
* NAME
*     DlgMPConnect::DoDialog
*
* DESCRIPTION
*
* INPUTS
*     NONE
*
* RESULT
*     True if dialog created successfully.
*
******************************************************************************/

bool DlgMPConnect::DoDialog(int teamChoice, unsigned int clanID)
	{
	DlgMPConnect* popup = new DlgMPConnect(teamChoice, clanID);

	if (popup)
		{
		popup->Start_Dialog();
		popup->Release_Ref();
		}

	return (popup != NULL);
	}


/******************************************************************************
*
* NAME
*     DlgMPConnect::DlgMPConnect
*
* DESCRIPTION
*     Constructor
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

DlgMPConnect::DlgMPConnect(int teamChoice, unsigned int clanID) :
		PopupDialogClass(GetRenegadeDialog(RenegadeDialogID::IDD_MULTIPLAY_CONNECTING)),
		mTeamChoice(teamChoice),
		mClanID(clanID),
		mTheGame(NULL),
		mFailed(false)
	{
	WWDEBUG_SAY(("DlgMPConnect: Instantiated\n"));
	}


/******************************************************************************
*
* NAME
*     DlgMPConnect::~DlgMPConnect
*
* DESCRIPTION
*     Destructor
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

DlgMPConnect::~DlgMPConnect()
	{
	WWDEBUG_SAY(("DlgMPConnect: Destructing\n"));
	}


/******************************************************************************
*
* NAME
*     DlgMPConnect::Connected
*
* DESCRIPTION
*     Handle connection.
*
* INPUTS
*     GameData - Pointer to game data instance
*
* RESULT
*     NONE
*
******************************************************************************/

void DlgMPConnect::Connected(cGameData* theGame)
	{
	mTheGame = theGame;
	}


/******************************************************************************
*
* NAME
*     DlgMPConnect::Connected
*
* DESCRIPTION
*     Handle connection.
*
* INPUTS
*     GameData - Pointer to game data instance
*
* RESULT
*     NONE
*
******************************************************************************/

void DlgMPConnect::Failed_To_Connect(void)
	{
	mFailed = true;
	}


/******************************************************************************
*
* NAME
*     DlgMPConnect::On_Command
*
* DESCRIPTION
*     Process command messages from controls
*
* INPUTS
*     Ctrl    - ID of control
*     Message -
*     Param   -	0 = User invoked abort. 1 = Connection refused by server.
*
* RESULT
*     NONE
*
******************************************************************************/

void DlgMPConnect::On_Command(int ctrlID, int message, unsigned int param)
	{
	if ((IDCANCEL == ctrlID) && (1 != param))
		{
		if (cNetwork::I_Am_Client())
			{
			cNetwork::Cleanup_Client();
			}
		if (cGameSpyAdmin::Get_Is_Launched_From_Gamespy()) 
			{
			extern void Stop_Main_Loop (int);
			Stop_Main_Loop(EXIT_SUCCESS);
			}
		else if (DialogMgrClass::Get_Dialog_Count () == 1)
			{
			START_DIALOG (MainMenuDialogClass);
			}
		}

	PopupDialogClass::On_Command(ctrlID, message, param);
	}


/******************************************************************************
*
* NAME
*     DlgMPConnect::On_Periodic
*
* DESCRIPTION
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void DlgMPConnect::On_Periodic(void)
	{
	PopupDialogClass::On_Periodic();

	if (mTheGame != NULL)
		{
		// Add a reference to keep us alive while we process the game start
		Add_Ref();

		// Remove the dialog from menuing system
		End_Dialog();

		// Check to ensure the settings are playable
		WideStringClass outMsg;

		if (mTheGame->Is_Valid_Settings(outMsg))
			{
			WWDEBUG_SAY(("DlgMPConnect: Starting the game.\n"));

			CampaignManager::Select_Backdrop_Number_By_MP_Type(mTheGame->Get_Game_Type());

			// Start the game!
			GameInitMgrClass::Set_Is_Client_Required(true);
			GameInitMgrClass::Set_Is_Server_Required(false);
			GameInitMgrClass::Start_Game(mTheGame->Get_Map_Name(), mTeamChoice, mClanID);
			}
		else
			{
			WWDEBUG_SAY(("ERROR: %s\n", (const unichar_t*)outMsg));
			}

		// Release the keep alive reference (this will delete this object)
		Release_Ref();
		}
	else
		{
		if (mFailed)
			{

			Add_Ref();

			// Remove the dialog from menuing system
			End_Dialog();

			if (GameModeManager::Find("LAN")->Is_Active())
				{
      		PLC->Refusal_Actions();
   			}
			else
				{
		 		GameModeClass* gameMode = GameModeManager::Find("WOL");

		 		if (gameMode && gameMode->Is_Active())
					{
			 		WolGameModeClass* wolGame = static_cast<WolGameModeClass*>(gameMode);
			 		WWASSERT(wolGame);
			 		wolGame->Refusal_Actions();
		 			}
				}

			Release_Ref();
			}
		}
	}
