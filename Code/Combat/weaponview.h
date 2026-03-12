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
 *                     $Archive:: /Commando/Code/Combat/weaponview.h                          $*
 *                                                                                             *
 *                      $Author:: Byon_g                                                      $*
 *                                                                                             *
 *                     $Modtime:: 1/03/02 5:09p                                               $*
 *                                                                                             *
 *                    $Revision:: 18                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef	WEAPONVIEW_H
#define	WEAPONVIEW_H

#ifndef	ALWAYS_H
	#include "always.h"
#endif

#ifndef	VECTOR3_H
	#include "vector3.h"
#endif

class	ChunkSaveClass;
class	ChunkLoadClass;

/*
**
*/
class	WeaponViewClass {

public:
	static	void	Init();
	static	void 	Shutdown();
	static	void 	Enable( bool enable );
	static	void 	Think();
	static	void 	Reset();

	static	bool	Save( ChunkSaveClass &csave );
	static	bool	Load( ChunkLoadClass &cload );

	static	Vector3	Get_Muzzle_Pos();
};

#endif
