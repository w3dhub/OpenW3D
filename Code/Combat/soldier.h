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
 *                     $Archive:: /Commando/Code/Combat/soldier.h                             $* 
 *                                                                                             * 
 *                      $Author:: Byon_g                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 3/29/02 1:41p                                               $* 
 *                                                                                             * 
 *                    $Revision:: 192                                                         $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef	SOLDIER_H
#define	SOLDIER_H

#ifndef	ALWAYS_H
	#include "always.h"
#endif

#ifndef SMARTGAMEOBJ_H
	#include "smartgameobj.h"
#endif

#ifndef HUMANSTATE_H
	#include "humanstate.h"
#endif

#ifndef	WWSTRING_H
	#include "wwstring.h"
#endif

#ifndef	__DIALOGUE_H
	#include "dialogue.h"
#endif


struct TransitionCompletionDataStruct;
class	PrivateTimerClass;
class DynamicSpeechAnimClass;
class	TransitionEffectClass;
class PersistantSurfaceEmitterClass;
class	SoldierObserverClass;

//const int NO_FLAG = -1;


/*
**
*/
typedef enum {
	SOLDIER_INNATE_EVENT_ENEMY_SEEN 		=	1 << 0,
	SOLDIER_INNATE_EVENT_FOOTSTEP_HEARD	=	1 << 1,
	SOLDIER_INNATE_EVENT_GUNSHOT_HEARD	=	1 << 2,
	SOLDIER_INNATE_EVENT_BULLET_HEARD 	=	1 << 3,
	SOLDIER_INNATE_ACTIONS					=	1 << 4,
} SoldierInnateEvents;



/*
** SoldierGameObjDef - Defintion class for a SoldierGameObj
*/
class SoldierGameObjDef : public SmartGameObjDef
{
public:
	SoldierGameObjDef( void );

	virtual uint32								Get_Class_ID (void) const override;
	virtual PersistClass *					Create( void ) const override;
	virtual bool								Save( ChunkSaveClass &csave ) override;
	virtual bool								Load( ChunkLoadClass &cload ) override;
	virtual const PersistFactoryClass &	Get_Factory( void ) const override;

	DialogueClass *							Get_Dialog_List( void )			{ return DialogList; }

	DECLARE_EDITABLE( SoldierGameObjDef, SmartGameObjDef );

protected:
	float											TurnRate;
	float											JumpVelocity;
	float											SkeletonHeight;
	float											SkeletonWidth;
	bool											UseInnateBehavior;
	float											InnateAggressiveness;
	float											InnateTakeCoverProbability;
	bool											InnateIsStationary;
	DialogueClass								DialogList[DIALOG_MAX];
	StringClass									FirstPersonHands;
	int											HumanAnimOverrideDefID;
	int											HumanLoiterCollectionDefID;
	int											DeathSoundPresetID;

	friend	class								SoldierGameObj;
	friend	class								SoldierObserverClass;
};


/*
**
*/
class SoldierGameObj : public SmartGameObj {

public:
	SoldierGameObj();
	virtual	~SoldierGameObj();

	// Definitions
	virtual	void	Init( void ) override;
	void	Init( const SoldierGameObjDef & definition );
	void	Copy_Settings( const SoldierGameObjDef & definition );
	void	Re_Init( const SoldierGameObjDef & definition );
	const SoldierGameObjDef & Get_Definition( void ) const ;

	// Save / Load / Construction Factory
	virtual	bool	Save( ChunkSaveClass & csave ) override;
	virtual	bool	Load( ChunkLoadClass & cload ) override;
	virtual	void	On_Post_Load( void ) override;
	virtual	const	PersistFactoryClass & Get_Factory( void ) const override;

	// Physics
	HumanPhysClass	*	Peek_Human_Phys( void ) const;

	// Think
	virtual	void	Think( void ) override;
	virtual	void	Post_Think( void ) override;
	//virtual	void	Think_Pathfind( void );

	// Control
	virtual	void	Set_Control_Owner(int control_owner) override;
	virtual	void	Generate_Control( void ) override;
	virtual	void	Apply_Control( void ) override;

	// Damage
	virtual	void	Apply_Damage( const OffenseObjectClass & damager, float scale = 1.0f, int alternate_skin = -1 ) override;
   virtual	void	Apply_Damage_Extended( const OffenseObjectClass & offense, float scale = 1.0f,
			const	Vector3 & direction = Vector3( 0,0,0 ), const char * collision_box_name = NULL ) override;
	virtual	void	Completely_Damaged( const OffenseObjectClass & /* damager */ ) override {}
	virtual CollisionReactionType	Collision_Occurred( const CollisionEventClass & event ) override;
	virtual	Vector3	Get_Bullseye_Position( void ) override;

