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
 *                     $Modtime:: 7/19/01 7:32p       $*
 *                                                                                             *
 *                    $Revision:: 8                                                         $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef __AFXWIN_H__
#error include 'stdafx.h' before including this file for PCH
#endif

// Includes.
#include "resource.h"
#include "LightMapDoc.h"
#include <stdlib.h>


class LightMapApp : public CWinApp
{
	public:

		// Functions.
		char  *Working_Path()		  {return (WorkingPath);}
		char  *Instance_Name()		  {return (InstanceName);}
		DWORD	 Application_Version() {return (ApplicationVersion);}

		static bool			 Explorer_Style();
		static DWORD	    File_Dialog_Flags();

		static const char *File_Dialog_Filter()
		{
			return ("Westwood 3D Files (*.w3d)|*.w3d|All Files (*.*)|*.*||");
		}

		static const char *Document_File_Extension()
		{
			return (".w3d");
		}

	protected:

		// Functions.
		LightMapDoc* GetDoc();

	private:

		void  Do_Version_Check();

		char  WorkingPath [_MAX_PATH];	// Path of executing application.
		char  InstanceName [_MAX_PATH];	// Name unique to this instance of the application.

		DWORD ApplicationVersion;			// Version no. of this application.

// The following is maintained by MFC tools.
public:
	LightMapApp();
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(LightMapApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(LightMapApp)
	afx_msg void OnAppAbout();
	afx_msg void OnFileOpen();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
protected:
};


// Externals.
extern LightMapApp theApp;



