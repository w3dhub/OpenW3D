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
 *                     $Archive:: /Commando/Code/Combat/buildingaggregate.h                   $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Byon_g                                                      $*
 *                                                                                             *
 *                     $Modtime:: 11/21/01 2:47p                                              $*
 *                                                                                             *
 *                    $Revision:: 8                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BUILDINGAGGREGATE_H
#define BUILDINGAGGREGATE_H

#include "always.h"
#include "staticanimphys.h"
#include "buildingstate.h"


class BuildingAggregateDefClass;


/**
** BuildingAggregateClass (BAG)
** This class implements the behavior of a "building aggregate".  Building aggregates are animated 
** static objects which display segments of their animation depending on the state of the building.  
*/
class	BuildingAggregateClass : public StaticAnimPhysClass 
{
public:

	//	Constructor and Destructor
	BuildingAggregateClass( void );
	virtual ~BuildingAggregateClass( void );

	// Definitions
	void	Init( const BuildingAggregateDefClass & definition );
	const BuildingAggregateDefClass * Get_BuildingAggregateDef( void ) const { WWASSERT( Definition ); return (BuildingAggregateDefClass *)Definition; }

	// State changing
	int				Get_Current_State(void);
	void				Set_Current_State(int new_state,bool force_update = false);
	bool				Is_MCT(void);

	// Save / Load
	virtual	bool	Save( ChunkSaveClass & csave ) override;
	virtual	bool	Load( ChunkLoadClass & cload ) override;

	virtual	void	Save_State( ChunkSaveClass & csave ) override;
	virtual	void	Load_State( ChunkLoadClass & cload ) override;

	virtual	void	On_Post_Load( void ) override;
	virtual	const	PersistFactoryClass & Get_Factory( void ) const override;

protected:
	
	bool				Is_Power_On(int state);

	int				CurrentState;

};



/**
** BuildingAggregateDefClass (BAGDef!)
** The data contained in this class defines the behavior of a "building aggregate".  Building
** aggregates are animated static objects which display segments of their animation depending
** on the state of the building.  
** Obviously, this is a very special case object; designed specifically to provide a set of
** features needed by the building logic in Renegade.
*/
class BuildingAggregateDefClass : public StaticAnimPhysDefClass 
{
public:
	BuildingAggregateDefClass(void);
	
	virtual uint32								Get_Class_ID (void) const override;
	virtual const char *						Get_Type_Name(void) override				{ return "BuildingAggregateDef"; }
	virtual bool								Is_Type(const char *) override;
	virtual PersistClass *					Create( void ) const override;
	virtual bool								Save( ChunkSaveClass &csave ) override;
	virtual bool								Load( ChunkLoadClass &cload ) override;
	virtual const PersistFactoryClass &	Get_Factory( void ) const override;

	DECLARE_EDITABLE( BuildingAggregateDefClass, StaticAnimPhysDefClass );

protected:

	bool											Save_State_Animation_Data(ChunkSaveClass & csave,int state_index);
	bool											Load_State_Animation_Data(ChunkLoadClass & cload,int state_index);

	/*
	** Animation Logic Setting
	** ANIM_LOGIC_LINEAR - Frame0 is used for each state, if the damage state changes, the animation
	**		is played to the new state's Frame0.  If the power state changes, we pop to the same relative
	**    position in the alternate power state.
	** ANIM_LOGIC_LOOP - In this mode, each state has a pair of frames which it loops between.  Whenever
	**		the state changes, we pop to the same relative position in the new loop.  In addition, certain
	**		states can have animation disabled; this will cause the object to freeze on the appropriate frame
	**		right after the state change.  
	*/
	enum 
	{
		ANIM_LOGIC_LINEAR						= 0,
		ANIM_LOGIC_LOOP,
		ANIM_LOGIC_SEQUENCE,
	};

	int											AnimLogicMode;
	bool											IsMCT;
	
	/*
	** Animation controls for each state.  In LOOP mode, we use Frame0 and Frame1 to define a loop.  In
	** Linear mode, only Frame0 is used.
	*/
	int											Frame0[BuildingStateClass::STATE_COUNT];
	int											Frame1[BuildingStateClass::STATE_COUNT];
	bool											AnimationEnabled[BuildingStateClass::STATE_COUNT];

	friend class BuildingAggregateClass;
};



#endif //BUILDINGAGGREGATE_H

