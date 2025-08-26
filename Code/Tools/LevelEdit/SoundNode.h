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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/SoundNode.h       $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 7/16/01 4:49p                                               $*
 *                                                                                             *
 *                    $Revision:: 9                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __SOUND_NODE_H
#define __SOUND_NODE_H

#include "node.h"
#include "vector.h"
#include "icons.h"
#include "decophys.h"
#include "AudibleSound.h"
#include "AttenuationSphere.h"
#include "definitionclassids.h"

////////////////////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////////////////////
class PresetClass;


////////////////////////////////////////////////////////////////////////////
//
//	SoundNodeClass
//
////////////////////////////////////////////////////////////////////////////
class SoundNodeClass : public NodeClass
{
public:
	
	//////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////////////
	SoundNodeClass (PresetClass *preset = NULL);
	SoundNodeClass (const SoundNodeClass &src);
	~SoundNodeClass (void);


	//////////////////////////////////////////////////////////////
	//	Public operators
	//////////////////////////////////////////////////////////////
	const SoundNodeClass &operator= (const SoundNodeClass &src);


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
	SoundNodeClass *As_SoundNodeClass (void)			{ return this; }
	
	//
	// From NodeClass
	//
	void			Initialize (void);
	NodeClass *	Clone (void)								{ return new SoundNodeClass (*this); }
	NODE_TYPE	Get_Type (void) const					{ return NODE_TYPE_SOUND; }
	int			Get_Icon_Index (void) const			{ return SOUND_ICON; }
	PhysClass *	Peek_Physics_Obj (void)	const			{ return PhysObj; }
	bool			Is_Static (void) const					{ return false; }
	bool			Can_Be_Rotated_Freely (void) const	{ return true; }
	bool			Is_Rotation_Restricted (void) const	{ return false; }
	void			Add_To_Scene (void);
	void			Remove_From_Scene (void);
	void			Set_ID (uint32 id);
	bool			Show_Settings_Dialog (void);
	
	bool			Is_Attenuation_Sphere_Shown (void)	{ return (Sphere != NULL); }
	void			Show_Attenuation_Spheres (bool onoff);
	float			Get_Attenuation_Radius (void);
	void			Set_Attenuation_Radius (float radius);

	//
	//	Load support
	//
	void			Use_Legacy_Load (bool onoff)			{ UseLegacyLoad = onoff; }


	//
	//	Export methods
	//
	void			Pre_Export (void);
	void			Post_Export (void);

	//
	//	Notifications
	//
	void			On_Rotate (void);
	void			On_Translate (void);
	void			On_Transform (void);

protected:

	//////////////////////////////////////////////////////////////////
	//	Protected methods
	//////////////////////////////////////////////////////////////////
	void			Release_Sound (void);
	void			Update_Sound (void);

	//////////////////////////////////////////////////////////////////
	//	Protected member data
	//////////////////////////////////////////////////////////////////
	DecorationPhysClass *			PhysObj;
	AudibleSoundClass *				SoundObj;
	AttenuationSphereClass *		Sphere;
	AudibleSoundDefinitionClass	InstanceSettings;
	bool									OverridePreset;
	bool									UseLegacyLoad;
};


//////////////////////////////////////////////////////////////////
//	On_Rotate
//////////////////////////////////////////////////////////////////
inline void
SoundNodeClass::On_Rotate (void)
{
	if (SoundObj != NULL) {
		SoundObj->Set_Transform (m_Transform);
	}
	
	if (Sphere != NULL) {
		Sphere->Set_Transform (m_Transform);
	}

	NodeClass::On_Rotate ();
	return ;
}


//////////////////////////////////////////////////////////////////
//	On_Translate
//////////////////////////////////////////////////////////////////
inline void
SoundNodeClass::On_Translate (void)
{
	if (SoundObj != NULL) {
		SoundObj->Set_Transform (m_Transform);
	}

	if (Sphere != NULL) {
		Sphere->Set_Transform (m_Transform);
	}

	NodeClass::On_Translate ();
	return ;
}


//////////////////////////////////////////////////////////////////
//	On_Transform
//////////////////////////////////////////////////////////////////
inline void
SoundNodeClass::On_Transform (void)
{
	if (SoundObj != NULL) {
		SoundObj->Set_Transform (m_Transform);
	}

	if (Sphere != NULL) {
		Sphere->Set_Transform (m_Transform);
	}

	NodeClass::On_Transform ();
	return ;
}


//////////////////////////////////////////////////////////////////
//	Get_Attenuation_Radius
//////////////////////////////////////////////////////////////////
inline float
SoundNodeClass::Get_Attenuation_Radius (void)
{
	return InstanceSettings.Get_DropOff_Radius ();
}


//////////////////////////////////////////////////////////////////
//	Set_Attenuation_Radius
//////////////////////////////////////////////////////////////////
inline void
SoundNodeClass::Set_Attenuation_Radius (float radius)
{
	if (radius != InstanceSettings.Get_DropOff_Radius ()) {
		InstanceSettings.Set_DropOff_Radius (radius);
		Update_Sound ();
	}

	return ;
}


#endif //__SOUND_NODE_H

