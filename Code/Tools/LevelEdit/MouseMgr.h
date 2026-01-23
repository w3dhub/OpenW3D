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
//	MouseMgr.H
//
//	Class declarations for managing different mouse modes;
//

#ifndef __MOUSEMGR_H
#define __MOUSEMGR_H


#include "refcount.h"
#include "Utils.h"
#include "rendobj.h"
#include "assetmgr.h"
#include "cameramgr.h"


class WaypathNodeClass;
class ZoneInstanceClass;
class TransitionInstanceClass;


///////////////////////////////////////////////////////////////
//
//	MouseModeClass
//
///////////////////////////////////////////////////////////////
class MouseModeClass
{
	public:

		///////////////////////////////////////////////////
		//	Public constructors/destructors
		///////////////////////////////////////////////////
		MouseModeClass (void)
			: m_bUpdate (false) { }

		virtual ~MouseModeClass (void) { }

		
		///////////////////////////////////////////////////
		//	Public methods
		///////////////////////////////////////////////////

		//
		//	Mouse message handlers
		//
		virtual void					Handle_LButton_Dblclk (UINT /* flags */, CPoint /* point */) { }
		virtual void					Handle_LButton_Down (UINT flags, CPoint point);
		virtual void					Handle_LButton_Up (UINT flags, CPoint point);
		virtual void					Handle_RButton_Down (UINT flags, CPoint point);
		virtual void					Handle_RButton_Up (UINT flags, CPoint point);
		virtual void					Handle_Mouse_Move (UINT flags, CPoint point);

		//
		//	Misc virtuals
		//
		virtual void					On_Mode_Set (void) {}
		virtual void					On_Mode_Exit (void) {}

	protected:

		friend class MouseMgrClass;

		///////////////////////////////////////////////////
		//	Protected member data
		///////////////////////////////////////////////////
		
		CPoint							m_LastMousePoint;
		CPoint							m_MousePoint;
		bool								m_bUpdate;
		CameraMgr::CAMERA_MODE		m_LButtonMode;
		CameraMgr::CAMERA_MODE		m_RButtonMode;
		CameraMgr::CAMERA_MODE		m_BothButtonMode;
};


///////////////////////////////////////////////////////////////
//
//	MMCameraDefaultClass
//
///////////////////////////////////////////////////////////////
class MMCameraDefaultClass : public MouseModeClass
{
	public:

		/////////////////////////////////////////////////////
		//	Public constructors/destructors
		/////////////////////////////////////////////////////
		MMCameraDefaultClass (void)
			{
				m_LButtonMode = CameraMgr::MODE_ROTATE_FREE;
				m_RButtonMode = CameraMgr::MODE_MOVE_PLANE;
				m_BothButtonMode = CameraMgr::MODE_MOVE_UPDOWN;
			}

		virtual ~MMCameraDefaultClass (void) { }
};


///////////////////////////////////////////////////////////////
//
//	MMCameraWalkClass
//
///////////////////////////////////////////////////////////////
class MMCameraWalkClass : public MouseModeClass
{
	public:

		///////////////////////////////////////////////////
		//	Public constructors/destructors
		///////////////////////////////////////////////////
		MMCameraWalkClass (void)
			{
				m_LButtonMode = CameraMgr::MODE_WALK_THROUGH;
				m_RButtonMode = CameraMgr::MODE_WALK_THROUGH;
				m_BothButtonMode = CameraMgr::MODE_WALK_THROUGH;
			}

		virtual ~MMCameraWalkClass (void) { }


		///////////////////////////////////////////////////
		//	Public methods
		///////////////////////////////////////////////////
		virtual void			On_Mode_Set (void) { } //::Get_Camera_Mgr ()->Set_Camera_Mode (CameraMgr::MODE_WALK_THROUGH); }
};


///////////////////////////////////////////////////////////////
//
//	MMCameraFlyClass
//
//
class MMCameraFlyClass : public MouseModeClass
{
	public:

		///////////////////////////////////////////////////
		//	Public constructors/destructors
		///////////////////////////////////////////////////
		MMCameraFlyClass (void)
			{
				m_LButtonMode = CameraMgr::MODE_FLY_THROUGH;
				m_RButtonMode = CameraMgr::MODE_FLY_THROUGH;
				m_BothButtonMode = CameraMgr::MODE_FLY_THROUGH;
			}

		virtual ~MMCameraFlyClass (void) {}
};


///////////////////////////////////////////////////////////////
//
//	MMCameraOrbitClass
//
///////////////////////////////////////////////////////////////
class MMCameraOrbitClass : public MouseModeClass
{
public:

