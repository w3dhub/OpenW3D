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
 *                 Project Name : Combat																		  *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Combat/mapmgr.h           $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/10/02 10:31a                                              $*
 *                                                                                             *
 *                    $Revision:: 8                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __MAP_MGR_H
#define __MAP_MGR_H

#include "vector2.h"
#include "vector3.h"
#include "wwstring.h"
#include "bittype.h"
#include "saveloadsubsystem.h"
#include "combatchunkid.h"
#include <algorithm>


///////////////////////////////////////////////////////////////////////
// Global singleton instance
///////////////////////////////////////////////////////////////////////
extern class MapMgrClass _TheMapMgrSaveLoadSubsystem;


////////////////////////////////////////////////////////////////
//
//	MapMgrClass
//
////////////////////////////////////////////////////////////////
class MapMgrClass : public SaveLoadSubSystemClass
{
public:

	////////////////////////////////////////////////////////////////
	//	Public constants
	////////////////////////////////////////////////////////////////
	static constexpr int CLOUD_WIDTH			= 20;
	static constexpr int CLOUD_HEIGHT		= 20;
	static constexpr int CLOUD_VECTOR_SIZE	= ((CLOUD_WIDTH * CLOUD_HEIGHT) / 32) + 1;

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////

	//
	//	This is the texture that will be used to display the map
	//
	static void					Set_Map_Texture (const char *filename);
	static void					Get_Map_Texture_Filename (StringClass &filename);
	static const char *		Get_Map_Texture_Pathname (void)			{ return MapTextureName; }
	
	//
	//	Map centering and scale.
	//
	//		Note:	The scale means "how many pixels (of map) per meter (of world space)".
	//				This value can differ in the X and Y axes.
	//
	//				The map center is the pixel position (x,y) where world space (0, 0, 0) is.
	//

	static const Vector2 &	Get_Map_Scale (void)							{ return MapScale; }
	static void					Set_Map_Scale (const Vector2 &scale)	{ MapScale = scale; }

	static const Vector2 &	Get_Map_Center (void)						{ return MapCenterPoint; }
	static void					Set_Map_Center (const Vector2 &pos)		{ MapCenterPoint = pos; }

	//
	//	Cloud support
	//
	static void					Clear_Cloud_Cell (const Vector3 &pos);
	static void					Clear_Cloud_Cells (const Vector3 &pos, int pixel_radius);
	static void					Clear_Cloud_Cell (int x_pos, int y_pos);
	static void					Clear_Cloud_Cell_By_Pixel (int x_pos, int y_pos);
	static void					Clear_All_Cloud_Cells (void)				{ ::memset (CloudVector, 0, sizeof (CloudVector)); }
	static void					Cloud_All_Cells (void)						{ ::memset (CloudVector, 0xFF, sizeof (CloudVector)); }
	static bool					Is_Cell_Visible (int x_pos, int y_pos);

	//
	// VTOL Support.  Some maps do not support VTOL aircraft.
	//
	static void					Enable_VTOL_Vehicles(bool onoff)			{ EnableVTOL = onoff; }		
	static bool					Are_VTOL_Vehicles_Enabled(void)			{ return EnableVTOL; }

	//
	//	From SaveLoadSubSystemClass
	//
	uint32						Chunk_ID (void) const override			{ return CHUNKID_MAPMGR; }

	//
	//	Title support
	//
	static void					Set_Map_Title (int string_id)	{ MapTitleID = string_id; }
	static int					Get_Map_Title (void)				{ return MapTitleID; }

	//
	//	Player marker control
	//
	static bool					Is_Player_Marker_Visible (void)	{ return IsPlayerMarkerVisible; }
	static void					Show_Player_Marker (bool onoff)	{ IsPlayerMarkerVisible = onoff; }

protected:

	//
	//	From SaveLoadSubSystemClass
	//
	bool							Save (ChunkSaveClass &csave) override;
	bool							Load (ChunkLoadClass &cload) override;
	void							On_Post_Load (void) override;
	const char *				Name (void) const override					{ return "MapMgrClass"; }

	//
	//	Save load support
	//
	void							Load_Variables (ChunkLoadClass &cload);
	
private:

