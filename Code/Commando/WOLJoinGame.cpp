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
* FILE
*     $Archive: /Commando/Code/Commando/WOLJoinGame.cpp $
*
* DESCRIPTION
*
* PROGRAMMER
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Revision: 18 $
*     $Modtime: 1/17/02 11:43a $
*
******************************************************************************/

#include "WOLJoinGame.h"
#include "DlgWOLWait.h"
#include "DlgMessageBox.h"
#include "DlgMPConnect.h"
#include "nat.h"
#include "natter.h"
#include "FirewallWait.h"
#include "cnetwork.h"
#include "WOLGameInfo.h"
#include <WWOnline/WOLSession.h>
#include <WWOnline/WOLProduct.h>
#include <WWOnline/WOLChannel.h>
#include "string_ids.h"
#include <wwtranslatedb/translatedb.h>
#include <wwdebug/wwdebug.h>

using namespace WWOnline;

/******************************************************************************
*
* NAME
*     WOLJoinGame::JoinTheGame
*
* DESCRIPTION
*     Join the specified game.
*
* INPUTS
*     GameName - Name of the game to join
*     Password - Game password
*
* RESULT
*     NONE
*
******************************************************************************/

void WOLJoinGame::JoinTheGame(const unichar_t* gameName, const unichar_t* password, bool allowTeamSelect)
	{
	WWASSERT(gameName != NULL && "Invalid parameter");

	if (gameName)
		{
		// Create the object to handle the game joining process.
		WOLJoinGame* joinGame = new WOLJoinGame;
		WWASSERT(joinGame && "WOLJoinGame failed to instantiate");

		if (joinGame)
			{
			if (joinGame->FinalizeCreate())
				{
				// Start the join game process
				if (joinGame->Join(gameName, password, allowTeamSelect))
					{
					return;
					}
				}
			
			// This will delete the WOLJoinGame object
			joinGame->Release_Ref();
			}

		// If we got here then the join failed. Show a dialog telling the user
		// that we were unable to join the requested game.
		WideStringClass message(255, true);
		message.Format(TRANSLATE(IDS_GAME_JOINCHANNEL), gameName);
		DlgMsgBox::DoDialog(TRANSLATE(IDS_WOL_ERROR), message);
		}
	}


/******************************************************************************
*
* NAME
*     WOLJoinGame::WOLJoinGame
*
* DESCRIPTION
*     Default constructor
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

WOLJoinGame::WOLJoinGame() :
		mJoinState(IDLE_STATE),
		mAllowTeamSelect(true),
		mTeamChoice(-1),
		mClanID(0)
	{
	WWDEBUG_SAY(("WOLJoinGame: Instantiated\n"));
	}


/******************************************************************************
*
* NAME
*     WOLJoinGame::~WOLJoinGame
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

WOLJoinGame::~WOLJoinGame()
	{
	WWDEBUG_SAY(("WOLJoinGame: Destroyed\n"));
	}


/******************************************************************************
*
* NAME
*     WOLJoinGame::FinalizeCreate
*
* DESCRIPTION
*
* INPUTS
*     NONE
*
* RESULT
*     True if successful
*
******************************************************************************/

bool WOLJoinGame::FinalizeCreate(void)
	{
	mWOLSession = Session::GetInstance(false);
	return mWOLSession.IsValid();
	}


/******************************************************************************
*
* NAME
*     WOLJoinGame::CreateGameFromChannel
*
* DESCRIPTION
*     Create a game data instance from a WWOnline game channel.
*
* INPUTS
*     Channel - Game channel to create game data instance from.
*
* RESULT
*     Game - Instance to game.
*
******************************************************************************/

cGameData* WOLJoinGame::CreateGameFromChannel(const RefPtr<ChannelData>& channel)
	{
	WWASSERT(channel.IsValid() && "Invalid paramter");
	WWDEBUG_SAY(("WOLJoinGame: Create game from channel '%S'\n", (const unichar_t*)channel->GetName()));

	// Extract game information from the channel
	WOLGameInfo gameInfo(channel);

	if (gameInfo.IsDataValid() == false)
		{
		return NULL;
		}

	// Create an instance of the correct type.
	cGameData* theGame = cGameData::Create_Game_Of_Type((cGameData::GameTypeEnum)gameInfo.GameType());

	if (theGame)
		{
		// The game owner is the name of the channel
		const WideStringClass& channelName = channel->GetName();
		theGame->Set_Owner(const_cast<WideStringClass&>(channelName));

		if (gameInfo.IsPassworded())
			{
			theGame->Set_Password(mPassword);
			}
		
		theGame->Import_Tier_1_Data(gameInfo);
		}

	return theGame;
	}


/******************************************************************************
*
* NAME
*     WOLJoinGame::Join
*
* DESCRIPTION
*     The steps to join a game successfully are as follows:
*
*       1) Join the game channel.
*       2) Allow user to select his team.
*       3) Receive team selection choice.
*       4) Create an instance of the game.
*       5) Perform firewall negotiation
*       6) Connect to the server.
*
* INPUTS
*     Name        - Name of game channel (This is usually the same as the hosts name).
*     Password    - Password to enter game; NULL if not passworded.
*     AllowSelect - True if user is allowed to select their team.
*
* RESULT
*     True if join request is successful.
*
******************************************************************************/

