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
 *                 Project Name : LevelEdit                                                    *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwphys/PathfindSectorBuilder.cpp                                                                                                                                   $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "stdafx.h"
#include "pathfindsectorbuilder.h"
#include "phys3.h"
#include "filemgr.h"
#include "editorassetmgr.h"
#include "_assetmgr.h"
#include "humanphys.h"
#include "Utils.h"
#include "editorphys.h"
#include "collisiongroups.h"
#include "sceneeditor.h"
#include "boxrobj.h"
#include "Pathfind.h"
#include "systimer.h"
#include <algorithm>

//////////////////////////////////////////////////////////////////////////
//	Static member initialization
//////////////////////////////////////////////////////////////////////////
BodyBoxCullObj *BodyBoxCullObj::m_First = NULL;

//////////////////////////////////////////////////////////////////////////
//	Local constants
//////////////////////////////////////////////////////////////////////////

class SimDirInfoClass
{
	public:

		SimDirInfoClass (void) { }
		SimDirInfoClass (float _heading, float _speed, const Vector3 &_move, const Vector3 &min, const Vector3 &max)
			: heading (_heading), speed (_speed), move (_move), min_move (min), max_move (max)	{ }

		float		heading;
		float		speed;
		Vector3	move;
		Vector3	min_move;
		Vector3	max_move;
};

/*const SIM_DIR_INFO DIR_INFO[DIR_MAX] =
{
	SIM_DIR_INFO ( 0.0F,						 	 Vector3 ( 0.9F, -0.1F, -1.5F), Vector3 ( 2.0F,  0.1F, 1.5F) ),
	SIM_DIR_INFO ( (float)DEG_TO_RAD (90),	 Vector3 (-0.1F,  0.9F, -1.5F), Vector3 ( 0.1F,  2.0F, 1.5F) ),
	SIM_DIR_INFO ( (float)DEG_TO_RAD (180), Vector3 (-2.0F, -0.1F, -1.5F), Vector3 (-0.9F,  0.1F, 1.5F) ),
	SIM_DIR_INFO ( (float)DEG_TO_RAD (270), Vector3 (-0.1F, -2.0F, -1.5F), Vector3 ( 0.1F, -0.9F, 1.5F) )
};*/


//
//	Note:  The vector members are in "percentage of bounding box" units.
//
static const SimDirInfoClass DIR_INFO[DIR_MAX] =
{
	SimDirInfoClass ( 0.0F,						 	 0, Vector3 ( 1.0F,  0,    0), Vector3 ( 0.95F, -0.05F, -2.0F), Vector3 (  1.2F,  0.05F, 2.0F) ),
	SimDirInfoClass ( (float)DEG_TO_RAD (90),	 0, Vector3 ( 0,     1.0F, 0), Vector3 (-0.05F,  0.95F, -2.0F), Vector3 ( 0.05F,   1.2F, 2.0F) ),
	SimDirInfoClass ( (float)DEG_TO_RAD (180), 0, Vector3 (-1.0F,  0,    0), Vector3 ( -1.2F, -0.05F, -2.0F), Vector3 (-0.95F,  0.05F, 2.0F) ),
	SimDirInfoClass ( (float)DEG_TO_RAD (270), 0, Vector3 ( 0,    -1.0F, 0), Vector3 (-0.05F,  -1.2F, -2.0F), Vector3 ( 0.05F, -0.95F, 2.0F) )
};


