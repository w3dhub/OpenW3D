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
 *                     $Archive:: /Commando/Code/Combat/conversationremark.h       $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/31/01 11:11a                                              $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __CONVERSATIONREMARK_H
#define __CONVERSATIONREMARK_H

#include "wwstring.h"
#include "simplevec.h"
#include "refcount.h"
#include "orator.h"
#include "soldier.h"

////////////////////////////////////////////////////////////////
//	Forward declarations
////////////////////////////////////////////////////////////////
class ChunkSaveClass;
class ChunkLoadClass;


////////////////////////////////////////////////////////////////
//
//	ConversationRemarkClass
//
////////////////////////////////////////////////////////////////
class ConversationRemarkClass
{
public:

	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	ConversationRemarkClass (void);
	ConversationRemarkClass (const ConversationRemarkClass &src);
	virtual ~ConversationRemarkClass (void);

	////////////////////////////////////////////////////////////////
	//	Public operators
	////////////////////////////////////////////////////////////////
	const ConversationRemarkClass &	operator= (const ConversationRemarkClass &src);
	bool	operator== (const ConversationRemarkClass &src)		{ return (TextID == src.TextID) && (OratorID == src.OratorID); }
	bool	operator!= (const ConversationRemarkClass &src)		{ return (TextID != src.TextID) || (OratorID != src.OratorID); }

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////

	//
	//	Save/Load methods
	//
	bool				Save (ChunkSaveClass &csave);
	bool				Load (ChunkLoadClass &cload);

	//
	//	Accessors
	//
	int				Get_Orator_ID (void) const			{ return OratorID; }
	void				Set_Orator_ID (int id)				{ OratorID = id; }

	int				Get_Text_ID (void) const			{ return TextID; }
	void				Set_Text_ID (int id)					{ TextID = id; }

	const StringClass &	Get_Animation_Name (void) const				{ return AnimationName; }
	void						Set_Animation_Name (const char *name)		{ AnimationName = name; }

protected:

	////////////////////////////////////////////////////////////////
	//	Protected methods
	////////////////////////////////////////////////////////////////
	void				Load_Variables (ChunkLoadClass &cload);


	////////////////////////////////////////////////////////////////
	//	Protected data types
	////////////////////////////////////////////////////////////////
	int				OratorID;
	int				TextID;
	StringClass		AnimationName;
};


#endif //__CONVERSATIONREMARK_H
