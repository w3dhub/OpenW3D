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
 *                     $Archive:: /Commando/Code/Combat/conversation.cpp        $*
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

#include "conversation.h"
#include "chunkio.h"
#include "soldier.h"


////////////////////////////////////////////////////////////////
//	Constants
////////////////////////////////////////////////////////////////
enum
{
	CHUNKID_BASE_CLASS		= 0x08090315,
	CHUNKID_VARIABLES,
	CHUNKID_ORATOR,
	CHUNKID_REMARK
};

enum
{
	VARID_NAME					= 0,
	VARID_ID,
	XXXX_VARID_REMARK,
	VARID_OLD_PTR,
	XXXXX_PLAYERTYPE,
	VARID_ISINNATE,
	VARID_PROBABILITY,
	VARID_AI_STATE,
	VARID_CATEGORY_ID,
	VARID_LOOKAT_OBJID,
	VARID_PRIORITY,
	VARID_MAXDIST,
	VARID_IS_INTERRUPTABLE,
	VARID_ISKEY
};


////////////////////////////////////////////////////////////////
//
//	ConversationClass
//
////////////////////////////////////////////////////////////////
ConversationClass::ConversationClass (void)	:
	ID (0),
	IsInnate (true),
	IsKey (false),
	Probability (1.0F),
	AIState (AI_STATE_IDLE),
	CategoryID (0),
	LookAtObjID (0),
	Priority (30),
	MaxDist (0),
	IsInterruptable (true)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	ConversationClass
//
////////////////////////////////////////////////////////////////
ConversationClass::ConversationClass (const ConversationClass &src)	:
	ID (0),
	IsInnate (true),
	IsKey (false),
	Probability (1.0F),
	AIState (AI_STATE_IDLE),
	CategoryID (0),
	LookAtObjID (0),
	Priority (30),
	MaxDist (0),
	IsInterruptable (true)

{
	(*this) = src;
	return ;
}


