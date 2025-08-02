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
 *                     $Archive:: /Commando/Code/Combat/playerdata.h          $*
 *                                                                                             *
 *                      $Author:: Byon_g                                                      $*
 *                                                                                             *
 *                     $Modtime:: 3/29/02 3:42p                                               $*
 *                                                                                             *
 *                    $Revision:: 25                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef	PLAYERDATA_H
#define	PLAYERDATA_H

#ifndef	ALWAYS_H
	#include "always.h"
#endif

#ifndef	GAMEOBJREF_H
	#include "gameobjref.h"
#endif

#ifndef	_DATASAFE_H
    #include	"../Commando/datasafe.h"
#endif		//_DATASAFE_H

#ifndef	SIMPLEVEC_H
	#include "simplevec.h"
#endif


class	ChunkSaveClass;
class	ChunkLoadClass;
class	DamageableGameObj;
class	SmartGameObj;
class BitStreamClass;

/*
** PlayerDataClass
**
** This class maintains the score and statistics for human player.
** It is intended that the Commando code will derive a custom version,
** and install it into each Human Player controlled SoldierGameObj.
** When the SoldierGameObj dies, a new one should be created and the
** same PlayerDataClass should be plugged into the new SoldierGameObj
**
** It is the creators responsibility to save and restore these classes,
** and to re-install into the GameObjs after load.
*/
class PlayerDataClass
{
public:
	PlayerDataClass( void );
	~PlayerDataClass( void );

   virtual	void Reset_Player(void);

	bool Save(ChunkSaveClass & csave);
	bool Load(ChunkLoadClass & cload);

	void	Set_GameObj( SmartGameObj * soldier )	{ GameObj = (ScriptableGameObj*)soldier; }
	SmartGameObj * Get_GameObj( void )				{ return (SmartGameObj*)(GameObj.Get_Ptr()); }

	// Score access
				float	Get_Score( void ) const 		{ return (float)Score; }
	virtual	void	Set_Score( float score )		{ Score = score; }
	virtual	void	Increment_Score( float add );

	// Money access
				float	Get_Money( void ) const 		{ return (float)Money; }
	virtual	void	Set_Money( float money )		{ Money = money; }
	virtual	void	Increment_Money( float add );
	virtual	bool	Purchase_Item( int cost );

	// Damage Notification
	virtual	void	Apply_Damage_Points( float points, DamageableGameObj * victim );
	virtual	void	Apply_Death_Points( float points, DamageableGameObj * victim );

	// Network support
	void				Import_Occasional( BitStreamClass & packet );
	void				Export_Occasional( BitStreamClass & packet );

	// Stats
	void				Stats_Reset( void );
	void				Stats_Add_Game_Time( float time )		{ StatTimeInGame += time; StatTimeInSession+=time; }
	void				Stats_Add_Enemy_Killed( void )			{ StatEnemiesKilled++; }
	void				Stats_Add_Ally_Killed( void )				{ StatAlliesKilled++; }
	void				Stats_Add_Shot_Fired( void )				{ StatShotsFired++; }
	void				Stats_Add_Head_Shot( void )				{ StatHeadShots++; }
	void				Stats_Add_Torso_Shot( void )				{ StatTorsoShots++; }
	void				Stats_Add_Arm_Shot( void )					{ StatArmShots++; }
	void				Stats_Add_Leg_Shot( void )					{ StatLegShots++; }
	void				Stats_Add_Crotch_Shot( void )				{ StatCrotchShots++; }
	void				Stats_Set_Final_Health( float health )	{ StatFinalHealth = health; }
	void				Stats_Add_Powerup( void )					{ StatPowerups++; }
	void				Stats_Add_Vehicle_Destroyed( void )		{ StatVehiclesDestroyed++; }
	void				Stats_Add_Vehicle_Time( float time )	{ StatTimeInVehicles += time; }
	void				Stats_Add_Kill_From_Vehicle( void )		{ StatKillsFromVehicles++; }
	void				Stats_Add_Squish( void )					{ StatSquishes++; }
	void				Stats_Add_Credit_Grant( float grant )	{ StatCreditsGranted += grant; }
	void				Stats_Add_Building_Destroyed( void ) 	{ StatBuildingsDestroyed++; }
	void				Stats_Add_Head_Hit( void )					{ StatHeadHits++; }
	void				Stats_Add_Torso_Hit( void )				{ StatTorsoHits++; }
	void				Stats_Add_Arm_Hit( void )					{ StatArmHits++; }
	void				Stats_Add_Leg_Hit( void )					{ StatLegHits++; }
	void				Stats_Add_Crotch_Hit( void )				{ StatCrotchHits++; }
	void				Stats_Add_Weapon_Fired( int weapon_id );
	
