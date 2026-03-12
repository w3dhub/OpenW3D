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

/***********************************************************************************************
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : LightMap                                                     *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tool $*
 *                                                                                             *
 *                      $Author:: Ian_l               $*
 *                                                                                             *
 *                     $Modtime:: 1/24/01 3:29p       $*
 *                                                                                             *
 *                    $Revision:: 3                                                         $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if !defined(AFX_INSERTSOLVEDIALOG_H__6F280B62_836D_11D4_A3D2_009027447394__INCLUDED_)
#define AFX_INSERTSOLVEDIALOG_H__6F280B62_836D_11D4_A3D2_009027447394__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class InsertSolveDialog : public CDialog
{
// Construction
public:
	InsertSolveDialog (const char *initialpathname, CWnd* pParent = NULL);

	~InsertSolveDialog() {
		if (FileListBuffer != NULL) delete [] FileListBuffer;
		if (InclusionString != NULL) delete [] InclusionString;
	}

	char *Directory_Name()	 {return (DirectoryName);}
	char *Filename_List()	 {return (FilenameList);}
	bool  Apply_Selective()	 {return (ApplySelective);}
	bool	Invert_Selection() {return (InvertSelection);}
	bool	Blend_Noise()		 {return (BlendNoise);}
	char *Inclusion_String() {return (InclusionString);}

// Dialog Data
	//{{AFX_DATA(InsertSolveDialog)
	enum { IDD = IDD_INSERT_SOLVE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(InsertSolveDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(InsertSolveDialog)
	afx_msg void OnBrowse();
	afx_msg void OnApplySelective();
	afx_msg void OnBlendNoise();
	afx_msg void OnChangeInclusionString();
	afx_msg void OnMoreOptions();
	virtual BOOL OnInitDialog();
	afx_msg void OnContaining();
	afx_msg void OnNotContaining();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	const char *InitialPathname;				// Pathname used to indicate default browsing directory.
			char  DirectoryName [_MAX_DIR];	// Directory containing solve files.
			char *FileListBuffer;				// Buffer to store list of solve filenames.
			char *FilenameList;					// Ptr to list of solve filenames (separated by null terminating character).
			bool	ApplySelective;
			bool	InvertSelection;
			bool	BlendNoise;
			char *InclusionString;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INSERTSOLVEDIALOG_H__6F280B62_836D_11D4_A3D2_009027447394__INCLUDED_)
