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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/Grid3D.h                                                                                                                                                                                                                                                                                                                     $Modtime::                                                             $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __GRID_3D_H
#define __GRID_3D_H

#include "vector3.h"
#include "wwdebug.h"

//////////////////////////////////////////////////////////////////////////
//
//	Grid3DClass
//
//////////////////////////////////////////////////////////////////////////
template<class T>
class Grid3DClass
{
	public:

		////////////////////////////////////////////////////////////////////
		//	Public constructors/destructors
		////////////////////////////////////////////////////////////////////
		Grid3DClass (void)
			:	m_GridSize (0, 0, 0),
				m_Grid (NULL)				{ }

		Grid3DClass (const Vector3 &grid_size, const T &inital_value)
			:	m_GridSize (0, 0, 0),
				m_Grid (NULL)				{ Create_Grid (grid_size, inital_value); }
						
		virtual ~Grid3DClass (void)	{ Free_Grid (); }

		////////////////////////////////////////////////////////////////////
		//	Public methods
		////////////////////////////////////////////////////////////////////
		
		// Cell manipulation
		T &				Get_At (int x, int y, int z);		
		void				Set_At (int x, int y, int z, const T &value);		
		
		// Allocation routines
		void				Create_Grid (const Vector3 &grid_size, const T &initial_value);
		void				Free_Grid (void);

		// Information methods		
		Vector3			Get_Grid_Size (void) const { return m_GridSize; }
		int				Get_Cells_X (void) const { return m_GridSize.X; }
		int				Get_Cells_Y (void) const { return m_GridSize.Y; }
		int				Get_Cells_Z (void) const { return m_GridSize.Z; }

		// 'Flat' methods, for enumerating every cell
		T &				Get_At_Flat (int index)		{ return m_Grid[index]; }
		int				Get_Flat_Size (void) const	{ return (m_GridSize.X * m_GridSize.Y * m_GridSize.Z); }

	protected:
		
		////////////////////////////////////////////////////////////////////
		//	Protected methods
		////////////////////////////////////////////////////////////////////
		virtual int		Cell_Coord_To_Index (int x, int y, int z);

	private:

		////////////////////////////////////////////////////////////////////
		//	Private member data
		////////////////////////////////////////////////////////////////////
		T *		m_Grid;
		Vector3	m_GridSize;
};


//////////////////////////////////////////////////////////////////////////
//
//	Create_Grid
//
//////////////////////////////////////////////////////////////////////////
template<class T>
inline void
Grid3DClass<T>::Create_Grid (const Vector3 &grid_size, const T &inital_value)
{
	// Start fresh
	Free_Grid ();
	
	// Store the grid's dimensions
	m_GridSize = grid_size;
	
	//
	//	Allocate a flat array we can index into like a 3-D grid
	//
	int flat_size = m_GridSize.X * m_GridSize.Y * m_GridSize.Z;
	WWASSERT ((flat_size * sizeof (T)) < 16000000L);
	m_Grid = new T[flat_size];	

	//
	//	Initialize the grid cells
	//
	for (int index = 0; index < flat_size; index ++) {
		m_Grid[index] = inital_value;
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//	Free_Grid
//
//////////////////////////////////////////////////////////////////////////
template<class T>
inline void
Grid3DClass<T>::Free_Grid (void)
{
	if (m_Grid != NULL) {
		delete m_Grid;
		m_Grid = NULL;
	}

	m_GridSize.Set (0, 0, 0);	
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//	Cell_Coord_To_Index
//
//////////////////////////////////////////////////////////////////////////
template<class T>
inline int
Grid3DClass<T>::Cell_Coord_To_Index (int x, int y, int z)
{
	return (x + (y * m_GridSize.X) + (z * m_GridSize.X * m_GridSize.Y));
}


//////////////////////////////////////////////////////////////////////////
//
//	Get_At
//
//////////////////////////////////////////////////////////////////////////
template<class T>
inline T &
Grid3DClass<T>::Get_At (int x, int y, int z)
{
	WWASSERT (m_Grid != NULL);
	WWASSERT ((x < m_GridSize.X) && (y < m_GridSize.Y) && (z < m_GridSize.Z));

	return m_Grid[Cell_Coord_To_Index (x, y, z)];
}


//////////////////////////////////////////////////////////////////////////
//
//	Set_At
//
//////////////////////////////////////////////////////////////////////////
template<class T>
inline void
Grid3DClass<T>::Set_At (int x, int y, int z, const T &value)
{
	WWASSERT (m_Grid != NULL);
	WWASSERT ((x < m_GridSize.X) && (y < m_GridSize.Y) && (z < m_GridSize.Z));

	m_Grid[Cell_Coord_To_Index (x, y, z)] = value;
	return ;
}


#endif //__GRID_3D_H


