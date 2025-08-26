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
//	SelectionBoxClass.H
//
//	Class declaration for a selected item's bounding box.
//

#ifndef __SELECTIONBOX_H
#define __SELECTIONBOX_H


#include "Utils.h"
#include "Line3D.H"
#include "EditorPhys.H"


// Forward declarations
class NodeClass;


///////////////////////////////////////////////////////////////////////////////
//
//	SelectionBoxClass
//
///////////////////////////////////////////////////////////////////////////////
class SelectionBoxClass
{
public:

	////////////////////////////////////////////////////////
	// Public data types
	////////////////////////////////////////////////////////
	class DecorationLineClass : public EditorPhysClass
	{
		public:
			DecorationLineClass (void);
			virtual ~DecorationLineClass (void);

			// Used to reset the position of the line
			void				Reset (const Vector3 &start, const Vector3 &end, float width) { m_pLine3D->Reset (start, end, width); Update_Cull_Box(); }
			void				Set_Color (const Vector3 &color) { if (m_pLine3D != NULL) m_pLine3D->Re_Color (color.X, color.Y, color.Z); }

		private:
			Line3DClass		*m_pLine3D;
	};


public:

	////////////////////////////////////////////////////////
	// Public contsructors/destructors
	////////////////////////////////////////////////////////
	SelectionBoxClass (void);
	virtual ~SelectionBoxClass (void);


	////////////////////////////////////////////////////////
	// Public methods
	////////////////////////////////////////////////////////
	void		Display_Around_Node (const NodeClass &node);
	void		Display_Around_Node (const RenderObjClass &render_obj);
	void		Remove_From_Scene (void);
	void		Set_Color (const Vector3 &color);

private:

	////////////////////////////////////////////////////////
	// Private member data
	////////////////////////////////////////////////////////
	DecorationLineClass	*m_pBoundingLines[24];
	bool						m_bIsAddedToScene;
};


#endif //__SELECTIONBOX_H

