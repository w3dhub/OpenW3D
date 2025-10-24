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

/*********************************************************************************************** 
 ***                            Confidential - Westwood Studios                              *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : Commando                                                     * 
 *                                                                                             * 
 *                     $Archive:: /Commando/Code/Combat/scriptevents.h                        $* 
 *                                                                                             * 
 *                      $Author:: Byon_g                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 11/02/00 6:18p                                              $* 
 *                                                                                             * 
 *                    $Revision:: 56                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef	SCRIPTEVENTS_H
#define	SCRIPTEVENTS_H

#include	"gameobjobserver.h"


/*
** Types
*/
class ScriptCommandsClass;
class ScriptSaver;
class ScriptLoader;

/*
** Script Class
*/
class	ScriptClass : public GameObjObserverClass
{
	public:
		virtual ~ScriptClass() {}

//		virtual const char* Get_Name(void) = 0;

		virtual GameObject* Owner() = 0;

		virtual GameObject** Get_Owner_Ptr() = 0;

		virtual void Set_Parameters_String(const char* params) = 0;

		virtual void Get_Parameters_String(char* buffer, unsigned int size) = 0;

		// Save and Load specific script
		virtual	void Save(ScriptSaver& saver) = 0;
		virtual	void Load(ScriptLoader& loader) = 0;
};


/*
** DLL import/export macros
*/
#ifdef _WIN32
#ifdef BUILDING_DLL
	#define	SCRIPT_DLL_FUNCT extern "C" __declspec(dllexport)
#else
	#define	SCRIPT_DLL_FUNCT extern "C" __declspec(dllimport)
#endif
#else
    #define SCRIPT_DLL_FUNCT extern "C"
#endif


const char* const LPSTR_CREATE_SCRIPT = "Create_Script";
typedef ScriptClass* (*LPFN_CREATE_SCRIPT)(const char*);
SCRIPT_DLL_FUNCT ScriptClass* Create_Script(const char* name);


const char* const LPSTR_DESTROY_SCRIPT = "Destroy_Script";
typedef void (*LPFN_DESTROY_SCRIPT)(ScriptClass*);
SCRIPT_DLL_FUNCT void	Destroy_Script(ScriptClass* script);


const char* const LPSTR_SET_SCRIPT_COMMANDS = "Set_Script_Commands";
typedef bool (*LPFN_SET_SCRIPT_COMMANDS)(ScriptCommandsClass*);
SCRIPT_DLL_FUNCT bool Set_Script_Commands(ScriptCommandsClass* commands);


const char* const LPSTR_SET_REQUEST_DESTROY_FUNC = "Set_Request_Destroy_Func";
typedef void (*LPFN_SET_REQUEST_DESTROY_FUNC)(void (*function)(ScriptClass*));
SCRIPT_DLL_FUNCT void Set_Request_Destroy_Func(void (*function)(ScriptClass*));


const char* const LPSTR_GET_SCRIPT_COUNT = "Get_Script_Count";
typedef int (*LPFN_GET_SCRIPT_COUNT)(void);
SCRIPT_DLL_FUNCT int Get_Script_Count(void);


const char* const LPSTR_GET_SCRIPT_NAME = "Get_Script_Name";
typedef const char* (*LPFN_GET_SCRIPT_NAME)(int);
SCRIPT_DLL_FUNCT const char* Get_Script_Name(int index);


const char* const LPSTR_GET_SCRIPT_PARAM_DESCRIPTION = "Get_Script_Param_Description";
typedef const char* (*LPFN_GET_SCRIPT_PARAM_DESCRIPTION)(int);
SCRIPT_DLL_FUNCT const char* Get_Script_Param_Description(int index);


/*
** Script parameter datatype definitions
*/
typedef enum
{
	PARAM_TYPE_INT = 0,
	PARAM_TYPE_FLOAT,
	PARAM_TYPE_STRING,
	PARAM_TYPE_BOOL,
	PARAM_TYPE_ID,
	PARAM_TYPE_VECTOR3,
	PARAM_TYPE_ENUM,
	PARAM_TYPE_EMITTER,
	PARAM_TYPE_WEAPON,
	PARAM_TYPE_AMMO,
	PARAM_TYPE_EXPLOSION,
	PARAM_TYPE_ANIMATION,
	PARAM_TYPE_GANG,
	PARAM_TYPE_FILE,
	PARAM_TYPE_SOUND,
	PARAM_TYPE_COLOR,
	PARAM_TYPE_COUNT
} PARAM_TYPES;


const char * const PARAM_TYPE_STRINGS[PARAM_TYPE_COUNT] =
{
	"int",
	"float",
	"string",
	"bool",
	"ID",
	"vector3",
	"enum",
	"emitter",
	"weapon",
	"ammo",
	"explosion",
	"animation",
	"gang",
	"file",
	"sound",
	"color",
};

#endif	//	SCRIPTEVENTS_H
