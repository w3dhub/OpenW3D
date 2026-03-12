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
 *                     $Archive:: /Commando/Code/Tests/PhysTest/PhysTestSaveSystem.cpp        $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 11/02/00 6:20p                                              $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "StdAfx.h"
#include "PhysTestSaveSystem.h"
#include "MainFrm.h"
#include "chunkio.h"

/*
** Instantiate the Save-System
*/
PhysTestSaveSystemClass _PhysTestSaveSystem;


uint32 PhysTestSaveSystemClass::Chunk_ID(void) const
{
	return PHYSTEST_CHUNKID_SUBSYSTEM;
}

bool PhysTestSaveSystemClass::Save(ChunkSaveClass &csave)
{
	csave.Begin_Chunk(CHUNKID_MAINFRAME);
	((CMainFrame *)(::AfxGetMainWnd()))->Save(csave);
	csave.End_Chunk();

	return true;
}

bool PhysTestSaveSystemClass::Load(ChunkLoadClass &cload)
{
	while (cload.Open_Chunk()) {
		switch (cload.Cur_Chunk_ID()) {
		case CHUNKID_MAINFRAME:
			((CMainFrame *)(::AfxGetMainWnd()))->Load(cload);
			break;
		}
		cload.Close_Chunk();
	}
	return true;
}


