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
 *                     $Archive:: /Commando/Code/wwphys/shakeablestaticphys.h                 $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 8/17/01 8:27p                                               $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#ifndef SHAKEABLESTATICPHYS_H
#define SHAKEABLESTATICPHYS_H

#include "always.h"
#include "staticanimphys.h"


class ShakeableStaticPhysDefClass;


/**
** ShakeableStaticPhysClass
** This is a static animated physics class which plays its animation as a "shaking" effect
** when it is collided with
*/
class ShakeableStaticPhysClass : public StaticAnimPhysClass
{
public:
	//	Constructor and Destructor
	ShakeableStaticPhysClass( void );
	virtual ~ShakeableStaticPhysClass( void );

	// Definitions
	void	Init(const ShakeableStaticPhysDefClass & definition);
	const ShakeableStaticPhysDefClass * Get_ShakeableStaticPhysDef(void) const { WWASSERT( Definition ); return (ShakeableStaticPhysDefClass *)Definition; }

	// Play the animation once from start to end, typically going to be called from collision handling code
	void	Play_Animation(void);

	// Save / Load
	virtual	bool	Save(ChunkSaveClass & csave) override;
	virtual	bool	Load(ChunkLoadClass & cload) override;
	virtual	const	PersistFactoryClass & Get_Factory(void) const override;

};


/*
** ShakeableStaticPhysClass
*/
class ShakeableStaticPhysDefClass : public StaticAnimPhysDefClass 
{
public:
	ShakeableStaticPhysDefClass(void);
	
	virtual uint32								Get_Class_ID(void) const override;
	virtual const char *						Get_Type_Name(void) override				{ return "ShakeableStaticPhysDef"; }
	virtual bool								Is_Type(const char *) override;
	virtual PersistClass *					Create(void) const override;
	virtual bool								Save(ChunkSaveClass & csave) override;
	virtual bool								Load(ChunkLoadClass & cload) override;
	virtual const PersistFactoryClass &	Get_Factory(void) const override;

	DECLARE_EDITABLE( ShakeableStaticPhysDefClass, StaticAnimPhysDefClass );

protected:

	friend class								ShakeableStaticPhysClass;
};





#endif //SHAKEABLESTATICPHYS_H

