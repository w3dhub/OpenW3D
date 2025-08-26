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

// OutputFormClass.cpp : implementation file
//

#include "StdAfx.h"
#include "LevelEdit.h"
#include "OutputForm.h"
#include "Utils.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// OutputFormClass

//IMPLEMENT_DYNCREATE(OutputFormClass, DockableFormClass)

/////////////////////////////////////////////////////////////////////////////
//
// OutputFormClass
//
/////////////////////////////////////////////////////////////////////////////
OutputFormClass::OutputFormClass()
	: DockableFormClass(OutputFormClass::IDD)
{
	//{{AFX_DATA_INIT(OutputFormClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	//
	//	Create the log file
	//
	char filename[MAX_PATH];
	::GetModuleFileName (NULL, filename, sizeof (filename));

	CString directory = ::Strip_Filename_From_Path (filename);
	CString full_path = Make_Path (directory, "_editorlog.txt");

	//
	//	Try to open the log file
	//
	HANDLE file = ::CreateFile (	full_path,
											GENERIC_WRITE,
											FILE_SHARE_READ,
											NULL,
											OPEN_ALWAYS,
											0L,
											NULL);

	//
	//	If we succeeded then pass the handle onto our file object
	//
	if (file != INVALID_HANDLE_VALUE) {
		LogFile.Attach (file);
	}
	
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// ~OutputFormClass
//
/////////////////////////////////////////////////////////////////////////////
OutputFormClass::~OutputFormClass (void)
{
	LogFile.Close ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// DoDataExchange
//
void
OutputFormClass::DoDataExchange(CDataExchange* pDX)
{
	DockableFormClass::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(OutputFormClass)
	DDX_Control(pDX, IDC_OUTPUT_EDIT, m_OutputEdit);
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(OutputFormClass, DockableFormClass)
	//{{AFX_MSG_MAP(OutputFormClass)
	ON_WM_CREATE()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// OutputFormClass diagnostics

#ifdef _DEBUG
void OutputFormClass::AssertValid() const
{
	DockableFormClass::AssertValid();
}

void OutputFormClass::Dump(CDumpContext& dc) const
{
	DockableFormClass::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// OutputFormClass message handlers


/////////////////////////////////////////////////////////////////////////////
//
// OnCreate
//
int
OutputFormClass::OnCreate (LPCREATESTRUCT lpCreateStruct) 
{
    // Allow the base class to process this message	
	if (DockableFormClass::OnCreate(lpCreateStruct) == -1) {
		return -1;
	}
	
	return 0;
}


/////////////////////////////////////////////////////////////////////////////
//
// HandleInitDialog
//
void
OutputFormClass::HandleInitDialog (void) 
{
	//
	//	TODO - Any initialization
	//
	m_OutputEdit.SetTabStops (10);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnSize
//
void
OutputFormClass::OnSize
(
	UINT nType,
	int cx,
	int cy
)
{
	// Allow the base class to process this message	
	DockableFormClass::OnSize (nType, cx, cy);

	if (::IsWindow (m_OutputEdit) && (cx > 0) && (cy > 0)) {
		// Resize the tab control to fill the entire contents of the client area
		m_OutputEdit.SetWindowPos (NULL, 0, 0, cx, cy, SWP_NOZORDER | SWP_NOMOVE);	
	}
	return ;	
}


/////////////////////////////////////////////////////////////////////////////
//
// Output_Message
//
void
OutputFormClass::Output_Message (LPCTSTR new_message)
{
	//
	// Turn off repainting
	//
	m_OutputEdit.SetRedraw (FALSE);

	// Get the text currently displayed in the window
	TCHAR window_text[32768] = { 0 };
	int curr_text_len = m_OutputEdit.GetWindowText (window_text, sizeof (window_text)) + 1;

	// Get the new text length
	int new_text_len = ::lstrlen (new_message);

	// Do we need to scroll text out of the buffer?
	if (curr_text_len + new_text_len > (32768 - 16)) {
		// Move the first 4K out of the buffer
		::memmove (window_text, &window_text[4096], curr_text_len-4096);
	}

	// Concat the new message onto the current window text.
	::lstrcat (window_text, new_message);

	// Make sure the message has a line feed
	if (	new_message[new_text_len-2] != '\r' &&
			new_message[new_text_len-1] == '\n')
	{
		int text_len = ::lstrlen (window_text);

		window_text[text_len-1]	= '\r';
		window_text[text_len]	= '\n';
		window_text[text_len+1]	= 0;
	}		

	// Put the text back into the edit control
	m_OutputEdit.SetWindowText (window_text);
	int line_count = m_OutputEdit.GetLineCount ();
	m_OutputEdit.LineScroll (line_count-4);

	//
	// Turn painting back on
	//
	m_OutputEdit.SetRedraw (TRUE);

	if (LogFile.Is_Open ()) {
		LogFile.Write ((const char *)new_message, ::lstrlen (new_message));
	}

	return ;
}
