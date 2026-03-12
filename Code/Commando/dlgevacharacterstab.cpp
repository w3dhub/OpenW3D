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
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Combat																		  *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/commando/dlgevacharacterstab.cpp    $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 9/10/01 8:21a                                               $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "dlgevacharacterstab.h"



////////////////////////////////////////////////////////////////
//
//	On_Init_Dialog
//
////////////////////////////////////////////////////////////////
void
EvaCharactersTabClass::On_Init_Dialog (void)
{
	Set_Encyclopedia_Type (EncyclopediaMgrClass::TYPE_CHARACTER);

	ListCtrlClass *list_ctrl				= (ListCtrlClass *)Get_Dlg_Item (IDC_LIST_CTRL);
	DialogTextClass *affiliation_ctrl	= (DialogTextClass *)Get_Dlg_Item (IDC_AFFILIATION_STATIC);
	DialogTextClass *description_ctrl	= (DialogTextClass *)Get_Dlg_Item (IDC_DESCRIPTION_STATIC);
	ViewerCtrlClass *viewer_ctrl			= (ViewerCtrlClass *)Get_Dlg_Item (IDC_VIEWER_CTRL);

	//
	//	Let the base class know which controls to use
	//
	Set_List_Ctrl (list_ctrl);
	Set_Description_Ctrl (description_ctrl);
	Set_Affiliation_Ctrl (affiliation_ctrl);
	Set_Viewer_Ctrl (viewer_ctrl);

	//
	//	Let the base class know where to get its data
	//
	Set_INI_Filename ("characters.ini");

	EvaViewerTabClass::On_Init_Dialog ();
	return ;
}
