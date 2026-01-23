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
 *                     $Archive:: /Commando/Code/wwphys/movephys.h                            $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 12/05/01 6:28p                                              $*
 *                                                                                             *
 *                    $Revision:: 40                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef MOVEPHYS_H
#define MOVEPHYS_H

#include "always.h"
#include "dynamicphys.h"
#include "matrix3.h"
#include "dynamicshadowmanager.h"

class	PhysControllerClass;
class MoveablePhysDefClass;
class DynTexProjectClass;


/**
** MoveablePhysClass
** All objects that can be moved around in the world in some way will support this interface.
** This interface defines basic mass and gravity settings and gives access to the object's 
** velocities.
*/
class MoveablePhysClass : public DynamicPhysClass
{

public:

	MoveablePhysClass(void);
	virtual ~MoveablePhysClass(void);

	void Init(const MoveablePhysDefClass & definition );
	virtual MoveablePhysClass * As_MoveablePhysClass(void) override								{ return this; }
	const MoveablePhysDefClass * Get_MoveablePhysDef(void)								{ return (MoveablePhysDefClass*)Definition; }

	/*
	** DEBUGGING, re-initialize this object because our definition changed
	*/
	virtual void					Definition_Changed(void) override;

	/*
	** PhysClass interface.
	** Post_Timestep_Process - moveable objects update their shadows in post-timestep...
	*/
	virtual bool					Needs_Timestep(void) override											{ return true; }
	virtual void					Post_Timestep_Process(void) override;

	/*
	** Physical properites
	*/
	virtual void					Set_Mass(float mass)											{ Mass = mass; MassInv = 1.0f / mass; }
	float								Get_Mass(void)													{ return Mass; }
	float								Get_Mass_Inv(void)											{ return MassInv; }
	virtual void					Set_Gravity_Multiplier(float grav)						{ GravScale = grav; }
	float								Get_Gravity_Multiplier(void) const						{ return GravScale; }
	virtual void					Set_Elasticity(float e)										{ Elasticity = e; }
	float								Get_Elasticity(void) const									{ return Elasticity; }
	virtual void					Get_Inertia_Inv(Matrix3 * set_I_inv);

	/*
	** Teleport support
	*/
	virtual bool					Can_Teleport(const Matrix3D &/* new_tm */, bool /* check_dyn_only */ = false,NonRefPhysListClass * /* result_list */ = NULL)	{ return false; }
	virtual bool					Can_Teleport_And_Stand(const Matrix3D &new_tm, Matrix3D *out)	{ *out = new_tm; return false; }
	virtual bool					Find_Teleport_Location(const Vector3 &/* start */, float /* radius */, Vector3 */* out */)			{ return false; }
	virtual bool					Can_Move_To(const Matrix3D &/* new_tm */) { return false; }
	virtual bool					Cinematic_Move_To(const Matrix3D & new_tm);

	/*
	** Controller
	*/
	void								Set_Controller(PhysControllerClass * control)		{ Controller = control; }	
	PhysControllerClass *		Get_Controller(void)											{ return Controller; }


	/*
	** Save-Load system
	*/
	virtual bool					Save (ChunkSaveClass &csave) override;
	virtual bool					Load (ChunkLoadClass &cload) override;
	virtual void					On_Post_Load (void) override;
	
	/*
	** Access to the state of the object
	*/
	virtual void					Get_Velocity(Vector3 * set_vel) const					= 0;
	virtual void					Set_Velocity(const Vector3 & /* newvel */)					{ }

	/*
	** Shadow Casting. 
	** Get_Blob_Shadow_Bounding_Box - Should return a tight object-space aabox; used for shadow blobs
	*/
	virtual void					Get_Shadow_Blob_Box(AABoxClass * set_obj_space_box) override;
	virtual bool					Is_Casting_Shadow(void) override										{ return ShadowManager.Is_Casting_Shadow(); }

	/*
	** Rider support.  All moveable objects can be riders.
	*/
	virtual void					Link_To_Carrier(PhysClass * carrier,RenderObjClass * carrier_sub_obj = NULL) override;
	virtual PhysClass *			Peek_Carrier_Object(void); // TSS added 08-15-01
	virtual RenderObjClass *	Peek_Carrier_Sub_Object(void) override;

protected:

	float								Mass;
	float								MassInv;
	float								GravScale;
	float								Elasticity;
	
	PhysControllerClass *		Controller;
	PhysClass *						Carrier;
	RenderObjClass *				CarrierSubObject;
	DynamicShadowManagerClass	ShadowManager;

	// Not Implemented:
	MoveablePhysClass(const MoveablePhysClass &);
	MoveablePhysClass & operator = (const MoveablePhysClass &);
};


inline void MoveablePhysClass::Get_Inertia_Inv(Matrix3 * set_I_inv)					
{ 
	set_I_inv->Make_Identity();
	(*set_I_inv)[0][0] = MassInv;
	(*set_I_inv)[1][1] = MassInv;
	(*set_I_inv)[2][2] = MassInv;
}


/*
** MoveablePhysDefClass
** Initialization Structure/Factory/Editor Integration for a MoveablePhysClass
*/
class MoveablePhysDefClass : public DynamicPhysDefClass
{
public:
	
	MoveablePhysDefClass(void);
	
	// From PhysDefClass
	virtual const char *						Get_Type_Name(void) override			{ return "MoveablePhysDef"; }
	virtual bool								Is_Type(const char *) override;

	// From PersistClass
	virtual bool								Save(ChunkSaveClass &csave) override;
	virtual bool								Load(ChunkLoadClass &cload) override;

	// In-Game Editing (DEBUGGING/TESTING ONLY)
	float											Get_Mass(void)					{ return Mass; }
	float											Get_Grav_Scale(void)			{ return GravScale; }
	void											Set_Mass(float new_mass)	{ Mass = new_mass; }
	void											Set_Grav_Scale(float new_g){ GravScale = new_g; }

	//	Editable interface requirements
	DECLARE_EDITABLE(MoveablePhysDefClass,DynamicPhysDefClass);

protected:

	float				Mass;
	float				GravScale;
	float				Elasticity;

	enum { 
		CINEMATIC_COLLISION_NONE = 0, 
		CINEMATIC_COLLISION_STOP, 
		CINEMATIC_COLLISION_PUSH, 
		CINEMATIC_COLLISION_KILL 
	};
	
	int				CinematicCollisionMode;
	
	friend class MoveablePhysClass;
};



#endif