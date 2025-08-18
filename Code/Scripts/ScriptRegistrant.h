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

/******************************************************************************
*
* FILE
*
* DESCRIPTION
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*
* VERSION INFO
*     $Author: Byon_g $
*     $Revision: 2 $
*     $Modtime: 10/30/00 6:47p $
*     $Archive: /Commando/Code/Scripts/ScriptRegistrant.h $
*
******************************************************************************/

#ifndef _SCRIPTREGISTRANT_H_
#define _SCRIPTREGISTRANT_H_

#include <cstdlib>
#include "ScriptFactory.h"

// Script factory registrant
template<class T>
class	ScriptRegistrant : public ScriptFactory
	{
	public:
		ScriptRegistrant(const char* name, const char* param)
			: ScriptFactory(name, param)
			{}

		virtual ScriptImpClass* Create(void)
			{
			T* script = new T;
			assert(script != NULL);
			script->SetFactory(this);
			script->Register_Auto_Save_Variables();
			return script;
			}
	};


// Register script factory
#define REGISTER_SCRIPT(x, d) \
	class x; \
	ScriptRegistrant<x> _## x ##Registrant(#x, d);


#endif // _SCRIPTREGISTRANT_H_
