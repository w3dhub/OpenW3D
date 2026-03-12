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
 *                     $Archive:: /Commando/Code/Combat/action.h                              $*
 *                                                                                             *
 *                      $Author:: Patrick                                                     $*
 *                                                                                             *
 *                     $Modtime:: 10/09/01 2:15p                                              $*
 *                                                                                             *
 *                    $Revision:: 47                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef	ACTION_H
#define	ACTION_H

#ifndef	ALWAYS_H
	#include "always.h"
#endif

#ifndef	VECTOR3_H
	#include "vector3.h"
#endif

#ifndef	GAMEOBJREF_H
	#include "gameobjref.h"
#endif

#ifndef	WWSTRING_H
	#include "wwstring.h"
#endif

#ifndef	ACTIONPARAMS_H
	#include "actionparams.h"
#endif


/*
**
*/
class		ActionCodeClass;
class		SmartGameObj;

/*
**
*/
class		SafeActionParamsStruct : public ActionParamsStruct {
public:
	SafeActionParamsStruct & operator = (const ActionParamsStruct & src);	// Assignment operator.

	bool					Save( ChunkSaveClass & csave );
	bool					Load( ChunkLoadClass & cload );

	GameObjReference	MoveObjectRef;
	GameObjReference	AttackObjectRef;
	GameObjReference	LookObjectRef;
	StringClass			SafeAnimationName;
	StringClass			SafeConversationName;
};

/*
**
*/
class	ActionClass {

public:
	ActionClass( SmartGameObj *obj );
	virtual ~ActionClass( void );

	bool				Save( ChunkSaveClass & csave );
	bool				Load( ChunkLoadClass & cload );

	SmartGameObj *	Get_Action_Obj( void )								{ return ActionObj; }
	SafeActionParamsStruct &	Get_Parameters( void )				{ return Parameters; }
   bool				Is_Acting( void );
   bool				Is_Animating( void );
	void				Begin_Hibernation( void );
	void				End_Hibernation( void );

	bool				Reset( float priority );
	bool				Follow_Input( const ActionParamsStruct & parameters );
	bool				Stand( const ActionParamsStruct & parameters );
	bool				Play_Animation( const ActionParamsStruct & parameters );
	bool				Goto( const ActionParamsStruct & parameters );
	bool				Enter_Exit( const ActionParamsStruct & parameters );
	bool				Dive( const ActionParamsStruct & parameters );
	bool				Attack( const ActionParamsStruct & parameters );
	bool				Face_Location( const ActionParamsStruct & parameters );
	bool				Have_Conversation( const ActionParamsStruct & parameters );
	bool				Dock_Vehicle( const ActionParamsStruct & parameters );
	bool				Modify( const ActionParamsStruct & parameters, bool modify_move, bool modify_attack );

	void				Act( void );
	unsigned int	Get_Act_Count( void ) { return ActCount; }
	bool				Is_Active( void );
	bool				Is_Busy( void );

	bool				Is_Paused( void ) const	{ return IsPaused; }
	void				Pause( bool onoff)		{ IsPaused = onoff; }

	void				Done( int reason );												// Notify the obserever

protected:

	void				Notify_Completed( int observer_id, int action_id, int reason );		// Notify the obserever
	bool				Request_Action( ActionCodeClass * action, const ActionParamsStruct & parameters );

	SmartGameObj *				ActionObj;
	ActionCodeClass *			ActionCode;
	SafeActionParamsStruct	Parameters;
	bool							IsPaused;

	unsigned int				ActCount; // TSS - diagnostic

	void				Set_Action_Code( ActionCodeClass * code );

};

// Debugging
void	Toggle_Display_Findpaths( void );


#endif	// ACTION_H

