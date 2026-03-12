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
 *                 Project Name : ww3d2																		  *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/animatedsoundmgr.h                     $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 12/13/01 6:05p                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __ANIMATEDSOUNDMGR_H
#define __ANIMATEDSOUNDMGR_H

#include "simplevec.h"
#include "vector.h"
#include "hashtemplate.h"


//////////////////////////////////////////////////////////////////////
//	Forward declarations
//////////////////////////////////////////////////////////////////////
class HTreeClass;
class HAnimClass;
class Matrix3D;


//////////////////////////////////////////////////////////////////////
//
//	AnimatedSoundMgrClass
//
//////////////////////////////////////////////////////////////////////
class AnimatedSoundMgrClass
{
public:

	///////////////////////////////////////////////////////////////////
	//	Public methods
	///////////////////////////////////////////////////////////////////

	//
	//	Initialization and shutdown
	//
	static void		Initialize (const char *ini_filename = NULL);
	static void		Shutdown (void);

	//
	//	Sound playback
	//
	static bool		Does_Animation_Have_Embedded_Sounds (HAnimClass *anim);
	static float	Trigger_Sound (HAnimClass *anim, float old_frame, float new_frame, const Matrix3D &tm);

private:

	///////////////////////////////////////////////////////////////////
	//	Private data types
	///////////////////////////////////////////////////////////////////
	typedef struct
	{
		int		Frame;
		int		SoundDefinitionID;
	} ANIM_SOUND_INFO;

	typedef SimpleDynVecClass<ANIM_SOUND_INFO>	ANIM_SOUND_LIST;

	///////////////////////////////////////////////////////////////////
	//	Private member data
	///////////////////////////////////////////////////////////////////
	static HashTemplateClass<StringClass, ANIM_SOUND_LIST *> AnimationNameHash;
	static DynamicVectorClass<ANIM_SOUND_LIST *>					AnimSoundLists;

	///////////////////////////////////////////////////////////////////
	//	Private methods
	///////////////////////////////////////////////////////////////////
	static ANIM_SOUND_LIST *	Find_Sound_List (HAnimClass *anim);
};


#endif //__ANIMATEDSOUNDMGR_H
