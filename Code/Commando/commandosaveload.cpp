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
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando                                                     *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Commando/commandosaveload.cpp                $*
 *                                                                                             *
 *                      $Author:: Byon_g                                                      $*
 *                                                                                             *
 *                     $Modtime:: 9/06/01 1:57p                                               $*
 *                                                                                             *
 *                    $Revision:: 7                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "commandosaveload.h"
#include "chunkio.h"
#include "cnetwork.h"
#include "debug.h"
#include "wwmemlog.h"
#include "god.h"
#include "campaign.h"

/*
**
*/
CommandoSaveLoadClass	_CommandoSaveLoad;

enum	{
	CHUNKID_NETWORK								= 1011991043,
	CHUNKID_GOD,
	CHUNKID_CAMPAIGN,
};

/*
**
*/
bool	CommandoSaveLoadClass::Save( ChunkSaveClass &csave )
{
	WWMEMLOG(MEM_GAMEDATA);

	csave.Begin_Chunk( CHUNKID_NETWORK );
	cNetwork::Save( csave );
	csave.End_Chunk();

	csave.Begin_Chunk( CHUNKID_GOD );
	cGod::Save( csave );
	csave.End_Chunk();

	csave.Begin_Chunk( CHUNKID_CAMPAIGN );
	CampaignManager::Save( csave );
	csave.End_Chunk();

	return true;
}

bool	CommandoSaveLoadClass::Load( ChunkLoadClass &cload )
{
	WWMEMLOG(MEM_GAMEDATA);

	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_NETWORK:
				cNetwork::Load( cload );
				break;

			case CHUNKID_GOD:
				cGod::Load( cload );
				break;

			case CHUNKID_CAMPAIGN:
				CampaignManager::Load( cload );
				break;

			default:
				Debug_Say(( "Unrecognized Commando chunkID\n" ));
				break;

		}
		cload.Close_Chunk();
	}
	return true;
}


