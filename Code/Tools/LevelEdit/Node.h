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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/Node.h         $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 4/20/01 10:37a                                              $*
 *                                                                                             *
 *                    $Revision:: 25                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __NODE_H
#define __NODE_H

#include "refcount.h"
#include "phys.h"
#include "Utils.h"
#include "quat.h"
#include "persist.h"
#include "hittestinfo.h"
#include "preset.h"
#include "wwstring.h"

//////////////////////////////////////////////////////////////////
// Forward declarations
//////////////////////////////////////////////////////////////////
class SelectionBoxClass;
class VisPointGeneratorClass;
class ObjectNodeClass;
class TileNodeClass;
class TerrainNodeClass;
class SoundNodeClass;
class LightNodeClass;
class WaypathNodeClass;
class WaypointNodeClass;


//////////////////////////////////////////////////////////////////
//
//	NodeClass
//
//////////////////////////////////////////////////////////////////
class NodeClass : public PersistClass, public RefCountClass
{
public:

	//////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////////
	NodeClass (PresetClass *preset = NULL);
	NodeClass (const NodeClass &src);
	virtual ~NodeClass (void);

	//////////////////////////////////////////////////////////////
	//	Public operators
	//////////////////////////////////////////////////////////////
	const NodeClass &operator= (const NodeClass &src);

	//////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////
	
	//
	//	Creation methods
	//
	virtual void					Initialize (void) = 0;	
	virtual NodeClass *			Clone (void) = 0;
	
	//
	//	Identification methods
	//
	virtual NODE_TYPE				Get_Type (void) const = 0;
	virtual const char *			Get_Name (void) const;
	virtual void					Set_Name (const char *name);
	virtual uint32					Get_ID (void) const;
	virtual void					Set_ID (uint32 id);
	virtual int						Get_Icon_Index (void) const = 0;
	virtual const char *			Get_Comments (void) const;
	virtual void					Set_Comments (const char *comments);

	//
	//	RTTI
	//
	virtual ObjectNodeClass *		As_ObjectNodeClass (void)		{ return NULL; }
	virtual TileNodeClass *			As_TileNodeClass (void)			{ return NULL; }
	virtual TerrainNodeClass *		As_TerrainNodeClass (void)		{ return NULL; }
	virtual SoundNodeClass *		As_SoundNodeClass (void)		{ return NULL; }
	virtual LightNodeClass *		As_LightNodeClass (void)		{ return NULL; }
	virtual WaypathNodeClass *		As_WaypathNodeClass (void)		{ return NULL; }
	virtual WaypointNodeClass *	As_WaypointNodeClass (void)	{ return NULL; }

	//
	//	Scene methods
	//
	virtual void					Add_To_Scene (void);
	virtual void					Remove_From_Scene (void);
	virtual bool					In_Scene (void) const;
	virtual void					Reload (void);
	virtual uint32					Get_Cull_Link (void) const		{ return m_CullLink; }
	virtual void					Set_Cull_Link (uint32 link)	{ m_CullLink = link; }
	virtual void					Update_Cached_Cull_Link (void);

	//
	//	Transformation methods
	//
	virtual void					Set_Transform (const Matrix3D &tm);
	virtual Matrix3D				Get_Transform (void);
	virtual void					Set_Position (const Vector3 &pos);
	virtual Vector3				Get_Position (void) const;
	virtual void					Set_Orientation (const Quaternion &quat);
	virtual const Quaternion &	Get_Orientation (void) const;
	virtual void					Translate (const Vector3 &vector);
	virtual void					Rotate (const Matrix3D &rotation_matrix, const Matrix3D &coord_system);
	
	//
	//	Movement restriction methods
	//
	virtual bool					Can_Be_Rotated_Freely (void) const;
	virtual bool					Is_Rotation_Restricted (void) const;
	virtual void					Restrict_Rotation (bool restrict);
	virtual bool					Is_Locked (void) const;
	virtual void					Lock (bool lock);	

	//
	//	Intrinsic behavior methods
	//
	virtual bool					Is_Static (void) const = 0;

	//
	//	Notifications
	//
	virtual void					On_Rotate (void);
	virtual void					On_Translate (void);
	virtual void					On_Transform (void);
	virtual void					On_Delete (void) { }
	virtual void					On_Restore (void) { }

