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
 *                     $Archive:: /Commando/Code/wwphys/PathfindSectorBuilder.h                                                                                                                                   $Revision:: 2                                                           $*
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
#include "physcontrol.h"
#include "PathfindSector.h"
#include "pathfindbodybox.h"
#include "gridcull.h"

// Forward declarations
class Phys3Class;
class SimDirInfoClass;


//////////////////////////////////////////////////////////////////////////
//
//	Typedefs
//
//////////////////////////////////////////////////////////////////////////
typedef DynamicVectorClass<PathfindSectorClass *>	SECTOR_LIST;
typedef TypedGridCullSystemClass<BodyBoxCullObj>	BODY_BOX_CULLING_SYSTEM;



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
		void					Generate_Sectors (const Vector3 &start_pos);

	protected:

		////////////////////////////////////////////////////////////////////
		//	Protected methods
		////////////////////////////////////////////////////////////////////
		void					Do_Physics_Sim (const Vector3 &start_pos, PATHFIND_DIR direction);
		void					Floodfill (const Vector3 &start_pos);

		BodyBoxCullObj *	Get_Sector_Occupant (const Vector3 &pos);
		BodyBoxCullObj *	Mark_Sector (const Vector3 &pos);
		void					Mark_Sector (BodyBoxCullObj *body_box);

		void					Compress_Sectors (void);
		void					Free_Sectors (void);
		
		void					Import_Raw_Data (void);
		
	private:

		////////////////////////////////////////////////////////////////////
		//	Private member data
		////////////////////////////////////////////////////////////////////
		BodyBoxCullObj *					m_CurrentSector;
		Phys3Class *						m_PhysicsSim;
		PhysControllerClass				m_Controller;		
		Vector3								m_SimBoundingBox;
		int									m_RecurseLevel;
		int									m_RepartitionCount;
		bool									m_bCancel;		
		
		SECTOR_LIST							m_SectorList;
		BODY_BOX_LIST						m_FloodFillProcessList;
		BODY_BOX_CULLING_SYSTEM			m_BodyBoxCullingSystem;
		SimDirInfoClass *					m_DirInfo;
};


#endif //_PATHFIND_SECTOR_BUILDER_H

