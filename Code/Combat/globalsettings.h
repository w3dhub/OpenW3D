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
 *                     $Archive:: /Commando/Code/Combat/globalsettings.h                      $* 
 *                                                                                             * 
 *                      $Author:: Greg_h                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 11/27/01 10:56a                                             $* 
 *                                                                                             * 
 *                    $Revision:: 33                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef	GLOBALSETTINGS_H
#define	GLOBALSETTINGS_H

#ifndef	ALWAYS_H
	#include "always.h"
#endif

#ifndef DEFINITION_H
	#include "definition.h"
#endif

#ifndef RECT_H
	#include "rect.h"
#endif

/*
** GlobalSettingsDef - Defintion class for a GlobalSettingsDef
*/
class GlobalSettingsDef : public DefinitionClass
{
public:
	GlobalSettingsDef( void );
	~GlobalSettingsDef( void );

	virtual uint32								Get_Class_ID (void) const override;
	virtual PersistClass *					Create( void ) const override;
	virtual bool								Save( ChunkSaveClass &csave ) override;
	virtual bool								Load( ChunkLoadClass &cload ) override;
	virtual const PersistFactoryClass &	Get_Factory( void ) const override;

	DECLARE_EDITABLE( GlobalSettingsDef, DefinitionClass );

	static GlobalSettingsDef *		Get_Global_Settings (void)					{ return GlobalSettings; }	
	int									Get_Death_Sound_ID (void) const				{ return DeathSoundID; }
	int									Get_EVA_Objectives_Sound_ID (void) const	{ return EVAObjectivesSoundID; }
	int									Get_HUD_Help_Text_Sound_ID (void) const	{ return HUDHelpTextSoundID; }	
											
	float									Get_Conversation_Dist (void) const			{ return MaxConversationDist; }
	float									Get_Combat_Conversation_Dist (void) const	{ return MaxCombatConversationDist; }
											
	float									Get_Soldier_Walk_Speed(void) const			{ return SoldierWalkSpeed; }
	float									Get_Soldier_Crouch_Speed(void) const		{ return SoldierCrouchSpeed; }
											
	float									Get_Falling_Damage_Min_Distance(void) const	{ return FallingDamageMinDistance; }
	float									Get_Falling_Damage_Max_Distance(void) const	{ return FallingDamageMaxDistance; }
	int									Get_Falling_Damage_Warhead(void) const		{ return FallingDamageWarhead; }
											
	const char *						Get_Purchase_GDI_Characters_Texture(void) const	{ return PurchaseGDICharactersTexture; }
	const char *						Get_Purchase_GDI_Vehicles_Texture(void) const	{ return PurchaseGDIVehiclesTexture; }
	const char *						Get_Purchase_GDI_Equipment_Texture(void) const	{ return PurchaseGDIEquipmentTexture; }

	const char *						Get_Purchase_NOD_Characters_Texture(void) const	{ return PurchaseNODCharactersTexture; }
	const char *						Get_Purchase_NOD_Vehicles_Texture(void) const	{ return PurchaseNODVehiclesTexture; }
	const char *						Get_Purchase_NOD_Equipment_Texture(void) const	{ return PurchaseNODEquipmentTexture; }

	const char *						Get_Purchase_NOD_MUT_Characters_Texture(void) const	{ return PurchaseNODMUTCharactersTexture; }
	const char *						Get_Purchase_NOD_MUT_Vehicles_Texture(void) const		{ return PurchaseNODMUTVehiclesTexture; }
	const char *						Get_Purchase_NOD_MUT_Equipment_Texture(void) const		{ return PurchaseNODMUTEquipmentTexture; }

	const char *						Get_Purchase_GDI_MUT_Characters_Texture(void) const	{ return PurchaseGDIMUTCharactersTexture; }
	const char *						Get_Purchase_GDI_MUT_Vehicles_Texture(void) const		{ return PurchaseGDIMUTVehiclesTexture; }
	const char *						Get_Purchase_GDI_MUT_Equipment_Texture(void) const		{ return PurchaseGDIMUTEquipmentTexture; }

	int									Get_Encyclopedia_Event_String_ID(void) const	{ return EncyclopediaEventStringID; }

	float									Get_Stealth_Distance_Human(void) const			{ return StealthDistanceHuman; }
	float									Get_Stealth_Distance_Vehicle(void) const		{ return StealthDistanceVehicle; }

	float									Get_MP_Stealth_Distance_Human(void) const			{ return MPStealthDistanceHuman; }
	float									Get_MP_Stealth_Distance_Vehicle(void) const		{ return MPStealthDistanceVehicle; }

protected:

	int									DeathSoundID;
	int									EVAObjectivesSoundID;
	int									HUDHelpTextSoundID;
	float									MaxConversationDist;
	float									MaxCombatConversationDist;
	float									SoldierWalkSpeed;
	float									SoldierCrouchSpeed;
	float									FallingDamageMinDistance;
	float									FallingDamageMaxDistance;
	
