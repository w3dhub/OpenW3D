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
 *                     $Archive:: /Commando/Code/Commando/bandwidth.h                              $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 12/17/01 12:30p                                             $*
 *                                                                                             *
 *                    $Revision:: 6                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSV_VER)
#pragma once
#endif

#ifndef BANDWIDTH_H
#define BANDWIDTH_H

#include "bittype.h"
#include "unichar.h"

enum BANDWIDTH_TYPE_ENUM {

	BANDWIDTH_FIRST = 100,

	BANDWIDTH_MODEM_288 = BANDWIDTH_FIRST,
	BANDWIDTH_MODEM_336,
	BANDWIDTH_MODEM_56,
	BANDWIDTH_ISDN,
	BANDWIDTH_CABLE,
	BANDWIDTH_LANT1,
	BANDWIDTH_AUTO,
	BANDWIDTH_CUSTOM,

	BANDWIDTH_LAST = BANDWIDTH_CUSTOM
};

//
//
//
class	cBandwidth {
	public:
      cBandwidth(void);
      ~cBandwidth(void) {};

		static unsigned int						Get_Bandwidth_Bps_From_Type(BANDWIDTH_TYPE_ENUM bandwidth_type);
		static const unichar_t *	Get_Bandwidth_String_From_Type(BANDWIDTH_TYPE_ENUM bandwidth_type);
		static BANDWIDTH_TYPE_ENUM		Get_Bandwidth_Type_From_String(const char *bandwidth_string);
};

#endif	// BANDWIDTH_H
