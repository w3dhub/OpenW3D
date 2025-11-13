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
// Filename:     nicenum.cpp
// Author:       Tom Spencer-Smith
// Date:         Dec 1999
// Description:  NIC enumeration
//

#ifndef __NICENUM_H__
#define __NICENUM_H__

#include "bittype.h"

//-----------------------------------------------------------------------------
class cNicEnum 
{
public:
	static void			Init(void);
	static unsigned short		Get_Num_Nics(void)			{return NumNics;}
	static unsigned int *		Get_Nics(void)					{return NicList;}
	static unsigned short		Get_Num_GameSpy_Nics(void)	{return NumGSNics;}
	static unsigned int *		Get_GameSpy_Nics(void)		{return GSNicList;}

	enum					{MAX_NICS = 10};

private:
	static unsigned int		Enumerate_Nics(unsigned int * addresses, unsigned int max_addresses);

	static unsigned int		NicList[MAX_NICS];
	static unsigned int		GSNicList[MAX_NICS];
	static unsigned short		NumNics;
	static unsigned short		NumGSNics;
};

//-----------------------------------------------------------------------------

#endif // __NICENUM_H__




