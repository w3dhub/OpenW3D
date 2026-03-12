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

/* $Header: /Commando/Code/ww3d2/hanimmgr.cpp 3     1/16/02 9:51a Jani_p $ */
/***********************************************************************************************
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando / G 3D Library                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/hanimmgr.cpp                           $*
 *                                                                                             *
 *                       Author:: Greg_h                                                       *
 *                                                                                             *
 *                     $Modtime:: 1/16/02 9:49a                                               $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   HAnimManagerClass::HAnimManagerClass -- constructor                                       *
 *   HAnimManagerClass::~HAnimManagerClass -- destructor                                       *
 *   HAnimManagerClass::Load_Anim -- loads a set of motion data from a file                    *
 *   HAnimManagerClass::Get_Anim_ID -- looks up the ID of a named Hierarchy Animation          *
 *   HAnimManagerClass::Get_Anim -- returns a pointer to the specified animation data          *
 *   HAnimManagerClass::Get_Anim -- returns a pointer to the specified Hierarchy Animation     *
 *   HAnimManagerClass::Free -- de-allocate all memory in use                                  *
 *   HAnimManagerClass::Free_All_Anims -- de-allocate all currently loaded animations          *
 *   HAnimManagerClass::Load_Raw_Anim -- Load a raw anim                                       *
 *   HAnimManagerClass::Load_Compressed_Anim -- load a compressed animation                    *
 *	  HAnimManagerClass::Add_Anim -- Adds an externally created animation to the manager		  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "hanimmgr.h"
#include <string.h>
#include "hanim.h"
#include "hrawanim.h"
#include "hcanim.h"
#include "hmorphanim.h"
#include "chunkio.h"
#include "wwmemlog.h"
#include "animatedsoundmgr.h"


/***********************************************************************************************
 * HAnimManagerClass::HAnimManagerClass -- constructor                                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/11/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
HAnimManagerClass::HAnimManagerClass(void)
{
	// Create the hash tables
	AnimPtrTable = new HashTableClass( 2048 );
	MissingAnimTable = new HashTableClass( 2048 );
}


/***********************************************************************************************
 * HAnimManagerClass::~HAnimManagerClass -- destructor                                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/11/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
HAnimManagerClass::~HAnimManagerClass(void)
{
	Free_All_Anims();
	Reset_Missing();	// Jani: Deleting missing animations as well

	delete AnimPtrTable;
	AnimPtrTable = NULL;

	delete MissingAnimTable;
	MissingAnimTable = NULL;
}


/***********************************************************************************************
 * HAnimManagerClass::Load_Anim -- loads a set of motion data from a file                      *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/11/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
int HAnimManagerClass::Load_Anim(ChunkLoadClass & cload)
{
	WWMEMLOG(MEM_ANIMATION);

	switch (cload.Cur_Chunk_ID())
	{
	case W3D_CHUNK_ANIMATION:
		return Load_Raw_Anim(cload);
		break;

	case W3D_CHUNK_COMPRESSED_ANIMATION:
		return Load_Compressed_Anim(cload);
		break;

	case W3D_CHUNK_MORPH_ANIMATION:
		return Load_Morph_Anim(cload);
		break;
	}

	return 0;
}


/***********************************************************************************************
 * HAnimManagerClass::Load_Morph_Anim -- Load a HMorphAnimClass										  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/23/2000  pds : Created.                                                                 *
 *=============================================================================================*/
int HAnimManagerClass::Load_Morph_Anim(ChunkLoadClass & cload)
{
	HMorphAnimClass * newanim = new HMorphAnimClass;

	if (newanim == NULL) {
		goto Error;
	}

	SET_REF_OWNER( newanim );

	if (newanim->Load_W3D(cload) != HMorphAnimClass::OK) {
		// load failed!
		newanim->Release_Ref();
		goto Error;
	} else if (Peek_Anim(newanim->Get_Name()) != NULL) {
		// duplicate exists!
		newanim->Release_Ref();	// Release the one we just loaded
		goto Error;
	} else {
		Add_Anim( newanim );
		newanim->Release_Ref();
	}

	return 0;

Error:

	return 1;
}


/***********************************************************************************************
 * HAnimManagerClass::Load_Raw_Anim -- Load a raw anim                                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/23/2000  gth : Created.                                                                 *
 *=============================================================================================*/