	StringClass							PurchaseGDICharactersTexture;
	StringClass							PurchaseGDIVehiclesTexture;
	StringClass							PurchaseGDIEquipmentTexture;

	StringClass							PurchaseNODCharactersTexture;
	StringClass							PurchaseNODVehiclesTexture;
	StringClass							PurchaseNODEquipmentTexture;

	StringClass							PurchaseGDIMUTCharactersTexture;
	StringClass							PurchaseGDIMUTVehiclesTexture;
	StringClass							PurchaseGDIMUTEquipmentTexture;

	StringClass							PurchaseNODMUTCharactersTexture;
	StringClass							PurchaseNODMUTVehiclesTexture;
	StringClass							PurchaseNODMUTEquipmentTexture;

	int									EncyclopediaEventStringID;

	int									FallingDamageWarhead;

	float									StealthDistanceHuman;
	float									StealthDistanceVehicle;
	float									MPStealthDistanceHuman;
	float									MPStealthDistanceVehicle;

	static GlobalSettingsDef *		GlobalSettings;
};

/*
** HumanLoiterGlobalSettingsDef - Defintion class for a HumanLoiterGlobalSettingsDef
*/
class HumanLoiterGlobalSettingsDef : public DefinitionClass
{
public:
	HumanLoiterGlobalSettingsDef( void );
	~HumanLoiterGlobalSettingsDef( void );

	virtual uint32								Get_Class_ID (void) const override;
	virtual PersistClass *					Create( void ) const override;
	virtual bool								Save( ChunkSaveClass &csave ) override;
	virtual bool								Load( ChunkLoadClass &cload ) override;
	virtual const PersistFactoryClass &	Get_Factory( void ) const override;

	DECLARE_EDITABLE( HumanLoiterGlobalSettingsDef, DefinitionClass );

	float		Get_Activation_Delay( void )	{ return ActivationDelay; }
	const char *	Pick_Animation( void );

	static	HumanLoiterGlobalSettingsDef * Get_Default_Loiters( void );
	static	HumanLoiterGlobalSettingsDef * Get_Weapon_Loiters( void );
	static	HumanLoiterGlobalSettingsDef * Get_Weaponless_Loiters( void );

protected:
	float											ActivationDelay;
	float											LoiterFrequency;
	DynamicVectorClass<StringClass>		LoiterAnimList;

	static	HumanLoiterGlobalSettingsDef * DefaultLoiters;
	static	HumanLoiterGlobalSettingsDef * WeaponLoiters;
	static	HumanLoiterGlobalSettingsDef * WeaponlessLoiters;
};

/*
** HUDGlobalSettingsDef - Defintion class for a HUDGlobalSettingsDef
*/
class HUDGlobalSettingsDef : public DefinitionClass
{
public:
	HUDGlobalSettingsDef( void );
	virtual ~HUDGlobalSettingsDef( void );

	virtual uint32								Get_Class_ID (void) const override;
	virtual PersistClass *					Create( void ) const override;
	virtual bool								Save( ChunkSaveClass &csave ) override;
	virtual bool								Load( ChunkLoadClass &cload ) override;
	virtual const PersistFactoryClass &	Get_Factory( void ) const override;

	DECLARE_EDITABLE( HUDGlobalSettingsDef, DefinitionClass );

	static	const HUDGlobalSettingsDef *	Get_Instance( void )		{ return Instance; }

	// Colors
	const Vector3 &	Get_Nod_Color( void )	const					{ return NodColor; }
	const Vector3 &	Get_GDI_Color( void )	const					{ return GDIColor;	}
	const Vector3 &	Get_Neutral_Color( void )	const				{ return NeutralColor;	}
	const Vector3 &	Get_Mutant_Color( void )	const				{ return MutantColor; }
	const Vector3 &	Get_Renegade_Color( void )	const				{ return RenegadeColor;	}

	const Vector3 &	Get_Primary_Objective_Color( void )	const		{ return PrimaryObjectiveColor;	}
	const Vector3 &	Get_Secondary_Objective_Color( void )	const	{ return SecondaryObjectiveColor;	}
	const Vector3 &	Get_Tertiary_Objective_Color( void )	const	{ return TertiaryObjectiveColor;	}

	const Vector3 &	Get_Health_High_Color( void )	const			{ return HealthHighColor;	}
	const Vector3 &	Get_Health_Med_Color( void )	const			{ return HealthMedColor;	}
	const Vector3 &	Get_Health_Low_Color( void )	const			{ return HealthLowColor;	}

