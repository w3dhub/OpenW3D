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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/PathfindSectorBuilder.cpp                                                                                                                                                                                                                                                                                                                                   $Modtime:: 5/27/00 4:10p                                               $*
 *                                                                                             *
 *                    $Revision:: 56                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "StdAfx.h"
#include "PathfindSectorBuilder.h"
#include "phys3.h"
#include "editorassetmgr.h"
#include "humanphys.h"
#include "phys3.h"
#include "boxrobj.h"
#include "Pathfind.h"
#include "pscene.h"
#include "physcoltest.h"
#include "filemgr.h"
#include "_assetmgr.h"
#include "Utils.h"
#include "nodemgr.h"
#include "filelocations.h"
#include "transitionnode.h"
#include "collisiongroups.h"
#include "PathfindPortal.h"
#include "GeneratingPathfindDialog.h"
#include "terrainnode.h"
#include "sceneeditor.h"
#include "staticanimphys.h"
#include "elevator.h"
#include "doors.h"
#include "soldier.h"
#include "humanphys.h"
#include "combatchunkid.h"
#include <algorithm>


//////////////////////////////////////////////////////////////////////////
//	Local Prototypes
//////////////////////////////////////////////////////////////////////////
UINT fnPathfindDialogThread (DWORD_PTR dwparam1, DWORD_PTR dwparam2, DWORD_PTR, HRESULT *, HWND *);


///////////////////////////////////////////////////////////////////////////
//	Local inlines
///////////////////////////////////////////////////////////////////////////
static inline bool
Clip_Point (Vector3 *point, const AABoxClass &box)
{
	Vector3 temp_point = *point;

	//
	//	Clip the temporary point
	//
	temp_point.X = std::clamp (temp_point.X, box.Center.X - box.Extent.X, box.Center.X + box.Extent.X);
	temp_point.Y = std::clamp (temp_point.Y, box.Center.Y - box.Extent.Y, box.Center.Y + box.Extent.Y);
	temp_point.Z = std::clamp (temp_point.Z, box.Center.Z - box.Extent.Z, box.Center.Z + box.Extent.Z);

	//
	//	Did the clip change the point?
	//
	bool retval = (point->X != temp_point.X);
	retval		|= (point->Y != temp_point.Y);
	retval		|= (point->Z != temp_point.Z);
	
	//
	//	Pass the new point back to the caller
	//
	(*point) = temp_point;
	
	return retval;
}


//////////////////////////////////////////////////////////////////////////
//	Local constants
//////////////////////////////////////////////////////////////////////////
const float ONE_SEC_FALL_DIST		= 4.9F;


class SimDirInfoClass
{
	public:

		SimDirInfoClass (void) { }
		SimDirInfoClass (float _heading, float _speed, const Vector3 &_move, const Vector3 &_quadmove)
			: heading (_heading), speed (_speed), move (_move), quad_move (_quadmove) { }

		float		heading;
		float		speed;
		Vector3	move;
		Vector3	quad_move;
};

//
//	Note:  The vector members are in "percentage of bounding box" units.
//
static const SimDirInfoClass DIR_INFO[DIR_MAX] =
{
	SimDirInfoClass ( 0.0F,						 	 0, Vector3 ( 1.0F,  0,    0), Vector3 ( 1.5F,	-0.5F,	0) ),
	SimDirInfoClass ( (float)DEG_TO_RAD (90),	 0, Vector3 ( 0,     1.0F, 0), Vector3 ( 0.5F,	 1.5F,	0) ),
	SimDirInfoClass ( (float)DEG_TO_RAD (180), 0, Vector3 (-1.0F,  0,    0), Vector3 (-1.5F,	 0.5F,	0) ),
	SimDirInfoClass ( (float)DEG_TO_RAD (270), 0, Vector3 ( 0,    -1.0F, 0), Vector3 (-0.5F,	-1.5F,	0) )
};


static HumanPhysClass *_PhysSimObj = NULL;
static SoldierGameObj *_GameSimObj = NULL;


