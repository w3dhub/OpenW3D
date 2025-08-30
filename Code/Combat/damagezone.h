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
 *                     $Archive:: /Commando/Code/Combat/damagezone.h                          $* 
 *                                                                                             * 
 *                      $Author:: Byon_g                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 7/16/01 11:20a                                              $* 
 *                                                                                             * 
 *                    $Revision:: 12                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef	DAMAGEZONE_H
#define	DAMAGEZONE_H

#ifndef	ALWAYS_H
	#include "always.h"
#endif

#ifndef OBBOX_H
	#include "obbox.h"
#endif

#ifndef	BASEGAMEOBJ_H
	#include "basegameobj.h"
#endif

/*
** Damage Zone Definition
*/
class DamageZoneGameObjDef : public BaseGameObjDef
{

public:
	DamageZoneGameObjDef( void );

	virtual uint32								Get_Class_ID (void) const override;
	virtual PersistClass *					Create( void ) const override;
	virtual bool								Save( ChunkSaveClass &csave ) override;
	virtual bool								Load( ChunkLoadClass &cload ) override;
	virtual const PersistFactoryClass &	Get_Factory( void ) const override;

	DECLARE_EDITABLE( DamageZoneGameObjDef, BaseGameObjDef );

	const Vector3 &							Get_Color (void) const { return Color; }

protected:
	float			DamageRate;
	int			DamageWarhead;

	Vector3		Color;
	
	friend	class								DamageZoneGameObj;
};

/*
** Damage Zone 
*/
class DamageZoneGameObj : public BaseGameObj 
{

public:
									DamageZoneGameObj( void );
	virtual						~DamageZoneGameObj( void );

	virtual	void				Init( void ) override;
	void							Init( const DamageZoneGameObjDef & definition );
	const DamageZoneGameObjDef &	Get_Definition( void ) const;

	virtual bool				Save( ChunkSaveClass & csave ) override;
	virtual bool				Load( ChunkLoadClass & cload ) override;
	virtual const PersistFactoryClass &	Get_Factory( void ) const override;

	// Bounding Box
	void	Set_Bounding_Box( OBBoxClass & box )			{ BoundingBox = box; }
	const OBBoxClass & Get_Bounding_Box(void)				{ return BoundingBox; }

	virtual void				Think( void ) override;

	// Network support
	//virtual bool				Exists_On_Client( void ) const	{ return false; }

protected:
	OBBoxClass				BoundingBox;
	float						DamageTimer;

};

#endif