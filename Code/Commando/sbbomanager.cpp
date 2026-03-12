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
 *                     $Archive:: /Commando/Code/Commando/sbbomanager.cpp                    $*
 *                                                                                             *
 *                      $Author:: Tom_s                                                   $*
 *                                                                                             *
 *                     $Modtime:: 11/27/01 12:13p                                             $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "sbbomanager.h"

#include "debug.h"
#include "cnetwork.h"

//
// Class statics
//
float		cSbboManager::AccumTimeSNetUpdate		= 0;
float		cSbboManager::AccumTimeSCombatThink		= 0;
float		cSbboManager::NetToCombatRatio			= 0;
int		cSbboManager::PoorRatios					= 0;
int		cSbboManager::SlowSamples					= 0;
bool		cSbboManager::IsEnabled						= true;

//-----------------------------------------------------------------------------
void
cSbboManager::Reset
(
	void
)
{
	WWASSERT(cNetwork::I_Am_Server());

	AccumTimeSNetUpdate		= 0;
	AccumTimeSCombatThink	= 0;
	NetToCombatRatio			= 0;
	PoorRatios					= 0;
	SlowSamples					= 0;
}

//-----------------------------------------------------------------------------
void
cSbboManager::Think
(
	void
)
{
	//
	// This function reduces server bandwidth out if the framerate is low and we are
	// spending way too much time doing network updates.
	//

	if (!IsEnabled)
	{
		return;
	}

	WWASSERT(cNetwork::I_Am_Server());

	float total_time = AccumTimeSNetUpdate + AccumTimeSCombatThink;

	if (AccumTimeSCombatThink > 0 && total_time > 2)
	{
		NetToCombatRatio = AccumTimeSNetUpdate / AccumTimeSCombatThink;
		AccumTimeSNetUpdate = 0;
		AccumTimeSCombatThink = 0;

		if (NetToCombatRatio > 5) {
			PoorRatios++;
		} else {
			PoorRatios = 0;
		}

		if (cNetwork::Get_Fps() < 20) {
			SlowSamples++;
		} else {
			SlowSamples = 0;
		}

		if (SlowSamples >= 10 && PoorRatios >= 10) {

			ULONG sbbo = cNetwork::PServerConnection->Get_Bandwidth_Budget_Out();
			if (sbbo >= 64000)
			{
				sbbo *= 0.90;
				cNetwork::PServerConnection->Set_Bandwidth_Budget_Out(sbbo);
				Debug_Say(("cSbboManager::Think: reducing sbbo to %d\n", sbbo));
				SlowSamples = 0;
				PoorRatios = 0;
			}
		}
	}
}

//-----------------------------------------------------------------------------
void
cSbboManager::Increment_Accum_Time_S_Net_Update
(
	float time_increment_s
)
{
	WWASSERT(cNetwork::I_Am_Server());
	WWASSERT(time_increment_s >= 0);

	AccumTimeSNetUpdate += time_increment_s;
}

//-----------------------------------------------------------------------------
void
cSbboManager::Increment_Accum_Time_S_Combat_Think
(
	float time_increment_s
)
{
	WWASSERT(cNetwork::I_Am_Server());
	WWASSERT(time_increment_s >= 0);

	AccumTimeSCombatThink += time_increment_s;
}

//-----------------------------------------------------------------------------
float
cSbboManager::Get_Net_To_Combat_Ratio
(
	void
)
{
	WWASSERT(cNetwork::I_Am_Server());

	return NetToCombatRatio;
}

//-----------------------------------------------------------------------------
bool
cSbboManager::Toggle_Is_Enabled
(
	void
)
{
	IsEnabled = !IsEnabled;

	Reset();

	return IsEnabled;
}
