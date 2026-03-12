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

// SplineTestDoc.h : interface of the CSplineTestDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_SPLINETESTDOC_H__2AE09FDB_FBEF_11D2_9BD9_00A0C9988171__INCLUDED_)
#define AFX_SPLINETESTDOC_H__2AE09FDB_FBEF_11D2_9BD9_00A0C9988171__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "vector.h"

class Curve3DClass;
class Vector3;


class CSplineTestDoc : public CDocument
{
protected: // create from serialization only
	CSplineTestDoc();
	DECLARE_DYNCREATE(CSplineTestDoc)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSplineTestDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CSplineTestDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	enum { LINEAR=0,CATMULL_ROM,CARDINAL,TCB,VEHICLE };

	void									Reset_Curve(void);
	void									Set_Curve_Type(int type);
	int									Get_Curve_Type(void);
	Curve3DClass *						Get_Curve(void);

	bool									Grab_Point(const Vector3 & pos);
	bool									Drag_Point(const Vector3 & pos);
	bool									Release_Point(void);
	bool									Is_Dragging(void);
	bool									Add_Point(const Vector3 & pos);
	void									Edit_Point(const Vector3 & pos);

	void									Simulate_Turn_Radius (void);

protected:

	int									Pick(const Vector3 & pos);
	Curve3DClass *						Create_Curve(int type);

	int									CurveType;
	Curve3DClass *						Curve;
	int									DragIndex;

// Generated message map functions
protected:
	//{{AFX_MSG(CSplineTestDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SPLINETESTDOC_H__2AE09FDB_FBEF_11D2_9BD9_00A0C9988171__INCLUDED_)
