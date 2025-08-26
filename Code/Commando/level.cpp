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
 *                     $Archive:: /Commando/Code/commando/level.cpp                           $* 
 *                                                                                             * 
 *                      $Author:: Patrick                                                     $* 
 *                                                                                             * 
 *                     $Modtime:: 1/16/02 8:56a                                               $* 
 *                                                                                             * 
 *                    $Revision:: 81                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "level.h"
#include "combat.h"
#include "pscene.h"
#include "savegame.h"
#include "WWAudio.h"
#include "SoundScene.h"
#include "gameobjmanager.h"
#include "assets.h"
#include "transition.h"
#include "conversationmgr.h"
#include "wwprofile.h"
#include "phys.h"
#include "rendobj.h"

/*
** Release all objects and resources loaded for this level
*/
void		LevelManager::Release_Level( void )
{
WWPROFILE( "Release Level" );
	SaveGameManager::Set_Map_Filename( NULL );
	ConversationMgrClass::Reset_Active_Conversations ();

	//
	// Stop (and free) any currently playing sounds
	//
   WWASSERT(WWAudioClass::Get_Instance() != NULL);
   SoundSceneClass *sound_scene = WWAudioClass::Get_Instance ()->Get_Sound_Scene ();
	if( sound_scene != NULL ) {
		sound_scene->Flush_Scene ();
	}	

	WWAudioClass::Get_Instance ()->Flush_Playlist ();

	GameObjManager::Destroy_All();		// Kill All Objects (including the Camera)

	TransitionManager::Reset();

{WWPROFILE( "Scene remove" );
	COMBAT_SCENE->Remove_All();
}

{WWPROFILE( "Free assets" );
	WW3DAssetManager::Get_Instance()->Free_Assets();	
}
}