////////////////////////////////////////////////////////////////
//
//	~ConversationClass
//
////////////////////////////////////////////////////////////////
ConversationClass::~ConversationClass (void)
{
	Clear_Remarks ();
	Clear_Orators ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	operator=
//
////////////////////////////////////////////////////////////////
const ConversationClass &
ConversationClass::operator= (const ConversationClass &src)
{
	//
	//	Start fresh
	//
	Clear_Remarks ();
	Clear_Orators ();

	//
	//	Copy the simple data
	//
	Name					= src.Name;
	ID						= src.ID;
	LookAtObjID			= src.LookAtObjID;
	Priority				= src.Priority;
	MaxDist				= src.MaxDist;
	IsInterruptable	= src.IsInterruptable;

	//
	//	Copy the remark list
	//
	int index;
	for (index = 0; index < src.RemarkList.Count (); index ++) {		
		const ConversationRemarkClass &remark = src.RemarkList[index];
		Add_Remark (remark);
	}

	//
	//	Copy the orator list
	//	
	for (index = 0; index < src.OratorList.Count (); index ++) {		
		const OratorClass &orator = src.OratorList[index];
		Add_Orator (orator);
	}

	return (*this);
}


////////////////////////////////////////////////////////////////
//
//	Clear_Orators
//
////////////////////////////////////////////////////////////////
void
ConversationClass::Clear_Orators (void)
{
	OratorList.Delete_All ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Clear_Remarks
//
////////////////////////////////////////////////////////////////
void
ConversationClass::Clear_Remarks (void)
{
	RemarkList.Delete_All ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Orator_Count
//
////////////////////////////////////////////////////////////////
int
ConversationClass::Get_Orator_Count (void) const
{
	return OratorList.Count ();
}


////////////////////////////////////////////////////////////////
//
//	Add_Orator
//
////////////////////////////////////////////////////////////////
void
ConversationClass::Add_Orator (const OratorClass &orator)
{
	OratorList.Add (orator);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Add_Remark
//
////////////////////////////////////////////////////////////////
void
ConversationClass::Add_Remark (const ConversationRemarkClass &remark)
{
	RemarkList.Add (remark);
	return ;
}


////////////////////////////////////////////////////////////////
//
//	Get_Remark_Info
//
////////////////////////////////////////////////////////////////
bool
ConversationClass::Get_Remark_Info (int index, ConversationRemarkClass &remark)
{
	bool retval = false;

	WWASSERT (index >= 0 && index < RemarkList.Count ());
	if (index >= 0 && index < RemarkList.Count ()) {

		//
		//	Return the remark information to the caller
		//
		remark = RemarkList[index];
		retval = true;
	}

	return retval;
}


////////////////////////////////////////////////////////////////
//
//	Save
//
////////////////////////////////////////////////////////////////
bool
ConversationClass::Save (ChunkSaveClass &csave)
{
	csave.Begin_Chunk (CHUNKID_VARIABLES);

		WRITE_MICRO_CHUNK_WWSTRING (csave, VARID_NAME,	Name);
		WRITE_MICRO_CHUNK (csave, VARID_ID,					ID);
		WRITE_MICRO_CHUNK (csave, VARID_ISINNATE,			IsInnate);
		WRITE_MICRO_CHUNK (csave, VARID_ISKEY,				IsKey);
		WRITE_MICRO_CHUNK (csave, VARID_PROBABILITY,		Probability);
		WRITE_MICRO_CHUNK (csave, VARID_AI_STATE,			AIState);
		WRITE_MICRO_CHUNK (csave, VARID_CATEGORY_ID,		CategoryID);
		WRITE_MICRO_CHUNK (csave, VARID_LOOKAT_OBJID,	LookAtObjID);
		WRITE_MICRO_CHUNK (csave, VARID_PRIORITY,			Priority);		
		WRITE_MICRO_CHUNK (csave, VARID_MAXDIST,			MaxDist);
		WRITE_MICRO_CHUNK (csave, VARID_IS_INTERRUPTABLE,	IsInterruptable);
						
		//
		//	Save our current pointer so we can remap it on load
		//
		ConversationClass *old_ptr = this;
		WRITE_MICRO_CHUNK (csave, VARID_OLD_PTR, old_ptr);

	csave.End_Chunk ();

	//
	//	Save the orator list
	//
	int index;
	for (index = 0; index < OratorList.Count (); index ++) {
		OratorClass &orator = OratorList[index];

		//
		//	Save this orator to its own chunk
		//
		csave.Begin_Chunk (CHUNKID_ORATOR);
			orator.Save (csave);
		csave.End_Chunk ();		
	}

	//
	//	Save the remark list
	//
	for (index = 0; index < RemarkList.Count (); index ++) {
		ConversationRemarkClass &remark = RemarkList[index];

		//
		//	Save this remark to its own chunk
		//
		csave.Begin_Chunk (CHUNKID_REMARK);
			remark.Save (csave);
		csave.End_Chunk ();		
	}

	return true;
}


////////////////////////////////////////////////////////////////
//
//	Load
//
////////////////////////////////////////////////////////////////
bool
ConversationClass::Load (ChunkLoadClass &cload)
{
	OratorList.Resize (10);

	while (cload.Open_Chunk ()) {		
		switch (cload.Cur_Chunk_ID ()) {

			case CHUNKID_VARIABLES:
				Load_Variables (cload);
				break;

			case CHUNKID_ORATOR:
			{
				OratorList.Add (OratorClass ());
				OratorList[OratorList.Count () - 1].Load (cload);				
			}
			break;

			case CHUNKID_REMARK:
			{
				ConversationRemarkClass remark;
				remark.Load (cload);
				RemarkList.Add (remark);				
			}
			break;
		}

		cload.Close_Chunk ();
	}

	return true;
}


///////////////////////////////////////////////////////////////////////
//
//	Load_Variables
//
///////////////////////////////////////////////////////////////////////
void
ConversationClass::Load_Variables (ChunkLoadClass &cload)
{
	ConversationClass *old_ptr = NULL;

	//
	//	Loop through all the microchunks that define the variables
	//
	while (cload.Open_Micro_Chunk ()) {
		switch (cload.Cur_Micro_Chunk_ID ()) {

			READ_MICRO_CHUNK_WWSTRING (cload, VARID_NAME,		Name);
			READ_MICRO_CHUNK (cload, VARID_ID,						ID);
			READ_MICRO_CHUNK (cload, VARID_OLD_PTR,				old_ptr);
			READ_MICRO_CHUNK (cload, VARID_ISINNATE,				IsInnate);		
			READ_MICRO_CHUNK (cload, VARID_ISKEY,					IsKey);
			READ_MICRO_CHUNK (cload, VARID_PROBABILITY,			Probability);
			READ_MICRO_CHUNK (cload, VARID_AI_STATE,				AIState);
			READ_MICRO_CHUNK (cload, VARID_CATEGORY_ID,			CategoryID);
			READ_MICRO_CHUNK (cload, VARID_LOOKAT_OBJID,			LookAtObjID);
			READ_MICRO_CHUNK (cload, VARID_PRIORITY,				Priority);
			READ_MICRO_CHUNK (cload, VARID_MAXDIST,				MaxDist);
			READ_MICRO_CHUNK (cload, VARID_IS_INTERRUPTABLE,	IsInterruptable);

			//
			//	Old-style remark (for backwards compatiblility)
			//
			case XXXX_VARID_REMARK:
			{
				struct
				{
					int	OratorID;
					int	TextID;
				} OldRemarkStruct;

				//
				//	Read the old data from the chunk
				//
				cload.Read (&OldRemarkStruct, sizeof (OldRemarkStruct));

				//
				//	Convert the data
				//
				ConversationRemarkClass remark;
				remark.Set_Orator_ID (OldRemarkStruct.OratorID);
				remark.Set_Text_ID (OldRemarkStruct.TextID);

				//
				//	Add this remark to our list
				//
				Add_Remark (remark);
			}
			break;
		}

		cload.Close_Micro_Chunk ();
	}

	//
	//	Register our old pointer so other objects can safely remap to it
	//
	WWASSERT (old_ptr != NULL);
	SaveLoadSystemClass::Register_Pointer (old_ptr, this);
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Get_Orator
//
///////////////////////////////////////////////////////////////////////
OratorClass *
ConversationClass::Get_Orator (int index)
{
	OratorClass *orator = NULL;

	if (index >= 0 && index < OratorList.Count ()) {
		orator = &OratorList[index];
	}

	return orator;
}


///////////////////////////////////////////////////////////////////////
//
//	Find_Orator
//
///////////////////////////////////////////////////////////////////////
OratorClass *
ConversationClass::Find_Orator (int orator_id)
{
	OratorClass *orator = NULL;

	for (int index = 0; index < OratorList.Count (); index ++) {
		OratorClass *curr_orator = &OratorList[index];
		
		//
		//	Is this the orator we are looking for?
		//
		if (curr_orator->Get_ID () == orator_id) {
			orator = curr_orator;
			break;
		}
	}

	return orator;
}

