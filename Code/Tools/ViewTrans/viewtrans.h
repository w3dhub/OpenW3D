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

/* $Header: /Commando/Code/Tools/ViewTrans/viewtrans.h 3     7/06/98 6:27p Greg_h $ */
/*********************************************************************************************** 
 ***                            Confidential - Westwood Studios                              *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : Matrix Viewer Utility                                        * 
 *                                                                                             * 
 *                    File Name : VIEWTRANS.H                                                  * 
 *                                                                                             * 
 *                   Programmer : Greg Hjelstrom                                               * 
 *                                                                                             * 
 *                   Start Date : 02/25/97                                                     * 
 *                                                                                             * 
 *                  Last Update : February 25, 1997 [GH]                                       * 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef VIEWTRANS_H
#define VIEWTRANS_H

#include <Max.h>
#include <Quat.h>
#include <UTILAPI.H>



/****************************************************************************
** Globals
*/

class TransViewerUtility;
class UtilityClassDesc;

extern HINSTANCE			hInstance;
extern TransViewerUtility	TheUtility;
extern UtilityClassDesc		UtilityDesc;

#define CATEGORY_NAME _T("Westwood Tools")
#define ROLLUP_PAGE_TITLE _T("Transformation Viewer")
#define VIEWTRANS_CLASS_NAME _T("Transformation Viewer")
#define VIEWTRANS_CLASS_ID Class_ID(0x4e607ad, 0x155612cc)


/*****************************************************************************
*
*  Class declaration for TransViewerUtility.
*
*  This utility simply displays the transformation matrices of the currently
*  selected node.  
*
*****************************************************************************/
class TransViewerUtility : public UtilityObj 
{
public:

	TransViewerUtility();
	void OnInitDialog(HWND hWnd, LPARAM lParam);
	void BeginEditParams(Interface *ip, IUtil *iu);
	void EndEditParams(Interface *ip, IUtil *iu);
	void SelectionSetChanged(Interface *ip,IUtil *iu);
	void DeleteThis() {}

	void Display_Data
	(
		Matrix3 &nodetm,
		Matrix3 &objtm,
		Point3  &objoffpos,
		Quat    &objoffrot,
		ScaleValue &objoffscl
	);

	// Keep ahold of pointer to our interfaces to MAX.
	IUtil *iu;
	Interface *ip;
	
	// Window handle to the dialog box.
	HWND hPanel;
	
private:	

	// windows Dialog Proc which manipulates this class's data.
	friend BOOL CALLBACK UtilityDlgProc(HWND hDlg, UINT message, 
		WPARAM wParam, LPARAM lParam);

};


/*****************************************************************************
*
* Class Description 
*
*****************************************************************************/
class UtilityClassDesc : public ClassDesc 
{
public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = false) {return &TheUtility;}
	const TCHAR *	ClassName() {return VIEWTRANS_CLASS_NAME;}
	SClass_ID		SuperClassID() {return UTILITY_CLASS_ID;}
	Class_ID		ClassID() {return VIEWTRANS_CLASS_ID;}
	const TCHAR* 	Category() {return CATEGORY_NAME;}
};



#endif /*VIEWTRANS_H*/