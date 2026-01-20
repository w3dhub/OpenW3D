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
 *                     $Archive:: /Commando/Code/Combat/gameobjobserver.h                     $* 
 *                                                                                             * 
 *                      $Author:: Byon_g                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 11/29/01 11:03a                                             $* 
 *                                                                                             * 
 *                    $Revision:: 30                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef	GAMEOBJOBSERVER_H
#define	GAMEOBJOBSERVER_H

#ifndef	ALWAYS_H
	#include "always.h"
#endif

#include <cstdint>

class		ScriptableGameObj;
typedef	ScriptableGameObj	 GameObject;
class		CombatSound;
class		ChunkSaveClass;
class		ChunkLoadClass;

typedef enum {
	ACTION_COMPLETE_NORMAL,
	ACTION_COMPLETE_LOW_PRIORITY,
	ACTION_COMPLETE_PATH_BAD_START,
	ACTION_COMPLETE_PATH_BAD_DEST,
	ACTION_COMPLETE_MOVE_NO_PROGRESS_MADE,
	ACTION_COMPLETE_ATTACK_OUT_OF_RANGE,

	// Conversation support
	ACTION_COMPLETE_CONVERSATION_ENDED,
	ACTION_COMPLETE_CONVERSATION_INTERRUPTED,
	ACTION_COMPLETE_CONVERSATION_UNABLE_TO_INIT,

	MOVEMENT_COMPLETE_ARRIVED,		// TEMP
} ActionCompleteReason;

enum
{
	CUSTOM_EVENT_SYSTEM_FIRST						= 1000000000,
	CUSTOM_EVENT_SOUND_ENDED,
	CUSTOM_EVENT_BUILDING_POWER_CHANGED,
	CUSTOM_EVENT_DOCK_BACKING_IN,
	CUSTOM_EVENT_CINEMATIC_SET_FIRST_SLOT,
	CUSTOM_EVENT_CINEMATIC_SET_LAST_SLOT		= CUSTOM_EVENT_CINEMATIC_SET_FIRST_SLOT + 20,
	CUSTOM_EVENT_POWERUP_GRANTED,
	CUSTOM_EVENT_BUILDING_DAMAGED,
	CUSTOM_EVENT_BUILDING_REPAIRED,
	CUSTOM_EVENT_VEHICLE_ENTERED,
	CUSTOM_EVENT_VEHICLE_EXITED,
	CUSTOM_EVENT_ATTACK_ARRIVED,

	CUSTOM_EVENT_CONVERSATION_BEGAN,
	CUSTOM_EVENT_CONVERSATION_REMARK_STARTED,
	CUSTOM_EVENT_CONVERSATION_REMARK_ENDED,

	CUSTOM_EVENT_LADDER_OCCUPIED,
	CUSTOM_EVENT_FALLING_DAMAGE
};

#define	MovementCompleteReason		ActionCompleteReason		// TEMP

/*
** GameObjObserverClass
*/
class	GameObjObserverClass {

public:
	GameObjObserverClass( void ) : ID( 0 ) {}
	virtual	~GameObjObserverClass( void ) {}

	virtual	const char * Get_Name( void )													= 0;

	void			 Set_ID( int id )																{ ID = id; }
	int			 Get_ID( void )																{ return ID; }

	virtual	void	Attach( GameObject * obj )												= 0;
	virtual	void	Detach( GameObject * obj )												= 0;

	// Event Functions which will be called as events happen
	virtual	void	Created( GameObject * obj )											= 0;
	virtual	void	Destroyed( GameObject * obj )											= 0;
	virtual	void	Killed( GameObject * obj, GameObject * killer )					= 0;
	virtual	void	Damaged( GameObject * obj, GameObject * damager, float amount )	= 0;
	virtual	void	Custom( GameObject * obj, int type, intptr_t param, GameObject * sender )	= 0;
	virtual	void	Sound_Heard( GameObject * obj, const CombatSound & sound )	= 0;
	virtual	void	Enemy_Seen( GameObject * obj, GameObject * enemy )				= 0;
	virtual	void	Action_Complete( GameObject * obj, int action_id, ActionCompleteReason complete_reason )	= 0;
	virtual	void	Timer_Expired( GameObject * obj, int timer_id )					= 0;
	virtual	void	Animation_Complete( GameObject * obj, const char * animation_name )	= 0;
	virtual	void	Poked( GameObject * obj, GameObject * poker )					= 0;
	virtual	void	Entered( GameObject * obj, GameObject * enterer )				= 0;
	virtual	void	Exited( GameObject * obj, GameObject * exiter )					= 0;

protected:
	int		ID;
};

/*
**
*/
class	GameObjObserverManager {
public:

	static	void	Delete_Register( GameObjObserverClass * observer );
	static	void	Delete_Pending( void );

	static	bool	Save( ChunkSaveClass & csave );
	static	bool	Load( ChunkLoadClass & cload );

	static	void	Reset( void )							{ NextID = 6000000; }
	static	int	Get_Next_Observer_ID( void )		{ return NextID++; }

private:
	static	int	NextID;

};

#endif		//	GAMEOBJOBSERVER_H
