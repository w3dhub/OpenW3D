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
 *                     $Archive:: /Commando/Code/wwphys/decophys.h                            $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 1/05/02 3:55p                                               $*
 *                                                                                             *
 *                    $Revision:: 17                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef DECOPHYS_H
#define DECOPHYS_H

#include "always.h"
#include "dynamicphys.h"
#include "pscene.h"
#include "wwdebug.h"
#include "physcoltest.h"


class DecorationPhysDefClass;


/**
** DecorationPhysClass
** Decoration object.  It is an object that can be added into the dynamic physics
** system but doesn't really do anything.  It can be collided against if collision is
** enabled in its render object...
*/
class DecorationPhysClass : public DynamicPhysClass
{
public:

	DecorationPhysClass(void);
	virtual DecorationPhysClass *	As_DecorationPhysClass(void) override							{ return this; }
	
	void									Init(const DecorationPhysDefClass & def);
	virtual void						Timestep(float /* dt */) override										{ }
	virtual void						Set_Model(RenderObjClass * model) override;
	
	virtual bool						Cast_Ray(PhysRayCollisionTestClass & raytest) override;
	virtual bool						Cast_AABox(PhysAABoxCollisionTestClass & boxtest) override;
	virtual bool						Cast_OBBox(PhysOBBoxCollisionTestClass & boxtest) override;

	virtual bool						Intersection_Test(PhysAABoxIntersectionTestClass & test) override;
	virtual bool						Intersection_Test(PhysOBBoxIntersectionTestClass & test) override;

	virtual const AABoxClass &		Get_Bounding_Box(void) const;
	virtual const Matrix3D &		Get_Transform(void) const override;
	virtual void						Set_Transform(const Matrix3D & m) override;
	virtual void						Get_Shadow_Blob_Box(AABoxClass * set_obj_space_box) override;

	// save-load system
	virtual const PersistFactoryClass &	Get_Factory (void) const override;
	virtual bool								Save (ChunkSaveClass &csave) override;
	virtual bool								Load (ChunkLoadClass &cload) override;
	virtual void								On_Post_Load(void) override;

private:

	// Not implemented...
	DecorationPhysClass(const DecorationPhysClass &);
	DecorationPhysClass & operator = (const DecorationPhysClass &);

	AABoxClass							ObjSpaceWorldBox;
};


/**
** DecorationPhysDefClass
** Definition data structure for DecorationPhysClass
*/
class DecorationPhysDefClass : public DynamicPhysDefClass
{
public:
	
	DecorationPhysDefClass(void);
	
	// From DefinitionClass
	virtual uint32								Get_Class_ID (void) const override;
	virtual PersistClass *					Create(void) const override;

	// From PhysDefClass
	virtual const char *						Get_Type_Name(void) override;
	virtual bool								Is_Type(const char *) override;

	// From PersistClass
	virtual const PersistFactoryClass &	Get_Factory (void) const override;
	virtual bool								Save(ChunkSaveClass &csave) override;
	virtual bool								Load(ChunkLoadClass &cload) override;

	//	Editable interface requirements
	DECLARE_EDITABLE(DecorationPhysDefClass,DynamicPhysDefClass);

protected:
	
	friend class StaticPhysClass;
};


#endif
