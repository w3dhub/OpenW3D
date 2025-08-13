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
 *                 Project Name : Commando                                                     *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Combat/SoundEnvironment.h    $*
 *                                                                                             *
 *                       Author:: Ian Leslie																	  *
 *                                                                                             *
 *                     $Modtime:: 4/20/01 7:01p       $*
 *                                                                                             *
 *                    $Revision:: 5                   $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef SOUNDENVIRONMENT_H
#define SOUNDENVIRONMENT_H

// Includes.
#include "refcount.h"


// Class declarations.
class CameraClass;
class PhysicsSceneClass;


// A simple class that, upon each update, will determine of there is an object vertically
// above the camera (at any height). If so then an amplitude value is attenuated. If, the
// camera is also in an environment zone, then the amplitude is attenuated further. Amplitude
// values are in the range 0..1. This class also uses a small mixing buffer so that amplitude
// changes occur smoothly over time. 
class SoundEnvironmentClass : public RefCountClass
{
	public:
		 SoundEnvironmentClass();
		~SoundEnvironmentClass();
		
		void  Reset();
		void  Update (PhysicsSceneClass *scene, CameraClass *camera);
		float Get_Amplitude() 		{return (AmplitudeSum / AMPLITUDE_BUFFER_SIZE);}

		void Add_User()		{UserCount++;}		// Call to indicate that you want to start using this object.
		void Remove_User()	{UserCount--;}		// Call to indicate that you no longer need this object.

	protected:
		
		static constexpr int AMPLITUDE_BUFFER_SIZE = 8;

		unsigned	 UserCount;
		unsigned	 AmplitudeIndex;
		float		 AmplitudeSum;
		float		*AmplitudeBuffer;
};


#endif // SOUNDENVIRONMENT_H

