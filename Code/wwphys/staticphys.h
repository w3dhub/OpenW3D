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
 *                     $Archive:: /Commando/Code/wwphys/staticphys.h                          $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 2/18/02 2:13p                                               $*
 *                                                                                             *
 *                    $Revision:: 34                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef STATICPHYS_H
#define STATICPHYS_H

#include "phys.h"
#include "pscene.h"
#include "wwdebug.h"
#include "physcoltest.h"
#include "physinttest.h"
#include "vector.h"

class StaticPhysDefClass;

/**
** StaticPhysClass
** This class implements an object meant to be placed in the static object
** culling system.  Static objects are used to make up the bulk of the geometry for the
** static environment.  
** 
*/
class StaticPhysClass : public PhysClass
{
public:
	
	StaticPhysClass(void);
	~StaticPhysClass(void);
	virtual StaticPhysClass *	As_StaticPhysClass(void) override									{ return this; }
	StaticPhysDefClass *			Get_StaticPhysDef(void)										{ return (StaticPhysDefClass *)Definition; }
	
	void								Init(const StaticPhysDefClass & def);
	virtual bool					Needs_Timestep(void) override											{ return false; }
	virtual void					Timestep(float /* dt */) override											{ };
	virtual void					Set_Model(RenderObjClass * model) override;
	virtual void					Render_Vis_Meshes(RenderInfoClass & rinfo) override;
		
	/*
	** Collision detection - all collideable objects provide the following collision detection
	** functions so that other objects do not pass through them.  These functions should test
	** the given primitive against this object's geometric representation.
	*/
	virtual bool					Cast_Ray(PhysRayCollisionTestClass & raytest) override;
	virtual bool					Cast_AABox(PhysAABoxCollisionTestClass & boxtest) override;
	virtual bool					Cast_OBBox(PhysOBBoxCollisionTestClass & boxtest) override;

	virtual bool					Intersection_Test(PhysAABoxIntersectionTestClass & test) override;
	virtual bool					Intersection_Test(PhysOBBoxIntersectionTestClass & test) override;
	virtual bool					Intersection_Test(PhysMeshIntersectionTestClass & test) override;
	bool								Intersects(const OBBoxClass & obbox);

	virtual const AABoxClass & Get_Bounding_Box(void) const;
	virtual const Matrix3D &	Get_Transform(void) const override;
	virtual void					Set_Transform(const Matrix3D & m) override;

	/*
	** Classify this object for the visibility culling system.  If the rendered
	** representation of this object is translucent or if the object has multiple render 
	** states (e.g. an object that is destructible or can have its model be replaced)
	** then the object cannot be treated as an occluder.
	*/
	int								Is_Occluder(void);

	/*
	** Classify whether the model for this object is pre-lit.  In this case, we don't 
	** apply static lights or static projectors to it since their effect should be cooked
	** into the pre-processed lighting
	*/
	bool								Is_Model_Pre_Lit(void);
	bool								Is_Model_User_Lit(void);

	/*
	** Vis Data access.  In addition to having a vis object id, static physics objects
	** can define vis-sectors.
	*/
	void								Set_Vis_Sector_ID(int new_id);	
	int								Get_Vis_Sector_ID(void) const { return VisSectorID; }
	bool								Is_Vis_Sector(RenderObjClass * model = NULL) const;

	/*
	** Simulation and rendering toggles for all static physics objects
	*/
	virtual bool					Is_Simulation_Disabled(void) override				{ return _DisableStaticPhysSimulation; }
	virtual bool					Is_Rendering_Disabled(void) override				{ return _DisableStaticPhysRendering; }
	static void						Disable_All_Simulation(bool onoff)		{ _DisableStaticPhysSimulation = onoff; }
	static void						Disable_All_Rendering(bool onoff)		{ _DisableStaticPhysRendering = onoff; }
	static bool						Is_All_Simulation_Disabled(void)			{ return _DisableStaticPhysSimulation; }
	static bool						Is_All_Rendering_Disabled(void)			{ return _DisableStaticPhysRendering; }

	/*
	** Save-Load of state.  This interface is used to restore the state of any static
	** objects in the level.  Static objects are not re-created when a save-game is created
	** instead, their state is restored.  
	*/
	virtual bool								Has_Dynamic_State(void)							{ return false; }
	virtual void								Save_State(ChunkSaveClass & /* csave */)			{ }
	virtual void								Load_State(ChunkLoadClass & /* cload */)			{ }

	
	/*
	** Save-Load System
	*/
	virtual const PersistFactoryClass &	Get_Factory (void) const override;
	virtual bool								Save (ChunkSaveClass &csave) override;
	virtual bool								Load (ChunkLoadClass &cload) override;
	virtual void								On_Post_Load(void) override;

float	Compute_Vis_Mesh_Ram(RenderObjClass * model = NULL);

protected:

	void								Update_Cached_Model_Parameters(void);
	virtual void					Update_Sun_Status(void) override;

protected:

	int								VisSectorID;		// set if this static object contains a vis sector mesh (-1 if not)

private:

	static bool						_DisableStaticPhysSimulation;
	static bool						_DisableStaticPhysRendering;

	StaticPhysClass(const StaticPhysClass &);
	StaticPhysClass & operator = (const StaticPhysClass &);

};



/**
** StaticPhysDefClass
** Definition data structure for StaticPhysClass
*/
class StaticPhysDefClass : public PhysDefClass
{
public:
	
	StaticPhysDefClass(void);
	
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
	DECLARE_EDITABLE(StaticPhysDefClass,PhysDefClass);

protected:

	bool											IsNonOccluder;

	friend class StaticPhysClass;
};


/*
**
** Inline Functions
**
*/

inline bool StaticPhysClass::Cast_Ray(PhysRayCollisionTestClass & raytest)
{
	WWASSERT(Model);
	if (Model->Cast_Ray(raytest)) {
		raytest.CollidedPhysObj = this;
		return true;
	} else {
		return false;
	}
}

inline bool StaticPhysClass::Cast_AABox(PhysAABoxCollisionTestClass & boxtest)
{
	WWASSERT(Model);
	if (Model->Cast_AABox(boxtest)) {
		boxtest.CollidedPhysObj = this;
		return true;
	} else {
		return false;
	}
}

inline bool StaticPhysClass::Cast_OBBox(PhysOBBoxCollisionTestClass & boxtest)
{
	WWASSERT(Model);
	if (Model->Cast_OBBox(boxtest)) {
		boxtest.CollidedPhysObj = this;
		return true;
	} else { 
		return false;
	}
}

inline bool StaticPhysClass::Intersection_Test(PhysAABoxIntersectionTestClass & test)
{
	if (Model->Intersect_AABox(test)) {
		test.Add_Intersected_Object(this);
		return true;
	}
	return false;
}

inline bool StaticPhysClass::Intersection_Test(PhysOBBoxIntersectionTestClass & test)
{
	if (Model->Intersect_OBBox(test)) {
		test.Add_Intersected_Object(this);
		return true;
	}
	return false;
}

inline bool StaticPhysClass::Intersection_Test(PhysMeshIntersectionTestClass & /* test */)
{
	WWASSERT(0); // TODO: Mesh-Mesh intersection
	return false;
}

inline bool StaticPhysClass::Intersects(const OBBoxClass & obbox)
{
	CastResultStruct result;
	PhysOBBoxCollisionTestClass boxtest(	obbox,
														Vector3(0,0,0),
														&result,
														0x0F,
														1);
	Cast_OBBox(boxtest);
	return result.StartBad;
}



#endif




