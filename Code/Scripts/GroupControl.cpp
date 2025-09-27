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
*     GroupControl.cpp
*
* DESCRIPTION
*     Group controller
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*
* VERSION INFO
*     $Author: Pat_p $
*     $Revision: 2 $
*     $Modtime: 4/26/00 10:31a $
*     $Archive: /Commando/Code/Scripts/GroupControl.cpp $
*
******************************************************************************/

#include "groupcontrol.h"
#include "group.h"
#include "scripts.h"
#include <assert.h>

GroupController _GroupController;
GroupController* GroupController::_mInstance = NULL;

/******************************************************************************
*
* NAME
*     GroupController::Instance
*
* DESCRIPTION
*     Retrieve GroupController instance.
*
* INPUTS
*     NONE
*
* RESULTS
*     GroupController - GroupController instance pointer.
*
******************************************************************************/

GroupController* GroupController::Instance(void)
	{
	assert(_mInstance != NULL);
	return _mInstance;
	}


/******************************************************************************
*
* NAME
*     GroupController::GroupController
*
* DESCRIPTION
*     Construct GroupController
*
* INPUTS
*     NONE
*
* RESULTS
*     NONE
*
******************************************************************************/

GroupController::GroupController()
	{
	_mInstance = this;
	}


/******************************************************************************
*
* NAME
*     GroupController::~GroupController
*
* DESCRIPTION
*     Destruct GroupController
*
* INPUTS
*     NONE
*
* RESULTS
*     NONE
*
******************************************************************************/

GroupController::~GroupController()
	{
	mGroups.Remove_All();
	_mInstance = NULL;
	}


/******************************************************************************
*
* NAME
*     GroupController::AddToGroup
*
* DESCRIPTION
*     Add a game object to the specified Group.
*
* INPUTS
*     GroupName   - Name of the team to object to.
*     GameObject - Object to add.
*
* RESULTS
*     Success - Success / Failure condition
*
******************************************************************************/

bool GroupController::AddToGroup(const char* groupName, GameObject* object)
	{
	assert(groupName != NULL);
	assert(object != NULL);

	if ((groupName != NULL) && (object != NULL))
		{
		// Find the team to add the GameObject to.
		Group* group = FindOrCreateGroup(groupName);

		if (group != NULL)
			{
			group->AddMember(object);
			return true;
			}
		}

	return false;
	}


/******************************************************************************
*
* NAME
*     GroupController::RemoveFromGroup
*
* DESCRIPTION
*     Remove a game object from the specified Group.
*
* INPUTS
*     GroupName   - Name of group to remove object from.
*     GameObject - Object to remove.
*
* RESULTS
*     NONE
*
******************************************************************************/

void GroupController::RemoveFromGroup(const char* groupName, GameObject* object)
	{
	assert(groupName != NULL);
	assert(object != NULL);

	Group* group = FindGroup(groupName);

	if (group != NULL)
		{
		group->RemoveMember(object);
		}
	}


/******************************************************************************
*
* NAME
*     GroupController::FindGroup
*
* DESCRIPTION
*     Find a group by name,
*
* INPUTS
*     GroupName - Name of group to find.
*
* RESULTS
*     Group - Pointer to Group; NULL if not found.
*
******************************************************************************/

Group* GroupController::FindGroup(const char* groupName)
	{
	SLNode<Group>* node = mGroups.Head();

	// Go through all the Groups
	while (node != NULL)
		{
		Group* group = node->Data();
		assert(group != NULL);

		// If there is a group with a matching name then return with that group.
		const char* name = group->GetName();

		if (stricmp(groupName, name) == 0)
			{
			return group;
			}

		node = node->Next();
		}

	return NULL;
	}


/******************************************************************************
*
* NAME
*     GroupController::FindOrCreateGroup
*
* DESCRIPTION
*     Create a new group with the specified name. If the group already exists
*     then the existing group will be returned.
*
* INPUTS
*     GroupName - Name of group to find or create.
*
* RESULTS
*     Group - Pointer to Group; NULL if failure to create group.
*
******************************************************************************/

Group* GroupController::FindOrCreateGroup(const char* groupName)
	{
	Group* group = FindGroup(groupName);

	// Create the group only if it doesn't already exist.
	if (group == NULL)
		{
		Group* group = new Group(groupName);
		assert(group != NULL);

		if (group != NULL)
			{
			mGroups.Add_Tail(group);
			}
		}

	return group;
	}


/******************************************************************************
*
* NAME
*     GroupController::SendGroupCustomEvent
*
* DESCRIPTION
*     Send a Group a custom event.
*
* INPUTS
*     GroupName  - Name of group to send event to.
*     GameObject - Object event is from.
*     Event      - Event to send to Group.
*     Data       - Event specific data parameter.
*
* RESULTS
*     NONE
*
******************************************************************************/

void GroupController::SendGroupCustomEvent(const char* groupName, GameObject* from,
		int event, int data)
	{
	Group* group = FindGroup(groupName);

	if (group != NULL)
		{
		group->SendCustomEvent(from, event, data);
		}
	}
