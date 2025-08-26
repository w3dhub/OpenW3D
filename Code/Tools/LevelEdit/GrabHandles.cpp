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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/GrabHandles.cpp              $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 3/28/01 3:16p                                               $*
 *                                                                                             *
 *                    $Revision:: 8                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "StdAfx.h"
#include "grabhandles.h"
#include "sceneeditor.h"
#include "node.h"
#include "hittestinfo.h"
#include "Utils.h"
#include "MouseMgr.h"


//////////////////////////////////////////////////////////////////////
//
//	GrabHandlesClass
//
//////////////////////////////////////////////////////////////////////
GrabHandlesClass::GrabHandlesClass (void)
	: m_bIsAddedToScene (false),
	  m_pNode (NULL)
{
	Initialize ();
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	GrabHandlesClass
//
//////////////////////////////////////////////////////////////////////
GrabHandlesClass::GrabHandlesClass (const GrabHandlesClass &ref)
	: m_bIsAddedToScene (false),
	  m_pNode (NULL)
{
	*this = ref;
	Initialize ();	
	return ;
}

//////////////////////////////////////////////////////////////////////
//
//	~GrabHandlesClass
//
//////////////////////////////////////////////////////////////////////
GrabHandlesClass::~GrabHandlesClass (void)
{
	// Loop through and destroy all the handles
	for (int handle = 0; handle < 8; handle ++) {
		MEMBER_RELEASE (m_pHandles[handle]);
	}

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Initialize
//
//////////////////////////////////////////////////////////////////////
void
GrabHandlesClass::Initialize (void)
{
	// Create a default vertex material for the handles
	VertexMaterialClass *vmat = new VertexMaterialClass;
	vmat->Set_Emissive (1,1,1);
	vmat->Set_Diffuse (0,0,0);
	vmat->Set_Opacity (1.0F);

	// Loop through and create all the handles
	for (int handle = 0; handle < 8; handle ++) {
		m_pHandles[handle] = new GrabHandlePhysClass (handle, this);

		// Modify the color of this box to be solid black
		Box3DClass *pbox = m_pHandles[handle]->Get_Box (); 
		pbox->Set_Shader (ShaderClass::_PresetOpaqueSolidShader);
		pbox->Set_Vertex_Material (vmat);
		pbox->Set_Color (Vector3 (0.0F, 0.0F, 0.0F));
		pbox->Disable_Sort ();
		pbox->Enable_Bounding_Sphere ();
		pbox->Enable_Bounding_Box ();
	}

	MEMBER_RELEASE (vmat);	
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	operator=
//
//////////////////////////////////////////////////////////////////////
const GrabHandlesClass &
GrabHandlesClass::operator= (const GrabHandlesClass &ref)
{
	m_pNode = ref.m_pNode;
	
	// Return a reference to ourselves	
	return *this;
}


//////////////////////////////////////////////////////////////////////
//
//	Remove_From_Scene
//
//////////////////////////////////////////////////////////////////////
void
GrabHandlesClass::Remove_From_Scene (void)
{
	if (m_bIsAddedToScene) {

		// Get a pointer to the current scene
		SceneEditorClass *pscene = ::Get_Scene_Editor ();
		
		// Were we successful in getting the scene pointer?
		ASSERT (pscene != NULL);
		if (pscene != NULL) {

			// Loop through and remove all the handles from the scene
			for (int handle = 0; handle < 8; handle ++) {	
				if (m_pHandles[handle] != NULL) {
					pscene->Remove_Object (m_pHandles[handle]);
				}
			}

			// Success!
			m_bIsAddedToScene = false;
		}		
	}

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Hide
//
//////////////////////////////////////////////////////////////////////
void
GrabHandlesClass::Hide (bool bhide)
{
	// Loop through and hide all the handles
	for (int handle = 0; handle < 8; handle ++) {	
		if (m_pHandles[handle] != NULL) {
			m_pHandles[handle]->Peek_Model ()->Set_Hidden (bhide);

			if (bhide == true) {
				m_pHandles[handle]->Inc_Ignore_Counter();
			} else if (m_pHandles[handle]->Is_Ignore_Me()) {
				m_pHandles[handle]->Dec_Ignore_Counter();
			}

		}
	}

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Position_Around_Node
//
//////////////////////////////////////////////////////////////////////
void
GrabHandlesClass::Position_Around_Node (NodeClass *node)
{	
	RenderObjClass *render_obj = node->Peek_Render_Obj ();
	if (render_obj != NULL) {

		//
		// Get the object's bounding box so we know where to 
		// position the grab handles
		//
		AABoxClass bounding_box;
		render_obj->Get_Obj_Space_Bounding_Box (bounding_box);
		const Matrix3D &transform = render_obj->Get_Transform ();

		// Alias the extent vector
		Vector3 extent = bounding_box.Extent;
		extent.X = ::fabs (extent.X);
		extent.Y = ::fabs (extent.Y);
		extent.Z = ::fabs (extent.Z);

		//
		// Determine the new size for each grab handle
		//
		float largest_dim = max (extent.X, extent.Y);
		largest_dim			= max (largest_dim, extent.Z);
		float size			= max (largest_dim / 25.0F, 0.1F);

		// Calculate the positions of the eight verticies
		Vector3 verticies[8];
		verticies[0] = bounding_box.Center + Vector3 (-extent.X,  extent.Y, -extent.Z);
		verticies[1] = bounding_box.Center + Vector3 (-extent.X, -extent.Y, -extent.Z);
		verticies[2] = bounding_box.Center + Vector3 (-extent.X, -extent.Y,  extent.Z);
		verticies[3] = bounding_box.Center + Vector3 (-extent.X,  extent.Y,  extent.Z);

		verticies[4] = bounding_box.Center + Vector3 (extent.X,  extent.Y, -extent.Z);
		verticies[5] = bounding_box.Center + Vector3 (extent.X, -extent.Y, -extent.Z);
		verticies[6] = bounding_box.Center + Vector3 (extent.X, -extent.Y,  extent.Z);
		verticies[7] = bounding_box.Center + Vector3 (extent.X,  extent.Y,  extent.Z);

		// Now add 'handles' centered around each of the 8 verticies
		for (int vertex = 0; vertex < 8; vertex ++) {		

			// Rotate these verticies using the item's current transform
			verticies[vertex] = transform * verticies[vertex];

			// Resize the box
			Box3DClass *pbox = m_pHandles[vertex]->Get_Box ();
			pbox->Set_Dimensions (Vector3 (size, size, size));
			//pbox->Set_Dimensions (Vector3 (1, 1, 1));
			
			// Reposition this handle so its centered around the vertex
			Matrix3D handle_transform = m_pHandles[vertex]->Get_Transform ();
			handle_transform.Set_Translation (verticies[vertex]);
			m_pHandles[vertex]->Set_Transform (handle_transform);
		}
		
		if (m_bIsAddedToScene == false) {

			// Get a pointer to the current scene
			SceneEditorClass *pscene = ::Get_Scene_Editor ();
			
			// Were we successful in getting the scene pointer?
			ASSERT (pscene != NULL);
			if (pscene != NULL) {
				
				// Loop through all the handles and add them to the scene
				for (int handle = 0; handle < 8; handle ++) {
					pscene->Add_Dynamic_Object (m_pHandles[handle]);
				}

				// Success!
				m_bIsAddedToScene = true;
			}
		}
	}

	m_pNode = node;
	return ;
}


//*********************************************************************************************//
//
//	End of GrabHandlesClass
//
//*********************************************************************************************//

//*********************************************************************************************//
//
//	Start of GrabHandlePhysClass
//
//*********************************************************************************************//


//////////////////////////////////////////////////////////////////////
//
//	On_Click
//
//////////////////////////////////////////////////////////////////////
void
GrabHandlePhysClass::On_Click (void)
{
	if (m_pParent != NULL) {
		
		// Switch to the 'grabhandle drag' mode
		MouseMgrClass *mousemgr = ::Get_Mouse_Mgr ();
		mousemgr->Set_Mouse_Mode (MouseMgrClass::MODE_GRABHANDLE_DRAG);

		// Pass on the node/vertex information onto the mode mgr
		MMGrabHandleDragClass *mode = (MMGrabHandleDragClass *)mousemgr->Get_Mode_Mgr ();
		if (mode != NULL) {
			mode->Set_Node_Info (m_pParent->Get_Node (), m_iVertex);
		}
	}
		
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Initialize
//
//////////////////////////////////////////////////////////////////////
void
GrabHandlePhysClass::Initialize (void)
{
	Peek_Model ()->Set_User_Data (&m_HitTestInfo, FALSE);
	return ;
}


//*********************************************************************************************//
//
//	End of GrabHandlePhysClass
//
//*********************************************************************************************//
