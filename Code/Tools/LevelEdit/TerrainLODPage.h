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

#if !defined(AFX_TERRAINLODPAGE_H__633E0B5B_49FF_11D3_A050_00104B791122__INCLUDED_)
#define AFX_TERRAINLODPAGE_H__633E0B5B_49FF_11D3_A050_00104B791122__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "DockableForm.H"
#include "Resource.H"
#include "vector.h"


/////////////////////////////////////////////////////////////////////////////
//
// TerrainLODPageClass dialog
//
/////////////////////////////////////////////////////////////////////////////
class TerrainLODPageClass : public DockableFormClass
{
	public:

		TerrainLODPageClass (void);
		virtual ~TerrainLODPageClass (void);

// Dialog Data
	//{{AFX_DATA(TerrainLODPageClass)
	enum { IDD = IDD_TERRAIN_LOD_SETTINGS };
	CListCtrl	m_LODListCtrl;
	CSpinButtonCtrl	m_LODCountSpin;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(TerrainLODPageClass)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(TerrainLODPageClass)
	afx_msg void OnDblclkLodList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnUpdateLodCountEdit();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()


	public:

		///////////////////////////////////////////////////////
		//	Public methods
		///////////////////////////////////////////////////////
		void						HandleInitDialog (void);
		bool						Apply_Changes (void);

		DynamicVectorClass<unsigned int> &Get_Distance_List (void)	{ return m_SettingsList; }

	protected:
		
		///////////////////////////////////////////////////////
		//	Protected methods
		///////////////////////////////////////////////////////
		void						On_Count_Change (void);

	private:

		///////////////////////////////////////////////////////
		//	Private member data
		///////////////////////////////////////////////////////
		DynamicVectorClass<unsigned int>	m_SettingsList;
		bool										m_bFinishedInit;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TERRAINLODPAGE_H__633E0B5B_49FF_11D3_A050_00104B791122__INCLUDED_)
