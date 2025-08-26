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
*     $Revision: 4 $
*     $Modtime: 8/16/00 11:58a $
*     $Archive: /Commando/Code/Scripts/ScriptFactory.cpp $
*
******************************************************************************/

#include "ScriptFactory.h"
#include "ScriptRegistrar.h"
#include "scripts.h"
#include "DPrint.h"
#include <string.h>

/******************************************************************************
*
* NAME
*     ScriptFactory::ScriptFactory
*
* DESCRIPTION
*     ScriptFactory constructor
*
* INPUTS
*     Name       - Script name.
*     Parameters - Parameter description string.
*
* RESULTS
*     NONE
*
******************************************************************************/

ScriptFactory::ScriptFactory(const char* name, const char* param)
	: mNext(NULL)
{
	// Save script name
	assert(name != NULL);
	ScriptName = name;

	// Save parameter description
	assert(param != NULL);
	ParamDescription = param;

	// Register this factory with the registrar
	ScriptRegistrar::RegisterScript(this);
}


/******************************************************************************
*
* NAME
*     ScriptFactory::~ScriptFactory
*
* DESCRIPTION
*     ScriptFactory destructor
*
* INPUTS
*     NONE
*
* RESULTS
*     NONE
*
******************************************************************************/

ScriptFactory::~ScriptFactory()
{
	// Remove this factory from the registrar
	ScriptRegistrar::UnregisterScript(this);

	ScriptName = NULL;
	ParamDescription = NULL;
}


/******************************************************************************
*
* NAME
*     ScriptFactory::GetNext
*
* DESCRIPTION
*     Retrieve next script factory.
*
* INPUTS
*     NONE
*
* RESULTS
*     ScriptFactory*
*
******************************************************************************/

ScriptFactory* ScriptFactory::GetNext(void) const
{
	return mNext;
}


/******************************************************************************
*
* NAME
*     ScriptFactory::SetNext
*
* DESCRIPTION
*     Set next script factory.
*
* INPUTS
*     ScriptFactory* link
*
* RESULTS
*     NONE
*
******************************************************************************/

void ScriptFactory::SetNext(ScriptFactory* link)
{
	if (mNext != NULL) {
		assert(link != NULL);
		link->SetNext(mNext);
	}

	mNext = link;
}


/******************************************************************************
*
* NAME
*     ScriptFactory::GetName
*
* DESCRIPTION
*     Retrieve the name.
*
* INPUTS
*     NONE
*
* RESULTS
*     Name - Name of script.
*
******************************************************************************/

const char* ScriptFactory::GetName(void)
{
	return ScriptName;
}


/******************************************************************************
*
* NAME
*     ScriptFactory::GetParamDescription
*
* DESCRIPTION
*     Retrieve the parameter description.
*
* INPUTS
*     NONE
*
* RESULTS
*     Parameters - Parameter description string.
*
******************************************************************************/

const char* ScriptFactory::GetParamDescription(void)
{
	return ParamDescription;
}


