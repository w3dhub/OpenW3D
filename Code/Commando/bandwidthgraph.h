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
 *                     $Archive:: /Commando/Code/Commando/bandwidthgraph.h                              $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 10/26/01 12:45p                                             $*
 *                                                                                             *
 *                    $Revision:: 5                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef __BANDWIDTHGRAPH_H__
#define __BANDWIDTHGRAPH_H__

#include "bittype.h"
#include "wwstring.h"

class Render2DTextClass;
class Font3DInstanceClass;

//-----------------------------------------------------------------------------
class	cBandwidthGraph
{
public:
	static void							Onetime_Init(void);
	static void							Onetime_Shutdown(void);

	static void							Think(void);
	static void							Render(void);

	static void							Set_Scale(int scale) {BandwidthScaler = scale;}

private:
	static void							Bandwidth_Graph(StringClass &	label, int bps,
												int target_bps, float bandwidth_multiplier, float average_priority, bool is_loading);

	static Render2DTextClass	*	PTextRenderer;
	static Font3DInstanceClass *	PFont;
	static int							BandwidthScaler;
	static float						YPosition;
	static float						BarHeight;
	static float						BarWidth;
	static float						YIncrement;
};

//-----------------------------------------------------------------------------

#endif	// __BANDWIDTHGRAPH_H__
