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
 *                 Project Name : WWAudio                                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/WWAudio/StaticSoundCullObj.h      $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 5/24/99 5:08p                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __STATICSOUNDCULLOBJ_H
#define __STATICSOUNDCULLOBJ_H

#if 0
#include "SoundObj.H"
//#include "SoundCullObj.H"

/////////////////////////////////////////////////////////////////////////////
//
//	StaticSoundCullObjClass
//
//	Simple 'sound physics' object that wraps a SoundClass object and is derived
// from TileClass so it can be used with the static culling system.
//
class StaticSoundCullObjClass : public TileClass
{		
	public:

		//////////////////////////////////////////////////////////////////////
		//	Public constructors/destructors
		//////////////////////////////////////////////////////////////////////
		StaticSoundCullObjClass (void)
			: m_SoundObj (NULL),
			  m_Transform (1) {}

		virtual ~StaticSoundCullObjClass (void) { REF_PTR_RELEASE (m_SoundObj); }

		//////////////////////////////////////////////////////////////////////
		//	Get the 'bounds' of this sound
		//////////////////////////////////////////////////////////////////////
		virtual const AABoxClass & Get_Bounding_Box (void) const;

		//////////////////////////////////////////////////////////////////////
		//	Ray casting methods
		//////////////////////////////////////////////////////////////////////
		virtual bool					Cast_Ray(PhysRayCollisionTestClass & raytest)		{ return false; }
		virtual bool					Cast_AABox(PhysAABoxCollisionTestClass & boxtest)	{ return false; }
		virtual bool					Cast_OBBox(PhysOBBoxCollisionTestClass & boxtest)	{ return false; }

		//////////////////////////////////////////////////////////////////////
		//	Access to the Position/Orientation state of the object
		//////////////////////////////////////////////////////////////////////
		virtual const Matrix3D &	Get_Transform (void) const;
		virtual void					Set_Transform (const Matrix3D &transform);

		//////////////////////////////////////////////////////////////////////
		//	Timestep methods
		//////////////////////////////////////////////////////////////////////
		virtual void					Timestep (float dt) {}

		//////////////////////////////////////////////////////////////////////
		//	Sound object wrapping
		//////////////////////////////////////////////////////////////////////
		virtual void					Set_Sound_Obj (Sound3DClass *sound_obj);
		virtual Sound3DClass *		Get_Sound_Obj (void) const;
		virtual Sound3DClass *		Peek_Sound_Obj (void) const					{ return m_SoundObj; }

	protected:
		
		//////////////////////////////////////////////////////////////////////
		//	Protected methods
		//////////////////////////////////////////////////////////////////////		

	private:

		//////////////////////////////////////////////////////////////////////
		//	Private member data
		//////////////////////////////////////////////////////////////////////
		Sound3DClass *			m_SoundObj;
		mutable Matrix3D		m_Transform;
		mutable AABoxClass	m_AABox;
};


inline const Matrix3D &
StaticSoundCullObjClass::Get_Transform (void) const
{
	// Determine the transform to use
	if (m_SoundObj != NULL) {
		m_Transform = m_SoundObj->Get_Transform ();
	}

	// Return a reference to the matrix
	return m_Transform;
}


inline void
StaticSoundCullObjClass::Set_Transform (const Matrix3D &transform)
{
	m_Transform = transform;

	// Pass the tranform on
	if (m_SoundObj != NULL) {
		m_SoundObj->Set_Transform (m_Transform);
	}

	return ;
}


inline Sound3DClass *		
StaticSoundCullObjClass::Get_Sound_Obj (void) const
{
	if (m_SoundObj != NULL) {
		m_SoundObj->Add_Ref ();
	}

	// Return a pointer to the sound object
	return m_SoundObj;
}


inline void
StaticSoundCullObjClass::Set_Sound_Obj (Sound3DClass *sound_obj)
{
	// Start using this sound object
	REF_PTR_SET (m_SoundObj, sound_obj);
	if (m_SoundObj != NULL) {
		m_Transform = m_SoundObj->Get_Transform ();
	}

	return ;
}


inline const AABoxClass &
StaticSoundCullObjClass::Get_Bounding_Box (void) const
{
	// Get the 'real' values from the 
	if (m_SoundObj != NULL) {
		m_Transform = m_SoundObj->Get_Transform ();
		m_AABox.Extent.X = m_SoundObj->Get_DropOff_Radius ();
		m_AABox.Extent.Y = m_SoundObj->Get_DropOff_Radius ();
		m_AABox.Extent.Z = m_SoundObj->Get_DropOff_Radius ();
	} else {
		m_AABox.Extent.X = 0;
		m_AABox.Extent.Y = 0;
		m_AABox.Extent.Z = 0;
	}

	m_AABox.Center = m_Transform.Get_Translation ();
	return m_AABox;
}
#endif //0

#endif //__STATICSOUNDCULLOBJ_H
