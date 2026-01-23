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
 *                     $Archive:: /Commando/Code/commando/inputconfig.h         $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 7/18/01 12:44p                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __INPUT_CONFIG_H
#define __INPUT_CONFIG_H

#include "widestring.h"
#include "wwstring.h"


////////////////////////////////////////////////////////////////
//	Forward declarations
////////////////////////////////////////////////////////////////
class ChunkSaveClass;
class ChunkLoadClass;


////////////////////////////////////////////////////////////////
//
//	InputConfigClass
//
////////////////////////////////////////////////////////////////
class InputConfigClass
{
public:

	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////	
	InputConfigClass (void)	:
		IsDefault (false),
		IsCustom (false)			{}

	InputConfigClass (const InputConfigClass &src)	:
		IsDefault (false),
		IsCustom (false)			{ *this = src; }

	~InputConfigClass (void)	{}

	////////////////////////////////////////////////////////////////
	//	Public operators
	////////////////////////////////////////////////////////////////
	bool operator== (const InputConfigClass &/* src*/)	{ return false; }
	bool operator!= (const InputConfigClass &/* src*/)	{ return true; }

	const InputConfigClass &operator= (const InputConfigClass &src);

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////

	//
	//	Accessors
	//
	const unichar_t *	Get_Display_Name (void) const				{ return DisplayName; }
	const char *	Get_Filename (void) const					{ return Filename; }

	void				Set_Display_Name (const unichar_t *name)	{ DisplayName = name; }
	void				Set_Filename (const char *name)			{ Filename = name; }

	//
	//	Flags
	//
	bool				Is_Default (void) const			{ return IsDefault; }
	bool				Is_Custom (void) const			{ return IsCustom; }

	void				Set_Is_Default (bool onoff)	{ IsDefault = onoff; }
	void				Set_Is_Custom (bool onoff)		{ IsCustom = onoff; }

	//
	//	Save/load support
	//
	void				Save (ChunkSaveClass &csave);
	void				Load (ChunkLoadClass &cload);

private:

	////////////////////////////////////////////////////////////////
	//	Private methods
	////////////////////////////////////////////////////////////////
	void				Load_Variables (ChunkLoadClass &cload);

	////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////	
	WideStringClass	DisplayName;
	StringClass			Filename;
	bool					IsDefault;
	bool					IsCustom;
};


#endif //__INPUT_CONFIG_H
