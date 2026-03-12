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
 *                     $Archive:: /Commando/Code/wwphys/heightdb.h                            $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 7/31/00 3:56p                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __HEIGHT_DB_H
#define __HEIGHT_DB_H

#include "vector.h"
#include "vector3.h"
#include "wwdebug.h"


/////////////////////////////////////////////////////////////////////////
//	Forward declarations
/////////////////////////////////////////////////////////////////////////
class MeshClass;
class RenderObjClass;
class ChunkSaveClass;
class ChunkLoadClass;

/////////////////////////////////////////////////////////////////////////
//
//	HeightDBClass
//
/////////////////////////////////////////////////////////////////////////
class HeightDBClass
{
public:

	/////////////////////////////////////////////////////////////////////////
	// Public constructors/destructors
	/////////////////////////////////////////////////////////////////////////
	HeightDBClass (void);
	virtual ~HeightDBClass (void);

	/////////////////////////////////////////////////////////////////////////
	// Public methods
	/////////////////////////////////////////////////////////////////////////

	//
	//	Initialization
	//
	static void			Initialize (void);
	static void			Shutdown (void);

	//
	//	Data access
	//
	static float		Get_Height (const Vector3 &pos);

	//
	//	Generation
	//
	static void			Generate (void);

	//
	// Save/load stuff
	//
	static bool			Save (ChunkSaveClass &csave);
	static bool			Load (ChunkLoadClass &cload);

protected:

	/////////////////////////////////////////////////////////////////////////
	// Protected methods
	/////////////////////////////////////////////////////////////////////////
	static bool			Load_Variables (ChunkLoadClass &cload);
	static void			Free_Data (void);
	static float *		Get_Height_Entry (int row, int col);

	static void			Process_Render_Obj (RenderObjClass *render_obj);
	static void			Submit_Mesh (MeshClass &mesh);
	static void			Examine_Level_Geometry (void);


	/////////////////////////////////////////////////////////////////////////
	// Protected member data
	/////////////////////////////////////////////////////////////////////////
	static float *		m_HeightArray;
	static int			m_NumPointsX;
	static int			m_NumPointsY;
	static float		m_PatchSize;
	static Vector3		m_LevelMin;
	static Vector3		m_LevelMax;
};


/////////////////////////////////////////////////////////////////////////
// Get_Height_Entry
/////////////////////////////////////////////////////////////////////////
inline float *
HeightDBClass::Get_Height_Entry (int row, int col)
{
	float *retval = NULL;

	//
	//	If the row and column are valid, then return the address of the
	// entry at this location.
	//
	WWASSERT (row < m_NumPointsY && col < m_NumPointsX);
	if (row < m_NumPointsY && col < m_NumPointsX) {
		retval = &m_HeightArray[(row * m_NumPointsX) + col];
	}

	return retval;
}


#endif //__HEIGHT_DB_H
