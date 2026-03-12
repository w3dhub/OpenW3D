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
 *                     $Archive:: /Commando/Code/wwphys/lightsolveprogress.h                  $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 3/01/02 10:20a                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef LIGHTSOLVEPROGRESS_H
#define LIGHTSOLVEPROGRESS_H

/**
** LightSolveProgressClass
** This class is used to communicate the current status of the light solve
*/
class LightSolveProgressClass
{
public:
	LightSolveProgressClass(void);

	/*
	** Read status (UI uses this to provide progress display)
	*/
	int				Get_Object_Count(void)								{ return ObjectCount; }
	int				Get_Processed_Object_Count(void)					{ return ProcessedObjectCount; }

	const char *	Get_Current_Mesh_Name(void)						{ return CurrentMeshName; }
	int				Get_Current_Mesh_Vertex_Count(void)				{ return CurrentVertexCount; }
	int				Get_Current_Vertex(void)							{ return CurrentVertex; }

	/*
	** Set status (vertex solver does this)
	*/
	void				Set_Object_Count(int count)						{ ObjectCount = count; }
	void				Increment_Processed_Object_Count(void)			{ ProcessedObjectCount++; }

	void				Set_Current_Mesh_Name(const char * name)		{ CurrentMeshName = name; }
	void				Set_Current_Mesh_Vertex_Count(int vcount)		{ CurrentVertexCount = vcount; }
	void				Set_Current_Vertex(int index)						{ CurrentVertex = index; }

	/*
	** Cancellation request
	*/
	void				Request_Cancel(void)									{ CancelRequested = true; }
	bool				Is_Cancel_Requested(void)							{ return CancelRequested; }

protected:

	int				ObjectCount;
	int				ProcessedObjectCount;

	const char *	CurrentMeshName;
	int				CurrentVertexCount;
	int				CurrentVertex;

	bool				CancelRequested;
};

inline LightSolveProgressClass::LightSolveProgressClass(void) :
	ObjectCount(0),
	ProcessedObjectCount(0),
	CurrentMeshName(NULL),
	CurrentVertexCount(0),
	CurrentVertex(0),
	CancelRequested(false)
{
}


#endif