	const Vector3 &	Get_Enemy_Color( void )	const					{ return EnemyColor;	}
	const Vector3 &	Get_Friendly_Color( void )	const				{ return FriendlyColor;	}
	const Vector3 &	Get_No_Relation_Color( void )	const			{ return NoRelationColor;	}

public:
	Vector3		NodColor;
	Vector3		GDIColor;
	Vector3		NeutralColor;
	Vector3		MutantColor;
	Vector3		RenegadeColor;
	Vector3		PrimaryObjectiveColor;
	Vector3		SecondaryObjectiveColor;
	Vector3		TertiaryObjectiveColor;
	Vector3		HealthHighColor;
	Vector3		HealthMedColor;
	Vector3		HealthLowColor;
	Vector3		EnemyColor;
	Vector3		FriendlyColor;
	Vector3		NoRelationColor;

	// 2D interface settings
	float			InfoTextureSize;
	Vector2		StarBracketSize;
	Vector2		StarBracketOffset;
	RectClass	StarBracketUV;
	Vector2		StarBracketTopSize;
	Vector2		StarBracketTopOffset;
	RectClass	StarBracketTopUV;
	Vector2		StarBracketTopArmedSize;
	Vector2		StarBracketTopArmedOffset;
	RectClass	StarBracketTopArmedUV;
	Vector2		StarBarSize;
	Vector2		StarBarOffset;
	RectClass	StarBarUV;
	Vector2		StarBarEndSize;
	Vector2		StarBarEndOffset;
	RectClass	StarBarEndUV;
	Vector2		StarHealthSize;
	Vector2		StarHealthOffset;
	RectClass	StarHealthUV;
	Vector2		StarShieldSize;
	Vector2		StarShieldOffset;
	RectClass	StarShieldUV;
	Vector2		StarWeaponIconSize;
	Vector2		StarWeaponIconOffset;

	Vector2		TargetBracketSize;
	Vector2		TargetBracketOffset;
	RectClass	TargetBracketUV;
	Vector2		TargetIconSize;
	Vector2		TargetIconOffset;
	Vector2		TargetNameBarSize;
	Vector2		TargetNameBarOffset;
	RectClass	TargetNameBarUV;
	Vector2		TargetNameOffset;
	Vector2		TargetBarSize;
	Vector2		TargetBarOffset;
	RectClass	TargetBarUV;
	Vector2		TargetBarEndSize;
	Vector2		TargetBarEndOffset;
	RectClass	TargetBarEndUV;
	Vector2		TargetHealthSize;
	Vector2		TargetHealthOffset;
	RectClass	TargetHealthUV;
	Vector2		TargetShieldSize;
	Vector2		TargetShieldOffset;
	RectClass	TargetShieldUV;

	float			RadarTextureSize;
	Vector2		RadarOffset;
	float			RadarRadius;
	Vector2		RadarFrameSize;
	RectClass	RadarFrameUV;
	Vector2		RadarCompassOffset;
	Vector2		RadarCompassSize;
	RectClass	RadarCompassBaseUV;
	Vector2		RadarCompassUVOffset;
	RectClass	RadarHumanBlipUV;
	RectClass	RadarVehicleBlipUV;
	RectClass	RadarStationaryBlipUV;
	RectClass	RadarObjectiveBlipUV;
	RectClass	RadarBlipBracketUV;
	RectClass	RadarSweepUV;
	int			RadarOnSoundID;
	int			RadarOffSoundID;

	float			SniperTextureSize;
	RectClass	SniperView;
	RectClass	SniperViewUV;
	RectClass	SniperScanLineUV;
	RectClass	SniperBlackCoverUV;
	RectClass	SniperTiltBar;
	float			SniperTiltBarRate;
	RectClass	SniperTiltBarUV;
	RectClass	SniperTurnBar;
	float			SniperTurnBarRate;
	RectClass	SniperTurnBarUV;
	RectClass	SniperDistanceGraph;
	RectClass	SniperDistanceGraphUV;
	float			SniperDistanceGraphMax;
	RectClass	SniperZoomGraph;
	RectClass	SniperZoomGraphUV;

	RectClass	DamageIndicatorUV;
	RectClass	DamageDiagIndicatorUV;

	static		HUDGlobalSettingsDef *	Instance;
};

/*
** HumanAnimOverrides
*/
class	HumanAnimOverrideDef : public DefinitionClass {

public:
	HumanAnimOverrideDef( void );		

	virtual uint32								Get_Class_ID (void) const override;
	virtual PersistClass *					Create( void ) const override;
	virtual bool								Save( ChunkSaveClass &csave ) override;
	virtual bool								Load( ChunkLoadClass &cload ) override;
	virtual const PersistFactoryClass &	Get_Factory( void ) const override;

	DECLARE_EDITABLE( HumanAnimOverrideDef, DefinitionClass );

	StringClass									RunEmptyHands;
	StringClass									WalkEmptyHands;
	StringClass									RunAtChest;
	StringClass									WalkAtChest;
	StringClass									RunAtHip;
	StringClass									WalkAtHip;
};

#endif	//	GLOBALSETTINGS_H
