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

/* $Header: /Commando/Code/Tools/ViewTrans/viewtrans.cpp 3     7/06/98 6:28p Greg_h $ */
/*********************************************************************************************** 
 ***                            Confidential - Westwood Studios                              *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : Trnasformation Viewer Utility                                * 
 *                                                                                             * 
 *                    File Name : VIEWTRANS.CPP                                                * 
 *                                                                                             * 
 *                   Programmer : Greg Hjelstrom                                               * 
 *                                                                                             * 
 *                   Start Date : 02/24/97                                                     * 
 *                                                                                             * 
 *                  Last Update : February 25, 1997 [GH]                                       * 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 *   UtilityDlgProc -- Dialog Proc to handle all of the dialog's windows messages              * 
 *   TVU::TransViewerUtility -- Constructor                                                    * 
 *   TVU::OnInitDialog -- Initializes the custom controls.                                     * 
 *   TVU::BeginEditParams -- Adds the rollup to the control panel                              * 
 *   TVU::EndEditParams -- Removes the rollup from the panel                                   * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include <MAX.H>
#include <UTILAPI.H>
#include "viewtrans.h"
#include "resource.h"


/*****************************************************************************
** Globals
*/

HINSTANCE			hInstance;
TransViewerUtility	TheUtility;
UtilityClassDesc	UtilityDesc;