	float				Get_Game_Time(void) const					{return StatTimeInGame;}
	float				Get_Session_Time(void) const				{return StatTimeInSession;}	// Note: This is per-session, it is not saved or loaded!
	int				Get_Enemies_Killed(void) const			{return StatEnemiesKilled;}
	int				Get_Allies_Killed(void) const				{return StatAlliesKilled;}
	int				Get_Shots_Fired(void) const				{return StatShotsFired;}
	int				Get_Head_Shots(void) const					{return StatHeadShots;}
	int				Get_Torso_Shots(void) const				{return StatTorsoShots;}
	int				Get_Arm_Shots(void) const					{return StatArmShots;}
	int				Get_Leg_Shots(void) const					{return StatLegShots;}
	int				Get_Crotch_Shots(void) const				{return StatCrotchShots;}
	float				Get_Final_Health(void) const				{return StatFinalHealth;}
	int				Get_Powerups_Collected(void) const		{return StatPowerups;}
	int				Get_Vehiclies_Destroyed(void) const		{return StatVehiclesDestroyed;}
	float				Get_Vehicle_Time(void) const				{return StatTimeInVehicles;}
	int				Get_Kills_From_Vehicle(void) const		{return StatKillsFromVehicles;}
	int				Get_Squishes(void) const					{return StatSquishes;}
	float				Get_Credit_Grant(void) const				{return StatCreditsGranted;}
	int				Get_Building_Destroyed(void) const		{return StatBuildingsDestroyed;}
	int				Get_Head_Hit(void) const					{return StatHeadHits;}
	int				Get_Torso_Hit(void) const					{return StatTorsoHits;}
	int				Get_Arm_Hit(void) const						{return StatArmHits;}
	int				Get_Leg_Hit(void) const						{return StatLegHits;}
	int				Get_Crotch_Hit(void) const					{return StatCrotchHits;}

	int				Get_Weapon_Fired_Count( void ) const;
	bool				Get_Weapon_Fired( int index, unsigned long& weaponID, unsigned long& firedCount) const;

	float				Get_Punish_Timer( void )					{return PunishTimer;}
	void				Inc_Punish_Timer( float amount )			{PunishTimer += amount;}

private:
	safe_float			Score;
	safe_float			Money;
	GameObjReference	GameObj;
	float					PunishTimer;

protected:
	float				StatTimeInGame;
	float				StatTimeInSession;
	int				StatEnemiesKilled;
	int				StatAlliesKilled;
	int				StatShotsFired;
	int				StatHeadShots;
	int				StatTorsoShots;
	int				StatArmShots;
	int				StatLegShots;
	int				StatCrotchShots;
	float				StatFinalHealth;
	int				StatPowerups;
	int				StatVehiclesDestroyed;
	float				StatTimeInVehicles;
	int				StatKillsFromVehicles;
	int				StatSquishes;
	float				StatCreditsGranted;
	int				StatBuildingsDestroyed;
	int				StatHeadHits;
	int				StatTorsoHits;
	int				StatArmHits;
	int				StatLegHits;
	int				StatCrotchHits;

	int				StatLastWeaponIndex;
	SimpleDynVecClass<int>	StatWeaponIDFired;
	SimpleDynVecClass<int>	StatWeaponFireCount;
};

#endif	//	PLAYERDATA_H









	//safe_int				Score;
	//virtual	bool	Purchase_Item( int cost );
	//virtual	void	On_Kill( int victim_id )		{}
	//virtual	void	On_Kill( int victim_id, int victim_team )		{}
