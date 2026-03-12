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
 *                 Project Name : WWPHys                                                       *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwphys/visoptprogress.cpp                    $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 7/07/00 5:36p                                               $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "visoptprogress.h"


/************************************************************************************
**
** VisOptProgressClass Implementation
**
************************************************************************************/

VisOptProgressClass::VisOptProgressClass(void)
{
	Reset(0);
}

void VisOptProgressClass::Reset(int total_operation_count)
{
	TotalOps = total_operation_count;
	CompletedOps = 0;

	InitialBitCount = 0;
	FinalBitCount = 0;
	InitialSectorCount = 0;
	FinalSectorCount = 0;
	InitialObjectCount = 0;
	FinalObjectCount = 0;
	InitialDynamicCellCount = 0;
	FinalDynamicCellCount = 0;
	DynCellsRemoved = 0;
	ObjectsMerged = 0;
	SectorsMerged = 0;
}




