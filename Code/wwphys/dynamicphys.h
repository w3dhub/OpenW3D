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
 *                     $Archive:: /Commando/Code/wwphys/dynamicphys.h                         $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 8/17/01 8:43p                                               $*
 *                                                                                             *
 *                    $Revision:: 9                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef DYNAMICPHYS_H
#define DYNAMICPHYS_H

#include "always.h"
#include "phys.h"

class DynamicPhysDefClass;

/**
** DynamicPhysClass
** This class adds some behavior that will be common to all dynamic physics objects.  It tracks
** the current vis-ID for the object and has a method to automatically update it which should 
** be called whenever the object moves.  This class is not a concrete derived class.
*/
class DynamicPhysClass : public PhysClass
{
public:
	
	DynamicPhysClass(void);
	~DynamicPhysClass(void);
	virtual DynamicPhysClass *	As_DynamicPhysClass(void) override									{ return this; }
	void Init(const DynamicPhysDefClass & definition);
		
	virtual void								Set_Model(RenderObjClass * model) override;
	
	/*
	** Call this whenever the object moves to update its visibility status
	*/
	void											Update_Visibility_Status(void);
	virtual int									Get_Vis_Object_ID(void) override;

	/*
	** Simulation and rendering toggles for all dynamic physics objects
	*/
	virtual bool								Is_Simulation_Disabled(void) override				{ return _DisableDynamicPhysSimulation; }
	virtual bool								Is_Rendering_Disabled(void) override				{ return _DisableDynamicPhysRendering; }
	static void									Disable_All_Simulation(bool onoff)		{ _DisableDynamicPhysSimulation = onoff; }
	static void									Disable_All_Rendering(bool onoff)		{ _DisableDynamicPhysRendering = onoff; }
	static bool									Is_All_Simulation_Disabled(void)			{ return _DisableDynamicPhysSimulation; }
	static bool									Is_All_Rendering_Disabled(void)			{ return _DisableDynamicPhysRendering; }

	/*
	** Save-Load System
	*/
	virtual bool								Save (ChunkSaveClass &csave) override;
	virtual bool								Load (ChunkLoadClass &cload) override;
	virtual void								On_Post_Load(void) override;

protected:

	void											Internal_Update_Visibility_Status(void);

	bool											DirtyVisObjectID;			// dirty flag for the vis object id
	int											VisNodeID;					// ID of the node this object is in for temporal coherence
	unsigned int								VisStatusLastUpdated;	// tickcount of last vis update
	
private:

	static bool									_DisableDynamicPhysSimulation;
	static bool									_DisableDynamicPhysRendering;

	/*
	** Not implemented...
	*/
	DynamicPhysClass(const DynamicPhysClass &);
	DynamicPhysClass & operator = (const DynamicPhysClass &);

};


/**
** DynamicPhysDefClass
** Definition data structure for DynamicPhysClass
*/
class DynamicPhysDefClass : public PhysDefClass
{
public:
	
	DynamicPhysDefClass(void);

	// From PersistClass
	virtual bool								Save(ChunkSaveClass &csave) override;
	virtual bool								Load(ChunkLoadClass &cload) override;

	// From PhysDefClass
	virtual const char *						Get_Type_Name(void) override			{ return "DynamicPhysDef"; }
	virtual bool								Is_Type(const char *) override;

	// Validation methods
	virtual bool								Is_Valid_Config (StringClass &message) override;

	//	Editable interface requirements
	DECLARE_EDITABLE(DynamicPhysDefClass,PhysDefClass);

protected:

	friend class DynamicPhysClass;
};




#endif //DYNAMICPHYS_H



