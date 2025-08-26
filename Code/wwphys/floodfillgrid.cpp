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
 *                     $Archive:: /Commando/Code/wwphys/floodfillgrid.cpp        $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 6/16/00 11:51a                                              $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "floodfillgrid.h"
//#include "Utils.h"
#include "aabox.h"
#include "colmath.h"
#include "colmathaabox.h"


///////////////////////////////////////////////////////////////////////
//
//	FloodfillGridClass
//
///////////////////////////////////////////////////////////////////////
FloodfillGridClass::FloodfillGridClass (void)
	:	m_Grid (NULL),
		m_BoxExtent (0, 0, 0),
		m_CellSize (0, 0),
		m_CellsX (0),
		m_CellsY (0),
		m_WorldMin (0, 0),
		m_WorldMax (0, 0)
{
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	~FloodfillGridClass
//
///////////////////////////////////////////////////////////////////////
FloodfillGridClass::~FloodfillGridClass (void)
{
	Reset ();
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Reset
//
///////////////////////////////////////////////////////////////////////
void
FloodfillGridClass::Reset (void)
{
	if (m_Grid != NULL) {
		delete [] m_Grid;
		m_Grid = NULL;
	}
	m_CellsX	= 0;
	m_CellsY	= 0;
	m_CellSize.X	= 0;
	m_CellSize.Y	= 0;
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Initialize
//
///////////////////////////////////////////////////////////////////////
void
FloodfillGridClass::Initialize
(
	const Vector3 &box_extents,
	const Vector3 &min_extents,
	const Vector3 &max_extents
)
{
	Reset ();

	m_BoxExtent = box_extents;
	m_WorldMin.X = min_extents.X;
	m_WorldMin.Y = min_extents.Y;
	m_WorldMax.X = max_extents.X;
	m_WorldMax.Y = max_extents.Y;

	//
	//	Modify the min and max so they are multiples of the box extents
	//
	Vector3 box_size = box_extents * 2.0F;
	m_WorldMin.X = (int(m_WorldMin.X / box_size.X) * box_size.X) - box_extents.X;
	m_WorldMin.Y = (int(m_WorldMin.Y / box_size.Y) * box_size.Y) - box_extents.Y;
	m_WorldMax.X = (int(m_WorldMax.X / box_size.X) * box_size.X) + box_extents.X;
	m_WorldMax.Y = (int(m_WorldMax.Y / box_size.Y) * box_size.Y) + box_extents.Y;

	//
	//	Calcuate the grid dimensions
	//
	m_CellSize.X	= box_extents.X * 32;
	m_CellSize.Y	= box_extents.Y * 32;
	m_CellsX	= ((m_WorldMax.X - m_WorldMin.X) / m_CellSize.X) + 1;
	m_CellsY	= ((m_WorldMax.Y - m_WorldMin.Y) / m_CellSize.Y) + 1;
	
	//
	//	Allocate the grid
	//
	m_Grid = new FloodfillBoxClass *[m_CellsX * m_CellsY];
	::memset (m_Grid, 0, sizeof (FloodfillBoxClass *) * m_CellsX * m_CellsY);
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Collect_Boxes
//
///////////////////////////////////////////////////////////////////////
void
FloodfillGridClass::Collect_Boxes (const AABoxClass &vol)
{
	int min_cell_x = 0;
	int min_cell_y = 0;
	int max_cell_x = 0;
	int max_cell_y = 0;

	Point_To_Cell ((vol.Center - vol.Extent), &min_cell_x, &min_cell_y);
	Point_To_Cell ((vol.Center + vol.Extent), &max_cell_x, &max_cell_y);

	AABoxClass bounding_box;
	bounding_box.Extent = m_BoxExtent;

	m_CollectionList.Delete_All ();

	//
	//	Loop over all the cells this volume touches
	//
	for (int cell_y = min_cell_y; cell_y <= max_cell_y; cell_y ++) {
		for (int cell_x = min_cell_x; cell_x <= max_cell_x; cell_x ++) {
			
			//
			//	Loop over all the objects in this cell
			//
			FloodfillBoxClass *curr_box = m_Grid[cell_y * m_CellsX + cell_x];
			for (; curr_box != NULL; curr_box = curr_box->Get_Grid_Link ()) {
				bounding_box.Center = curr_box->Get_Position ();				

				//
				//	Does this box overlap the collection volume?
				//
				if (CollisionMath::Overlap_Test (vol, bounding_box) != CollisionMath::OUTSIDE) {
					m_CollectionList.Add (curr_box);
				}
			}
		}
	}

	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Find_Box
//
///////////////////////////////////////////////////////////////////////
FloodfillBoxClass *
FloodfillGridClass::Find_Box  (const Vector3 &pos)
{	
	int index = Get_Cell_Index (pos);

	FloodfillBoxClass *box = NULL;
	AABoxClass bounding_box;
	bounding_box.Extent = m_BoxExtent;

	//
	//	Loop over all the objects in this cell (looking for one inside the given point)
	//		
	for (	FloodfillBoxClass *curr_box = m_Grid[index];
			curr_box != NULL && box == NULL;
			curr_box = curr_box->Get_Grid_Link ())
	{
		bounding_box.Center = curr_box->Get_Position ();

		//
		//	Is the point inside this box?
		//
		if (	(WWMath::Fabs(pos.X - bounding_box.Center.X) <= bounding_box.Extent.X) &&
				(WWMath::Fabs(pos.Y - bounding_box.Center.Y) <= bounding_box.Extent.Y) &&
				(WWMath::Fabs(pos.Z - bounding_box.Center.Z) <= bounding_box.Extent.Z))
		{
			box = curr_box;
		}
	}

	return box;
}


///////////////////////////////////////////////////////////////////////
//
//	Convert_To_AABox - converts a floodfill box into an AABox
//
///////////////////////////////////////////////////////////////////////
AABoxClass
FloodfillGridClass::Convert_To_AABox(FloodfillBoxClass * floodbox)
{
	return AABoxClass(floodbox->Get_Position(),m_BoxExtent);
}

///////////////////////////////////////////////////////////////////////
//
//	Compute_Box_Count - counts the number of boxes that overlap into
// the specified AABox
//
///////////////////////////////////////////////////////////////////////
int
FloodfillGridClass::Compute_Box_Count(const AABoxClass & vol)
{
	int min_cell_x = 0;
	int min_cell_y = 0;
	int max_cell_x = 0;
	int max_cell_y = 0;

	Point_To_Cell ((vol.Center - vol.Extent), &min_cell_x, &min_cell_y);
	Point_To_Cell ((vol.Center + vol.Extent), &max_cell_x, &max_cell_y);

	AABoxClass bounding_box;
	bounding_box.Extent = m_BoxExtent;

	int count = 0;

	//
	//	Loop over all the cells this volume touches
	//
	for (int cell_y = min_cell_y; cell_y <= max_cell_y; cell_y ++) {
		for (int cell_x = min_cell_x; cell_x <= max_cell_x; cell_x ++) {
			
			//
			//	Loop over all the objects in this cell
			//
			FloodfillBoxClass *curr_box = m_Grid[cell_y * m_CellsX + cell_x];
			for (; curr_box != NULL; curr_box = curr_box->Get_Grid_Link ()) {
				bounding_box.Center = curr_box->Get_Position ();				

				//
				//	Does this box overlap the collection volume?
				//
				if (CollisionMath::Overlap_Test (vol, bounding_box) != CollisionMath::OUTSIDE) {
					count++;
				}
			}
		}
	}

	return count;
}



