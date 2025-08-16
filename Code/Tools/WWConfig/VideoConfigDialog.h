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

#if !defined(AFX_VIDEOCONFIGDIALOG_H__BE670D78_397C_4AFE_A45A_C7798BA1869B__INCLUDED_)
#define AFX_VIDEOCONFIGDIALOG_H__BE670D78_397C_4AFE_A45A_C7798BA1869B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"
#include "vector.h"
#include "rddesc.h"
#include "..\..\combat\specialbuilds.h"

/*
#ifdef MULTIPLAYERDEMO
#define RENEGADE_SUB_KEY_NAME_RENDER "Software\\Westwood\\RenegadeMPDemo\\Render"
#else
#define RENEGADE_SUB_KEY_NAME_RENDER "Software\\Westwood\\Renegade\\Render"
#endif // MULTIPLAYERDEMO
*/

#if	defined(FREEDEDICATEDSERVER)
#define RENEGADE_SUB_KEY_NAME_RENDER "Software\\Westwood\\RenegadeFDS\\Render"
#elif defined(MULTIPLAYERDEMO)
#define RENEGADE_SUB_KEY_NAME_RENDER "Software\\Westwood\\RenegadeMPDemo\\Render"
#elif defined(BETACLIENT)
#define RENEGADE_SUB_KEY_NAME_RENDER "Software\\Westwood\\RenegadeBeta\\Render"
#else
#define RENEGADE_SUB_KEY_NAME_RENDER "Software\\Westwood\\Renegade\\Render"
#endif

/////////////////////////////////////////////////////////////////////////////
//
// VideoConfigDialogClass
//
/////////////////////////////////////////////////////////////////////////////
class VideoConfigDialogClass : public CDialog
{
// Construction
public:
	VideoConfigDialogClass(CWnd* pParent = NULL);   // standard constructor
	~VideoConfigDialogClass();

// Dialog Data
	//{{AFX_DATA(VideoConfigDialogClass)
	enum { IDD = IDD_VIDEO_CONFIG };
	CSliderCtrl	m_ResSliderCtrl;
	CListCtrl	m_DriverListCtrl;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(VideoConfigDialogClass)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(VideoConfigDialogClass)
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnItemchangedDriverList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangeBitdepthCombo();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnWindowedCheck();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	/////////////////////////////////////////////////////////////////
	//	Public methods
	/////////////////////////////////////////////////////////////////
	void			Apply_Changes (void);
	static VideoConfigDialogClass* Get_Instance();

	int Get_Current_Bit_Depth() const { return CurrentBitDepth; }
	int Get_Current_Driver_Index() const { return CurrentDriverIndex; }
	const D3DADAPTER_IDENTIFIER9& Get_Current_Adapter_Identifier() const { return CurrentAdapterIdentifier; }
	const D3DCAPS9& Get_Current_Caps() const { return CurrentCaps; }

private:
	
	/////////////////////////////////////////////////////////////////
	//	Private methods
	/////////////////////////////////////////////////////////////////
	void			Update_Display_Settings (void);
	void			Update_Color_Combo (void);
	void			Update_Resolution_Slider (void);
	void			Update_Resolution_Text (void);
	void			Select_Default_Resolution (void);


	/////////////////////////////////////////////////////////////////
	//	Private member data
	/////////////////////////////////////////////////////////////////
	DynamicVectorClass<ResolutionDescClass>	ResolutionList;
	int													CurrentWidth;
	int													CurrentHeight;
	int													CurrentBitDepth;
	bool													CurrentIsWindowed;
	int													CurrentDriverIndex;
	D3DCAPS9												CurrentCaps;
	D3DADAPTER_IDENTIFIER9							CurrentAdapterIdentifier;

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VIDEOCONFIGDIALOG_H__BE670D78_397C_4AFE_A45A_C7798BA1869B__INCLUDED_)
