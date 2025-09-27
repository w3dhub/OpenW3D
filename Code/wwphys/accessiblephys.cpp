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
 *                 Project Name : wwphys																		  *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwphys/accessiblephys.cpp        $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 8/17/01 8:42p                                               $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "accessiblephys.h"
#include "wwphysids.h"
#include "persistfactory.h"
#include "simpledefinitionfactory.h"

/////////////////////////////////////////////////////////////////////////
//	Persist and definition factories
/////////////////////////////////////////////////////////////////////////
SimplePersistFactoryClass	<AccessiblePhysClass,		PHYSICS_CHUNKID_ACCESSIBLEPHYS>					_AccessiblePhysClassFactory;
SimplePersistFactoryClass	<AccessiblePhysDefClass,	PHYSICS_CHUNKID_ACCESSIBLEPHYSDEF>				_AccessiblePhysDefFactory;
DECLARE_DEFINITION_FACTORY	(AccessiblePhysDefClass,	CLASSID_ACCESSIBLEPHYSDEF, "AccessiblePhys")	_AccessiblePhysDefDefFactory;


/////////////////////////////////////////////////////////////////////////
//	Save/load constants
/////////////////////////////////////////////////////////////////////////
enum
{
	CHUNKID_PARENT				= 0x10311235,
	CHUNKID_VARIABLES,
	
	VARID_LOCKCODE				= 1,
};										


enum
{
	CHUNKID_DEF_PARENT		= 0x10311249,
	CHUNKID_DEF_VARIABLES,
	
	VARID_DEF_LOCKCODE		= 1,
};										


/////////////////////////////////////////////////////////////////////////
//
//	AccessiblePhysClass
//
/////////////////////////////////////////////////////////////////////////
AccessiblePhysClass::AccessiblePhysClass (void) :
	LockCode (0)
{
	return ;
}


/////////////////////////////////////////////////////////////////////////
//
//	~AccessiblePhysClass
//
/////////////////////////////////////////////////////////////////////////
AccessiblePhysClass::~AccessiblePhysClass (void)
{
	return ;
}


/////////////////////////////////////////////////////////////////////////
//
//	Init
//
/////////////////////////////////////////////////////////////////////////
void
AccessiblePhysClass::Init (const AccessiblePhysDefClass &definition)
{
	LockCode = definition.LockCode;
	StaticAnimPhysClass::Init ((const StaticAnimPhysDefClass &)definition);
	return ;
}


/////////////////////////////////////////////////////////////////////////
//
//	Get_Factory
//
/////////////////////////////////////////////////////////////////////////
const PersistFactoryClass &
AccessiblePhysClass::Get_Factory (void) const
{
	return _AccessiblePhysClassFactory;
}


/////////////////////////////////////////////////////////////////////////
//
//	Save
//
/////////////////////////////////////////////////////////////////////////
bool
AccessiblePhysClass::Save (ChunkSaveClass &csave)
{
	csave.Begin_Chunk (CHUNKID_PARENT);
		StaticAnimPhysClass::Save (csave);
	csave.End_Chunk ();

	csave.Begin_Chunk (CHUNKID_VARIABLES);
		Save_Variables (csave);
	csave.End_Chunk ();	
	return true;
}


/////////////////////////////////////////////////////////////////////////
//
//	Save_Variables
//
/////////////////////////////////////////////////////////////////////////
bool
AccessiblePhysClass::Save_Variables (ChunkSaveClass &csave)
{
	WRITE_MICRO_CHUNK (csave, VARID_LOCKCODE, LockCode);
	return true;
}


/////////////////////////////////////////////////////////////////////////
//
//	Load
//
/////////////////////////////////////////////////////////////////////////
bool
AccessiblePhysClass::Load (ChunkLoadClass &cload)
{
	while (cload.Open_Chunk ()) {
		
		switch(cload.Cur_Chunk_ID ()) 
		{
			case CHUNKID_PARENT:
				StaticAnimPhysClass::Load (cload);
				break;

			case CHUNKID_VARIABLES:
				Load_Variables (cload);
				break;
				
			default:
				WWDEBUG_SAY(("Unhandled Chunk: 0x%X File: %s Line: %d\r\n",cload.Cur_Chunk_ID(),__FILE__,__LINE__));
				break;
		}
		
		cload.Close_Chunk();
	}

	return true;
}