int HAnimManagerClass::Load_Raw_Anim(ChunkLoadClass & cload)
{
	HRawAnimClass * newanim = new HRawAnimClass;

	if (newanim == NULL) {
		goto Error;
	}

	SET_REF_OWNER( newanim );

	if (newanim->Load_W3D(cload) != HRawAnimClass::OK) {
		// load failed!
		newanim->Release_Ref();
		goto Error;
	} else if (Peek_Anim(newanim->Get_Name()) != NULL) {
		// duplicate exists!
		newanim->Release_Ref();	// Release the one we just loaded
		goto Error;
	} else {
		Add_Anim( newanim );
		newanim->Release_Ref();
	}

	return 0;

Error:

	return 1;
}


/***********************************************************************************************
 * HAnimManagerClass::Load_Compressed_Anim -- load a compressed animation                      *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/23/2000  gth : Created.                                                                 *
 *=============================================================================================*/
int HAnimManagerClass::Load_Compressed_Anim(ChunkLoadClass & cload)
{
	HCompressedAnimClass * newanim = new HCompressedAnimClass;

	if (newanim == NULL) {
		goto Error;
	}

	SET_REF_OWNER( newanim );

	if (newanim->Load_W3D(cload) != HCompressedAnimClass::OK) {
		// load failed!
		newanim->Release_Ref();
		goto Error;
	} else if (Peek_Anim(newanim->Get_Name()) != NULL) {
		// duplicate exists!
		newanim->Release_Ref();	// Release the one we just loaded
		goto Error;
	} else {
		Add_Anim( newanim );
		newanim->Release_Ref();
	}

	return 0;

Error:

	return 1;
}

/***********************************************************************************************
 * HAnimManagerClass::Peek_Anim -- returns a pointer to the specified animation data            *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/11/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
HAnimClass * HAnimManagerClass::Peek_Anim(const char * name)
{
	return (HAnimClass*)AnimPtrTable->Find( name );
}


/***********************************************************************************************
 * HAnimManagerClass::Get_Anim -- returns a pointer to the specified animation data            *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/11/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
HAnimClass * HAnimManagerClass::Get_Anim(const char * name)
{
	HAnimClass * anim = Peek_Anim( name );
	if ( anim != NULL ) {
		anim->Add_Ref();
	}
	return anim;
}


/***********************************************************************************************
 * HAnimManagerClass::Free_All_Anims -- de-allocate all currently loaded animations            *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/11/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
void HAnimManagerClass::Free_All_Anims(void)
{
	// Make an iterator, and release all ptrs
	HAnimManagerIterator it( *this );
	for( it.First(); !it.Is_Done(); it.Next() ) {
		HAnimClass *anim = it.Get_Current_Anim();
		anim->Release_Ref();
	}

	// Then clear the table
	AnimPtrTable->Reset();
}


/***********************************************************************************************
 * HAnimManagerClass::Add_Anim -- Adds an externally created animation to the manager			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/31/2000 PDS  : Created.                                                                *
 *=============================================================================================*/
bool HAnimManagerClass::Add_Anim(HAnimClass *new_anim)
{
	WWASSERT (new_anim != NULL);

	// Increment the refcount on the new animation and add it to our table.
	new_anim->Add_Ref ();
	AnimPtrTable->Add( new_anim );

	//
	//	Check to see if this animation has any embedded sounds that may
	// need to play while its animating.
	//
	bool has_sound_trigger = AnimatedSoundMgrClass::Does_Animation_Have_Embedded_Sounds( new_anim );
	new_anim->Set_Has_Embedded_Sounds( has_sound_trigger );

	return true;
}


/*
** Missing Anims
**
** The idea here, allow the system to register which anims are determined to be missing
** so that if they are asked for again, we can quickly return NULL, without searching the
** disk again.
*/
void	HAnimManagerClass::Register_Missing( const char * name )
{
	MissingAnimTable->Add( new MissingAnimClass( name ) );
}

bool	HAnimManagerClass::Is_Missing( const char * name )
{
	return ( MissingAnimTable->Find( name ) != NULL );
}

void	HAnimManagerClass::Reset_Missing( void )
{
	// Make an iterator, and release all ptrs
	HashTableIteratorClass it( *MissingAnimTable );
	for( it.First(); !it.Is_Done(); it.Next() ) {
		MissingAnimClass *missing = (MissingAnimClass *)it.Get_Current();
		delete missing;
	}

	// Then clear the table
	MissingAnimTable->Reset();
}


/*
** Iterator converter from HashableClass to HAnimClass
*/
HAnimClass * HAnimManagerIterator::Get_Current_Anim( void )
{
	return (HAnimClass *)Get_Current();
}

