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
 *                     $Archive:: /Commando/Code/Combat/scripts.cpp                           $* 
 *                                                                                             * 
 *                      $Author:: Greg_h                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 7/09/02 9:19a                                               $* 
 *                                                                                             * 
 *                    $Revision:: 53                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "scriptman.h"
#include "debug.h"
#include "scriptcommands.h"
#include "physicalgameobj.h"
#include "wwstring.h"
#include "combat.h"
#include "wwprofile.h"
#include "ffactorylist.h"
#include "rawfile.h"
#include "gametype.h"
#include <stdio.h>
#include <win.h>

ScriptCommands* EngineCommands = NULL;

#if 1
#define	SCRIPT_PROFILE_START( x )	WWProfileManager::Profile_Start( "Scripts" );
#define	SCRIPT_PROFILE_STOP( x )	WWProfileManager::Profile_Stop( ); 
#else
#define	SCRIPT_PROFILE_START( x )
#define	SCRIPT_PROFILE_STOP( x ) 
#endif

/*
**
*/
HINSTANCE hDLL = NULL;
LPFN_CREATE_SCRIPT ScriptManager::ScriptCreateFunct = NULL;
LPFN_DESTROY_SCRIPT ScriptManager::ScriptDestroyFunct = NULL;
SimpleDynVecClass<ScriptClass *> ScriptManager::ActiveScriptList;
SimpleDynVecClass<ScriptClass *> ScriptManager::PendingDestroyList;
bool	ScriptManager::EnableScriptCreation = true;



/*
**
*/
void ScriptManager::Init(void)
{
	hDLL = NULL;
	EngineCommands = Get_Script_Commands();

#ifdef	PARAM_EDITING_ON	// Editor build
	Load_Scripts("SCRIPTS.DLL");
#else
	#ifdef	WWDEBUG		// DEBUG and PROFILE
		if ( DebugManager::Load_Debug_Scripts() ) {
			Load_Scripts("SCRIPTSD.DLL");		// DEBUG
		} else {
	#ifdef	NDEBUG		// PROFILE
			Load_Scripts("SCRIPTSP.DLL");		// PROFILE
	#else
			Load_Scripts("SCRIPTSD.DLL");		// DEBUG
	#endif
		}
	#else
		Load_Scripts("SCRIPTS.DLL");		// RELEASE
	#endif
#endif
}


/*
**
*/
void ScriptManager::Shutdown(void)
{
	// Release scripts
	while (ActiveScriptList.Count()) {
		ScriptClass* script = ActiveScriptList[0];
		assert(script != NULL);

		assert(ScriptDestroyFunct != NULL);
		ScriptDestroyFunct(script);
		
		ActiveScriptList.Delete(0);
	}

	if (hDLL != NULL) {
		FreeLibrary(hDLL);
		hDLL = NULL;
	}
}


void ScriptManager::Destroy_Pending(void)
{
	// Destroy all the scripts in the pending destroy list.
	while (PendingDestroyList.Count()) {
		ScriptClass* script = PendingDestroyList[0];
		assert(script != NULL);

		// If the script has an owner then it must be detached before it
		// can be destroyed.
		ScriptableGameObj* object = script->Owner();

		if (object != NULL) {
			object->Remove_Observer(script);
		}

		// Destroy the script
		assert(ScriptDestroyFunct != NULL);
		ScriptDestroyFunct(script);
		PendingDestroyList.Delete(0);
	}
}


