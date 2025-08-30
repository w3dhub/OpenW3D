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
 *                     $Archive:: /Commando/Code/wwphys/motorcycle.h                          $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 8/17/01 8:41p                                               $*
 *                                                                                             *
 *                    $Revision:: 13                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef MOTORCYCLE_H
#define MOTORCYCLE_H

#ifndef ALWAYS_H
#include "always.h"
#endif

#ifndef WHEELVEHICLE_H
#include "wheelvehicle.h"
#endif

class MotorcycleDefClass;

/**
** MotorcycleClass
** This is a derivation of WheeledVehicleClass which adds self-balancing and
** leaning into turns.
*/
class MotorcycleClass : public WheeledVehicleClass
{
public:

	MotorcycleClass(void);
	virtual ~MotorcycleClass(void);
	virtual MotorcycleClass *	As_MotorcycleClass(void) override	{ return this; }

	void								Init(const MotorcycleDefClass & def);

	// State variables
	float								Get_Lean_Value(void)			{ return LeanValue; }
	
	// Save-Load system
	virtual const PersistFactoryClass &	Get_Factory (void) const override;
	virtual bool								Save (ChunkSaveClass &csave) override;
	virtual bool								Load (ChunkLoadClass &cload) override;

protected:

	float								LeanK0;
	float								LeanK1;
	float								LeanValue;	// how much the character should be leaning to balance the bike

	virtual void					Compute_Force_And_Torque(Vector3 * force,Vector3 * torque) override;
	virtual void					Compute_Inertia(void) override;

private:

	MotorcycleClass(const MotorcycleClass &);
	MotorcycleClass & operator = (const MotorcycleClass &);

};



/*
** MotorcycleDefClass - Initialization Structure/Factory/Editor Integration for a MotorcycleClass
*/
class MotorcycleDefClass : public WheeledVehicleDefClass
{
public:
	
	MotorcycleDefClass (void);
	
	// From DefinitionClass
	virtual uint32								Get_Class_ID (void) const override;
	virtual PersistClass *					Create(void) const override;

	// From PhysDefClass
	virtual const char *						Get_Type_Name(void) override			{ return "MotorcycleDef"; }
	virtual bool								Is_Type(const char *) override;

	// From PersistClass
	virtual const PersistFactoryClass &	Get_Factory (void) const override;
	virtual bool								Save(ChunkSaveClass &csave) override;
	virtual bool								Load(ChunkLoadClass &cload) override;

	//	Editable interface requirements
	DECLARE_EDITABLE(MotorcycleDefClass,WheeledVehicleDefClass);

protected:

	float				LeanK0;
	float				LeanK1;

	friend class MotorcycleClass;
};



#endif