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
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Combat																		  *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Combat/oratortypes.cpp         $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 9/07/01 10:41a                                              $*
 *                                                                                             *
 *                    $Revision:: 7                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "oratortypes.h"
#include "wwhack.h"

DECLARE_FORCE_LINK (OratorTypes);


////////////////////////////////////////////////////////////////
//	Constants
////////////////////////////////////////////////////////////////
const char *ORATOR_TYPE_NAMES[ORATOR_TYPE_COUNT] =
{
	"GEEN: GDI Enlisted Engineer",
	"GEGR: GDI Enlisted Grenadier",
	"GEMG: GDI Enlisted Mini-gunner",
	"GERS: GDI Enlisted Rocket Soldier",
	"GOEN: GDI Officer Engineer Officer",
	"GOGR: GDI Officer Grenadier Officer",
	"GOMG: GDI Officer Mini-gunner Officer",
	"GORS: GDI Officer Rocket Solider Officer",
	"GOLS: GDI Officer Logan Sheppard",
	"GSEN: GDI Special Forces Engineer",
	"GSGR: GDI Special Forces Grenadier",
	"GSMG: GDI Special Forces Mini-gunner",
	"GSRS: GDI Special Forces Rocket soldier",
	"GBMG: GDI Boss Havoc",
	"GBRS: GDI Boss Sydney Mobius",

	"MEIN: Mutant Enlisted Initiate",
	"MOAC: Mutant Officer Acolyte",
	"MSTM: Mutant Special Forces Templar",
	"MBPT: Mutant Boss Mutant Petrova",
	"MBRS: Mutant Boss Mutant Raveshaw",

	"NEEN: Nod Enlisted Engineer",
	"NEFT: Nod Enlisted Flame Thrower",
	"NEMG: Nod Enlisted Mini-gunner",
	"NERS: Nod Enlisted Rocket Soldier",
	"NOEN: Nod Officer Engineer Officer",
	"NOFT: Nod Officer Flame Thrower Officer",
	"NOMG: Nod Officer Mini-gunner Officer",
	"NORS: Nod Officer Rocket Solider Officer",
	"NSCW: Nod Special Forces Chem Warrior",
	"NSEN: Nod Special Forces Engineer",
	"NSMG: Nod Special Forces Mini-gunner",
	"NSRS: Nod Special Forces Rocket Soldier",
	"NSSS: Nod Special Forces Stealth Solder",
	"NBFT: Nod Boss Mendoza",
	"NBRS: Nod Boss Raveshaw",
	"NBMG: Nod Boss Sakura",
	"NOCP: Nod Officer Ship Captain",
	"NEFM: Nod Enlisted First Mate",

	"GCBL: GDI Civilian Brigadier Adam Locke",
	"GCFL: GDI Civilian Female Lieutenant",
	"GCIM: GDI Civilian Ignatio Mobius",
	"GCTK: GDI Civilian Technician",
	"GCMP: GDI Civilian MP's",
	"GCP1: GDI Civilian GDI Prisoner 1",
	"GCP2: GDI Civilian GDI Prisoner 2",
	"GCP3: GDI Civilian GDI Prisoner 3",
	"GCBB: GDI Civilian Resistance Babushka",
	"GCF1: GDI Civilian Resistance Female",
	"GCM1: GDI Civilian Resistance Male 1",
	"GCM2: GDI Civilian Resistance Male 2",
	"GCXN: GDI Civilian Nun",
	"GCXP: GDI Civilian Priest",
	"GCC1: GDI Civilian 1",
	"GCC2: GDI Civilian 2",
	"GCC3: GDI Civilian 3",
	"GCC4: GDI Civilian 4",
	"GCC5: GDI Civilian 5",
	"GCC6: GDI Civilian 6",
	"GCBS: GDI Civilian Brigadier Shepherd",
	"GCLS: GDI Civilian Logan Shepsherd",
	"GCSO: GDI Civilian UN Secretary Generl Charles Ovlivette",

	"NCXK: Civilian Kane",
	"NCTK: Nod Technician",
	"NCXP: Civilian Petrova",
	"NCXS: Civilian Seth",
	"NCGB: Nod Civilian Greg Brudette",
	"NCXT: Nod Civilian Technician",

	"CCFM: Neutral Civilian Farmer",
	"CCNC: Civilian Newscaster",
	"CCSF: Servant, Female",
	"CCSM: Servant, Male",
	"CCCK: Civilian Cook",

	"AVIS: Visceroid",

	"RGS1: GDI Radio Speaker 1",
	"RGS2: GDI Radio Speaker 2",
	"RGS3: GDI Radio Speaker 3",
	"RGS4: GDI Radio Speaker 4",
	"RNS1: Nod Radio Speaker 1",
	"RNS2: Nod Radio Speaker 2",
	"RNS3: Nod Radio Speaker 3",
	"RNS4: Nod Radio Speaker 4",

	"EVAG: EVA",
	"EVAL: Brigadier Adam Locke via EVA",
	"EVAN: Nod Public Announcement Speaker",
	"EVAC: Temple of Nod Computer (malfunctioning)",

	"VNCH: Transport Chinook Helicopter"
};


//////////////////////////////////////////////////////////////////////////////////
//
//	Get_Count
//
//////////////////////////////////////////////////////////////////////////////////
int
OratorTypeClass::Get_Count (void)
{
	return ORATOR_TYPE_COUNT;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Get_ID
//
//////////////////////////////////////////////////////////////////////////////////
int
OratorTypeClass::Get_ID (int index)
{
	int retval = 0;

	if (index >= 0 && index < ORATOR_TYPE_COUNT) {
		retval = ORATOR_TYPE_START + index;
	}

	return retval;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Get_Description
//
//////////////////////////////////////////////////////////////////////////////////
const char *
OratorTypeClass::Get_Description (int index)
{
	const char *retval = 0;

	if (index >= 0 && index < ORATOR_TYPE_COUNT) {
		retval = ORATOR_TYPE_NAMES[index];
	}

	return retval;
}