	// Turret
	virtual bool	Is_Turreted( void )					{ return false; }

	// Weapons
	virtual bool	Set_Targeting( const Vector3 & pos, bool do_tilt = true  ) override;
	float		Get_Weapon_Height( void );
	float		Get_Weapon_Length( void );
	virtual const Matrix3D &Get_Muzzle( int index = 0 ) override;

	bool		Detonate_C4( void )							{ return DetonateC4; }

   // Display
	virtual void Set_Weapon_Model( const char *model_name );
	virtual void Set_Weapon_Animation( const char *anim_name );
	virtual void	Start_Transition_Animation( const char * anim_name, TransitionCompletionDataStruct * completion_data );
	virtual	void	Set_Animation( const char *animation_name, bool looping = true, float frame_offset = 0.0f ) override;
	virtual	void	Set_Blended_Animation( const char *animation_name, bool looping = true, float frame_offset = 0.0f, bool play_backwards = false );

	void	Handle_Legs( void );

	// Ladders
	void	Exit_Ladder( void );
	void	Enter_Ladder( bool top );

	// Network support
   virtual	void	Import_State_Cs( BitStreamClass & packet ) override;
   virtual	void	Export_State_Cs( BitStreamClass & packet ) override;
	void				Interpret_Sc_Position_Data(const Vector3 & sc_position);
	void				Interpret_Sc_State_Data(HumanStateClass::HumanStateType state, 
							int sub_state, LPCSTR trans_name, const Vector3 & velocity, const Vector3 & sc_position);
	int				Tally_Vis_Visible_Soldiers(void);
	bool				Is_In_Elevator(void);

	virtual void	Export_Creation( BitStreamClass &packet ) override;
	virtual void	Import_Creation( BitStreamClass &packet ) override;
	virtual void	Export_Rare( BitStreamClass &packet ) override;
	virtual void	Import_Rare( BitStreamClass &packet ) override;
	virtual void	Export_Occasional( BitStreamClass &packet ) override;
	virtual void	Import_Occasional( BitStreamClass &packet ) override;
   virtual void	Export_Frequent( BitStreamClass & packet ) override;
   virtual void	Import_Frequent( BitStreamClass & packet ) override;


	bool				Is_Dead( void )			{ return Get_State() == HumanStateClass::DEATH; }
	bool				Is_Destroyed( void )		{ return Get_State() == HumanStateClass::DESTROY; }
	bool				Is_Upright( void )		{ return Get_State() == HumanStateClass::UPRIGHT; }
	bool				Is_Wounded( void )		{ return Get_State() == HumanStateClass::WOUNDED; }
	bool				In_Transition( void )	{ return Get_State() == HumanStateClass::TRANSITION; }
	bool				Is_Airborne( void )		{ return Get_State() == HumanStateClass::AIRBORNE; }
	bool				Is_Crouched( void )		{ return HumanState.Get_State_Flag( HumanStateClass::CROUCHED_FLAG ); }
	bool				Is_Sniping( void )		{ return HumanState.Get_State_Flag( HumanStateClass::SNIPING_FLAG ); }
	bool				Is_Slow( void )			{ return (Get_Sub_State() & HumanStateClass::SUB_STATE_SLOW) != 0; }
	bool				Is_On_Ladder( void )		{ return Get_State() == HumanStateClass::LADDER; }
	bool				Is_State_Locked( void )	{ return HumanState.Is_Locked(); }
	bool				Is_In_Vehicle( void )	{ return Get_State() == HumanStateClass::IN_VEHICLE; }

	void				Reset_Loiter_Delay( void )				{ HumanState.Reset_Loiter_Delay(); }
	void				Set_Loiters_Allowed( bool allowed )	{ HumanState.Set_Loiters_Allowed( allowed ); }

	virtual	void	Get_Information( StringClass & string ) override;
	//virtual	void	Get_Extended_Information( StringClass & description );
	virtual void	Get_Description(StringClass & description) override;

	void		Toggle_Fly_Mode( void );			// Fly mode (for debugging/screen shots)

	// Object Motion
	virtual float		Get_Max_Speed( void ) override;
	virtual void 		Set_Max_Speed( float speed );
	virtual float		Get_Turn_Rate( void ) override;

