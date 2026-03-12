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
 *                     $Archive:: /Commando/Code/Combat/transition.h                          $*
 *                                                                                             *
 *                      $Author:: Patrick                                                     $*
 *                                                                                             *
 *                     $Modtime:: 11/16/01 3:18p                                              $*
 *                                                                                             *
 *                    $Revision:: 22                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef	TRANSITION_H
#define	TRANSITION_H

#include "always.h"
#include "matrix3d.h"
#include "aabox.h"
#include "gameobjref.h"
#include "vector.h"
#include "obbox.h"
#include "wwstring.h"

class	SoldierGameObj;
class	VehicleGameObj;

/*
** TransitionDataClass
*/
class TransitionDataClass {

public:
	TransitionDataClass( void );
	~TransitionDataClass( void )	{ }

	bool	Save( ChunkSaveClass & csave );
	bool	Load( ChunkLoadClass & cload );

	typedef enum {
		DISABLED							= -1,
		LADDER_EXIT_TOP				= 0,
		LADDER_EXIT_BOTTOM,
		LADDER_ENTER_TOP,
		LADDER_ENTER_BOTTOM,
		LEGACY_VEHICLE_ENTER_0,
		LEGACY_VEHICLE_ENTER_1,
		LEGACY_VEHICLE_EXIT_0,
		LEGACY_VEHICLE_EXIT_1,
		VEHICLE_ENTER,
		VEHICLE_EXIT,
		NUM_TRANSITION_TYPE
	} StyleType;

	static int	Get_Num_Types( void )	{ return NUM_TRANSITION_TYPE; }
	static const char * Get_Type_Name( StyleType type );

	StyleType	Get_Type( void ) const	{ return Type; }
	void	Set_Type( StyleType type )		{ Type = type;	}

	const OBBoxClass & Get_Zone( void ) const { return Zone; }
	void	Set_Zone( const OBBoxClass & zone )	{ Zone = zone; }

	const char * Get_Animation_Name( void ) const 	{ return AnimationName; }
	void	Set_Animation_Name( const char * name )	{ AnimationName = name; }

	const Matrix3D & Get_Ending_TM( void ) const { return EndingTM; }
	void	Set_Ending_TM( const Matrix3D & tm )	{ EndingTM = tm; }

private:
	StyleType		Type;
	OBBoxClass		Zone;
	StringClass		AnimationName;
	Matrix3D			EndingTM;
};

typedef DynamicVectorClass<TransitionDataClass *> TRANSITION_DATA_LIST;


/*
** TransitionCompletionDataClass
*/
struct	TransitionCompletionDataStruct {

public:
	bool	Save( ChunkSaveClass & csave );
	bool	Load( ChunkLoadClass & cload );

	TransitionDataClass::StyleType	Type;
	GameObjReference			Vehicle;

};

/*
** TransitionInstanceClass
*/
class	TransitionInstanceClass {

public:
	TransitionInstanceClass( const TransitionDataClass & data );
	~TransitionInstanceClass( void );

	// Check for Triggering
	bool				Check( SoldierGameObj *obj, bool action_trigger );
	void				Start( SoldierGameObj *obj );
	static	void	End( SoldierGameObj *obj, TransitionCompletionDataStruct * data );

	TransitionDataClass::StyleType	Get_Type( void ) const 	{ return Data.Get_Type(); }

	void		Set_Parent_Transform( const Matrix3D & tm );

	VehicleGameObj * Get_Vehicle( void )	{ return (VehicleGameObj*)Vehicle.Get_Ptr(); }
	void		Set_Vehicle( VehicleGameObj * vehicle )	{ Vehicle = (GameObjRefPtr)vehicle; }

	// Accessors
	const OBBoxClass &	Get_Zone( void ) const			{ return Zone; }
	const Matrix3D &		Get_Ending_TM( void ) const	{ return EndingTM; }

	int		Get_Ladder_Index( void )					{ return LadderIndex; }
	void		Set_Ladder_Index( int ladder_index )	{ LadderIndex = ladder_index; }

protected:
	Matrix3D							EndingTM;
	OBBoxClass						Zone;
	GameObjReference				Vehicle;
	const	TransitionDataClass & Data;
	int								LadderIndex;
};

/*
**
*/
class	TransitionManager {

public:
	static	void					Reset( void );
	static	void					Add( TransitionInstanceClass * transition );
	static	void					Destroy( TransitionInstanceClass * transition );
	static	void					Destroy_Pending( void );

	static	bool					Check( SoldierGameObj *obj, bool action_trigger );

	static void						Build_Ladder_List (DynamicVectorClass<TransitionInstanceClass *> &list);
};

#endif	//	TRANSITION_H



