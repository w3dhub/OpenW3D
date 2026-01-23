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
 *                     $Archive:: /Commando/Code/wwphys/octbox.cpp                            $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 11/21/01 2:54p                                              $*
 *                                                                                             *
 *                    $Revision:: 20                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "octbox.h"
#include "pscene.h"
#include "physcoltest.h"
#include "physinttest.h"
#include "physcon.h"

#define SHOW_CONTACT_DETECTORS 0

const float DEFAULT_STIFFNESS = 5.0f;
const float DEFAULT_DAMPING = 0.1f;
const float	DEFAULT_THICKNESS = 0.1f;

const float CONTACT_GRAVITY_MULTIPLIER = 2.0f;	// maximum contact force is multiplier * gravity.
const float CONTACT_DAMPING_FACTOR = 0.33f;		// ratio of damping / (critical damping) (less than zero is underdamped)


OctBoxClass::OctBoxClass
(
	RigidBodyClass & parent,
	const OBBoxClass & box
) :
	Parent(parent),
	InnerBox(box),
	Thickness(DEFAULT_THICKNESS),
	Stiffness(DEFAULT_STIFFNESS),
	Damping(DEFAULT_DAMPING)
{
	Set_Thickness(Thickness);
}


OctBoxClass::~OctBoxClass(void)
{
}

void OctBoxClass::Set_Thickness(float thickness)
{
	Thickness = thickness;

	// Outer corner of the "octant boxes" should be at the corner of the box
	// Inner corner of the "octant boxes" should be overlapping the neighbor boxes by 'thickness'
	Vector3 octant_max = InnerBox.Extent;
	Vector3 octant_min(-Thickness,-Thickness,-Thickness);

	OctantExtent = (octant_max - octant_min) / 2.0f;
	OctantCenter = (octant_max + octant_min) / 2.0f;
}

void OctBoxClass::Update_Contact_Parameters(void)
{
	float mass = Parent.Get_Mass();
	
	// Make the contact force have a maximum of MULTIPLIER * gravity
	Stiffness = mass * WWMath::Fabs(PhysicsConstants::GravityAcceleration.Z);
	Stiffness *= CONTACT_GRAVITY_MULTIPLIER;

	// Compute critical damping, achieve equilibrium in minimum time.
	Damping  = 2.0f * mass * WWMath::Sqrt(Stiffness / mass);

	// Critical damping seems to be too stiff so reduce it
	Damping *= CONTACT_DAMPING_FACTOR;
}

void OctBoxClass::Get_Outer_Bounds(AABoxClass * set_bounds)
{
	WWASSERT(set_bounds != NULL);
	OBBoxClass wrld_outer_box;
	wrld_outer_box = WrldInnerBox;
	wrld_outer_box.Extent += Vector3(Thickness,Thickness,Thickness);

	set_bounds->Center = wrld_outer_box.Center;
	wrld_outer_box.Compute_Axis_Aligned_Extent(&(set_bounds->Extent));
}

bool OctBoxClass::Is_Intersecting(NonRefPhysListClass * result_list,bool check_static_objs,bool check_dyn_objs)
{
	[[maybe_unused]] PhysicsSceneClass * the_scene = PhysicsSceneClass::Get_Instance();
	WWASSERT(the_scene != NULL);

	/*
	** Test inner box for intersection
	*/
	PhysOBBoxIntersectionTestClass test(WrldInnerBox,
													Parent.Get_Collision_Group(),
													COLLISION_TYPE_PHYSICAL | COLLISION_TYPE_VEHICLE,
													result_list);

	test.CheckStaticObjs = check_static_objs;
	test.CheckDynamicObjs = check_dyn_objs;
	
	Parent.Inc_Ignore_Counter ();
	bool intersect = PhysicsSceneClass::Get_Instance ()->Intersection_Test(test);
	Parent.Dec_Ignore_Counter ();
	return intersect;
}

