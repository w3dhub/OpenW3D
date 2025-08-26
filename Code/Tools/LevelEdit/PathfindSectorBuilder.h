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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/PathfindSectorBuilder.h                                                                                                                                                                                                                                                                                                                                    $Modtime:: 5/27/00 4:10p                                               $*
 *                                                                                             *
 *                    $Revision:: 25                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef _PATHFIND_SECTOR_BUILDER_H
#define _PATHFIND_SECTOR_BUILDER_H

#include "vector3.h"
#include "matrix3d.h"
#include "vector.h"
//#include "physcontrol.h"
#include "PathfindSector.h"
#include "floodfillbox.h"
#include "transition.h"
#include "aabtreecull.h"
#include "floodfillgrid.h"
#include "heightwatcher.h"
#include "levelfeature.h"


//////////////////////////////////////////////////////////////////////////
// Forward declarations
//////////////////////////////////////////////////////////////////////////
class Phys3Class;
class SimDirInfoClass;
class ZoneInstanceClass;
class GeneratingPathfindDialogClass;
class TransitionNodeClass;


//////////////////////////////////////////////////////////////////////////
//	BOX_PERIMETER
//////////////////////////////////////////////////////////////////////////
typedef struct _BOX_PERIMETER
{
	int	count_up;
	int	count_down;
	int	count_left;
	int	count_right;

	bool	stop_up;
	bool	stop_down;
	bool	stop_left;
	bool	stop_right;

} BOX_PERIMETER;


//////////////////////////////////////////////////////////////////////////
//	Typedefs
//////////////////////////////////////////////////////////////////////////
typedef DynamicVectorClass<Vector3>								START_POINT_LIST;
typedef DynamicVectorClass<LevelFeatureClass *>				LEVEL_FEATURE_LIST;
typedef TypedAABTreeCullSystemClass<LevelFeatureClass>	LEVEL_FEATURE_CULLING_SYSTEM;


//////////////////////////////////////////////////////////////////////////
//
//	PathfindSectorBuilderClass
//
//////////////////////////////////////////////////////////////////////////
class PathfindSectorBuilderClass
{
public:

	////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////////
	PathfindSectorBuilderClass (void);
	~PathfindSectorBuilderClass (void);

	////////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////////
	
	//
	//	Initialization
	//
	void						Initialize (void);
	void						Shutdown (void);
	
	//
	//	Generation management
	//
	void						Add_Start_Point (const Vector3 &start_pos);
	void						Generate_Sectors (void);
	void						Compress_Sectors_For_Pathfind (void);
	void						Compress_Sectors_For_Dyna_Culling (void);

	//
	//	Compression control
	//
	void						Set_Max_Sector_Size (float max_dim);

	//
	//	Flags
	//
	void						Allow_Water_Floodfill (bool onoff);

protected:

	////////////////////////////////////////////////////////////////////
	//	Protected methods
	////////////////////////////////////////////////////////////////////
	void							Do_Physics_Sim (const Vector3 &start_pos, PATHFIND_DIR direction);
	void							Do_Real_Physics_Sim (const Vector3 &start_pos, PATHFIND_DIR direction);
	void							Floodfill (const Vector3 &start_pos);
	
	bool							Try_Standing_Here (const Vector3 &expected_pos, AABoxClass *real_pos);
	bool							Try_Moving_Here (const Vector3 &start_pos, const Vector3 &expected_pos, AABoxClass *real_pos);
	bool							Find_Ground (const Vector3 &pos, float *ground_pos);
	bool							Find_Ceiling (const Vector3 &pos, float *ceiling_pos);


	FloodfillBoxClass *		Get_Sector_Occupant (const Vector3 &pos);
	FloodfillBoxClass *		Mark_Sector (const AABoxClass &bounding_box);
	void							Mark_Sector (FloodfillBoxClass *body_box);
	FloodfillBoxClass *		Submit_Box (FloodfillBoxClass *from_obj, const AABoxClass &new_box, PATHFIND_DIR direction);

	PathfindSectorClass *	Build_Sector (FloodfillBoxClass *upper_left_ptr, int cells_right, int cells_down);	
	void							Generate_Portals (void);
	void							Free_Floodfill_Boxes (void);
	
	void							Determine_Height (FloodfillBoxClass *start_box, float *min_z_pos, float *max_z_pos);
	int							Build_Height_Values (void);

