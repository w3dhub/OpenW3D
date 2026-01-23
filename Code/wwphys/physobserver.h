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
 *                 Project Name : WWPhys                                                       *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwphys/physobserver.h                        $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 1/08/02 5:51p                                               $*
 *                                                                                             *
 *                    $Revision:: 10                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef PHYSOBSERVER_H
#define PHYSOBSERVER_H

#include "always.h"

class	PhysClass;
class RenderObjClass;
struct CastResultStruct;

/*
** CollisionEventStruct
** This structure is simply used to package up the information pertaining to
** a collision.
*/
class CollisionEventClass
{
public:
	CollisionEventClass(void) : 
		OtherObj(NULL),
		CollisionResult(NULL),
		CollidedRenderObj(NULL)
	{}

	PhysClass *						OtherObj;				// set to the other object before given to you
	const CastResultStruct *	CollisionResult;		// actual collision data.
	RenderObjClass *				CollidedRenderObj;	// actual render object collided against (may be NULL!)
};


/*
** CollisionReactionType
** This is an enumeration of possible replies that an observer can make when
** it gets a collision event.
*/
enum _CollisionReactionType
{
	COLLISION_REACTION_DEFAULT,
	COLLISION_REACTION_STOP_MOTION,
	COLLISION_REACTION_NO_BOUNCE,
};

typedef int CollisionReactionType;

/*
** ExpirationReactionType
** This is an enumeration of the responses that the game object can give
** when the physics object it is observing is about to expire
*/
enum _ExpirationReactionType
{
	EXPIRATION_DENIED,
	EXPIRATION_APPROVED,
};

typedef int ExpirationReactionType;

/*
** PhysObserverClass
** This class defines the interface for an observer of a physics object
** Each physics object can have a single observer installed into it which
** will be notified when certain things occur.
*/
class PhysObserverClass
{
public:

	virtual CollisionReactionType		Collision_Occurred(const CollisionEventClass & /* event */) { return COLLISION_REACTION_DEFAULT; }
	virtual ExpirationReactionType	Object_Expired(PhysClass * /* observed_obj */)					{ return EXPIRATION_APPROVED; }
	virtual void							Object_Removed_From_Scene(PhysClass * /* observed_obj */)	{ };
	virtual void							Object_Shattered_Something(PhysClass * /* observed_obj */, PhysClass * /* shattered_obj */, int /* surface_type */) { };
};


#endif	// PHYSOBSERVER_H
