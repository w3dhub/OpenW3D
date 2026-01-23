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
 *                     $Archive:: /Commando/Code/wwphys/renderobjphys.h                       $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 8/13/00 11:17a                                              $*
 *                                                                                             *
 *                    $Revision:: 5                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef RENDEROBJPHYS_H
#define RENDEROBJPHYS_H

#include "always.h"
#include "dynamicphys.h"
#include "pscene.h"
#include "wwdebug.h"
#include "physcoltest.h"


/**
** RenderObjPhysClass
** The *ONLY* purpose of this class is as a wrapper when render objects are added 
** directly to the physics scene.  This class plugs a pointer back to it into the UserData 
** field of its render object.  If you find yourself creating these objects,
** consider using DecorationPhysClass instead...
*/
class RenderObjPhysClass : public DynamicPhysClass
{
public:

	RenderObjPhysClass(void);
	~RenderObjPhysClass(void);

	virtual void					Timestep(float /* dt */) override											{ }
	virtual void					Set_Model(RenderObjClass * model) override;
	virtual bool					Cast_Ray(PhysRayCollisionTestClass & raytest) override;
	virtual bool					Cast_AABox(PhysAABoxCollisionTestClass & boxtest) override;
	virtual bool					Cast_OBBox(PhysOBBoxCollisionTestClass & boxtest) override;

	virtual const AABoxClass & Get_Bounding_Box(void) const;
	virtual const Matrix3D &	Get_Transform(void) const override;
	virtual void					Set_Transform(const Matrix3D & m) override;

	virtual RenderObjPhysClass *	As_RenderObjPhysClass(void) override							{ return this; }

	/*
	** Save-Load 
	*/
	virtual const PersistFactoryClass &	Get_Factory (void) const override;
	virtual bool								Save (ChunkSaveClass &csave) override;
	virtual bool								Load (ChunkLoadClass &cload) override;
	virtual void								On_Post_Load (void) override;


private:

	// Not implemented...
	RenderObjPhysClass(const RenderObjPhysClass &);
	RenderObjPhysClass & operator = (const RenderObjPhysClass &);

};


#endif

