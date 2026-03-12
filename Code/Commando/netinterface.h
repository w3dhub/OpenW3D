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
// Filename:     netinterface.h
// Project:      Network.lib, for Commando
// Author:       Tom Spencer-Smith
// Date:         Dec 1998
// Description:
//
//-----------------------------------------------------------------------------
#if defined(_MSV_VER)
#pragma once
#endif

#ifndef NETINTERFACE_H
#define NETINTERFACE_H

#include "bittype.h"
#include "widestring.h"

//GAMESPY - allow 30 chars + ( + 2 chars for collision (add 2..99) + ) + NULL
//const USHORT MAX_NICKNAME_LENGTH = 17; // Includes NULL.
const USHORT MAX_NICKNAME_LENGTH = 35;

//-----------------------------------------------------------------------------
class cNetInterface
{
   public:
      cNetInterface(void);
      ~cNetInterface(void);

      static void						Set_Random_Nickname(void);
      static void						Set_Nickname(WideStringClass & name);
      static WideStringClass		Get_Nickname(void);

		static void Set_Side_Preference(int side);
		static int Get_Side_Preference(void);

   private:
      cNetInterface(const cNetInterface& rhs);					// Disallow copy (compile/link time)
      cNetInterface& operator=(const cNetInterface& rhs);	// Disallow assignment (compile/link time)

		static WideStringClass		Nickname;
		static int mSidePreference;
};

//-----------------------------------------------------------------------------
#endif // NETINTERFACE_H




      //static WideStringClass &	Get_Nickname(void);
