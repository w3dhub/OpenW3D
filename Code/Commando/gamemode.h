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
 *                     $Archive:: /Commando/Code/Commando/gamemode.h                           $* 
 *                                                                                             * 
 *                      $Author:: Tom_s                                                       $* 
 *                                                                                             * 
 *                     $Modtime:: 5/04/01 7:08p                                               $* 
 *                                                                                             * 
 *                    $Revision:: 13                                                         $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef	GAMEMODE_H
#define	GAMEMODE_H

#ifndef	ALWAYS_H
	#include "always.h"
#endif

#ifndef	VECTOR3_H
	#include "vector3.h"
#endif

#ifndef	WWDEBUG_H
	#include "wwdebug.h"
#endif

/*
** Game modes are objects to manage each of the game's modes and
** sub-modes.  Primarily, each object will provide a Initializing
** routine to allow it to allocate any required resources, a Shutdown
** routine to allow it to free said resources, and a Think routine,
** which will be called each time through the main loop, allowing the
** mode to perform any necessary periodic tasks.
**
** Each mode can be Activated, Deactivated, Suspended, or Resumed
*/

typedef enum {
	GAME_MODE_ACTIVE,
	GAME_MODE_INACTIVE,
	GAME_MODE_INACTIVE_PENDING,
	GAME_MODE_SUSPENDED,
} GameModeState;


class	GameModeClass {

protected:
	GameModeState	State;

public:
	GameModeClass() { State = GAME_MODE_INACTIVE; }	// starts off inactive
	virtual	~GameModeClass() { WWASSERT( State == GAME_MODE_INACTIVE ); }	// assert inactive

	// gets the current state
	GameModeState	Get_State() { 	return State; }
	
	virtual void Activate();				// activates the mode
	virtual void Deactivate();				// deactivates the mode (don't shutdown until safe)
	virtual void Safely_Deactivate();	// shutdown if requested
	virtual void Suspend();					// suspends the mode from thinking, but does not deactivate it
	virtual void Resume();					// resumes a suspended mode

	virtual bool	Is_Inactive( void )	{ return ( State == GAME_MODE_INACTIVE ) || ( State == GAME_MODE_INACTIVE_PENDING ); }
	virtual bool	Is_Suspended( void )	{ return ( State == GAME_MODE_SUSPENDED ); }
	virtual bool	Is_Active( void )		{ return ( State == GAME_MODE_ACTIVE ); }

	// the virtual functions
	virtual	const char *Name() = 0;	// the name of this mode
	virtual	void	Init() = 0;			// called when the mode is activated
	virtual	void 	Shutdown() = 0;	// called when the mode is deactivated
	virtual	void 	Render() = 0;		// called each time through the main loop	to draw if not inactive
	virtual	void 	Think() = 0;		// called each time through the main loop to think if not inactive
};

/*
** Only 1 Major Game mode can be active at any time
*/
class	GameMajorModeClass : public GameModeClass {

public:
	virtual void Activate() override;		// Override Activate and Deactivate to count
	virtual void Deactivate() override;

private:
	static int NumActiveMajorModes;

};



/*
** GameModeManager - an object to maintain a list of all GameModes
*/
class GameModeManager {

public:
	/*
	** Add, Remove, Destroy and count the modes
	*/
	static	GameModeClass *Add( GameModeClass *mode );	// { GameModeList.Add_Tail( mode ); return mode; }
	static	void	Remove( GameModeClass *mode );		//			{ GameModeList.Remove( mode ); }
	static	int	Count();	//										{ return GameModeList.Get_Count(); }
	static	void	Destroy( GameModeClass *mode );
	static	void	Destroy_All( void );

	static	void	List_Active_Game_Modes(void); // diagnostic

	//
	static	void	Set_Background_Color( const Vector3 & color )	{ BackgroundColor = color; }

	/*
	** let all non-inactive, game modes think
	*/
	static	void	Think( void );

	/*
	** let all non-inactive, game modes draw
	*/
	static	void	Render( void );

	/*
	** find a registered game mode by name
	*/
	static	GameModeClass	*Find( const char * name );

   //static bool IsLanNameCollisionDetected;

	static	void	Safely_Deactivate();

	static void Hide_Render_Frames(unsigned frame_count);		// hide rendering for n frames

private:
	static	Vector3	BackgroundColor;
};


#endif	// GAMEMODE_H