//////////////////////////////////////////////////////////////////////////
//
//	PathfindSectorBuilderClass
//
//////////////////////////////////////////////////////////////////////////
PathfindSectorBuilderClass::PathfindSectorBuilderClass (void)
	:	m_PhysicsSim (NULL),
		m_RecurseLevel (0),
		m_RepartitionCount (0),
		m_CurrentSector (NULL),
		m_SimBoundingBox (Vector3 (0.5F, 0.5F, 1.8F)),
		m_DirInfo (NULL),
		m_bCancel (false)
{
	//
	//	Create an instance of the commando render object we can
	// use to simulate character movement
	//
	CStringA full_path;
	full_path = Get_File_Mgr ()->Make_Full_Path ("Characters/Commando");
	_pThe3DAssetManager->Set_Current_Directory (full_path);

	RenderObjClass *commando_obj = WW3DAssetManager::Get_Instance()->Create_Render_Obj ("COMMANDO");
	if (commando_obj != NULL) {

		//
		//	Attempt to find the collision box for this physics object
		//
		RenderObjClass *collision_box = NULL;
		if (commando_obj->Class_ID () == RenderObjClass::CLASSID_DISTLOD) {
			RenderObjClass *lod0	= commando_obj->Get_Sub_Object(0);
			collision_box			= lod0->Get_Sub_Object_By_Name ("WORLDBOX");
			MEMBER_RELEASE (lod0);
		} else {
			collision_box			= commando_obj->Get_Sub_Object_By_Name ("WORLDBOX");
		}

		//
		//	Store the exents of the collision box for use in our simulation
		//
		if (collision_box != NULL) {
			const AABoxClass &box = collision_box->Get_Bounding_Box ();
			m_SimBoundingBox = box.Extent * 2;
			MEMBER_RELEASE (collision_box);
		}

		//
		//	Build an array of simulation information (specific to this
		// physics object).
		//
		m_DirInfo = new SimDirInfoClass[DIR_MAX];
		for (int index = 0; index < DIR_MAX; index ++) {
			m_DirInfo[index]					= DIR_INFO[index];
			m_DirInfo[index].move.X			*= m_SimBoundingBox.X;
			m_DirInfo[index].move.Y			*= m_SimBoundingBox.Y;
			m_DirInfo[index].move.Z			*= m_SimBoundingBox.Z;
			m_DirInfo[index].min_move.X	*= m_SimBoundingBox.X;
			m_DirInfo[index].min_move.Y	*= m_SimBoundingBox.Y;
			m_DirInfo[index].min_move.Z	*= m_SimBoundingBox.Z;
			m_DirInfo[index].max_move.X	*= m_SimBoundingBox.X;
			m_DirInfo[index].max_move.Y	*= m_SimBoundingBox.Y;
			m_DirInfo[index].max_move.Z	*= m_SimBoundingBox.Z;
			m_DirInfo[index].speed			= m_DirInfo[index].move.Length ();
		}

		//
		//	Create the physics object we will use for character simulation
		//
		m_PhysicsSim = new HumanPhysClass;
		m_PhysicsSim->Set_Model (commando_obj);
		m_PhysicsSim->Set_Controller (&m_Controller);
		m_PhysicsSim->Set_Collision_Group (0);
		MEMBER_RELEASE (commando_obj);

		//
		//	Setup the culling system for the boxes.
		//
		float max_extent	= std::max (m_SimBoundingBox.X, m_SimBoundingBox.Y);
		max_extent			= std::max (m_SimBoundingBox.Z, max_extent);
		Vector3 min;
		Vector3 max;
		PhysicsSceneClass::Get_Instance ()->Get_Level_Extents (min, max);
		m_BodyBoxCullingSystem.Re_Partition (min, max, max_extent * 5);
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//	~PathfindSectorBuilderClass
//
//////////////////////////////////////////////////////////////////////////
PathfindSectorBuilderClass::~PathfindSectorBuilderClass (void)
{
	//MEMBER_RELEASE (m_PhysicsSim);
	Free_Sectors ();
	SAFE_DELETE_ARRAY (m_DirInfo);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//	Generate_Sectors
//
//////////////////////////////////////////////////////////////////////////
void
PathfindSectorBuilderClass::Generate_Sectors (const Vector3 &start_pos)
{
	bool sectors_displayed = PathfindClass::Get_Instance ()->Are_Sectors_Displayed ();
	bool portals_displayed = PathfindClass::Get_Instance ()->Are_Portals_Displayed ();
	PathfindClass::Get_Instance ()->Display_Sectors (false);
	PathfindClass::Get_Instance ()->Display_Portals (false);

	//
	// Start fresh
	//
	Free_Sectors ();
	PathfindClass::Get_Instance ()->Reset_Sectors ();
	Import_Raw_Data ();
	m_RecurseLevel			= 0;
	m_RepartitionCount	= 0;
	m_bCancel				= false;

	//
	//	Normalize the starting position.  We do this so that we can start as many
	// different 'generations' in the world and they will all match up perfectly.
	//
	Vector3 normalized_start_pos = start_pos;
	normalized_start_pos.X = (int(start_pos.X / m_SimBoundingBox.X)) * m_SimBoundingBox.X;
	normalized_start_pos.Y = (int(start_pos.Y / m_SimBoundingBox.Y)) * m_SimBoundingBox.Y;

	//
	//	Start flood filling from the given position
	//
	m_CurrentSector = Mark_Sector (normalized_start_pos);
	Floodfill (normalized_start_pos);

	DWORD start_ticks = TIMEGETTIME ();

	//
	//	Process all the sectors in our queue
	//
	while (m_FloodFillProcessList.Count () > 0) {
		m_CurrentSector = m_FloodFillProcessList[0];
		m_FloodFillProcessList.Delete (0);
		Floodfill (m_CurrentSector->Get_Transform ().Get_Translation ());
		m_bCancel = ((TIMEGETTIME () - start_ticks) >  20000000L);
	}

	//
	//	Compress the sectors into the largest possible rectangular regions
	//
	DWORD before_ticks = TIMEGETTIME ();
	Compress_Sectors ();
	DWORD after_ticks = TIMEGETTIME ();

	CStringA message;
	message.Format ("Time spent compressing: %d secs.\r\n", (after_ticks-before_ticks)/1000);
	::Ouput_Message (message);

	//
	//	Add the sectors to our global pathfind object
	//
	for (int index = 0; index < m_SectorList.Count (); index ++) {
		PathfindSectorClass *sector = m_SectorList[index];
		if (sector->Is_Valid ()) {
			PathfindClass::Get_Instance ()->Add_Sector (sector);
		}
	}

	//
	//	Turn the debug display back on if necessary
	//
	PathfindClass::Get_Instance ()->Display_Sectors (sectors_displayed);
	PathfindClass::Get_Instance ()->Display_Portals (portals_displayed);
	return ;
}


float Re_Orient_Vector (const Vector3 &pos, const Vector3 &look_at)
{
	float dx = (look_at[0] - pos[0]);
	float dy = (look_at[1] - pos[1]);
	float len = (float)sqrt(dx*dx + dy*dy);

	float angle = 0;

	if (fabs (dx) >= fabs(dy)) {
		double cosy = 0;
		if (len != 0.0f) {
			cosy = dx/len;
		} else {
			cosy = 1.0f;
		}

		angle = ::acos (cosy);
	} else {
		double siny = 0;
		if (len != 0.0f) {
			siny = dy/len;
		} else {
			siny = 0.0f;
		}

		angle = ::asin (siny);
	}

	return angle;
}


//////////////////////////////////////////////////////////////////////////
//
//	Do_Physics_Sim
//
//////////////////////////////////////////////////////////////////////////
void
PathfindSectorBuilderClass::Do_Physics_Sim
(
	const Vector3 &	start_pos,
	PATHFIND_DIR		direction
)
{
	//
	//	Position the physics object at the test location
	//
	Matrix3D transform (start_pos);
	m_PhysicsSim->Set_Transform (transform);

	//
	//	Determine how far the simulation can move in each of the 3 directions
	// and still be considered 'valid'.
	//
	Vector3 expected_move			= m_DirInfo[direction].move;
	Vector3 min_exceptable_move	= m_DirInfo[direction].min_move;
	Vector3 max_exceptable_move	= m_DirInfo[direction].max_move;

	//
	//	Set-up the simulation to move one 'step' in the given direction
	//
	m_Controller.Reset ();
	m_Controller.Set_Move_Forward  (1);
	m_PhysicsSim->Set_Normalized_Speed (m_DirInfo[direction].speed);
	m_PhysicsSim->Set_Heading (m_DirInfo[direction].heading);

	Vector3 expected_pos = start_pos + expected_move;


	//
	//	Do the simulation
	//
	Vector3 new_pos;
	Vector3 move_vector;
	bool found					= false;
	bool on_ground				= false;

	#if 0
		bool should_continue		= true;
		for (int attempt = 0; (attempt < 50) && should_continue; attempt ++) {

			//
			//	Break the physics timestep down into small steps (however
			// make sure it processes for exactly 1 sec).
			//
			for (int index = 0; index < 2; index ++) {
				m_PhysicsSim->Timestep (0.05F);
			}

			new_pos		= m_PhysicsSim->Get_Position ();
			move_vector	= new_pos - start_pos;

			//
			//	Did we move into the expected range?
			//
			if ((move_vector.X >= min_exceptable_move.X) &&
				 (move_vector.Y >= min_exceptable_move.Y) &&
				 (move_vector.Z >= min_exceptable_move.Z) &&
				 (move_vector.X <= max_exceptable_move.X) &&
				 (move_vector.Y <= max_exceptable_move.Y) &&
				 (move_vector.Z <= max_exceptable_move.Z))
			{
				should_continue = false;
				found = true;
			} else {
				float angle = Re_Orient_Vector (new_pos, expected_pos);
				m_PhysicsSim->Set_Heading (angle);
			}
		}
		on_ground = m_PhysicsSim->Is_In_Contact ();
	#else

		float half_height = m_SimBoundingBox.Z * 0.5F;
		float movement = (4.9F + (m_SimBoundingBox.Z * 0.15F));

		//m_PhysicsSim->Set_Transform (Matrix3D (expected_pos));
		AABoxClass box;
		box.Center		= expected_pos;
		box.Center.Z	+= (m_SimBoundingBox.Z * 0.75F);
		box.Extent.X	= m_SimBoundingBox.X * 0.5F;
		box.Extent.Y	= m_SimBoundingBox.Y * 0.5F;
		box.Extent.Z	= half_height;

		CastResultStruct result;
		PhysAABoxCollisionTestClass test(	box,
														Vector3 (0, 0, -movement),
														&result,
														m_PhysicsSim->Get_Collision_Group (),
														COLLISION_TYPE_PHYSICAL);

		PhysicsSceneClass::Get_Instance ()->Cast_AABox (test);

		found = (result.StartBad == false) && (result.Normal.Z > 0.7F);
		if (found) {
			on_ground = (result.Fraction < 1.0F);
			new_pos.Z = box.Center.Z - half_height - (movement * result.Fraction) + 0.001F;

		} else {
			int test = 0;
		}

	#endif

	if (found && on_ground) {

		//
		//	Force the x and y values to snap to the expected position
		//
		new_pos.X = start_pos.X + expected_move.X;
		new_pos.Y = start_pos.Y + expected_move.Y;

		//
		//	Is this cell already marked?
		//
		BodyBoxCullObj *occupant = Get_Sector_Occupant (new_pos);
		if (occupant == NULL) {

			//
			//	Mark this cell and add it to the list of sectors
			// that need to be recursed.
			//
			occupant = Mark_Sector (new_pos);
			m_FloodFillProcessList.Add (occupant);
		}

		//
		//	Let the sector know who its new neighbor is
		//
		if (occupant != m_CurrentSector) {
			m_CurrentSector->Set_Neighbor (direction, occupant);
		}
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//	Floodfill
//
//////////////////////////////////////////////////////////////////////////
void
PathfindSectorBuilderClass::Floodfill (const Vector3 &start_pos)
{
	//
	//	Do a simulation in each of the four directions from the
	// current cell.
	//
	if (m_bCancel == false) {
		Do_Physics_Sim (start_pos, DIR_FORWARD);
		Do_Physics_Sim (start_pos, DIR_LEFT);
		Do_Physics_Sim (start_pos, DIR_BACKWARD);
		Do_Physics_Sim (start_pos, DIR_RIGHT);

		//
		//	Let the current sector 'know' its been completely processed
		//
		m_CurrentSector->Needs_Processing (false);
	}

	//
	//	Allow the user to cancel this operation
	//
	if (!m_bCancel && (::GetAsyncKeyState (VK_ESCAPE) < 0)) {
		m_bCancel = true;
		::MessageBoxA (NULL, "Operation cancelled.", "Pathfind", MB_OK | MB_SETFOREGROUND);
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//	Get_Sector_Occupant
//
//////////////////////////////////////////////////////////////////////////
BodyBoxCullObj *
PathfindSectorBuilderClass::Get_Sector_Occupant (const Vector3 &pos)
{
	Vector3 box_center_pos	= pos;
	//box_center_pos.Z			+= (m_SimBoundingBox.Z * 0.1F);

	//
	//	Collect all the objects in our culling system that occupy the
	// given point
	//
	m_BodyBoxCullingSystem.Reset_Collection ();
	m_BodyBoxCullingSystem.Collect_Objects (box_center_pos);
	BodyBoxCullObj *body_box = m_BodyBoxCullingSystem.Get_First_Collected_Object ();

	if (body_box == NULL) {

		box_center_pos		= pos;
		box_center_pos.Z	+= (m_SimBoundingBox.Z * 0.25F);

		m_BodyBoxCullingSystem.Reset_Collection ();
		m_BodyBoxCullingSystem.Collect_Objects (box_center_pos);
		body_box = m_BodyBoxCullingSystem.Get_First_Collected_Object ();
	}

	if (body_box == NULL) {

		box_center_pos		= pos;
		box_center_pos.Z	+= (m_SimBoundingBox.Z * 0.5F);

		m_BodyBoxCullingSystem.Reset_Collection ();
		m_BodyBoxCullingSystem.Collect_Objects (box_center_pos);
		body_box = m_BodyBoxCullingSystem.Get_First_Collected_Object ();
	}

	//
	//	If we found any objects in the culling system, then the sector is occupied
	//
	return body_box;
}


//////////////////////////////////////////////////////////////////////////
//
//	Mark_Sector
//
//////////////////////////////////////////////////////////////////////////
void
PathfindSectorBuilderClass::Mark_Sector (BodyBoxCullObj *body_box)
{
	//
	//	Add the culling object  to our AABTree culling system.  (This
	// effectively marks the cell as 'occupied').
	//
	body_box->Add_Ref ();
	m_BodyBoxCullingSystem.Add_Object (body_box);
	BodyBoxCullObj::Add (body_box);

	//
	//	Repartition the culling system every few hundred
	// entries.
	//
	/*m_RepartitionCount ++;
	if (m_RepartitionCount > 1000) {
		m_RepartitionCount = 0;
		m_BodyBoxCullingSystem.Re_Partition ();
	}*/

	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//	Mark_Sector
//
//////////////////////////////////////////////////////////////////////////
BodyBoxCullObj *
PathfindSectorBuilderClass::Mark_Sector (const Vector3 &pos)
{
	//
	//	Create a culling system object to represent this 'sector' and
	// add it to our AABTree culling system.  (This effectively marks
	// the cell as 'occupied').
	//
	BodyBoxCullObj *body_box = new BodyBoxCullObj;
	AABoxClass box;
	box.Center		= pos;
	box.Extent.X	= m_SimBoundingBox.X * 0.5F;
	box.Extent.Y	= m_SimBoundingBox.Y * 0.5F;
	box.Extent.Z	= m_SimBoundingBox.Z * 0.5F;
	body_box->Set_Bounding_Box (box);
	body_box->Init_Transform (pos);

	Mark_Sector (body_box);
	body_box->Release_Ref ();
	return body_box;
}


//////////////////////////////////////////////////////////////////////////
//
//	Compress_Sectors
//
//////////////////////////////////////////////////////////////////////////
void
PathfindSectorBuilderClass::Compress_Sectors (void)
{
	BODY_BOX_LIST body_box_list;
	int sectors = 0;

	int largest_area						= 0;
	int largest_area_right				= 0;
	int largest_area_down				= 0;
	BodyBoxCullObj *upper_left_ptr	= NULL;

	//
	//	Keep looping until we've compressed all the body boxes into sectors
	//
	while (BodyBoxCullObj::Get_First () != NULL) {

		//
		//	Find the largest possible rectangular area that's 'free'
		// in the world.
		//
		largest_area = 0;
		for (	BodyBoxCullObj *cull_obj = BodyBoxCullObj::Get_First ();
				cull_obj != NULL;
				cull_obj = cull_obj->Get_Next ())
		{
			int max_down	= 0;
			int max_right	= 0;
			int max_area	= 0;

			int curr_down	= 65536;
			int curr_right = 1;

			//
			//	Find the largest rectangular area that this box is the upper-left
			// corner for.
			//
			BodyBoxCullObj *neighbor	= cull_obj;
			Vector3 pos						= cull_obj->Get_Position ();
			float min_z_pos				= pos.Z - m_SimBoundingBox.Z;
			float max_z_pos				= pos.Z + (m_SimBoundingBox.Z * 1.5F);
			while (neighbor != NULL)
			{
				curr_down = std::min (neighbor->Get_Count (DIR_BACKWARD, min_z_pos, max_z_pos) + 1, curr_down);

				//
				//	Determine if this region (right x down) contains the
				// largest area so far.
				//
				int area = (curr_down * curr_right);
				if (area > max_area) {
					max_area		= area;
					max_down		= curr_down;
					max_right	= curr_right;
				}

				//
				//	Move to the next cell to the right
				//
				curr_right ++;
				neighbor = neighbor->Next_Valid (DIR_RIGHT, min_z_pos, max_z_pos);
			}

			if (max_area > largest_area) {
				upper_left_ptr			= cull_obj;
				largest_area			= max_area;
				largest_area_right	= max_right;
				largest_area_down		= max_down;
			}
		}

		//
		//	Debug info
		//
		if (largest_area > 1) {

			CStringA message;
			message.Format ("Combining cells: width = %d, height = %d, area = %d\r\n", largest_area_right, largest_area_down, largest_area);
			::Ouput_Message (message);

			sectors ++;
		}

		Vector3 min_point (100000, 100000, 100000);
		Vector3 max_point (-100000, -100000, -100000);

		PathfindSectorClass *sector = new PathfindSectorClass;

		//
		//	Remove all the cells that compose this area from the list
		//
		BodyBoxCullObj *curr_cell = upper_left_ptr;
		for (int right = 0; (right < largest_area_right) && (curr_cell != NULL); right ++) {

			BodyBoxCullObj *down_obj = curr_cell;
			for (int down = 0; (down < largest_area_down) && (down_obj != NULL); down ++) {

				Matrix3D transform	= down_obj->Get_Transform ();
				Vector3 position		= transform.Get_Translation ();

				min_point.X = std::min (min_point.X, position.X - (m_SimBoundingBox.X / 2));
				min_point.Y = std::min (min_point.Y, position.Y - (m_SimBoundingBox.Y / 2));
				min_point.Z = std::min (min_point.Z, position.Z);

				max_point.X = std::max (max_point.X, position.X + (m_SimBoundingBox.X / 2));
				max_point.Y = std::max (max_point.Y, position.Y + (m_SimBoundingBox.Y / 2));
				max_point.Z = std::max (max_point.Z, position.Z + m_SimBoundingBox.Z);

				ASSERT (down_obj->Is_Taken () == false);
				down_obj->Set_Sector (sector);

				body_box_list.Add (down_obj);
				down_obj->Remove ();
				down_obj->Set_Taken ();
				down_obj = down_obj->Peek_Neighbor (DIR_BACKWARD);
			}

			curr_cell = curr_cell->Peek_Neighbor (DIR_RIGHT);
		}

		//
		//	Sumbit this sector
		//
		if (largest_area > 1) {

			AABoxClass bounding_box;
			bounding_box.Center = min_point + ((max_point - min_point) / 2.0F);

			bounding_box.Extent.X = (max_point.X - min_point.X) / 2;
			bounding_box.Extent.Y = (max_point.Y - min_point.Y) / 2;
			bounding_box.Extent.Z = (max_point.Z - min_point.Z) / 2;

			sector->Set_Bounding_Box (bounding_box);
			m_SectorList.Add (sector);
		} else {
			sector->Set_Valid (false);
			m_SectorList.Add (sector);
		}
	}

	//
	//	Build portals
	//
	for (int index = 0; index < body_box_list.Count (); index ++) {
		BodyBoxCullObj *cull_obj = body_box_list[index];

		PathfindSectorClass *sector = cull_obj->Peek_Sector ();
		if (sector->Is_Valid ()) {

			//
			//	Make portals out of this box for any of the four directions
			//
			for (int dir = 0; dir < DIR_MAX; dir ++) {

				if (cull_obj->Is_New_Portal ((PATHFIND_DIR)dir)) {
					PathfindPortalClass *portal = cull_obj->Make_Portal ((PATHFIND_DIR)dir, m_SimBoundingBox);
					sector->Add_Portal (portal);
				}
			}
		}
	}

	//
	//	Free the body box culling objects
	//
	for (index = 0; index < body_box_list.Count (); index ++) {

		//
		//	Remove the body box from the culling system
		//
		BodyBoxCullObj *cull_obj = body_box_list[index];
		m_BodyBoxCullingSystem.Remove_Object (cull_obj);

		//
		//	Save the body box in the global list so we can combine
		// generated sections later (if we want).  Also usefull
		// for debugging purposes.
		//
		cull_obj->Reset_Portal_Info ();
		cull_obj->Set_Taken (false);
		cull_obj->Set_Sector (NULL);
		PathfindClass::Get_Instance ()->Add_Body_Box (cull_obj);
		MEMBER_RELEASE (cull_obj);
	}
	body_box_list.Delete_All ();

	//
	//	Debug info
	//
	CStringA message;
	message.Format ("Total sectors: %d\r\n", sectors);
	::Ouput_Message (message);

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Free_Sectors
//
///////////////////////////////////////////////////////////////////////////
void
PathfindSectorBuilderClass::Free_Sectors (void)
{
	//
	//	Free all the sector objects
	//
	for (int index = 0; index < m_SectorList.Count (); index ++) {
		PathfindSectorClass *sector = m_SectorList[index];
		MEMBER_RELEASE (sector);
	}

	m_SectorList.Delete_All ();
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Import_Raw_Data
//
///////////////////////////////////////////////////////////////////////////
void
PathfindSectorBuilderClass::Import_Raw_Data (void)
{
	BODY_BOX_LIST &raw_list = PathfindClass::Get_Instance ()->Get_Raw_Data ();

	//
	//	Add all the body boxes from a previous build to the system.
	//
	for (int index = 0; index < raw_list.Count (); index ++) {
		BodyBoxCullObj *body_box = raw_list[index];
		Mark_Sector (body_box);

		//
		//	Add this box to our process queue if it wasn't processed
		// the first time around.
		//
		if (body_box->Needs_Processing ()) {
			m_FloodFillProcessList.Add (body_box);
		}
	}

	PathfindClass::Get_Instance ()->Reset_Generated_Data ();
	return ;
}
