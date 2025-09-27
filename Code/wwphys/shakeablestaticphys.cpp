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
 *                 Project Name : WWPhys                                                       *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwphys/shakeablestaticphys.cpp               $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 8/17/01 7:02p                                               $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "shakeablestaticphys.h"
#include "simpledefinitionfactory.h"
#include "persistfactory.h"
#include "wwphysids.h"
#include "hanim.h"
#include "wwdebug.h"
#include "wwhack.h"


DECLARE_FORCE_LINK( shakeablestaticphys );

/*
** Chunk Id's used by ShakeableStaticPhysClass
*/
enum 
{
	SHAKEABLESTATICPHYS_CHUNK_STATICANIMPHYS				= 7311740,
};

/*
** Chunk Id's used by ShakeableStaticPhysDefClass
*/
enum	
{
	SHAKEABLESTATICPHYSDEF_CHUNK_STATICANIMPHYSDEF		= 7311734,
};


/************************************************************************************************
**
** ShakeableStaticPhysDefClass Implementation
**
************************************************************************************************/
SimplePersistFactoryClass<ShakeableStaticPhysDefClass, PHYSICS_CHUNKID_SHAKEABLESTATICPHYSDEF>	_ShakeableStaticPhysDefPersistFactory;

//(gth) for now, not publishing this class to the editor.  I need to find a solution for
//how to detect when a vehicle collides with this object (vehicles don't actually find out
//who they collide with!)  Also, maybe the feature provided by this class could be rolled into
//StaticAnim instead of requiring a whole new class...

#if 0
DECLARE_DEFINITION_FACTORY(ShakeableStaticPhysDefClass, CLASSID_SHAKEABLESTATICPHYSDEF, "ShakeableStaticPhys") _ShakeableStaticPhysDefDefFactory;
#endif


ShakeableStaticPhysDefClass::ShakeableStaticPhysDefClass(void)
{
}

uint32 ShakeableStaticPhysDefClass::Get_Class_ID(void) const	
{ 
	return CLASSID_SHAKEABLESTATICPHYSDEF; 
}

bool ShakeableStaticPhysDefClass::Is_Type(const char * type_name)
{
	if (stricmp(type_name,ShakeableStaticPhysDefClass::Get_Type_Name()) == 0) {
		return true;
	} else {
		return StaticAnimPhysDefClass::Is_Type(type_name);
	}
}

PersistClass *	ShakeableStaticPhysDefClass::Create(void) const 
{
	ShakeableStaticPhysClass * obj = new ShakeableStaticPhysClass;
	obj->Init(*this);
	return obj;
}


bool	ShakeableStaticPhysDefClass::Save(ChunkSaveClass & csave)
{
	csave.Begin_Chunk(SHAKEABLESTATICPHYSDEF_CHUNK_STATICANIMPHYSDEF);
		StaticAnimPhysDefClass::Save(csave);
	csave.End_Chunk();

	return true;
}

bool	ShakeableStaticPhysDefClass::Load( ChunkLoadClass &cload )
{
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case SHAKEABLESTATICPHYSDEF_CHUNK_STATICANIMPHYSDEF:
				StaticAnimPhysDefClass::Load( cload );
				break;
  
			default:
				WWDEBUG_SAY(( "Unrecognized ShakeableStaticPhysDef chunkID\n" ));
				break;

		}
		cload.Close_Chunk();
	}

	return true;
}

const PersistFactoryClass & ShakeableStaticPhysDefClass::Get_Factory (void) const 
{ 
	return _ShakeableStaticPhysDefPersistFactory; 
}



/************************************************************************************************
**
** ShakeableStaticPhysClass Implementation
**
************************************************************************************************/
SimplePersistFactoryClass<ShakeableStaticPhysClass, PHYSICS_CHUNKID_SHAKEABLESTATICPHYS>	_ShakeableStaticPhysPersistFactory;

const PersistFactoryClass & ShakeableStaticPhysClass::Get_Factory (void) const 
{
	return _ShakeableStaticPhysPersistFactory;
}

ShakeableStaticPhysClass::ShakeableStaticPhysClass(void)
{
}

ShakeableStaticPhysClass::~ShakeableStaticPhysClass( void )
{
}

void ShakeableStaticPhysClass::Init( const ShakeableStaticPhysDefClass & def )
{
	StaticAnimPhysClass::Init(def);
}

void ShakeableStaticPhysClass::Play_Animation(void)
{
	/*
	** When a vehicle collides with this object, it will call this function
	** to cause the shake animation to play.  We simply set the current frame
	** number to the start of the animation and set the target frame number
	** to the end of the animation...
	*/
	AnimCollisionManagerClass & anim_mgr = Get_Animation_Manager();
	
	HAnimClass * anim = anim_mgr.Peek_Animation();
	if (anim != NULL) {
		anim_mgr.Set_Current_Frame(0);
		anim_mgr.Set_Animation_Mode(AnimCollisionManagerClass::ANIMATE_TARGET);
		anim_mgr.Set_Target_Frame(anim->Get_Num_Frames()-1);
	}
}

bool ShakeableStaticPhysClass::Save( ChunkSaveClass & csave )
{
	/*
	** Save the parent class data
	*/
	csave.Begin_Chunk(SHAKEABLESTATICPHYS_CHUNK_STATICANIMPHYS);
		StaticAnimPhysClass::Save( csave );
	csave.End_Chunk();

	return true;
}

bool ShakeableStaticPhysClass::Load(ChunkLoadClass & cload)
{
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case SHAKEABLESTATICPHYS_CHUNK_STATICANIMPHYS:
				StaticAnimPhysClass::Load( cload );
				break;
		
			default:
				WWDEBUG_SAY(( "Unrecognized ShakeableStaticPhys chunkID\n" ));
				break;

		}
		cload.Close_Chunk();
	}

	return true;
}





