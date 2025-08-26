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

#if !defined(AFX_LIGHTSETTINGSPAGE_H__C0BCECAD_B6EA_11D2_9FF8_00104B791122__INCLUDED_)
#define AFX_LIGHTSETTINGSPAGE_H__C0BCECAD_B6EA_11D2_9FF8_00104B791122__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LightSettingsPage.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// LightSettingsPageClass form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif


class LightInstanceClass;
#include "DockableForm.H"
#include "vector3.h"
#include "NodeSettingsInterface.H"


//////////////////////////////////////////////////////////
//
//	LightSettingsPageClass
//
class LightSettingsPageClass : public DockableFormClass
{
	public:
		LightSettingsPageClass (void);
		LightSettingsPageClass (LightSettingsClass *plight);
		virtual ~LightSettingsPageClass (void);

// Form Data
public:
	//{{AFX_DATA(LightSettingsPageClass)
	enum { IDD = IDD_LIGHT_SETTINGS };
	CSliderCtrl	m_IntensitySlider;
	CSliderCtrl	m_OuterAttenSlider;
	CSliderCtrl	m_InnerAttenSlider;
	//}}AFX_DATA

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(LightSettingsPageClass)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:	
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(LightSettingsPageClass)
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnChangeInnerAttenEdit();
	afx_msg void OnChangeOuterAttenEdit();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnKillfocusOuterAttenEdit();
	afx_msg void OnKillfocusInnerAttenEdit();
	afx_msg void OnAmbientButton();
	afx_msg void OnDiffuseButton();
	afx_msg void OnSpecularButton();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	public:

		///////////////////////////////////////////////////////
		//
		//	Public methods
		//
		void						HandleInitDialog (void);
		bool						Apply_Changes (void);
		void						Discard_Changes (void);

	protected:
		
		///////////////////////////////////////////////////////
		//
		//	Inline accessors
		//

	private:

		///////////////////////////////////////////////////////
		//
		//	Private member data
		//
		LightSettingsClass *	m_pLight;
		Vector3					m_OrigAmbient;
		Vector3					m_OrigDiffuse;
		Vector3					m_OrigSpecular;
		float						m_OrigIntensity;
		float						m_OrigInnerAttenuation;
		float						m_OrigOuterAttenuation;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LIGHTSETTINGSPAGE_H__C0BCECAD_B6EA_11D2_9FF8_00104B791122__INCLUDED_)
