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
 *                 Project Name : Max2W3d                                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/max2w3d/GMaxMtlDlg.h                   $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 3/19/02 4:18p                                               $*
 *                                                                                             *
 *                    $Revision:: 9                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */




#ifndef GMAXMTLDLG_H
#define GMAXMTLDLG_H
#if defined W3D_GMAXDEV

class GameMtl;
class GameMtlPassDlg;

#include "imtledit.h"
#include "FPMatNav.h"
#include "Ref.h"
#include "Notify.h"


////////////////////////////////////////////////////////////////////////
// GMaxMtlDlg
//
// Dialog box interface in the material editor for GameMtl
//	This is basically a cannibalized version of the Standard
// Max material's dialog.
//
////////////////////////////////////////////////////////////////////////
#define FPMATNAV_INTERFACE    Interface_ID(0x60151ad4, 0x3b325af3)

class GMaxMtlDlg: public ParamDlg , public IFPMatNav //,public ReferenceMaker
{
public:
	void DestroyDialog();
	
	////////////////////////////////////////////////////////////////////////
	// Methods
	////////////////////////////////////////////////////////////////////////
	GMaxMtlDlg(HWND hwMtlEdit, IMtlParams *imp, GameMtl *m); 
	~GMaxMtlDlg();

//============================================================================================
	virtual void Launch(void);
	virtual void SetMultiMaterialTabBySlot( int iSlot, int iSubMtlTexIndex, int iElement );
	virtual void SetMaterialTabBySlot( int iSlot, int iElement );
	virtual void SetMultiMaterialBySlot( int iSlot, int iIndex );
	virtual void SetMaterialBySlot( int iSlot );
//============================================================================================
	// From ParamDlg:
	Class_ID				ClassID(void);
	void					SetThing(ReferenceTarget *m);
	ReferenceTarget*	GetThing(void) { return (ReferenceTarget*)TheMtl; }
	void					DeleteThis() { delete this;  }	
	void					SetTime(TimeValue t);
	void					ReloadDialog(void);
	void					ActivateDlg(BOOL onOff);

	void					Invalidate(void);
	void					Update_Display(void)	{GetCOREInterface()->ForceCompleteRedraw(true);}
	void					Reset(GameMtl* mtl);
	void					Reinitialize(GameMtl* new_mtl, bool update_multimtl = true);
	void					SetMultimaterial(Mtl* mtl){Game_multi_mtl = mtl; Current_Submtl_Index = 0;}
	void					DoMaterialNavigator();
	void					DoGetMaterial();
	void					ApplyToSelection(GameMtl* mtl);
	void					DoNewMaterial();
	void					DeleteMtl();
	void					DeleteAllSceneMtl();
	void					AddToolTip(HWND hControl, UINT strID);
	void					LoadButtonBitmaps(HWND hDlg);
	void					DeleteNodeMaterial(INode* node, MtlBase* mtl);
	void					ShowHideControls();
	void					NextSibling();
	void					PreviousSibling();
	void					SetCurrentPage(int i){CurrentPage = i;}
	bool					HasMultiMtl(){return Game_multi_mtl != NULL;}
	void					SetMtl(GameMtl* mtl){TheMtl = mtl;}
	GameMtl*				GetMtl(){return TheMtl;}
	bool					IsVisible(){ return 	HwndEdit &&	IsWindowVisible(HwndEdit);}
	GameMtl*				ConvertStdMtl(Mtl* stdmtl);
	////////////////////////////////////////////////////////////////////////
	// Max interface pointer
	////////////////////////////////////////////////////////////////////////

	Interface* Ip;

protected:

	void					Build_Dialog(HWND hParent = NULL);
	void					Make_PropertySheet(HWND hParent);
	void					Make_Floater(HWND hParent);
	Mtl*					Game_multi_mtl;
	HBITMAP				HGetMtlBmp;
	HBITMAP				HBrowseMtlBmp;
	HBITMAP				HAssignMtlBmp;
	HBITMAP				HDeleteMtlBmp;
	HBITMAP				HNewMtlBmp;
	HBITMAP				HNextSiblingBmp;
	HBITMAP				HPreviousSiblingBmp;
	int					Current_Submtl_Index;	//
	bool					DontShowMtlType;			//
	bool					DontShowDeleteAll;			//
	int					CurrentPage;				//Which prop page is currently active
	BOOL					DisplacementMapProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	BOOL					SurfaceTypeProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	BOOL					PassCountProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	void					Set_Pass_Count_Dialog(void);

	enum { MAX_PASSES = 4 };

	////////////////////////////////////////////////////////////////////////
	// Windows handles
	////////////////////////////////////////////////////////////////////////
	HWND					HwndEdit;		// window handle of the materials editor dialog
	HWND					HwndPassCount;	// Rollup pass count panel
	HWND					HwndSurfaceType;	// Rollup surface type panel
	HWND					HwndDisplacementMap;
	HPALETTE				HpalOld;

	GameMtlPassDlg *	PassDialog[MAX_PASSES];	

	////////////////////////////////////////////////////////////////////////
	// Material dialog interface
	////////////////////////////////////////////////////////////////////////
	IMtlParams *		IParams;			// interface to the material editor
	GameMtl *			TheMtl;			// current mtl being edited.
	
	////////////////////////////////////////////////////////////////////////
	// Member variables
	////////////////////////////////////////////////////////////////////////
	TimeValue			CurTime;
	int					IsActive;
	friend BOOL CALLBACK DisplacementMapDlgProc(HWND, UINT, WPARAM,LPARAM);
	friend BOOL CALLBACK SurfaceTypePanelDlgProc(HWND, UINT, WPARAM,LPARAM);
	friend BOOL CALLBACK PassCountPanelDlgProc(HWND, UINT, WPARAM,LPARAM);
	friend class GameMtl;
};
	GameMtl*				GetMtlFromSelection();
#endif
#endif