	////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////
	static bool				IsPlayerMarkerVisible;
	static int				MapTitleID;
	static StringClass	MapTextureName;
	static Vector2			MapCenterPoint;
	static Vector2			MapScale;
	static Vector2			MapSize;
	static uint32			CloudVector[CLOUD_VECTOR_SIZE];
	static bool				EnableVTOL;
};


////////////////////////////////////////////////////////////////
//	Clear_Cloud_Cell
////////////////////////////////////////////////////////////////
inline void
MapMgrClass::Clear_Cloud_Cell (const Vector3 &pos)
{
	if (MapSize.X <= 0 || MapSize.Y <= 0) {
		return ;
	}

	//
	//	Calculate where (in map pixels) this position lies
	//
	float map_x_pos	= MapCenterPoint.X + (pos.X * MapScale.X);
	float map_y_pos	= MapCenterPoint.Y - (pos.Y * MapScale.Y);

	//
	//	Convert this map coordinate into a cell coordinate
	//
	int cell_x	= int((map_x_pos / MapSize.X) * CLOUD_WIDTH);
	int cell_y	= int((map_y_pos / MapSize.Y) * CLOUD_HEIGHT);

	//
	//	Clamp the cells
	//
	cell_x = std::clamp (cell_x, 0, CLOUD_WIDTH - 1);
	cell_y = std::clamp (cell_y, 0, CLOUD_HEIGHT - 1);

	//
	//	Determine which bit we need to clear
	//
	int bit_offset	= (cell_y * CLOUD_WIDTH) + cell_x;
	int index		= bit_offset / 32;
	int bit			= (bit_offset - (index * 32)) + 1;

	//
	//	Clear the bit
	//
	CloudVector[index] &= ~(1 << bit);
	return ;
}


////////////////////////////////////////////////////////////////
//	Clear_Cloud_Cell
////////////////////////////////////////////////////////////////
inline void
MapMgrClass::Clear_Cloud_Cell (int x_pos, int y_pos)
{
	if (MapSize.X <= 0 || MapSize.Y <= 0) {
		return ;
	}

	//
	//	Clamp the cells
	//
	int cell_x	= std::clamp (x_pos, 0, CLOUD_WIDTH - 1);
	int cell_y	= std::clamp (y_pos, 0, CLOUD_HEIGHT - 1);

	//
	//	Determine which bit we need to clear
	//
	int bit_offset	= (cell_y * CLOUD_WIDTH) + cell_x;
	int index		= bit_offset / 32;
	int bit			= (bit_offset - (index * 32)) + 1;

	//
	//	Clear the bit
	//
	CloudVector[index] &= ~(1 << bit);
	return ;
}


////////////////////////////////////////////////////////////////
//	Clear_Cloud_Cell_By_Pixel
////////////////////////////////////////////////////////////////
inline void
MapMgrClass::Clear_Cloud_Cell_By_Pixel (int x_pos, int y_pos)
{
	if (MapSize.X <= 0 || MapSize.Y <= 0) {
		return ;
	}

	//
	//	Convert this map coordinate into a cell coordinate
	//
	int cell_x	= int((x_pos / MapSize.X) * CLOUD_WIDTH);
	int cell_y	= int((y_pos / MapSize.Y) * CLOUD_HEIGHT);

	//
	//	Clamp the cells
	//
	cell_x = std::clamp (cell_x, 0, CLOUD_WIDTH - 1);
	cell_y = std::clamp (cell_y, 0, CLOUD_HEIGHT - 1);

	//
	//	Determine which bit we need to clear
	//
	int bit_offset	= (cell_y * CLOUD_WIDTH) + cell_x;
	int index		= bit_offset / 32;
	int bit			= (bit_offset - (index * 32)) + 1;

	//
	//	Clear the bit
	//
	CloudVector[index] &= ~(1 << bit);
	return ;
}


////////////////////////////////////////////////////////////////
//	Is_Cell_Visible
////////////////////////////////////////////////////////////////
inline bool
MapMgrClass::Is_Cell_Visible (int x_pos, int y_pos)
{
	int bit_offset	= (y_pos * CLOUD_WIDTH) + x_pos;
	int index		= bit_offset / 32;
	int bit			= (bit_offset - (index * 32)) + 1;
	
	//
	//	Index into the vector to see if this cell is clouded or not
	//
	return ((CloudVector[index] & (1 << bit)) == 0);
}

#endif //__MAP_MGR_H

