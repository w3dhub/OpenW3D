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
 *                     $Archive:: /Commando/Code/wwphys/floodfillbox.cpp        $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 5/03/01 5:29p                                               $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "floodfillbox.h"
#include "PathfindSector.h"
#include "PathfindPortal.h"
#include "pathfind.h"


//////////////////////////////////////////////////////////////////////////
//	Static member initialization
//////////////////////////////////////////////////////////////////////////
FloodfillBoxClass *FloodfillBoxClass::m_First = NULL;
FloodfillBoxClass *FloodfillBoxClass::m_Last = NULL;



///////////////////////////////////////////////////////////////////////////
//
//	Is_Two_Way_Traversible
//
///////////////////////////////////////////////////////////////////////////
bool
FloodfillBoxClass::Is_Two_Way_Traversible (PATHFIND_DIR dir)
{
	bool retval = false;

	FloodfillBoxClass *neighbor = Peek_Neighbor (dir);
	if (neighbor != NULL) {		
		retval = (neighbor->Peek_Neighbor (::Inverse_Pathfind_Dir (dir)) == this);
	} 

	return retval;
}


///////////////////////////////////////////////////////////////////////////
//
//	Is_New_Portal
//
///////////////////////////////////////////////////////////////////////////
bool
FloodfillBoxClass::Is_New_Portal (PATHFIND_DIR dir, PathfindSectorClass *dest_sector)
{
	bool retval = false;

	//
	//	Basically this box is part of a new 'portal' if:
	//	
	//		a) Its not already part of a portal.
	//		b) It has a neighbor in the specified direction.
	//		c) Its neighbor doesn't belong to the same sector OR
	//		c) Its neighbor is the sector we are looking for
	//
	if (	(Part_Of_Portal (dir) == false) &&
			(m_Sector != NULL) &&
			(m_Neighbors[dir] != NULL) &&
			(Get_Traversible (dir))) {
			
		if (	(dest_sector != NULL) &&
				(m_Neighbors[dir]->m_Sector == dest_sector) &&
				(m_Neighbors[dir]->m_Sector->Is_Valid ()))
		{
			retval = true;
		} else if (	(dest_sector == NULL) &&
						(m_Neighbors[dir]->m_Sector != m_Sector) &&
						(m_Neighbors[dir]->m_Sector != NULL) &&
						m_Neighbors[dir]->m_Sector->Is_Valid ())
		{
			retval = true;
		}
	}

	return retval;
}


