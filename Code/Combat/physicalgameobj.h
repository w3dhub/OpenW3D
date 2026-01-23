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
 *                     $Archive:: /Commando/Code/Combat/physicalgameobj.h                     $* 
 *                                                                                             * 
 *                      $Author:: Greg_h                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 1/08/02 6:39p                                               $* 
 *                                                                                             * 
 *                    $Revision:: 126                                                         $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef	PHYSICALGAMEOBJ_H
#define	PHYSICALGAMEOBJ_H

#ifndef	ALWAYS_H
	#include "always.h"
#endif

#ifndef	DAMAGEABLEGAMEOBJ_H
	#include "damageablegameobj.h"
#endif

#ifndef	COMBATPHYSOBSERVER_H
	#include "combatphysobserver.h"
#endif

#ifndef WWPACKET_H
	#include "wwpacket.h"
#endif

#ifndef PHYS_H
	#include "phys.h"
#endif

/*
**
*/
class		SoldierGameObj;
class		VehicleGameObj;
class		PowerUpGameObj;
class		ArmedGameObj;
class    C4GameObj;
class    BeaconGameObj;
class		AnimControlClass;
class		CinematicGameObj;
class		SimpleGameObj;
class		ActiveConversationClass;


/*
** PhysicalGameObjDef - Defintion class for a PhysicalGameObj
*/
class PhysicalGameObjDef : public DamageableGameObjDef
{
public:
	PhysicalGameObjDef( void );

	virtual bool								Save( ChunkSaveClass &csave ) override;
	virtual bool								Load( ChunkLoadClass &cload ) override;
	virtual bool								Is_Valid_Config (StringClass &message) override;

	int		Get_Phys_Def_ID( void ) const	{ return PhysDefID; }
	int		Get_Orator_Type( void ) const	{ return OratorType; }

	DECLARE_EDITABLE( PhysicalGameObjDef, DamageableGameObjDef );

protected:
	int											Type;					
	int											RadarBlipType;
	float										BullseyeOffsetZ;
	StringClass									Animation;
	int											PhysDefID;
	int											KilledExplosion;
	bool										DefaultHibernationEnable;
	bool										AllowInnateConversations;
	int											OratorType;
	bool										UseCreationEffect;

	friend	class								PhysicalGameObj;
};

/*
**
*/
class PhysicalGameObj : public DamageableGameObj, public CombatPhysObserverClass {

public:
	//	Constructor and Destructor
	PhysicalGameObj( void );
	virtual	~PhysicalGameObj( void );

	// Definitions
	void	Init( const PhysicalGameObjDef & definition );
	void	Copy_Settings( const PhysicalGameObjDef & definition );
	void	Re_Init( const PhysicalGameObjDef & definition );
	const PhysicalGameObjDef & Get_Definition( void ) const ;

	// Save / Load
	virtual	bool	Save( ChunkSaveClass & csave ) override;
	virtual	bool	Load( ChunkLoadClass & cload ) override;
	virtual	void	On_Post_Load(void) override;

	virtual	void	Startup( void )	{}	

	// Physics
	WWINLINE PhysClass *	Peek_Physical_Object( void ) const { return PhysObj; }

	void					Attach_To_Object_Bone( PhysicalGameObj * host, const char * bone_name );
	bool					Is_Attached_To_An_Object( void ) { return (HostGameObj.Get_Ptr () != NULL); }
	void					Teleport_To_Host_Bone( void );

	void					Set_Transform(const Matrix3D & tm);
	const Matrix3D &	Get_Transform(void) const;
	virtual	void		Get_Position(Vector3 * set_pos) const override;
	void					Set_Position(const Vector3 & pos);
	float					Get_Facing(void) const;

	// Display
	WWINLINE RenderObjClass	*	Peek_Model( void ) { return Peek_Physical_Object()->Peek_Model(); }

	AnimControlClass*	Get_Anim_Control( void );
	void					Set_Anim_Control( AnimControlClass * anim_control );
	// Note: Set_Animation calls will force an AnimControl to be created, if needed
	virtual	void	Set_Animation( const char *animation_name, bool looping = true, float frame_offset = 0.0f );
	virtual	void	Set_Animation_Frame ( const char *animation_name, int frame );

	// Targeting
	float		Get_Bullseye_Offset_Z( void ) const 		{ return Get_Definition().BullseyeOffsetZ; }
	virtual	Vector3	Get_Bullseye_Position( void );

	virtual	void	Get_Information( StringClass & string ) override;

	// Damage
	virtual	void	Apply_Damage( const OffenseObjectClass & damager, float scale = 1.0f, int alternate_skin = -1 ) override;
   virtual	void	Apply_Damage_Extended( const OffenseObjectClass & offense, float scale = 1.0f,
			const	Vector3 & direction = Vector3( 0,0,0 ), const char * collision_box_name = NULL );
	virtual	void	Completely_Damaged( const OffenseObjectClass & damager ) override;
	bool	Is_Soft( void );
	virtual	bool	Takes_Explosion_Damage( void )						{ return true; }

	// Game Object Type
	int	Get_Type()	const		{ return Get_Definition().Type; }

	//	Thinking
	virtual	void	Post_Think() override;

	// Collision
	void		Set_Collision_Group( int group );
	virtual ExpirationReactionType	Object_Expired( PhysClass * /* observed_obj */ ) override { Set_Delete_Pending(); return EXPIRATION_APPROVED; }