	void							Compress_Sectors (DynamicVectorClass<AABoxClass> *box_list = NULL);
	FloodfillBoxClass *		Find_Perimeter (FloodfillBoxClass *start_box, BOX_PERIMETER *perimeter);
	bool							Check_Edge (FloodfillBoxClass *start_box, int count_left, int count_right, int count_up, int count_down, PATHFIND_DIR move_dir);
	FloodfillBoxClass *		Move_Dir (FloodfillBoxClass *start_box, PATHFIND_DIR dir, int dir_mask);
	
	//
	//	Ladder and other transition handling
	//
	void							Post_Process_Floodfill_For_Level_Features (void);
	void							Apply_Level_Features (void);
	void							Build_Sector_For_Level_Feature (LevelFeatureClass *level_feature);
	void							Check_For_Level_Feature (FloodfillBoxClass *body_box);
	void							Path_Across_Feature (LevelFeatureClass *level_feature);
	void							Detect_Level_Features (void);
	void							Detect_Elevators (void);
	void							Detect_Doors (void);
	void							Detect_Level_Transitions (void);
	void							Cleanup_Level_Features (void);
	
	//
	//	Environment detection methods
	//
	TransitionInstanceClass *	Find_Transition (TransitionDataClass::StyleType type, const Vector3 &start_pos, float z_delta, TransitionNodeClass **transition_node);
	PathfindSectorClass *		Find_Sector (const Vector3 &point, const AABoxClass &box);
	

	//
	//	Prepartion methods
	//
	void							Prepare_Level (void);
	void							Restore_Level (void);

	//
	//	Floodfill box methods
	//
	AABoxClass					Get_Body_Box_Bounds (FloodfillBoxClass *box);

	//
	//	User interface methods
	//
	void							Show_Dialog (void);
	void							Close_Dialog (void);


	bool							Is_Valid_Sector (FloodfillBoxClass **upper_left_ptr, int &cells_right, int &cells_down);
	
private:

	////////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////////
	FloodfillBoxClass *				m_CurrentSector;
	Vector3								m_SimBoundingBox;
	Vector3								m_SimBoxExtents;
	float									m_StepHeight;
	bool									m_AllowWaterFloodfill;
	
	START_POINT_LIST					m_StartPointList;
	BODY_BOX_LIST						m_FloodFillProcessList;
	FloodfillGridClass				m_BodyBoxCullingSystem;
	LEVEL_FEATURE_LIST				m_LevelFeatureList;
	LEVEL_FEATURE_CULLING_SYSTEM	m_LevelFeatureCullingSystem;
	SimDirInfoClass *					m_DirInfo;

	BODY_BOX_LIST						m_PossiblePortalList;
	BODY_BOX_LIST						m_BodyBoxReleaseList;

	//
	//	Compression
	//
	HeightWatcherClass				m_HeightWatcher;

	// Simulation data
	Vector3								m_SimMovement;
	AABoxClass							m_SimSweepBox;

	// UI
	GeneratingPathfindDialogClass *m_pDialog;
	int									m_TotalBoxCount;
	int									m_BeforeUpdateCount;
	int									m_TotalBoxGuess;

	float									m_MaxSectorDim;
};


////////////////////////////////////////////////////////////////////
//	Get_Body_Box_Bounds
////////////////////////////////////////////////////////////////////
inline AABoxClass
PathfindSectorBuilderClass::Get_Body_Box_Bounds (FloodfillBoxClass *box)
{
	return AABoxClass (box->Get_Position (), m_SimBoxExtents);
}

////////////////////////////////////////////////////////////////////
//	Allow_Water_Floodfill
////////////////////////////////////////////////////////////////////
inline void
PathfindSectorBuilderClass::Allow_Water_Floodfill (bool onoff)
{
	m_AllowWaterFloodfill = onoff;
	return ;
}

////////////////////////////////////////////////////////////////////
//	Add_Start_Point
////////////////////////////////////////////////////////////////////
inline void
PathfindSectorBuilderClass::Add_Start_Point (const Vector3 &start_pos)
{
	m_StartPointList.Add (start_pos);
	return ;
}

////////////////////////////////////////////////////////////////////
//	Set_Max_Sector_Size
////////////////////////////////////////////////////////////////////
inline void
PathfindSectorBuilderClass::Set_Max_Sector_Size (float max_dim)
{
	m_MaxSectorDim = max_dim;
	return ;
}


#endif //_PATHFIND_SECTOR_BUILDER_H