/*
**
*/
void ScriptManager::Load_Scripts(const char* dll_filename)
{
	char dll_fullpath[MAX_PATH];
	{
		char path_to_exe[MAX_PATH];
		char fdrive[_MAX_DRIVE];
		char fdir[_MAX_DIR];
		GetModuleFileNameA(NULL, path_to_exe, sizeof(path_to_exe));
		_splitpath(path_to_exe, fdrive, fdir, NULL, NULL);
		_makepath(dll_fullpath, fdrive, fdir, dll_filename, NULL);
		dll_filename = dll_fullpath;
	}

	Debug_Say(("Script Manager Loading Script File %s\n", dll_filename));


	// If we're in multiplay and not the server, just bail
   if (!IS_SOLOPLAY && CombatManager::I_Am_Only_Client())
	{
		return;
	}

#ifndef	PARAM_EDITING_ON	// Only do this in the *game*

	// Check if we have a mod, if so, un-pack the scripts from the PKG (if present)
	FileFactoryClass * mod_pkg = FileFactoryListClass::Get_Instance()->Peek_Temp_FileFactory();
	if (mod_pkg != NULL) {
		FileClass * scripts_dll = mod_pkg->Get_File( dll_filename );
		if ((scripts_dll != NULL) && (scripts_dll->Is_Available())) {

			const char * _TMP_SCRIPTS_DLL_FILENAME = "_MOD_SCRIPTS.DLL";

			scripts_dll->Open(FileClass::READ);
			RawFileClass unpacked_scripts(_TMP_SCRIPTS_DLL_FILENAME);

			if (unpacked_scripts.Create()) {
		
				unpacked_scripts.Open(FileClass::WRITE);

				// Copy the dll from the PKG (mix) file into our temporary _scripts directory
				static char buffer[16000];
				int scripts_size = scripts_dll->Size();
				int cur_pos = 0;
				while (cur_pos < scripts_size) {
					int read_count = WWMath::Min(scripts_size - cur_pos,sizeof(buffer));
					scripts_dll->Read(buffer,read_count);
					unpacked_scripts.Write(buffer,read_count);
					cur_pos += read_count;
				}


				// change 'dll_filename' so that we load the newly created dll
				if (cur_pos == scripts_size) {
					dll_filename = _TMP_SCRIPTS_DLL_FILENAME;
				}

				unpacked_scripts.Close();
			}
			scripts_dll->Close();
			mod_pkg->Return_File(scripts_dll);
		}
	}
#endif

	hDLL = ::LoadLibraryA(dll_filename);

	if (hDLL == NULL) {
		Debug_Say(("Cound not load DLL file %s\n", dll_filename));
		return;
	}

	// Get create script function
	ScriptCreateFunct = (LPFN_CREATE_SCRIPT)GetProcAddress(hDLL, LPSTR_CREATE_SCRIPT);
	assert(ScriptCreateFunct != NULL);

	if (!ScriptCreateFunct) {
		Debug_Say(("Cound not find Create_Script\n"));
	}

	// Get destroy script function
	ScriptDestroyFunct = (LPFN_DESTROY_SCRIPT)GetProcAddress(hDLL, LPSTR_DESTROY_SCRIPT);
	assert(ScriptDestroyFunct != NULL);

	if (!ScriptDestroyFunct) {
		Debug_Say(("Cound not find Destroy_Script\n"));
	}

	// Initialize request script destroy function
	LPFN_SET_REQUEST_DESTROY_FUNC set_request_destroy_func = 
		(LPFN_SET_REQUEST_DESTROY_FUNC)GetProcAddress(hDLL, LPSTR_SET_REQUEST_DESTROY_FUNC);
	assert(set_request_destroy_func != NULL);

	if (set_request_destroy_func != NULL) {
		set_request_destroy_func(Request_Destroy_Script);
	} else {
		Debug_Say(("Cound not find Set_Request_Destroy_Func\n"));
	}

	// Initialize script commands if not being run from the editor
	if (CombatManager::Are_Observers_Active()) {
		LPFN_SET_SCRIPT_COMMANDS set_commands_func =
			(LPFN_SET_SCRIPT_COMMANDS)GetProcAddress(hDLL, LPSTR_SET_SCRIPT_COMMANDS);
		assert(set_commands_func != NULL);

		if (set_commands_func != NULL) {
			ScriptCommandsClass commands;
			commands.Commands = EngineCommands;

			bool success = set_commands_func(&commands);

			if (!success) {
				Debug_Say(("Failed to set script commands!\n"));

				// This should keep us from going to scripts!
				ScriptCreateFunct = NULL;
			}
		} else {
			Debug_Say(("Cound not find Set_Script_Commands\n"));
		}
	}
}


/*
**
*/
ScriptClass* ScriptManager::Create_Script(const char* script_name)
{
	ScriptClass* script = NULL;

	if (EnableScriptCreation && ScriptCreateFunct != NULL) {
		script = ScriptCreateFunct(script_name);

		if (script != NULL) {
			script->Set_ID( GameObjObserverManager::Get_Next_Observer_ID() );
			ActiveScriptList.Add(script);
		}
	}

	return script;
}


/*
**
*/
void ScriptManager::Request_Destroy_Script(ScriptClass* script)
{
	ActiveScriptList.Delete(script);

	// Do not add the script to the destroy list if it is already there.
	for (int index = 0; index < PendingDestroyList.Count(); index++) {
		if (PendingDestroyList[index] == script) {
			return;
		}
	}

	PendingDestroyList.Add(script);
}


/*
** Script Manager Save and Load
*/
enum	{
	CHUNKID_SCRIPT_ENTRY					=	131001134,
	CHUNKID_SCRIPT_HEADER,
	CHUNKID_SCRIPT_DATA,

	MICROCHUNKID_NAME						= 1,

// Denzil 3/31/00 - This information is now saved by the script.
#if(0)
	MICROCHUNKID_PARAM_COUNT,
#endif
	MICROCHUNKID_PARAM,
	MICROCHUNKID_GAME_OBJ_OBSERVER_PTR,
	MICROCHUNKID_OWNER_PTR,
	MICROCHUNKID_ID,
};


