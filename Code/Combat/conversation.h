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
 *                     $Archive:: /Commando/Code/Combat/conversation.h         $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 10/29/01 10:23p                                             $*
 *                                                                                             *
 *                    $Revision:: 8                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __CONVERSATION_H
#define __CONVERSATION_H

#include "wwstring.h"
#include "vector.h"
#include "refcount.h"
#include "orator.h"
#include "soldier.h"
#include "conversationremark.h"


////////////////////////////////////////////////////////////////
//	Forward declarations
////////////////////////////////////////////////////////////////
class ChunkSaveClass;
class ChunkLoadClass;

////////////////////////////////////////////////////////////////
//
//	ConversationClass
//
////////////////////////////////////////////////////////////////
class ConversationClass : public RefCountClass
{
public:

	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	ConversationClass (void);
	ConversationClass (const ConversationClass &src);
	virtual ~ConversationClass (void);

	////////////////////////////////////////////////////////////////
	//	Public operators
	////////////////////////////////////////////////////////////////
	const ConversationClass &	operator= (const ConversationClass &src);

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////

	//
	//	Save/load methods
	//
	bool				Save (ChunkSaveClass &csave);
	bool				Load (ChunkLoadClass &cload);

	//
	//	Identification methods
	//
	int				Get_ID (void) const					{ return ID; }
	void				Set_ID (int id)						{ ID = id; }

	const char *	Get_Name (void) const				{ return Name; }
	void				Set_Name (const char *name)		{ Name = name; }

	//
	//	Orator information
	//
	int				Get_Orator_Count (void) const;
	OratorClass *	Get_Orator (int index);
	OratorClass *	Find_Orator (int orator_id);
	void				Add_Orator (const OratorClass &orator);
	void				Clear_Orators (void);

	//
	//	State requirements
	//
	SoldierAIState	Get_AI_State( void ) const					{ return AIState; }
	void				Set_AI_State( SoldierAIState state )	{ AIState = state; }


	//
	//	Remark access
	//
	int				Get_Remark_Count (void) const		{ return RemarkList.Count (); }
	bool				Get_Remark_Info (int index, ConversationRemarkClass &remark);
	void				Add_Remark (const ConversationRemarkClass &remark);
	void				Clear_Remarks (void);

	//
	//	Accessors
	//
	bool				Is_Innate (void) const				{ return IsInnate; }
	void				Set_Is_Innate (bool is_innate)	{ IsInnate = is_innate; }

	bool				Is_Key (void) const					{ return IsKey; }
	void				Set_Is_Key (bool is_key)			{ IsKey = is_key; }

	float				Get_Probability (void) const		{ return Probability; }
	void				Set_Probability (float percent)	{ Probability = percent; }

	int				Get_Category_ID (void) const		{ return CategoryID; }
	void				Set_Category_ID (int id)			{ CategoryID = id; }

	int				Get_Look_At_Obj_ID (void) const	{ return LookAtObjID; }
	void				Set_Look_At_Obj_ID (int id)		{ LookAtObjID = id; }

	int				Get_Priority (void) const			{ return Priority; }
	void				Set_Priority (int priority)		{ Priority = priority; }

	float				Get_Max_Dist (void) const			{ return MaxDist; }
	void				Set_Max_Dist (float max_dist)		{ MaxDist = max_dist; }

	bool				Is_Interruptable (void) const			{ return IsInterruptable; }
	void				Set_Is_Interruptable (bool onoff)	{ IsInterruptable = onoff; }

protected:

	////////////////////////////////////////////////////////////////
	//	Protected methods
	////////////////////////////////////////////////////////////////
	void				Load_Variables (ChunkLoadClass &cload);

	////////////////////////////////////////////////////////////////
	//	Protected member data
	////////////////////////////////////////////////////////////////
	StringClass								Name;
	int										ID;
	DynamicVectorClass<ConversationRemarkClass>	RemarkList;
	DynamicVectorClass<OratorClass>					OratorList;
	SoldierAIState							AIState;
	bool										IsInnate;
	bool										IsKey;
	float										Probability;
	int										CategoryID;
	int										LookAtObjID;

	int										Priority;
	float										MaxDist;
	bool										IsInterruptable;
};


#endif //__CONVERSATION_H
