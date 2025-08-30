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
 *                     $Archive:: /Commando/Code/wwphys/dynamicanimphys.h                     $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 11/01/01 2:23p                                              $*
 *                                                                                             *
 *                    $Revision:: 8                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef DYANMICANIMPHYS_H
#define DYNAMICANIMPHYS_H

#include "always.h"
#include "decophys.h"
#include "animcollisionmanager.h"
#include "dynamicshadowmanager.h"


class ChunkLoadClass;
class ChunkSaveClass;
class DynamicAnimPhysDefClass;


/**
** DynamicAnimPhysClass
** This class manages dynamic animations.  These are similar to static anims but they can
** be dynamically created and destroyed and their bounding boxes can be animated (since they
** are placed into the dynamic culling system.  However, since they are dynamic objects,
** they do not VIS or cull as accurately.  An example of a dynamic animation would be an
** animated plane that flys across the level while an example of a static animation would
** be an animating elevator.
*/
class DynamicAnimPhysClass : public DecorationPhysClass
{
public:

	DynamicAnimPhysClass(void);
	virtual ~DynamicAnimPhysClass(void);

	virtual DynamicAnimPhysClass *		As_DynamicAnimPhysClass(void) override								{ return this; }
	const DynamicAnimPhysDefClass *		Get_DynamicAnimPhysDef(void);
	
	void											Init(const DynamicAnimPhysDefClass & def);
	virtual void								Set_Model(RenderObjClass * model) override;

	virtual bool								Needs_Timestep(void) override											{ return true; }
	virtual void								Timestep(float dt) override;
	virtual void								Post_Timestep_Process(void) override;

	/*
	** State Import/Export and Save/Load
	*/
	virtual bool								Has_Dynamic_State(void)										{ return true; }

	/*
	** save-load system
	*/
	virtual const PersistFactoryClass &	Get_Factory (void) const override;
	virtual bool								Save (ChunkSaveClass &csave) override;
	virtual bool								Load (ChunkLoadClass &cload) override;
	virtual void								On_Post_Load(void) override;

	/*
	** Animation and animated collision control
	*/
	AnimCollisionManagerClass &			Get_Animation_Manager(void)								{ return AnimManager; }

protected:
	
	void											Update_Cached_Model_Parameters(void);
	void											Reset_Mappers(RenderObjClass * model);

	AnimCollisionManagerClass				AnimManager;
	DynamicShadowManagerClass				ShadowManager;
	
private:

	// Not implemented...
	DynamicAnimPhysClass(const DynamicAnimPhysClass &);
	DynamicAnimPhysClass & operator = (const DynamicAnimPhysClass &);

};


/**
** DynamicAnimPhysDefClass
** Definition data structure for DynamicAnimPhysClass
*/
class DynamicAnimPhysDefClass : public DecorationPhysDefClass
{
public:
	
	DynamicAnimPhysDefClass(void);
	
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
	DECLARE_EDITABLE(DynamicAnimPhysDefClass,DecorationPhysDefClass);

protected:
	
	// Animation and animated collision support
	AnimCollisionManagerDefClass			AnimManagerDef;

	bool											CastsShadows;
	float											ShadowNearZ;
	float											ShadowFarZ;

	friend class DynamicAnimPhysClass;
};



/*
** Inlines
*/
inline const DynamicAnimPhysDefClass * DynamicAnimPhysClass::Get_DynamicAnimPhysDef(void)
{
	return (DynamicAnimPhysDefClass *)Definition;
}



#endif //DYNAMICANIMPHYS_H

