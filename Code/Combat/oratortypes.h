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
 *                     $Archive:: /Commando/Code/Combat/oratortypes.h         $*
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

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __ORATORTYPES_H
#define __ORATORTYPES_H


////////////////////////////////////////////////////////////////
//	Constants
////////////////////////////////////////////////////////////////
typedef enum
{
	ORATOR_TYPE_START		= 1000,

	ORATOR_GEEN				= ORATOR_TYPE_START,
	ORATOR_GEGR,
	ORATOR_GEMG,
	ORATOR_GERS,
	ORATOR_GOEN,
	ORATOR_GOGR,
	ORATOR_GOMG,
	ORATOR_GORS,
	ORATOR_GOLS,
	ORATOR_GSEN,
	ORATOR_GSGR,
	ORATOR_GSMG,
	ORATOR_GSRS,
	ORATOR_GBMG,
	ORATOR_GBRS,

	ORATOR_MEIN,
	ORATOR_MOAC,
	ORATOR_MSTM,
	ORATOR_MBPT,
	ORATOR_MBRS,

	ORATOR_NEEN,
	ORATOR_NEFT,
	ORATOR_NEMG,
	ORATOR_NERS,
	ORATOR_NOEN,
	ORATOR_NOFT,
	ORATOR_NOMG,
	ORATOR_NORS,
	ORATOR_NSCW,
	ORATOR_NSEN,
	ORATOR_NSMG,
	ORATOR_NSRS,
	ORATOR_NSSS,
	ORATOR_NBFT,
	ORATOR_NBRS,
	ORATOR_NBMG,
	ORATOR_NOCP,
	ORATOR_NEFM,

	ORATOR_GCBL,
	ORATOR_GCFL,
	ORATOR_GCIM,
	ORATOR_GCTK,
	ORATOR_GCMP,
	ORATOR_GCP1,
	ORATOR_GCP2,
	ORATOR_GCP3,
	ORATOR_GCBB,
	ORATOR_GCF1,
	ORATOR_GCM1,
	ORATOR_GCM2,
	ORATOR_GCXN,
	ORATOR_GCXP,
	ORATOR_GCC1,
	ORATOR_GCC2,
	ORATOR_GCC3,
	ORATOR_GCC4,
	ORATOR_GCC5,
	ORATOR_GCC6,
	ORATOR_GCBS,
	ORATOR_GCLS,
	ORATOR_GCSO,

	ORATOR_NCXK,
	ORATOR_NCTK,
	ORATOR_NCXP,
	ORATOR_NCXS,
	ORATOR_NCGB,
	ORATOR_NCXT,

	ORATOR_CCFM,
	ORATOR_CCNC,
	ORATOR_CCSF,
	ORATOR_CCSM,
	ORATOR_CCCK,

	ORATOR_AVIS,

	ORATOR_RGS1,
	ORATOR_RGS2,
	ORATOR_RGS3,
	ORATOR_RGS4,
	ORATOR_RNS1,
	ORATOR_RNS2,
	ORATOR_RNS3,
	ORATOR_RNS4,

	ORATOR_EVAG,
	ORATOR_EVAL,
	ORATOR_EVAN,
	ORATOR_EVAC,

	ORATOR_VNCH,

	ORATOR_TYPE_MAX,

	ORATOR_TYPE_COUNT				= ORATOR_TYPE_MAX - ORATOR_TYPE_START,

}	ORATOR_TYPE;


////////////////////////////////////////////////////////////////
//	Strings
////////////////////////////////////////////////////////////////
extern const char *ORATOR_TYPE_NAMES[ORATOR_TYPE_COUNT];

//////////////////////////////////////////////////////////////////////////////////
//
//	OratorTypeClass
//
//////////////////////////////////////////////////////////////////////////////////
class OratorTypeClass
{
public:

	//////////////////////////////////////////////////////////////////////////////////
	//	Static methods
	//////////////////////////////////////////////////////////////////////////////////
	static int				Get_Count (void);
	static int				Get_ID (int index);
	static const char *	Get_Description (int index);

private:

	//////////////////////////////////////////////////////////////////////////////////
	//	Static member data
	//////////////////////////////////////////////////////////////////////////////////
};


#endif //__ORATORTYPES_H
