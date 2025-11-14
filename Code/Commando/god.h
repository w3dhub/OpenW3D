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

//
// Filename:     god.h
// Author:       Tom Spencer-Smith
// Date:         Dec 1998
// Description:  
//
//-----------------------------------------------------------------------------
#if defined(_MSV_VER)
#pragma once
#endif

#ifndef GOD_H
#define GOD_H

#include "inventory.h"

class SoldierGameObj;
class cPlayer;
class Vector3;
class ChunkSaveClass;
class ChunkLoadClass;
class WideStringClass;

//-----------------------------------------------------------------------------
class cGod
{
	public:
		static void					Think(void);
		static cPlayer *			Create_Player(int client_id, const WideStringClass & name,
														int team_choice, unsigned int clanID, bool is_invulnerable = false);

		static void					Create_Ai_Player(void);
		static void					Create_Grunt(Vector3 & pos);

#ifdef WWDEBUG
		static void					Reinitialize_Ai_On_Star(void);
#endif // WWDEBUG

		static void					Reset( void );
		static void					Star_Killed( void );
		static void					Respawn( void );
		static void					Restart( void );
		static void					Load_Game( void );
		static void					Mission_Failed( void );
		static void					Exit( void );

		static	bool				Save(ChunkSaveClass & csave);
		static	bool				Load(ChunkLoadClass & cload);

		static	void				Store_Inventory( SoldierGameObj * );
		static	void				Restore_Inventory( SoldierGameObj * );
		static	void				Reset_Inventory( void );

	private:
		static SoldierGameObj * Create_Commando(int client_id, int player_type);
		static SoldierGameObj * Create_Commando(cPlayer * p_player);

		static	int				State;

		static	InventoryClass	LevelStartInventory;
};

//-----------------------------------------------------------------------------
#endif // GOD_H

