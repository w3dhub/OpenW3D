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

///////////////////////////////////////////////////////////////////////////////////////
//
//	SelectionBoxClass.cpp
//
//	Class declaration for a selected item's bounding box.
//


#include "StdAfx.h"
#include "selectionbox.h"
#include "matrix3d.h"
#include "sceneeditor.h"
#include "rendobj.h"
#include "node.h"
#include <algorithm>


//////////////////////////////////////////////////////////////////////
//
//	SelectionBoxClass::DecorationLineClass
//
//////////////////////////////////////////////////////////////////////
SelectionBoxClass::DecorationLineClass::DecorationLineClass (void)
	: m_pLine3D (NULL)
{
	m_pLine3D = new Line3DClass (Vector3 (0,0,0), Vector3 (10,10,10), 0.3F, 0.8F, 0.8F, 0.8F);

	// Don't do collision detections on us
	Inc_Ignore_Counter();

	// Let the base class know who the model is
	Set_Model (m_pLine3D);
	return ;
}

//////////////////////////////////////////////////////////////////////
//
//	SelectionBoxClass::DecorationLineClass::~DecorationLineClass
//
//////////////////////////////////////////////////////////////////////
SelectionBoxClass::DecorationLineClass::~DecorationLineClass (void)
{
	MEMBER_RELEASE (m_pLine3D);
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	SelectionBoxClass
//
//////////////////////////////////////////////////////////////////////
SelectionBoxClass::SelectionBoxClass (void)
	: m_bIsAddedToScene (false)
{
	//
	// Loop through and create all the line segments
	//
	for (int line = 0; line < 24; line ++) {
		m_pBoundingLines[line] = new SelectionBoxClass::DecorationLineClass;
	}

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	~SelectionBoxClass
//
//////////////////////////////////////////////////////////////////////
SelectionBoxClass::~SelectionBoxClass (void)
{
	// Remove this item from the scene
	Remove_From_Scene ();

	// Loop through and release all the line segments
	for (int line = 0; line < 24; line ++) {
		MEMBER_RELEASE (m_pBoundingLines[line]);
	}

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Display_Around_Node
//
//////////////////////////////////////////////////////////////////////
void
SelectionBoxClass::Display_Around_Node (const NodeClass &node)
{
	RenderObjClass *render_obj = node.Peek_Render_Obj ();
	if (render_obj != NULL) {
		Display_Around_Node (*render_obj);
	}

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Display_Around_Node
//
//////////////////////////////////////////////////////////////////////
void
SelectionBoxClass::Display_Around_Node (const RenderObjClass &render_obj)
{
	// Get the bounding box of the item
	AABoxClass bounding_box;
	render_obj.Get_Obj_Space_Bounding_Box (bounding_box);
	const Matrix3D &transform = render_obj.Get_Transform ();
	
	// Alias the extent vector
	Vector3 extent = bounding_box.Extent;
	extent.X += extent.X / 50.0F;
	extent.Y += extent.Y / 50.0F;
	extent.Z += extent.Z / 50.0F;

	//
	// Calculate the positions of the eight verticies
	//
	Vector3 verticies[8];
	verticies[0] = bounding_box.Center + Vector3 (extent.X, extent.Y, extent.Z);
	verticies[1] = bounding_box.Center + Vector3 (-extent.X, extent.Y, extent.Z);
	verticies[2] = bounding_box.Center + Vector3 (-extent.X, extent.Y, -extent.Z);
	verticies[3] = bounding_box.Center + Vector3 (extent.X, extent.Y, -extent.Z);

	verticies[4] = bounding_box.Center + Vector3 (extent.X, -extent.Y, extent.Z);
	verticies[5] = bounding_box.Center + Vector3 (-extent.X, -extent.Y, extent.Z);
	verticies[6] = bounding_box.Center + Vector3 (-extent.X, -extent.Y, -extent.Z);
	verticies[7] = bounding_box.Center + Vector3 (extent.X, -extent.Y, -extent.Z);

	//
	// Rotate these verticies using the item's current transform
	//
	for (int vertex = 0; vertex < 8; vertex ++) {		
		verticies[vertex] = transform * verticies[vertex];
	}

	//
	// Determine the new size for each grab handle
	//
	float largest_dim = std::max (extent.X, extent.Y);
	largest_dim			= std::max (largest_dim, extent.Z);
	float width			= std::max (largest_dim / 55.0F, 0.02F);

	//
	//	Set the positions of the line segments that make up the box
	//
	m_pBoundingLines[0]->Reset (verticies[0], verticies[0] + (verticies[1]-verticies[0])/4.00F, width);
	m_pBoundingLines[4]->Reset (verticies[0], verticies[0] + (verticies[4]-verticies[0])/4.00F, width);
	m_pBoundingLines[12]->Reset (verticies[0], verticies[0] + (verticies[3]-verticies[0])/4.00F, width);

	m_pBoundingLines[1]->Reset (verticies[1], verticies[1] + (verticies[2]-verticies[1])/4.00F, width);
	m_pBoundingLines[5]->Reset (verticies[1], verticies[1] + (verticies[5]-verticies[1])/4.00F, width);
	m_pBoundingLines[13]->Reset (verticies[1], verticies[1] + (verticies[0]-verticies[1])/4.00F, width);
	

	m_pBoundingLines[2]->Reset (verticies[2], verticies[2] + (verticies[3]-verticies[2])/4.00F, width);
	m_pBoundingLines[6]->Reset (verticies[2], verticies[2] + (verticies[6]-verticies[2])/4.00F, width);
	m_pBoundingLines[14]->Reset (verticies[2], verticies[2] + (verticies[1]-verticies[2])/4.00F, width);

	m_pBoundingLines[3]->Reset (verticies[3], verticies[3] + (verticies[0]-verticies[3])/4.00F, width);			
	m_pBoundingLines[7]->Reset (verticies[3], verticies[3] + (verticies[7]-verticies[3])/4.00F, width);
	m_pBoundingLines[15]->Reset (verticies[3], verticies[3] + (verticies[2]-verticies[3])/4.00F, width);


	m_pBoundingLines[8]->Reset (verticies[4], verticies[4] + (verticies[5]-verticies[4])/4.00F, width);
	m_pBoundingLines[16]->Reset (verticies[4], verticies[4] + (verticies[0]-verticies[4])/4.00F, width);
	m_pBoundingLines[17]->Reset (verticies[4], verticies[4] + (verticies[7]-verticies[4])/4.00F, width);

	m_pBoundingLines[9]->Reset (verticies[5], verticies[5] + (verticies[6]-verticies[5])/4.00F, width);
	m_pBoundingLines[18]->Reset (verticies[5], verticies[5] + (verticies[4]-verticies[5])/4.00F, width);
	m_pBoundingLines[19]->Reset (verticies[5], verticies[5] + (verticies[1]-verticies[5])/4.00F, width);
	

	m_pBoundingLines[10]->Reset (verticies[6], verticies[6] + (verticies[7]-verticies[6])/4.00F, width);
	m_pBoundingLines[20]->Reset (verticies[6], verticies[6] + (verticies[5]-verticies[6])/4.00F, width);
	m_pBoundingLines[21]->Reset (verticies[6], verticies[6] + (verticies[2]-verticies[6])/4.00F, width);

	m_pBoundingLines[11]->Reset (verticies[7], verticies[7] + (verticies[4]-verticies[7])/4.00F, width);
	m_pBoundingLines[22]->Reset (verticies[7], verticies[7] + (verticies[6]-verticies[7])/4.00F, width);
	m_pBoundingLines[23]->Reset (verticies[7], verticies[7] + (verticies[3]-verticies[7])/4.00F, width);

	//
	//	Add the lines to the scene if necessary
	//
	if (m_bIsAddedToScene == false) {

		SceneEditorClass *scene = ::Get_Scene_Editor ();		
		if (scene != NULL) {
			
			// Loop through all the line segments and add them to the scene
			for (int line = 0; line < 24; line ++) {
				scene->Add_Dynamic_Object (m_pBoundingLines[line]);
			}

			// Success!
			m_bIsAddedToScene = true;
		}
	}

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Remove_From_Scene
//
//////////////////////////////////////////////////////////////////////
void 
SelectionBoxClass::Remove_From_Scene (void)
{
	// Get a pointer to the current scene
	SceneEditorClass *scene = ::Get_Scene_Editor ();
	if (scene != NULL) {
		
		// Remove all the lines from the scene
		for (int line = 0; line < 24; line ++) {			
			scene->Remove_Object (m_pBoundingLines[line]);
		}

		// Success!
		m_bIsAddedToScene = true;
	}		

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Color
//
//////////////////////////////////////////////////////////////////////
void 
SelectionBoxClass::Set_Color (const Vector3 &color)
{
	// Change the color of all the lines
	for (int line = 0; line < 24; line ++) {		
		m_pBoundingLines[line]->Set_Color (color);
	}

	return ;
}
