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
// Filename:     regfloat.h
// Author:       Tom Spencer-Smith
// Date:         Dec 1998
// Description:  
//
//-----------------------------------------------------------------------------
#if defined(_MSV_VER)
#pragma once
#endif

#ifndef REGFLOAT_H
#define REGFLOAT_H

#include "bittype.h"

//-----------------------------------------------------------------------------
class cRegistryFloat
{
	public:
      cRegistryFloat(const char *registry_location, const char *key_name, float initial_value);

      void Set(float value);
      float Get(void) const {return Value;}

	private:

      float	Value;
      char	RegistryLocation[400];
      char	KeyName[100];
};

//-----------------------------------------------------------------------------
#endif // REGFLOAT_H
