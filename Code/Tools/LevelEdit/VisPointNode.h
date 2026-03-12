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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/VisPointNode.h      $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 3/26/01 2:46p                                               $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __VISPOINT_NODE_H
#define __VISPOINT_NODE_H

#include "node.h"
#include "vector.h"
#include "icons.h"
#include "decophys.h"

// Forward declarations
class PresetClass;


////////////////////////////////////////////////////////////////////////////
//
//	VisPointNodeClass
//
////////////////////////////////////////////////////////////////////////////
class VisPointNodeClass : public NodeClass
{
public:

	//////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////////////
	VisPointNodeClass (PresetClass *preset = NULL);
	VisPointNodeClass (const VisPointNodeClass &src);
	~VisPointNodeClass (void);

	//////////////////////////////////////////////////////////////
	//	Public operators
	//////////////////////////////////////////////////////////////
	const VisPointNodeClass &operator= (const VisPointNodeClass &src);

	//////////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////////

	//
	// VisPointNodeClass specific
	//
	void					Set_Vis_Tile_Location (const Vector3 &location);
	const Vector3 &	Get_Vis_Tile_Location (void) const;

	void					Save_Camera_Settings (const CameraClass &camera);
	void					Setup_Camera (CameraClass &camera);

	//
	// From PersistClass
	//
	virtual const PersistFactoryClass &	Get_Factory (void) const;

	//
	// From NodeClass
	//
	void			Initialize (void);
	NodeClass *	Clone (void)								{ return new VisPointNodeClass (*this); }
	NODE_TYPE	Get_Type (void) const					{ return NODE_TYPE_VIS_POINT; }
	int			Get_Icon_Index (void) const			{ return VIS_ICON; }
	PhysClass *	Peek_Physics_Obj (void)	const			{ return m_PhysObj; }
	bool			Is_Static (void) const					{ return false; }
	bool			Show_Settings_Dialog (void)			{ return true; }
	bool			Can_Be_Rotated_Freely (void) const	{ return true; }
	void			Set_Transform (const Matrix3D &tm);

	//
	//	Export methods
	//
	void			Pre_Export (void);
	void			Post_Export (void);


	//	From PersistClass
	bool			Save (ChunkSaveClass &csave);
	bool			Load (ChunkLoadClass &cload);

protected:

	//////////////////////////////////////////////////////////////////
	//	Protected methods
	//////////////////////////////////////////////////////////////////
	bool			Load_Variables (ChunkLoadClass &cload);

	//////////////////////////////////////////////////////////////////
	//	Protected member data
	//////////////////////////////////////////////////////////////////
	DecorationPhysClass *	m_PhysObj;
	Vector3						m_VisTileLocation;

	float							m_NearClipPlane;
	float							m_HFov;
	float							m_VFov;
};


//////////////////////////////////////////////////////////////////
//	Set_Vis_Tile_Location
//////////////////////////////////////////////////////////////////
inline void
VisPointNodeClass::Set_Vis_Tile_Location (const Vector3 &location)
{
	m_VisTileLocation = location;
	return ;
}


//////////////////////////////////////////////////////////////////
//	Get_Vis_Tile_Location
//////////////////////////////////////////////////////////////////
inline const Vector3 &
VisPointNodeClass::Get_Vis_Tile_Location (void) const
{
	return m_VisTileLocation;
}


#endif //__VISPOINT_NODE_H