///////////////////////////////////////////////////////////////////////////
//
//	Make_Portal
//
///////////////////////////////////////////////////////////////////////////
PathfindPortalClass *
FloodfillBoxClass::Make_Portal
(
	PATHFIND_DIR		dir,
	const Vector3 &	box_size,
	float					z_pos,
	float					min_acceptable_size
)
{
	PATHFIND_DIR slide_dir1 = PATHFIND_DIR((dir + 1) & 1);
	PATHFIND_DIR slide_dir2 = PATHFIND_DIR(slide_dir1 + 2);

	WWASSERT (m_Neighbors[dir] != NULL);
	PathfindSectorClass *dest_sector = m_Neighbors[dir]->m_Sector;

	FloodfillBoxClass *curr_box	= this;

	Vector3 min_point (100000.0F, 100000.0F, 100000.0F);
	Vector3 max_point (-100000.0F, -100000.0F, -100000.0F);

	Vector3 offset (0.0F, 0.0F, 0.0F);
	Vector3 thickness (0.0F, 0.0F, box_size.Z * 1.1F);

	if (dir == DIR_FORWARD) {
		offset.X		= box_size.X * 0.5F;
		thickness.X	= box_size.X * 0.05F;
		thickness.Y	= box_size.Y * 0.5F;
	} else if (dir == DIR_BACKWARD) {
		offset.X		= box_size.X * -0.5F;
		thickness.X	= box_size.X * -0.05F;
		thickness.Y	= box_size.Y * 0.5F;
	} else if (dir == DIR_RIGHT) {
		offset.Y		= box_size.Y * -0.5F;
		thickness.Y	= box_size.Y * 0.05F;
		thickness.X	= box_size.X * 0.5F;
	} else {
		offset.Y		= box_size.Y * 0.5F;
		thickness.Y	= box_size.Y * -0.05F;
		thickness.X	= box_size.X * 0.5F;
	}

	DynamicVectorClass<FloodfillBoxClass *> portal_list;

	//
	//	Determine if this portal is going to be a one way or a two way...
	//
	bool is_two_way = Is_Two_Way_Traversible (dir);

	//
	//	Calculate the height of the portal...
	//
	min_point.Z = z_pos;
	max_point.Z = z_pos + (box_size.Z * 1.1F);
	if (is_two_way) {
		PathfindSectorClass *sector_from	= m_Sector;
		PathfindSectorClass *sector_to	= dest_sector;
		const AABoxClass &box_from	= sector_from->Get_Bounding_Box ();
		const AABoxClass &box_to	= sector_to->Get_Bounding_Box ();

		float min_z1 = box_from.Center.Z - box_from.Extent.Z;
		float min_z2 = box_to.Center.Z - box_to.Extent.Z;
		float max_z1 = box_from.Center.Z + box_from.Extent.Z;
		float max_z2 = box_to.Center.Z + box_to.Extent.Z;
		min_point.Z = max (min_z1, min_z2);
		max_point.Z = min (max_z1, max_z2);
	}


	bool keep_going = true;

	//
	//	Find the last box in the row that is a portal to the given sector.
	//	Note:  This is inclusive of the starting box.
	//
	while (keep_going) {

		portal_list.Add (curr_box);

		Vector3 position = curr_box->Get_Position ();
		position += offset;

		min_point.X = min (min_point.X, position.X);
		min_point.Y = min (min_point.Y, position.Y);
		min_point.X = min (min_point.X, position.X + thickness.X);
		min_point.Y = min (min_point.Y, position.Y + thickness.Y);
		min_point.X = min (min_point.X, position.X - thickness.X);
		min_point.Y = min (min_point.Y, position.Y - thickness.Y);

		max_point.X = max (max_point.X, position.X);
		max_point.Y = max (max_point.Y, position.Y);
		max_point.X = max (max_point.X, position.X + thickness.X);
		max_point.Y = max (max_point.Y, position.Y + thickness.Y);
		max_point.X = max (max_point.X, position.X - thickness.X);
		max_point.Y = max (max_point.Y, position.Y - thickness.Y);

		//
		//	Should we keep going?
		//
		keep_going =	((curr_box = curr_box->Peek_Neighbor (slide_dir1)) != NULL) &&
							(curr_box->m_Sector == m_Sector) &&
							(curr_box->Is_New_Portal (dir, dest_sector)) &&
							(curr_box->Is_Two_Way_Traversible (dir) == is_two_way);
	}

	//
	//	Find the first box in the row that is a portal to the given sector.
	//
	curr_box	= this;
	while (	((curr_box = curr_box->Peek_Neighbor (slide_dir2)) != NULL) &&
				(curr_box->m_Sector == m_Sector) &&
				(curr_box->Is_New_Portal (dir, dest_sector)) &&
				(curr_box->Is_Two_Way_Traversible (dir) == is_two_way))
	{
		portal_list.Add (curr_box);

		Vector3 position = curr_box->Get_Position ();
		position += offset;

		min_point.X = min (min_point.X, position.X);
		min_point.Y = min (min_point.Y, position.Y);
		min_point.X = min (min_point.X, position.X + thickness.X);
		min_point.Y = min (min_point.Y, position.Y + thickness.Y);
		min_point.X = min (min_point.X, position.X - thickness.X);
		min_point.Y = min (min_point.Y, position.Y - thickness.Y);

		max_point.X = max (max_point.X, position.X);
		max_point.Y = max (max_point.Y, position.Y);
		max_point.X = max (max_point.X, position.X + thickness.X);
		max_point.Y = max (max_point.Y, position.Y + thickness.Y);
		max_point.X = max (max_point.X, position.X - thickness.X);
		max_point.Y = max (max_point.Y, position.Y - thickness.Y);
	}

	PathfindPortalClass *portal = NULL;

	//
	//	Choose the largest dimension
	//
	float test1 = (max_point.X - min_point.X);
	float test2 = (max_point.Y - min_point.Y);
	float test_value = max (test1, test2);	

	//
	//	Is this portal large enough?
	//
	if (test_value >= min_acceptable_size) {

		//
		//	Let each box know that it is being bound into this portal
		//
		for (int index = 0; index < portal_list.Count (); index ++) {
			FloodfillBoxClass *portal_box = portal_list[index];
			portal_box->Part_Of_Portal (dir, true);
			if (is_two_way) {
				portal_box->m_Neighbors[dir]->Part_Of_Portal (::Inverse_Pathfind_Dir (dir), true);
			}			
		}

		//
		//	Create the new portal
		//
		portal = new PathfindPortalClass;
		portal->Add_Dest_Sector (dest_sector);

		if (is_two_way) {
			portal->Add_Dest_Sector (m_Sector);
		}

		//
		//	Set the portal's bounding box
		//
		AABoxClass bounding_box;		
		bounding_box.Center = min_point + ((max_point - min_point) / 2.0F);
		
		bounding_box.Extent.X = (max_point.X - min_point.X) / 2.0F;
		bounding_box.Extent.Y = (max_point.Y - min_point.Y) / 2.0F;
		bounding_box.Extent.Z = (max_point.Z - min_point.Z) / 2.0F;

		portal->Set_Bounding_Box (bounding_box);
		
		//
		//	Register the portal with the system
		//
		int portal_index = PathfindClass::Get_Instance ()->Add_Portal (portal);
		m_Sector->Add_Portal (portal_index);
		if (is_two_way) {
			dest_sector->Add_Portal (portal_index);
		}
	}

	return portal;
}
