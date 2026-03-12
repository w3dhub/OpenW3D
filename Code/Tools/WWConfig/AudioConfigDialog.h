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

#if !defined(AFX_AUDIOCONFIGDIALOG_H__A52D1748_C2C5_4536_A5E2_50988E5D5AB2__INCLUDED_)
#define AFX_AUDIOCONFIGDIALOG_H__A52D1748_C2C5_4536_A5E2_50988E5D5AB2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
//
// AudioConfigDialogClass
//
/////////////////////////////////////////////////////////////////////////////
class AudioConfigDialogClass : public CDialog
{
// Construction
public:
	AudioConfigDialogClass(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(AudioConfigDialogClass)
	enum { IDD = IDD_AUDIO_CONFIG };
	CSliderCtrl	m_MusicVolSlider;
	CSliderCtrl	m_SoundVolSlider;
	CSliderCtrl	m_DialogVolSlider;
	CSliderCtrl	m_CinematicVolSlider;
	CListCtrl	m_ListCtrl;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(AudioConfigDialogClass)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(AudioConfigDialogClass)
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	/////////////////////////////////////////////////////////////////
	//	Public methods
	/////////////////////////////////////////////////////////////////
	void			Apply_Changes (void);

private:

	/////////////////////////////////////////////////////////////////
	//	Private methods
	/////////////////////////////////////////////////////////////////
	void			Update_Slider_Enable_State (void);


	/////////////////////////////////////////////////////////////////
	//	Private member data
	/////////////////////////////////////////////////////////////////

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AUDIOCONFIGDIALOG_H__A52D1748_C2C5_4536_A5E2_50988E5D5AB2__INCLUDED_)