/*
**
*/
bool ScriptManager::Save(ChunkSaveClass& csave)
{
	for (int index = 0; index < ActiveScriptList.Count(); index++) {
		ScriptClass* script = ActiveScriptList[ index ];

		csave.Begin_Chunk( CHUNKID_SCRIPT_ENTRY );
		csave.Begin_Chunk( CHUNKID_SCRIPT_HEADER );

		StringClass name = script->Get_Name();
//		Debug_Say(("Saving script '%s'\n", name));
		WRITE_MICRO_CHUNK_WWSTRING( csave, MICROCHUNKID_NAME, name );

		char paramString[256];
		script->Get_Parameters_String(paramString, sizeof(paramString));
//		Debug_Say(("\tParameters: '%s'\n", paramString));
		WRITE_MICRO_CHUNK_STRING(csave, MICROCHUNKID_PARAM, paramString);

		GameObjObserverClass* game_obj_observer_ptr = (GameObjObserverClass*)script;
		WRITE_MICRO_CHUNK_PTR( csave, MICROCHUNKID_GAME_OBJ_OBSERVER_PTR, game_obj_observer_ptr );

		ScriptableGameObj* owner_ptr = *(script->Get_Owner_Ptr());
//		Debug_Say(("\tObjectPtr: '%p'\n", *owner_ptr));
		WRITE_MICRO_CHUNK_PTR( csave, MICROCHUNKID_OWNER_PTR, owner_ptr );

		int id = script->Get_ID();
		WRITE_MICRO_CHUNK( csave, MICROCHUNKID_ID, id );
//		Debug_Say(( "Saved Script ID %d\n", id ));

		csave.End_Chunk();

		// If data is not saved, script will be re-created
		if (CombatManager::Are_Observers_Active()) {
			csave.Begin_Chunk(CHUNKID_SCRIPT_DATA);
			ScriptSaver	saver(csave);
			script->Save(saver);
			csave.End_Chunk();
		}

		csave.End_Chunk();
	}
	return true;
}


bool	ScriptManager::Load( ChunkLoadClass & cload )
{
	WWASSERT( ActiveScriptList.Count() == 0 );

	while (cload.Open_Chunk()) {

		GameObjObserverClass * game_obj_observer_ptr = NULL;
		PhysicalGameObj * owner_ptr = NULL;

		WWASSERT( cload.Cur_Chunk_ID() == CHUNKID_SCRIPT_ENTRY );

		ScriptClass *script = NULL;

		// Load header
		cload.Open_Chunk();
		WWASSERT( cload.Cur_Chunk_ID() == CHUNKID_SCRIPT_HEADER );

		int obs_id = -1;

//		int param_index = 0;
		while (cload.Open_Micro_Chunk()) {
			int id = cload.Cur_Micro_Chunk_ID();
			switch( id ) {
				case MICROCHUNKID_NAME:
				{
					StringClass	name;
					LOAD_MICRO_CHUNK_WWSTRING( cload, name );
					WWASSERT( script == NULL );
					script = Create_Script( name );
					if ( script == NULL ) {
						Debug_Say(( "Script %s not found \n", name ));
					}

					// A Missing script is not fatal
//					WWASSERT( script != NULL );
					break;
				}

				case MICROCHUNKID_PARAM:
				{
					if ( script != NULL ) {
						StringClass	param;
						LOAD_MICRO_CHUNK_WWSTRING( cload, param );
						script->Set_Parameters_String(param);
					}
					break;
				}

				READ_MICRO_CHUNK_PTR( cload, MICROCHUNKID_GAME_OBJ_OBSERVER_PTR, game_obj_observer_ptr );
				READ_MICRO_CHUNK_PTR( cload, MICROCHUNKID_OWNER_PTR, owner_ptr );

				READ_MICRO_CHUNK( cload, MICROCHUNKID_ID, obs_id );

				default:
					Debug_Say(( "Unrecognized ScriptCollection Header chunkID\n" ));
					break;
			}
			cload.Close_Micro_Chunk();
		}
		cload.Close_Chunk();

		if ( script != NULL ) {

			if ( obs_id != -1 ) {
				script->Set_ID( obs_id );
//				Debug_Say(( "Loaded Script ID %d\n", obs_id ));
			}

			// If there is data, load
			if ( cload.Open_Chunk() ) {
				WWASSERT( cload.Cur_Chunk_ID() == CHUNKID_SCRIPT_DATA );
				ScriptLoader loader( cload );
				script->Load( loader );
				cload.Close_Chunk();
			}

			WWASSERT( game_obj_observer_ptr != NULL );
			if ( game_obj_observer_ptr != NULL ) {
				SaveLoadSystemClass::Register_Pointer(game_obj_observer_ptr, (GameObjObserverClass *)script);
			}

			// set the owner, and request remap
			*(script->Get_Owner_Ptr()) = owner_ptr;
			REQUEST_POINTER_REMAP( (void **)script->Get_Owner_Ptr() );
		} else {
			SaveLoadSystemClass::Register_Pointer(game_obj_observer_ptr, (GameObjObserverClass *)NULL);
		}


		cload.Close_Chunk();
	}
	return true;
}


 