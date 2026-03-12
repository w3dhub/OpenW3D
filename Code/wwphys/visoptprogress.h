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
 *                 Project Name : WWPhys                                                       *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwphys/visoptprogress.h                      $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 7/07/00 7:44p                                               $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef VISOPTPROGRESS_H
#define VISOPTPROGRESS_H

#include "always.h"


/**
** VisOptProgressClass
** This class encapsulates the progress and final statistics for a vis optimization process.
** During the optimization process, an external thread can read the TotalOperations and
** CompletedOperations counters (using the accessor functions) to update a progress meter
** (in a separate thread).  After the process is complete, the other statistics will be
** valid.
**
** Note that the TotalOperations count is an upper estimate and the process may (will usually)
** complete before Completed_Operation_Count reaches it.
*/
class VisOptProgressClass
{
public:
	VisOptProgressClass(void);

	/*
	** Initialization and update
	*/
	void						Reset(int total_operation_count);
	void						Increment_Completed_Operations(int count = 1)	{ CompletedOps += count; }

	/*
	** Accessors
	*/
	int						Get_Total_Operation_Count(void)				{ return TotalOps; }
	int						Get_Completed_Operation_Count(void)			{ return CompletedOps; }

	int						Get_Initial_Bit_Count(void)					{ return InitialBitCount; }
	int						Get_Final_Bit_Count(void)						{ return FinalBitCount; }
	int						Get_Initial_Sector_Count(void)				{ return InitialSectorCount; }
	int						Get_Final_Sector_Count(void)					{ return FinalSectorCount; }
	int						Get_Initial_Object_Count(void)				{ return InitialObjectCount; }
	int						Get_Final_Object_Count(void)					{ return FinalObjectCount; }
	int						Get_Initial_Dynamic_Cell_Count(void)		{ return InitialDynamicCellCount; }
	int						Get_Final_Dynamic_Cell_Count(void)			{ return FinalDynamicCellCount; }
	int						Get_Dynamic_Cells_Removed(void)				{ return DynCellsRemoved; }
	int						Get_Objects_Merged(void)						{ return ObjectsMerged; }
	int						Get_Sectors_Merged(void)						{ return SectorsMerged; }
	int						Get_Total_Bits_Removed(void)					{ return InitialBitCount - FinalBitCount; }

	/*
	** Control
	*/
	void 						Set_Initial_Bit_Count(int count)				{ InitialBitCount = count; }
	void 						Set_Final_Bit_Count(int count)				{ FinalBitCount = count; }
	void 						Set_Initial_Sector_Count(int count)			{ InitialSectorCount = count; }
	void 						Set_Final_Sector_Count(int count)			{ FinalSectorCount = count; }
	void 						Set_Initial_Object_Count(int count)			{ InitialObjectCount = count; }
	void 						Set_Final_Object_Count(int count)			{ FinalObjectCount = count; }
	void 						Set_Initial_Dynamic_Cell_Count(int count)	{ InitialDynamicCellCount = count; }
	void 						Set_Final_Dynamic_Cell_Count(int count)	{ FinalDynamicCellCount = count; }
	void 						Increment_Dynamic_Cells_Removed(int count=1){ DynCellsRemoved+=count; }
	void						Increment_Objects_Merged(int count=1)		{ ObjectsMerged+=count; }
	void						Increment_Sectors_Merged(int count=1)		{ SectorsMerged+=count; }

protected:

	int						TotalOps;
	int						CompletedOps;

	int						InitialBitCount;									// Initial number of bits in the vis system
	int						FinalBitCount;										// Final number of bits in the vis system
	int						InitialSectorCount;								// Initial number of sectors
	int						FinalSectorCount;									// Final number of sectors
	int						InitialObjectCount;								// initial number of objects
	int						FinalObjectCount;									// Final number of objects
	int						InitialDynamicCellCount;						// Initial number of dynamic objects
	int						FinalDynamicCellCount;							// Final number of dynamic objects

	int						DynCellsRemoved;									// Number of dynamic object cells optimized away
	int						ObjectsMerged;										// Number of arbitrary vis objects merged
	int						SectorsMerged;										// Number of arbitrary vis sectors merged

};



#endif //VISOPTPROGRESS_H

