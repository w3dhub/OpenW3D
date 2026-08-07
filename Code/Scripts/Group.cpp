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
*     Group.cpp
*
* DESCRIPTION
*     Group manager for GameObjects
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*
* VERSION INFO
*     $Author: Denzil_l $
*     $Revision: 1 $
*     $Modtime: 4/17/00 9:47a $
*     $Archive: /Commando/Code/Scripts/Group.cpp $
*
******************************************************************************/

#include "group.h"

/******************************************************************************
*
* NAME
*     Group::Group
*
* DESCRIPTION
*     Group Constructor
*
* INPUTS
*     Name - Name of team.
*
* RESULTS
*     NONE
*
******************************************************************************/

Group::Group(const char* name)
	{
	mName[0] = '\0';

	// Make copy of team name
	assert(name != nullptr);

	if (name != nullptr)
		{
		strncpy(mName, name, sizeof(mName));
		mName[sizeof(mName) - 1] = '\0';
		}
	}


/******************************************************************************
*
* NAME
*     Group::~Group
*
* DESCRIPTION
*     Group Destructor
*
* INPUTS
*     NONE
*
* RESULTS
*     NONE
*
******************************************************************************/

Group::~Group()
	{
	}


/******************************************************************************
*
* NAME
*     Group::GetName
*
* DESCRIPTION
*     Retrieve name of Group.
*
* INPUTS
*     NONE
*
* RESULTS
*     Name - Name of team
*
******************************************************************************/

const char* Group::GetName(void) const
	{
	return mName;
	}


/******************************************************************************
*
* NAME
*     Group::AddMember
*
* DESCRIPTION
*     Add a game object to the Group.
*
* INPUTS
*     GameObject - Pointer to game object to add.
*
* RESULTS
*     NONE
*
******************************************************************************/

void Group::AddMember(GameObject* object)
	{
	assert(object != nullptr);

	// Add the object if it is not already a member of the team.
	if (IsMember(object) == false)
		{
		mMembers.Add(object);
		}
	}


/******************************************************************************
*
* NAME
*     Group::RemoveMember
*
* DESCRIPTION
*     Remove a game object from the Group.
*
* INPUTS
*     GameObject - Pointer to game object to remove.
*
* RESULTS
*     NONE
*
******************************************************************************/

void Group::RemoveMember(GameObject* object)
	{
	assert(object != nullptr);
	mMembers.Delete(object);
	}


/******************************************************************************
*
* NAME
*     Group::IsMember
*
* DESCRIPTION
*     Check if a game object is member of the Group.
*
* INPUTS
*     GameObjectID objectID
*
* RESULTS
*     bool
*
******************************************************************************/

bool Group::IsMember(GameObject* object) const
	{
	assert(object != nullptr);

	for (int index = 0; index < mMembers.Count(); index++)
		{
		if (mMembers[index] == object)
			{
			return true;
			}
		}

	return false;
	}


/******************************************************************************
*
* NAME
*     MemberCount
*
* DESCRIPTION
*     Get the number of members in the Group.
*
* INPUTS
*     NONE
*
* RESULTS
*     Count - Member count
*
******************************************************************************/

int Group::MemberCount(void) const
	{
	return mMembers.Count();
	}


/******************************************************************************
*
* NAME
*     Group::GetMember
*
* DESCRIPTION
*     Get a Group member.
*
* INPUTS
*     int index
*
* RESULTS
*     GameObject*
*
******************************************************************************/

GameObject* Group::GetMember(int index)
	{
	assert(index >= 0);
	assert(index < MemberCount());

	return mMembers[index];
	}


/******************************************************************************
*
* NAME
*     Group::SendCommand
*
* DESCRIPTION
*     Send a command to all the members of the Group.
*
* INPUTS
*     Command - Command to send to Group members.
*     Data    - Command specific data parameter.
*
* RESULTS
*     NONE
*
******************************************************************************/

void Group::SendCustomEvent(GameObject* from, int event, int data)
	{
	for (int index = 0; index < mMembers.Count(); index++)
		{
		GameObject* object = mMembers[index];
		assert(object != nullptr);
		Commands->Send_Custom_Event(from, object, event, data);
		}
	}
