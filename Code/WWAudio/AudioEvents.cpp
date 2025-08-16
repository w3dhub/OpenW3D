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
 *                     $Archive:: /Commando/Code/WWAudio/AudioEvents.cpp                      $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 11/02/01 11:58a                                             $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "AudioEvents.h"
#include "SoundSceneObj.h"


/////////////////////////////////////////////////////////////////////////////////
//
//	AudioCallbackClass
//
/////////////////////////////////////////////////////////////////////////////////
AudioCallbackClass::AudioCallbackClass (void)
{
	return ;
}


/////////////////////////////////////////////////////////////////////////////////
//
//	~AudioCallbackClass
//
/////////////////////////////////////////////////////////////////////////////////
AudioCallbackClass::~AudioCallbackClass (void)
{
	Remove_All_Callbacks ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////////
//
//	On_Registered
//
/////////////////////////////////////////////////////////////////////////////////
void
AudioCallbackClass::On_Registered (SoundSceneObjClass *sound)
{
	if (SoundList.ID (sound) == -1) {
		SoundList.Add (sound);
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////////
//
//	On_UnRegistered
//
/////////////////////////////////////////////////////////////////////////////////
void
AudioCallbackClass::On_UnRegistered (SoundSceneObjClass *sound)
{
	int index = SoundList.ID (sound);
	if (index != -1) {
		SoundList.Delete (index);
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////////
//
//	Remove_All_Callbacks
//
/////////////////////////////////////////////////////////////////////////////////
void
AudioCallbackClass::Remove_All_Callbacks (void)
{
	//
	//	Simply remove ourselves from the sound object
	//
	for (int index = 0; index < SoundList.Count (); index ++) {
		SoundList[index]->Remove_Callback ();
	}

	SoundList.Delete_All ();
	return ;
}
