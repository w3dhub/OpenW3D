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
 *                     $Archive:: /Commando/Code/Commando/console.h                           $* 
 *                                                                                             * 
 *                      $Author:: Jani_p                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 7/25/01 2:12p                                               $* 
 *                                                                                             * 
 *                    $Revision:: 12                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef	CONSOLE_H
#define	CONSOLE_H

#ifndef	ALWAYS_H
	#include "always.h"
#endif

#ifndef	GAMEMODE_H
	#include "gamemode.h"
#endif

#include <cstddef>

#ifndef	BITTYPE_H
	#include "bittype.h"
#endif


class WWProfileIterator;



#define	Get_Console()		ConsoleGameModeClass::Get_Instance()

/*
** Game (Sub) Mode to display console & fps
*/
class	ConsoleGameModeClass : public GameModeClass {
public:
	
	virtual	const char *Name() override	{ return "Console"; }	// the name of this mode
	virtual	void	Init() override;		 	// called when the mode is activated
	virtual	void 	Shutdown() override; 	// called when the mode is deactivated
	virtual	void 	Think() override;			// called each time through the main loop
	virtual	void 	Render() override {}		// called each time through the main loop

	void 		Parse_Input( char * string );
	//void 		Toggle_FPS( void )	{ FPSActive = !FPSActive; }
	bool		Is_FPS_Active( void )	{ return FPSActive; }
	void		Set_FPS_Active( bool onoff )	{ FPSActive = onoff; }

	void		Toggle_Player_Position( void )							{ ShowPlayerPosition = !ShowPlayerPosition; }
	void		Set_Player_Position( Vector3 & pos, float facing )	{ PlayerPosition = pos; PlayerFacing = facing; }

	void		Profile_Command( const char * command );

	static	ConsoleGameModeClass * Get_Instance() { return Instance; }

	static void Load_Registry_Keys(void);
	static void Save_Registry_Keys(void);

private:

	void		Clear_Suggestion(void);
	void		Accept_Suggestion(char * cmd);
	void		Update_Suggestion(char * cmd,bool go_to_next);
	
	enum { 
		MAX_INPUT_LINE_LENGTH		= 100,
		MAX_DRAWER_CHARS				= 4000,
	};

	char							InputLine[MAX_INPUT_LINE_LENGTH];
	char							HelpLine[MAX_INPUT_LINE_LENGTH];
	char							Suggestion[MAX_INPUT_LINE_LENGTH];
	bool							InputActive;

	bool	FPSActive;
	int	FPSFrames;
	float FPSTime;
	float FPSLastTime;
	float FPS;

	bool		ShowPlayerPosition;
	Vector3	PlayerPosition;
	float		PlayerFacing;

	// Profile Display
	WWProfileIterator	* ProfileIterator;

	void		Update_Profile( void );
	void		Begin_Profile_Log( void );
	void		End_Profile_Log( void );
	void		Process_Profile_Log( void );
	void		Update_Memory_Log( void );

	bool	PerformanceSamplingActive;
   DWORD ConsoleInputType;
   size_t   PromptLength;
   static const float LeftMargin;

	static ConsoleGameModeClass * Instance;
};



#endif
