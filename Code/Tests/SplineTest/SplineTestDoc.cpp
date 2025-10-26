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

// SplineTestDoc.cpp : implementation of the CSplineTestDoc class
//

#include "stdafx.h"
#include "SplineTest.h"

#include "SplineTestDoc.h"
#include "CurvePoints.h"
#include "curve.h"
#include "hermitespline.h"
#include "catmullromspline.h"
#include "cardinalspline.h"
#include "tcbspline.h"
#include "wwmath.h"
#include "CardinalDialog.h"
#include "TCBDialog.h"
#include "matrix3d.h"
#include "vehiclecurve.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const float GRAB_DIST = 0.25f;

/////////////////////////////////////////////////////////////////////////////
// CSplineTestDoc

IMPLEMENT_DYNCREATE(CSplineTestDoc, CDocument)

BEGIN_MESSAGE_MAP(CSplineTestDoc, CDocument)
	//{{AFX_MSG_MAP(CSplineTestDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSplineTestDoc construction/destruction
CSplineTestDoc::CSplineTestDoc()
{
	CurveType = LINEAR;
	Curve = new LinearCurve3DClass;
	DragIndex = -1;
}

CSplineTestDoc::~CSplineTestDoc()
{
	if (Curve != NULL) {
		delete Curve;
	}
}

BOOL CSplineTestDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return false;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return true;
}

/////////////////////////////////////////////////////////////////////////////
// CSplineTestDoc serialization

void CSplineTestDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CSplineTestDoc diagnostics

#ifdef _DEBUG
void CSplineTestDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CSplineTestDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CSplineTestDoc commands

void CSplineTestDoc::Reset_Curve(void)
{
	if (Curve != NULL) {
		delete Curve;
	}
	Curve = Create_Curve(CurveType);
	DragIndex = -1;
	UpdateAllViews(NULL);
}

void CSplineTestDoc::Set_Curve_Type(int type)
{
	assert (Curve != NULL);

	if (type != CurveType) {
		Curve3DClass * new_curve = NULL;
		new_curve = Create_Curve(type);

		for (int i=0; i<Curve->Key_Count(); i++) {
			Vector3 pt;
			float t;
			Curve->Get_Key(i,&pt,&t);
			new_curve->Add_Key(pt,t);

			UpdateAllViews(NULL);
		}

		delete Curve;
		Curve = new_curve;
		
		CurveType = type;
	}
}

int CSplineTestDoc::Get_Curve_Type(void)
{
	return CurveType;
}

Curve3DClass *	CSplineTestDoc::Get_Curve(void)
{
	return Curve;
}

Curve3DClass * CSplineTestDoc::Create_Curve(int type)
{
	Curve3DClass * new_curve = NULL;
	switch (type) {
	case LINEAR:
		new_curve = new LinearCurve3DClass;
		break;
	case VEHICLE:
		new_curve = new VehicleCurveClass (1.0F);
		break;
	case CATMULL_ROM:
		new_curve = new CatmullRomSpline3DClass;		
		break;
	case CARDINAL:
		new_curve = new CardinalSpline3DClass;
		break;
	case TCB:
		new_curve = new TCBSpline3DClass;
		break;
	}
	return new_curve;
}

bool CSplineTestDoc::Grab_Point(const Vector3 & pos)
{
	DragIndex = -1;
	if (Curve) {
		DragIndex = Pick(pos);
	}

	return (DragIndex != -1);
}

bool  CSplineTestDoc::Drag_Point(const Vector3 & pos)
{
	if (DragIndex == -1) {
		return false;
	} else {
		if (Curve) {
			Curve->Set_Key(DragIndex,pos);
			UpdateAllViews(NULL);
		}
		return true;
	}
}

bool  CSplineTestDoc::Release_Point(void)
{
	DragIndex = -1;
	return true;
}

bool CSplineTestDoc::Is_Dragging(void)
{
	return (DragIndex != -1);
}

bool  CSplineTestDoc::Add_Point(const Vector3 & pos)
{
	if (Curve == NULL) return false;
	if (Curve->Key_Count() == 0) {
		Curve->Add_Key(pos,0.0f);
	} else {
		// make dt random for testing
		Curve->Add_Key(pos,Curve->Get_End_Time() + 1.0f); //WWMath::Random_Float(0.1f,5.0f));
	}
	UpdateAllViews(NULL);
	return true;
}

int CSplineTestDoc::Pick(const Vector3 & pos)
{
	int index = -1;
	for (int pi=0; pi<Curve->Key_Count(); pi++) {
		Vector3 keypt;
		Curve->Get_Key(pi,&keypt,NULL);
		if ((keypt - pos).Length() < GRAB_DIST) {
			index = pi;
			break;
		}
	}
	return index;
}

void CSplineTestDoc::Edit_Point(const Vector3 & pos)
{
	int index = Pick(pos);
	if (index == -1) return;

	switch (CurveType) {
	case LINEAR:
	case CATMULL_ROM:
	case VEHICLE:
		break;
	case CARDINAL:
		{
			CCardinalDialog dlg(::AfxGetMainWnd(),(CardinalSpline3DClass *)Curve,index);
			dlg.DoModal();
			break;
		}
	case TCB:
		{
			CTCBDialog dlg(::AfxGetMainWnd(),(TCBSpline3DClass *)Curve,index);
			dlg.DoModal();
			break;
		}
	}

	UpdateAllViews(NULL);
	return;
}


void CSplineTestDoc::Simulate_Turn_Radius (void)
{
	return ;
}