bool OctBoxClass::Is_In_Contact_Zone(void)
{
	PhysicsSceneClass * the_scene = PhysicsSceneClass::Get_Instance();
	WWASSERT(the_scene != NULL);

	/*
	** Test outer box for intersection
	*/
	OBBoxClass wrld_outer_box = WrldInnerBox;
	wrld_outer_box.Extent += Vector3(Thickness,Thickness,Thickness);

	int colgroup = Parent.Get_Collision_Group();
	int coltype = COLLISION_TYPE_PHYSICAL | COLLISION_TYPE_VEHICLE;
	return the_scene->Intersection_Test(wrld_outer_box,colgroup,coltype,true);
}


OctBoxClass::CollisionResult
OctBoxClass::Compute_Contacts(bool lock_to_centroids)
{
	Parent.Inc_Ignore_Counter();
	CollisionResult result = Internal_Compute_Contacts(lock_to_centroids);
	Parent.Dec_Ignore_Counter();
	return result;
}


OctBoxClass::CollisionResult			
OctBoxClass::Internal_Compute_Contacts(bool lock_to_centroids)
{
	/*
	** Algorithm:
	** Quick Rejection:
	** - Try to quick reject the entire collision by checking the outer box
	** - Check if we're going to have to search for TOC by checking the inner box
	** Find Contacts:
	** - For each octant (total of 8)
	**   - Sweep the octant along the diagonal and record the contact point (if any)
	*/
	PhysicsSceneClass * the_scene = PhysicsSceneClass::Get_Instance();
	WWASSERT(the_scene != NULL);
	Reset_Contacts();

	/*
	** compute the outer box in WS
	*/
	OBBoxClass wrld_outer_box;
	wrld_outer_box = WrldInnerBox;
	wrld_outer_box.Extent += Vector3(Thickness,Thickness,Thickness);

	/*
	** Check for quick-rejection (intersection or completely separated)
	*/
	int colgroup = Parent.Get_Collision_Group();
	int coltype = COLLISION_TYPE_PHYSICAL | COLLISION_TYPE_VEHICLE;

	if (!the_scene->Intersection_Test(wrld_outer_box,colgroup,coltype,true)) {
		//Parent.Add_Debug_OBBox(wrld_outer_box,Vector3(0,1,0));
		return RESULT_NO_COLLISION;
	}

	if (the_scene->Intersection_Test(WrldInnerBox,colgroup,coltype,true)) {
		//Parent.Add_Debug_OBBox(wrld_outer_box,Vector3(1,0,0));
		return RESULT_INTERSECTION;
	}
	
	/*
	** Check the octants for contact
	*/
	for (int i=0; i<8; i++) {
		Compute_Octant_Contact(i,lock_to_centroids);
	}

	return RESULT_CONTACT;
}


