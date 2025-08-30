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
 *                     $Archive:: /Commando/Code/Combat/cinematicgameobj.h                    $* 
 *                                                                                             * 
 *                      $Author:: Tom_s                                                       $* 
 *                                                                                             * 
 *                     $Modtime:: 9/17/01 4:24p                                               $* 
 *                                                                                             * 
 *                    $Revision:: 19                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef	CINEMATICGAMEOBJ_H
#define	CINEMATICGAMEOBJ_H

#ifndef	ALWAYS_H
	#include "always.h"
#endif

#ifndef ARMEDGAMEOBJ_H
	#include "armedgameobj.h"
#endif

class	AudibleSoundClass;

/*
** CinematicGameObjDef - Defintion class for a CinematicGameObj
*/
class CinematicGameObjDef : public ArmedGameObjDef
{
public:
	CinematicGameObjDef( void );

	virtual uint32								Get_Class_ID (void) const override;
	virtual PersistClass *					Create( void ) const override;
	virtual bool								Save( ChunkSaveClass &csave ) override;
	virtual bool								Load( ChunkLoadClass &cload ) override;
	virtual const PersistFactoryClass &	Get_Factory( void ) const override;

	DECLARE_EDITABLE( CinematicGameObjDef, ArmedGameObjDef );

protected:
	int											SoundDefID;
	StringClass									SoundBoneName;
	StringClass									AnimationName;
	bool											AutoFireWeapon;
	bool											DestroyAfterAnimation;
	bool											CameraRelative;

	friend	class								CinematicGameObj;
};


/*
**
*/
class CinematicGameObj : public ArmedGameObj {

public:
	CinematicGameObj();
	virtual	~CinematicGameObj();

	// Definitions
	virtual	void	Init( void ) override;
	void	Init( const CinematicGameObjDef & definition );
	const CinematicGameObjDef & Get_Definition( void ) const ;
	void	Cinematic_Init( void );

	// Save / Load / Construction Factory
	virtual	bool	Save( ChunkSaveClass & csave ) override;
	virtual	bool	Load( ChunkLoadClass & cload ) override;
	virtual	void	On_Post_Load( void ) override;
	virtual	const	PersistFactoryClass & Get_Factory( void ) const override;

	// Think
	virtual	void	Think( void ) override;
	virtual	void	Post_Think( void ) override;

	// Type Identification
	virtual	CinematicGameObj		*As_CinematicGameObj( void ) override	{ return this; }

	// Sound
	void				Set_Sound( int	sound_def_id, const char * bone_name = ""  );

	// Damage
	virtual	bool	Takes_Explosion_Damage( void ) override						{ return false; }
	virtual	void	Completely_Damaged( const OffenseObjectClass & damager ) override;

	// Information
	float				Get_Animation_Length( void );

	// Network support
	//TSS: why import with no export?
	//virtual	void	Import_Creation( BitStreamClass &packet );

	// Network support
	virtual void	Export_Rare( BitStreamClass &packet ) override;
	virtual void	Import_Rare( BitStreamClass &packet ) override;

protected:
	AudibleSoundClass		*Sound;
};


#endif	//	CINEMATICGAMEOBJ_H