/*********************************************************************************************** 
 * UtilityDlgProc -- Dialog Proc to handle all of the dialog's windows messages                * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   02/25/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
BOOL CALLBACK UtilityDlgProc(HWND hDlg, UINT message, WPARAM wParam,LPARAM lParam)
{
	TransViewerUtility *to = &TheUtility;

	// Respond to the message type...
	switch (message) {

		case WM_INITDIALOG:
			// Initialize all our Custom Controls
			TheUtility.OnInitDialog(hDlg, lParam);
		 	return true;

		case WM_DESTROY:
			// Release all our Custom Controls
			return false;

		case WM_LBUTTONDOWN: case WM_LBUTTONUP:	case WM_MOUSEMOVE:
			// Pass these messages on to the RollupMouseMessage method
			// to allow 'hand cursor' scrolling with unused area of the
			// rollup page.
   			to->ip->RollupMouseMessage(hDlg, message, wParam, lParam);
			break;

		case WM_MOUSEACTIVATE:
			to->ip->RealizeParamPanel();
			return false;

		default:
			break;
	}
	return false;
}



/*********************************************************************************************** 
 * TVU::TransViewerUtility -- Constructor                                                      * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   02/25/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
TransViewerUtility::TransViewerUtility(void) 
{
	iu = NULL;
	ip = NULL;	
	hPanel = NULL;	
}


/*********************************************************************************************** 
 * TVU::BeginEditParams -- Adds the rollup to the control panel                                * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   02/25/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
void TransViewerUtility::BeginEditParams(Interface *ip, IUtil *iu) 
{
	this->iu = iu;
	this->ip = ip;

	// Add the rollup page to the command panel.
	hPanel = ip->AddRollupPage(
		hInstance, 
		MAKEINTRESOURCE(IDD_VIEWTRANS),
		UtilityDlgProc,
		ROLLUP_PAGE_TITLE, 
		(LPARAM)this);		

	
	// update the display
	SelectionSetChanged(ip,iu);
}
	
/*********************************************************************************************** 
 * TVU::EndEditParams -- Removes the rollup from the panel                                     * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   02/25/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
void TransViewerUtility::EndEditParams(Interface *ip, IUtil *iu) 
{
	// Delete the rollup page
	ip->DeleteRollupPage(hPanel);		

	// The panel index is only valid within BeginEditParams and 
	// EndEditParams.  Set it to null for safety.
	hPanel = NULL;				
}

/*********************************************************************************************** 
 * TVU::OnInitDialog -- Initializes the custom controls.                                       * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   02/25/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
void TransViewerUtility::OnInitDialog(HWND hDlg, LPARAM lParam)
{
	
}


void TransViewerUtility::SelectionSetChanged(Interface *ip,IUtil *iu)
{
	// If there is not one and only one selected, clear
	// the display with zeros.
	if (ip->GetSelNodeCount() != 1) {
		Display_Data(
			Matrix3(1),
			Matrix3(1),
			Point3(0.0f,0.0f,0.0f),
			Quat(0.0f,0.0f,0.0f,0.0f),
			ScaleValue(Point3(1.0f,1.0f,1.0f))
		);
		return;
	}

	// Get all sorts of info about this node!
	INode * node = ip->GetSelNode(0);
	Matrix3 nodetm = node->GetNodeTM(ip->GetTime());
	Matrix3 objtm = node->GetObjectTM(ip->GetTime());
	Point3  objoffpos = node->GetObjOffsetPos();
	Quat    objoffrot = node->GetObjOffsetRot();
	ScaleValue objoffscl = node->GetObjOffsetScale();

	// display on the panel
	Display_Data(nodetm,objtm,objoffpos,objoffrot,objoffscl);
}


void TransViewerUtility::Display_Data
(
	Matrix3 &nodetm,
	Matrix3 &objtm,
	Point3  &objoffpos,
	Quat    &objoffrot,
	ScaleValue &objoffscl
)
{
	char string[256];
	Point3 vect;

	//////////////////////////////////////////
	// 3x3 sub-matrix of the ObjectTM
	//////////////////////////////////////////
	vect = objtm.GetRow(0);
	sprintf(string,"%5.3f",vect.x);
	SetWindowText(GetDlgItem(hPanel, IDC_OBJTM_0),string);
	sprintf(string,"%5.3f",vect.y);
	SetWindowText(GetDlgItem(hPanel, IDC_OBJTM_1),string);
	sprintf(string,"%5.3f",vect.z);
	SetWindowText(GetDlgItem(hPanel, IDC_OBJTM_2),string);

	vect = objtm.GetRow(1);
	sprintf(string,"%5.3f",vect.x);
	SetWindowText(GetDlgItem(hPanel, IDC_OBJTM_3),string);
	sprintf(string,"%5.3f",vect.y);
	SetWindowText(GetDlgItem(hPanel, IDC_OBJTM_4),string);
	sprintf(string,"%5.3f",vect.z);
	SetWindowText(GetDlgItem(hPanel, IDC_OBJTM_5),string);

	vect = objtm.GetRow(2);
	sprintf(string,"%5.3f",vect.x);
	SetWindowText(GetDlgItem(hPanel, IDC_OBJTM_6),string);
	sprintf(string,"%5.3f",vect.y);
	SetWindowText(GetDlgItem(hPanel, IDC_OBJTM_7),string);
	sprintf(string,"%5.3f",vect.z);
	SetWindowText(GetDlgItem(hPanel, IDC_OBJTM_8),string);

	
	///////////////////////////////////////////
	// Translation portion of the ObjectTM
	///////////////////////////////////////////
	vect = objtm.GetTrans();
	sprintf(string,"%5.3f",vect.x);
	SetWindowText(GetDlgItem(hPanel, IDC_OBJTM_9),string);
	sprintf(string,"%5.3f",vect.y);
	SetWindowText(GetDlgItem(hPanel, IDC_OBJTM_10),string);
	sprintf(string,"%5.3f",vect.z);
	SetWindowText(GetDlgItem(hPanel, IDC_OBJTM_11),string);

	///////////////////////////////////////////
	// 3x3 sub-matrix of the NodeTM
	///////////////////////////////////////////
	vect = objtm.GetRow(0);
	sprintf(string,"%5.3f",vect.x);
	SetWindowText(GetDlgItem(hPanel, IDC_NODETM_0),string);
	sprintf(string,"%5.3f",vect.y);
	SetWindowText(GetDlgItem(hPanel, IDC_NODETM_1),string);
	sprintf(string,"%5.3f",vect.z);
	SetWindowText(GetDlgItem(hPanel, IDC_NODETM_2),string);

	vect = objtm.GetRow(1);
	sprintf(string,"%5.3f",vect.x);
	SetWindowText(GetDlgItem(hPanel, IDC_NODETM_3),string);
	sprintf(string,"%5.3f",vect.y);
	SetWindowText(GetDlgItem(hPanel, IDC_NODETM_4),string);
	sprintf(string,"%5.3f",vect.z);
	SetWindowText(GetDlgItem(hPanel, IDC_NODETM_5),string);

	vect = objtm.GetRow(2);
	sprintf(string,"%5.3f",vect.x);
	SetWindowText(GetDlgItem(hPanel, IDC_NODETM_6),string);
	sprintf(string,"%5.3f",vect.y);
	SetWindowText(GetDlgItem(hPanel, IDC_NODETM_7),string);
	sprintf(string,"%5.3f",vect.z);
	SetWindowText(GetDlgItem(hPanel, IDC_NODETM_8),string);
	
	///////////////////////////////////////////
	// Translation portion of the NodeTM
	///////////////////////////////////////////
	vect = nodetm.GetTrans();
	sprintf(string,"%5.3f",vect.x);
	SetWindowText(GetDlgItem(hPanel, IDC_NODETM_9),string);
	sprintf(string,"%5.3f",vect.y);
	SetWindowText(GetDlgItem(hPanel, IDC_NODETM_10),string);
	sprintf(string,"%5.3f",vect.z);
	SetWindowText(GetDlgItem(hPanel, IDC_NODETM_11),string);

	///////////////////////////////////////////
	// Object-Offset translation:
	///////////////////////////////////////////
	vect = objoffpos;
	sprintf(string,"%5.3f",vect.x);
	SetWindowText(GetDlgItem(hPanel, IDC_OBJOFF_TRANS_X),string);
	sprintf(string,"%5.3f",vect.y);
	SetWindowText(GetDlgItem(hPanel, IDC_OBJOFF_TRANS_Y),string);
	sprintf(string,"%5.3f",vect.z);
	SetWindowText(GetDlgItem(hPanel, IDC_OBJOFF_TRANS_Z),string);

	///////////////////////////////////////////
	// Object-Offset scaling:
	///////////////////////////////////////////
	vect = objoffscl.s;
	sprintf(string,"%5.3f",vect.x);
	SetWindowText(GetDlgItem(hPanel, IDC_OBJOFF_SCALE_X),string);
	sprintf(string,"%5.3f",vect.y);
	SetWindowText(GetDlgItem(hPanel, IDC_OBJOFF_SCALE_Y),string);
	sprintf(string,"%5.3f",vect.z);
	SetWindowText(GetDlgItem(hPanel, IDC_OBJOFF_SCALE_Z),string);

	///////////////////////////////////////////
	// Object-Offset Quaternion:
	///////////////////////////////////////////
	sprintf(string,"%5.3f",objoffrot[0]);
	SetWindowText(GetDlgItem(hPanel, IDC_OBJOFF_ROT_X),string);
	sprintf(string,"%5.3f",objoffrot[1]);
	SetWindowText(GetDlgItem(hPanel, IDC_OBJOFF_ROT_Y),string);
	sprintf(string,"%5.3f",objoffrot[2]);
	SetWindowText(GetDlgItem(hPanel, IDC_OBJOFF_ROT_Z),string);
	sprintf(string,"%5.3f",objoffrot[3]);
	SetWindowText(GetDlgItem(hPanel, IDC_OBJOFF_ROT_W),string);


}
