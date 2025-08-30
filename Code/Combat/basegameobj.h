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
 ***                            Confidential - Westwood Studios                              *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : Commando                                                     * 
 *                                                                                             * 
 *                     $Archive:: /Commando/Code/Combat/basegameobj.h                         $* 
 *                                                                                             * 
 *                      $Author:: Greg_h                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 10/15/01 7:48p                                              $* 
 *                                                                                             * 
 *                    $Revision:: 17                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef	BASEGAMEOBJ_H
#define	BASEGAMEOBJ_H

#ifndef	ALWAYS_H
	#include "always.h"
#endif

#ifndef PERSIST_H
	#include "persist.h"
#endif

#ifndef DEFINITION_H
	#include "definition.h"
#endif

#ifndef __NETWORKOBJECT_H
	#include "networkobject.h"
#endif

#include "netclassids.h"

/*
**
*/
class	SmartGameObj;
class	PhysicalGameObj;
class	ScriptableGameObj;
class VehicleGameObj;

/*
** BaseGameObjDef - Defintion class for a BaseGameObj
*/
class BaseGameObjDef : public DefinitionClass
{
public:
	virtual bool								Save( ChunkSaveClass &csave ) override;
	virtual bool								Load( ChunkLoadClass &cload ) override;
};

/*
**
*/
class BaseGameObj : public PersistClass, public NetworkObjectClass {

public:
	//	Constructor and Destructor
	BaseGameObj( void );
	virtual	~BaseGameObj( void );

	// Definitions
	virtual	void					Init( void )											= 0;
	void								Init( const BaseGameObjDef & definition );
	const BaseGameObjDef &		Get_Definition( void ) const ;

	// Save / Load
	virtual	bool					Save( ChunkSaveClass & csave ) override;
	virtual	bool					Load( ChunkLoadClass & cload ) override;

	//	Thinking
	virtual	void					Think()				{ IsPostThinkAllowed = true; }
	virtual	void					Post_Think()		{};

	// ID
	void	Set_ID( int id )				{ Set_Network_ID (id); }
	int	Get_ID( void )	const			{ return Get_Network_ID (); }

	// Hibernation
	virtual	bool	Is_Hibernating( void )	{ return false; }

	// Termination
	//virtual	void					Destroy(bool damaged = false)	{ DestroyType = damaged ? DESTROY_DAMAGED : DESTROY_CONTROLLED; }
	//bool								Is_Destroy()						{ return (DestroyType != DESTROY_NONE); }
	//bool								Is_Damage_Destroyed()			{ return (DestroyType == DESTROY_DAMAGED); }

	// Type identification
	virtual	PhysicalGameObj	*As_PhysicalGameObj( void )	{ return (PhysicalGameObj*)NULL; };
	virtual  VehicleGameObj		*As_VehicleGameObj( void )		{ return (VehicleGameObj *)NULL; }
	virtual	SmartGameObj		*As_SmartGameObj( void )		{ return (SmartGameObj*)NULL; };
	virtual	ScriptableGameObj	*As_ScriptableGameObj( void )	{ return (ScriptableGameObj*)NULL; };

	// Network support
	virtual uint32					Get_Network_Class_ID( void ) const override		{ return NETCLASSID_GAMEOBJ; }
	virtual void					Delete (void) override									{ delete this; }

	bool								Is_Post_Think_Allowed( void )				{ return IsPostThinkAllowed; }

	void								Enable_Cinematic_Freeze( bool enable )	{ EnableCinematicFreeze = enable; }
	bool								Is_Cinematic_Freeze_Enabled( void )		{ return EnableCinematicFreeze; }

private:

	// Constants
	/*enum
	{
		DESTROY_NONE			= 0,
		DESTROY_DAMAGED,
		DESTROY_CONTROLLED
	};*/

	// Member data
	const BaseGameObjDef *	Definition;	
	//int							DestroyType;
	//int							ID;

	// This is used to prevent postthinking before a think call
	bool							IsPostThinkAllowed;

	// This keeps certain object alive during cinematic freeze
	bool							EnableCinematicFreeze;
};

#endif	//	BASEGAMEOBJ_H

