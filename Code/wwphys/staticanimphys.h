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
 *                     $Archive:: /Commando/Code/wwphys/staticanimphys.h                      $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 12/05/01 6:18p                                              $*
 *                                                                                             *
 *                    $Revision:: 27                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef STATICANIMPHYS_H
#define STATICANIMPHYS_H

#include "always.h"
#include "staticphys.h"
#include "wwstring.h"
#include "projectormanager.h"
#include "animcollisionmanager.h"

class StaticAnimPhysDefClass;
class MeshClass;


/**
** StaticAnimPhysClass
** This class is a static object which needs to perform additional processing 
** each frame.  Here are some of the features planned for this class:
** - animation which causes collisions in the world (like doors, elevators and more)
** - animation which updates a texture projector for effects like spot-lights, animated shadows
** - It can cause collisions due to its model animating (e.g. doors, lifts)
** and it can manage a texture projector. 
** - Possibly support multiple models and switch between the models for damage
*/
class StaticAnimPhysClass : public StaticPhysClass
{
public:
	
	StaticAnimPhysClass(void);
	~StaticAnimPhysClass(void);
	virtual StaticAnimPhysClass *			As_StaticAnimPhysClass(void) override								{ return this; }

	void											Init(const StaticAnimPhysDefClass & def);
	virtual void								Set_Model(RenderObjClass * model) override;
	virtual void								Vis_Render(SpecialRenderInfoClass & rinfo) override;
	
	/*
	** Static properties
	*/
	StaticAnimPhysDefClass *				Get_StaticAnimPhysDef(void)								{ return (StaticAnimPhysDefClass *)Definition; }

	/*
	** Processing
	*/
	virtual bool								Needs_Timestep(void) override;
	virtual void								Timestep(float dt) override;

	/*
	** State Import/Export and Save/Load
	*/
	virtual bool								Has_Dynamic_State(void) override										{ return true; }
	virtual void								Save_State(ChunkSaveClass & csave) override;
	virtual void								Load_State(ChunkLoadClass & cload) override;

	/*
	** Riders
	*/
	virtual bool								Internal_Link_Rider(PhysClass * rider) override;
	virtual bool								Internal_Unlink_Rider(PhysClass * rider) override;

	/*
	** Animation and animated collision control
	*/
	AnimCollisionManagerClass &			Get_Animation_Manager(void)								{ return AnimManager; }

	/*
	** Shadow control.  These are static texture projectors which affect 
	** dynamic objects.  Examples are tree shadows and stained glass windows
	*/
	void											Set_Shadow(TexProjectClass * shadow);
	virtual bool								Is_Casting_Shadow(void) override										{ return (ShadowProjector != NULL); }

	/*
	** Save-Load Support
	*/
	virtual const PersistFactoryClass &	Get_Factory(void) const override;
	virtual bool								Save(ChunkSaveClass &csave) override;
	virtual bool								Load(ChunkLoadClass &cload) override;
	virtual void								On_Post_Load(void) override;

protected:
	
	void											Update_Cached_Model_Parameters(void);
	virtual void								Update_Sun_Status(void) override;

protected:

	/*
	** Attached Projector Management
	*/
	ProjectorManagerClass					ProjectorManager;

	/*
	** Animation, animated collision, and rider management
	*/
	AnimCollisionManagerClass				AnimManager;

	/*
	** Static shadow management
	*/
	TexProjectClass *							ShadowProjector;

private:

	StaticAnimPhysClass(const StaticPhysClass &);
	StaticAnimPhysClass & operator = (const StaticPhysClass &);

};


/**
** StaticAnimPhysDefClass
** Initialization data structure for StaticAnimPhysClass
*/
class StaticAnimPhysDefClass : public StaticPhysDefClass
{
public:
	
	StaticAnimPhysDefClass(void);
	
	// From DefinitionClass
	virtual uint32								Get_Class_ID (void) const override;
	virtual PersistClass *					Create(void) const override;

	// From PhysDefClass
	virtual const char *						Get_Type_Name(void) override			{ return "StaticAnimPhysDef"; }
	virtual bool								Is_Type(const char *) override;

	// From PersistClass
	virtual const PersistFactoryClass &	Get_Factory (void) const override;
	virtual bool								Save(ChunkSaveClass &csave) override;
	virtual bool								Load(ChunkLoadClass &cload) override;

	//	Editable interface requirements
	DECLARE_EDITABLE(StaticAnimPhysDefClass,StaticPhysDefClass);

	// Accessors
	bool											Shadow_Dynamic_Objs(void)			{ return ShadowDynamicObjs; }
	bool											Shadow_Is_Additive(void)			{ return ShadowIsAdditive; }
	bool											Shadow_Ignores_Z_Rotation(void)	{ return ShadowIgnoresZRotation; }
	float											Shadow_NearZ(void)					{ return ShadowNearZ; }
	float											Shadow_FarZ(void)						{ return ShadowFarZ; }
	float											Shadow_Intensity(void)				{ return ShadowIntensity; }

	bool											Does_Collide_In_Pathfind(void)	{ return DoesCollideInPathfind; }

protected:
	
	bool											IsCosmetic;

	// Animation and animated collision support
	AnimCollisionManagerDefClass			AnimManagerDef;

	// Animated projector support
	ProjectorManagerDefClass				ProjectorManagerDef;

	// Static shadow support 
	bool											ShadowDynamicObjs;
	bool											ShadowIsAdditive;
	bool											ShadowIgnoresZRotation;
	float											ShadowNearZ;
	float											ShadowFarZ;
	float											ShadowIntensity;

	// Pathfind support
	bool											DoesCollideInPathfind;
	
	friend class StaticAnimPhysClass;
};

#endif //STATICANIMPHYS_H