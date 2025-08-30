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
 *                     $Archive:: /Commando/Code/Combat/spawn.h                               $*
 *                                                                                             *
 *                      $Author:: Tom_s                                                       $*
 *                                                                                             *
 *                     $Modtime:: 11/19/01 10:27a                                             $*
 *                                                                                             *
 *                    $Revision:: 50                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef	SPAWN_H
#define	SPAWN_H

#ifndef	ALWAYS_H
	#include "always.h"
#endif

#ifndef PERSIST_H
	#include "persist.h"
#endif

#ifndef DEFINITION_H
	#include "definition.h"
#endif

#ifndef VECTOR_H
	#include "vector.h"
#endif

#ifndef GAMEOBJREF_H
	#include "gameobjref.h"
#endif

class	PhysicalGameObj;
class	SoldierGameObj;

/*
**
*/
class	SpawnerDefClass : public DefinitionClass {

public:
	SpawnerDefClass( void );

	virtual uint32								Get_Class_ID (void) const override;
	virtual PersistClass *					Create( void ) const override;
	virtual bool								Save( ChunkSaveClass &csave ) override;
	virtual bool								Load( ChunkLoadClass &cload ) override;
	virtual const PersistFactoryClass &	Get_Factory( void ) const override;

	DECLARE_EDITABLE( SpawnerDefClass, DefinitionClass );

	const DynamicVectorClass<int>	&		Get_Spawn_Definition_ID_List (void) const { return SpawnDefinitionIDList; }

	int											Get_Player_Type( void )	const					{ return PlayerType; }
	bool											Is_Multiplay_Weapon_Spawner(void) const	{ return IsMultiplayWeaponSpawner; }

protected:
	DynamicVectorClass<int>					SpawnDefinitionIDList;
	int											PlayerType;

	int											SpawnMax;
	float											SpawnDelay;
	float											SpawnDelayVariation;

	bool											IsPrimary;
	bool											IsSoldierStartup;
	bool											GotoSpawnerPos;
	float											GotoSpawnerPosPriority;
	bool											TeleportFirstSpawn;

	int											SpecialEffectsObjID;
	float											PostVisualSpawnDelay;

	bool											StartsDisabled;
	bool											KillHibernatingSpawn;
	bool											ApplySpawnMaterialEffect;
	bool											IsMultiplayWeaponSpawner;

	DynamicVectorClass<StringClass>		ScriptNameList;
	DynamicVectorClass<StringClass>		ScriptParameterList;

	friend	class								SpawnerClass;
	friend	class								SpawnManager;
};


/*
**
*/
typedef	DynamicVectorClass<Matrix3D>	SpawnPointListType;

class SpawnerClass : public PersistClass {

public:
	//	Constructor and Destructor
	SpawnerClass( void );
	virtual	~SpawnerClass( void );

	// Definitions
	void	Init( const SpawnerDefClass & definition );
	const SpawnerDefClass & Get_Definition( void ) const { WWASSERT( Definition ); return *Definition; }

	// Save / Load
	virtual	bool	Save( ChunkSaveClass & csave ) override;
	virtual	bool	Load( ChunkLoadClass & cload ) override;
	virtual	const	PersistFactoryClass & Get_Factory( void ) const override;

	// Parameters
	SpawnPointListType * Get_Spawn_Point_List( void )	{ return &SpawnPointList; }
	Matrix3D	Get_TM( void )				{ return TM; }
	void	Set_TM( Matrix3D & tm )		{ TM = tm; }
	int	Get_ID( void )					{ return ID; }
	void	Set_ID( int id )				{ ID = id; }

	// Needed for Level Editor
	PhysicalGameObj * Create_Spawned_Object( int obj_id = -1 );

	// Add a script to this spawner instance (only applied to the spawn)
	void	Add_Script( const char * script_name, const char * script_parameter );

	bool					Can_Spawn_Object( int obj_def_id );
	PhysicalGameObj * Spawn_Object( int obj_def_id );
	void					Enable( bool enable )			{ Enabled = enable;	}

private:
	int						ID;
	Matrix3D					TM;
	Matrix3D					SpawnTM;
	const SpawnerDefClass * Definition;

	GameObjReference		LastSpawn;

	bool						Enabled;
	int						SpawnCount;
	float						SpawnDelayTimer;

	SpawnPointListType	SpawnPointList;

	DynamicVectorClass<StringClass>		ScriptNameList;
	DynamicVectorClass<StringClass>		ScriptParameterList;

	/*
	** Spawning
	*/
	PhysicalGameObj * Spawn( int obj_id = -1 );
	void					Check_Auto_Spawn( float dtime );

	bool					Determine_Spawn_TM( PhysicalGameObj * obj );

	friend	class				SpawnManager;
};


/*
**
*/
class	SpawnManager {

public:
	~SpawnManager(void);
	static	void	Update( void );

	static	bool	Save( ChunkSaveClass &csave );
	static	bool	Load( ChunkLoadClass &cload );

	static	Matrix3D	Get_Primary_Spawn_Location( void );
	static	Matrix3D	Get_Multiplayer_Spawn_Location( int player_type, SoldierGameObj * soldier );
	static	Matrix3D	Get_Ctf_Spawn_Location( int team );
	static	bool		Spawner_Exists( int player_type );
	static	SpawnerClass *	Get_Primary_Spawner( void );

	static	PhysicalGameObj * Spawner_Trigger( int id );
	static	void					Spawner_Enable( int id, bool enable );

	static	void	Display_Unused_Spawners( void );
	static	bool	Toggle_Allow_Killing_Hibernating_Spawn( void );

	static	DynamicVectorClass<SpawnerClass*>	Get_Spawner_List(void) {return SpawnerList;}

protected:
	static	void	Remove_All_Spawners( void );
	static	void	Add_Spawner( SpawnerClass * spawner );
	static	void	Remove_Spawner( SpawnerClass * spawner );

	static	DynamicVectorClass<SpawnerClass*>	SpawnerList;
	static	float											AutoSpawnTimer;

	friend	class	SpawnerClass;
};

#endif	//	SPAWN_H	







//static	Matrix3D	Get_First_Spawn_Location( int player_type );
	//int						SpawnNumber;				// These are used for sequenced spawns
	//static int				GlobalSpawnNumber;

	/*
	void	Set_Spawn_Number(int spawn_number) {SpawnNumber = spawn_number;}
	int	Get_Spawn_Number(void) {return SpawnNumber;}
	void	Update_Spawn_Number( void ) { SpawnNumber = GlobalSpawnNumber++; }
	*/

