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
 *                 Project Name : WWPhys                                                       *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwphys/vissectorstats.h                      $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Patrick                                                     $*
 *                                                                                             *
 *                     $Modtime:: 6/07/00 11:33a                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef VISSECTORSTATS_H
#define VISSECTORSTATS_H

#include "always.h"
#include "vector3.h"

class StaticPhysClass;
class VisTableClass;


/**
** VisSectorStatsClass
** This object is used to compute some statistics about the given vis sector.  For example,
** it stores how many polygons are in the vis-set from anywhere in the given sector.  
*/
class VisSectorStatsClass
{
public:

	VisSectorStatsClass(void);
	VisSectorStatsClass(const VisSectorStatsClass &src);
	~VisSectorStatsClass(void);

	const VisSectorStatsClass &	operator= (const VisSectorStatsClass &src);

	void					Compute_Stats(StaticPhysClass * obj,VisTableClass * vistbl);

	int					Get_Vis_Id(void);
	int					Get_Polygon_Count(void);
	int					Get_Texture_Count(void);
	int					Get_Texture_Bytes(void);
	Vector3				Get_Position(void);
	StaticPhysClass *	Peek_Phys_Obj(void);
	const char *		Get_Name(void);

	//
	// For use with DynamicVectorClass
	//
	bool operator== (const VisSectorStatsClass &/* src*/)	{ return false; }
	bool operator!= (const VisSectorStatsClass &/* src*/)	{ return true; }

protected:

	int						VisId;
	int						PolygonCount;
	int						TextureCount;
	int						TextureBytes;
	Vector3					CenterPoint;
	StaticPhysClass *		PhysObj;
};


/*
** inlines
*/

inline int VisSectorStatsClass::Get_Vis_Id(void)
{
	return VisId;
}

inline int VisSectorStatsClass::Get_Polygon_Count(void)
{
	return PolygonCount;
}

inline int VisSectorStatsClass::Get_Texture_Count(void)
{
	return TextureCount;
}

inline int VisSectorStatsClass::Get_Texture_Bytes(void)
{
	return TextureBytes;
}

inline Vector3 VisSectorStatsClass::Get_Position(void)
{
	return CenterPoint;
}

inline StaticPhysClass *VisSectorStatsClass::Peek_Phys_Obj(void)
{
	return PhysObj;
}


#endif // VISSECTORSTATS_H

