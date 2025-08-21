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
 *                     $Archive:: /Commando/Code/wwphys/lightphys.h                           $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 4/02/01 12:22p                                              $*
 *                                                                                             *
 *                    $Revision:: 16                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef LIGHTPHYS_H
#define LIGHTPHYS_H

#include "decophys.h"
#include "rendobj.h"


/**
** LightPhysClass
** Just a class that is used by the light culling system.  Its main purposes
** are to enforce that the user (editor) places only lights into the system,
** to add save and load functionality, and to add any special effects or processing
** like lens flares and shadows.
*/
class LightPhysClass : public DecorationPhysClass
{
public:
	
	LightPhysClass(bool auto_allocate_light = false);
	virtual LightPhysClass *				As_LightPhysClass(void) override		{ return this; }

	void											Set_Model(RenderObjClass * model) override;

	/*
	** Visibility ID's.  Static lights can store a vis SECTOR id so that they
	** have a pre-calculated pvs for occlusion culling.
	*/
	void											Set_Vis_Sector_ID(int new_id)		{ VisSectorID = new_id; }	
	int											Get_Vis_Sector_ID(void) const		{ return VisSectorID; }

	/*
	** Group ID's.  Static lights can be assigned a "Group ID".  This id can be used by
	** external code.  For example, the building code uses the Group ID to differentiate between
	** lights that should be 'on' when the power to the building is 'on' and lights that should
	** be 'on' when the power is 'off'.
	*/
	void											Set_Group_ID(uintptr_t new_id)			{ GroupID = new_id; }
	uintptr_t											Get_Group_ID(void) const			{ return GroupID; }

	/*
	** Visibility checking
	*/
	int											Is_Vis_Object_Visible(int vis_object_id);

	/*
	** Save-Load system
	*/
	virtual const PersistFactoryClass &	Get_Factory (void) const override;
	virtual bool								Save (ChunkSaveClass &csave) override;
	virtual bool								Load (ChunkLoadClass &cload) override;
	virtual void								On_Post_Load(void) override;

private:

	int											VisSectorID;			// static lights have a pvs.
	uintptr_t											GroupID;					// group id, used by external code

	// Not implemented...
	LightPhysClass(const LightPhysClass &);
	LightPhysClass & operator = (const LightPhysClass &);
};



#endif
