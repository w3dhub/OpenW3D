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
 *                     $Archive:: /Commando/Code/Combat/doors.h                         $* 
 *                                                                                             * 
 *                      $Author:: Patrick                                                     $* 
 *                                                                                             * 
 *                     $Modtime:: 1/17/02 10:37a                                              $* 
 *                                                                                             * 
 *                    $Revision:: 23                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef	DOORS_H
#define	DOORS_H

#ifndef	ALWAYS_H
	#include "always.h"
#endif

#ifndef	__ACCESSIBLE_PHYS_H
	#include "accessiblephys.h"
#endif

class SoldierGameObj;

/*
**	Door States
*/
enum
{
	STATE_CLOSED_DOOR				= 0,
	STATE_OPENED_DOOR,
	STATE_OPENING_DOOR,
	STATE_CLOSING_DOOR,
	STATE_ACCESS_DENIED,
	STATE_MAX
};


/*
** DoorPhysDefClass
*/
class DoorPhysDefClass : public AccessiblePhysDefClass 
{
public:
	DoorPhysDefClass(void);
	
	virtual uint32								Get_Class_ID (void) const override;
	virtual const char *						Get_Type_Name(void) override				{ return "DoorPhysDef"; }
	virtual bool								Is_Type(const char *) override;
	virtual PersistClass *					Create( void ) const override;
	virtual bool								Save( ChunkSaveClass &csave ) override;
	virtual bool								Load( ChunkLoadClass &cload ) override;
	virtual const PersistFactoryClass &	Get_Factory( void ) const override;

	DECLARE_EDITABLE( DoorPhysDefClass, AccessiblePhysDefClass );

	const OBBoxClass &						Get_Trigger_Zone1 (void) const { return TriggerZone1; }
	const OBBoxClass &						Get_Trigger_Zone2 (void) const { return TriggerZone2; }


	bool											Is_Vehicle_Door (void) const	{ return DoorOpensForVehicles; }

protected:

	OBBoxClass									TriggerZone1;
	OBBoxClass									TriggerZone2;

	float											CloseDelay;
	int											OpenSoundDefID;
	int											CloseSoundDefID;
	int											UnlockSoundDefID;
	int											AccessDeniedSoundDefID;
	bool											DoorOpensForVehicles;

	friend	class								DoorPhysClass;
};


/*
** DoorPhysClass
*/
class	DoorPhysClass : public AccessiblePhysClass
{
public:
	//	Constructor and Destructor
	DoorPhysClass( void );
	virtual ~DoorPhysClass( void );

	// RTTI
	virtual DoorPhysClass *	As_DoorPhysClass(void) override	{ return this; }

	// Definitions
	void	Init( const DoorPhysDefClass & definition );
	const DoorPhysDefClass * Get_DoorPhysDef( void ) const { WWASSERT( Definition ); return (DoorPhysDefClass *)Definition; }

	// State import/export
	static void		Set_Precision(void);

	// Save / Load
	virtual	bool	Save( ChunkSaveClass & csave ) override;
	virtual	bool	Load( ChunkLoadClass & cload ) override;
	virtual	const	PersistFactoryClass & Get_Factory( void ) const override;

	virtual	void	Save_State( ChunkSaveClass & csave ) override;
	virtual	void	Load_State( ChunkLoadClass & cload ) override;

	// Timestep
	virtual void	Timestep( float dt ) override;

	// State access
	bool				Is_Door_Open( void ) const;
	void				Request_Door_Open( void )		{ OpenRequestPending = true; }
	void				Lock_Door_Open( bool onoff );
	bool				Is_State_Locked( void )			{ return LockState; }
	bool				Can_Unlock_Me( SoldierGameObj * soldier ) const;

protected:
	
	// State determination
	virtual void	Update_State( float dt );
	virtual int		Can_Open_Door( void );
	virtual int		Check_Door_Trigger( const OBBoxClass &trigger_zone );
	virtual bool	Set_State( int new_state );
	
	float		Timer;
	float		CheckTimer;
	int		State;
	bool		OpenRequestPending;
	bool		LockState;

	// Friends
	friend class DoorNetworkObjectClass;
};

#endif	// DOORS_H

