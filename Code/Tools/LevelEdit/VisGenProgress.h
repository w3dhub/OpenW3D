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
 *                 Project Name : LevelEdit                                                    *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/VisGenProgress.h             $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 7/16/00 1:59p                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef VISGENPROGRESS_H
#define VISGENPROGRESS_H

#include "always.h"

/**
** VisGenProgressClass
** This class encapsulates the progress of a vis generation process.
** During the process, an external thread can read the status of the variables
** in this object to update a dialog box.
*/
class VisGenProgressClass
{
public:
	VisGenProgressClass(void);

	int				Get_Node_Count(void)									{ return NodeCount; }
	int				Get_Processed_Node_Count(void)					{ return ProcessedNodeCount; }
	int				Get_Total_Edge_Count(void)							{ return TotalEdgeCount; }
	int				Get_Total_Sample_Count(void)						{ return TotalSampleCount; }
	int				Get_Current_Node_Edge_Count(void)				{ return CurrentNodeEdgeCount; }
	int				Get_Current_Node_Sample_Count(void)				{ return CurrentNodeSampleCount; }
	float				Get_Average_Samples_Per_Node(void);

	void				Set_Node_Count(int count)							{ NodeCount = count; }
	void				Increment_Processed_Node_Count(void)			{ ProcessedNodeCount++; CurrentNodeEdgeCount = CurrentNodeSampleCount = 0; }
	void				Increment_Edge_Count(void)							{ TotalEdgeCount++; CurrentNodeEdgeCount++; }
	void				Increment_Sample_Count(void)						{ TotalSampleCount++; CurrentNodeSampleCount++; }

	void				Request_Cancel(void)									{ CancelRequested = true; }
	bool				Is_Cancel_Requested(void)							{ return CancelRequested; }

protected:

	int				NodeCount;
	int				ProcessedNodeCount;

	int				TotalEdgeCount;
	int				TotalSampleCount;
	int				CurrentNodeEdgeCount;
	int				CurrentNodeSampleCount;

	bool				CancelRequested;
};


inline VisGenProgressClass::VisGenProgressClass(void) :
	NodeCount(0),
	ProcessedNodeCount(0),
	TotalEdgeCount(0),
	TotalSampleCount(0),
	CurrentNodeEdgeCount(0),
	CurrentNodeSampleCount(0),
	CancelRequested(false)
{
}


inline float VisGenProgressClass::Get_Average_Samples_Per_Node(void)
{
	if (ProcessedNodeCount > 0) {
		return (float)TotalSampleCount / (float)ProcessedNodeCount;
	} else {
		return 0.0f;
	}
}

#endif //VISGENPROGRESS_H

