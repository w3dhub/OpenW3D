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

// PhysTestDoc.h : interface of the CPhysTestDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_PHYSTESTDOC_H__616293EA_E4F0_11D2_802E_0040056350C8__INCLUDED_)
#define AFX_PHYSTESTDOC_H__616293EA_E4F0_11D2_802E_0040056350C8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CGraphicView;
class CDataView;
class PhysicsSceneClass;
class LightClass;
class RenderObjClass;
class PhysClass;


class CPhysTestDoc : public CDocument
{
protected: // create from serialization only
	CPhysTestDoc();
	DECLARE_DYNCREATE(CPhysTestDoc)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPhysTestDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	//}}AFX_VIRTUAL

// Implementation
public:

	virtual ~CPhysTestDoc(void);
	void				Init_Scene(void);
	void				Load_LEV_File(LPCTSTR lpszPathName);
	void				Load_W3D_File(LPCTSTR lpszPathName);
	void				Load_PHY_File(LPCTSTR lpszPathName);
	void				Save_PHY_File(LPCTSTR lpszPathName);

	CGraphicView * Get_Graphic_View(void);
	CDataView *		Get_Data_View(void);
	void				Add_Physics_Object(PhysClass * obj);
	int				Get_Physics_Object_Count(void);

#ifdef _DEBUG
	virtual void AssertValid(void) const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	PhysicsSceneClass *	Scene;
	LightClass *			Light;
	RenderObjClass *		Origin;
	DWORD						LastTime;

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CPhysTestDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PHYSTESTDOC_H__616293EA_E4F0_11D2_802E_0040056350C8__INCLUDED_)
