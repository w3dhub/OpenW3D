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
 *                 Project Name : WWPhys																		  *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwphys/pathmgr.cpp          $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 12/10/01 12:40p                                             $*
 *                                                                                             *
 *                    $Revision:: 8                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "pathmgr.h"
#include "pathsolve.h"
#include "chunkio.h"
#include "win.h"
#include "wwmemlog.h"
#include "systimer.h"


////////////////////////////////////////////////////////////////
//	Save/Load constants
////////////////////////////////////////////////////////////////
enum
{
	CHUNKID_PATH_SOLVE_OBJECT	= 0x11070935,
};


/////////////////////////////////////////////////////////////////////////
//	Static member initialization
/////////////////////////////////////////////////////////////////////////
DynamicVectorClass<PathSolveClass *>	PathMgrClass::AvailablePathList;
DynamicVectorClass<PathSolveClass *>	PathMgrClass::UsedPathList;
PathSolveClass *								PathMgrClass::ActivePath = NULL;
int64_t											PathMgrClass::TicksPerMilliSec = 0;


/////////////////////////////////////////////////////////////////////////
//	Constants
/////////////////////////////////////////////////////////////////////////
static const int DEFAULT_OBJ_COUNT	= 15;


/////////////////////////////////////////////////////////////////////////
//
//	Initialize
//
/////////////////////////////////////////////////////////////////////////
void
PathMgrClass::Initialize (void)
{
	Allocate_Objects ();

	//
	//	Determine what the resolution of our timer is
	//
	if (TicksPerMilliSec == 0) {
		::QueryPerformanceFrequency ((LARGE_INTEGER *)&TicksPerMilliSec);
		TicksPerMilliSec /= 1000;
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////
//
//	Shutdown
//
/////////////////////////////////////////////////////////////////////////
void
PathMgrClass::Shutdown (void)
{
	Free_Objects ();
	return ;
}


/////////////////////////////////////////////////////////////////////////
//
//	Free_Objects
//
/////////////////////////////////////////////////////////////////////////
void
PathMgrClass::Free_Objects (void)
{
	if (ActivePath != NULL) {
		ActivePath->Unlink_Pathfind_Hooks ();
		ActivePath = NULL;
	}

	//
	//	Free the list of available objects
	//
	int index;
	for (index = 0; index < AvailablePathList.Count (); index ++) {
		PathSolveClass *path = AvailablePathList[index];
		REF_PTR_RELEASE (path);
	}

	//
	//	Free the list of used objects
	//
	for (index = 0; index < UsedPathList.Count (); index ++) {
		PathSolveClass *path = UsedPathList[index];
		REF_PTR_RELEASE (path);
	}

	//
	//	Reset the lists
	//
	AvailablePathList.Delete_All ();
	UsedPathList.Delete_All ();
	return ;
}


/////////////////////////////////////////////////////////////////////////
//
//	Allocate_Objects
//
/////////////////////////////////////////////////////////////////////////
void
PathMgrClass::Allocate_Objects (void)
{
	Free_Objects ();

	//
	//	Allocate a default number of path objects
	//
	for (int index = 0; index < DEFAULT_OBJ_COUNT; index ++) {
		AvailablePathList.Add (new PathSolveClass);
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////
//
//	Request_Path_Object
//
/////////////////////////////////////////////////////////////////////////
PathSolveClass *
PathMgrClass::Request_Path_Object (void)
{
	WWMEMLOG(MEM_PATHFIND);
	PathSolveClass *path_object = NULL;

	int avail_count = AvailablePathList.Count ();
	if (avail_count > 0) {

		//
		//	Return the path object from the end of the list. Note: this is a
		// little more efficient when working with DynamicVectorClass objects.
		//
		path_object = AvailablePathList[avail_count - 1];
		AvailablePathList.Delete (avail_count - 1);
	} else {

		//
		//	Allocate a new object (note: we should get this object back later so
		// it will be added to our internal list at that time).
		//
		path_object = new PathSolveClass;
	}

	//
	//	Add this object to the used path list
	//
	if (path_object != NULL) {
		UsedPathList.Add (path_object);
	}

	//
	//	Return the path object to the caller
	//
	return path_object;
}


/////////////////////////////////////////////////////////////////////////
//
//	Return_Path_Object
//
/////////////////////////////////////////////////////////////////////////
void
PathMgrClass::Return_Path_Object (PathSolveClass *path)
{
	WWASSERT (path != NULL);
	if (path != NULL) {

		//
		//	Make sure the object doesn't already exist in our list
		//
		int index = AvailablePathList.ID (path);
		WWASSERT (index == -1);
		if (index == -1) {

			//
			//	Find out where the object exists in the used list
			//
			int used_index = UsedPathList.ID (path);
			WWASSERT (used_index != -1);
			if (used_index != -1) {

				//
				//	Reset our active path pointer (if necessary)
				//
				if (path == ActivePath) {
					ActivePath->Unlink_Pathfind_Hooks ();
					ActivePath = NULL;
				}

				//
				//	Remove the object from the used list
				//
				UsedPathList.Delete (used_index);

				//
				//	Add the path object to our list (its assumed we
				// will inherit the ref-count from the caller)
				//
				path->Reset_Lists ();
				AvailablePathList.Add (path);
			}
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Save
//
////////////////////////////////////////////////////////////////////////////////////////////
void
PathMgrClass::Save (ChunkSaveClass &csave)
{
	//
	//	Save each of the path objects
	//
	for (int index = 0; index < UsedPathList.Count (); index ++) {
		PathSolveClass *path = UsedPathList[index];

		csave.Begin_Chunk (CHUNKID_PATH_SOLVE_OBJECT);
			path->Save (csave);
		csave.End_Chunk ();
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Load
//
////////////////////////////////////////////////////////////////////////////////////////////
void
PathMgrClass::Load (ChunkLoadClass &cload)
{
	Free_Objects ();

	while (cload.Open_Chunk ()) {
		switch (cload.Cur_Chunk_ID ()) {

			case CHUNKID_PATH_SOLVE_OBJECT:
			{
				//
				//	Allocate the path object, load its state, and add it to our list
				//
				PathSolveClass *path_object = new PathSolveClass;
				path_object->Load (cload);
				UsedPathList.Add (path_object);
			}
			break;
		}

		cload.Close_Chunk ();
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Get_Time
//
///////////////////////////////////////////////////////////////////////////
static inline int64_t
Get_Time (void)
{
	int64_t curr_time = 0;
	::QueryPerformanceCounter ((LARGE_INTEGER *)&curr_time);
	return curr_time;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Resolve_Paths
//
////////////////////////////////////////////////////////////////////////////////////////////
void
PathMgrClass::Resolve_Paths (const Vector3 &camera_pos, uint32 milliseconds)
{
	int64_t start_time	= Get_Time ();
	int64_t end_time		= start_time + (((int64_t)milliseconds) * TicksPerMilliSec);

	WWMEMLOG(MEM_PATHFIND);

	//
	//	Keep processing path's until we've used up our timeslice
	//
	do
	{
		//
		//	Find a path that needs to be solved
		//
		if (ActivePath == NULL) {
			Activate_New_Priority_Path (camera_pos);
		}

		//
		//	Do we have any paths to solve?
		//
		if (ActivePath != NULL) {

			//
			//	Let this path think for (up to) the remainder of our timeslice
			//
			uint32 time_slice = uint32((end_time - Get_Time ()) / TicksPerMilliSec);
			PathSolveClass::STATE_DESC result = ActivePath->Timestep (time_slice);

			//
			//	If the path finished solving, then reset the active path
			//
			if (result != PathSolveClass::THINKING) {
				ActivePath->Unlink_Pathfind_Hooks ();
				ActivePath = NULL;
			}

		} else {
			break;
		}

	} while (Get_Time () < end_time);

	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Activate_New_Priority_Path
//
////////////////////////////////////////////////////////////////////////////////////////////
void
PathMgrClass::Activate_New_Priority_Path (const Vector3 &camera_pos)
{
	ActivePath				= NULL;
	float	best_priority	= 0;

	//
	//	Find the highest priority path that needs solving
	//
	for (int index = 0; index < UsedPathList.Count (); index ++) {
		PathSolveClass *path = UsedPathList[index];

		//
		//	Don't bother with paths that are already solved
		//
		if (path->Get_State () == PathSolveClass::THINKING) {

			//
			//	Get the different priority factors for this path
			//
			float dist				= (path->Get_Start_Pos () - camera_pos).Length ();
			float pos_priority	= 1.0F - WWMath::Clamp (dist / 20.0F, 0.0F, 1.0F);
			float path_priority	= WWMath::Clamp (path->Get_Priority (), 0.0F, 1.0F);
			float time_priority	= (TIMEGETTIME () - path->Get_Birth_Time ()) / 5000.0F;

			//
			//	Calculate a final priority based on these factors
			//
			float priority	= (path_priority * 0.5F) + (pos_priority * 0.5F) + time_priority;

			//
			//	If this is best path so far, then choose it
			//
			if (priority > best_priority) {
				best_priority	= priority ;
				ActivePath		= path;
			}
		}
	}

	//
	//	Kick off the pathfind
	//
	if (ActivePath != NULL) {
		ActivePath->Process_Initial_Sector ();
	}

	return ;
}