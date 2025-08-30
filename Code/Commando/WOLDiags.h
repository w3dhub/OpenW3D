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

#ifdef WWDEBUG
#ifndef __WOLDIAGS_H__
#define __WOLDIAGS_H__
/******************************************************************************
*
* FILE
*     $Archive: /Commando/Code/Commando/WOLDiags.h $
*
* DESCRIPTION
*
* PROGRAMMER
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Revision: 2 $
*     $Modtime: 9/21/01 3:14p $
*
******************************************************************************/

#include "consolefunction.h"

class WOLConsoleFunctionClass : public ConsoleFunctionClass {
public:
	const char* Get_Name(void) override
		{return ("wol");}

	const char* Get_Help(void) override;
	
	void Activate(const char* input) override;
};

#endif // __WOLDIAGS_H__
#endif // WWDEBUG
