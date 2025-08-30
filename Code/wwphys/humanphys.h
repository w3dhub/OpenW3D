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
 *                     $Archive:: /Commando/Code/wwphys/humanphys.h                           $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 8/17/01 8:42p                                               $*
 *                                                                                             *
 *                    $Revision:: 15                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif


#ifndef HUMANPHYS_H
#define HUMANPHYS_H

#include "vector3.h"
#include "phys3.h"

class HumanPhysDefClass;

/**
** HumanPhysClass
** This class performs additional human physics logic.  It basically adds
** user control to the normal Phys3Class behavior.
*/

class HumanPhysClass : public Phys3Class
{
public:

	HumanPhysClass(void);
	virtual ~HumanPhysClass(void);
	virtual HumanPhysClass *	As_HumanPhysClass(void) override { return this; }

	void								Init(const HumanPhysDefClass & def);

	virtual void					Timestep(float dt) override;
	virtual void					Render(RenderInfoClass & rinfo) override;

	/*
	** Simulation and rendering toggles for all static physics objects
	*/
	virtual bool					Is_Simulation_Disabled(void) override				{ return _DisableHumanSimulation || Phys3Class::Is_Simulation_Disabled(); }
	virtual bool					Is_Rendering_Disabled(void) override				{ return _DisableHumanRendering || Phys3Class::Is_Simulation_Disabled(); }
	static void						Disable_All_Simulation(bool onoff)		{ _DisableHumanSimulation = onoff; }
	static void						Disable_All_Rendering(bool onoff)		{ _DisableHumanRendering = onoff; }
	static bool						Is_All_Simulation_Disabled(void)			{ return _DisableHumanSimulation; }
	static bool						Is_All_Rendering_Disabled(void)			{ return _DisableHumanRendering; }

	/*
	** Save-Load system
	*/
	virtual const PersistFactoryClass &	Get_Factory (void) const override;
	virtual bool								Save (ChunkSaveClass &csave) override;
	virtual bool								Load (ChunkLoadClass &cload) override;

	bool								Has_Just_Jumped (void) const { return JustJumped; }
	void								Jump_To_Point (const Vector3 &point);
	
protected:

	virtual void					Check_Ground(const AABoxClass & box,GroundStateStruct * gs,float check_dist) override;
	virtual bool					Ballistic_Move(float dt) override;
	virtual bool					Normal_Move(const GroundStateStruct & gs,float dt) override;
	virtual bool					Slide_Move(const GroundStateStruct & gs,float dt) override;
	void								Compute_Desired_Move_Vector(const GroundStateStruct & gs,float dt,Vector3 * set_move);

	bool								JustJumped;			// character jumped and has not started coming down yet, dont snap him!
	bool								IsAIControlledJump;

private:
	
	static bool						_DisableHumanSimulation;
	static bool						_DisableHumanRendering;

	// not implemented
	HumanPhysClass(const HumanPhysClass &);
	HumanPhysClass & operator = (const HumanPhysClass &);

};


/**
** HumanPhysDefClass - Initialization Structure/Factory/Editor Integration for a HumanPhysClass
*/
class HumanPhysDefClass : public Phys3DefClass
{
public:
	
	HumanPhysDefClass(void);
	
	// From DefinitionClass
	virtual uint32								Get_Class_ID (void) const override;
	virtual PersistClass *					Create(void) const override;

	// From PhysDefClass
	virtual const char *						Get_Type_Name(void) override			{ return "HumanPhysDef"; }
	virtual bool								Is_Type(const char *) override;

	// From PersistClass
	virtual const PersistFactoryClass &	Get_Factory (void) const override;
	virtual bool								Save(ChunkSaveClass &csave) override;
	virtual bool								Load(ChunkLoadClass &cload) override;

	//	Editable interface requirements
	DECLARE_EDITABLE(HumanPhysDefClass,Phys3DefClass);

protected:
	
	// No variables for now...
};

#endif

