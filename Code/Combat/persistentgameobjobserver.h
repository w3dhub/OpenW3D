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
 *                     $Archive:: /Commando/Code/Combat/persistentgameobjobserver.h           $* 
 *                                                                                             * 
 *                      $Author:: Byon_g                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 11/28/01 5:18p                                              $* 
 *                                                                                             * 
 *                    $Revision:: 4                                                           $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef	PERSISTENTGAMEOBJOBSERVER_H
#define	PERSISTENTGAMEOBJOBSERVER_H

#ifndef	ALWAYS_H
	#include "always.h"
#endif

#ifndef	GAMEOBJOBSERVER_H
	#include "gameobjobserver.h"
#endif

#ifndef	PERSIST_H
	#include "persist.h"
#endif

#ifndef	VECTOR_H
	#include "vector.h"
#endif


/*
** PersistentGameObjObserverClass
*/
class	PersistentGameObjObserverClass : public PersistClass, public GameObjObserverClass {
public:
				 PersistentGameObjObserverClass( void );
	virtual	~PersistentGameObjObserverClass( void );

	virtual	bool	Save( ChunkSaveClass & csave ) override;
	virtual	bool	Load( ChunkLoadClass & cload ) override;

};

/*
** class	PersistentGameObjObserverManager
*/
class	PersistentGameObjObserverManager {
public:
	static	void	Add( PersistentGameObjObserverClass * observer );
	static	void	Remove( PersistentGameObjObserverClass * observer );

	static	bool	Save( ChunkSaveClass & csave );
	static	bool	Load( ChunkLoadClass & cload );

	static	void	Reset( void );

private:
	static	DynamicVectorClass<PersistentGameObjObserverClass *>	ObserverList;
};

#endif	//	PERSISTENTGAMEOBJOBSERVER_H
