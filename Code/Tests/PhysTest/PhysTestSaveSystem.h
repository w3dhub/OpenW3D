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
 *                 Project Name : PhysTest                                                     *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tests/PhysTest/PhysTestSaveSystem.h          $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 11/02/00 6:14p                                              $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef PHYSTESTSAVESYSTEM_H
#define PHYSTESTSAVESYSTEM_H

#include "always.h"
#include "saveloadsubsystem.h"
#include "saveloadids.h"

/******************************************************************************************
**
** PhysTestSaveSystem
** Saves all data for the PhysTest app
**
******************************************************************************************/
class PhysTestSaveSystemClass : public SaveLoadSubSystemClass
{
public:
	
	virtual uint32				Chunk_ID (void) const override;

protected:

	virtual bool				Save (ChunkSaveClass &csave);
	virtual bool				Load (ChunkLoadClass &cload);

	/*
	** internal chunk id's
	*/
	enum 
	{
		CHUNKID_MAINFRAME			= 0x00667001,
	};
};

/*
** _PhysTestSaveSystem - global instance of the sub-system for PhysTest
*/
extern PhysTestSaveSystemClass _PhysTestSaveSystem;


/******************************************************************************************
**
** Chunk ID's used by PhysTest objects 
**
******************************************************************************************/
enum
{
	PHYSTEST_CHUNKID_SUBSYSTEM			= CHUNKID_PHYSTEST_BEGIN,
};

#endif
