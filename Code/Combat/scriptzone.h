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
 *                     $Archive:: /Commando/Code/Combat/scriptzone.h                          $* 
 *                                                                                             * 
 *                      $Author:: Patrick                                                     $* 
 *                                                                                             * 
 *                     $Modtime:: 11/27/01 4:46p                                              $* 
 *                                                                                             * 
 *                    $Revision:: 14                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef	SCRIPTZONEOBJ_H
#define	SCRIPTZONEOBJ_H

#ifndef	ALWAYS_H
	#include "always.h"
#endif

#ifndef SCRIPTABLEGAMEOBJ_H
	#include "scriptablegameobj.h"
#endif

#ifndef OBBOX_H
	#include "obbox.h"
#endif

#ifndef SLIST_H
	#include "slist.h"
#endif

/*
** ZoneConstants
** Convienent namespace declaration for the constants used with zones
*/
namespace ZoneConstants
{
	typedef enum
	{
		TYPE_DEFAULT					= 0,
		TYPE_CTF,
		TYPE_VEHICLE_CONSTRUCTION,
		TYPE_VEHICLE_REPAIR,
		TYPE_TIBERIUM_FIELD,
		TYPE_BEACON,
		TYPE_GDI_TIB_FIELD,
		TYPE_NOD_TIB_FIELD,
		TYPE_COUNT
	} ZoneType;
}


/*
** ZoneGameObjDef - Defintion class for a ZoneGameObj
*/
class ScriptZoneGameObjDef : public ScriptableGameObjDef
{
public:
	ScriptZoneGameObjDef( void );

	virtual uint32								Get_Class_ID (void) const override;
	virtual PersistClass *					Create( void ) const override;
	virtual bool								Save( ChunkSaveClass &csave ) override;
	virtual bool								Load( ChunkLoadClass &cload ) override;
	virtual bool								Is_Valid_Config (StringClass &/* message */) override { return true; }
	virtual const PersistFactoryClass &	Get_Factory( void ) const override;

	DECLARE_EDITABLE( ScriptZoneGameObjDef, ScriptableGameObjDef );

	const Vector3 &							Get_Color (void) const { return Color; }

	ZoneConstants::ZoneType					Get_Type (void) const	{ return ZoneType; }

protected:

	ZoneConstants::ZoneType					ZoneType;
	Vector3										Color;
	bool											IsCTFZone;
	bool											CheckStarsOnly;
	bool											IsEnvironmentZone;
	friend	class								ScriptZoneGameObj;
};


/*
**
*/
class ScriptZoneGameObj : public ScriptableGameObj {

public:
	ScriptZoneGameObj();
	virtual	~ScriptZoneGameObj();

	// Definitions
	virtual	void	Init( void ) override;
	void	Init( const ScriptZoneGameObjDef & definition );
	const ScriptZoneGameObjDef & Get_Definition( void ) const;
	
	// RTTI
	ScriptZoneGameObj	*As_ScriptZoneGameObj( void ) override	{ return this; };

	// Save / Load / Construction Factory
	virtual	bool	Save( ChunkSaveClass & csave ) override;
	virtual	bool	Load( ChunkLoadClass & cload ) override;
	virtual	const	PersistFactoryClass & Get_Factory( void ) const override;

	//	Thinking
	virtual	void		Think() override;

	virtual	void		Get_Position(Vector3 * set_pos) const override { *set_pos = BoundingBox.Center; }

	// Bounding Box
	void	Set_Bounding_Box( OBBoxClass & box )			{ BoundingBox = box; }
	const OBBoxClass & Get_Bounding_Box(void)				{ return BoundingBox; }

	// PlayerType (a simple copy of what's used in PhyiscalGameObj, needed for CTF
   int		Get_Player_Type( void) const					{ return PlayerType; }
   void		Set_Player_Type( int type )					{ PlayerType = type; }

   int		Count_Team_Members_Inside( int player_type );

	static ScriptZoneGameObj *	Find_Closest_Zone (const Vector3 &pos, ZoneConstants::ZoneType type);

	// Network support
	//virtual bool	Exists_On_Client( void ) const;

   bool	Is_Environment_Zone( void )						{ return Get_Definition().IsEnvironmentZone; }


protected:
	OBBoxClass			BoundingBox;
	int					PlayerType;

	// a list of all SMART objects currently inside me and a checking function
	SList<GameObjReference>	  	InsideList;
	void		Entered( SmartGameObj * obj );
	bool		In_List( SmartGameObj * obj );
	bool		Inside_Me( const SmartGameObj * obj );

};

#endif


#if 0

// New Zone System
// We need zones for script triggering, transitions, doors, elevators, spawners

// Can Zones trigger from Soldier / smart / physical?

/*
**
*/
class	ZoneObserverClass {

	typedef enum {
		ENTERED,
		EXITED,
		TRIGGER,
		VEHICLE_KEY,
	};

	void	Trigger( SmartGameObj * obj, int trigger_type )						= 0;

	void	Object_Entered( SmartGameObj * obj )									= 0;
	void	Object_Exited( SmartGameObj * obj )										= 0;
	void	Object_Inside_Actioned( SmartGameObj * obj, int action_type )	= 0;
};

/*
** Abstract Base ZoneClass
*/ 
class	ZoneClass {

public:
	void	Set_Zone( const OBBoxClass	& zone );
	void	Set_Observer( ZoneObserverClass * observer );

	void	Trigger_Check( const DynamicVectorClass<SmartGameObj *> & obj_list )	= 0;

protected:
	OBBoxClass				ZoneBox;
	ZoneObserverClass *	Observer;
}

/*
** EnterExitZoneClass maintains a list of SmartGameObject inside it
** and notifies observer of enter and exit changes.
*/
class	EnterExitZoneClass : public ZoneClass {

public:
	void	Trigger_Check( const DynamicVectorClass<SmartGameObj *> & obj_list );

protected:
	DynamicVectorClass<SmartGameObj *> Contents;	// Must be a gameobjreference
};

/*
** TransitionZoneClass 
*/
class	TransitionZoneClass : public ZoneClass {
public:
	void	Set_Trigger_Movement_Vector( const Vector3 & trigger_move );
	void	Set_Trigger_Movement_Threshhold( float threshold );
	void	Set_TM( const Matrix3D & tm );

	void	Trigger_Check( const DynamicVectorClass<SmartGameObj *> & obj_list );

protected:
	Matrix3D			TM;
};

/*
**
*/
class	ZoneManager {

	void	Update( void );

	// Add a zone
	void	Add_Zone( ZoneClass * zone );
	void	Remove_Zone( ZoneClass * zone );

};

#endif  // SCRIPTZONEOBJ