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
 *                     $Archive:: /Commando/Code/commando/gamemenu.h                           $* 
 *                                                                                             * 
 *                      $Author:: Patrick                                                     $* 
 *                                                                                             * 
 *                     $Modtime:: 1/02/02 3:57p                                               $* 
 *                                                                                             * 
 *                    $Revision:: 13                                                         $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef	GAMEMENU_H
#define	GAMEMENU_H

#ifndef	ALWAYS_H
	#include "always.h"
#endif

#ifndef	GAMEMODE_H
	#include "gamemode.h"
#endif

/*
** Game Mode to do menus
*/
class	MenuGameModeClass2 : public GameModeClass {

public:
	MenuGameModeClass2 (void);

	virtual	const char *Name() override	{ return "Menu"; }	// the name of this mode
	virtual	void	Init() override;		 	// called when the mode is activated
	virtual	void 	Shutdown() override; 	// called when the mode is deactivated
	virtual	void 	Think() override;			// called each time through the main loop
	virtual	void 	Render() override;		// called each time through the main loop

	virtual	void Activate() override;		// activates the mode
	virtual	void Deactivate() override;	// deactivates the mode (don't shutdown until safe)

private:
	class AudibleSoundClass *	MenuMusic;
};


#endif