	///////////////////////////////////////////////////
	//	Public constructors/destructors
	///////////////////////////////////////////////////
	MMCameraOrbitClass (void)
		{
			m_LButtonMode = CameraMgr::MODE_ORBIT;
			m_RButtonMode = CameraMgr::MODE_ORBIT;
			m_BothButtonMode = CameraMgr::MODE_ORBIT;
		}

	virtual ~MMCameraOrbitClass (void) {}
};


///////////////////////////////////////////////////////////////
//
//	MMHeightfieldEditClass
//
///////////////////////////////////////////////////////////////
class MMHeightfieldEditClass : public MouseModeClass
{
public:

	///////////////////////////////////////////////////
	//	Public constructors/destructors
	///////////////////////////////////////////////////
	MMHeightfieldEditClass (void)
		{
			m_LButtonMode		= CameraMgr::MODE_MOVE_PLANE;
			m_RButtonMode		= CameraMgr::MODE_MOVE_PLANE;
			m_BothButtonMode	= CameraMgr::MODE_MOVE_PLANE;
		}

	virtual ~MMHeightfieldEditClass (void) {}

	///////////////////////////////////////////////////
	//	Public methods
	///////////////////////////////////////////////////

	//
	//	Mouse message handlers
	//
	virtual void			Handle_LButton_Down (UINT flags, CPoint point);
	virtual void			Handle_LButton_Up (UINT flags, CPoint point);
	virtual void			Handle_RButton_Down (UINT flags, CPoint point);
	virtual void			Handle_RButton_Up (UINT flags, CPoint point);	

	virtual void			On_Mode_Set (void);
	virtual void			On_Mode_Exit (void);
};


///////////////////////////////////////////////////////////////
//
//	MMObjectDropClass
//
/////////////////////////////////////////////////////////////////
class MMObjectDropClass : public MouseModeClass
{
public:

	///////////////////////////////////////////////////
	//	Public constructors/destructors
	///////////////////////////////////////////////////
	MMObjectDropClass (void)				{ }
	virtual ~MMObjectDropClass (void)	{ }

	
	///////////////////////////////////////////////////
	//	Public methods
	///////////////////////////////////////////////////

	//
	//	Mouse message handlers
	//
	virtual void					Handle_LButton_Down (UINT /* flags */, CPoint /* point */)	{ }
	virtual void					Handle_LButton_Up (UINT /* flags */, CPoint /* point */)		{ }
	virtual void					Handle_RButton_Down (UINT /* flags */, CPoint /* point */)	{ }
	virtual void					Handle_RButton_Up (UINT /* flags */, CPoint /* point */)		{ }
};


///////////////////////////////////////////////////////////////
//
//	MMObjectManipulateClass
//
//
///////////////////////////////////////////////////////////////
class MMObjectManipulateClass : public MouseModeClass
{
public:

	///////////////////////////////////////////////////
	//	Public constructors/destructors
	///////////////////////////////////////////////////
	MMObjectManipulateClass (void)
		:	m_ObjectMode (MODE_NONE),
			m_ForceDropToGround (false),
			m_CurrentMovePos (0, 0, 0),
			m_IntersectPoint (0, 0, 0)
		{
			m_LButtonMode = CameraMgr::MODE_MOVE_PLANE;
			m_RButtonMode = CameraMgr::MODE_MOVE_PLANE;
			m_BothButtonMode = CameraMgr::MODE_MOVE_PLANE;
		}

	virtual ~MMObjectManipulateClass (void) { }

	
	///////////////////////////////////////////////////
	//	Public methods
	///////////////////////////////////////////////////

	//
	//	Mouse message handlers
	//
	virtual void					Handle_LButton_Dblclk (UINT flags, CPoint point);
	virtual void					Handle_LButton_Down (UINT flags, CPoint point);
	virtual void					Handle_LButton_Up (UINT flags, CPoint point);
	virtual void					Handle_RButton_Down (UINT flags, CPoint point);
	virtual void					Handle_RButton_Up (UINT flags, CPoint point);
	virtual void					Handle_Mouse_Move (UINT flags, CPoint point);
	virtual void					On_Mode_Set (void);

	void								Set_Move_Nodes_Mode (void);

protected:

	///////////////////////////////////////////////////
	//	Public data types
	///////////////////////////////////////////////////
	typedef enum
	{
		MODE_NONE		= -1,
		MODE_SELECT		= 0,
		MODE_LINK,
		MODE_MOVE,
		MODE_ROTATE,
		MODE_COUNT
	} OBJECT_MODE;

