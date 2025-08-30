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
 *                     $Archive:: /Commando/Code/commando/movie.h                             $* 
 *                                                                                             * 
 *                      $Author:: Patrick                                                     $* 
 *                                                                                             * 
 *                     $Modtime:: 1/03/02 11:02a                                              $* 
 *                                                                                             * 
 *                    $Revision:: 7                                                           $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef	MOVIE_H
#define	MOVIE_H

#ifndef	ALWAYS_H
	#include "always.h"
#endif

#ifndef	GAMEMODE_H
	#include "gamemode.h"
#endif

#ifndef	QUAT_H
	#include "quat.h"
#endif

#ifndef	VECTOR3_H
	#include "vector3.h"
#endif

#include "cdverify.h"
#include "wwstring.h"


// Game Mode to do movies
class	MovieGameModeClass : public GameMajorModeClass, public Observer<CDVerifyEvent>
{
public:
	virtual	const char *Name() override	{ return "Movie"; }	// the name of this mode
	virtual	void	Init() override;		 	// called when the mode is activated
	virtual	void 	Shutdown() override; 	// called when the mode is deactivated
	virtual	void 	Think() override;			// called each time through the main loop
	virtual	void 	Render() override;			// called each time through the main loop

	void	Start_Movie( const char * filename );

	void	Startup_Movies( void );
	void	Movie_Done( void );

	//
	//	Notification
	//
	void		HandleNotification (CDVerifyEvent &event) override;

protected:
	void	Play_Movie( const char * filename );

	CDVerifyClass		CDVerifier;
	StringClass			PendingMovieFilename;
	bool					IsPending;
	bool					IsPlaying;
};

#endif //	MOVIE_H