//////////////////////////////////////////////////////////////////////////
//
//	PathfindSectorBuilderClass
//
//////////////////////////////////////////////////////////////////////////
PathfindSectorBuilderClass::PathfindSectorBuilderClass (void)
	:	m_CurrentSector (NULL),
		m_SimBoundingBox (Vector3 (0.5F, 0.5F, 1.8F)),
		m_SimBoxExtents (Vector3 (0.25F, 0.25F, 0.9F)),
		m_DirInfo (NULL),
		m_StepHeight (0),
		m_TotalBoxCount (0),
		m_TotalBoxGuess (0),
		m_BeforeUpdateCount (0),
		m_AllowWaterFloodfill (false),
		m_MaxSectorDim (28000.0F)
{
	RenderObjClass *commando_obj = NULL;

	//
	//	Create an instance of the commando render object we can 
	// use to simulate character movement
	//
	SoldierGameObjDef *definition = (SoldierGameObjDef *)DefinitionMgrClass::Find_Typed_Definition ("Commando", CLASSID_GAME_OBJECT_DEF_SOLDIER, false);
	if (definition != NULL) {
		SoldierGameObj *game_obj = new SoldierGameObj;
		game_obj->Init (*definition);
		REF_PTR_SET (commando_obj, game_obj->Peek_Model ());
		game_obj->Set_Delete_Pending ();
	}

	if (commando_obj != NULL) {

		//
		//	Attempt to find the collision box for this physics object
		//
		RenderObjClass *collision_box = NULL;
		if (commando_obj->Class_ID () == RenderObjClass::CLASSID_DISTLOD) {
			RenderObjClass *lod0	= commando_obj->Get_Sub_Object(0);
			collision_box			= lod0->Get_Sub_Object_By_Name ("WORLDBOX");
			REF_PTR_RELEASE (lod0);
		} else {
			collision_box			= commando_obj->Get_Sub_Object_By_Name ("WORLDBOX");
		}
		
		//
		//	Store the exents of the collision box for use in our simulation
		//
		if (collision_box != NULL) {
			const AABoxClass &box	= collision_box->Get_Bounding_Box ();
			m_SimBoundingBox			= box.Extent;
			m_SimBoundingBox.Z		= (box.Extent.Z * 2);
			m_SimBoxExtents			= m_SimBoundingBox * 0.5F;
			m_StepHeight				= 0.25F;

			REF_PTR_RELEASE (collision_box);
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
			
			m_DirInfo[index].quad_move.X	*= m_SimBoundingBox.X;
			m_DirInfo[index].quad_move.Y	*= m_SimBoundingBox.Y;
			m_DirInfo[index].quad_move.Z	*= m_SimBoundingBox.Z;

			m_DirInfo[index].speed			= m_DirInfo[index].move.Length ();
		}		

		REF_PTR_RELEASE (commando_obj);

		//
		//	Configure the body box culling system
		//
		Vector3 lev_min;
		Vector3 lev_max;
		PhysicsSceneClass::Get_Instance ()->Get_Level_Extents (lev_min, lev_max);
		m_BodyBoxCullingSystem.Initialize (m_SimBoxExtents, lev_min, lev_max);

		//
		//	Setup the simulation constants
		//
		m_SimMovement = Vector3 (0, 0, -(ONE_SEC_FALL_DIST + (m_SimBoundingBox.Z * 0.75F)));
		m_SimSweepBox.Extent = m_SimBoxExtents;
	}

	Detect_Level_Features ();	

	/*AABoxClass collision_box;
	collision_box.Center.Set (0, 0, 0);
	collision_box.Extent = m_SimBoxExtents;
	_PhysSimObj->Set_Collision_Box (collision_box);*/
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//	~PathfindSectorBuilderClass
//
//////////////////////////////////////////////////////////////////////////
PathfindSectorBuilderClass::~PathfindSectorBuilderClass (void)
{
	/*if (_GameSimObj != NULL) {
		_GameSimObj->Destroy ();
		_GameSimObj = NULL;
	}*/

	if (m_DirInfo != NULL) {
		delete [] m_DirInfo;
		m_DirInfo = NULL;
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//	Prepare_Level
//
//////////////////////////////////////////////////////////////////////////
void
PathfindSectorBuilderClass::Prepare_Level (void)
{
	::Get_Scene_Editor ()->Display_Static_Anim_Phys (true);

	//
	//	Get the list of all static objects from the physics scene
	//
	RefPhysListIterator iterator = ::Get_Scene_Editor ()->Get_Static_Object_Iterator ();
	for (iterator.First (); !iterator.Is_Done (); iterator.Next ()) {

		//
		//	Is this object a static anim phys?
		//
		PhysClass *phys_obj							= iterator.Peek_Obj ();
		StaticAnimPhysClass *static_phys_obj	= phys_obj->As_StaticAnimPhysClass ();
		if (static_phys_obj != NULL) {

			//
			//	Should we disable collisions with this object during pathfind?
			//
			StaticAnimPhysDefClass *definition = static_phys_obj->Get_StaticAnimPhysDef ();
			if (definition->Does_Collide_In_Pathfind () == false) {
				phys_obj->Inc_Ignore_Counter ();
			} else {
				while (phys_obj->Is_Ignore_Me ()) {
					phys_obj->Dec_Ignore_Counter ();
				}
			}
		}
	}

	//
	//	Make sure that tile's use collision group 15...
	//
	for (	NodeClass *node = NodeMgrClass::Get_First (NODE_TYPE_TILE);
			node != NULL;
			node = NodeMgrClass::Get_Next (node, NODE_TYPE_TILE))
	{
		PhysClass *phys_obj = node->Peek_Physics_Obj ();
		if (phys_obj != NULL) {
			phys_obj->Set_Collision_Group (15);
		}
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//	Restore_Level
//
//////////////////////////////////////////////////////////////////////////
void
PathfindSectorBuilderClass::Restore_Level (void)
{
	//
	//	Restore each tile's collision group...
	//
	for (	NodeClass *node = NodeMgrClass::Get_First (NODE_TYPE_TILE);
			node != NULL;
			node = NodeMgrClass::Get_Next (node, NODE_TYPE_TILE))
	{
		PhysClass *phys_obj = node->Peek_Physics_Obj ();
		if (phys_obj != NULL) {
			phys_obj->Set_Collision_Group (EDITOR_COLLISION_GROUP);
		}
	}

	//
	//	Get the list of all static objects from the physics scene
	//
	RefPhysListIterator iterator = ::Get_Scene_Editor ()->Get_Static_Object_Iterator ();
	for (iterator.First (); !iterator.Is_Done (); iterator.Next ()) {

		//
		//	Is this object a static anim phys?
		//
		PhysClass *phys_obj							= iterator.Peek_Obj ();
		StaticAnimPhysClass *static_phys_obj	= phys_obj->As_StaticAnimPhysClass ();
		if (static_phys_obj != NULL) {

			//
			//	Do we need to re-enable collisions with this object?
			//
			StaticAnimPhysDefClass *definition = static_phys_obj->Get_StaticAnimPhysDef ();
			if (definition->Does_Collide_In_Pathfind () == false) {
				phys_obj->Dec_Ignore_Counter ();
			}			
		}
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//	Initialize
//
//////////////////////////////////////////////////////////////////////////
void
PathfindSectorBuilderClass::Initialize (void)
{
	Prepare_Level ();

	//
	//	Turn off sector display
	//
	PathfindClass::Get_Instance ()->Display_Sectors (false);
	PathfindClass::Get_Instance ()->Display_Portals (false);

	//
	// Initialize the dynamic vector array's so they will resize
	// in appropriate size steps (to decrease the total number of resizes)
	//
	m_FloodFillProcessList.Set_Growth_Step (20000);
	m_PossiblePortalList.Set_Growth_Step (20000);
	m_BodyBoxReleaseList.Set_Growth_Step (100000);

	//
	// Make a guess at the total number of boxes in the world based on
	// its size
	//
	Vector3 lev_min;
	Vector3 lev_max;
	PhysicsSceneClass::Get_Instance ()->Get_Level_Extents (lev_min, lev_max);
	int width = (lev_max.X - lev_min.X) / m_SimBoundingBox.X;
	int height = (lev_max.Y - lev_min.Y) / m_SimBoundingBox.Y;
	m_TotalBoxGuess = width * height;

	//
	// Show some UI
	//
	Show_Dialog ();
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//	Shutdown
//
//////////////////////////////////////////////////////////////////////////
void
PathfindSectorBuilderClass::Shutdown (void)
{
	//
	//	Do some cleanup
	//
	Free_Floodfill_Boxes ();
	Cleanup_Level_Features ();	

	//
	//	Make sure we re-partition the sector culling system so
	// it does us some good... :)
	//
	PathfindClass::Get_Instance ()->Re_Partition_Sector_Tree ();

	//
	//	Shutdown the UI and restore any level objects we modified
	//
	Close_Dialog ();
	Restore_Level ();
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//	Compress_Sectors_For_Dyna_Culling
//
//////////////////////////////////////////////////////////////////////////
void
PathfindSectorBuilderClass::Compress_Sectors_For_Dyna_Culling (void)
{
	const float MAX_DYNAMIC_VIS_CELL_DIMENSION = 20.0f;

	//
	// Change the dialog's checkmark so the user knows we are comprssing sectors
	//
	m_pDialog->Set_State(GeneratingPathfindDialogClass::STATE_COMPRESS);

	//
	// Floodfill the level and compress the floodfill into sectors
	//
	DynamicVectorClass<AABoxClass> sectors;

	Set_Max_Sector_Size(MAX_DYNAMIC_VIS_CELL_DIMENSION);
	
	Compress_Sectors(&sectors);

	// 
	// Hand the sectors into the scene (NOTE: this will have to reset the
	// VIS system)
	//
	PhysicsSceneClass::Get_Instance()->Re_Partition_Dynamic_Culling_System(sectors);

	return;
}


//////////////////////////////////////////////////////////////////////////
//
//	Compress_Sectors_For_Pathfind
//
//////////////////////////////////////////////////////////////////////////
void
PathfindSectorBuilderClass::Compress_Sectors_For_Pathfind (void)
{
	//
	// Discard any previously generated data
	//
	PathfindClass::Get_Instance ()->Reset_Sectors ();
	PathfindClass::Get_Instance ()->Reset_Portals ();

	//
	// Change the dialog's checkmark so the user knows we are comprssing sectors
	//
	m_pDialog->Set_State (GeneratingPathfindDialogClass::STATE_COMPRESS);

	//
	//	Compress the sectors into the largest possible rectangular regions
	//
	DWORD before_ticks = ::GetTickCount ();
	
	Post_Process_Floodfill_For_Level_Features ();
	Compress_Sectors ();
	Generate_Portals ();

	DWORD after_ticks = ::GetTickCount ();

	WWDEBUG_SAY(("Time spent compressing: %d secs.\r\n", (after_ticks-before_ticks)/1000));
	
	//
	//	Loop through the level and create any portals around
	// ladders, doors, etc that may be needed.
	//
	Apply_Level_Features ();	
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//	Generate_Sectors
//
//////////////////////////////////////////////////////////////////////////
void
PathfindSectorBuilderClass::Generate_Sectors (void)
{
	//
	//	Start floodfilling from each of the start points
	//
	for (int index = 0; index < m_StartPointList.Count (); index ++) {
		Vector3 start_pos = m_StartPointList[index];

		//
		//	Normalize the starting position.  We do this so that all the different
		// generation points will match up perfectly.
		//
		start_pos.X = (int(start_pos.X / m_SimBoundingBox.X)) * m_SimBoundingBox.X;
		start_pos.Y = (int(start_pos.Y / m_SimBoundingBox.Y)) * m_SimBoundingBox.Y;
		start_pos.Z += (m_SimBoundingBox.Z * 0.5F);

		//
		// Start floodfilling from this position
		//
		AABoxClass new_box;		
		if (Try_Standing_Here (start_pos, &new_box)) {
			m_CurrentSector = Mark_Sector (new_box);
			Floodfill (start_pos);
		}
	}

	//
	//	Process all the floodfill boxes that have been queued up
	//
	while (m_FloodFillProcessList.Count () > 0) {				
		
		//
		//	Pop a floodfill box off the list and process its neighbors
		//
		m_CurrentSector = m_FloodFillProcessList[0];
		m_FloodFillProcessList.Delete (0);

		AABoxClass box	= Get_Body_Box_Bounds (m_CurrentSector);
		Vector3 pos		= box.Center;
		pos.Z				= box.Center.Z - box.Extent.Z + (m_SimBoundingBox.Z * 0.5F);
		Floodfill (pos);
	}	


	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//	Find_Ground
//
//////////////////////////////////////////////////////////////////////////
bool
PathfindSectorBuilderClass::Find_Ground (const Vector3 &pos, float *ground_pos)
{
	bool retval = false;

	AABoxClass sweep_box;
	sweep_box.Center		= pos;
	sweep_box.Center.Z	+= m_StepHeight;
	sweep_box.Extent		= m_SimBoxExtents;
	sweep_box.Extent.Z	= 0.1F;

	//
	//	Sweep a very short box downwards to see where the ground is
	//
	CastResultStruct result;
	Vector3 sweep_vector (0, 0, -6.5F);
	PhysAABoxCollisionTestClass test(	sweep_box,
													sweep_vector,
													&result,
													STATIC_OBJ_COLLISION_GROUP,
													COLLISION_TYPE_PHYSICAL);

	PhysicsSceneClass::Get_Instance ()->Cast_AABox (test);

	//
	//	Check to see if we found a floor, and its valid to stand on.
	//
	if (	(result.StartBad == false) &&
			(result.Fraction < 1.0F) &&
			(result.Normal.Z > 0.71F) &&			
			(m_AllowWaterFloodfill || (result.SurfaceType != SURFACE_TYPE_WATER)))
	{
		//
		//	Calculate what z-position the ground is
		//
		(*ground_pos) = (sweep_box.Center.Z + (sweep_vector.Z * result.Fraction)) - sweep_box.Extent.Z;
		retval = true;
	}

	return retval;
}


//////////////////////////////////////////////////////////////////////////
//
//	Find_Ceiling
//
//////////////////////////////////////////////////////////////////////////
bool
PathfindSectorBuilderClass::Find_Ceiling (const Vector3 &pos, float *ceiling_pos)
{
	bool retval = false;

	AABoxClass sweep_box;
	sweep_box.Center		= pos;
	sweep_box.Center.Z	+= m_StepHeight;
	sweep_box.Extent		= m_SimBoxExtents;
	sweep_box.Extent.Z	= 0.1F;

	//
	//	Sweep a very short box downwards to see where the ground is
	//
	CastResultStruct result;
	Vector3 sweep_vector (0, 0, 6.5F);
	PhysAABoxCollisionTestClass test(	sweep_box,
													sweep_vector,
													&result,
													STATIC_OBJ_COLLISION_GROUP,
													COLLISION_TYPE_PHYSICAL);

	PhysicsSceneClass::Get_Instance ()->Cast_AABox (test);

	//
	//	Check to ensure we didn't start the cast intersecting a poly
	//
	if (result.StartBad == false) {
		
		//
		//	Calculate what the z-position of the ceiling is
		//
		(*ceiling_pos) = (sweep_box.Center.Z + (sweep_vector.Z * result.Fraction)) + sweep_box.Extent.Z;
		retval = true;
	}

	return retval;
}


//////////////////////////////////////////////////////////////////////////
//
//	Try_Moving_Here
//
//////////////////////////////////////////////////////////////////////////
bool
PathfindSectorBuilderClass::Try_Moving_Here
(
	const Vector3 &	start_pos,
	const Vector3 &	expected_pos,
	AABoxClass *		real_pos
)
{
	bool is_valid = false;

	float start_ground_pos	= start_pos.Z - m_SimBoxExtents.Z;
	float curr_ground_pos	= 0;

	//
	//	Find the ground level where we were standing and the ground level where
	// we want to be standing
	//
	if (Find_Ground (expected_pos, &curr_ground_pos)) {

		//
		//	Check to see if the character can step from the old position to the new one
		//
		float step_height = curr_ground_pos - start_ground_pos;
		if (step_height < m_StepHeight) {

			//
			//	Find the ceiling
			//
			float curr_ceiling_pos = curr_ground_pos;
			Find_Ceiling (expected_pos, &curr_ceiling_pos);

			//
			//	Can the character fit between the floor and ceiling without crouching?
			//
			if ((curr_ceiling_pos - curr_ground_pos) > m_SimBoundingBox.Z) {

				//
				//	Success!  Build a bounding box for the new position
				//
				real_pos->Extent.X	= m_SimBoxExtents.X;
				real_pos->Extent.Y	= m_SimBoxExtents.Y;
				real_pos->Extent.Z	= m_SimBoxExtents.Z;
				real_pos->Center.X	= expected_pos.X;
				real_pos->Center.Y	= expected_pos.Y;
				real_pos->Center.Z	= curr_ground_pos + m_SimBoxExtents.Z + WWMATH_EPSILON;
				is_valid					= true;
			}
		}
	}

	return is_valid;
}


//////////////////////////////////////////////////////////////////////////
//
//	Try_Standing_Here
//
//////////////////////////////////////////////////////////////////////////
bool
PathfindSectorBuilderClass::Try_Standing_Here
(
	const Vector3 &	expected_pos,
	AABoxClass *		real_pos
)
{
	bool is_valid = false;

	AABoxClass sweep_box;
	sweep_box.Center		= expected_pos;
	sweep_box.Center.Z	+= m_StepHeight;
	sweep_box.Extent		= m_SimSweepBox.Extent;
	sweep_box.Extent.Z	-= m_StepHeight;	

	//
	//	Do a box cast to see if we can 'stand' here.
	//
	CastResultStruct result;
	PhysAABoxCollisionTestClass test(	sweep_box,
													m_SimMovement,
													&result,
													STATIC_OBJ_COLLISION_GROUP,
													COLLISION_TYPE_PHYSICAL);

	test.CheckStaticObjs = true;
	test.CheckDynamicObjs = false;

	PhysicsSceneClass::Get_Instance ()->Cast_AABox (test);

	//
	//	Is the start of the cast valid and is the polygon we
	// landed on flat enough to stand on?
	//
	if ((result.StartBad == false) && (result.Normal.Z > 0.71F)) {			

		//
		//	Are we standing on the ground?  Is the ground a valid type?
		//
		bool on_ground = (result.Fraction < 1.0F);
		if (on_ground && (m_AllowWaterFloodfill || (result.SurfaceType != SURFACE_TYPE_WATER))) {
			
			//
			//	Snap the x and y positions and calculate the z position
			// of the new box.
			//			
			real_pos->Extent.X	= sweep_box.Extent.X;
			real_pos->Extent.Y	= sweep_box.Extent.Y;
			real_pos->Extent.Z	= ((-m_SimMovement.Z * result.Fraction) * 0.5F) + (m_SimBoundingBox.Z * 0.5F);
			real_pos->Center.X	= expected_pos.X;
			real_pos->Center.Y	= expected_pos.Y;
			real_pos->Center.Z	= (sweep_box.Center.Z + (m_SimBoundingBox.Z * 0.5F) - real_pos->Extent.Z) + 0.001F;
			is_valid					= true;
		}
	}

	return is_valid;
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
	//	Determine where the next cell should be
	//
	Vector3 expected_pos	= start_pos + m_DirInfo[direction].move;

	//
	//	Do a quick-and-dirty test to see if we already 'know' about a
	// body-box in the requested direction.
	//
	bool found = false;
	FloodfillBoxClass *neighbor = m_CurrentSector->Peek_Neighbor (direction, false);
	if (neighbor != NULL) {

		m_SimSweepBox.Center = expected_pos;

		//
		//	If the neighbor's bounding box completely CONTAINS the bounding
		// box we would have cast, then we can skip the cast and simply
		// mark the direction as 'traversible'
		//
		AABoxClass bounding_box = Get_Body_Box_Bounds (neighbor);
		found = bounding_box.Contains (m_SimSweepBox);

		float min1 = bounding_box.Center.Z - bounding_box.Extent.Z;
		float max1 = bounding_box.Center.Z + bounding_box.Extent.Z;
		float min2 = m_SimSweepBox.Center.Z - m_SimSweepBox.Extent.Z;
		float max2 = m_SimSweepBox.Center.Z + m_SimSweepBox.Extent.Z;

		found = (min1 <= min2) && (max1 >= max2);
		if (found) {
			m_CurrentSector->Set_Neighbor (direction, neighbor, true);
		}
	}
	
	//
	//	Did the quick-and-dirty check fail?  If so, then do the full box-cast
	//
	if (found == false) {
				
		Vector3 move_vector	= (m_DirInfo[direction].move);
		Vector3 new_pos		= start_pos + move_vector;

		AABoxClass new_box;
		//if (Try_Standing_Here (new_pos, &new_box)) {
		if (Try_Moving_Here (start_pos, new_pos, &new_box)) {		
			Submit_Box (m_CurrentSector, new_box, direction);
		}
	}

	return ;
}






//////////////////////////////////////////////////////////////////////////
//
//	Do_Real_Physics_Sim
//
//////////////////////////////////////////////////////////////////////////
void
PathfindSectorBuilderClass::Do_Real_Physics_Sim
(
	const Vector3 &	start_pos,
	PATHFIND_DIR		direction
)
{
	//
	//	Determine where the next cell should be
	//
	Vector3 expected_pos	= start_pos + m_DirInfo[direction].move;

	//
	//	Do a quick-and-dirty test to see if we already 'know' about a
	// body-box in the requested direction.
	//
	bool found = false;
	FloodfillBoxClass *neighbor = m_CurrentSector->Peek_Neighbor (direction, false);
	if (neighbor != NULL) {

		m_SimSweepBox.Center = expected_pos;

		//
		//	If the neighbor's bounding box completely CONTAINS the bounding
		// box we would have cast, then we can skip the cast and simply
		// mark the direction as 'traversible'
		//
		AABoxClass bounding_box = Get_Body_Box_Bounds (neighbor);
		found = bounding_box.Contains (m_SimSweepBox);

		float min1 = bounding_box.Center.Z - bounding_box.Extent.Z;
		float max1 = bounding_box.Center.Z + bounding_box.Extent.Z;
		float min2 = m_SimSweepBox.Center.Z - m_SimSweepBox.Extent.Z;
		float max2 = m_SimSweepBox.Center.Z + m_SimSweepBox.Extent.Z;

		found = (min1 <= min2) && (max1 >= max2);
		if (found) {
			m_CurrentSector->Set_Neighbor (direction, neighbor, true);
		}
	}
	
	//
	//	Did the quick-and-dirty check fail?  If so, then do the full box-cast
	//
	if (found == false) {

		Vector3 move_vector	= (m_DirInfo[direction].move);
		Vector3 new_pos		= start_pos + move_vector;

		AABoxClass test_box (start_pos, m_SimBoxExtents);
		test_box.Add_Box (AABoxClass (new_pos, m_SimBoxExtents));
		test_box.Center.Z += 0.1F;

		NonRefPhysListClass obj_list;
		PhysicsSceneClass::Get_Instance ()->Collect_Collideable_Objects (test_box, COLLISION_TYPE_PHYSICAL, true, false, &obj_list);

		if (obj_list.Count () == 0) {

			AABoxClass new_box;
			if (Try_Standing_Here (new_pos, &new_box)) {
				Submit_Box (m_CurrentSector, new_box, direction);
			}

			return ;
		} else {
			
			bool did_intersect = false;

			NonRefPhysListIterator it (&obj_list);
			for (it.First(); !it.Is_Done(); it.Next()) {
				PhysClass *phys_obj = it.Peek_Obj ();
				if (phys_obj != NULL) {
					RenderObjClass *model = phys_obj->Peek_Model ();
					
					AABoxIntersectionTestClass int_test (test_box, COLLISION_TYPE_PHYSICAL);
					if (model->Intersect_AABox (int_test)) {
						did_intersect = true;
						break;
					}
				}
			}

			if (did_intersect == false) {

				AABoxClass new_box;
				if (Try_Standing_Here (new_pos, &new_box)) {
					Submit_Box (m_CurrentSector, new_box, direction);					
				}
				
				return ;
			}
		}

		
		_PhysSimObj->Set_Position (start_pos);
		_PhysSimObj->Set_Velocity (Vector3 (0, 0, 0));
		int attempt;
		for (attempt = 0; attempt < 20; attempt ++) {
			if (_PhysSimObj->Is_In_Contact ()) {
				break;
			}

			_PhysSimObj->Timestep (0.1F);
		}

		bool keep_going = true;
		for (attempt = 0; keep_going && (attempt < 20); attempt ++) {

			Vector3 curr_pos = _PhysSimObj->Get_Position ();

			Vector3 delta = (new_pos - curr_pos);
			delta.Z = 0;

			float delta_len = delta.Length ();
			if (delta_len < 0.05F && _PhysSimObj->Is_In_Contact ()) {
				
				AABoxClass new_box;
				new_box.Center		= new_pos;
				new_box.Center.Z	= curr_pos.Z;
				new_box.Extent		= m_SimBoxExtents;
				Submit_Box (m_CurrentSector, new_box, direction);
				keep_going = false;

			} else {
				
				Vector3 x_axis = (new_pos - curr_pos);				
				Vector3 z_axis = Vector3 (0, 0, 1);
				x_axis.Normalize ();
				
				Vector3 y_axis = Vector3::Cross_Product (x_axis, z_axis);

				Matrix3D new_tm (x_axis, y_axis, z_axis, curr_pos);
				_PhysSimObj->Set_Transform (new_tm);

				_PhysSimObj->Get_Controller ()->Reset ();
				_PhysSimObj->Get_Controller ()->Set_Move_Forward (0.09F);
				_PhysSimObj->Timestep (0.075F);
			}
		}

		/*AABoxClass new_box;
		if (Try_Standing_Here (new_pos, &new_box)) {
			Submit_Box (m_CurrentSector, new_box, direction);
		}*/
	}

	return ;
}







//////////////////////////////////////////////////////////////////////////
//
//	Submit_Box
//
//////////////////////////////////////////////////////////////////////////
FloodfillBoxClass *
PathfindSectorBuilderClass::Submit_Box
(
	FloodfillBoxClass *		from_obj,
	const AABoxClass &	new_box,
	PATHFIND_DIR			direction
)
{
	//
	//	Is this cell already marked?
	//
	Vector3 position	= new_box.Center;
	position.Z			= new_box.Center.Z - new_box.Extent.Z + (m_SimBoundingBox.Z * 0.5F);

	FloodfillBoxClass *occupant = Get_Sector_Occupant (position);
	if (occupant == NULL) {
		
		//
		//	Mark this cell and add it to the list of sectors
		// that need to be recursed.
		//
		occupant = Mark_Sector (new_box);
		m_FloodFillProcessList.Add (occupant);

		//
		//	Let the new occupant know where it came from (even if it can't get there itself).
		//
		occupant->Set_Neighbor (::Inverse_Pathfind_Dir (direction), from_obj, false);
		m_TotalBoxCount ++;
	}

	//
	//	Let the sector know who its new neighbor is
	//
	if (occupant != from_obj) {
		from_obj->Set_Neighbor (direction, occupant, true);
	}
	
	return occupant;
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
	if (m_pDialog->Was_Cancelled () == false) {
		
		//
		//	Loop through and check any neighbors we don't already know about
		//
		for (int index = 0; index < DIR_MAX; index ++) {
			if (m_CurrentSector->Peek_Neighbor (PATHFIND_DIR(index)) == NULL) {
				Do_Physics_Sim (start_pos, PATHFIND_DIR(index));
				m_BeforeUpdateCount ++;				
			}
		}

		//
		//	Check to see if we need to climb up any ladders, open doors, etc...
		//		
		Check_For_Level_Feature (m_CurrentSector);

		//
		//	Let the current sector 'know' its been completely processed
		//
		m_CurrentSector->Needs_Processing (false);

		//
		// Update the UI
		//
		if (m_BeforeUpdateCount > 512) {
			CString message;
			message.Format ("Floodfilled %d boxes. Approx total boxes: %d", m_TotalBoxCount, m_TotalBoxGuess);
			m_pDialog->Set_Status (message, float (m_TotalBoxCount) / float (m_TotalBoxGuess));
			m_BeforeUpdateCount = 0;
		}
	}

	
	//
	// Allow windows messages to be processed
	//
	static int message_pump = 0;
	message_pump++;
	if (message_pump > 5000) {
		General_Pump_Messages ();
		message_pump = 0;
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//	Get_Sector_Occupant
//
//////////////////////////////////////////////////////////////////////////
FloodfillBoxClass *
PathfindSectorBuilderClass::Get_Sector_Occupant (const Vector3 &pos)
{
	//
	//	If we found any objects in the culling system, then the sector is occupied
	//
	return m_BodyBoxCullingSystem.Find_Box (pos);
}


//////////////////////////////////////////////////////////////////////////
//
//	Mark_Sector
//
//////////////////////////////////////////////////////////////////////////
void
PathfindSectorBuilderClass::Mark_Sector (FloodfillBoxClass *body_box)
{
	//
	//	Add the culling object  to our culling system.  (This
	// effectively marks the cell as 'occupied').
	//
	body_box->Add_Ref ();
	m_BodyBoxCullingSystem.Add_Box (body_box);
	FloodfillBoxClass::Add (body_box);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//	Mark_Sector
//
//////////////////////////////////////////////////////////////////////////
FloodfillBoxClass *
PathfindSectorBuilderClass::Mark_Sector (const AABoxClass &bounding_box)
{
	//
	//	Create a culling system object to represent this 'sector' and
	// add it to our AABTree culling system.  (This effectively marks
	// the cell as 'occupied').
	//
	FloodfillBoxClass *body_box = new FloodfillBoxClass;
	body_box->Set_Position (bounding_box.Center);

	Mark_Sector (body_box);
	body_box->Release_Ref ();
	return body_box;
}


///////////////////////////////////////////////////////////////////////////
//
//	Determine_Height
//
///////////////////////////////////////////////////////////////////////////
void
PathfindSectorBuilderClass::Determine_Height
(
	FloodfillBoxClass *	start_box,
	float	*			min_z_pos,
	float	*			max_z_pos
)
{
	AABoxClass box = Get_Body_Box_Bounds (start_box);
	float z_extent = box.Extent.Z;
	box.Extent.X	-= WWMATH_EPSILON;
	box.Extent.Y	-= WWMATH_EPSILON;	
	box.Extent.Z	= 500.0F;

	//
	//	Loop over all the body-boxes that exist above or below the starting box
	//
	m_BodyBoxCullingSystem.Collect_Boxes (box);
	DynamicVectorClass<FloodfillBoxClass *> &list = m_BodyBoxCullingSystem.Get_Collection_List ();
	for (int index = 0; index < list.Count (); index ++) {
		FloodfillBoxClass *body_box = list[index];

		//
		//	Only check 'other' boxes
		//
		if (body_box != start_box) {
			AABoxClass curr_box = Get_Body_Box_Bounds (body_box);

			//
			//	Determine the min and max z-positions of the current box
			//
			float curr_min_z = curr_box.Center.Z - (curr_box.Extent.Z + WWMATH_EPSILON);
			float curr_max_z = curr_box.Center.Z + (curr_box.Extent.Z + WWMATH_EPSILON);

			//
			//	Return new minimum and maximum
			//			
			if (curr_max_z < box.Center.Z && curr_max_z > (*min_z_pos)) {
				(*min_z_pos) = curr_max_z;
			}
			
			if (curr_min_z > box.Center.Z && curr_min_z < (*max_z_pos)) {
				(*max_z_pos) = curr_min_z;
			}
		}
	}

	/*float delta_up		= (*max_z_pos) - box.Center.Z;
	float delta_down	= box.Center.Z - (*min_z_pos);

	delta_up		= std::max (delta_up / 2.0F, (z_extent + WWMATH_EPSILON));
	delta_down	= std::max (delta_down / 2.0F, (z_extent + WWMATH_EPSILON));

	(*max_z_pos) = box.Center.Z + delta_up;
	(*min_z_pos) = box.Center.Z - delta_down;*/

	//
	//	Make sure a box's min position is at least the
	// bottom of its box.
	//
	float box_bottom = (box.Center.Z - z_extent);
	if ((*min_z_pos) > (box_bottom - 0.5F)) {
		(*min_z_pos) = (box_bottom - 0.5F);
	}

	
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//	Build_Height_Values
//
//////////////////////////////////////////////////////////////////////////
int
PathfindSectorBuilderClass::Build_Height_Values (void)
{
	int before_ticks = ::GetTickCount ();
	int total_box_count = 0;

	//
	//	Backup the body-box list to ensure we delete them all...
	//
	for (	FloodfillBoxClass *body_box = FloodfillBoxClass::Get_First ();
			body_box != NULL;
			body_box = body_box->Get_Next ())
	{
		m_BodyBoxReleaseList.Add (body_box);

		Vector3 pos	= body_box->Get_Position ();
		float min_z	= pos.Z - (m_SimBoundingBox.Z * 500.0F);
		float max_z	= pos.Z + (m_SimBoundingBox.Z * 500.0F);
		Determine_Height (body_box, &min_z, &max_z);
		body_box->Set_Min_Z_Pos (min_z);
		body_box->Set_Max_Z_Pos (max_z);

		total_box_count ++;
	}

	int after_ticks = ::GetTickCount ();
	WWDEBUG_SAY(("Time spent generating z-values: %d secs.\r\n", (after_ticks-before_ticks)/1000));

	return total_box_count;
}


//////////////////////////////////////////////////////////////////////////
//
//	Can_Traverse
//
//////////////////////////////////////////////////////////////////////////
inline bool
Can_Traverse (FloodfillBoxClass *cell, PATHFIND_DIR /* dir */)
{
	//
	//	Make sure the cell passes the basic 'traverse-requirements'
	// to move in the given direction
	//
	return (	cell != NULL &&
				cell->Needs_Processing () == false &&
				cell->Is_Taken () == false);
}


//////////////////////////////////////////////////////////////////////////
//
//	Check_Directions
//
//////////////////////////////////////////////////////////////////////////
inline bool
Check_Directions (FloodfillBoxClass *cell, int dir_mask)
{
	bool is_valid = true;

	//
	//	Check to make sure we can walk from the cell to its
	// adjacent boxes (Only checking the requested directions).
	//
	for (int mask = 1; mask < MASK_MAX; mask <<= 1) {
		if (dir_mask & mask) {
			PATHFIND_DIR check_dir = ::Mask_to_Dir (DIRECTION_MASK(mask));
			is_valid &= cell->Is_Two_Way_Traversible (check_dir);
		}
	}

	return is_valid;
}


//////////////////////////////////////////////////////////////////////////
//
//	Move_Dir
//
//////////////////////////////////////////////////////////////////////////
inline FloodfillBoxClass *
PathfindSectorBuilderClass::Move_Dir (FloodfillBoxClass *start_box, PATHFIND_DIR dir, int dir_mask)
{
	//
	//	Peek a the cell in the given direction
	//
	FloodfillBoxClass *retval		= NULL;
	FloodfillBoxClass *new_cell	= start_box->Peek_Neighbor (dir);


	//
	//	Does this cell pass our traversal requirements?
	//
	if (	start_box->Is_Two_Way_Traversible (dir) &&
			Can_Traverse (new_cell, dir) &&
			Check_Directions (new_cell, dir_mask))
	{
		//
		//	Add this cell's min and max heights to the height-watcher.
		// This is done so we can break the sector before it intersects
		// a sector above or below it
		//
		m_HeightWatcher.Add_Lower_Bound (new_cell->Get_Min_Z_Pos ());
		m_HeightWatcher.Add_Upper_Bound (new_cell->Get_Max_Z_Pos ());

		//
		//	Is this cell within the min and max height values?
		//
		AABoxClass box = Get_Body_Box_Bounds (new_cell);
		if (	m_HeightWatcher.Is_Current_Pos_Valid (box.Center.Z - box.Extent.Z) &&
				m_HeightWatcher.Is_Current_Pos_Valid (box.Center.Z + box.Extent.Z))
		{
			retval = new_cell;
		}		
	}

	return retval;
}


//////////////////////////////////////////////////////////////////////////
//
//	Check_Edge
//
//////////////////////////////////////////////////////////////////////////
inline bool
PathfindSectorBuilderClass::Check_Edge
(
	FloodfillBoxClass *	start_box,
	int				count_left,
	int				count_right,
	int				count_up,
	int				count_down,
	PATHFIND_DIR	move_dir
)
{
	bool retval = false;

	//
	//	Check the first cell, can we move in the requested direction?
	//
	int dir_mask = 0;
	dir_mask |= (count_left > 0) ? MASK_LEFT : 0;
	dir_mask |= (count_right > 0) ? MASK_RIGHT : 0;
	dir_mask |= (count_up > 0) ? MASK_UP : 0;
	dir_mask |= (count_down > 0) ? MASK_DOWN : 0;
	if (Move_Dir (start_box, move_dir, dir_mask) != NULL) {
		retval = true;
	}

	//
	//	Check the given number of cells in the left direction
	//
	FloodfillBoxClass *curr_cell = start_box;
	int x;
	for (x = 0; retval && x < count_left; x++) {
		
		//
		// Can we move one step in this direction?
		//
		curr_cell = Move_Dir (curr_cell, DIR_LEFT, 0);
		if (curr_cell != NULL) {
			
			//
			// Can this new cell move in the movement direction AND
			// can its new cell move right and, if necessary, left?
			//
			dir_mask = MASK_RIGHT;
			dir_mask |= ((x + 1) < count_left) ? MASK_LEFT : 0;
			retval = (Move_Dir (curr_cell, move_dir, dir_mask) != NULL);
		
		} else {
			retval = false;
		}
	}

	//
	//	Check the given number of cells in the left direction
	//
	curr_cell = start_box;
	for (x = 0; retval && x < count_right; x++) {
		
		//
		// Can we move one step in this direction?
		//
		curr_cell = Move_Dir (curr_cell, DIR_RIGHT, 0);
		if (curr_cell != NULL) {
			
			//
			// Can this new cell move in the movement direction AND
			// can its new cell move left and, if necessary, right?
			//
			dir_mask = MASK_LEFT;
			dir_mask |= ((x + 1) < count_right) ? MASK_RIGHT : 0;
			retval = (Move_Dir (curr_cell, move_dir, dir_mask) != NULL);
		
		} else {
			retval = false;
		}
	}

	//
	//	Check the given number of cells in the up direction
	//
	curr_cell = start_box;
	int y;
	for (y = 0; retval && y < count_up; y++) {
		
		//
		// Can we move one step in this direction?
		//
		curr_cell = Move_Dir (curr_cell, DIR_UP, 0);
		if (curr_cell != NULL) {
			
			//
			// Can this new cell move in the movement direction AND
			// can its new cell move down and, if necessary, up?
			//
			dir_mask = MASK_DOWN;
			dir_mask |= ((y + 1) < count_up) ? MASK_UP : 0;
			retval = (Move_Dir (curr_cell, move_dir, dir_mask) != NULL);
		
		} else {
			retval = false;
		}
	}

	//
	//	Check the given number of cells in the down direction
	//
	curr_cell = start_box;
	for (y = 0; retval && y < count_down; y++) {
		
		//
		// Can we move one step in this direction?
		//
		curr_cell = Move_Dir (curr_cell, DIR_DOWN, 0);
		if (curr_cell != NULL) {
			
			//
			// Can this new cell move in the movement direction AND
			// can its new cell move up and, if necessary, down?
			//
			dir_mask = MASK_UP;
			dir_mask |= ((y + 1) < count_down) ? MASK_DOWN : 0;
			retval = (Move_Dir (curr_cell, move_dir, dir_mask) != NULL);
		
		} else {
			retval = false;
		}
	}

	return retval;
}


//////////////////////////////////////////////////////////////////////////
//
//	Find_Perimeter
//
//////////////////////////////////////////////////////////////////////////
FloodfillBoxClass *
PathfindSectorBuilderClass::Find_Perimeter (FloodfillBoxClass *start_box, BOX_PERIMETER *perimeter)
{	
	FloodfillBoxClass *up_cell		= start_box;
	FloodfillBoxClass *down_cell		= start_box;
	FloodfillBoxClass *left_cell		= start_box;
	FloodfillBoxClass *right_cell	= start_box;
	
	//
	//	Determine what the maximum number of cells in a given direction can be
	//
	int max_dim = m_MaxSectorDim / std::max (m_SimBoundingBox.X, m_SimBoundingBox.Y);

	//
	//	Create an object that will allow us to break the sector if
	// it would intersect with a sector above or below us.
	//
	Vector3 pos = start_box->Get_Position ();
	m_HeightWatcher.Initialize (pos.Z - (m_SimBoundingBox.Z * 500.0F), pos.Z + (m_SimBoundingBox.Z * 500.0F), pos.Z);
	
	do
	{
		//
		//	Expand the box 'up' one row (if possible)
		//
		if (perimeter->stop_up == false) {
			perimeter->stop_up = true;
			
			FloodfillBoxClass *new_cell = Move_Dir (up_cell, DIR_UP, 0);
			if (new_cell != NULL) {
				if (Check_Edge (up_cell, perimeter->count_left, perimeter->count_right, 0, 0, DIR_UP)) {
					up_cell = new_cell;
					perimeter->count_up ++;
					perimeter->stop_up = false;
				}
			}
		}

		//
		//	Expand the box 'down' one row (if possible)
		//
		if (perimeter->stop_down == false) {
			perimeter->stop_down = true;

			FloodfillBoxClass *new_cell = Move_Dir (down_cell, DIR_DOWN, 0);
			if (new_cell != NULL) {
				if (Check_Edge (down_cell, perimeter->count_left, perimeter->count_right, 0, 0, DIR_DOWN)) {
					down_cell = new_cell;
					perimeter->count_down ++;
					perimeter->stop_down = false;
				}
			}
		}

		//
		//	Expand the box 'left' one column (if possible)
		//
		if (perimeter->stop_left == false) {
			perimeter->stop_left = true;

			FloodfillBoxClass *new_cell = Move_Dir (left_cell, DIR_LEFT, 0);
			if (new_cell != NULL) {
				if (Check_Edge (left_cell, 0, 0, perimeter->count_up, perimeter->count_down, DIR_LEFT)) {
					left_cell = new_cell;
					perimeter->count_left ++;
					perimeter->stop_left = false;
				}
			}
		}

		//
		//	Expand the box 'right' one column (if possible)
		//
		if (perimeter->stop_right == false) {
			perimeter->stop_right = true;

			FloodfillBoxClass *new_cell = Move_Dir (right_cell, DIR_RIGHT, 0);
			if (new_cell != NULL) {
				if (Check_Edge (right_cell, 0, 0, perimeter->count_up, perimeter->count_down, DIR_RIGHT)) {
					right_cell = new_cell;
					perimeter->count_right ++;
					perimeter->stop_right = false;
				}
			}
		}

		//
		//	Check to see if we are going to exceed the maximum sector size in either
		// direction.
		//
		if (perimeter->count_left + perimeter->count_right >= max_dim) {
			perimeter->stop_left = true;
			perimeter->stop_right = true;
		}

		if (perimeter->count_up + perimeter->count_down >= max_dim) {
			perimeter->stop_up = true;
			perimeter->stop_down = true;
		}
	}
	while (	perimeter->stop_up == false || perimeter->stop_down == false ||
				perimeter->stop_left == false || perimeter->stop_right == false);

	
	//
	//	Some debug checking to make sure we built the region
	// correctly.
	//
	FloodfillBoxClass *upper_left1 = up_cell;
	FloodfillBoxClass *upper_left2 = left_cell;
	int index;
	for (index = 0; index < perimeter->count_left; index ++) {
		upper_left1 = upper_left1->Peek_Neighbor (DIR_LEFT);
	}

	for (index = 0; index < perimeter->count_up; index ++) {
		upper_left2 = upper_left2->Peek_Neighbor (DIR_UP);
	}

	WWASSERT (upper_left1 == upper_left2);
	if (upper_left1 != upper_left2) {
		CString message;
		message.Format ("upper_left1 = %d\nupper_left2 = %d\nup_cell = %d\nleft_cell = %d", upper_left1, upper_left2, up_cell, left_cell);
		::MessageBox (NULL, message, "Info", MB_OK);
	}

	return upper_left1;
}


//////////////////////////////////////////////////////////////////////////
//
//	Compress_Sectors
//
//////////////////////////////////////////////////////////////////////////
void
PathfindSectorBuilderClass::Compress_Sectors (DynamicVectorClass<AABoxClass> *box_list)
{
	//
	//	Generate a set of maximum and minimum height values
	// for each box
	//
	int total_box_count	= Build_Height_Values ();
	int box_count			= 0;

	bool keep_going					= true;
	FloodfillBoxClass *start_box			= NULL;
	FloodfillBoxClass *old_start_box	= NULL;

	BOX_PERIMETER old_perimeter = { 0 };

	//
	//	Loop over all the boxes and compress them into rectangular regions
	//
	while (keep_going && ((start_box = FloodfillBoxClass::Get_First ()) != NULL)) {

		//
		//	Don't compress any boxes that still need processing
		//
		if (start_box->Needs_Processing ()) {
			start_box->Remove ();
			continue;
		}

		//
		//	Perform a simple check to make sure we don't keep getting the same
		// box off the top of the list (this should never, ever, ever, happen).
		//
		if (	(start_box == old_start_box) &&
				(FloodfillBoxClass::Get_First () != FloodfillBoxClass::Get_Last ()))
		{
			::MessageBox (::AfxGetMainWnd ()->m_hWnd, "Found the same box twice...", "Compression Error", MB_OK | MB_ICONERROR);
			keep_going = false;
		}
		old_start_box = start_box;

		//
		//	Find the largest rectangular region that this box can be part of
		//
		BOX_PERIMETER perimeter = { 0 };
		FloodfillBoxClass *upper_left_cell = Find_Perimeter (start_box, &perimeter);
		old_perimeter = perimeter;

		//
		//	How big is the new rectangular region?
		//
		int width	= (perimeter.count_left + perimeter.count_right) + 1;
		int height	= (perimeter.count_up + perimeter.count_down) + 1;

		//
		//	If the area is of an exceptable size, then compress it into a sector
		//
		if (width > 2 || height > 2) {
			
			int skipped_count = start_box->Get_Compress_Skipped_Count ();
			int min_dim			= std::min (width, height);

			//
			//	Should we skip this box in hopes that we find a better sector later?
			//
			if (skipped_count <= (12 - min_dim)) {
				start_box->Inc_Compress_Skipped_Count ();
				start_box->Remove ();
				FloodfillBoxClass::Add (start_box);
			} else {

				//
				//	Compress the region into a new pathfinding sector
				//
				PathfindSectorClass *sector = Build_Sector (upper_left_cell, width, height);
				if (sector != NULL) {

					//
					//	Sumbit this sector
					//
					if (box_list != NULL) {
						AABoxClass bbox = sector->Get_Bounding_Box ();
						box_list->Add (bbox);
						MEMBER_RELEASE (sector);
					} else {
						PathfindClass::Get_Instance ()->Add_Sector (sector);
					}
				}

				//
				//	Update the UI
				//
				box_count += (width * height);
				CString message;
				message.Format ("Compressed %d of %d boxes.", box_count, total_box_count);
				m_pDialog->Set_Status (message, ((float)box_count) / ((float)total_box_count));
			}

		} else {
			
			//
			//	Sector is too small, so remove the box
			// from the system.
			//
			start_box->Set_Taken ();
			start_box->Remove ();
		}
	}
	
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//	Filter_Useless_Sectors
//
//////////////////////////////////////////////////////////////////////////
/*void
PathfindSectorBuilderClass::Filter_Useless_Sectors (DynamicVectorClass<PathfindSectorClass *> &sector_list)
{
	DynamicVectorClass<PathfindSectorClass *> final_sector_list;

	for (int index = 0; index < sector_list.Count (); index ++) {
		PathfindSectorClass *sector = sector_list[index];

		bool found_usefull_portal = false;
		int portal_count = sector->Get_Portal_Count ();
		for (int portal_index = 0; portal_index < portal_count; portal_index ++) {
			
		}

		if (found_usefull_portal) {
			final_sector_list.Add (sector);
		}
	}

	//
	//	Add all the sectors to the pathfind system
	//
	for (index = 0; index < final_sector_list.Count (); index ++) {
	}

	return ;
}*/


//////////////////////////////////////////////////////////////////////////
//
//	Scan_Edge
//
//////////////////////////////////////////////////////////////////////////
void
Scan_Edge
(
	FloodfillBoxClass *	start_obj,
	int						cell_count,
	PATHFIND_DIR			check_dir,
	PATHFIND_DIR			move_dir,
	int *						best_count
)
{
	FloodfillBoxClass *	best_first_cell	= 0;

	FloodfillBoxClass *	first_cell	= NULL;
	int						count			= 0;

	//
	//	Go along the edge
	//
	FloodfillBoxClass *box_obj = start_obj;
	for (int box_index = 0; box_index < cell_count; box_index ++) {

		//
		//	Can we traverse to the left and back?
		//
		if (box_obj->Is_Two_Way_Traversible (check_dir)) {

			//
			//	Start recording this run
			//
			if (first_cell == NULL) {
				first_cell	= box_obj;
				count			= 1;
			} else {
				count ++;
			}

		} else {
			
			//
			//	Is this the best run we've found?
			//
			if (count > (*best_count)) {
				best_first_cell	= first_cell;
				(*best_count)		= count;
			}

			//
			//	Reset
			//
			first_cell	= NULL;
			count			= 0;
		}

		//
		//	Move to the next edge cell
		//
		box_obj = box_obj->Peek_Neighbor (move_dir);
	}

	//
	//	Is this the best run we've found?
	//
	if (count > (*best_count)) {
		best_first_cell	= first_cell;
		(*best_count)		= count;
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//	Is_Valid_Sector
//
//////////////////////////////////////////////////////////////////////////
bool
PathfindSectorBuilderClass::Is_Valid_Sector
(
	FloodfillBoxClass **	upper_left_ptr,
	int &						cells_right,
	int &						cells_down
)
{
	bool retval = true;	

	//
	//	Is this sector to small?
	//
	if (cells_right < 3 || cells_down < 3) {
		retval = false;

		if (cells_right < 3) {			

			FloodfillBoxClass *ul_ptr = (*upper_left_ptr);
			FloodfillBoxClass *ur_ptr = (*upper_left_ptr)->Find_Relative (cells_right - 1, 0);
			
			//
			//	Determine if either edge can enter an adjacent sector.  This
			// would make the sector walkable
			//
			int best_count = 0;
			Scan_Edge (ul_ptr, cells_down, DIR_LEFT, DIR_DOWN, &best_count);
			Scan_Edge (ur_ptr, cells_down, DIR_RIGHT, DIR_DOWN, &best_count);
			
			//
			//	Was this run good enough?
			//
			retval = (best_count >= 3);

		} else {

			FloodfillBoxClass *ul_ptr = (*upper_left_ptr);
			FloodfillBoxClass *ll_ptr = (*upper_left_ptr)->Find_Relative (0, cells_down - 1);

			//
			//	Determine if either edge can enter an adjacent sector.  This
			// would make the sector walkable
			//
			int best_count = 0;
			Scan_Edge (ul_ptr, cells_right, DIR_UP, DIR_RIGHT, &best_count);
			Scan_Edge (ll_ptr, cells_right, DIR_DOWN, DIR_RIGHT, &best_count);
			
			//
			//	Was this run good enough?
			//
			retval = (best_count >= 3);
		}
	}

	return retval;
}


//////////////////////////////////////////////////////////////////////////
//
//	Build_Sector
//
//////////////////////////////////////////////////////////////////////////
PathfindSectorClass *
PathfindSectorBuilderClass::Build_Sector
(
	FloodfillBoxClass *	upper_left_ptr,
	int				cells_right,
	int				cells_down
)
{
	Vector3 min_point (100000.0F, 100000.0F, 100000.0F);
	Vector3 max_point (-100000.0F, -100000.0F, -100000.0F);

	//
	//	Check to see if this sector meets are minimum requirements
	//
	bool is_valid = true;//Is_Valid_Sector (&upper_left_ptr, cells_right, cells_down);

	//
	//	Allocate a new sector
	//
	PathfindSectorClass *sector = NULL;
	if (is_valid) {
		sector = new PathfindSectorClass;
	}
	
	//
	//	Remove all the cells that compose this area from the list
	//
	FloodfillBoxClass *curr_cell = upper_left_ptr;
	for (int right = 0; (right < cells_right); right ++) {
		
		FloodfillBoxClass *down_obj = curr_cell;
		for (int down = 0; (down < cells_down); down ++) {
			
			AABoxClass bounding_box = Get_Body_Box_Bounds (down_obj);
			
			min_point.X = std::min (min_point.X, bounding_box.Center.X - bounding_box.Extent.X);
			min_point.Y = std::min (min_point.Y, bounding_box.Center.Y - bounding_box.Extent.Y);
			min_point.Z = std::min (min_point.Z, bounding_box.Center.Z - bounding_box.Extent.Z);

			max_point.X = std::max (max_point.X, bounding_box.Center.X + bounding_box.Extent.X);
			max_point.Y = std::max (max_point.Y, bounding_box.Center.Y + bounding_box.Extent.Y);
			max_point.Z = std::max (max_point.Z, bounding_box.Center.Z + bounding_box.Extent.Z);
			
			WWASSERT (down_obj->Is_Taken () == false);
			if (is_valid) {
				down_obj->Set_Sector (sector);
			}
			down_obj->Remove ();
			down_obj->Set_Taken ();

			//
			//	Is this an edge case? (If so its a possible portal)
			//
			if (	(down == 0) ||
					(right == 0) ||
					(down == (cells_down - 1)) ||
					(right == (cells_right - 1)))
			{
				if (is_valid) {
					m_PossiblePortalList.Add (down_obj);
				}
			}

			//
			//	Advance to the next box down
			//
			down_obj = down_obj->Peek_Neighbor (DIR_DOWN);
		}

		//
		//	Advance to the next box right
		//
		curr_cell = curr_cell->Peek_Neighbor (DIR_RIGHT);
	}

	//
	//	Calculate this sector's bounding box
	//
	if (is_valid) {
		AABoxClass bounding_box;
		bounding_box.Center		= min_point + ((max_point - min_point) * 0.5F);
		bounding_box.Extent.X	= (max_point.X - min_point.X) * 0.5F;
		bounding_box.Extent.Y	= (max_point.Y - min_point.Y) * 0.5F;
		bounding_box.Extent.Z	= (max_point.Z - min_point.Z) * 0.5F;
		sector->Set_Bounding_Box (bounding_box);
	}

	return sector;
}


//////////////////////////////////////////////////////////////////////////
//
//	Generate_Portals
//
//////////////////////////////////////////////////////////////////////////
void
PathfindSectorBuilderClass::Generate_Portals (void)
{
	//
	//	Loop through all the body-boxes that are possible portals
	//
	for (int index = 0; index < m_PossiblePortalList.Count (); index ++) {
		FloodfillBoxClass *cull_obj = m_PossiblePortalList[index];
		
		PathfindSectorClass *sector = cull_obj->Peek_Sector ();
		if ((sector != NULL) && sector->Is_Valid ()) {
			
			const AABoxClass &box	= sector->Get_Bounding_Box ();
			Vector3 portal_size		= m_SimBoundingBox;
			portal_size.Z				= box.Extent.Z * 2;
			float z_pos					= box.Center.Z - box.Extent.Z;

			//
			//	Make portals out of this box for any of the four directions
			//
			for (int dir = 0; dir < DIR_MAX; dir ++) {

				if (cull_obj->Is_New_Portal ((PATHFIND_DIR)dir)) {
					
					//
					//	Create the portal
					//
					cull_obj->Make_Portal ((PATHFIND_DIR)dir, portal_size, z_pos, 0.75F);
				}
			}
		}
	}

	m_PossiblePortalList.Delete_All ();
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Free_Floodfill_Boxes
//
///////////////////////////////////////////////////////////////////////////
void
PathfindSectorBuilderClass::Free_Floodfill_Boxes (void)
{
	//
	//	Loop over all the floodfill boxes
	//
	for (int index = 0; index < m_BodyBoxReleaseList.Count (); index ++) {
		
		//
		//	Remove the floodfill box from the culling system and
		// free our hold on it
		//
		FloodfillBoxClass *body_box = m_BodyBoxReleaseList[index];
		body_box->Reset_Portal_Info ();
		body_box->Set_Taken (false);
		body_box->Set_Sector (NULL);
		body_box->Remove ();
		REF_PTR_RELEASE (body_box);
	}

	//
	// Reset our systems
	//
	m_BodyBoxCullingSystem.Reset ();	
	m_BodyBoxReleaseList.Delete_All ();
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Cleanup_Level_Features
//
///////////////////////////////////////////////////////////////////////////
void
PathfindSectorBuilderClass::Cleanup_Level_Features (void)
{
	//
	//	Loop over all the transition objects and remove them from the
	// culling system
	//
	int count = m_LevelFeatureList.Count ();
	for (int index = 0; index < count; index ++) {
		LevelFeatureClass *cull_obj = m_LevelFeatureList[index];
		m_LevelFeatureCullingSystem.Remove_Object (cull_obj);
		MEMBER_RELEASE (cull_obj);
	}

	m_LevelFeatureList.Delete_All ();
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Detect_Level_Features
//
///////////////////////////////////////////////////////////////////////////
void
PathfindSectorBuilderClass::Detect_Level_Features (void)
{
	//
	//	Detect the different types of level features
	//
	Detect_Doors ();
	Detect_Elevators ();
	Detect_Level_Transitions ();

	//
	//	Optimize the level feature culling system
	//
	m_LevelFeatureCullingSystem.Re_Partition ();
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Detect_Elevators
//
///////////////////////////////////////////////////////////////////////////
void
PathfindSectorBuilderClass::Detect_Elevators (void)
{
	//
	//	Get the list of all static objects from the physics scene
	//
	RefPhysListIterator iterator = ::Get_Scene_Editor ()->Get_Static_Object_Iterator ();
	for (iterator.First (); !iterator.Is_Done (); iterator.Next ()) {

		//
		//	Is this object an elevator?
		//
		PhysClass *phys_obj				= iterator.Peek_Obj ();
		ElevatorPhysClass *elevator	= phys_obj->As_ElevatorPhysClass ();
		if (elevator != NULL) {
			
			const ElevatorPhysDefClass *definition = elevator->Get_ElevatorPhysDef ();

			//
			//	Get information about the elevator
			//

			OBBoxClass lower_call;
			OBBoxClass upper_call;
			OBBoxClass lower_inside;
			OBBoxClass upper_inside;

			const Matrix3D &tm = elevator->Get_Transform ();
			OBBoxClass::Transform (tm, definition->Get_Zone (ZONE_LOWER_CALL), &lower_call);
			OBBoxClass::Transform (tm, definition->Get_Zone (ZONE_UPPER_CALL), &upper_call);
			OBBoxClass::Transform (tm, definition->Get_Zone (ZONE_LOWER_INSIDE), &lower_inside);
			OBBoxClass::Transform (tm, definition->Get_Zone (ZONE_UPPER_INSIDE), &upper_inside);

			//
			//	Create a box that bounds the area the unit will actually be 'moving' in
			//
			AABoxClass bottom_box;
			AABoxClass top_box;

			bottom_box.Center	= lower_inside.Center;
			top_box.Center		= upper_inside.Center;
			top_box.Center.Z	+= m_SimBoxExtents.Z;

			lower_inside.Compute_Axis_Aligned_Extent (&bottom_box.Extent);
			upper_inside.Compute_Axis_Aligned_Extent (&top_box.Extent);

			AABoxClass teleport_zone = bottom_box;
			teleport_zone.Add_Box (top_box);			

			//
			//	Create a level feature for going UP the elevator
			//
			LevelFeatureClass *up_feature = new LevelFeatureClass;			
			up_feature->Set_Type (LevelFeatureClass::TYPE_ELEVATOR);
			up_feature->Set_Start (lower_call);
			up_feature->Set_End (upper_call);
			up_feature->Set_End_TM (Matrix3D (upper_call.Center));
			up_feature->Set_Teleport_Zone (teleport_zone);
			up_feature->Set_Mechanism_ID (elevator->Get_ID ());

			//
			//	Create a level feature for going DOWN the elevator
			//
			LevelFeatureClass *down_feature = new LevelFeatureClass;			
			down_feature->Set_Type (LevelFeatureClass::TYPE_ELEVATOR);
			down_feature->Set_Start (upper_call);
			down_feature->Set_End (lower_call);
			down_feature->Set_End_TM (Matrix3D (lower_call.Center));
			down_feature->Set_Teleport_Zone (teleport_zone);
			down_feature->Set_Mechanism_ID (elevator->Get_ID ());

			//
			//	Add the level features to the culling system
			//
			m_LevelFeatureCullingSystem.Add_Object (up_feature);
			m_LevelFeatureCullingSystem.Add_Object (down_feature);
			m_LevelFeatureList.Add (up_feature);
			m_LevelFeatureList.Add (down_feature);
		}
	}
	
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Detect_Doors
//
///////////////////////////////////////////////////////////////////////////
void
PathfindSectorBuilderClass::Detect_Doors (void)
{
	//
	//	Get the list of all static objects from the physics scene
	//
	RefPhysListIterator iterator = ::Get_Scene_Editor ()->Get_Static_Object_Iterator ();
	for (iterator.First (); !iterator.Is_Done (); iterator.Next ()) {

		//
		//	Is this object an elevator?
		//
		PhysClass *phys_obj	= iterator.Peek_Obj ();
		DoorPhysClass *door	= phys_obj->As_DoorPhysClass ();
		if (	door != NULL &&
				door->Get_DoorPhysDef () != NULL &&
				door->Get_DoorPhysDef ()->Is_Vehicle_Door () == false)
		{			
			const DoorPhysDefClass *definition = door->Get_DoorPhysDef ();

			//
			//	Get the door's trigger zones
			//					 
			const Matrix3D &tm = door->Get_Transform ();
			OBBoxClass trigger_zone1;
			OBBoxClass trigger_zone2;
			OBBoxClass::Transform (tm, definition->Get_Trigger_Zone1 (), &trigger_zone1);
			OBBoxClass::Transform (tm, definition->Get_Trigger_Zone2 (), &trigger_zone2);

			//
			//	Create a box that bounds the area the unit will actually be 'moving' in
			//
			AABoxClass box1;
			AABoxClass box2;

			box1.Center	= trigger_zone1.Center;
			box2.Center	= trigger_zone2.Center;

			trigger_zone1.Compute_Axis_Aligned_Extent (&box1.Extent);
			trigger_zone2.Compute_Axis_Aligned_Extent (&box2.Extent);

			AABoxClass teleport_zone = box1;
			teleport_zone.Add_Box (box2);

			//
			//	Create a level feature for going one direction through the door
			//
			LevelFeatureClass *feature1 = new LevelFeatureClass;			
			feature1->Set_Type (LevelFeatureClass::TYPE_DOOR);
			feature1->Set_Start (trigger_zone1);
			feature1->Set_End (trigger_zone2);
			feature1->Set_End_TM (Matrix3D (trigger_zone2.Center));
			feature1->Set_Teleport_Zone (teleport_zone);
			feature1->Set_Mechanism_ID (door->Get_ID ());

			//
			//	Create a level feature for going the other direction through the door
			//
			LevelFeatureClass *feature2 = new LevelFeatureClass;			
			feature2->Set_Type (LevelFeatureClass::TYPE_DOOR);
			feature2->Set_Start (trigger_zone2);
			feature2->Set_End (trigger_zone1);
			feature2->Set_End_TM (Matrix3D (trigger_zone1.Center));
			feature2->Set_Teleport_Zone (teleport_zone);
			feature2->Set_Mechanism_ID (door->Get_ID ());

			//
			//	Add the level features to the culling system
			//
			m_LevelFeatureCullingSystem.Add_Object (feature1);
			m_LevelFeatureCullingSystem.Add_Object (feature2);
			m_LevelFeatureList.Add (feature1);
			m_LevelFeatureList.Add (feature2);
		}
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Detect_Level_Transitions
//
///////////////////////////////////////////////////////////////////////////
void
PathfindSectorBuilderClass::Detect_Level_Transitions (void)
{
	int ladder_index = 0;

	//
	// Loop through all the transitions in the level
	//	
	TransitionNodeClass *node = NULL;
	for (node = (TransitionNodeClass *)NodeMgrClass::Get_First (NODE_TYPE_TRANSITION);
		  node != NULL;
		  node = (TransitionNodeClass *)NodeMgrClass::Get_Next (node, NODE_TYPE_TRANSITION))
	{
		//
		// Loop through all the different transitions in this node
		//		
		bool need_new_ladder_index	= true;		
		int count						= node->Get_Transition_Count ();
		for (int index = 0; index < count; index ++) {
			
			TransitionInstanceClass *transition = node->Get_Transition (index);
			if (transition != NULL) {

				//
				//	Build an AABox from the zone this transition uses
				//
				OBBoxClass zone = transition->Get_Zone ();

				//
				//	We handle each type of transition differently
				//
				switch (transition->Get_Type ())
				{
					//
					//	For ladders, we need to find the 'exit' point
					//
					case TransitionDataClass::LADDER_ENTER_TOP:
					case TransitionDataClass::LADDER_ENTER_BOTTOM:
					{
						Vector3 pos = zone.Center;

						//
						//	Insert this transition into our culling system
						//
						LevelFeatureClass *level_feature = new LevelFeatureClass;				
						level_feature->Set_Start (zone);

						//
						//	Search for the exit transition
						//
						TransitionInstanceClass *exit_transition	= NULL;
						TransitionNodeClass *exit_node				= NULL;
						if (transition->Get_Type () == TransitionDataClass::LADDER_ENTER_BOTTOM) {
							exit_transition =  Find_Transition (TransitionDataClass::LADDER_EXIT_TOP, pos, 100.0F, &exit_node);
						} else {
							exit_transition =  Find_Transition (TransitionDataClass::LADDER_EXIT_BOTTOM, pos, -100.0F, &exit_node);
						}

						//
						//	Pass the exit zone onto the level feature object
						//
						if (exit_transition != NULL) {
							level_feature->Set_End (exit_transition->Get_Zone ());
							level_feature->Set_End_TM (exit_transition->Get_Ending_TM ());

							AABoxClass box;
							box.Center		= pos;
							box.Extent		= m_SimBoxExtents;
							box.Extent.X	*= 4.0F;
							box.Extent.Y	*= 4.0F;

							//
							//	Create a box that encompasses the start position and end position
							//
							AABoxClass teleport_box (box);
							box.Center		= exit_transition->Get_Ending_TM ().Get_Translation ();
							box.Center.Z	+= m_SimBoxExtents.Z;
							teleport_box.Add_Box (box);

							//
							//	Pass the teleport area onto the level feature
							//
							level_feature->Set_Teleport_Zone (teleport_box);
						}

						//
						//	Set the level feature's type
						//
						if (transition->Get_Type () == TransitionDataClass::LADDER_ENTER_BOTTOM) {
							level_feature->Set_Type (LevelFeatureClass::TYPE_LADDER_BOTTOM);
						} else {
							level_feature->Set_Type (LevelFeatureClass::TYPE_LADDER_TOP);
						}

						int new_ladder_index = -1;

						//
						//	Can we reuse the same ladder index that the exit portal uses?
						//
						if (exit_node != NULL && exit_node->Get_Transition_Game_Obj () != NULL) {
							new_ladder_index = exit_node->Get_Transition_Game_Obj ()->Get_Ladder_Index ();
							if (new_ladder_index != -1) {
								need_new_ladder_index = false;
							}
						}

						//
						//	Increment the ladder counter as necessary
						//
						if (need_new_ladder_index) {
							need_new_ladder_index = false;
							ladder_index ++;
							new_ladder_index = ladder_index;
						}

						//
						//	Give the level feature a unique identifier
						//
						level_feature->Set_Mechanism_ID (ladder_index);
						if (node->Get_Transition_Game_Obj () != NULL) {
							node->Get_Transition_Game_Obj ()->Set_Ladder_Index (ladder_index);
						}

						//
						//	Attempt to tie the bottom and top transitions to the same value
						//
						if (exit_node != NULL && exit_node->Get_Transition_Game_Obj () != NULL) {
							exit_node->Get_Transition_Game_Obj ()->Set_Ladder_Index (ladder_index);
						}

						//
						//	Add the level feature to the culling systems
						//
						m_LevelFeatureCullingSystem.Add_Object (level_feature);
						m_LevelFeatureList.Add (level_feature);
					}											
					break;

					default:
						break;
				}			
			}
		}
	}
		
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Post_Process_Floodfill_For_Level_Features
//
///////////////////////////////////////////////////////////////////////////
void
PathfindSectorBuilderClass::Post_Process_Floodfill_For_Level_Features (void)
{
	//
	//	Loop over all the level features
	//
	for (int index = 0; index < m_LevelFeatureList.Count (); index ++) {		
		LevelFeatureClass *level_feature = m_LevelFeatureList[index];
		if (level_feature != NULL) {

			//
			//	Get the area where the player will be moving
			//
			AABoxClass box = level_feature->Get_Teleport_Zone ();

			//
			//	Loop over all the body-boxes that exist in this area
			//
			m_BodyBoxCullingSystem.Collect_Boxes (box);
			DynamicVectorClass<FloodfillBoxClass *> &list = m_BodyBoxCullingSystem.Get_Collection_List ();
			for (int index = 0; index < list.Count (); index ++) {
				FloodfillBoxClass *body_box = list[index];

				//
				//	Force the test to pass in the z-component
				//
				box.Extent.Z = 1000.0F;

				//
				//	Remove this box from the system if its completely contained
				// inside the teleport zone
				//
				AABoxClass test_box = Get_Body_Box_Bounds (body_box);
				if (CollisionMath::Overlap_Test (box, test_box) == CollisionMath::INSIDE) {

					//
					//	Remove the box from the possible portal list (if necessary)
					//
					int portal_list_index = m_PossiblePortalList.ID (body_box);
					if (portal_list_index != -1) {
						m_PossiblePortalList.Delete (portal_list_index);
					}
					
					//
					//	Remove this box from the system
					//
					m_BodyBoxReleaseList.Add (body_box);
					body_box->Unlink ();
					m_BodyBoxCullingSystem.Remove_Box (body_box);
				}
			}
		}
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Apply_Level_Features
//
///////////////////////////////////////////////////////////////////////////
void
PathfindSectorBuilderClass::Apply_Level_Features (void)
{
	//
	//	Loop over all the level features and create sectors and portals
	// for them
	//
	for (int index = 0; index < m_LevelFeatureList.Count (); index ++) {		
		LevelFeatureClass *level_feature = m_LevelFeatureList[index];
		if (level_feature != NULL) {

			//
			//	Integrate this feature into the pathfind data
			//
			Build_Sector_For_Level_Feature (level_feature);
		}
	}
	
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Find_Transition
//
///////////////////////////////////////////////////////////////////////////
TransitionInstanceClass *
PathfindSectorBuilderClass::Find_Transition
(
	TransitionDataClass::StyleType	transition_type,
	const Vector3 &						start_pos,
	float										/* z_delta */,
	TransitionNodeClass **				transition_node
)
{
	TransitionInstanceClass *retval	= NULL;
	(*transition_node)					= NULL;

	float start_z		= start_pos.Z;
	float	closest_z	= 1000000;

	//
	//	Loop over all the transitions in the level and find the one
	// that most closely matches what we are looking for
	//
	TransitionNodeClass *node = NULL;
	for (node = (TransitionNodeClass *)NodeMgrClass::Get_First (NODE_TYPE_TRANSITION);
		  node != NULL;
		  node = (TransitionNodeClass *)NodeMgrClass::Get_Next (node, NODE_TYPE_TRANSITION))
	{
		//
		//	See if we can find a transition game object inside the node
		// that is the type we are looking for.
		//
		int index = node->Find_Transition (transition_type);
		if (index >= 0) {
			
			//
			//	Get the transition game object
			//
			TransitionInstanceClass *transition = node->Get_Transition (index);
			if (transition != NULL) {

				const OBBoxClass &zone = transition->Get_Zone ();

				//
				//	Determine how 'far' we are from this transition
				//
				float curr_z_min = zone.Center.Z - zone.Extent.Z;
				float curr_z_max = zone.Center.Z + zone.Extent.Z;
				float curr_delta = std::min (WWMath::Fabs (start_z - curr_z_min), WWMath::Fabs (start_z - curr_z_max));
				
				//
				//	Is this transition zone the one we are looking for?
				//
				if (	(curr_delta < closest_z) &&
						(WWMath::Fabs (start_pos.X - zone.Center.X) < 1.0F) &&
						(WWMath::Fabs (start_pos.Y - zone.Center.Y) < 1.0F))
				{
					(*transition_node)	= node;
					retval					= transition;
					closest_z				= curr_delta;
				}
			}
		}
	}

	return retval;
}


///////////////////////////////////////////////////////////////////////////
//
//	Find_Sector
//
///////////////////////////////////////////////////////////////////////////
PathfindSectorClass *
PathfindSectorBuilderClass::Find_Sector
(
	const Vector3 &		point,
	const AABoxClass &	box
)
{
	PathfindSectorClass *sector = PathfindClass::Get_Instance ()->Find_Sector (point, 1.0F);

	if (sector == NULL) {
		sector = PathfindClass::Get_Instance ()->Find_Sector (box.Center, 1.0F);
	} 

	if (sector == NULL) {
		Vector3 box_point	= box.Center;
		box_point.X			+= box.Extent.X;
		box_point.Y			+= box.Extent.Y;
		sector				= PathfindClass::Get_Instance ()->Find_Sector (box_point, 1.0F);
	} 

	if (sector == NULL) {
		Vector3 box_point	= box.Center;
		box_point.X			-= box.Extent.X;
		box_point.Y			-= box.Extent.Y;
		sector				= PathfindClass::Get_Instance ()->Find_Sector (box_point, 1.0F);
	} 

	if (sector == NULL) {
		Vector3 box_point	= box.Center;
		box_point.X			+= box.Extent.X;
		box_point.Y			-= box.Extent.Y;
		sector				= PathfindClass::Get_Instance ()->Find_Sector (box_point, 1.0F);
	} 

	if (sector == NULL) {
		Vector3 box_point	= box.Center;
		box_point.X			-= box.Extent.X;
		box_point.Y			+= box.Extent.Y;
		sector				= PathfindClass::Get_Instance ()->Find_Sector (box_point, 1.0F);
	} 

	return sector;
}


///////////////////////////////////////////////////////////////////////////
//
//	Build_Sector_For_Level_Feature
//
///////////////////////////////////////////////////////////////////////////
void
PathfindSectorBuilderClass::Build_Sector_For_Level_Feature (LevelFeatureClass *level_feature)
{
	if (level_feature == NULL) {
		return ;
	}
				
	//
	//	Get the starting and ending positions
	//
	Matrix3D end_tm	= level_feature->Get_End_TM ();
	Vector3 end_pos	= end_tm.Get_Translation ();
	Vector3 start_pos	= level_feature->Get_Start ().Center;
		
	//
	//	Get the starting and ending AABox's for this feature
	//
	AABoxClass start_box;
	AABoxClass end_box;
	level_feature->Get_Start_AABox (start_box);
	level_feature->Get_End_AABox (end_box);
	Vector3 sector_end_pos = end_pos;
	
	//
	//	Assume the player will want to exit "away" from where (s)he entered
	//	
	if (level_feature->Get_Type () == LevelFeatureClass::TYPE_DOOR) {
		Vector3 offset = end_box.Center - start_box.Center;
		offset.Normalize ();
		sector_end_pos = end_pos + offset;
		Clip_Point (&sector_end_pos, end_box);
	}

	//
	//	Are there any pathfinding sectors intersecting the
	// start and ending regions?
	//
	PathfindSectorClass *start_sector	= Find_Sector (start_pos, start_box);
	PathfindSectorClass *end_sector		= Find_Sector (sector_end_pos, end_box);

	if (	level_feature->Get_Type () == LevelFeatureClass::TYPE_LADDER_BOTTOM ||
			level_feature->Get_Type () == LevelFeatureClass::TYPE_LADDER_TOP)
	{
		if (end_sector != NULL) {
			end_box = end_sector->Get_Bounding_Box ();
		}
	}

	//
	//	Expand the end box by a little bit
	//
	/*end_box.Extent.X += 0.25F;
	end_box.Extent.Y += 0.25F;
	end_box.Extent.Z += 0.25F;

	//
	//	Build a list of sectors that the exit should connect to
	//
	DynamicVectorClass<PathfindSectorClass *> end_sector_list;
	PathfindClass::Get_Instance ()->Collect_Sectors (end_sector_list, end_box, start_sector);*/

	//
	//	Were we successful?
	//
	if ((start_sector != NULL) && (end_sector != NULL)) {

		//
		//	Create a sector from scratch
		//
		PathfindSectorClass *new_sector = new PathfindSectorClass;
		new_sector->Set_Bounding_Box (AABoxClass (Vector3 (0, 0, 0), Vector3 (0, 0, 0)));
		PathfindClass::Get_Instance ()->Add_Sector (new_sector);
		new_sector->Release_Ref ();

		//
		//	Create the starting portal
		//
		PathfindActionPortalClass *enter_portal = new PathfindActionPortalClass;
		enter_portal->Set_Bounding_Box (start_box);
		enter_portal->Add_Dest_Sector (new_sector);
		enter_portal->Set_Destination (end_pos);

		//
		//	Create the ending portal
		//
		PathfindActionPortalClass *exit_portal = new PathfindActionPortalClass;
		exit_portal->Set_Bounding_Box (end_box);
		exit_portal->Add_Dest_Sector (end_sector);

		//
		//	Determine which type of action should be performed at this portal
		//
		switch (level_feature->Get_Type ())
		{
			case LevelFeatureClass::TYPE_LADDER_BOTTOM:
			case LevelFeatureClass::TYPE_LADDER_TOP:
			{
				AABoxClass end_trigger_zone;
				level_feature->Get_End_AABox (end_trigger_zone);
				enter_portal->Set_Destination (end_trigger_zone.Center);

				enter_portal->Set_Action_Type (PathClass::ACTION_LADDER);
				enter_portal->Set_Mechanism_ID (level_feature->Get_Mechanism_ID ());				
				exit_portal->Set_Action_Type (PathClass::ACTION_NONE);
				break;
			}

			case LevelFeatureClass::TYPE_DOOR:
			case LevelFeatureClass::TYPE_ELEVATOR:
				enter_portal->Set_Action_Type (PathClass::ACTION_MECHANISM);
				enter_portal->Set_Mechanism_ID (level_feature->Get_Mechanism_ID ());
				exit_portal->Set_Action_Type (PathClass::ACTION_NONE);
				break;
		}

		//
		//	Let the enter portal "know" where its going to end up...
		//
		enter_portal->Set_Exit_Portal (exit_portal);
		exit_portal->Set_Enter_Portal (enter_portal);
		
		//
		//	Register the new portals with the pathfinding system,
		// and let the sectors know they can access these portals.
		//
		int enter_portal_index	= PathfindClass::Get_Instance ()->Add_Portal (enter_portal);
		int exit_portal_index	= PathfindClass::Get_Instance ()->Add_Portal (exit_portal);				
		start_sector->Add_Portal (enter_portal_index);
		new_sector->Add_Portal (exit_portal_index);
		REF_PTR_RELEASE (enter_portal);
		REF_PTR_RELEASE (exit_portal);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Check_For_Level_Feature
//
///////////////////////////////////////////////////////////////////////////
void
PathfindSectorBuilderClass::Check_For_Level_Feature (FloodfillBoxClass *body_box)
{
	if (m_LevelFeatureCullingSystem.Object_Count () > 0) {
		AABoxClass bounding_box = Get_Body_Box_Bounds (body_box);

		//
		//	Find all level features that intersect our current position
		//
		m_LevelFeatureCullingSystem.Reset_Collection ();
		m_LevelFeatureCullingSystem.Collect_Objects (bounding_box);

		//
		//	Find all the features this body box intersects
		//
		DynamicVectorClass<LevelFeatureClass *> feature_list;
		LevelFeatureClass *level_feature = NULL;
		for (	level_feature = m_LevelFeatureCullingSystem.Get_First_Collected_Object ();
				level_feature != NULL;
				level_feature = m_LevelFeatureCullingSystem.Get_Next_Collected_Object (level_feature))
		{
			feature_list.Add (level_feature);
		}

		//
		//	Loop through all the features in our list and process any 'portals' (ladders, doors, etc)
		//
		for (int index = 0; index < feature_list.Count (); index ++) {
			level_feature = feature_list[index];
			
			//
			//	Get the bounding box for the pathfind object and the level feature's 'start-zone'
			//
			AABoxClass pathfind_box	= Get_Body_Box_Bounds (body_box);
			AABoxClass feature_box;
			level_feature->Get_Start_AABox (feature_box);

			//
			//	Do these boxe's intersect?
			//
			if (	(pathfind_box.Center.X + pathfind_box.Extent.X < feature_box.Center.X + feature_box.Extent.X) &&
					(pathfind_box.Center.X - pathfind_box.Extent.X > feature_box.Center.X - feature_box.Extent.X) &&
					(pathfind_box.Center.Y + pathfind_box.Extent.Y < feature_box.Center.Y + feature_box.Extent.Y) &&
					(pathfind_box.Center.Y - pathfind_box.Extent.Y > feature_box.Center.Y - feature_box.Extent.Y))
			{
				//
				//	Continue our pathfind across this feature (i.e. up the ladder, down the elevator, etc)
				//
				Path_Across_Feature (level_feature);
			}
		}
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Path_Across_Feature
//
///////////////////////////////////////////////////////////////////////////
void
PathfindSectorBuilderClass::Path_Across_Feature (LevelFeatureClass *level_feature)
{
	//
	//	Get the point where we think this feature will 'teleport' us to
	//
	Vector3 expected_pos	= level_feature->Get_End_TM ().Get_Translation ();
	expected_pos.Z			+= m_SimBoxExtents.Z;
	
	//
	//	Snap this position to our pathfind grid
	//
	Vector3 normalized_pos = expected_pos;
	normalized_pos.X = (int(expected_pos.X / m_SimBoundingBox.X)) * m_SimBoundingBox.X;
	normalized_pos.Y = (int(expected_pos.Y / m_SimBoundingBox.Y)) * m_SimBoundingBox.Y;
	
	//
	//	Can we stand at this new position?
	//
	AABoxClass new_box;
	if (Try_Standing_Here (normalized_pos, &new_box)) {

		//
		//	Is this cell already marked?
		//
		Vector3 position = new_box.Center;
		position.Z = new_box.Center.Z;

		FloodfillBoxClass *occupant = Get_Sector_Occupant (position);
		if (occupant == NULL) {

			//
			//	Mark this cell and add it to the list of sectors
			// that need to be recursed.
			//
			occupant = Mark_Sector (new_box);
			m_FloodFillProcessList.Add (occupant);
		}
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// fnPathfindDialogThread
//
////////////////////////////////////////////////////////////////////////////
UINT
fnPathfindDialogThread
(
	DWORD_PTR /* dwparam1 */,
	DWORD_PTR dwparam2,
	DWORD_PTR /*dwparam3*/,
	HRESULT* /*presult*/,
	HWND* /*phmain_wnd*/
)
{
	GeneratingPathfindDialogClass *dialog = new GeneratingPathfindDialogClass (::AfxGetMainWnd ());
	dialog->ShowWindow (SW_SHOW);

	//
	//	Return the dialog object to the caller
	//
	GeneratingPathfindDialogClass **return_val = (GeneratingPathfindDialogClass **)dwparam2;
	if (return_val != NULL) {
		(*return_val) = dialog;
	}

	return 1;
}


////////////////////////////////////////////////////////////////////////////
//
//  Show_Dialog
//
////////////////////////////////////////////////////////////////////////////
void
PathfindSectorBuilderClass::Show_Dialog (void)
{
	::Create_UI_Thread (fnPathfindDialogThread, 0, (DWORD_PTR)&m_pDialog, 0, NULL, NULL);
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Close_Dialog
//
////////////////////////////////////////////////////////////////////////////
void
PathfindSectorBuilderClass::Close_Dialog (void)
{
	if (m_pDialog != NULL) {
		::PostMessage (m_pDialog->m_hWnd, WM_USER+101, 0, 0L);
	}

	return ;
}
