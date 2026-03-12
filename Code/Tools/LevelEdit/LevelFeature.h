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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/LevelFeature.h                                                                                                                                                                                                                                                                                                                                             $Modtime:: 5/27/00 4:10p                                               $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __LEVELFEATURE_H
#define __LEVELFEATURE_H


#include "cullsys.h"
#include "obbox.h"
#include "aabox.h"


//////////////////////////////////////////////////////////////////////////
//
//	LevelFeatureClass
//
//////////////////////////////////////////////////////////////////////////
class LevelFeatureClass : public CullableClass
{
public:

	////////////////////////////////////////////////////////////////////
	//	Public data types
	////////////////////////////////////////////////////////////////////
	typedef enum
	{
		TYPE_UNKNOWN				= 0,
		TYPE_LADDER_BOTTOM,
		TYPE_LADDER_TOP,
		TYPE_DOOR,
		TYPE_ELEVATOR,

	}	TYPE;

	////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////////
	LevelFeatureClass (void)
		:	Type (TYPE_UNKNOWN),
			MechanismID (0),
			EndTM (1),
			StartZone (Vector3 (0, 0, 0), Vector3 (0, 0, 0)),
			EndZone (Vector3 (0, 0, 0), Vector3 (0, 0, 0))		{ }

	////////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////////

	TYPE						Get_Type (void) const					{ return Type; }
	void						Set_Type (TYPE type)						{ Type = type; }

	uint32					Get_Mechanism_ID (void) const			{ return MechanismID; }
	void						Set_Mechanism_ID (uint32 id)			{ MechanismID = id; }

	const OBBoxClass &	Get_Start (void) const					{ return StartZone; }
	void						Set_Start (const OBBoxClass &zone);

	const OBBoxClass &	Get_End (void) const						{ return EndZone; }
	void						Set_End (const OBBoxClass &zone)		{ EndZone = zone; }

	const Matrix3D &		Get_End_TM (void) const					{ return EndTM; }
	void						Set_End_TM (const Matrix3D &tm)		{ EndTM = tm; }

	const AABoxClass &	Get_Teleport_Zone (void) const		{ return TeleportZone; }
	void						Set_Teleport_Zone (const AABoxClass &zone)	{ TeleportZone = zone; }

	//
	//	Conversion
	//
	void						Get_Start_AABox (AABoxClass &box);
	void						Get_End_AABox (AABoxClass &box);

private:

	////////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////////
	TYPE						Type;
	OBBoxClass				StartZone;
	OBBoxClass				EndZone;
	AABoxClass				TeleportZone;
	Matrix3D					EndTM;
	uint32					MechanismID;
};


////////////////////////////////////////////////////////////////////
//	Get_Start_AABox
////////////////////////////////////////////////////////////////////
inline void
LevelFeatureClass::Get_Start_AABox (AABoxClass &box)
{
	box.Center = StartZone.Center;
	StartZone.Compute_Axis_Aligned_Extent (&box.Extent);
	return ;
}


////////////////////////////////////////////////////////////////////
//	Get_End_AABox
////////////////////////////////////////////////////////////////////
inline void
LevelFeatureClass::Get_End_AABox (AABoxClass &box)
{
	box.Center = EndZone.Center;
	EndZone.Compute_Axis_Aligned_Extent (&box.Extent);
	return ;
}


////////////////////////////////////////////////////////////////////
//	Set_Start
////////////////////////////////////////////////////////////////////
inline void
LevelFeatureClass::Set_Start (const OBBoxClass &zone)
{
	StartZone = zone;

	//
	//	Update the bounding-box we use for culling
	//
	AABoxClass cull_box;
	Get_Start_AABox (cull_box);
	Set_Cull_Box (cull_box);
	return ;
}


#endif //__LEVELFEATURE_H