	//
	//	Accessors
	//
	virtual PhysClass *			Peek_Physics_Obj (void) const = 0;
	virtual RenderObjClass *	Peek_Render_Obj (void) const;
	virtual PhysClass *			Peek_Collision_Obj (void) const;
	virtual LPCTSTR				Get_Model_Name (void) const;
	void								Set_Preset (PresetClass *preset);
	PresetClass *					Get_Preset (void) const;
	uint32							Get_Preset_ID (void) const;

	//
	//	Visibility methods
	//	
	virtual void					Show_Selection_Box (bool onoff);	
	virtual void					Hide (bool hide);	
	virtual bool					Is_Hidden (void) const;

	//
	//	Attenuation methods
	//
	virtual bool					Is_Attenuation_Sphere_Shown (void)		{ return false; }
	virtual void					Show_Attenuation_Spheres (bool /* onoff */)	{ }
	virtual float					Get_Attenuation_Radius (void)				{ return 0.0F; }
	virtual void					Set_Attenuation_Radius (float /* radius */)	{ }

	//
	//	Editing methods
	//
	virtual bool					Show_Settings_Dialog (void);
	virtual NodeClass *			Add_Child_Node (const Matrix3D &/* tm */)		{ return NULL; }
	virtual NodeClass *			Get_Parent_Node (void) const				{ return NULL; }
	virtual bool					Can_Add_Child_Nodes (void)	const			{ return false; }
	virtual bool					Is_A_Child_Node (NodeClass *) const		{ return false; }
	
	
	virtual bool					Is_Proxied (void) const						{ return m_IsProxied; }
	virtual void					Set_Is_Proxied (bool onoff)				{ m_IsProxied = onoff; }
	virtual uint32					Get_Container_Preset_ID (void) const	{ return m_ContainerPresetID; }
	virtual void					Set_Container_Preset_ID (int id)			{ m_ContainerPresetID = id; }

	//
	//	Export methods
	//
	virtual void					Pre_Export (void)		{ }
	virtual void					Post_Export (void)	{ }

	//
	//	Vis-related methods
	//
	virtual void					Add_Vis_Points (VisPointGeneratorClass &generator, RenderObjClass *render_obj = NULL);
	virtual bool					Has_Vis_Sectors (RenderObjClass *render_obj = NULL);
	virtual void					Update_Cached_Vis_IDs (void) { };

	//
	//	Sub-node Enumeration
	//
	virtual int						Get_Sub_Node_Count (void) const	{ return 0; }
	virtual NodeClass *			Get_Sub_Node (int /* index */)			{ return NULL; }

	//
	//	From PersistClass
	//
	virtual bool					Save (ChunkSaveClass &csave);
	virtual bool					Load (ChunkLoadClass &cload);

	//
	//	Save filtering
	//
	virtual bool					Needs_Save (void) const			{ return m_NeedsSave; }
	virtual void					Set_Needs_Save (bool onoff)	{ m_NeedsSave = onoff; }

	//
	//	Selection box methods
	//
	virtual void					Update_Selection_Color (void);
	virtual void					Update_Selection_Box (void);

	//
	//	Zone edit methods
	//
	virtual void					On_Vertex_Drag_Begin (int /* vertex_index */)			{ }
	virtual void					On_Vertex_Drag (int /* vertex_index */, POINT /* point */)	{ }
	virtual void					On_Vertex_Drag_End (int /* vertex_index */)				{ }

protected:

	//////////////////////////////////////////////////////////////
	//	Protected methods
	//////////////////////////////////////////////////////////////
	
	//
	//	Save/load methods
	//
	bool								Load_Variables (ChunkLoadClass &cload);

	//////////////////////////////////////////////////////////////
	//	Protected member data
	//////////////////////////////////////////////////////////////
	StringClass				m_Name;
	StringClass				m_Comments;
	uint32					m_ID;
	uint32					m_PresetID;
	bool						m_IsLocked;
	bool						m_RotationRestricted;
	bool						m_IsInScene;
	bool						m_NeedsSave;
	bool						m_IsProxied;
	PresetClass *			m_Preset;
	uint32					m_ContainerPresetID;
	SelectionBoxClass *	m_SelectionBox;
	Quaternion				m_Orientation;
	Vector3					m_SelColor;	
	Matrix3D					m_Transform;
	uint32					m_CullLink;


