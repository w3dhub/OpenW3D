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
 *                     $Archive:: /Commando/Code/wwphys/octbox.h                              $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 11/21/01 2:48p                                              $*
 *                                                                                             *
 *                    $Revision:: 14                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef OCTBOX_H
#define OCTBOX_H

#include "always.h"
#include "obbox.h"
#include "rbody.h"


/**
** OctBoxClass
** This class encapsulates the collision detection algorithms used by Renegade's "rigid-body"
** physics.  It basically acts like rigid box with a contact zone around it.  Contacts are detected
** when geometry is within "thickness" distance of the rigid box.
*/
class OctBoxClass
{
public:

	OctBoxClass(RigidBodyClass & parent,const OBBoxClass & box);
	~OctBoxClass(void);

	void							Set_Stiffness(float stiffness)		{ Stiffness = stiffness; }
	float							Get_Stiffness(void) const				{ return Stiffness; }
	void							Set_Damping(float damping)				{ Damping = damping; }
	float							Get_Damping(void) const					{ return Damping; }
	void							Set_Thickness(float thickness);
	float							Get_Thickness(void)						{ return Thickness; }

	void							Update_Contact_Parameters(void);

	enum CollisionResult
	{
		RESULT_NO_COLLISION	= 0,		// this object isn't touching anything
		RESULT_INTERSECTION,				// this object has penetrated another, have to back up
		RESULT_CONTACT,					// no penetration but there is contact.
	};

	void							Set_Transform(const Matrix3D & tm);
	void							Set_Transform(const Quaternion & q,const Vector3 & p);

	bool							Is_Intersecting(NonRefPhysListClass * result_list = NULL,bool check_static_objs = true,bool check_dyn_objs = true);
	bool							Is_In_Contact_Zone(void);
	CollisionResult			Compute_Contacts(bool lock_to_centroids = false);

	int							Get_Contact_Count(void);
	const CastResultStruct &Get_Contact(int index);
	PhysClass *					Peek_Contacted_Object(int index);

	void							Get_Outer_Bounds(AABoxClass * set_bounds);
	void							Get_Outer_Box(OBBoxClass * set_box)	{ *set_box = Get_World_Inner_Box(); }
	void							Get_Inner_Box(OBBoxClass * set_box) { *set_box = Get_World_Inner_Box(); set_box->Extent += Vector3(Thickness,Thickness,Thickness); }
	void							Get_Inner_Box(OBBoxClass * set_box, const Quaternion & q, const Vector3 & p);
	float							Get_Extent_Length2()						{ return InnerBox.Extent.Length2(); }

public: //protected:

	CollisionResult			Internal_Compute_Contacts(bool lock_to_centroids);
	void							Compute_Octant_Contact(int oi,bool lock_to_centroids);

	void							Transform_To_World_Space(const Matrix3D & tm);
	void							Transform_To_World_Space(const Quaternion & q,const Vector3 & p);

	void							Reset_Contacts(void);
	void							Add_Contact(const CastResultStruct & res,PhysClass * other_obj);

	const OBBoxClass &		Get_World_Inner_Box(void) { return WrldInnerBox; }

	/*
	** Properties
	*/
	RigidBodyClass &			Parent;					// parent object this OctBox is working for
	OBBoxClass					InnerBox;				// OBBox for the parent when parent's TM is I
	float							Thickness;				// thickness of the "squishy zone"
	float							Stiffness;				// spring stiffness to use
	float							Damping;					// damping coefficient to use

	Vector3						OctantCenter;			// offest to center of octant0 in InnerBox's coord system
	Vector3						OctantExtent;			// extent of the octant boxes

	/*
	** Current State
	*/
	OBBoxClass					WrldInnerBox;

	/*
	** Contact Accumulation
	*/
	enum { MAX_CONTACTS = 16 };
	int							ContactCount;
	CastResultStruct			Contacts[MAX_CONTACTS];
	PhysClass *					ContactedObject[MAX_CONTACTS];
};

inline void OctBoxClass::Set_Transform(const Matrix3D & tm)
{
	Transform_To_World_Space(tm);
}

inline void OctBoxClass::Set_Transform(const Quaternion & q,const Vector3 & p)
{
	Transform_To_World_Space(q,p);
}

inline int OctBoxClass::Get_Contact_Count(void)
{
	return ContactCount;
}

inline const CastResultStruct & OctBoxClass::Get_Contact(int index)
{
	static CastResultStruct _no_result;

	if ((index >= 0) && (index < ContactCount)) {
		return Contacts[index];
	} else {
		return _no_result;
	}
}

inline PhysClass * OctBoxClass::Peek_Contacted_Object(int index)
{
	if ((index >= 0) && (index < ContactCount)) {
		return ContactedObject[index];
	} else {
		return NULL;
	}
}

inline void OctBoxClass::Transform_To_World_Space(const Matrix3D & tm)
{
	OBBoxClass::Transform(tm,InnerBox,&WrldInnerBox);
}

inline void OctBoxClass::Transform_To_World_Space(const Quaternion & q,const Vector3 & p)
{
	Matrix3D tm(q,p);
	OBBoxClass::Transform(tm,InnerBox,&WrldInnerBox);
}

inline void OctBoxClass::Reset_Contacts(void)
{
	ContactCount = 0;
}

inline void	OctBoxClass::Add_Contact(const CastResultStruct & res,PhysClass * other_obj)
{
	WWASSERT(ContactCount < MAX_CONTACTS);
	Contacts[ContactCount] = res;
	ContactedObject[ContactCount] = other_obj;
	ContactCount++;
}

inline void OctBoxClass::Get_Inner_Box(OBBoxClass * set_box, const Quaternion & q, const Vector3 & p)
{
	Matrix3D tm(q,p);
	OBBoxClass::Transform(tm,InnerBox,set_box);
}

#endif
