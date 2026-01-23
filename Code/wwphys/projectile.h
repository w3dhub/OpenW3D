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
 *                     $Archive:: /Commando/Code/wwphys/projectile.h                          $*
 *                                                                                             *
 *                       Author:: Byon_g                                                       *
 *                                                                                             *
 *                     $Modtime:: 8/17/01 8:25p                                               $*
 *                                                                                             *
 *                    $Revision:: 26                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   ProjectileClass::Get_Velocity -- returns the velocity of this projectile                  *
 *   ProjectileClass::Get_Angular_Velocity -- returns the angular velocity                     *
 *   ProjectileClass::Set_Velocity -- Sets the velocity of this projectile                     *
 *   ProjectileClass::Set_Angular_Velocity -- This will set the tumble rate for the projectile *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef PROJECTILE_H
#define PROJECTILE_H

#include "always.h"
#include "movephys.h"
#include "ode.h"


/*
** Projectile State Struct
** This is the state vector for a Projectile.  It has three
** degrees of freedom in translation
*/
struct ProjectileStateStruct 
{
	Vector3	Position;
	Vector3	Velocity;
};


class ProjectileDefClass;

/*
** ProjectileClass
** 
** This class is used for things like grenades which do not need to have "size" and
** do not allow other objects to collide with them.  Projectiles are basically
** an optimization, they are represented only by a point to the collision system
** and nothing will ever try to collide with them.
*/
class ProjectileClass : public MoveablePhysClass
{
public:

	ProjectileClass(void);
	virtual ~ProjectileClass(void);
	virtual ProjectileClass *	As_ProjectileClass(void) override											{ return this; }

	void								Init(const ProjectileDefClass & def);

	virtual void					Timestep(float dt) override;
	virtual bool					Cast_Ray(PhysRayCollisionTestClass & raytest) override;
	virtual const AABoxClass & Get_Bounding_Box(void) const;
	virtual const Matrix3D &	Get_Transform(void) const override;
	virtual void					Set_Transform(const Matrix3D & m) override;

	virtual void	 				Get_Velocity(Vector3 * set_vel) const override;
	virtual void	 				Get_Angular_Velocity(Vector3 * set_avel) const;
	virtual void					Set_Velocity(const Vector3 & newvel) override;
	virtual void					Set_Angular_Velocity(const Vector3 & newavel);

	virtual void					Set_Lifetime( float time );
	virtual float					Get_Lifetime( void );

	virtual void					Set_Bounce_Count(int count);
	virtual int						Get_Bounce_Count(void);

	virtual void					Apply_Impulse(const Vector3 & /* imp */)								{ }
	virtual void					Apply_Impulse(const Vector3 & /* imp */,const Vector3 & /* wpos */)	{ }

	/*
	** Collides on Move
	*/
	virtual void					Set_Collides_On_Move( bool onoff )								{ CollidesOnMove = onoff; }

	/*
	** Control over the method used to generate the orientation of the projectile.  Projectiles
	** don't have true "physical" orientation; they provide a couple of methods for generating
	** an orientation: aligned with their path, fixed, or tumbling.
	*/
	void								Set_Orientation_Mode_Aligned(void)								{ OrientationMode = ORIENTATION_ALIGNED; }
	void								Set_Orientation_Mode_Fixed(void)									{ OrientationMode = ORIENTATION_FIXED; }
	void								Set_Orientation_Mode_Aligned_Fixed(void);
	void								Set_Orientation_Mode_Tumbling(void);
	void								Set_Orientation_Mode_Tumbling(const Vector3 & axis,float rate);

	/*
	** Save-Load system
	*/
	virtual const PersistFactoryClass &	Get_Factory (void) const override;
	virtual bool								Save (ChunkSaveClass &csave) override;
	virtual bool								Load (ChunkLoadClass &cload) override;

protected:

	/*
	** Internal functions
	*/
	void								Integrate(float dt);
	void								Update_Transform(float dt);

	// state of the projectile
	ProjectileStateStruct		State;

	// Collides on Move
	bool								CollidesOnMove;

	// Orientation of the projectile
	enum
	{
		ORIENTATION_ALIGNED = 0,
		ORIENTATION_FIXED,
		ORIENTATION_TUMBLING,
		ORIENTATION_ALIGNED_FIXED,
	};
	
	int								OrientationMode;
	Vector3							TumbleAxis;
	float								TumbleRate;
	float								Lifetime;
	int								BounceCount;

private:

	// not implemented
	ProjectileClass(const ProjectileClass & that);
	ProjectileClass & operator = (const ProjectileClass & that);

	friend class ProjectileDefClass;
};




/***********************************************************************************************
 * ProjectileClass::Get_Velocity -- returns the velocity of this projectile                    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/18/98    GTH : Created.                                                                 *
 *=============================================================================================*/
inline void ProjectileClass::Get_Velocity(Vector3 * set_vel) const
{
	*set_vel = State.Velocity; 
}


/***********************************************************************************************
 * ProjectileClass::Get_Angular_Velocity -- returns the angular velocity                       *
 *                                                                                             *
 * Projectiles dont really have angular velocity so this always returns 0,0,0                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/18/98    GTH : Created.                                                                 *
 *=============================================================================================*/
inline void ProjectileClass::Get_Angular_Velocity(Vector3 * set_avel) const
{
	set_avel->Set(0,0,0);
}


/***********************************************************************************************
 * ProjectileClass::Set_Velocity -- Sets the velocity of this projectile                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/18/98    GTH : Created.                                                                 *
 *=============================================================================================*/
inline void ProjectileClass::Set_Velocity(const Vector3 & newvel)
{
	State.Velocity = newvel;
	
	if (OrientationMode == ORIENTATION_ALIGNED_FIXED) {
		Matrix3D tm;
		tm.Obj_Look_At(State.Position,State.Position + State.Velocity,0.0f);
		Model->Set_Transform(tm);
	}
}


/***********************************************************************************************
 * ProjectileClass::Set_Angular_Velocity -- This will set the tumble rate for the projectile   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 * Note that the tumble rate is only used if you put the projectile into "tumble" mode         *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/18/98    GTH : Created.                                                                 *
 *=============================================================================================*/
inline void	ProjectileClass::Set_Angular_Velocity(const Vector3 & newavel) 
{ 
	TumbleAxis = newavel;
	TumbleRate = TumbleAxis.Length();
	TumbleAxis *= (1.0f / TumbleRate);
}



/**
** ProjectileDefClass
** Initialization/Game-Database support for Phys3Class
*/
class ProjectileDefClass : public MoveablePhysDefClass
{
public:
	
	ProjectileDefClass(void);
	
	// From Definition
	virtual uint32								Get_Class_ID (void) const override;
	virtual PersistClass *					Create(void) const override;

	// From PhysDefClass
	virtual const char *						Get_Type_Name(void) override			{ return "ProjectileDef"; }
	virtual bool								Is_Type(const char *) override;

	// From PersistClass
	virtual const PersistFactoryClass &	Get_Factory (void) const override;
	virtual bool								Save(ChunkSaveClass &csave) override;
	virtual bool								Load(ChunkLoadClass &cload) override;

		//	Editable interface requirements
	DECLARE_EDITABLE(ProjectileDefClass,MoveablePhysDefClass);

protected:

	bool											CollidesOnMove;
	int											OrientationMode;
	Vector3										TumbleAxis;
	float											TumbleRate;
	float											Lifetime;
	int											BounceCount;

	friend class ProjectileClass;

};

#endif