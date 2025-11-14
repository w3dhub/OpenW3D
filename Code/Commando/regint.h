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
// Filename:     regint.h
// Author:       Tom Spencer-Smith
// Date:         Dec 1998
// Description:  
//
//-----------------------------------------------------------------------------
#if defined(_MSV_VER)
#pragma once
#endif

#ifndef REGINT_H
#define REGINT_H

#include "bittype.h"

//-----------------------------------------------------------------------------
class cRegistryInt
{
	public:
      cRegistryInt(const char *registry_location, const char *key_name, int initial_value);

      void Set(int value);
      int Get(void) const {return Value;}

	private:

      int Value;
      char RegistryLocation[400];
      char KeyName[100];
};

//-----------------------------------------------------------------------------
#endif // REGINT_H