	///////////////////////////////////////////////////
	//	Protected methods
	///////////////////////////////////////////////////
	void								Move_Selection (CPoint point);
	void								Rotate_Selection (CPoint point);
	void								Link_Item_Hit_Test (CPoint point);
	void								On_Link_Begin (CPoint point);
	void								On_Link_End (CPoint point, bool cancelled = false);
	void								Change_Operation (OBJECT_MODE type);

private:

	///////////////////////////////////////////////////
	//	Private member data
	///////////////////////////////////////////////////
	OBJECT_MODE						m_ObjectMode;
	Vector3							m_CurrentMovePos;
	Vector3							m_IntersectPoint;

	bool								m_ForceDropToGround;

	//class LinkLineClass **		m_pLinkLines;
	//int								m_iLinkLineCount;
	//BOOL								m_bValidLink;
};


///////////////////////////////////////////////////////////////
//
//	MMObjectSelectClass
//
///////////////////////////////////////////////////////////////
class MMObjectSelectClass : public MouseModeClass
{
public:

	///////////////////////////////////////////////////
	//	Public constructors/destructors
	///////////////////////////////////////////////////
	MMObjectSelectClass (void) {}
	virtual ~MMObjectSelectClass (void) {}

	
	///////////////////////////////////////////////////
	//	Public methods
	///////////////////////////////////////////////////

	//
	//	Mouse message handlers
	//
	virtual void					Handle_LButton_Down (UINT /* flags */, CPoint /* point */)	{ }
	virtual void					Handle_LButton_Up (UINT /* flags */, CPoint /* point */)		{ }
	virtual void					Handle_RButton_Down (UINT /* flags */, CPoint /* point */)	{ }
	virtual void					Handle_RButton_Up (UINT /* flags */, CPoint /* point */)		{ }
};


///////////////////////////////////////////////////////////////
//
//	MMWaypathEditClass
//
///////////////////////////////////////////////////////////////
class MMWaypathEditClass : public MouseModeClass
{
public:

	///////////////////////////////////////////////////
	//	Public constructors/destructors
	///////////////////////////////////////////////////
	MMWaypathEditClass (void)
		:	m_CurrentPoint (-1),
			m_Waypath (NULL)
		{
			m_LButtonMode		= CameraMgr::MODE_MOVE_PLANE;
			m_RButtonMode		= CameraMgr::MODE_MOVE_PLANE;
			m_BothButtonMode	= CameraMgr::MODE_MOVE_PLANE;
		}

	virtual ~MMWaypathEditClass (void) { }

	
	///////////////////////////////////////////////////
	//	Public methods
	///////////////////////////////////////////////////

	//
	//	Mouse message handlers
	//
	virtual void			Handle_LButton_Down (UINT flags, CPoint point);
	virtual void			Handle_LButton_Up (UINT flags, CPoint point);
	virtual void			Handle_RButton_Down (UINT flags, CPoint point);
	virtual void			Handle_RButton_Up (UINT flags, CPoint point);
	virtual void			Handle_Mouse_Move (UINT flags, CPoint point);
	virtual void			On_Mode_Set (void);
	virtual void			On_Mode_Exit (void);

	virtual void			Set_Waypath (WaypathNodeClass *waypath, int index = 0) { m_Waypath = waypath; m_CurrentPoint = index; }

protected:

	///////////////////////////////////////////////////
	//	Protected methods
	///////////////////////////////////////////////////
	void						Exit_Mode (void);

private:

	///////////////////////////////////////////////////
	//	Private member data
	///////////////////////////////////////////////////
	WaypathNodeClass *	m_Waypath;
	int						m_CurrentPoint;
};


///////////////////////////////////////////////////////////////
//
//	MMGrabHandleDragClass
//
///////////////////////////////////////////////////////////////
class MMGrabHandleDragClass : public MouseModeClass
{
public:

	///////////////////////////////////////////////////
	//	Public constructors/destructors
	///////////////////////////////////////////////////
	MMGrabHandleDragClass (void)
		:	m_Node (NULL),
			m_Vertex (0)
		{
			m_LButtonMode = CameraMgr::MODE_MOVE_PLANE;
			m_RButtonMode = CameraMgr::MODE_MOVE_PLANE;
			m_BothButtonMode = CameraMgr::MODE_MOVE_PLANE;
		}

	virtual ~MMGrabHandleDragClass (void) {}

	
	///////////////////////////////////////////////////
	//	Public methods
	///////////////////////////////////////////////////

