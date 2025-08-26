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
*     GroupScript.cpp
*
* DESCRIPTION
*     Group script
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*
* VERSION INFO
*     $Author: Rich_d $
*     $Revision: 2 $
*     $Modtime: 6/14/00 12:55p $
*     $Archive: /Commando/Code/Scripts/GroupScript.cpp $
*
******************************************************************************/

#include "scripts.h"
#include "groupcontrol.h"
#include "group.h"
#include "customevents.h"
#include "DPrint.h"


DECLARE_SCRIPT(MXX_Group_Member_DEL, "GroupName:string")
	{
	const char* mGroupName;

	// Add the object to the group when it is created.
	void Created(GameObject* owner)
		{
		mGroupName = Get_Parameter("GroupName");
		assert(mGroupName != NULL);

		GroupController* controller = GroupController::Instance();
		assert(controller != NULL);
		controller->AddToGroup(mGroupName, owner);
		}

	
	// Remove the object from the group when it is destroyed.
	void Destroyed(GameObject* owner)
		{
		GroupController* controller = GroupController::Instance();
		assert(controller != NULL);
		controller->RemoveFromGroup(mGroupName, owner);
		}


	// Notify group that a member was killed.
	void Killed(GameObject* owner, GameObject* killer)
		{
		GroupController* controller = GroupController::Instance();
		assert(controller != NULL);

		GroupEventInfo info;
		info.GroupName = mGroupName;
		info.Event = GROUP_MEMBER_KILLED;
		info.Object = killer;

		Group* group = controller->FindGroup(mGroupName);
		assert(group != NULL);
		group->SendCustomEvent(owner, SCMD_GROUP_EVENT, (int)&info);
		}

	
	// Notify group that a member was damaged.
	void Damaged(GameObject* owner, GameObject* damager)
		{
		GroupController* controller = GroupController::Instance();
		assert(controller != NULL);

		GroupEventInfo info;
		info.GroupName = mGroupName;
		info.Event = GROUP_MEMBER_DAMAGED;
		info.Object = damager;

		Group* group = controller->FindGroup(mGroupName);
		assert(group != NULL);
		group->SendCustomEvent(owner, SCMD_GROUP_EVENT, (int)&info);
		}


	// Notify group that a member heard a sound.
	void Sound_Heard(GameObject* owner, const CombatSound& sound)
		{
		GroupController* controller = GroupController::Instance();
		assert(controller != NULL);

		GroupEventInfo info;
		info.GroupName = mGroupName;
		info.Event = GROUP_MEMBER_HEARD;
		info.Sound = &sound;

		Group* group = controller->FindGroup(mGroupName);
		assert(group != NULL);
		group->SendCustomEvent(owner, SCMD_GROUP_EVENT, (int)&info);
		}


	// Notify group that a member saw the enemy.
	void Enemy_Seen(GameObject* owner, GameObject* enemy)
		{
		GroupController* controller = GroupController::Instance();
		assert(controller != NULL);

		GroupEventInfo info;
		info.GroupName = mGroupName;
		info.Event = GROUP_MEMBER_SAW;
		info.Object = enemy;

		Group* group = controller->FindGroup(mGroupName);
		assert(group != NULL);
		group->SendCustomEvent(owner, SCMD_GROUP_EVENT, (int)&info);
		}


	#ifdef _DEBUG
	void Custom(GameObject* owner, int event, int data, GameObject* sender)
		{
		if (SCMD_GROUP_EVENT == event)
			{
			GroupEventInfo* info = (GroupEventInfo*)data;
			assert(info != NULL);

			int senderID = Commands->Get_ID(sender);
			int objectID = Commands->Get_ID(info->Object);

			switch (info->Event)
				{
				case GROUP_MEMBER_DAMAGED:
					DebugPrint("Group %s member %d damaged by object %d\n",
						info->GroupName, senderID, objectID);
				break;

				case GROUP_MEMBER_KILLED:
					DebugPrint("Group %s member %d killed by object %d\n",
						info->GroupName, senderID, objectID);
				break;

				case GROUP_MEMBER_HEARD:
					{
					const CombatSound* sound = info->Sound;
					objectID = Commands->Get_ID(sound->Creator);
					DebugPrint("Group %s member %d heard a sound from object %d\n",
						info->GroupName, senderID, objectID);
					}
				break;

				case GROUP_MEMBER_SAW:
					DebugPrint("Group %s member %d saw object %d\n",
						info->GroupName, senderID, objectID);
				break;

				default:
				break;
				}
			}
		}
	#endif
	};
