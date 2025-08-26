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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/NodeSettingsInterface.h                                                                                                                                                                                                                                                                                                                                          $Modtime::                                                             $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif


#ifndef __NODE_SETTINGS_INTERFACE_H
#define __NODE_SETTINGS_INTERFACE_H

#include "vector3.h"

/////////////////////////////////////////////////////////////////////////////
//
//	LightSettingsClass
//
//	Abstract virtual class that defines an interface for manipulating light
// specific settings.
//
class LightSettingsClass
{
	public:
		
		///////////////////////////////////////////////////
		//
		//	Public methods
		//
		
		//
		// Information retrieval methods
		//
		virtual const Vector3 &			Get_Ambient_Color (void) const = 0;
		virtual const Vector3 &			Get_Diffuse_Color (void) const = 0;
		virtual const Vector3 &			Get_Specular_Color (void) const = 0;
		virtual float						Get_Intensity (void) const = 0;
		virtual void						Get_Attenuation (float &inner, float &outer) const = 0;

		//
		// Information manipulation methods
		//
		virtual void						Set_Ambient_Color (const Vector3 &color) = 0;
		virtual void						Set_Diffuse_Color (const Vector3 &color) = 0;
		virtual void						Set_Specular_Color (const Vector3 &color) = 0;
		virtual void						Set_Intensity (float intensity) = 0;
		virtual void						Set_Attenuation (float inner, float outer) = 0;

		//
		// Update methods
		//
		virtual void						Update_Light (void) {}
};


/////////////////////////////////////////////////////////////////////////////
//
//	SoundSettingsClass
//
//	Abstract virtual class that defines an interface for manipulating sound
// specific settings.
//
class SoundSettingsClass
{
	public:
		
		///////////////////////////////////////////////////
		//
		//	Public methods
		//
		
		//
		// Information retrieval methods
		//

		//
		// Information manipulation methods
		//

		//
		// Update methods
		//
		virtual void						Update_Sound (void) {}

		virtual float						Get_Volume (void) const = 0;
		virtual void						Set_Volume (float volume)	= 0;

		virtual int							Get_Loop_Count (void) const = 0;
		virtual void						Set_Loop_Count (int count)	= 0;

		virtual bool						Is_Music (void) const = 0;
		virtual void						Set_Music (bool is_music) = 0;

		virtual bool						Is_3D (void) const = 0;
		virtual void						Set_3D (bool is_3d) = 0;
		
		virtual float						Get_Priority (void) const = 0;
		virtual void						Set_Priority (float priority)	= 0;
		
		virtual void						Set_Max_Vol_Radius (float radius = 0) = 0;
		virtual float						Get_Max_Vol_Radius (void) const = 0;
		
		virtual void						Set_DropOff_Radius (float radius = 1) = 0;
		virtual float						Get_DropOff_Radius (void) const = 0;
		
		virtual void						Set_Filename (LPCTSTR filename) = 0;
		virtual LPCTSTR					Get_Filename (void) const = 0;

};


#endif //__NODE_SETTINGS_INTERFACE_H