	//
	//	Mouse message handlers
	//
	virtual void					Handle_LButton_Dblclk (UINT /* flags */, CPoint /* point */) {}
	virtual void					Handle_LButton_Down (UINT /* flags */, CPoint /* point */) {}
	virtual void					Handle_LButton_Up (UINT flags, CPoint point);
	virtual void					Handle_RButton_Down (UINT /* flags */, CPoint /* point */) {}
	virtual void					Handle_RButton_Up (UINT /* flags */, CPoint /* point */) {}
	virtual void					Handle_Mouse_Move (UINT flags, CPoint point);
	virtual void					On_Mode_Set (void) { m_Node = NULL; }

	//
	// Specific methods
	//
	void								Set_Node_Info (NodeClass *node, int vertex_index);

private:

	///////////////////////////////////////////////////
	//	Private member data
	///////////////////////////////////////////////////
	NodeClass *			m_Node;
	int					m_Vertex;
};


///////////////////////////////////////////////////////////////
//
//	MouseMgrClass
//
///////////////////////////////////////////////////////////////
class MouseMgrClass
{
public:

	///////////////////////////////////////////////////
	//	Public constructors/destructors
	///////////////////////////////////////////////////
	MouseMgrClass (void);
	virtual ~MouseMgrClass (void);

	///////////////////////////////////////////////////
	//	Public data types
	///////////////////////////////////////////////////
	typedef enum
	{
		MODE_CAMERA_DEFAULT	= 0,
		MODE_CAMERA_WALK,
		MODE_CAMERA_FLY,
		MODE_CAMERA_ORBIT,
		MODE_OBJECT_DROP,
		MODE_OBJECT_MANIPULATE,
		MODE_OBJECT_SELECT,
		MODE_GRABHANDLE_DRAG,
		MODE_WAYPATH_EDIT,
		MODE_HEIGHTFIELD_EDIT,
		MODE_COUNT

	} MOUSE_MODE;


	///////////////////////////////////////////////////
	//	Public methods
	///////////////////////////////////////////////////
	MouseMgrClass::MOUSE_MODE	Get_Mouse_Mode (void) const				{ return m_MouseMode; }
	void								Set_Mouse_Mode (MOUSE_MODE new_mode);
	MouseModeClass *				Get_Mode_Mgr (void)							{ return m_pModeObjects[m_MouseMode]; }

	//
	//	Mouse message handlers
	//
	void					Handle_LButton_Dblclk (UINT flags, CPoint point)	{ m_pModeObjects[m_MouseMode]->Handle_LButton_Dblclk (flags, point); }
	void					Handle_LButton_Down (UINT flags, CPoint point)		{ m_pModeObjects[m_MouseMode]->Handle_LButton_Down (flags, point); IsLButtonDown = true; }
	void					Handle_LButton_Up (UINT flags, CPoint point)			{ m_pModeObjects[m_MouseMode]->Handle_LButton_Up (flags, point); IsLButtonDown = false; }
	void					Handle_RButton_Down (UINT flags, CPoint point)		{ m_pModeObjects[m_MouseMode]->Handle_RButton_Down (flags, point); IsRButtonDown = true; }
	void					Handle_RButton_Up (UINT flags, CPoint point)			{ m_pModeObjects[m_MouseMode]->Handle_RButton_Up (flags, point); IsRButtonDown = false; }
	void					Handle_Mouse_Move (UINT flags, CPoint point)			{ m_pModeObjects[m_MouseMode]->Handle_Mouse_Move (flags, point); }

	//
	//	Mouse point methods
	//
	CPoint				Get_Mouse_Point (void) const			{ return m_pModeObjects[m_MouseMode]->m_MousePoint; }
	CPoint				Get_Last_Mouse_Point (void) const	{ return m_pModeObjects[m_MouseMode]->m_LastMousePoint; }

	//
	//	Object manipulation methods
	//
	void					Move_Node (NodeClass *node);

	//
	//	Button access
	//
	bool					Is_LButton_Down (void) const	{ return IsLButtonDown; }
	bool					Is_RButton_Down (void) const	{ return IsRButtonDown; }

private:

	///////////////////////////////////////////////////
	//	Private member data
	///////////////////////////////////////////////////	
	class MouseModeClass *	m_pModeObjects[MOUSE_MODE::MODE_COUNT];
	MOUSE_MODE					m_MouseMode;

	bool							IsLButtonDown;
	bool							IsRButtonDown;
};


#endif //__MOUSEMGR_H
