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

#if !defined(AFX_SPECSHEET_H__9CC8BAEB_7052_11D3_A05A_00104B791122__INCLUDED_)
#define AFX_SPECSHEET_H__9CC8BAEB_7052_11D3_A05A_00104B791122__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SpecSheet.h : header file
//

#include "vector.h"

// Forward declarations
class DefinitionClass;
class ParameterCtrlClass;
class ParameterClass;


/////////////////////////////////////////////////////////////////////////////
//
// SpecSheetClass
//
/////////////////////////////////////////////////////////////////////////////
class SpecSheetClass : public CWnd
{
// Construction
public:
	SpecSheetClass (DefinitionClass *definition);

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(SpecSheetClass)
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~SpecSheetClass();

	// Generated message map functions
protected:
	//{{AFX_MSG(SpecSheetClass)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnNcMouseMove(UINT nHitTest, CPoint point);
	afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);
	afx_msg void OnNcLButtonUp(UINT nHitTest, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	
	///////////////////////////////////////////////////////////////////
	//	Public methods
	///////////////////////////////////////////////////////////////////
	void					Apply (void);
	DefinitionClass *	Get_Definition (void) const { return m_Definition; }

	int					Get_Parameter_Count (void) const { return m_CtrlList.Count (); }
	ParameterClass *	Get_Parameter (int index);
	void					Get_Current_Filename_Value (int index, CString &value);

	// Accessors
	bool					Is_Temp (void) const			{ return m_IsTemp; }
	void					Set_Is_Temp (bool is_temp)	{ m_IsTemp = is_temp; }

	bool					Is_Read_Only (void) const	{ return m_IsReadOnly; }
	void					Set_Read_Only (bool onoff) { m_IsReadOnly = onoff; }

	void					Show_File_Only (bool onoff)		{ m_ShowFileOnly = onoff; }	
	bool					Were_Files_Changed (void) const	{ return m_WereFilesChanged; }

	void					Set_Asset_Tree_Only (bool onoff)				{ m_AssetTreeOnly = onoff; }

protected:

	///////////////////////////////////////////////////////////////////
	//	Protected methods
	///////////////////////////////////////////////////////////////////
	bool					Is_Filtered (ParameterClass *parameter);
	void					Add_Parameter (ParameterClass *parameter);
	void					Scroll_Controls (int amount);
	void					Set_Scroll_Pos (int new_pos);

private:

	///////////////////////////////////////////////////////////////////
	//	Private member data
	///////////////////////////////////////////////////////////////////
	bool					m_IsReadOnly;
	bool					m_IsTemp;
	bool					m_ShowFileOnly;
	bool					m_WereFilesChanged;
	bool					m_IsScrolling;
	bool					m_AssetTreeOnly;
	CPoint				m_LastPoint;
	int					m_ScrollPos;
	int					m_MaxScrollPos;

	DefinitionClass *	m_Definition;

	DynamicVectorClass<ParameterCtrlClass *>	m_CtrlList;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SPECSHEET_H__9CC8BAEB_7052_11D3_A05A_00104B791122__INCLUDED_)
