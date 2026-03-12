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
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : WWPhys                                                       *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwphys/bpt.h                                 $*
 *                                                                                             *
 *                        Author:: Greg_h                                                      *
 *                                                                                             *
 *                     $Modtime:: 9/09/99 11:29a                                              $*
 *                                                                                             *
 *                    $Revision:: 12                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#if 0 //OBSOLETE
#ifdef PORT140

#ifndef BPT_H
#define BPT_H

#include "always.h"
#include "lineseg.h"
#include "aabox.h"
#include "obbox.h"
#include "physcoltest.h"
#include "rendobj.h"


class srScene;
class BptImpClass;
class MeshClass;
class CameraClass;
class ChunkLoadClass;
class ChunkSaveClass;


/*
** BptClass - Mesh with an auxiliary binary partition tree.
** The bpt is traversed in collision detection queries and to
** build the APT for the mesh each frame.
*/
class BptClass : public Render3DObjClass
{

public:

	BptClass(void);
	BptClass(const BptClass & src)					{ assert(0); }
	BptClass & operator = (const BptClass &)		{ assert(0); return *this; }

	virtual							~BptClass(void);

	/*
	** Release all assets in use, called by destructor and the load function
	*/
	void								Free(void);

	/*
	** Required clone function for all render objects.
	** Currently not implemented.
	*/
	virtual RenderObjClass *	Clone(void) const	{ return new BptClass(*this); }

	/*
	** Set Mesh, re-builds the bsp data for the given mesh, also generates
	** a new mesh due to splitting required by the partitioning process
	*/
	void								Build(MeshClass * mesh);

	/*
	** File IO
	*/
	int								Load(ChunkLoadClass & cload);
	int								Save(ChunkSaveClass & csave);

	/*
	** Function to update the rendering object - this will be called every
	** frame that the object is in view.
	*/
	virtual void					Render( srScene * scene, const CameraClass &camera );

	/*
	** Collision Detection Support
	*/
	bool								Cast_Ray(PhysRayCollisionTestClass & ray) const;
	bool								Cast_AABox(PhysAABoxCollisionTestClass & aaboxtest) const;
	bool								Cast_OBBox(PhysOBBoxCollisionTestClass & obboxtest) const;

//	MeshClass *						Get_Mesh(void);

protected:

	void								Update_Cached_Bounding_Volumes(void) const;

	BptImpClass	*					BptImp;

};

#endif

#endif //PORT140
#endif
