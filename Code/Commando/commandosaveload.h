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
 *                     $Archive:: /Commando/Code/Commando/commandosaveload.h                  $* 
 *                                                                                             * 
 *                      $Author:: Byon_g                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 11/02/00 6:11p                                              $* 
 *                                                                                             * 
 *                    $Revision:: 4                                                           $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef	COMMANDOSAVELOAD_H
#define	COMMANDOSAVELOAD_H

#ifndef	ALWAYS_H
	#include "always.h"
#endif

#ifndef	SAVELOADSUBSYSTEM_H
	#include "saveloadsubsystem.h"
#endif

#ifndef	COMMANDOCHUNKIDS_H
	#include "commandochunkids.h"
#endif

/*
**
*/
class CommandoSaveLoadClass : public SaveLoadSubSystemClass {

public:
	CommandoSaveLoadClass(void) {}
	virtual ~CommandoSaveLoadClass(void) {}

	virtual uint32	Chunk_ID (void) const override { return CHUNKID_COMMANDO; }

protected:
	virtual bool	Save( ChunkSaveClass &csave ) override;
	virtual bool	Load( ChunkLoadClass &cload ) override;
	virtual const char* Name() const override { return "CommandoSaveLoadClass"; }
};

/*
**
*/
extern	CommandoSaveLoadClass	_CommandoSaveLoad;

#endif	//	COMBATSAVELOAD_H
