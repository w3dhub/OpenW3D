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
 *                     $Archive:: /Commando/Code/wwphys/timeddecophys.h                       $*
 *                                                                                             *
 *                       Author:: Byon Garrabrant                                              *
 *                                                                                             *
 *                     $Modtime:: 5/03/01 7:38p                                               $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef TIMEDDECOPHYS_H
#define TIMEDDECOPHYS_H

#include "decophys.h"

class TimedDecorationPhysDefClass;

/**
** TimedDecorationPhysClass
** DecorationPhysClass with a Lifetime
*/
class TimedDecorationPhysClass : public DecorationPhysClass
{
public:

	TimedDecorationPhysClass(void);
	
	void									Init(const TimedDecorationPhysDefClass & def);
	virtual bool						Needs_Timestep(void) override										{ return true; }
	virtual void						Timestep(float dt) override;

	virtual void						Set_Lifetime( float time );
	virtual float						Get_Lifetime( void );

	virtual TimedDecorationPhysClass *	As_TimedDecorationPhysClass(void) override			{ return this; }

	// save-load system
	virtual const PersistFactoryClass &	Get_Factory (void) const override;
	virtual bool								Save (ChunkSaveClass &csave) override;
	virtual bool								Load (ChunkLoadClass &cload) override;

private:

	float											Lifetime;

	// Not implemented...
	TimedDecorationPhysClass(const TimedDecorationPhysClass &);
	TimedDecorationPhysClass & operator = (const TimedDecorationPhysClass &);

};


/**
** TimedDecorationPhysDefClass
** Definition data structure for TimedDecorationPhysClass
*/
class TimedDecorationPhysDefClass : public DecorationPhysDefClass
{
public:
	
	TimedDecorationPhysDefClass(void);
	
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

	// accessors
	float											Get_Lifetime(void) { return Lifetime; }

	//	Editable interface requirements
	DECLARE_EDITABLE(TimedDecorationPhysDefClass,DecorationPhysDefClass);

protected:

	float											Lifetime;

	friend class TimedDecorationPhysClass;
};


#endif
