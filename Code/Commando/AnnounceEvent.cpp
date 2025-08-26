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
*     $Archive: /Commando/Code/Commando/AnnounceEvent.cpp $
*
* DESCRIPTION
*     Client announcement
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Revision: 9 $
*     $Modtime: 1/12/02 9:33p $
*
******************************************************************************/

#include "AnnounceEvent.h"
#include "networkobjectfactory.h"
#include "apppackettypes.h"
#include "networkobjectmgr.h"
#include "cnetwork.h"
#include "gamemode.h"
#include "playertype.h"
#include "translateobj.h"
#include "translatedb.h"
#include "WWAudio.h"
#include "messagewindow.h"
#include <wwlib/widestring.h>
#include "CNCModeSettings.h"
#include "floodprotectionmgr.h"


DECLARE_NETWORKOBJECT_FACTORY(CSAnnouncement, NETCLASSID_CSANNOUNCEMENT);


CSAnnouncement::CSAnnouncement(void) :
		mFromID(-1),
		mToID(-1),
		mAnnouncementID(0),
		mRadioCmdID(-1),
		mType(ANNOUNCEMENT_PUBLIC)
	{
	Set_App_Packet_Type(APPPACKETTYPE_CSANNOUNCEMENT);
	}


CSAnnouncement::~CSAnnouncement()
	{
	}


void CSAnnouncement::Init(int to_id, int announcementID, AnnouncementEnum type, int radio_cmd_id)
	{
	WWASSERT(cNetwork::I_Am_Client());

	mToID = to_id;
	mFromID = cNetwork::Get_My_Id();
	mAnnouncementID = announcementID;
	mRadioCmdID = radio_cmd_id;
	mType = type;

	Set_Network_ID(NetworkObjectMgrClass::Get_New_Client_ID());

	//
	//	Is this user "flooding" the server with text?
	//
	if (FloodProtectionMgrClass::Detect_Flooding (L"") == false)
		{

		if (cNetwork::I_Am_Server())
			{
			Act();
			}
		else
			{
			Set_Object_Dirty_Bit(0, BIT_CREATION, true);
			}
		}
	else
		{
			//
			//	Flooding detected -- don't send the message
			//
			Set_Delete_Pending();			
		}
	}


void CSAnnouncement::Act(void)
	{
	WWASSERT(cNetwork::I_Am_Server());

	if (GameModeManager::Find("Combat")->Is_Active())
		{
		SCAnnouncement* announce = new SCAnnouncement;
		announce->Init(mToID, mFromID, mAnnouncementID, mType, mRadioCmdID);
		}

	Set_Delete_Pending();
	}


void CSAnnouncement::Export_Creation(BitStreamClass& packet)
	{
	WWASSERT(cNetwork::I_Am_Only_Client());

	cNetEvent::Export_Creation(packet);
	packet.Add(mToID);
	packet.Add(mFromID);
	packet.Add(mAnnouncementID);
	packet.Add(mRadioCmdID);
	packet.Add((BYTE)mType);	

	Set_Delete_Pending();
	}


void CSAnnouncement::Import_Creation(BitStreamClass& packet)
	{
	WWASSERT(cNetwork::I_Am_Server());

	cNetEvent::Import_Creation(packet);
	packet.Get(mToID);
	packet.Get(mFromID);
	packet.Get(mAnnouncementID);
	packet.Get(mRadioCmdID);

	BYTE type = 0;
	packet.Get(type);
	mType = (AnnouncementEnum)type;	

	Act();
	}


//-----------------------------------------------------------------------------
DECLARE_NETWORKOBJECT_FACTORY(SCAnnouncement, NETCLASSID_SCANNOUNCEMENT);


SCAnnouncement::SCAnnouncement(void) :
		mToID(-1),
		mFromID(-1),
		mAnnouncementID(0),
		mRadioCmdID(-1),
		mType(ANNOUNCEMENT_PUBLIC)
	{
	Set_App_Packet_Type(APPPACKETTYPE_SCANNOUNCEMENT);
	}


SCAnnouncement::~SCAnnouncement()
	{
	}


