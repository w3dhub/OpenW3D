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
 *                     $Archive:: /Commando/Code/Combat/hud.h                                 $*
 *                                                                                             *
 *                      $Author:: Patrick                                                     $*
 *                                                                                             *
 *                     $Modtime:: 1/03/02 11:42a                                              $*
 *                                                                                             *
 *                    $Revision:: 65                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef	HUD_H
#define	HUD_H

#ifndef	ALWAYS_H
	#include "always.h"
#endif

/*
**
*/
class	ChunkSaveClass;
class	ChunkLoadClass;

/*
**
*/
class	HUDClass {

public:
	static	void	Init(bool render_available = true);
	static	void 	Shutdown();
	static	void 	Think();
	static	void 	Render();
	static	void 	Reset();

	static	bool	Save( ChunkSaveClass &csave );
	static	bool	Load( ChunkLoadClass &cload );

	static	void	Display_Points( float points );
	static	void	Toggle_Hide_Points( void );
	static	bool	Is_Enabled( void );
	static	void	Enable( bool );

	static	void	Force_Weapon_Chart_Update( void );
	static	void	Force_Weapon_Chart_Display( void );

	// Powerups
	static	void	Add_Powerup_Weapon( int id, int rounds );
	static	void	Add_Powerup_Ammo( int id, int rounds );
	static	void	Add_Shield_Grant( float strength );
	static	void	Add_Health_Grant( float amount );
	static	void	Add_Shield_Upgrade_Grant( float strength );
	static	void	Add_Health_Upgrade_Grant( float amount );
	static	void	Add_Key_Grant( int key );
	static	void	Add_Objective( int type );
	static	void	Add_Data_Link( void );
	static	void	Add_Map_Reveal( void );

	// Damage rendering support
	static	void	Damage_Render( void );
};

#endif
