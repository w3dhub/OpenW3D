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
 *                     $Archive:: /Commando/Code/Commando/langmode.cpp                        $*
 *                                                                                             *
 *                      $Author:: Denzil_l                                                    $*
 *                                                                                             *
 *                     $Modtime:: 11/16/01 1:29p                                              $*
 *                                                                                             *
 *                    $Revision:: 21                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "langmode.h" // I WANNA BE FIRST!
#include "miscutil.h"
#include "wwdebug.h"
#include "wwprofile.h"
//
// class statics
//
cLanChat * LanGameModeClass::PLanChat = nullptr;

//-----------------------------------------------------------------------------
//
// called when the mode is activated
//
void LanGameModeClass::Init(void)
{
	WWDEBUG_SAY(("LanGameModeClass::Init\n"));

	WWASSERT(PLanChat == nullptr);
	PLanChat = new cLanChat();
	WWASSERT(PLanChat != nullptr);
}

//-----------------------------------------------------------------------------
//
// called when the mode is deactivated
//
void LanGameModeClass::Shutdown(void)
{
	WWDEBUG_SAY(("LanGameModeClass::Shutdown"));

	WWASSERT(PLanChat != nullptr);
	delete PLanChat;
	PLanChat = nullptr;
}

//-----------------------------------------------------------------------------
//
// called each time through the main loop
//
void LanGameModeClass::Think(void)
{
	WWPROFILE( "Lan Think" );

	WWASSERT(PLanChat != nullptr);
	PLanChat->Think();
}

//-----------------------------------------------------------------------------
cLanChat * LanGameModeClass::Get_Lan_Interface(void)
{
	//WWASSERT(PLanChat != nullptr);

	return PLanChat;
}
