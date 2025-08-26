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

#if !defined(AFX_SKYPROPPAGE_H__CFBFA3F7_4179_4243_AA5E_47415502AC01__INCLUDED_)
#define AFX_SKYPROPPAGE_H__CFBFA3F7_4179_4243_AA5E_47415502AC01__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SkyPropPage.h : header file
//

#include <afxdtctl.h>
#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// SkyPropPageClass dialog

class SkyPropPageClass : public CPropertyPage
{
	DECLARE_DYNCREATE(SkyPropPageClass)

// Construction
public:
	SkyPropPageClass();
	~SkyPropPageClass();

// Dialog Data
	//{{AFX_DATA(SkyPropPageClass)
	enum { IDD = IDD_BACKGROUND_SKY };
	CSliderCtrl	CloudGloominessCtrl;
	CSliderCtrl	CloudCoverCtrl;
	CDateTimeCtrl	TimeCtrl;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(SkyPropPageClass)
	public:
	virtual void OnOK();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(SkyPropPageClass)
	virtual BOOL OnInitDialog();
	afx_msg void OnSkyLightMoon();
	afx_msg void OnSkyLightSun();
	afx_msg void OnSkyMoonFull();
	afx_msg void OnSkyMoonPartial();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SKYPROPPAGE_H__CFBFA3F7_4179_4243_AA5E_47415502AC01__INCLUDED_)