///////////////////////////////////////////////////////////////////////
//
//	Load_Variables
//
///////////////////////////////////////////////////////////////////////
bool
AccessiblePhysClass::Load_Variables (ChunkLoadClass &cload)
{
	//
	//	Loop through all the microchunks that define the variables
	//
	while (cload.Open_Micro_Chunk ()) {
		switch (cload.Cur_Micro_Chunk_ID ()) {
			
			READ_MICRO_CHUNK (cload, VARID_LOCKCODE, LockCode);
		}

		cload.Close_Micro_Chunk ();
	}

	return true;
}


///////////////////////////////////////////////////////////////////////
//
//	AccessiblePhysDefClass
//
///////////////////////////////////////////////////////////////////////
AccessiblePhysDefClass::AccessiblePhysDefClass (void) :
	LockCode (0)
{
	EDITABLE_PARAM (AccessiblePhysDefClass, ParameterClass::TYPE_INT, LockCode);
	return ;
}


///////////////////////////////////////////////////////////////////////
//
//	Get_Class_ID
//
///////////////////////////////////////////////////////////////////////
uint32
AccessiblePhysDefClass::Get_Class_ID (void) const	
{ 
	return CLASSID_ACCESSIBLEPHYSDEF; 
}


///////////////////////////////////////////////////////////////////////
//
//	Create
//
///////////////////////////////////////////////////////////////////////
PersistClass *
AccessiblePhysDefClass::Create (void) const
{
	AccessiblePhysClass * obj = NEW_REF(AccessiblePhysClass,());
	obj->Init(*this);
	return obj;
}


///////////////////////////////////////////////////////////////////////
//
//	Get_Factory
//
///////////////////////////////////////////////////////////////////////
const
PersistFactoryClass &
AccessiblePhysDefClass::Get_Factory (void) const
{
	return _AccessiblePhysDefFactory;
}


///////////////////////////////////////////////////////////////////////
//
//	Is_Type
//
///////////////////////////////////////////////////////////////////////
bool
AccessiblePhysDefClass::Is_Type (const char *type_name)
{
	bool retval = false;

	if (::stricmp (type_name, AccessiblePhysDefClass::Get_Type_Name ()) == 0) {
		retval = true;
	} else {
		retval = StaticAnimPhysDefClass::Is_Type (type_name);
	}

	return retval;
}


/////////////////////////////////////////////////////////////////////////
//
//	Save
//
/////////////////////////////////////////////////////////////////////////
bool
AccessiblePhysDefClass::Save (ChunkSaveClass &csave)
{
	csave.Begin_Chunk (CHUNKID_DEF_PARENT);
		StaticAnimPhysDefClass::Save (csave);
	csave.End_Chunk ();

	csave.Begin_Chunk (CHUNKID_DEF_VARIABLES);
		Save_Variables (csave);
	csave.End_Chunk ();
	return true;
}


/////////////////////////////////////////////////////////////////////////
//
//	Save_Variables
//
/////////////////////////////////////////////////////////////////////////
bool
AccessiblePhysDefClass::Save_Variables (ChunkSaveClass &csave)
{
	WRITE_MICRO_CHUNK (csave, VARID_DEF_LOCKCODE, LockCode);
	return true;
}


/////////////////////////////////////////////////////////////////////////
//
//	Load
//
/////////////////////////////////////////////////////////////////////////
bool
AccessiblePhysDefClass::Load (ChunkLoadClass &cload)
{
	while (cload.Open_Chunk ()) {
		
		switch(cload.Cur_Chunk_ID ()) 
		{
			case CHUNKID_DEF_PARENT:
				StaticAnimPhysDefClass::Load (cload);
				break;

			case CHUNKID_DEF_VARIABLES:
				Load_Variables (cload);
				break;
				
			default:
				WWDEBUG_SAY (("Unhandled Chunk: 0x%X File: %s Line: %d\r\n", cload.Cur_Chunk_ID (), __FILE__, __LINE__));
				break;
		}
		
		cload.Close_Chunk();
	}

	return true;
}


///////////////////////////////////////////////////////////////////////
//
//	Load_Variables
//
///////////////////////////////////////////////////////////////////////
bool
AccessiblePhysDefClass::Load_Variables (ChunkLoadClass &cload)
{
	//
	//	Loop through all the microchunks that define the variables
	//
	while (cload.Open_Micro_Chunk ()) {
		switch (cload.Cur_Micro_Chunk_ID ()) {
			
			READ_MICRO_CHUNK (cload, VARID_DEF_LOCKCODE, LockCode);
		}

		cload.Close_Micro_Chunk ();
	}

	return true;
}

