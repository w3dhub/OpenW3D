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
 *                     $Archive:: /Commando/Code/Combat/combatsaveload.h                      $* 
 *                                                                                             * 
 *                      $Author:: Byon_g                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 11/02/00 6:08p                                              $* 
 *                                                                                             * 
 *                    $Revision:: 5                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef	COMBATSAVELOAD_H
#define	COMBATSAVELOAD_H

#ifndef	ALWAYS_H
	#include "always.h"
#endif

#ifndef	SAVELOADSUBSYSTEM_H
	#include "saveloadsubsystem.h"
#endif

#ifndef	COMBATCHUNKID_H
	#include "combatchunkid.h"
#endif

/*
**
*/
class CombatSaveLoadClass : public SaveLoadSubSystemClass {

public:
	CombatSaveLoadClass(void) {}
	virtual ~CombatSaveLoadClass(void) {}

	virtual uint32	Chunk_ID (void) const override { return CHUNKID_COMBAT; }

protected:
	virtual bool	Save( ChunkSaveClass &csave ) override;
	virtual bool	Load( ChunkLoadClass &cload ) override;
	virtual const char*		Name() const override { return "CombatSaveLoadClass"; }
	virtual void	On_Post_Load (void) override;

};

/*
**
*/
extern	CombatSaveLoadClass	_CombatSaveLoad;

#endif	//	COMBATSAVELOAD_H
