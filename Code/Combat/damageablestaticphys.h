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
 *                 Project Name : Combat                                                       *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Combat/damageablestaticphys.h                $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 8/17/01 8:27p                                               $*
 *                                                                                             *
 *                    $Revision:: 9                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#ifndef DAMAGEABLESTATICPHYS_H
#define DAMAGEABLESTATICPHYS_H

#include "always.h"
#include "staticanimphys.h"
#include "damage.h"


class DamageableStaticPhysDefClass;

/**
** DamageableStaticPhysClass
** This is a static physics object which tracks its health.  When the health drops to zero,
** it plays its animation and stops on the end frame.
*/
class	DamageableStaticPhysClass : public StaticAnimPhysClass 
{
public:
	//	Constructor and Destructor
	DamageableStaticPhysClass( void );
	virtual ~DamageableStaticPhysClass( void );

	virtual void			Timestep(float dt) override;

	// RTTI
	virtual DamageableStaticPhysClass *	As_DamageableStaticPhysClass (void) override	{ return this; }

	// Definitions
	void	Init(const DamageableStaticPhysDefClass & definition);
	const DamageableStaticPhysDefClass * Get_DamageableStaticPhysDef(void) const { WWASSERT( Definition ); return (DamageableStaticPhysDefClass *)Definition; }

	// Damage Management
   void	Apply_Damage_Static(	const OffenseObjectClass & offense );

	void	Reset_Health(void);

	// Save / Load
	virtual	bool	Save(ChunkSaveClass & csave) override;
	virtual	bool	Load(ChunkLoadClass & cload) override;
	virtual	const	PersistFactoryClass & Get_Factory(void) const override;

protected:

	void				Start_Loop(void);
	void				Play_Twitch(void);
	void				Play_Death_Transition(void);

	enum {
		STATE_LIVE_LOOP = 0,
		STATE_LIVE_TWITCH,
		STATE_DEATH_TRANSITION,
		STATE_DEAD_LOOP,
		STATE_DEAD_TWITCH,
	};
			
	int											CurState;
	DefenseObjectClass						DefenseObject;

	friend class DSAPONetworkObjectClass;
};


/*
** DamageableStaticPhysDefClass
** Damageable static objects support two animation loops and several sequences.  An object
** can have a pair of frame numbers that are looped between while it is alive (the "Live-Loop"),
** a pair that are looped while it is dead (the "Dead-Loop"), a "twitch" sequence that is played
** whenever it is shot in either the live or dead state, and a sequence that is played when the 
** object transitions between alive and dead.
*/
class DamageableStaticPhysDefClass : public StaticAnimPhysDefClass 
{
public:
	DamageableStaticPhysDefClass(void);
	
	virtual uint32								Get_Class_ID(void) const override;
	virtual const char *						Get_Type_Name(void) override				{ return "DamageableStaticPhysDef"; }
	virtual bool								Is_Type(const char *) override;
	virtual PersistClass *					Create(void) const override;
	virtual bool								Save(ChunkSaveClass & csave) override;
	virtual bool								Load(ChunkLoadClass & cload) override;
	virtual const PersistFactoryClass &	Get_Factory(void) const override;

	DECLARE_EDITABLE( DamageableStaticPhysDefClass, StaticAnimPhysDefClass );

protected:

	int											KilledExplosion;
	
	int											LiveLoopStart;
	int											LiveLoopEnd;
	int											LiveTwitchStart;
	int											LiveTwitchEnd;
	int											DeathTransitionStart;
	int											DeathTransitionEnd;
	int											DeadLoopStart;
	int											DeadLoopEnd;
	int											DeadTwitchStart;
	int											DeadTwitchEnd;
	bool											PlayTwitchesToCompletion;
	DefenseObjectDefClass					DefenseObjectDef;
	

	friend class								DamageableStaticPhysClass;
	friend class								DSAPONetworkObjectClass;
};


#endif	// DAMAGEABLESTATICPHYS_H

