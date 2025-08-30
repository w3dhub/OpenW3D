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
 *                     $Archive:: /Commando/Code/Combat/beacongameobj.h                       $* 
 *                                                                                             * 
 *                      $Author:: Byon_g                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 1/04/02 2:58p                                               $* 
 *                                                                                             * 
 *                    $Revision:: 16                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef	__BEACONGAMEOBJ_H
#define	__BEACONGAMEOBJ_H

#include "always.h"
#include "simplegameobj.h"
#include "timemgr.h"


////////////////////////////////////////////////////////////////
//	Forward delcarations
////////////////////////////////////////////////////////////////
class BaseControllerClass;
class WeaponDefinitionClass;
class AudibleSoundClass;


////////////////////////////////////////////////////////////////
//
//	BeaconGameObjDef
//
////////////////////////////////////////////////////////////////
class BeaconGameObjDef : public SimpleGameObjDef
{
public:

	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	BeaconGameObjDef (void);
	~BeaconGameObjDef (void);

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////

	virtual uint32								Get_Class_ID (void) const override;
	virtual PersistClass *					Create (void) const override;
	virtual bool								Save (ChunkSaveClass &csave) override;
	virtual bool								Load (ChunkLoadClass &cload) override;
	virtual const PersistFactoryClass &	Get_Factory (void) const override;

	bool Is_Nuke(void) const {return (IsNuke != 0);}

	DECLARE_EDITABLE (BeaconGameObjDef, SimpleGameObjDef);

protected:

	////////////////////////////////////////////////////////////////
	//	Protected methods
	////////////////////////////////////////////////////////////////
	void			Load_Variables (ChunkLoadClass &cload);

	////////////////////////////////////////////////////////////////
	//	Protected friends
	////////////////////////////////////////////////////////////////
	friend class BeaconGameObj;

	////////////////////////////////////////////////////////////////
	//	Protected member data
	////////////////////////////////////////////////////////////////
	float			BroadcastToAllTime;
	
	float			ArmTime;
	float			DisarmTime;
	float			PreDetonateCinematicDelay;
	float			DetonateTime;
	float			PostDetonateTime;

	int			ArmedSoundDefID;
	
	int			DisarmingTextID;
	int			DisarmedTextID;
	int			ArmingTextID;

	int			ArmingInterruptedTextID;
	int			DisarmingInterruptedTextID;

	int			PreDetonateCinematicDefID;
	int			PostDetonateCinematicDefID;

	int			ExplosionDefID;
	int			IsNuke;

	StringClass	ArmingAnimationName;
};


////////////////////////////////////////////////////////////////
//
//	BeaconGameObj
//
////////////////////////////////////////////////////////////////
class BeaconGameObj : public SimpleGameObj
{
public:

	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	BeaconGameObj (void);
	virtual ~BeaconGameObj (void);

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////

	//
	// Definition support
	//
	virtual	void					Init( void ) override;
	void								Init (const BeaconGameObjDef & definition);
	const BeaconGameObjDef &	Get_Definition (void) const;

	//
	// RTTI
	//
	virtual	BeaconGameObj * As_BeaconGameObj (void)	override { return this; }

	//
	// Save / Load / Construction Factory
	//
	bool				Save (ChunkSaveClass & csave) override;
	bool				Load (ChunkLoadClass & cload) override;
	const				PersistFactoryClass & Get_Factory (void) const override;

	//
	//	GameObj methods
	//
	void				Think (void) override;
	void				Get_Information (StringClass &string) override;

	//
	//	Beacon initialization
	//
	void				Init_Beacon (const WeaponDefinitionClass *definiton, SoldierGameObj *owner, const Vector3 &position);

	//
	//	Beacon stuff
	//
	bool				Can_Place_Here (const Vector3 &position);
	void				On_Arming_Interrupted (void);
	void				Begin_Arming (void);
	void				On_Poked (ScriptableGameObj *poker);

	virtual	void	Completely_Damaged( const OffenseObjectClass & damager ) override;

	virtual void	Export_Rare( BitStreamClass &packet ) override;
	virtual void	Import_Rare( BitStreamClass &packet ) override;

	SoldierGameObj *			Get_Owner (void);

private:

	////////////////////////////////////////////////////////////////
	//	Private methods
	////////////////////////////////////////////////////////////////
	void							Load_Variables (ChunkLoadClass &cload);
	void							Stop_Armed_Sound (void);
	void							Stop_Current_Message_Sound (void);
	void							Display_Message (int text_id);
	void							Start_Owner_Animation (void);
	void							Stop_Owner_Animation (void);
	bool							Was_Owner_Interrupted (void);
	void							Start_Cinematic (int id);
	bool							Is_In_Enemy_Base( void );
	void							Create_Explosion (void);

	//
	//	State support
	//
	void							Set_State (int state);
	void							Update_State (void);
	BaseControllerClass *	Get_Enemy_Base (void);


	////////////////////////////////////////////////////////////////
	//	Private constants
	////////////////////////////////////////////////////////////////
	enum
	{
		STATE_NULL			= 0,
		STATE_ARMING,
		STATE_ARMED,
		STATE_DISARMED,
		STATE_DETONATING
	};

	////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////
	GameObjReference					Owner;
	int									State;
	float									StateTimer;
	float									PreDetonateTimer;
	float									DetonateTimer;
	float									WarningTimer;
	AudibleSoundClass *				ArmedSound;
	AudibleSoundClass *				MessageSound;
	bool									IsArmed;
	const WeaponDefinitionClass *	WeaponDefinition;

	GameObjReference					CinematicObject;

	void *			OwnerBackup;
	void				Restore_Owner( void );

};

#endif	// __BEACONGAMEOBJ_H


