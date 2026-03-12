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
 *                     $Archive:: /Commando/Code/Commando/sbbomanager.h                           $*
 *                                                                                             *
 *                      $Author:: Tom_s                                                   $*
 *                                                                                             *
 *                     $Modtime:: 10/24/01 1:52p                                              $*
 *                                                                                             *
 *                    $Revision:: 3                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef __SBBOMANAGER_H__
#define __SBBOMANAGER_H__


//-----------------------------------------------------------------------------
//
// Manager for server bandwidth budget out ("sbbo")
//
class	cSbboManager
{
public:
	static void		Reset(void);
	static void		Think(void);
	static void		Increment_Accum_Time_S_Net_Update(float time_increment_s);
	static void		Increment_Accum_Time_S_Combat_Think(float time_increment_s);
	static float	Get_Net_To_Combat_Ratio(void);
	static bool		Toggle_Is_Enabled(void);

private:

	static float	AccumTimeSNetUpdate;
	static float	AccumTimeSCombatThink;
	static float	NetToCombatRatio;
	static int		PoorRatios;
	static int		SlowSamples;
	static bool		IsEnabled;
};

//-----------------------------------------------------------------------------

#endif	// __SBBOMANAGER_H__
