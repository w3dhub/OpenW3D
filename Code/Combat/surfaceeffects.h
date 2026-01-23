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
 *                     $Archive:: /Commando/Code/Combat/surfaceeffects.h                      $* 
 *                                                                                             * 
 *                      $Author:: Byon_g                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 11/12/01 11:15a                                             $* 
 *                                                                                             * 
 *                    $Revision:: 29                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef	SURFACEEFFECTS_H
#define	SURFACEEFFECTS_H

#ifndef	ALWAYS_H
	#include "always.h"
#endif

class	Matrix3D;
class PhysClass;
class	PhysicalGameObj;

class	PersistantSurfaceSoundClass;
class	PersistantSurfaceEmitterClass;
class	PhysicalGameObj;

/*
**
*/
class	SurfaceEffectsManager {

public:

	/*
	**
	*/
	typedef enum {
		
		MODE_OFF				= 0,
		MODE_NO_EMITTERS,
		MODE_FULL
	} MODE;

	static	void	Init( void );
	static	void	Shutdown( void );

	static	void	Enable_Effects( bool /* enable */ )			{ Mode = MODE_FULL; }
	static	bool	Are_Effects_Enabled( void )			{ return (Mode != MODE_OFF); }

	static	void	Set_Mode( MODE mode )					{ Mode = mode; }
	static	MODE	Get_Mode( void )							{ return Mode; }

	/*
	**
	*/
	enum {		// ONLY ADD TO THE END.....

		HITTER_TYPE_NONE,							// used to shut off persistant effects
		HITTER_TYPE_GENERIC_OBJECT,			// generic dynamic object 

		HITTER_TYPE_FOOTSTEP_RUN,
		HITTER_TYPE_FOOTSTEP_WALK,
		HITTER_TYPE_FOOTSTEP_CROUCHED,
		HITTER_TYPE_FOOTSTEP_JUMP,
		HITTER_TYPE_FOOTSTEP_LAND,

		HITTER_TYPE_BULLET,
		HITTER_TYPE_BULLET_FIRE,
		HITTER_TYPE_BULLET_GRENADE,
		HITTER_TYPE_BULLET_CHEM,
		HITTER_TYPE_BULLET_ELECTRIC,
		HITTER_TYPE_BULLET_LASER,
		HITTER_TYPE_BULLET_SQUISH,
		HITTER_TYPE_BULLET_TIB_SPRAY,
		HITTER_TYPE_BULLET_TIB_BULLET,
		HITTER_TYPE_BULLET_SHOTGUN,
		HITTER_TYPE_BULLET_SILENCED,
		HITTER_TYPE_BULLET_SNIPER,
		HITTER_TYPE_BULLET_WATER,

		HITTER_TYPE_EJECT_CASING,				// casings ejected from weapons (all are assumed to make the same sounds)
		HITTER_TYPE_SHELL_SHOTGUN,

		HITTER_TYPE_TIRE_ROLLING,				// wheels on humvees, etc...
		HITTER_TYPE_TIRE_SLIDING,				
		HITTER_TYPE_TRACK_ROLLING,				// tank tracks...
		HITTER_TYPE_TRACK_SLIDING,

		HITTER_TYPE_FOOTSTEP_LADDER,

		// Note:  If you change this enum, update the names in surfaceeffect.cpp

		NUM_HITTER_TYPES,
	};

	static	int	Num_Hitter_Types( void )			{ return NUM_HITTER_TYPES; }
   static	const char * Hitter_Type_Name( int hitter );

	// The transform for Apply_Effect is in camera convention, +Z is the normal of the surface
	static	void	Apply_Effect(		int surface_type, 
												int hitter_type, 
												const Matrix3D & tm, 
												PhysClass * hit_obj = NULL, 
												PhysicalGameObj * creator = NULL,
												bool allow_decals = true,
												bool allow_emitters = true);

	// Persist Effects
	static	PersistantSurfaceSoundClass *		Create_Persistant_Sound( void );
	static	void										Destroy_Persistant_Sound( PersistantSurfaceSoundClass * effect );
	static	void										Update_Persistant_Sound( PersistantSurfaceSoundClass * effect,
																int surface_type, int hitter_type, const Matrix3D & tm );
	
	static	PersistantSurfaceEmitterClass *	Create_Persistant_Emitter( void );
	static	void										Destroy_Persistant_Emitter( PersistantSurfaceEmitterClass * effect );
	static	void										Update_Persistant_Emitter( PersistantSurfaceEmitterClass * effect,
																int surface_type, int hitter_type, const Matrix3D & tm );

	static	void										Apply_Damage( int surface_type, PhysicalGameObj * obj );

	static	bool	Does_Surface_Stop_Bullets( int surface_type );

	static	void	Toggle_Debug( void );

	static	bool	Is_Surface_Permeable( int surface );

	/*
	** OverrideSurfaceType:
	** If you set an "override surface type" then *all* surfaces will be
	** treated as if they are that type.
	*/
	static	void	Set_Override_Surface_Type(int type);
	static	int	Get_Override_Surface_Type(void)	{ return OverrideSurfaceType; }

private:
	static	MODE	Mode;						// Do we need to save this???
	static	int	OverrideSurfaceType;					
};

#endif	// SURFACEEFFECTS_H