	// Vehicles
	void					Enter_Vehicle( VehicleGameObj * vehicle, const char * anim_name );
	void					Exit_Vehicle( void );
	void					Exit_Destroyed_Vehicle( int seat_num, const Vector3 & vehicle_pos );
	bool					Is_Permitted_To_Enter_Vehicle( void );
	VehicleGameObj	*	Get_Vehicle( void )			{ return Vehicle; }
	VehicleGameObj	*	Get_Profile_Vehicle( void );				// HACK
	bool					Use_Ladder_View( void );

	StringClass			Get_Animation_Name( void )		{ return AnimationName; }
	const char *		Get_State_Name( void )		{ return HumanState.Get_State_Name(); }
	HumanStateClass	* Get_Human_State( void )			{ return &HumanState; }

	void					Set_Model(const char *model_name);

	// Type identification
	virtual SoldierGameObj		*As_SoldierGameObj() override		{ return this; };

	// Velocity
	void              Get_Velocity(Vector3 & vel) override;
   void              Set_Velocity(Vector3 & vel);

   void              Give_All_Weapons(void);

	bool					Can_See(SoldierGameObj * p_soldier);

	// Rendering
	void					Adjust_Skeleton( float height, float width );

	/*
	// Capture The Flag
	void					Set_Ctf_Team_Flag(int team);
	int					Get_Ctf_Team_Flag(void) {return CtfTeamFlag;}
	*/

	// Head Looking
	void					Look_At( const Vector3 & pos, float time )	{ HeadLookTarget = pos; HeadLookDuration = time; HeadLookAngle = Vector3(0,0,0); }
	void					Update_Look_At( const Vector3 & pos )			{ HeadLookTarget = pos; }
	void					Cancel_Look_At( void )								{ HeadLookDuration = 0.001F; }
	void					Look_Random( float time );
	bool					Is_Looking( void )									{ return (HeadLookDuration > 0); }
	virtual Matrix3D	Get_Look_Transform(void) override;

	// Head and facing
	void					Lock_Facing( PhysicalGameObj * game_obj, bool turn_body );

	// Innate Disabling
	void					Innate_Enable( int bits = 0xFFFFFFFF )	{ InnateEnableBits |= bits; }
	void					Innate_Disable( int bits = 0xFFFFFFFF )	{ InnateEnableBits &= ~bits; }
	bool					Is_Innate_Enabled( int bits = 0xFFFFFFFF )	{ return (InnateEnableBits & bits) != 0; }

	//static void			Set_Precision(void);

	// Dialog support
	void					Say_Dialogue( int dialog_id );
	static float		Say_Dynamic_Dialogue( int text_id, SoldierGameObj *speaker = NULL, AudibleSoundClass **sound_obj_to_return = NULL );
	void					Stop_Current_Speech( void );
	RenderObjClass *	Find_Head_Model( void );
	void					Prepare_Speech_Framework( void );


	//
	//	"Ghost" support.  "Ghosts" are soldier's who have their collision turned off
	// so they can walk through other soldiers.
	//
	void					Enable_Ghost_Collision( bool onoff );
	bool					Is_Soldier_Blocked( const Vector3 &curr_pos );
	bool					Is_Safe_To_Disable_Ghost_Collision( const Vector3 &curr_pos );
	

	//
	// Speech support
	//
	DynamicSpeechAnimClass *Get_Facial_Anim (void) { return SpeechAnim; }
	void							Set_Emot_Icon (const char *model_name, float duration );

	// Innate AI access
	SoldierObserverClass	*	Get_Innate_Controller( void );
	SoldierAIState		Get_AI_State( void ) const					{ return AIState; }
	void					Set_AI_State( SoldierAIState state );

	// Innate observer access
	void							Set_Innate_Observer( SoldierObserverClass * observer )	{ InnateObserver = observer; }
	SoldierObserverClass *	Get_Innate_Observer( void )	{ return InnateObserver; }
	void							Clear_Innate_Observer( void )	{ InnateObserver = NULL; }

	const char * Get_First_Person_Hands_Model_Name( void );

	void					Perturb_Position(float max_perturb = 5);

	int					Get_Key_Ring( void ) const		{ return KeyRing; }
	void					Give_Key( int key_number )		{ KeyRing |= (1 << key_number); }
	void					Remove_Key( int key_number )	{ KeyRing &= ~(1 << key_number); }
	bool					Has_Key( int key_number )		{ return ((1 << key_number) & KeyRing) != 0; }

