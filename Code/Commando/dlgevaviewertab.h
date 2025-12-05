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
 *                     $Archive:: /Commando/Code/commando/dlgevaviewertab.h       $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 11/09/01 6:03p                                              $*
 *                                                                                             *
 *                    $Revision:: 9                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __DLG_EVA_VIEWER_TAB_H
#define __DLG_EVA_VIEWER_TAB_H

#include "childdialog.h"
#include "wwstring.h"
#include "widestring.h"
#include "vector.h"
#include "encyclopediamgr.h"
#include "playertype.h"


////////////////////////////////////////////////////////////////
//	Forward declarations
////////////////////////////////////////////////////////////////
class ListCtrlClass;
class DialogTextClass;
class ViewerCtrlClass;


////////////////////////////////////////////////////////////////
//
//	EvaViewerObjectClass
//
////////////////////////////////////////////////////////////////
class EvaViewerObjectClass
{
public:

	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	EvaViewerObjectClass (void)	:
		ID (0),
		PlayerType (PLAYERTYPE_NEUTRAL),
		MinDistance (0)				{}
	~EvaViewerObjectClass (void)	{}

	////////////////////////////////////////////////////////////////
	//	Public operators
	////////////////////////////////////////////////////////////////
	bool				operator== (const EvaViewerObjectClass &src) const	{ return false; }
	bool				operator!= (const EvaViewerObjectClass &src)	const	{ return true; }

	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////
	int				Get_ID (void) const							{ return ID; }
	const wchar_t *	Get_Name (void) const						{ return	Name; }
	const wchar_t *	Get_Description (void) const				{ return Description; }
	const wchar_t *	Get_Affiliation (void) const				{ return Affiliation; }
	const char *	Get_Model_Name (void) const				{ return ModelName; }	
	const char *	Get_Anim_Name (void) const					{ return AnimName; }		
	const char *	Get_Definition_Name (void) const			{ return DefinitionName; }
	float				Get_Min_Distance (void) const				{ return MinDistance; }
	int				Get_Player_Type (void) const				{ return PlayerType; }

	void				Set_ID (int id)								{ ID = id; }
	void				Set_Name (const wchar_t *name)				{ Name = name; }
	void				Set_Description (const wchar_t *string)	{ Description = string; }
	void				Set_Affiliation (const wchar_t *string)	{ Affiliation = string; }
	void				Set_Model_Name (const char *name)		{ ModelName = name; }	
	void				Set_Anim_Name (const char *name)			{ AnimName = name; }
	void				Set_Definition_Name (const char *name)	{ DefinitionName = name; }
	void				Set_Min_Distance (float dist)				{ MinDistance = dist; }
	void				Set_Player_Type (int type)					{ PlayerType = type; }

protected:

	////////////////////////////////////////////////////////////////
	//	Protected member data
	////////////////////////////////////////////////////////////////
	WideStringClass	Name;
	WideStringClass	Description;
	WideStringClass	Affiliation;
	StringClass			ModelName;
	StringClass			AnimName;
	StringClass			DefinitionName;
	float					MinDistance;
	int					ID;
	int					PlayerType;
};


////////////////////////////////////////////////////////////////
//
//	EvaViewerTabClass
//
////////////////////////////////////////////////////////////////
class EvaViewerTabClass : public ChildDialogClass
{
public:

	////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////
	EvaViewerTabClass (const DialogResource *dialog_resource);


	////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////

	//
	//	Data
	//
	void		Set_INI_Filename (const char *filename)		{ INIFilename = filename; }

	//
	//	Configuration
	//
	void		Set_List_Ctrl (ListCtrlClass *list_ctrl)		{ ListCtrl = list_ctrl; }
	void		Set_Description_Ctrl (DialogTextClass *ctrl)	{ DescriptionCtrl = ctrl; }
	void		Set_Affiliation_Ctrl (DialogTextClass *ctrl)	{ AffiliationCtrl = ctrl; }
	void		Set_Viewer_Ctrl (ViewerCtrlClass *ctrl)		{ ViewerCtrl = ctrl; }

	void		Set_Encyclopedia_Type (EncyclopediaMgrClass::TYPE type)	{ EncyclopediaType = type; }

	//
	//	Notifications
	//
	void		On_ListCtrl_Sel_Change (ListCtrlClass *list_ctrl, int ctrl_id, int old_index, int new_index) override;
	void		On_ViewerCtrl_Model_Loaded (ViewerCtrlClass *viewer_ctrl, int ctrl_id, RenderObjClass *model) override;
	
protected:

	////////////////////////////////////////////////////////////////
	//	Protected methods
	////////////////////////////////////////////////////////////////
	void		On_Init_Dialog (void) override;
	void		On_Destroy_Dialog (void);	
	void		View_Entry (int entry_index);

	void		Build_Object_List (void);
	void		Prepare_Model (RenderObjClass *model);

	////////////////////////////////////////////////////////////////
	//	Static methods
	////////////////////////////////////////////////////////////////
	static int CALLBACK ListSortCallback (ListCtrlClass *list_ctrl, int item_index1, int item_index2, uint32 user_param);

	////////////////////////////////////////////////////////////////
	//	Protected member data
	////////////////////////////////////////////////////////////////
	ListCtrlClass *	ListCtrl;
	DialogTextClass *	DescriptionCtrl;
	DialogTextClass *	AffiliationCtrl;
	ViewerCtrlClass *	ViewerCtrl;
	StringClass			INIFilename;
	
	EncyclopediaMgrClass::TYPE	EncyclopediaType;

	DynamicVectorClass<EvaViewerObjectClass> ObjectList;
};


#endif //__DLG_EVA_VIEWER_TAB_H

