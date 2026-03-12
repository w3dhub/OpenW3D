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

#if !defined(AFX_SOUNDSETTINGSPAGE_H__C0BCECAD_B6EA_11D2_9FF8_00104B791122__INCLUDED_)
#define AFX_SOUNDSETTINGSPAGE_H__C0BCECAD_B6EA_11D2_9FF8_00104B791122__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif


#include "dockableform.h"


//////////////////////////////////////////////////////////
//	Forward declarations
//////////////////////////////////////////////////////////
class AudibleSoundDefinitionClass;


//////////////////////////////////////////////////////////
//
//	SoundSettingsPageClass
//
//////////////////////////////////////////////////////////
class SoundSettingsPageClass : public DockableFormClass
{
	public:
		SoundSettingsPageClass (void);
		virtual ~SoundSettingsPageClass (void);

// Form Data
public:
	//{{AFX_DATA(SoundSettingsPageClass)
	enum { IDD = IDD_SOUND_SETTINGS };
	CSliderCtrl	m_VolumeSlider;
	//}}AFX_DATA

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(SoundSettingsPageClass)
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
	//{{AFX_MSG(SoundSettingsPageClass)
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnChangeVolumeEdit();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	///////////////////////////////////////////////////////
	//	Public methods
	///////////////////////////////////////////////////////
	void			HandleInitDialog (void);
	bool			Apply_Changes (void);
	void			Discard_Changes (void);

	//
	//	Accessors
	//
	void			Set_Definition (AudibleSoundDefinitionClass *definition)	{ Definition = definition; }

protected:

	///////////////////////////////////////////////////////
	//	Protected methods
	///////////////////////////////////////////////////////
	void			Update_Enable_State (void);

private:

	///////////////////////////////////////////////////////
	//	Private member data
	///////////////////////////////////////////////////////
	AudibleSoundDefinitionClass *	Definition;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SOUNDSETTINGSPAGE_H__C0BCECAD_B6EA_11D2_9FF8_00104B791122__INCLUDED_)