	// Type identification
	virtual	PhysicalGameObj		*As_PhysicalGameObj( void )	override { return this; }
	virtual	DamageableGameObj		*As_DamageableGameObj( void ) override	{ return this; }		// Re-implement for CombatPhysObserverClass
	virtual	SoldierGameObj			*As_SoldierGameObj( void ) override		{ return (SoldierGameObj *)NULL; }
	virtual	PowerUpGameObj			*As_PowerUpGameObj( void )		{ return (PowerUpGameObj *)NULL; }
	virtual  VehicleGameObj		   *As_VehicleGameObj( void ) override		{ return (VehicleGameObj *)NULL; }
	virtual  C4GameObj		      *As_C4GameObj( void )	      { return (C4GameObj *)NULL; }
	virtual	BeaconGameObj			*As_BeaconGameObj (void)		{ return (BeaconGameObj *)NULL; }
	virtual	ArmedGameObj			*As_ArmedGameObj( void )		{ return (ArmedGameObj *)NULL; }
	virtual	CinematicGameObj		*As_CinematicGameObj( void )	{ return NULL; }
	virtual	SimpleGameObj			*As_SimpleGameObj( void )		{ return NULL; }

	// Network diagnostics
	BYTE		Get_Server_Skips(void) {return ServerUpdateSkips;}
	void		Reset_Server_Skips(BYTE value);
	void		Increment_Server_Skips(void);


	/*
	// Color Tinting
	void		Set_Tint(Vector3 color);
	const		Vector3 & Get_Tint(void) {return TintColor;}
	*/

	// Hibernation
	virtual	bool	Is_Hibernating( void ) override					{ return HibernationTimer <= 0; }
	void				Enable_Hibernation( bool enable )	{ HibernationEnable = enable; if ( Is_Hibernating() ) HibernationTimer = 1;}
	void				Reset_Hibernating( void );
	void				Do_Not_Hibernate( void )				{ if ( HibernationTimer < 1 ) HibernationTimer = 1; }
	virtual	void	Begin_Hibernation( void );
	virtual	void	End_Hibernation( void );

	// Radar Blips
	int				Get_Radar_Blip_Shape_Type( void )				{ return RadarBlipShapeType; }
	void				Set_Radar_Blip_Shape_Type( int type )			{ RadarBlipShapeType = type; }
	void				Reset_Radar_Blip_Shape_Type( void )				{ RadarBlipShapeType = Get_Definition().RadarBlipType; }
	int				Get_Radar_Blip_Color_Type( void )				{ return RadarBlipColorType; }
	void				Set_Radar_Blip_Color_Type( int type )			{ RadarBlipColorType = type; }
	void				Reset_Radar_Blip_Color_Type( void );
	float				Get_Radar_Blip_Intensity( void )					{ return RadarBlipIntensity; }
	void				Set_Radar_Blip_Intensity( float value )		{ RadarBlipIntensity = value; }

	// Network support
	virtual void	Export_Creation( BitStreamClass &packet ) override;
	virtual void	Import_Creation( BitStreamClass &packet ) override;
	virtual void	Export_Rare( BitStreamClass &packet ) override;
	virtual void	Import_Rare( BitStreamClass &packet ) override;
	virtual void	Export_Frequent( BitStreamClass &packet ) override;
	virtual void	Import_Frequent( BitStreamClass &packet ) override;

	virtual int		Get_Vis_ID(void) override;
	virtual bool	Get_World_Position (Vector3 &pos) const override		{ Get_Position (&pos); return true; }

	//
	//	Conversation support
	//
	bool				Are_Innate_Conversations_Enabled( void )				{ return Get_Definition().AllowInnateConversations && IsInnateConversationsEnabled; }
	void				Enable_Innate_Conversations( bool enable )				{ IsInnateConversationsEnabled = enable; }
	bool				Is_In_Conversation( void ) const							{ return ActiveConversation != NULL; }
	void				Set_Conversation( ActiveConversationClass *conversation );

	void				Hide_Muzzle_Flashes( bool hide = true );

	void				Enable_HUD_Pokable_Indicator( bool enable );
	bool				Is_HUD_Pokable_Indicator_Enabled( void )		{ return HUDPokableIndicatorEnabled; } 

	virtual void		Set_Player_Type(int type) override;

	//
	// Phys Observer support
	//
	virtual void	Object_Shattered_Something(PhysClass * observed_obj, PhysClass * shattered_obj, int surface_type) override;

protected:

	//
	//	Conversation stuff
	//
	ActiveConversationClass *ActiveConversation;

private:

	PhysClass *				PhysObj;
	AnimControlClass *	AnimControl;

	//
	// Networking bandwidth stuff
	//
	BYTE						ServerUpdateSkips;

	//Vector3					TintColor;

	float						HibernationTimer;
	bool						HibernationEnable;

	GameObjReference		HostGameObj;
	int						HostGameObjBone;

	// Radar Blips
	int						RadarBlipShapeType;
	int						RadarBlipColorType;
	float						RadarBlipIntensity;

	// Pending Host Obj
	int						PendingHostObjID;

	bool					HUDPokableIndicatorEnabled;
	bool					IsInnateConversationsEnabled;
};


#endif	//	PHYSICALGAMEOBJ_H
