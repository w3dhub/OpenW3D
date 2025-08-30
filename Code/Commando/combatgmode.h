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
 *                     $Archive:: /Commando/Code/Commando/combatgmode.h                       $* 
 *                                                                                             * 
 *                      $Author:: Tom_s                                                       $* 
 *                                                                                             * 
 *                     $Modtime:: 11/10/01 12:34p                                             $* 
 *                                                                                             * 
 *                    $Revision:: 23                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef	COMBATGMODE_H
#define	COMBATGMODE_H

#ifndef	ALWAYS_H
	#include "always.h"
#endif

#ifndef	GAMEMODE_H
	#include "gamemode.h"
#endif

#ifndef	COMBAT_H
	#include "combat.h"
#endif

/*
**
*/
class AudibleSoundClass;
class PhysicsSceneClass;

#define	COMBAT_BACKGROUND_MUSIC		CombatGameModeClass::BackgroundMusic

/*
** Game Mode to do the main combat mode
*/
class	CombatGameModeClass : public GameMajorModeClass {
public:
	virtual	const char *Name() override	{ return "Combat"; }	// the name of this mode
	virtual	void	Init() override;		 	// called when the mode is activated
	virtual	void 	Shutdown() override; 	// called when the mode is deactivated
	virtual	void 	Think() override;			// called each time through the main loop to think when non-inactive
	virtual	void 	Render() override;		// called each time through the main loop to draw when non-inactive
	virtual	void	Resume() override;		// activates the mode
	virtual	void	Suspend() override;		// deactivates the mode (don't shutdown until safe)

				void	Load_Level();
				void 	Core_Shutdown();
				void 	Core_Restart();

	static   void  Toggle_Multi_Hud( void );

	static	AudibleSoundClass	* BackgroundMusic;

	static	void	Quick_Save( void );

private:

	void				Combat_Keyboard( void );

	static	void	Load_Registry_Keys(void);
	static	void	Save_Registry_Keys(void);

	static	int	IsHudShown;

	static	void	Post_Load_Id_Uniqueness_Check(void);
	static	void	Post_Load_Dynamic_Object_Filtering(void);
	static	void	Compute_World_Size(void);
	static	void	Spawn_Point_Validation(void);

};


#endif



