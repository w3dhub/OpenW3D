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
// Filename:     boolean.h
// Author:       Tom Spencer-Smith
// Date:         Nov 1999
// Description:
//
//-----------------------------------------------------------------------------
#if defined(_MSV_VER)
#pragma once
#endif

#ifndef BOOLEAN_H
#define BOOLEAN_H

//-----------------------------------------------------------------------------
class cBoolean
{
	public:
      cBoolean(bool value = false)  {Value = value;}
      bool Toggle(void)             {Value = !Value; return Value;}
      bool Set(bool value)          {Value = value; return Value;}
      bool Get(void)       const    {return Value;}
      bool Is_True(void)   const    {return Value == true;}
      bool Is_False(void)  const    {return Value == false;}

	private:

      bool Value;
};

//-----------------------------------------------------------------------------
#endif // BOOLEAN_H
