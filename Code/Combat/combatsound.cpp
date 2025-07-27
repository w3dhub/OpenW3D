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
 *                     $Archive:: /Commando/Code/Combat/combatsound.cpp                       $* 
 *                                                                                             * 
 *                      $Author:: Patrick                                                     $* 
 *                                                                                             * 
 *                     $Modtime:: 11/29/00 2:16p                                              $* 
 *                                                                                             * 
 *                    $Revision:: 13                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "combatsound.h"
#include "wwaudio.h"

/*
**
*/
const char * CombatSoundTypeNames[NUM_DEFINED_SOUND_TYPES] = 
{
	"None",
	"Old Weapon",
	"Footsteps",
	"Vehicle",
	"Gunshot",
	"Bullet Hit"
};

/*
**
*/
void	CombatSoundManager::Init( void )
{
	int i;
	for ( i = 0; i < NUM_DEFINED_SOUND_TYPES; i++ ) {
		WWAudioClass::Get_Instance()->Add_Logical_Type( i, CombatSoundTypeNames[i] );
	}

	for ( i = SOUND_TYPE_DESIGNER01; i <= SOUND_TYPE_DESIGNER09; i++ ) {
		
		StringClass type_name;
		type_name.Format ("Designer%02d", (i - SOUND_TYPE_DESIGNER01) + 1);
		WWAudioClass::Get_Instance()->Add_Logical_Type( i, type_name );
	}
}

void	CombatSoundManager::Shutdown( void )
{
	WWAudioClass::Get_Instance()->Reset_Logical_Types();
}