	virtual bool		Wants_Powerups( void )							{ return Is_Human_Controlled(); }
	virtual bool		Allow_Special_Damage_State_Lock( void )	{ return (Is_Human_Controlled() == false); }

	virtual	bool	Is_Visible( void ) override						{ return IsVisible; }
				void	Set_Is_Visible( bool visible )		{ IsVisible = visible; }	

	//
	//	Static debug support
	//
	static void			Enable_Ghost_Collision_Debug_Display (bool onoff)	{ DisplayDebugBoxForGhostCollision = onoff; }
	static bool			Is_Ghost_Collision_Debug_Display_Enabled (void)		{ return DisplayDebugBoxForGhostCollision; }

	virtual	bool		Is_Targetable( void ) const override;

	// Stealth
	virtual float		Get_Stealth_Fade_Distance(void) const override;

protected:
	RenderObjClass		*	WeaponRenderModel;
	RenderObjClass		*	BackWeaponRenderModel;
	RenderObjClass		*	BackFlagRenderModel;

	AnimControlClass	*	WeaponAnimControl;
	bool						DetonateC4;
	TransitionCompletionDataStruct *	TransitionCompletionData;
	StringClass				AnimationName;
	VehicleGameObj		*	Vehicle;
	HumanStateClass		HumanState;
	float						LegFacing;		// Direction the legs are facing
	bool						SyncLegs;		// Are we in the process of syncing the legs?
	bool						LastLegMode;
	int						KeyRing;
	bool						IsUsingGhostCollision;

	DialogueClass			DialogList[DIALOG_MAX];
	AudibleSoundClass *	CurrentSpeech;

	// Head Look
	float						HeadLookDuration;
	Vector3					HeadRotation;
	Vector3					HeadLookTarget;
	Vector3					HeadLookAngle;
	float						HeadLookAngleTimer;

	HumanStateClass::HumanStateType 	Get_State( void )			{ return HumanState.Get_State(); }
	int										Get_Sub_State( void )	{ return HumanState.Get_Sub_State(); }

	void						Update_Locked_Facing( void );
	void						Update_Back_Gun( void );
	void						Set_Back_Weapon_Model( const char *model_name );
	// Move below to public?
	void						Set_Back_Flag_Model( const char *model_name, const Vector3 & tint = Vector3(0, 0, 0) );

	int						Get_Ouch_Type( const	Vector3 & direction, const char * collision_box_name ) { return HumanState.Get_Ouch_Type( direction, collision_box_name ); }
	bool						Internal_Set_Targeting( const Vector3 & pos, bool do_tilt = true  );

	ArmorWarheadManager::SpecialDamageType			SpecialDamageMode;
	float						SpecialDamageTimer;
	GameObjReference			SpecialDamageDamager;
	TransitionEffectClass * SpecialDamageEffect;
	void						Set_Special_Damage_Mode( ArmorWarheadManager::SpecialDamageType mode, ArmedGameObj * damager = NULL );

	TransitionEffectClass * HealingEffect;

	void						Handle_Head_look( void );

	//int						CtfTeamFlag; // Capture The Flag - use NO_FLAG for no flag

	GameObjReference		FacingObject;
	bool						FacingAllowBodyTurn;

	int						InnateEnableBits;
	SoldierObserverClass *InnateObserver;
	SoldierAIState			AIState;	
	
	DynamicSpeechAnimClass *SpeechAnim;
	float						GenerateIdleFacialAnimTimer;
	RenderObjClass *		HeadModel;
	RenderObjClass *		EmotIconModel;
	float						EmotIconTimer;	

	bool						InFlyMode;
	bool						IsVisible;

	bool						LadderUpMask;
	bool						LadderDownMask;

	float						ReloadingTilt;	

	bool						WeaponChanged;

	PersistantSurfaceEmitterClass * WaterWake;

	// Soldiers maintain all RenderObjs they have created for later use and saving
	// This is used for weapon models
	DynamicVectorClass<RenderObjClass*>	RenderObjList;
	void						Add_RenderObj( RenderObjClass * obj );
	RenderObjClass *		Find_RenderObj( const char * name );
	void						Reset_RenderObjs( void );

	void						Update_Healing_Effect( void );


	//
	//	Static debug support
	//
	static bool				DisplayDebugBoxForGhostCollision;

	int						Check(void);

};

#endif	//	SOLDIER_H

