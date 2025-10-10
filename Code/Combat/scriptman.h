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
 *                     $Archive:: /Commando/Code/Combat/scriptman.h                             $* 
 *                                                                                             * 
 *                      $Author:: Byon_g                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 8/06/01 6:48p                                               $* 
 *                                                                                             * 
 *                    $Revision:: 26                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef	SCRIPTMAN_H
#define	SCRIPTMAN_H

#ifndef	ALWAYS_H
	#include "always.h"
#endif

#ifndef	SCRIPTEVENTS_H
	#include "scriptevents.h"
#endif

#ifndef	SIMPLEVEC_H
	#include "simplevec.h"
#endif


/*
**
*/
class		PhysicalGameObj;
class		ChunkSaveClass;
class		ChunkLoadClass;

#define	SCRIPT_MAX_PARAMS		100

/*
**
*/
class		ScriptSaver	{
public:
	ScriptSaver( ChunkSaveClass & csave	) : CSave( csave ) {}
	operator ChunkSaveClass	& (void) const { return CSave; }
	ChunkSaveClass	& CSave;
};

class		ScriptLoader	{
public:
	ScriptLoader( ChunkLoadClass & cload	) : CLoad( cload ) {}
	operator ChunkLoadClass	& (void) const { return CLoad; }
	ChunkLoadClass	& CLoad;
};


/*
** Script Manager
*/
class	ScriptManager {

public:
	static	void				Init( void );
	static	void				Shutdown( void );

	// Create a script.  Add to the active list
	static	ScriptClass	*	Create_Script( const char * script_name );
	// Add Script to the Destroy List
	static	void				Request_Destroy_Script( ScriptClass * script );
	// Destroy scripts from the destroy list
	static	void				Destroy_Pending( void );

	// Save & load all active scripts
	static	bool				Save( ChunkSaveClass & csave );
	static	bool				Load( ChunkLoadClass & cload );

	static	void				Enable_Script_Creation( bool enable ) { EnableScriptCreation = enable; }

private:
	static void Load_Scripts(const char * dll_filename);

	static LPFN_CREATE_SCRIPT ScriptCreateFunct;
	static LPFN_DESTROY_SCRIPT ScriptDestroyFunct;

	static SimpleDynVecClass<ScriptClass *> ActiveScriptList;
	static SimpleDynVecClass<ScriptClass *> PendingDestroyList;

	static bool	EnableScriptCreation;
};

#endif		// SCRIPTMAN_H