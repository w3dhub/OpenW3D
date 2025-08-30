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
 *                     $Archive:: /Commando/Code/Combat/smartgameobj.h                        $* 
 *                                                                                             * 
 *                      $Author:: Greg_h                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 11/27/01 11:45a                                             $* 
 *                                                                                             * 
 *                    $Revision:: 124                                                         $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                 * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef	SMARTGAMEOBJ_H
#define	SMARTGAMEOBJ_H

#ifndef	ALWAYS_H
	#include "always.h"
#endif

#ifndef ARMEDGAMEOBJ_H
	#include "armedgameobj.h"
#endif

#ifndef ACTION_H
	#include "action.h"
#endif

#ifndef CONTROL_H
	#include "control.h"
#endif

#ifndef COMBATSOUND_H
	#include "combatsound.h"
#endif

#ifndef PHYSCONTROL_H
	#include "physcontrol.h"
#endif

#ifndef __AUDIO_EVENTS_H
    #include "AudioEvents.h"
#endif

/*
**
*/
class VehicleGameObj;
class	SoldierGameObj;
class BitStreamClass;
class PlayerDataClass;
class StealthEffectClass;

/*
** SmartGameObjDef - Defintion class for a SmartGameObj
*/
class SmartGameObjDef : public ArmedGameObjDef
{
public:
	SmartGameObjDef( void );

	virtual bool								Save( ChunkSaveClass &csave ) override;
	virtual bool								Load( ChunkLoadClass &cload ) override;

	DECLARE_EDITABLE( SmartGameObjDef, ArmedGameObjDef );

protected:
	float											SightRange;
	float											SightArc;
	float											ListenerScale;
	bool											IsStealthUnit;

	friend	class								SmartGameObj;
};


/*
**
*/
class SmartGameObj : public ArmedGameObj {

public:
	//	Constructor and Destructor
	SmartGameObj( void );
	virtual	~SmartGameObj( void );

	// Definitions
	void	Init( const SmartGameObjDef & definition );
	void	Copy_Settings( const SmartGameObjDef & definition );
	void	Re_Init( const SmartGameObjDef & definition );
	const SmartGameObjDef & Get_Definition( void ) const ;

	// Save / Load
	virtual	bool	Save( ChunkSaveClass & csave ) override;
	virtual	bool	Load( ChunkLoadClass & cload ) override;
	virtual void On_Post_Load(void) override;

	// Commands
	void	Clear_Control( void )						{ Control.Clear_Control(); }
	void	Set_Boolean_Control( ControlClass::BooleanControl control, bool state = true )	{ Control.Set_Boolean( control, state ); }
	void	Set_Analog_Control( ControlClass::AnalogControl control, float value )				{ Control.Set_Analog( control, value ); }
   void	Import_Control_Cs( BitStreamClass & packet )     { Control.Import_Cs(packet); }
   void	Export_Control_Cs( BitStreamClass & packet )     { Control.Export_Cs(packet); }
   void	Import_Control_Sc( BitStreamClass & packet )     { Control.Import_Sc(packet); }
   void	Export_Control_Sc( BitStreamClass & packet )     { Control.Export_Sc(packet); }
	ControlClass & Get_Control()						{ return Control; }
   void	Control_Enable( bool enable )				{ ControlEnabled = enable; }
   bool	Is_Control_Enabled( void )					{ return ControlEnabled; }
	void	Reset_Controller( void );
   
	// Update this objects control by reading inputs or running AI/action code
	virtual	void	Generate_Control( void );

	// Client who controls this object
	enum {
		SERVER_CONTROL_OWNER		= -99999,
	};

   int Get_Control_Owner(void)						{ return ControlOwner; }
   virtual int Get_Weapon_Control_Owner(void)	{ return Get_Control_Owner(); }
	virtual void Set_Control_Owner(int control_owner)		{ ControlOwner = control_owner; }

	// Player Data
	PlayerDataClass * Get_Player_Data( void )						{ return PlayerData; }
	void Set_Player_Data( PlayerDataClass * player_data );

