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
*     $Author: Patrick $
*     $Revision: 10 $
*     $Modtime: 8/29/01 4:04p $
*     $Archive: /Commando/Code/Scripts/DLLmain.cpp $
*
******************************************************************************/

#include <windows.h>
#include "scripts.h"
#include "ScriptRegistrar.h"
//#include "missioncontrol.h"
#include "DPrint.h"

/******************************************************************************
*
* NAME
*     DllMain
*
* DESCRIPTION
*     Main DLL entry point
*
* INPUTS
*     HINSTANCE hinst
*     DWORD reason
*     LPVOID
*
* RESULTS
*     BOOL APIENTRY
*
******************************************************************************/

__declspec(dllexport)
BOOL APIENTRY DllMain(HINSTANCE /*hinst*/, DWORD reason, LPVOID)
{
	if (reason == DLL_PROCESS_ATTACH) {
//		DebugPrint("\n========== Script.dll loaded ==========\n");
		DebugPrint("Total registered scripts: %d\n", ScriptRegistrar::Count());

	} else if (reason == DLL_PROCESS_DETACH) {
//		DebugPrint("\n========== Script.dll Unloaded ==========\n");
	}

	return true;
}


/******************************************************************************
*
* NAME
*     Create_Script
*
* DESCRIPTION
*     DLL entry to create a script instance.
*
* INPUTS
*     const char* name
*
* RESULTS
*     ScriptClass*
*
******************************************************************************/

ScriptClass* Create_Script(const char* name)
{
	return ScriptRegistrar::CreateScript(name);
}


/******************************************************************************
*
* NAME
*     Destroy_Script
*
* DESCRIPTION
*     DLL entry to destroy a script instance
*
* INPUTS
*     ScriptClass* script
*
* RESULTS
*     NONE
*
******************************************************************************/

void Destroy_Script(ScriptClass* script)
{
	assert(script != NULL);
	delete script;
}


/******************************************************************************
*
* NAME
*     Get_Script_Count
*
* DESCRIPTION
*     DLL entry to count the number of registered scripts
*
* INPUTS
*     NONE
*
* RESULTS
*     Count
*
******************************************************************************/

int Get_Script_Count(void)
{
	return ScriptRegistrar::Count();
}


/******************************************************************************
*
* NAME
*     Get_Script_Name
*
* DESCRIPTION
*     DLL entry to retrieve script name.
*
* INPUTS
*     int index
*
* RESULTS
*     const char*
*
******************************************************************************/

const char* Get_Script_Name(int index)
{
	ScriptFactory* factory = ScriptRegistrar::GetScriptFactory(index);

	if (factory != NULL) {
		return factory->GetName();
	}

	return NULL;
}


/******************************************************************************
*
* NAME
*     Get_Script_Param_Description
*
* DESCRIPTION
*     DLL entry to retrieve Script parameter description.
*
* INPUTS
*     int index
*
* RESULTS
*     const char*
*
******************************************************************************/

const char* Get_Script_Param_Description(int index)
{
	ScriptFactory* factory = ScriptRegistrar::GetScriptFactory(index);

	if (factory != NULL) {
		return factory->GetParamDescription();
	}

	return NULL;
}


/******************************************************************************
*
* NAME
*     Set_Script_Commands
*
* DESCRIPTION
*     DLL entry to initialize script commands hooks.
*
* INPUTS
*     ScriptCommandsClass* commands
*
* RESULTS
*     Success - True if successful; otherwise false.
*
******************************************************************************/

bool Set_Script_Commands(ScriptCommandsClass* commands)
{
	assert(commands != NULL);

	// Save the commands list
	Commands = commands->Commands;

	DebugPrint("Setting script commands (Version %d, Size %d)\n",
		Commands->Version, Commands->Size);

	// Check the commands version number
	if ((Commands->Size != sizeof(ScriptCommands))
			|| (Commands->Version != SCRIPT_COMMANDS_VERSION)) {

		DebugPrint("***** Invalid script commands (Expected Version %d, Size %d)\n",
			SCRIPT_COMMANDS_VERSION, sizeof(ScriptCommands));
		Commands->Debug_Message("******** Incorrect Script Commands Version\n");
		return false;
	}

	return true;
}


/******************************************************************************
*
* NAME
*     Set_Request_Destroy_Func
*
* DESCRIPTION
*
* INPUTS
*     Function
*
* RESULTS
*     NONE
*
******************************************************************************/

void Set_Request_Destroy_Func(void (*function)(ScriptClass*))
{
	assert(function != NULL);
	ScriptImpClass::Set_Request_Destroy_Func(function);
}
