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
 *                     $Archive:: /Commando/Code/WWAudio/Listener.h         $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 2/07/01 6:10p                                               $*
 *                                                                                             *
 *                    $Revision:: 7                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __LISTENER_H
#define __LISTENER_H


#include "Sound3D.h"


/////////////////////////////////////////////////////////////////////////////////
//
//	Listener3DClass
//
//	Class defining the 'listeners' 3D position/velocity in the world.  This should
// only be used by the SoundSceneClass.
//
class Listener3DClass : public Sound3DClass
{
	public:

		//////////////////////////////////////////////////////////////////////
		//	Friend classes
		//////////////////////////////////////////////////////////////////////
		friend class SoundSceneClass;

		//////////////////////////////////////////////////////////////////////
		//	Public constructors/destructors
		//////////////////////////////////////////////////////////////////////
		Listener3DClass (void);
		virtual ~Listener3DClass (void);

		//////////////////////////////////////////////////////////////////////
		//	Identification methods
		//////////////////////////////////////////////////////////////////////
		virtual SOUND_CLASSID	Get_Class_ID (void) const override	{ return CLASSID_LISTENER; }

		//////////////////////////////////////////////////////////////////////
		//	Conversion methods
		//////////////////////////////////////////////////////////////////////		
		virtual Listener3DClass *	As_Listener3DClass (void) override 	{ return this; }

		//////////////////////////////////////////////////////////////////////
		//	Initialization methods
		//////////////////////////////////////////////////////////////////////				
		virtual void			On_Added_To_Scene (void);
		virtual void			On_Removed_From_Scene (void);

		//////////////////////////////////////////////////////////////////////
		//	State control methods
		//////////////////////////////////////////////////////////////////////
		//virtual bool			Play (void)		{ return false; }
		virtual bool			Pause (void) override	{ return false; }
		virtual bool			Resume (void) override	{ return false; }
		virtual bool			Stop (bool /*remove*/) override		{ return false; }
		virtual void			Seek (unsigned int /* milliseconds */) override { }
		virtual SOUND_STATE	Get_State (void) const override	{ return STATE_STOPPED; }


		//////////////////////////////////////////////////////////////////////
		//	Attenuation settings
		//////////////////////////////////////////////////////////////////////
		virtual void			Set_Max_Vol_Radius (float /* radius */ = 0) override			{ }
		virtual float			Get_Max_Vol_Radius (void) const override					{ return 0; }
		virtual void			Set_DropOff_Radius (float /* radius */ = 1) override			{ }
		virtual float			Get_DropOff_Radius (void) const override					{ return 0; }

		//////////////////////////////////////////////////////////////////////
		//	Velocity methods
		//////////////////////////////////////////////////////////////////////				
		virtual void			Set_Velocity (const Vector3 &/* velocity */) override { }


	protected:

		//////////////////////////////////////////////////////////////////////
		//	Internal representations
		//////////////////////////////////////////////////////////////////////
		virtual void			Start_Sample (void)							{ }
		virtual void			Stop_Sample (void)							{ }
		virtual void			Resume_Sample (void)							{ }
		virtual void			End_Sample (void)								{ }
		virtual void			Set_Sample_Volume (float /* volume */)			{ }
		virtual float			Get_Sample_Volume (void)					{ return 0.0F; }
		virtual void			Set_Sample_Pan (float /* pan */)					{ }
		virtual float			Get_Sample_Pan (void)						{ return 0.5F; }
		virtual void			Set_Sample_Loop_Count (unsigned /* count */)		{ }
		virtual unsigned				Get_Sample_Loop_Count (void)				{ return 0; }
		virtual void			Set_Sample_MS_Position (unsigned /* ms */)			{ }
		virtual void			Get_Sample_MS_Position (int */* len */, int */* pos */) { }
		virtual int				Get_Sample_Playback_Rate (void)			{ return 0; }
		virtual void			Set_Sample_Playback_Rate (int /* rate */)		{ }

		//////////////////////////////////////////////////////////////////////
		//	Handle information
		//////////////////////////////////////////////////////////////////////				
		virtual void			Initialize_Miles_Handle (void) override;
		virtual void			Allocate_Miles_Handle (void) override;
		virtual void			Free_Miles_Handle (void) override;

	private:

		//////////////////////////////////////////////////////////////////////
		//	Private member data
		//////////////////////////////////////////////////////////////////////
};


#endif //__LISTENER_H