   bool Has_Player(void);
   bool Is_Human_Controlled(void);
	bool Is_Controlled_By_Me(void);

	// Act on this object's control state
	virtual	void	Apply_Control( void );

	//	Thinking
	virtual	void		Think() override;
	virtual	void		Post_Think() override;

	// Damage
	virtual	void		Apply_Damage( const OffenseObjectClass & damager, float scale = 1.0f, int alternate_skin = -1 ) override;

	// Object Motion
	virtual float		Get_Max_Speed( void )		{ return 10; }
	virtual float		Get_Turn_Rate( void )		{ return DEG_TO_RADF(360); }

   // Actions
	ActionClass			*Get_Action( void )						{ return &Action; }

	// Type identification
	virtual SmartGameObj			*As_SmartGameObj() override		{ return this; };

	// State import/export
   virtual	void	Import_Frequent( BitStreamClass & packet ) override;
   virtual	void	Export_Frequent( BitStreamClass & packet ) override;
   virtual	void	Import_State_Cs( BitStreamClass & packet ) override;
   virtual	void	Export_State_Cs( BitStreamClass & packet ) override;
	virtual	void	Export_Creation( BitStreamClass & packet ) override;
	virtual	void	Import_Creation( BitStreamClass & packet ) override;


   bool Is_Control_Data_Dirty(cPacket & packet);

	bool	Is_Obj_Visible( PhysicalGameObj *obj );

	void	Set_Enemy_Seen_Enabled( bool enabled )	{ IsEnemySeenEnabled = enabled; }
   bool	Is_Enemy_Seen_Enabled( void )				{ return IsEnemySeenEnabled; }
	virtual Matrix3D Get_Look_Transform(void) { return Get_Transform(); }

	virtual void Get_Velocity(Vector3 & vel) {vel.Set(0, 0, 0);}

	virtual	bool	Is_Visible( void )						{ return true; }

	virtual void	On_Logical_Heard (LogicalListenerClass *listener, LogicalSoundClass *sound_obj) override;

	virtual	void	Begin_Hibernation( void ) override;
	virtual	void	End_Hibernation( void ) override;

	virtual	void	Get_Information( StringClass & string ) override;

	// 
	// Stealth interface
	// Enable_Stealth - turn this object's cloaking device on or off
	// Is_Stealth_Enabled - is this object's cloaking device turned on?  (may not be cloaked yet though)
	// Is_Stealthed - is the object actually stealthed (takes some time to turn on and off...)
	// Get_Stealth_Fade_Distance - humans and vehicles fade in at different distances.
	//
	void				Enable_Stealth(bool onoff);
	void				Toggle_Stealth(void);
	bool				Is_Stealth_Enabled(void);
	bool				Is_Stealthed(void) const;
	virtual float	Get_Stealth_Fade_Distance(void) const			{ return 25.0f; }

	void				Grant_Stealth_Powerup(float seconds);
	float				Remaining_Stealth_Powerup_Time(void);
	StealthEffectClass * Peek_Stealth_Effect(void);

	static	float	Get_Global_Sight_Range_Scale( void )			{ return GlobalSightRangeScale; }
	static	void	Set_Global_Sight_Range_Scale( float scale )	{ GlobalSightRangeScale = scale; }

protected:

	ControlClass			Control;
	PhysControllerClass	Controller;				// controller for the physics object
	bool						ControlEnabled;

	void						Alloc_Stealth_Effect(void);

	bool						StealthEnabled;		// stealth enabled by script or initialization code
	float						StealthPowerupTimer;	// stealth power is in effect
	float						StealthFiringTimer;	// timer for de-cloaking during
	StealthEffectClass *	StealthEffect;			// possible stealth effect

private:

	ActionClass				Action;		 			// for actions

   int						ControlOwner;			// Client who controls this object
	PlayerDataClass *		PlayerData;

	bool						IsEnemySeenEnabled;
	float						MovingSoundTimer;

	LogicalListenerClass* Listener;

	void Register_Listener(void);

	static	float			GlobalSightRangeScale;
};


#endif	//	SMARTGAMEOBJ_H













		//MOBIUS_CONTROL_OWNER		= -1
