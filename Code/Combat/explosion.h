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
 *                     $Archive:: /Commando/Code/Combat/explosion.h                           $* 
 *                                                                                             * 
 *                      $Author:: Byon_g                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 1/09/02 3:16p                                               $* 
 *                                                                                             * 
 *                    $Revision:: 17                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef	EXPLOSION_H
#define	EXPLOSION_H

#ifndef	ALWAYS_H
	#include "always.h"
#endif

#ifndef	DEFINITION_H
	#include "definition.h"
#endif

class	ArmedGameObj;
class	BuildingGameObj;
class	DamageableGameObj;

/*
**
*/
class	ExplosionDefinitionClass : public DefinitionClass {

public:
	ExplosionDefinitionClass( void );		

	virtual uint32								Get_Class_ID( void ) const override;
	virtual PersistClass *					Create( void ) const override					{ WWASSERT( 0 ); return NULL; }
	virtual bool								Save( ChunkSaveClass &csave ) override;
	virtual bool								Load( ChunkLoadClass &cload ) override;
	virtual const PersistFactoryClass &	Get_Factory( void ) const override;

	DECLARE_EDITABLE( ExplosionDefinitionClass, DefinitionClass );

	int											PhysDefID;
	int											SoundDefID;
	float											DamageRadius;
	float											DamageStrength;
	int											DamageWarhead;
	bool											DamageIsScaled;
	StringClass									DecalFilename;
	float											DecalSize;
	bool											AnimatedExplosion;

	float											CameraShakeIntensity;
	float											CameraShakeRadius;
	float											CameraShakeDuration;
};

/*
** Explosion Manager
*/
class	ExplosionManager {

public:
	static	void	Create_Explosion_At( int exlosion_id, const Vector3 & pos, ArmedGameObj * damager, const Vector3 & blast_direction = Vector3( 0,0,-1), DamageableGameObj * force_victim = NULL );
	static	void	Create_Explosion_At( int exlosion_id, const Matrix3D & tm, ArmedGameObj * damager, const Vector3 & blast_direction = Vector3( 0,0,-1), DamageableGameObj * force_victim = NULL  );

	static	void	Explosion_Damage_Building( int exlosion_id, BuildingGameObj * building, bool mct_damage, ArmedGameObj * damager );

	static	void	Server_Explode( int explosion_id, const Vector3 & pos, int owner_id, DamageableGameObj * force_victim = NULL );
	static	void	Explode( int explosion_id, const Vector3 & pos, int owner_id, int victim_id = 0 );
};

#endif	//	EMITTER_H
