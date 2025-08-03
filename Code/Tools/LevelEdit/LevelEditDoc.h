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

// LevelEditDoc.h : interface of the CLevelEditDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_LEVELEDITDOC_H__6D711A29_651E_11D2_9FC8_00104B791122__INCLUDED_)
#define AFX_LEVELEDITDOC_H__6D711A29_651E_11D2_9FC8_00104B791122__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


#include "vector3.h"
#include "matrix3d.h"


///////////////////////////////////////////////////////////
//
//	Constants
//
///////////////////////////////////////////////////////////
const int	MODE_MOD_DROP_TO_GROUND			= 0x000000001;



///////////////////////////////////////////////////////////
//
//	CLevelEditDoc
//
///////////////////////////////////////////////////////////
class CLevelEditDoc : public CDocument
{
protected: // create from serialization only
	CLevelEditDoc();
	DECLARE_DYNCREATE(CLevelEditDoc)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLevelEditDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	virtual void PreCloseFrame( CFrameWnd* pFrame );
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CLevelEditDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CLevelEditDoc)
	afx_msg void OnFileSave();
	afx_msg void OnFileSaveAs();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	public:

		///////////////////////////////////////////////
		//
		//	Public data types
		//
		typedef enum {
			RESTRICT_NONE	= 0,
			RESTRICT_X,
			RESTRICT_Y,
			RESTRICT_Z
			
		} AXIS_RESTRICTION;


		typedef enum {
			COORD_WORLD	= 0,
			COORD_OBJECT,
			COORD_VIEW
			
		} COORD_SYSTEM;

	
		///////////////////////////////////////////////
		//
		//	Public methods
		//
		void								Init_Scene (void);
		class SceneEditorClass *	Get_Scene (void) const { return m_pScene; }
		
		//
		//	Save methods
		//
		void								Set_Current_Path (const char *path);

		//
		//	Operation modifiers
		//
		AXIS_RESTRICTION				Get_Axis_Restriction (void) const { return m_AxisRestriction; }
		void								Set_Axis_Restriction (AXIS_RESTRICTION	restriction) { m_AxisRestriction = restriction; }
		COORD_SYSTEM					Get_Coord_System (void) const { return m_CoordSystem; }
		void								Set_Coord_System (COORD_SYSTEM system) { m_CoordSystem = system; }
		Matrix3D							Get_Coord_System_Transform (class NodeClass *node);

		//
		//	Mode modifiers
		//
		int								Get_Mode_Modifiers () const { return m_ModeModifiers; }
		void								Set_Mode_Modifiers (int modifiers) { m_ModeModifiers = modifiers; }
		
		//
		//	Cleanup routines
		//
		void								CleanupResources (void);

		//
		//	Performance sampling methods		
		//
		bool								Is_Performance_Sampling_On (void) const { return m_bPerformanceSampling; }
		void								Performance_Sampling_On (bool sampling_on) { m_bPerformanceSampling = sampling_on; }

		//
		//	Snap points methods
		//
		bool								Are_Snap_Points_Active (void) const { return m_bSnapPointsActive; }
		void								Activate_Snap_Points (bool bactive) { m_bSnapPointsActive = bactive; }

		//
		//	'Dirty' methods
		//
		void								Need_RePartition (void) { m_bNeedRepartition = true; }
		void								Repartition_If_Necessary (void);

		//
		//	File methods
		//
		class FileMgrClass *			Get_File_Mgr (void) { return m_pFileMgr; }
		bool								Change_Base_Path (void);
		void								Update_Core_Files (void);
		void								Update_Asset_Tree (void);
		void								Update_Assets_For_Mod_Tree (void);

		//
		//	Light methods
		//
		bool								Are_Lights_On (void) const { return m_bLightsOn; }
		void								Turn_Lights_On (bool onoff);

		//
		//	Pathfind Debug methods
		//
		bool								Is_Path_Web_Displayed (void) const	{ return m_DisplayPathWeb; }
		void								Display_Path_Web (bool onoff)			{ m_DisplayPathWeb = onoff; }

		bool								Are_Full_Paths_Displayed (void) const	{ return m_DisplayFullPaths; }
		void								Display_Full_Paths (bool onoff)			{ m_DisplayFullPaths = onoff; }

		//
		//	Debug methods
		//
		void								Set_Debug_Script_Mode (bool onoff);
		bool								Get_Debug_Script_Mode (void) const	{ return m_DebugScriptsMode; }
		
		//
		//	Preload methods
		//
		void								Preload_Assets (void);
		void								Preload_Human_Data (void);
		void								Preload_Tool_Assets (void);

		//
		//	Batch export information
		//
		static bool						Is_Batch_Export_Mode (void)				{ return IsBatchExportMode; }
		static void						Set_Is_Batch_Export_Mode (bool onoff)	{ IsBatchExportMode = onoff; }

	protected:		

		//
		//	VSS methods
		//
		bool								Initialize_VSS (void);
		bool								Do_Version_Check (void);

		//
		//	Export methods
		//
		void								Save_Level_File (const char *full_path);
		bool								Pick_Save_Path (CString &save_path);

	private:

		/////////////////////////////////////////////////////////////////
		//	Private member data
		/////////////////////////////////////////////////////////////////
		class SceneEditorClass *	m_pScene;
		class FileMgrClass *			m_pFileMgr;
		COORD_SYSTEM					m_CoordSystem;
		AXIS_RESTRICTION				m_AxisRestriction;
		int								m_ModeModifiers;
		bool								m_bPerformanceSampling;
		bool								m_bSnapPointsActive;
		bool								m_bNeedRepartition;
		bool								m_bLightsOn;

		bool								m_DisplayPathWeb;
		bool								m_DisplayFullPaths;

		bool								m_DebugScriptsMode;

		static bool						IsBatchExportMode;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LEVELEDITDOC_H__6D711A29_651E_11D2_9FC8_00104B791122__INCLUDED_)
