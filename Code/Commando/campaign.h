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
 *                     $Archive:: /Commando/Code/Commando/campaign.h                          $*
 *                                                                                             *
 *                      $Author:: Byon_g                                                      $*
 *                                                                                             *
 *                     $Modtime:: 12/01/01 12:02p                                             $*
 *                                                                                             *
 *                    $Revision:: 8                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef	CAMPAIGN_H
#define	CAMPAIGN_H

#ifndef	ALWAYS_H
	#include "always.h"
#endif

class	ChunkSaveClass;
class	ChunkLoadClass;

/*
** CampaignManager is responsible for controlling the flow of missions and sub-states through
** the playing of a campaign.  It also is the main entry point for single mission, and multi-play sessions.
*/
class	CampaignManager {

public:
	static	void	Init( void );
	static	void	Shutdown( void );

	static	bool	Save(ChunkSaveClass & csave);
	static	bool	Load(ChunkLoadClass &cload);

	static	void	Start_Campaign( int difficulty );
	static	void	Continue( bool success = true );

	static	void	Reset();

	// Backdrop Descriptions
	static	int	Get_Backdrop_Description_Count( void );
	static	const char * Get_Backdrop_Description( int index );

	static	void	Select_Backdrop_Number( int state_number );
	static	void	Select_Backdrop_Number_By_MP_Type( int type );

	static	void	Replay_Level( const char * mission_name, int difficulty );

private:
	static	int	State;
	static	int	BackdropIndex;
};

#endif
