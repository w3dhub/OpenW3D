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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/LightNode.h      $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 2/14/02 5:15p                                               $*
 *                                                                                             *
 *                    $Revision:: 11                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __LIGHT_NODE_H
#define __LIGHT_NODE_H

#include "node.h"
#include "vector.h"
#include "icons.h"
#include "lightphys.h"
#include "lightdefinition.h"
#include "attenuationsphere.h"
#include "sceneeditor.h"

// Forward declarations
class PresetClass;


////////////////////////////////////////////////////////////////////////////
//
//	LightNodeClass
//
////////////////////////////////////////////////////////////////////////////
class LightNodeClass : public NodeClass
{
public:
	
	//////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////////////
	LightNodeClass (PresetClass *preset = NULL);
	LightNodeClass (const LightNodeClass &src);
	~LightNodeClass (void);


	//////////////////////////////////////////////////////////////
	//	Public operators
	//////////////////////////////////////////////////////////////
	const LightNodeClass &operator= (const LightNodeClass &src);


	//////////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////////

	//
	// From PersistClass
	//
	virtual const PersistFactoryClass &	Get_Factory (void) const;
	virtual bool								Save (ChunkSaveClass &csave);
	virtual bool								Load (ChunkLoadClass &cload);

	//
	//	RTTI
	//
	LightNodeClass *As_LightNodeClass (void)		{ return this; }

	//
	// From NodeClass
	//
	void			Initialize (void);
	NodeClass *	Clone (void)							{ return new LightNodeClass (*this); }
	NODE_TYPE	Get_Type (void) const				{ return NODE_TYPE_LIGHT; }
	int			Get_Icon_Index (void) const		{ return LIGHT_ICON; }
	PhysClass *	Peek_Physics_Obj (void)	const		{ return m_DisplayObj; }
	bool			Is_Static (void) const				{ return false; }
	bool			Show_Settings_Dialog (void);
	bool			Can_Be_Rotated_Freely (void) const	{ return true; }

	bool			Is_Attenuation_Sphere_Shown (void)	{ return (m_Sphere != NULL); }
	void			Show_Attenuation_Spheres (bool onoff);
	float			Get_Attenuation_Radius (void);
	void			Set_Attenuation_Radius (float radius);
	void			Update_Cached_Vis_IDs (void);

	//
	//	Notifications
	//
	void			On_Rotate (void);
	void			On_Translate (void);
	void			On_Transform (void);

	//
	//	Export methods
	//
	void			Pre_Export (void);
	void			Post_Export (void);

	//
	//	Scene methods
	//
	void			Add_To_Scene (void);
	void			Remove_From_Scene (void);

	//
	//	Light specific
	//
	void			Initialize_From_Light (LightClass *light);
	LightClass *Peek_Light (void);

	LightPhysClass *	Peek_Light_Phys (void)			{ return m_LightPhysObj; }

	void			Set_Group_ID (uintptr_t group_id);
	uintptr_t			Get_Group_ID (void) const;

	uint32		Get_Vis_Sector_ID (void) const		{ return m_VisSectorID; }
	void			Set_Vis_Sector_ID (uint32 vis_id);

protected:

	//////////////////////////////////////////////////////////////////
	//	Protected methods
	//////////////////////////////////////////////////////////////////
	void			Update_Light (void);
	bool			Save_Variables (ChunkSaveClass &csave);
	bool			Load_Variables (ChunkLoadClass &cload);

	//////////////////////////////////////////////////////////////////
	//	Protected member data
	//////////////////////////////////////////////////////////////////
	LightDefinitionClass m_InstanceSettings;

	LightPhysClass *				m_LightPhysObj;
	PhysClass *						m_DisplayObj;
	AttenuationSphereClass *	m_Sphere;

	bool					m_UsePreset;
	uint32				m_VisSectorID;
};


//////////////////////////////////////////////////////////////////
//	On_Rotate
//////////////////////////////////////////////////////////////////
inline void
LightNodeClass::On_Rotate (void)
{
	if (m_LightPhysObj != NULL) {
		m_LightPhysObj->Set_Transform (m_Transform);
		::Get_Scene_Editor ()->Update_Lighting ();
	}

	if (m_Sphere != NULL) {
		m_Sphere->Set_Transform (m_Transform);
	}

	NodeClass::On_Rotate ();
	return ;
}


//////////////////////////////////////////////////////////////////
//	On_Translate
//////////////////////////////////////////////////////////////////
inline void
LightNodeClass::On_Translate (void)
{
	if (m_LightPhysObj != NULL) {
		m_LightPhysObj->Set_Transform (m_Transform);
		::Get_Scene_Editor ()->Update_Lighting ();
	}

	if (m_Sphere != NULL) {
		m_Sphere->Set_Transform (m_Transform);
	}

	NodeClass::On_Translate ();
	return ;
}


//////////////////////////////////////////////////////////////////
//	On_Transform
//////////////////////////////////////////////////////////////////
inline void
LightNodeClass::On_Transform (void)
{
	if (m_LightPhysObj != NULL) {
		m_LightPhysObj->Set_Transform (m_Transform);
		::Get_Scene_Editor ()->Update_Lighting ();
	}

	if (m_Sphere != NULL) {
		m_Sphere->Set_Transform (m_Transform);
	}

	NodeClass::On_Transform ();
	return ;
}


//////////////////////////////////////////////////////////////////
//	Get_Attenuation_Radius
//////////////////////////////////////////////////////////////////
inline float
LightNodeClass::Get_Attenuation_Radius (void)
{
	return m_InstanceSettings.Get_Outer_Radius ();
}


//////////////////////////////////////////////////////////////////
//	Set_Attenuation_Radius
//////////////////////////////////////////////////////////////////
inline void
LightNodeClass::Set_Attenuation_Radius (float radius)
{
	if (radius > 0) {
		
		//
		//	Change the settings
		//
		m_UsePreset = false;	
		m_InstanceSettings.Set_Outer_Radius (radius);
		
		//
		//	Update the light model
		//
		Update_Light ();

		//
		//	Update the sphere (if necessary)
		//
		if (m_Sphere != NULL) {
			m_Sphere->Set_Radius (radius);
		}
	}

	return ;
}


//////////////////////////////////////////////////////////////////
//	Set_Group_ID
//////////////////////////////////////////////////////////////////
inline void
LightNodeClass::Set_Group_ID (uintptr_t group_id)
{
	if (m_LightPhysObj != NULL) {
		m_LightPhysObj->Set_Group_ID (group_id);
	}
		
	return ;
}


//////////////////////////////////////////////////////////////////
//	Get_Group_ID
//////////////////////////////////////////////////////////////////
inline uintptr_t
LightNodeClass::Get_Group_ID (void) const
{
	uintptr_t group_id = 0;
	if (m_LightPhysObj != NULL) {
		group_id = m_LightPhysObj->Get_Group_ID ();
	}

	return group_id;
}


#endif //__LIGHT_NODE_H