void SCAnnouncement::Init(int to_id, int from_id, int announcementID, AnnouncementEnum type, int radio_cmd_id)
	{
	WWASSERT(cNetwork::I_Am_Server());

	mToID = to_id;
	mFromID = from_id;
	mAnnouncementID = announcementID;
	mRadioCmdID = radio_cmd_id;
	mType = type;

	switch (type)
		{
		case ANNOUNCEMENT_PUBLIC:
			Set_Object_Dirty_Bit(BIT_CREATION, true);
			break;

		case ANNOUNCEMENT_TEAM:
			if (mToID >= 0)
				{
				Set_Dirty_Bit_For_Team(BIT_CREATION, mToID);
				}
			break;

		case ANNOUNCEMENT_PRIVATE:
			if (mToID >= 0)
				{
				Set_Object_Dirty_Bit(mToID, BIT_CREATION, true);
				}
			break;

		default:
			break;
		}

	// Explicitly show the message on the server if he is among the recipients.
	if (cNetwork::I_Am_Client() && Is_Client_Dirty(cNetwork::Get_My_Id()))
		{
		Act();
		}
	}


void SCAnnouncement::Set_Dirty_Bit_For_Team(DIRTY_BIT bit, int team)
	{
	WWASSERT(team == PLAYERTYPE_NOD || team == PLAYERTYPE_GDI);

	SList<cPlayer>* playerList = cPlayerManager::Get_Player_Object_List();

	if (playerList)
		{
		SLNode<cPlayer>* playerNode = playerList->Head();

		while (playerNode)
			{
			cPlayer* player = playerNode->Data();

			if (player && player->Is_Active() && (player->Get_Player_Type() == team))
				{
				Set_Object_Dirty_Bit(player->Get_Id(), bit, true);
				}

			playerNode = playerNode->Next();
			}
		}
	}


void SCAnnouncement::Act(void)
	{
	if (mAnnouncementID > 0)
		{
		// Lookup the translation object from the strings database
		TDBObjClass* translateObj = TranslateDBClass::Find_Object(mAnnouncementID);

		cPlayer* sender = cPlayerManager::Find_Player(mFromID);

		//
		//	Display the emot icon as ncessary
		//
		if (sender != NULL && mRadioCmdID != -1)
			{

			//
			//	Dig the soldier game object out from the player data
			//
			SmartGameObj *game_obj = sender->Get_GameObj();
			if (game_obj != NULL && game_obj->As_SoldierGameObj () != NULL)
				{
				CNCModeSettingsDef* cncDef = CNCModeSettingsDef::Get_Instance();
				if (cncDef != NULL)
					{
					//
					//	Display the emot icon
					//
					const float EMOT_ICON_DURATION = 2.0F;
					const char *emot_icon_name = cncDef->Get_Radio_Command_Emot_Icon (mRadioCmdID);
					game_obj->As_SoldierGameObj ()->Set_Emot_Icon (emot_icon_name, EMOT_ICON_DURATION);
					}				
				}
			}

		if (translateObj)
			{
			// Display the text on the screen
			const WCHAR* string = translateObj->Get_String();

			if (string)
				{

		    if (sender)
					{
					WideStringClass message(0, true);
					message.Format(L"%s: %s", sender->Get_Name(), string);
					CombatManager::Get_Message_Window()->Add_Message(message, sender->Get_Color());
					}
				else
					{
					CombatManager::Get_Message_Window()->Add_Message(string, Vector3(1,1,1));
					}
				}		

			// Play the sound effect
			int soundID = (int)translateObj->Get_Sound_ID();

			if (soundID > 0)
				{
				WWAudioClass::Get_Instance()->Create_Instant_Sound(soundID, Matrix3D(1));
				}
			}
		}


	Set_Delete_Pending();
	}


//-----------------------------------------------------------------------------
void SCAnnouncement::Export_Creation(BitStreamClass& packet)
	{
	cNetEvent::Export_Creation(packet);
	packet.Add(mToID);
	packet.Add(mFromID);
	packet.Add(mAnnouncementID);
	packet.Add(mRadioCmdID);
	packet.Add((BYTE)mType);

	Set_Delete_Pending();
	}


//-----------------------------------------------------------------------------
void SCAnnouncement::Import_Creation(BitStreamClass& packet)
	{
	cNetEvent::Import_Creation(packet);

	WWASSERT(cNetwork::I_Am_Only_Client());

	packet.Get(mToID);
	packet.Get(mFromID);
	packet.Get(mAnnouncementID);
	packet.Add(mRadioCmdID);

	BYTE type = 0;
	packet.Get(type);
	mType = (AnnouncementEnum)type;

	Act();
	}