void OctBoxClass::Compute_Octant_Contact(int oi,bool /* lock_to_centroids */)
{
	static Vector3 _octant_offset[8] =
	{
		Vector3( 1, 1, 1),
		Vector3(-1, 1, 1),
		Vector3(-1,-1, 1),
		Vector3( 1,-1, 1),
		Vector3( 1, 1,-1),
		Vector3(-1, 1,-1),
		Vector3(-1,-1,-1),
		Vector3( 1,-1,-1),
	};

	/*
	** Compute the corner contact "hair" which doubles as the 
	** move vector for the octant box.
	*/
	CastResultStruct corner_result;
	corner_result.ComputeContactPoint = true;

	Vector3 corner = _octant_offset[oi];
	corner.X *= WrldInnerBox.Extent.X;
	corner.Y *= WrldInnerBox.Extent.Y;
	corner.Z *= WrldInnerBox.Extent.Z;
	Matrix3::Rotate_Vector(WrldInnerBox.Basis,corner,&corner);
	corner += WrldInnerBox.Center;

	Vector3 corner_move = Thickness * _octant_offset[oi];
	Matrix3::Rotate_Vector(WrldInnerBox.Basis,corner_move,&corner_move);
	
	/*
	** Collision detect for the corner line-segment 
	*/
	LineSegClass ray(corner,corner + corner_move);
	WWASSERT(corner_move.Length2() <= ((3*Thickness*Thickness) + 0.01f));
	WWASSERT(ray.Get_DP().Length2() <= ((3*Thickness*Thickness) + 0.01f));

	PhysRayCollisionTestClass raytest(	ray,
													&corner_result,
													Parent.Get_Collision_Group(),
													COLLISION_TYPE_PHYSICAL | COLLISION_TYPE_VEHICLE);
	PhysicsSceneClass::Get_Instance()->Cast_Ray(raytest,true);

#ifdef WWDEBUG
#if SHOW_CONTACT_DETECTORS
	Parent.Add_Debug_Vector(corner,corner_move,Vector3(0,1,0));
#endif
#endif

	/*
	** Compute the contact for this octant-box. 
	*/
	OBBoxClass octbox = WrldInnerBox;
	octbox.Extent = OctantExtent;

	Vector3 dc = _octant_offset[oi];
	dc.X *= OctantCenter.X;
	dc.Y *= OctantCenter.Y;
	dc.Z *= OctantCenter.Z;
	Matrix3::Rotate_Vector(WrldInnerBox.Basis,dc,&dc);
	octbox.Center += dc;

	/*
	** Collision detect with this octant box
	*/
	CastResultStruct octant_result;
	octant_result.ComputeContactPoint = true;
	PhysOBBoxCollisionTestClass boxtest(	octbox,
														corner_move,
														&octant_result,
														Parent.Get_Collision_Group(),
														COLLISION_TYPE_PHYSICAL | COLLISION_TYPE_VEHICLE);
	PhysicsSceneClass::Get_Instance()->Cast_OBBox(boxtest,true);
	
	/*
	** Wake up any vehicle in contact with us
	*/
	if ((boxtest.CollidedPhysObj != NULL) && (boxtest.CollidedPhysObj->As_RigidBodyClass() != NULL)) {
		boxtest.CollidedPhysObj->Force_Awake();
	}

	/*
	** Display the octant box
	*/
#ifdef WWDEBUG
#if SHOW_CONTACT_DETECTORS
	static Vector3 _oct_colors[8] = { 
		Vector3(1,0,0),Vector3(0,1,0),Vector3(0,0,1),Vector3(1,1,1),
		Vector3(1,1,1),Vector3(0,0,1),Vector3(0,1,0),Vector3(1,0,0)
	};
		
	octbox.Center += octant_result.Fraction * corner_move;
	//Parent.Add_Debug_OBBox(octbox,_oct_colors[oi]);
	//Parent.Add_Debug_Vector(result.Point,result.Normal,Vector3(0,1,0));
#endif
#endif

	/*
	** Now, decide which (if any) contact to use.  We prefer the
	** corners but have to use the box's contact if it is significantly 
	** closer.  If neither hits anything, just bail out of this routine.
	*/
	if ((corner_result.Fraction >= 1.0f) && (octant_result.Fraction >= 1.0f)) {
		return;
	}
	
	/*
	** If the corner is within 'CORNER_BIAS' of the result of the octant
	** then use its result
	*/
	const float CORNER_BIAS = 0.25f;
	if ((corner_result.Fraction < 1.0f) && (corner_result.Fraction - CORNER_BIAS < octant_result.Fraction)) {

		if (corner_result.Normal.Length2() > 0.0f) {					// the normal will be 0,0,0 when we hit a backside
			Add_Contact(corner_result,raytest.CollidedPhysObj);
		}

	} else {

		if ((octant_result.Fraction < 1.0f) && (!octant_result.StartBad)) {
			if (octant_result.Normal.Length2() > 0.0f) {				// the normal will be 0,0,0 when we hit a backside
				Add_Contact(octant_result,boxtest.CollidedPhysObj);
			} 
		}
		
		if (corner_result.Fraction < 1.0f) {
			// also add the corner contact!
			if (corner_result.Normal.Length2() > 0.0f) {				// the normal will be 0,0,0 when we hit a backside
				Add_Contact(corner_result,raytest.CollidedPhysObj);
			}
		}
	}
}




