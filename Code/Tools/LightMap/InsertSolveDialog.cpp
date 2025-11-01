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
 *                     $Modtime:: 1/24/01 3:40p       $* 
 *                                                                                             * 
 *                    $Revision:: 3                                                         $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

// Includes.
#include "StdAfx.h"
#include "LightMap.h"
#include "InsertSolveDialog.h"
#include "OptionsDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


InsertSolveDialog::InsertSolveDialog (const char *initialpathname, CWnd* pParent /*=NULL*/)
	: CDialog(InsertSolveDialog::IDD, pParent)
{
	InitialPathname = initialpathname;
	*DirectoryName	 = '\0';	
	FileListBuffer	 = NULL;
	FilenameList	 = NULL;	
	ApplySelective	 = false;
	InvertSelection = false;
	BlendNoise		 = false;
	InclusionString = NULL;	

	//{{AFX_DATA_INIT(InsertSolveDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void InsertSolveDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(InsertSolveDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(InsertSolveDialog, CDialog)
	//{{AFX_MSG_MAP(InsertSolveDialog)
	ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
	ON_BN_CLICKED(IDC_APPLY_SELECTIVE, OnApplySelective)
	ON_BN_CLICKED(IDC_BLEND_NOISE, OnBlendNoise)
	ON_EN_CHANGE(IDC_INCLUSION_STRING, OnChangeInclusionString)
	ON_BN_CLICKED(IDC_MORE_OPTIONS, OnMoreOptions)
	ON_BN_CLICKED(IDC_CONTAINING, OnContaining)
	ON_BN_CLICKED(IDC_NOT_CONTAINING, OnNotContaining)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/***********************************************************************************************
 * InsertSolveDialog::OnMoreOptions --																			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/06/00    IML : Created.                                                                * 
 *=============================================================================================*/
BOOL InsertSolveDialog::OnInitDialog() 
{
	char inclusionstring [2] = "^";

	CDialog::OnInitDialog();
	
	CheckDlgButton (IDC_APPLY_SELECTIVE, ApplySelective);
	CheckDlgButton (IDC_CONTAINING, true);
	CheckDlgButton (IDC_NOT_CONTAINING, false);
	CheckDlgButton (IDC_BLEND_NOISE, BlendNoise);
	GetDlgItem (IDC_INCLUSION_STRING)->SetWindowText (inclusionstring);
	GetDlgItem (IDC_INCLUSION_STRING)->EnableWindow (ApplySelective);
	GetDlgItem (IDC_CONTAINING)->EnableWindow (ApplySelective);
	GetDlgItem (IDC_NOT_CONTAINING)->EnableWindow (ApplySelective);

	return (true);	// return true unless you set the focus to a control
		            // EXCEPTION: OCX Property Pages should return false
}


/***********************************************************************************************
 * InsertSolveDialog::OnBrowse --																				  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/06/00    IML : Created.                                                                * 
 *=============================================================================================*/
void InsertSolveDialog::OnBrowse() 
{
	const char		*filefilter = "Lightscape Solution Files (*.ls)|*.ls|All Files (*.*)|*.*||";	// Filter for supported solve file formats.
	const unsigned  filelistbuffersize = 4096;
		
	char  initialdrivename [_MAX_DRIVE];
	char  initialdirectoryname [_MAX_DIR];
	char  initialpath [_MAX_PATH];

	// Select one or more solve files.
	CFileDialog dialog (true, NULL, NULL, LightMapApp::File_Dialog_Flags() | OFN_ALLOWMULTISELECT, filefilter);
		
	// Allocate a buffer to contain the list of files selected.
	// NOTE: Add one to buffer size in case an extra null must be added (see below).
	FileListBuffer = new char [filelistbuffersize + 1];		  
	ASSERT (FileListBuffer != NULL);
	*FileListBuffer = '\0';
	dialog.m_ofn.lpstrFile = FileListBuffer;
	dialog.m_ofn.nMaxFile  = filelistbuffersize;

	// Specify initial path.
	_splitpath (InitialPathname, initialdrivename, initialdirectoryname, NULL, NULL);
	_makepath (initialpath, initialdrivename, initialdirectoryname, NULL, NULL);
	dialog.m_ofn.lpstrInitialDir = initialpath;

	if (dialog.DoModal() == IDOK) {
		
		char *cptr;
		char *windowtext, *windowtextptr;

		// NOTE: Old style (not Explorer style) dialog boxes separate strings in the file list buffer
		// with spaces rather than null terminators.
		if (!LightMapApp::Explorer_Style()) {

			// Replace all spaces in the string with null terminators.
			cptr = FileListBuffer;
			while (*cptr != '\0') {
				if (*cptr == ' ') *cptr = '\0';
				cptr++;
			}

			// Append an extra null to indicate end of list.
			*(cptr + 1)	= '\0';
		}

		// Must be room for directory name (including null terminator).
		ASSERT (dialog.m_ofn.nFileOffset < sizeof (DirectoryName));
			
		// Copy directory name from file list buffer and append a backslash.
		if (dialog.m_ofn.nFileOffset > 0) {
			strncpy (DirectoryName, FileListBuffer, dialog.m_ofn.nFileOffset);
			DirectoryName [dialog.m_ofn.nFileOffset - 1] = '\0';
			strcat (DirectoryName, "\\");
		} else {
			strcpy (DirectoryName, "");
		}

		FilenameList = FileListBuffer + dialog.m_ofn.nFileOffset;

		// Set the solve file list edit control.
		windowtext = new char [filelistbuffersize];
		ASSERT (windowtext != NULL);
		cptr = FilenameList;
		windowtextptr = windowtext;
		while (true) {
			if (*cptr != '\0') {
				*windowtextptr = *cptr;
			} else {
				if (*(cptr + 1) == '\0') {
					*windowtextptr = '\0';
					break;
				} else {
					*windowtextptr = ';';
				}
			}
			cptr++;
			windowtextptr++;
		}
		GetDlgItem (IDC_SOLVE_FILE_LIST)->SetWindowText (windowtext);

		// Clean-up.
		delete [] windowtext;

		GetDlgItem (IDOK)->EnableWindow();
	}
}


/***********************************************************************************************
 * InsertSolveDialog::OnApplySelective --																		  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/06/00    IML : Created.                                                                * 
 *=============================================================================================*/
void InsertSolveDialog::OnApplySelective() 
{
	ApplySelective = !ApplySelective;
	GetDlgItem (IDC_INCLUSION_STRING)->EnableWindow (ApplySelective);
	GetDlgItem (IDC_CONTAINING)->EnableWindow (ApplySelective);
	GetDlgItem (IDC_NOT_CONTAINING)->EnableWindow (ApplySelective);
}


/***********************************************************************************************
 * InsertSolveDialog::OnContaining --																			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/06/00    IML : Created.                                                                * 
 *=============================================================================================*/
void InsertSolveDialog::OnContaining() 
{
	InvertSelection = false;
}


/***********************************************************************************************
 * InsertSolveDialog::OnNotContaining --																		  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/06/00    IML : Created.                                                                * 
 *=============================================================================================*/
void InsertSolveDialog::OnNotContaining() 
{
	InvertSelection = true;
}


/***********************************************************************************************
 * InsertSolveDialog::OnBlendNoise --																			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/06/00    IML : Created.                                                                * 
 *=============================================================================================*/
void InsertSolveDialog::OnBlendNoise() 
{
	BlendNoise = !BlendNoise;
}


/***********************************************************************************************
 * InsertSolveDialog::OnChangeInclusionString --															  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/06/00    IML : Created.                                                                * 
 *=============================================================================================*/
void InsertSolveDialog::OnChangeInclusionString() 
{
	int textlength;

	if (InclusionString != NULL) delete [] InclusionString;
	textlength = GetDlgItem (IDC_INCLUSION_STRING)->GetWindowTextLength() + 1;
	InclusionString = new char [textlength];
	ASSERT (InclusionString != NULL);
	GetDlgItem (IDC_INCLUSION_STRING)->GetWindowText (InclusionString, textlength);
}


/***********************************************************************************************
 * InsertSolveDialog::OnMoreOptions --																			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/06/00    IML : Created.                                                                * 
 *=============================================================================================*/
void InsertSolveDialog::OnMoreOptions() 
{
	OptionsDialog options;

	options.DoModal();
}


