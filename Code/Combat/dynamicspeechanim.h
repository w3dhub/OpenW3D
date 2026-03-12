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
 *                 Project Name : Combat																		  *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Combat/dynamicspeechanim.h       $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/16/02 7:15p                                               $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __DYNAMICSPEECHANIM_H
#define __DYNAMICSPEECHANIM_H

#include "hmorphanim.h"
#include "viseme.h"


////////////////////////////////////////////////////////////////
//
//	DynamicSpeechAnimClass
//
////////////////////////////////////////////////////////////////
class DynamicSpeechAnimClass : public HMorphAnimClass
{
public:

	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	DynamicSpeechAnimClass (const char *skeleton_name);
	~DynamicSpeechAnimClass (void);

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////

	//
	//	Animation control
	//
	bool	Generate_Animation (const char *text, float duration);
	void	Generate_Idle_Animation (float duration, float frequency);

protected:

	////////////////////////////////////////////////////////////////
	//	Protected methods
	////////////////////////////////////////////////////////////////
	void	Generate_Eyebrows (float duration, float frequency = 1.0F);
	int	Insert_Blink (int current_frame, int current_pose);

	////////////////////////////////////////////////////////////////
	//	Protected member data
	////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////
	//	Static methods
	////////////////////////////////////////////////////////////////
	static VisemeManager		VisemeLookupMgr;
};


#endif //__DYNAMICSPEECHANIM_H