bool WOLJoinGame::Join(const unichar_t* gameName, const unichar_t* password, bool allowTeamSelect)
	{
	WWDEBUG_SAY(("WOLJoinGame: Joining game channel '%S' Password: '%S'\n", gameName, password));

	mAllowTeamSelect = allowTeamSelect;

	mGameName = gameName;
	mPassword = password;

	//---------------------------------------------------------------------------
	// First we need to join the channel
	//---------------------------------------------------------------------------
	RefPtr<Product> product = Product::Current();
	WWASSERT(product.IsValid());

	if (!product.IsValid())
		{
		WWDEBUG_SAY(("ERROR: WWOnline product not initialized.\n"));
		return false;
		}

	int gameType = product->GetGameCode();

	RefPtr<WaitCondition> wait = mWOLSession->JoinChannel(gameName, password, gameType);
	
	if (!wait.IsValid())
		{
		WWDEBUG_SAY(("ERROR: Couldn't create JoinChannellWait\n"));
		return false;
		}

	// Tell the firewall code that we are connecting as a client.
	WOLNATInterface.Set_Server(false);

	// Listen for channel events. We will use this to obtain the channel
	Observer<ChannelEvent>::NotifyMe(*mWOLSession);

	// Generate message for user to see while we are attempting to join the channel.
	WideStringClass message(255, true);
	message.Format(TRANSLATE(IDS_GAME_JOINCHANNEL), gameName);

	mJoinState = JOINING_STATE;
	return DlgWOLWait::DoDialog((const unichar_t*)message, wait, this);
	}


/******************************************************************************
*
* NAME
*     WOLJoinGame::ProceedWithConnection
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*     NONE
*
******************************************************************************/

void WOLJoinGame::ProceedWithConnection(int teamChoice)
	{
	WWDEBUG_SAY(("WOLJoinGame: Proceeding with connection\n"));

	bool success = StartServerConnect();

	// If the connection is underway then wait for it to complete.
	if (success)
		{
		mTeamChoice = teamChoice;
		return;
		}

	// An error has occured while attempting to connect.
	DlgMsgBox::DoDialog(IDS_WOL_ERROR, IDS_MP_UNABLE_CONNECT_TO_SERVER);

	Leave();
	}


/******************************************************************************
*
* NAME
*     WOLJoinGame::Leave
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

void WOLJoinGame::Leave(void)
	{
	WWDEBUG_SAY(("WOLJoinGame: Leaving channel\n"));

	// If the user elected to leave the game or we encountered an error starting
	// the connection then leave the channel.
	RefPtr<WaitCondition> wait = mWOLSession->LeaveChannel();
	DlgWOLWait::DoDialog(IDS_GAME_LEAVECHANNEL, wait);

	// This will delete the WOLJoinGame object
	Release_Ref();
	}


/******************************************************************************
*
* NAME
*     WOLJoinGame::StartServerConnect
*
* DESCRIPTION
*     Start server connection negotiation
*
* INPUTS
*     NONE
*
* RESULT
*     True if negotiation successfully started.
*
******************************************************************************/

bool WOLJoinGame::StartServerConnect(void)
	{
	WWDEBUG_SAY(("WOLJoinGame: Start server connect\n"));

	// If the channel is NOT valid then bad things have happened.
	if (!mTheChannel.IsValid())
		{
		WWDEBUG_SAY(("WOLJoinGame: ERROR channel not valid when connecting to host\n"));
		WWASSERT(mTheChannel.IsValid() && "Bad channel connecting to host");
		return false;
		}

	// Verify that the user can join this game.
	WOLGameInfo gameInfo(mTheChannel);
	RefPtr<UserData> user = mWOLSession->GetCurrentUser();

	if (gameInfo.CanUserJoin(user) == false)
		{
		WWDEBUG_SAY(("WOLJoinGame: ERROR - User cannot join this game\n"));
		return false;
		}

	// If we are playing a clan game then we need the users clan ID in order to join.
	if (gameInfo.IsClanGame())
		{
		mClanID = user->GetSquadID();
		}

	// Delete any existing game.
	if (PTheGameData)
		{
		delete PTheGameData;
		PTheGameData = NULL;
		}

	// Create a new game from the channel
	cGameData* theGame = CreateGameFromChannel(mTheChannel);
	WWASSERT(theGame && "WOLJoinGame failed to create cGameData");

	if (theGame == NULL)
		{
		WWDEBUG_SAY(("WOLJoinGame: ERROR failed to create cGameData\n"));
		return false;
		}

	// Assign the new game instance
	PTheGameData = theGame;

	// We are done with the channel
	mTheChannel.Release();

	//---------------------------------------------------------------------------
	// Wait for the firewall negotiation. Will send DlgWOLWaitEvent when complete.
	//---------------------------------------------------------------------------
	RefPtr<WaitCondition> wait = FirewallConnectWait::Create();

	if (!wait.IsValid())
		{
		WWDEBUG_SAY(("WOLJoinGame: ERROR couldn't create FireWallWait\n"));
		return false;
		}

	mJoinState = FIREWALL_STATE;
	return DlgWOLWait::DoDialog(TRANSLATE (IDS_MENU_CONNECTING), wait, this);
	}


