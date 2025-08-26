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
 *                     $Archive:: /Commando/Code/Commando/langmode.h                          $* 
 *                                                                                             * 
 *                      $Author:: Denzil_l                                                    $* 
 *                                                                                             * 
 *                     $Modtime:: 11/16/01 1:30p                                              $* 
 *                                                                                             * 
 *                    $Revision:: 11                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSV_VER)
#pragma once
#endif

#ifndef LANGMODE_H
#define LANGMODE_H

#include "gamemode.h"
#include "lanchat.h"

#define PLC LanGameModeClass::Get_Lan_Interface()

// 
// Game Mode to do LAN interface
//
class	LanGameModeClass :
		public GameModeClass
{
	public:
		static cLanChat * Get_Lan_Interface(void);

		virtual	const char *Name(void)	{ return "LAN"; }	// the name of this mode
		virtual	void	Init(void);		 	// called when the mode is activated
		virtual	void 	Shutdown(void); 	// called when the mode is deactivated
		virtual	void 	Think(void);		// called each time through the main loop
		virtual	void 	Render(void) {};		// called each time through the main loop

	private:
		static cLanChat * PLanChat;
};

#endif	//	LANGMODE_H