	// For hit-test feedback
	HITTESTINFO				m_HitTestInfo;


	// For use with NodeMgrClass
	NodeClass *				m_NextNode;
	NodeClass *				m_PrevNode;

	//////////////////////////////////////////////////////////////
	//	Friends
	//////////////////////////////////////////////////////////////
	friend class NodeMgrClass;
};


//////////////////////////////////////////////////////////////////
//	Set_Transform
//////////////////////////////////////////////////////////////////
inline void
NodeClass::Set_Transform (const Matrix3D &tm)
{
	m_Transform = tm;

	PhysClass *phys_obj = Peek_Physics_Obj ();		
	if (phys_obj != NULL) {
		phys_obj->Set_Transform (tm);		
		On_Transform ();
	}
	
	return ;
}

//////////////////////////////////////////////////////////////////
//	Get_Transform
//////////////////////////////////////////////////////////////////
inline Matrix3D
NodeClass::Get_Transform (void)
{
	Matrix3D tm (1);

	PhysClass *phys_obj = Peek_Physics_Obj ();		
	if (phys_obj != NULL) {
		tm = phys_obj->Get_Transform ();
	}

	return tm;
}

//////////////////////////////////////////////////////////////////
//	Set_Position
//////////////////////////////////////////////////////////////////
inline void
NodeClass::Set_Position (const Vector3 &pos)
{
	m_Transform.Set_Translation (pos);

	PhysClass *phys_obj = Peek_Physics_Obj ();		
	if (phys_obj != NULL) {
		phys_obj->Set_Transform (m_Transform);		
		On_Translate ();
	}
	
	return ;
}

//////////////////////////////////////////////////////////////////
//	Get_Position
//////////////////////////////////////////////////////////////////
inline Vector3
NodeClass::Get_Position (void) const
{
	return m_Transform.Get_Translation ();
}

//////////////////////////////////////////////////////////////////
//	Can_Be_Rotated_Freely
//////////////////////////////////////////////////////////////////
inline bool
NodeClass::Can_Be_Rotated_Freely (void) const
{
	return true;
}

//////////////////////////////////////////////////////////////////
//	Is_Rotation_Restricted
//////////////////////////////////////////////////////////////////
inline bool
NodeClass::Is_Rotation_Restricted (void) const
{
	return m_RotationRestricted;
}

//////////////////////////////////////////////////////////////////
//	Restrict_Rotation
//////////////////////////////////////////////////////////////////
inline void
NodeClass::Restrict_Rotation (bool restrict)
{
	m_RotationRestricted = restrict;
	return ;
}

//////////////////////////////////////////////////////////////////
//	Get_Preset_ID
//////////////////////////////////////////////////////////////////
inline uint32
NodeClass::Get_Preset_ID (void) const
{
	return m_PresetID;
}

//////////////////////////////////////////////////////////////////
//	Get_Preset
//////////////////////////////////////////////////////////////////
inline PresetClass *
NodeClass::Get_Preset (void) const
{
	return m_Preset;
}

//////////////////////////////////////////////////////////////////
//	Set_Preset
//////////////////////////////////////////////////////////////////
inline void
NodeClass::Set_Preset (PresetClass *preset)
{
	m_Preset = preset;
	if (m_Preset != NULL) {
		m_PresetID = m_Preset->Get_ID ();
	}

	return ;
}

//////////////////////////////////////////////////////////////////
//	In_Scene
//////////////////////////////////////////////////////////////////
inline bool
NodeClass::In_Scene (void) const
{
	return m_IsInScene;
}

//////////////////////////////////////////////////////////////////
//	Get_ID
//////////////////////////////////////////////////////////////////
inline uint32
NodeClass::Get_ID (void) const
{
	return m_ID;
}

//////////////////////////////////////////////////////////////////
//	Peek_Collision_Obj
//////////////////////////////////////////////////////////////////
inline PhysClass *
NodeClass::Peek_Collision_Obj (void) const
{
	return Peek_Physics_Obj ();
}


INLINE_ACCESSOR_CONST (const Quaternion &,	NodeClass, Orientation);
INLINE_ACCESSOR_CONST (const char *,			NodeClass, Name);
//INLINE_ACCESSOR_CONST (uint32,					NodeClass, ID);
INLINE_ACCESSOR_CONST (const char *,			NodeClass, Comments);


#endif //__NODE_H