/******************************************************************************
*
* NAME
*     WOLJoinGame::ConnectToServer
*
* DESCRIPTION
*     Establish connection to game server.
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
******************************************************************************/

void WOLJoinGame::ConnectToServer(void)
	{
	WWDEBUG_SAY(("WOLJoinGame: Connect to server\n"));

	// Start the client
	unsigned short my_port = FirewallHelper.Get_Client_Bind_Port();
	cNetwork::Init_Client(my_port);

	// Display the "connecting" dialog.
	// Note: This dialog will close automatically when the connection has been
	// successfully completed.
	DlgMPConnect::DoDialog(mTeamChoice, mClanID);

	// Our job is done. (This will delete the WOLJoinGame object)
	Release_Ref();
	}


/******************************************************************************
*
* NAME
*     WOLJoinGame::HandleNotification(ChannelEvent)
*
* DESCRIPTION
*     Handle channel events. Of particular interest during a join is the
*     channel data event. This event tells us that there is information
*     available from the channel encoded in the topic and exinfo fields that
*     we can use to create the game from.
*
* INPUTS
*     ChannelEvent
*
* RESULT
*     NONE
*
******************************************************************************/

void WOLJoinGame::HandleNotification(ChannelEvent& event)
	{
	// The channel data is always updated immediately after a channel is joined.
	// Therefore use this event to start the creation of the game we are joining.
	if (event.GetStatus() == ChannelNewData)
		{
		// Get the channel that is the subject of this event.
		const RefPtr<ChannelData>& channel = event.Subject();
		WWASSERT(channel.IsValid());

		// If the name of this channel is the name of the game we are joining
		// then create the game from this channel.
		const WideStringClass& channelName = channel->GetName();
	
		if (channelName.Compare_No_Case(mGameName) == 0)
			{
			WWDEBUG_SAY(("WOLJoinGame: Joined game channel '%S'\n", (const unichar_t*)channelName));

			// Keep a reference to the channel so we can create the game from it latter
			mTheChannel = channel;
			}
		}
	}


/******************************************************************************
*
* NAME
*     WOLJoinGame::HandleNotification(DlgWOLWaitEvent)
*
* DESCRIPTION
*     Handle result of channel join and firewall negotiation. If everything
*     went okay then proceed with connecting to the game server. Otherwise
*     tell the user what went wrong.
*
* INPUTS
*     Event - Result event from joining WOL game wait condition.
*
* RESULT
*     NONE
*
******************************************************************************/

void WOLJoinGame::HandleNotification(DlgWOLWaitEvent& event)
	{
	// If the wait condition completed with success.
	if (event.Result() == WaitCondition::ConditionMet)
		{
		if (JOINING_STATE == mJoinState)
			{
			mJoinState = IDLE_STATE;

			// If we just joined the channel we need to present the user with
			// the team selection dialog. This dialog will allow them to view
			// the current teams and / or change their team.
			if (mAllowTeamSelect)
				{
				// DlgMPTeamSelect will send a signal indicating if the user has opted
				// to continue with the game or to back out. See ReceiveSignal()
				DlgMPTeamSelect::DoDialog(*this);
				}
			else
				{
				ProceedWithConnection(-1);
				}
			}
		else if (FIREWALL_STATE == mJoinState)
			{
			// If we have finished the firewall negotiation then proceed with
			// the server connection.
			mJoinState = IDLE_STATE;
			ConnectToServer();
			}
		}
	else
		{
		// If the wait condition failed then report the reason for the failure and
		// leave the channel.
		const WideStringClass& text = event.Subject()->GetResultText();
		WWDEBUG_SAY(("WOLJoinGame: Wait failed - %S\n", (const unichar_t*)text));
		
		DlgMsgBox::DoDialog(TRANSLATE(IDS_WOL_ERROR), text);

		Leave();
		}
	}


/******************************************************************************
*
* NAME
*     WOLJoinGame::ReceiveSignal(MPChooseTeamSignal)
*
* DESCRIPTION
*     The team selection dialog will send a signal indicating the users choice
*     regarding his team preference.
*
* INPUTS
*     Signal
*
* RESULT
*     NONE
*
******************************************************************************/

void WOLJoinGame::ReceiveSignal(MPChooseTeamSignal& signal)
	{
	// If the user wants to proceed then connect to the server othewise leave
	// the game channel.
	if (signal.GetItemA() == true)
		{
		ProceedWithConnection(signal.GetItemB());
		}
	else
		{
		Leave();
		}
	}
