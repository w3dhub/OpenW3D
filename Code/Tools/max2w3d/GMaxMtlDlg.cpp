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
 *                     $Archive:: /Commando/Code/Tools/max2w3d/GMaxMtlDlg.cpp                 $*
 *                                                                                             *
 *                       Author:: Moumine Ballo                                               *
 *                                                                                             *
 *                     $Modtime:: 4/05/02 11:10a                                              $*
 *                                                                                             *
 *                    $Revision:: 11                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#if defined W3D_GMAXDEV

#include <Max.h>
#include <gport.h>
#include <hsv.h>
#include <bmmlib.h>

#include "GMaxMtlDlg.h"
#include "GameMtl.h"
#include "GameMtlPassDlg.h"
#include "dllmain.h"
#include "resource.h"
#include "w3d_file.h"
#include "prsht.h"
#include "gameMtlForm.h"


extern GMaxMtlDlg * GMaxMaterialDialog;
#include <pshpack1.h>
typedef struct DLGTEMPLATEEX{
    WORD dlgVer;
    WORD signature;
    DWORD helpID;
    DWORD exStyle;
    DWORD style;
    WORD cDlgItems;
    short x;
    short y;
    short cx;
    short cy;
} DLGTEMPLATEEX, *LPDLGTEMPLATEEX;
#include <poppack.h>


//============================================================================================

#define WM_USER_UPDATE_MULTIMTL (WM_USER+141)

static BOOL CALLBACK DisplacementMapDlgProc(HWND hwndDlg, UINT msg, WPARAM wPara,LPARAM lParam);
static BOOL CALLBACK SurfaceTypePanelDlgProc(HWND hwndDlg, UINT msg, WPARAM wPara,LPARAM lParam);
static BOOL CALLBACK PassCountPanelDlgProc(HWND hwndDlg, UINT msg, WPARAM wPara,LPARAM lParam);
static BOOL CALLBACK PassCountDialogDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam,LPARAM lParam);
static _Num_Of_NoNames(0); //Tracks the number addded to new material name
GameMtl* ConvertStdMtl(Mtl* stdmtl);
bool _UsingLargeFonts(false);


// GMaxMtlDlg::GMaxMtlDlg -- constructor                                                       * 
//============================================================================================
GMaxMtlDlg::GMaxMtlDlg(HWND hwMtlEdit, IMtlParams *imp, GameMtl *m) 
{
	Ip = GetCOREInterface();
	HwndEdit = hwMtlEdit;
	HwndPassCount = NULL;
	HwndSurfaceType = NULL;
	HwndDisplacementMap = NULL;
	HpalOld = NULL;
	Game_multi_mtl = NULL;
	if(imp){
		Game_multi_mtl = ((FakeIMtlParams*)imp)->Multi_mtl_list;
	}
	HGetMtlBmp				= NULL;
	HBrowseMtlBmp			= NULL;
	HAssignMtlBmp			= NULL;
	HDeleteMtlBmp			= NULL;
	HNewMtlBmp				= NULL;
	HPreviousSiblingBmp  = NULL;
	HNextSiblingBmp		= NULL;
	DontShowMtlType		= false;
	DontShowDeleteAll    = false;
	fpMatNav.SetIFPMatNavCallback( this );

	for (int i=0; i<MAX_PASSES; i++) {
		PassDialog[i] = NULL;
	}

	TheMtl = m; 
	IParams = imp;
	IsActive = 0;

	Build_Dialog(hwMtlEdit);
}

// GMaxMtlDlg::~GMaxMtlDlg -- destructor!                                                       
//============================================================================================
GMaxMtlDlg::~GMaxMtlDlg() {
	for (int i=0; i<MAX_PASSES; i++) {
		if (PassDialog[i]) { 
			delete PassDialog[i];
			PassDialog[i] = NULL;
		}
	}
	TheMtl->SetParamDlg(NULL);
}

// GMaxMtlDlg::ClassID -- Returns the ClassID of GameMtl                                        
//============================================================================================
Class_ID GMaxMtlDlg::ClassID(){
	return GameMaterialClassID;  
}

// GMaxMtlDlg::SetThing -- Sets the material to be edited                                       
//============================================================================================
void GMaxMtlDlg::SetThing(ReferenceTarget *m) 
{
	assert (m);
	assert (m->SuperClassID()==MATERIAL_CLASS_ID);
	assert ((m->ClassID()==GameMaterialClassID) || (m->ClassID()==PS2GameMaterialClassID));
	assert (TheMtl);
	int pass;
	// destroy our old pass dialogs
	for (pass=0; pass<TheMtl->Get_Pass_Count();pass++) {
		delete PassDialog[pass];
		PassDialog[pass] = NULL;
	}
	// install the new material
	TheMtl->SetParamDlg(NULL);
	TheMtl = (GameMtl *)m;
	TheMtl->SetParamDlg(this);
	// build a new set of pass dialogs
	for (pass=0; pass<TheMtl->Get_Pass_Count(); pass++) {
		PassDialog[pass] = new GameMtlPassDlg(HwndEdit, IParams, TheMtl, pass);
	}
	// refresh the contents of the dialogs
	ReloadDialog();
}

// GMaxMtlDlg::SetTime -- Sets the time value, updates the material and the dialog              
//============================================================================================
void GMaxMtlDlg::SetTime(TimeValue t) 
{
	if (t!=CurTime) {
		CurTime = t;
//		TheMtl->Update(Ip->GetTime(),Valid);
		ReloadDialog();
	}
}
	
// GMaxMtlDlg::ReloadDialog -- Updates the values in all of the dialog's controls               
//============================================================================================
void GMaxMtlDlg::ReloadDialog() 
{
	//Init the pass count panel
	assert(TheMtl && HwndPassCount && HwndSurfaceType);
	// Init the surface count panel
	::SendMessage (HwndSurfaceType, WM_USER+101, 0, 0L);
	#ifdef WANT_DISPLACEMENT_MAPS
		::SendMessage (HwndDisplacementMap, WM_USER+101, 0, 0L);	
	#endif //WANT_DISPLACEMENT_MAPS
	// Init the pass count panel
	char a[12];
	sprintf(a, "%d", TheMtl->Get_Pass_Count());
	SetWindowText(GetDlgItem(HwndPassCount, IDC_GAMEMTL_PASSCOUNT_STATIC), a);	
	//Init each pass panel
	for(int i = 0; i < TheMtl->Get_Pass_Count(); i++){
		PassDialog[i]->ReloadDialog();
	}
}

// GMaxMtlDlg::ActivateDlg -- Activates and deactivates the dialog                              
//============================================================================================
void GMaxMtlDlg::ActivateDlg(BOOL onoff){
	for(int i = 0; i < TheMtl->Get_Pass_Count(); i++){
		assert(PassDialog[i]);
		PassDialog[i]->ActivateDlg(onoff);
	}
}

// GMaxMtlDlg::Invalidate -- causes the dialog to be redrawn                                    
//============================================================================================
void GMaxMtlDlg::Invalidate(){	
	InvalidateRect(HwndSurfaceType,NULL,0);
	#ifdef WANT_DISPLACEMENT_MAPS
		InvalidateRect(HwndDisplacementMap,NULL,0);	
	#endif //WANT_DISPLACEMENT_MAPS
	InvalidateRect(HwndPassCount,NULL,0);
}
BOOL	GMaxMtlDlg::DisplacementMapProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam){
	switch (message){
		case WM_INITDIALOG:
		case WM_USER + 101:{
			SetDlgItemInt (hDlg, IDC_AMOUNT_EDIT, TheMtl->Get_Displacement_Amount () * 100, true);
			SetupIntSpinner(hDlg, IDC_AMOUNT_SPIN, IDC_AMOUNT_EDIT, -999, 999, TheMtl->Get_Displacement_Amount () * 100);
			Texmap *map = TheMtl->Get_Displacement_Map ();
			if (map != NULL) {
				SetDlgItemText (hDlg, IDC_TEXTURE_BUTTON, map->GetFullName ());
			}
		break;
		}
		case CC_SPINNER_CHANGE:{
				ISpinnerControl *control = (ISpinnerControl *)lParam;
				TheMtl->Set_Displacement_Amount (((float)control->GetIVal ()) / 100.0F);
			break;
		}
		case WM_COMMAND:
			switch(LOWORD(wParam)){
			case IDC_TEXTURE_BUTTON:
				if(HIWORD(wParam) == BN_CLICKED){					
					PostMessage(HwndEdit, WM_TEXMAP_BUTTON, TheMtl->Get_Displacement_Map_Index (), (LPARAM)TheMtl);
				}
			break;
		}
	}
	return false;
}
//============================================================================================
BOOL	GMaxMtlDlg::SurfaceTypeProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam){
	switch (message){
		case WM_INITDIALOG:{
			//	Fill the combobox with the names of the different surface types
			for (int index = 0; index < SURFACE_TYPE_MAX; index ++){
				::SendDlgItemMessage (	hDlg,IDC_SURFACE_TYPE_COMBO,CB_ADDSTRING,0,(LPARAM)SURFACE_TYPE_STRINGS[index]);
			}
			// Limit the range of the static sort level spinner to 0 - MAX_SORT_LEVEL.
			int sort_level = TheMtl->Get_Sort_Level();
			SetupIntSpinner(hDlg, IDC_SORT_LEVEL_SPIN, IDC_SORT_LEVEL, 0, MAX_SORT_LEVEL, sort_level);
			// Check the checkbox if sort_level is not SORT_LEVEL_NONE.
			::SendDlgItemMessage(hDlg, IDC_ENABLE_SORT_LEVEL, BM_SETCHECK,
				sort_level == SORT_LEVEL_NONE ? BST_UNCHECKED : BST_CHECKED, 0);
			LoadButtonBitmaps(hDlg);
			//Show Hide the sub mat controls
			ShowHideControls();
			TSTR name_str = TheMtl->GetName();
			//SetDlgItemText(hDlg, IDC_MTLNAME, dbg_txt);//name_str.data());
			ICustEdit* pEdit = GetICustEdit(GetDlgItem(HwndSurfaceType, IDC_MTLNAME));
			pEdit->SetText(name_str.data());
			ReleaseICustEdit(pEdit);
			//Multi mat name
			if(Game_multi_mtl != NULL){
				ICustEdit* pEdit = GetICustEdit(GetDlgItem(hDlg, IDC_MULTIMTLNAME));
				pEdit->GetText(name_str.data(), MAX_PATH);
				ReleaseICustEdit(pEdit);
				Game_multi_mtl->SetName(name_str);
			}
		}
		case WM_USER + 101:{
			//	Select the current surface type
			::SendDlgItemMessage (	hDlg,IDC_SURFACE_TYPE_COMBO,CB_SETCURSEL,(WPARAM)TheMtl->Get_Surface_Type (),	0L);
			// Set the correct sort level
			int sort_level = TheMtl->Get_Sort_Level();
			ISpinnerControl *spinner = GetISpinner(::GetDlgItem(hDlg, IDC_SORT_LEVEL_SPIN));
			assert(spinner);
			spinner->SetValue(sort_level, false);
			::SendDlgItemMessage(hDlg, IDC_ENABLE_SORT_LEVEL, BM_SETCHECK,
				sort_level == SORT_LEVEL_NONE ? BST_UNCHECKED : BST_CHECKED, 0);
			break;
		}
		case WM_USER_UPDATE_MULTIMTL:{
			//Show Hide the sub mat controls
			ShowHideControls();
			break;
		}
		case WM_USER+140:{
			ShowWindow(HwndEdit, SW_HIDE);
			return true;
		}
		case WM_NOTIFY:{
			NMHDR * header = (NMHDR *)lParam;
			switch(header->code) { 
				case PSN_SETACTIVE:{
					SetCurrentPage(0);
					return true;
				}
				case PSN_RESET:{
					SetWindowLong(hDlg,DWL_MSGRESULT,false);
					::SendMessage(hDlg, WM_USER+140,0,0);
					return true;
				}
			
			}
			break;
		}
		case WM_COMMAND:{
			switch(LOWORD(wParam)){
				case IDC_SURFACE_TYPE_COMBO:{
					if(HIWORD(wParam) == CBN_SELCHANGE){
						unsigned int type = ::SendDlgItemMessage (hDlg, IDC_SURFACE_TYPE_COMBO, CB_GETCURSEL, 0, 0L);
						TheMtl->Set_Surface_Type (type);
					}
					break;
				}
				case IDC_ENABLE_SORT_LEVEL:{
					if (HIWORD(wParam) == BN_CLICKED){
						// If the 'enable' checkbox was unchecked, set the sort level to NONE.
						int state = ::IsDlgButtonChecked(hDlg, IDC_ENABLE_SORT_LEVEL);
						ISpinnerControl *spinner = GetISpinner(::GetDlgItem(hDlg, IDC_SORT_LEVEL_SPIN));
						assert(spinner);
						if (state == BST_UNCHECKED){
							spinner->SetValue(SORT_LEVEL_NONE, false);
							TheMtl->Set_Sort_Level(SORT_LEVEL_NONE);
						}else{
							if(state == BST_CHECKED){
							// Sort level was enabled, so set it's level to 1 if it was NONE before.
								if (spinner->GetIVal() == SORT_LEVEL_NONE){
									spinner->SetValue(1, false);
									TheMtl->Set_Sort_Level(1);
								}
							}
						}
					}
					break;
				}
				case IDC_SETPASSCOUNT:{
					if(HIWORD(wParam) == BN_CLICKED){
						Set_Pass_Count_Dialog();
					}
					break;
				}
				case IDC_ASSIGN:{
					ApplyToSelection(TheMtl);
					break;
				}
				case IDC_NEWMTL:{
					DoNewMaterial();
					break;
				}
				case IDC_MTLNAME:{
					if(HIWORD(wParam) == EN_CHANGE){
						TSTR name_str;
						char str[MAX_PATH];
						ICustEdit* pEdit = GetICustEdit(GetDlgItem(hDlg, IDC_MTLNAME));
						pEdit->GetText(str, MAX_PATH);
						ReleaseICustEdit(pEdit);
						name_str = str;
						TheMtl->SetName(name_str);
					}
					break;
				}
				case IDC_MULTIMTLNAME:{
					if(HIWORD(wParam) == EN_CHANGE){
						TSTR name_str;
						char str[MAX_PATH];
						ICustEdit* pEdit = GetICustEdit(GetDlgItem(hDlg, IDC_MULTIMTLNAME));
						pEdit->GetText(str, MAX_PATH);
						ReleaseICustEdit(pEdit);
						name_str = str;
						Game_multi_mtl->SetName(name_str);
					}
					break;
				}
				case IDC_NAVIGATOR:{
					DoMaterialNavigator();
					break;
				}
				case IDC_GETMTL:{
					DoGetMaterial();
					break;
				}
				case IDC_DELETEMTL:{
					DeleteMtl();
					break;
				}
				case IDC_DELETEALL:{
					DeleteAllSceneMtl();
					break;
				}
				case IDC_SUBMTL_NUM:{
					if(HIWORD(wParam) == EN_CHANGE){
						if(HwndPassCount != NULL){ //Only after initializations

							// (gth) MATERIAL BEING CHANGED, user edited the current submtl index
							ICustEdit* pEdit = GetICustEdit(GetDlgItem(HwndSurfaceType, IDC_SUBMTL_NUM));
							Current_Submtl_Index = pEdit->GetInt()-1 ;
							ReleaseICustEdit(pEdit);
							Mtl * new_mtl = Game_multi_mtl->GetSubMtl(Current_Submtl_Index);
							if(NULL != new_mtl && new_mtl != TheMtl && new_mtl->ClassID() == GameMaterialClassID) {
								Reinitialize((GameMtl*)new_mtl, false);
							}
						}
					}
					break;
				}
				case IDC_NEXTSIBLING:{
					NextSibling();
					break;
				}
				case IDC_PREVIOUSSIBLING:{
					PreviousSibling();
					break;
				}
			}
			break;
		}
		case CC_SPINNER_CHANGE:{
			ISpinnerControl *spinner = (ISpinnerControl*)lParam;
			switch(LOWORD(wParam)){
				case IDC_SORT_LEVEL_SPIN:{
					// Check the 'enabled' checkbox if sort level != SORT_LEVEL_NONE, uncheck it otherwise.
					::SendDlgItemMessage(hDlg, IDC_ENABLE_SORT_LEVEL, BM_SETCHECK,
						spinner->GetIVal() == SORT_LEVEL_NONE ? BST_UNCHECKED : BST_CHECKED, 0);
					TheMtl->Set_Sort_Level(spinner->GetIVal());
					break;
				}
			}
			break;
		}
	}
	return false;
}
//============================================================================================
BOOL	GMaxMtlDlg::PassCountProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam){
	switch (message){		
		case WM_INITDIALOG:
			break;
		case WM_COMMAND:{
			switch(LOWORD(wParam)){
				case IDC_SETPASSCOUNT:{
					if(HIWORD(wParam) == BN_CLICKED){
						Set_Pass_Count_Dialog();
					}
					break;
				}
			}
			break;
		}
	}
	return false;
}

//============================================================================================
static BOOL CALLBACK DisplacementMapDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam,LPARAM lParam){
	GMaxMtlDlg * theDlg;
	if (msg == WM_INITDIALOG) {
		lParam = ((PROPSHEETPAGE*)lParam)->lParam;
		theDlg = (GMaxMtlDlg*)lParam;
		theDlg->HwndDisplacementMap = hwndDlg;
		SetWindowLong(hwndDlg, GWL_USERDATA,(LPARAM)theDlg);
	} else {
		if ((theDlg = (GMaxMtlDlg *)GetWindowLong(hwndDlg, GWL_USERDATA) ) == NULL) {
			return false; 
		}
	}
	theDlg->IsActive = 1;
	BOOL res = theDlg->DisplacementMapProc(hwndDlg,msg,wParam,lParam);
	theDlg->IsActive = 0;
	return res;
}

//============================================================================================
static BOOL CALLBACK SurfaceTypePanelDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam,LPARAM lParam){
	GMaxMtlDlg * theDlg;
	if (msg == WM_INITDIALOG) {
		lParam = ((PROPSHEETPAGE*)lParam)->lParam;
		theDlg = (GMaxMtlDlg*)lParam;
		theDlg->HwndSurfaceType = hwndDlg;
		SetWindowLong(hwndDlg, GWL_USERDATA,(LPARAM)theDlg);
		// Set HwndPassCount to HwndSurfaceType
		theDlg->HwndPassCount = theDlg->HwndSurfaceType;
		PropSheet_Changed(GetParent(hwndDlg), hwndDlg); //Enable the "Apply" at all time
	} else {
		if ((theDlg = (GMaxMtlDlg *)GetWindowLong(hwndDlg, GWL_USERDATA) ) == NULL) {
			return false; 
		}
	}
	theDlg->IsActive = 1;
	BOOL res = theDlg->SurfaceTypeProc(hwndDlg,msg,wParam,lParam);
	theDlg->IsActive = 0;

	return res;
}

//============================================================================================
static BOOL CALLBACK PassCountPanelDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam,LPARAM lParam){
	GMaxMtlDlg * theDlg = (GMaxMtlDlg *)GetWindowLong(hwndDlg, GWL_USERDATA);
	switch(msg){
		case WM_INITDIALOG: {
			lParam = ((PROPSHEETPAGE*)lParam)->lParam;
			theDlg = (GMaxMtlDlg*)lParam;
			theDlg->HwndPassCount = hwndDlg;
			SetWindowLong(hwndDlg, GWL_USERDATA,(LPARAM)theDlg);
			return false;
		}
	}
	if ((theDlg = (GMaxMtlDlg *)GetWindowLong(hwndDlg, GWL_USERDATA) ) == NULL) {
		return false; 
	}
	theDlg->IsActive = 1;
	BOOL res = theDlg->PassCountProc(hwndDlg,msg,wParam,lParam);
	theDlg->IsActive = 0;
	return res;
}
//============================================================================================
void GMaxMtlDlg::Set_Pass_Count_Dialog(void){
	int res = DialogBoxParam(
		AppInstance,
		MAKEINTRESOURCE(IDD_GAMEMTL_PASS_COUNT_DIALOG),
		HwndPassCount,
		PassCountDialogDlgProc,
		(LPARAM)TheMtl->Get_Pass_Count());
	if (res>=0){
		if(res<=0){
			res = 1;
		}
		if(res>4){
			res = 4;
		}
		char a[12];
		sprintf(a, "%d", res);
		SetWindowText(GetDlgItem(HwndPassCount, IDC_GAMEMTL_PASSCOUNT_STATIC), a);
		if(TheMtl->Get_Pass_Count() != res){
			int npass(TheMtl->Get_Pass_Count());
			if(res > npass){ //Add pages
				for(int i = npass; i < res; i++){
					PassDialog[i] = new GameMtlPassDlg(HwndEdit, IParams, TheMtl, i); 
				}
			}else{//Remove pages
				for(int i = npass; i > res; i--){
					PropSheet_RemovePage(HwndEdit, i,NULL);
					PassDialog[i] = NULL;//new GameMtlPassDlg(HwndEdit, IParams, TheMtl, i-1); 
				}
			}
			TheMtl->Set_Pass_Count(res);
		}
	}
}

//============================================================================================
static BOOL CALLBACK PassCountDialogDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam,LPARAM lParam){
	switch (msg){
		case WM_INITDIALOG:{
			ISpinnerControl *spin =	SetupIntSpinner(hwndDlg,IDC_PASSCOUNT_SPIN, IDC_PASSCOUNT_EDIT,1,4,(int)lParam);
			ReleaseISpinner(spin);
			CenterWindow(hwndDlg,GetParent(hwndDlg));
			break;
		}
		case WM_COMMAND:
			switch (LOWORD(wParam)){
				case IDOK:{
					ISpinnerControl *spin = GetISpinner(GetDlgItem(hwndDlg,IDC_PASSCOUNT_SPIN));
					EndDialog(hwndDlg,spin->GetIVal());
					ReleaseISpinner(spin);
					break;
				}
				case IDCANCEL:{
					EndDialog(hwndDlg,-1);
					break;
				}
			break;
			}
		default:
			return false;
	}
	return true;
}


/*********************************************************************************************** 
 * GMaxMtlDlg::Build_Dialog -- Adds the dialog to the material editor                          * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   06/26/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
void GMaxMtlDlg::Build_Dialog(HWND hParent)// = NULL 
{
	//Need to initialize this global here
	if(GMaxMaterialDialog == NULL){
		GMaxMaterialDialog = this;
	}
	Make_PropertySheet(hParent);
	ReloadDialog();

}
//================================================
int CALLBACK GameMtl_PropSheetProc( HWND hDlg, UINT msg, LPARAM lParam){
	switch(msg){
		case PSCB_INITIALIZED:{
			ShowWindow(GetDlgItem(hDlg,IDCANCEL), SW_HIDE);
			ShowWindow(GetDlgItem(hDlg,IDOK), SW_HIDE);
			return true;
		} 			
		case PSCB_PRECREATE:{
        if (((LPDLGTEMPLATEEX)lParam)->signature ==  0xFFFF){
            ((LPDLGTEMPLATEEX)lParam)->style &= ~DS_CONTEXTHELP;
        }
        else {
            ((LPDLGTEMPLATE)lParam)->style &= ~DS_CONTEXTHELP;
        }
			return true;
		}
	}
	return false;
}
//================================================
void GMaxMtlDlg::Make_PropertySheet(HWND hParent){
    PROPSHEETPAGE ps_Pages[3];
	int pos(0);
	NONCLIENTMETRICS nc_metrics;
	nc_metrics.cbSize = sizeof(NONCLIENTMETRICS);
	//nc_metrics.lfMessageFont.lfHeight = -1;
	SystemParametersInfo(SPI_GETNONCLIENTMETRICS,sizeof(NONCLIENTMETRICS),&nc_metrics,0);
	_UsingLargeFonts = (ABS(nc_metrics.lfMessageFont.lfHeight) > 12);
	 //Surface type
    ps_Pages[pos].dwSize = sizeof(PROPSHEETPAGE);
    ps_Pages[pos].dwFlags = PSP_USEICONID | PSP_USETITLE ;
    ps_Pages[pos].hInstance = AppInstance;
	 if(_UsingLargeFonts){
		ps_Pages[pos].pszTemplate = MAKEINTRESOURCE(IDD_GMAXMTL_SURFACE_TYPE);
	 }else{
		ps_Pages[pos].pszTemplate = MAKEINTRESOURCE(IDD_GMAXMTL_SURFACE_TYPE_SMALL);
	 }
    ps_Pages[pos].pszIcon = MAKEINTRESOURCE(IDI_ICONW3D);
    ps_Pages[pos].pfnDlgProc = SurfaceTypePanelDlgProc;
    ps_Pages[pos].pszTitle = MAKEINTRESOURCE(IDS_GMAXSURFACE_TYPE);
    ps_Pages[pos].lParam = (LPARAM)this;
    ps_Pages[pos].pfnCallback = NULL;
	 pos++;
	#ifdef WANT_DISPLACEMENT_MAPS
	 //Displacement type
    ps_Pages[pos].dwSize = sizeof(PROPSHEETPAGE);
    ps_Pages[pos].dwFlags = PSP_USEICONID | PSP_USETITLE;
    ps_Pages[pos].hInstance = AppInstance;
    ps_Pages[pos].pszTemplate = MAKEINTRESOURCE(IDD_GMAXMTL_DISPLACEMENT_MAP);
    ps_Pages[pos].pszIcon = MAKEINTRESOURCE(IDI_ICONW3D);
    ps_Pages[pos].pfnDlgProc = DisplacementMapDlgProc;
    ps_Pages[pos].pszTitle = MAKEINTRESOURCE(IDS_DISPLACEMENT_MAP);
    ps_Pages[pos].lParam = (LPARAM)this;
    ps_Pages[pos].pfnCallback = NULL;
	 pos++;
	#endif //WANT_DISPLACEMENT_MAPS
	 //Create the PropSheet
	PROPSHEETHEADER ps_Header;
	memset(&ps_Header, 0, sizeof(ps_Header));
	ps_Header.dwSize = sizeof(PROPSHEETHEADER);
	ps_Header.dwFlags = PSH_PROPSHEETPAGE | PSH_MODELESS | PSH_USECALLBACK | PSH_USEICONID | PSH_NOAPPLYNOW;// ;
	ps_Header.hwndParent = hParent;//GetCOREInterface()->GetMAXHWnd();
	ps_Header.hInstance = AppInstance;
	ps_Header.pszIcon = MAKEINTRESOURCE(IDI_ICONW3D);
	ps_Header.pszCaption = MAKEINTRESOURCE(IDS_W3DMATEDITOR);
	ps_Header.nPages = pos ;
	ps_Header.nStartPage = 0;
	ps_Header.ppsp = (LPCPROPSHEETPAGE) &ps_Pages;
	ps_Header.pfnCallback = GameMtl_PropSheetProc;
	HwndEdit = (HWND)PropertySheet(&ps_Header);
	//Crop out the area for "OK" "Apply"...
	RECT wind_rect;
	GetWindowRect(HwndEdit, &wind_rect);
	MoveWindow(HwndEdit, wind_rect.left, wind_rect.top,wind_rect.right - wind_rect.left,
		wind_rect.bottom - wind_rect.top -40, true);
//	Position beneath the toolbar and slightly to the right
	SlideWindow(HwndEdit, 50,150);
	PropSheet_SetCurSel(HwndEdit,NULL, pos -1);//Force WM_INITDLG for the page
	HWND hwnd = PropSheet_IndexToHwnd(HwndEdit, 1);
	PropSheet_Changed(HwndEdit, hwnd); //Enable the "Apply" at all time
	for (int i=0; i<TheMtl->Get_Pass_Count(); i++) {
		PassDialog[i] = new GameMtlPassDlg(HwndEdit, IParams, TheMtl, i);
		PropSheet_SetCurSel(HwndEdit,NULL, i+pos);//Force WM_INITDLG for the page
	}
	PropSheet_SetCurSel(HwndEdit,NULL, 0);
}
//================================================
static BOOL CALLBACK GmaxMaterialDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam,LPARAM lParam){
	return false;
}
//================================================
void GMaxMtlDlg::Reinitialize(GameMtl* new_mtl, bool update_multimtl){// = false
	//Delete pages
	int npages(0);
	if(TheMtl){
		npages = TheMtl->Get_Pass_Count();
	}
	//save 
	int cur_page = CurrentPage;
	for (int i(npages); i > 0; i--) {
		PropSheet_RemovePage(HwndEdit, i,NULL);
		delete PassDialog[i];
		PassDialog[i] = NULL;
	}
	//Let know that ht material is no longer in the dialog
	if(TheMtl){
		TheMtl->SetParamDlg(NULL);
	}
	//Add new pages
	new_mtl->SetParamDlg(this);
	TheMtl = new_mtl;
	npages = TheMtl->Get_Pass_Count();
	for(i=0; i < npages; i++) {
		PassDialog[i] = new GameMtlPassDlg(HwndEdit, IParams, TheMtl, i);
		PropSheet_SetCurSel(HwndEdit,NULL, i+1);//Force WM_INITDLG for the page
	}
	//Make sure  cur_page is not out of range for a new material
	if(cur_page > npages){
		cur_page = npages;
	}
	//====HwndSurfaceType Spinner
	int sort_level = TheMtl->Get_Sort_Level();
	SetupIntSpinner(HwndSurfaceType, IDC_SORT_LEVEL_SPIN, IDC_SORT_LEVEL, 0, MAX_SORT_LEVEL, sort_level);
	// Check the checkbox if sort_level is not SORT_LEVEL_NONE.
	::SendDlgItemMessage(HwndSurfaceType, IDC_ENABLE_SORT_LEVEL, BM_SETCHECK,
		sort_level == SORT_LEVEL_NONE ? BST_UNCHECKED : BST_CHECKED, 0);
	//Surfface type combo
	::SendDlgItemMessage (HwndSurfaceType,IDC_SURFACE_TYPE_COMBO,CB_SETCURSEL,(WPARAM)TheMtl->Get_Surface_Type (),0L);
	//==== ShowHide multimaterial stuff
	if(update_multimtl){
		::SendMessage(HwndSurfaceType, WM_USER_UPDATE_MULTIMTL, 0, 0);
	}
	//Update name field
	TSTR name_str = TheMtl->GetName();
	char* dbg_txt = name_str.data();
	ICustEdit* pEdit = GetICustEdit(GetDlgItem(HwndSurfaceType, IDC_MTLNAME));
	pEdit->SetText(dbg_txt);
	ReleaseICustEdit(pEdit);
	//Multi
	if(Game_multi_mtl != NULL){
		name_str = Game_multi_mtl->GetName();
		pEdit = GetICustEdit(GetDlgItem(HwndSurfaceType, IDC_MULTIMTLNAME));
		pEdit->SetText(name_str.data());
		ReleaseICustEdit(pEdit);	//==== HwndPassCount
	}
	char a[12];
	sprintf(a, "%d", TheMtl->Get_Pass_Count());
	SetWindowText(GetDlgItem(HwndSurfaceType, IDC_GAMEMTL_PASSCOUNT_STATIC), a);	
	PropSheet_SetCurSel(HwndEdit,NULL, cur_page);//Force WM_INITDLG for the page
	ReloadDialog();

}
//============================================================================================
void GMaxMtlDlg::Reset(GameMtl* new_mtl){
	Reinitialize(new_mtl);
	ShowWindow(HwndEdit, SW_SHOW);
}
//============================================================================================
//Adds a tooltip to a control
void GMaxMtlDlg::AddToolTip(HWND hControl, UINT strID){

	char str[MAX_PATH];
	TOOLINFO    ti;
	ti.cbSize = sizeof(TOOLINFO);
	ti.uFlags = TTF_SUBCLASS ;
	ti.hwnd   = hControl;
	ti.uId    = 0;
	ti.hinst  = AppInstance;
	LoadString(AppInstance, strID,str , MAX_PATH);
	ti.lpszText  = str;
	GetClientRect(hControl, &ti.rect);
	HWND hwndTT = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS,  NULL,  WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,		
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,CW_USEDEFAULT, hControl,NULL ,AppInstance, NULL  );
	SetWindowPos(hwndTT, HWND_TOPMOST, 0, 0,   0,    0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
   int res = SendMessage(hwndTT, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &ti);	
}
//============================================================================================
void GMaxMtlDlg::LoadButtonBitmaps(HWND hDlg){
	//Get 
	if(NULL == HGetMtlBmp){
		HGetMtlBmp = LoadBitmap(AppInstance, MAKEINTRESOURCE(IDB_GETMTL));
	}
	if(NULL != HGetMtlBmp){
		SendDlgItemMessage(hDlg,IDC_GETMTL, BM_SETIMAGE,(WPARAM) IMAGE_BITMAP, (LPARAM)HGetMtlBmp);
		AddToolTip(GetDlgItem(hDlg,IDC_GETMTL), IDS_GETMTL);
	}
	//Browse 
	if(NULL == HBrowseMtlBmp){
		HBrowseMtlBmp = LoadBitmap(AppInstance, MAKEINTRESOURCE(IDB_BROWSE));
	}
	if(NULL != HBrowseMtlBmp){
		SendDlgItemMessage(hDlg,IDC_NAVIGATOR, BM_SETIMAGE,(WPARAM) IMAGE_BITMAP, (LPARAM)HBrowseMtlBmp);
		AddToolTip(GetDlgItem(hDlg,IDC_NAVIGATOR), IDS_NAVIGATOR);
	}
	//Assign 
	if(NULL == HAssignMtlBmp){
		HAssignMtlBmp = LoadBitmap(AppInstance, MAKEINTRESOURCE(IDB_ASSIGN));
	}
	if(NULL != HAssignMtlBmp){
		SendDlgItemMessage(hDlg,IDC_ASSIGN, BM_SETIMAGE,(WPARAM) IMAGE_BITMAP, (LPARAM)HAssignMtlBmp);
		AddToolTip(GetDlgItem(hDlg,IDC_ASSIGN), IDS_ASSIGNMTL);
	}
	//Delete 
	if(NULL == HDeleteMtlBmp){
		HDeleteMtlBmp = LoadBitmap(AppInstance, MAKEINTRESOURCE(IDB_DELETEMTL));
	}
	if(NULL != HDeleteMtlBmp){
		SendDlgItemMessage(hDlg,IDC_DELETEMTL, BM_SETIMAGE,(WPARAM) IMAGE_BITMAP, (LPARAM)HDeleteMtlBmp);
		AddToolTip(GetDlgItem(hDlg,IDC_DELETEMTL), IDS_DELETEMTL);
	}
	//New
	if(NULL == HNewMtlBmp){
		HNewMtlBmp = LoadBitmap(AppInstance, MAKEINTRESOURCE(IDB_NEWMTL));
	}
	if(NULL != HNewMtlBmp){
		SendDlgItemMessage(hDlg,IDC_NEWMTL, BM_SETIMAGE,(WPARAM) IMAGE_BITMAP, (LPARAM)HNewMtlBmp);
		AddToolTip(GetDlgItem(hDlg,IDC_NEWMTL), IDS_NEWMTL);
	}
	//Next Siblling
	if(NULL == HNextSiblingBmp){
		HNextSiblingBmp = LoadBitmap(AppInstance, MAKEINTRESOURCE(IDB_NEXTSIBLING));
	}
	if(NULL != HNextSiblingBmp){
		SendDlgItemMessage(hDlg,IDC_NEXTSIBLING, BM_SETIMAGE,(WPARAM) IMAGE_BITMAP, (LPARAM)HNextSiblingBmp);
		AddToolTip(GetDlgItem(hDlg,IDC_NEXTSIBLING), IDS_NEXTSIBLING);
	}
	//Previous Siblling
	if(NULL == HPreviousSiblingBmp){
		HPreviousSiblingBmp = LoadBitmap(AppInstance, MAKEINTRESOURCE(IDB_PREVIOUSSIBLING));
	}
	if(NULL != HPreviousSiblingBmp){
		SendDlgItemMessage(hDlg,IDC_PREVIOUSSIBLING, BM_SETIMAGE,(WPARAM) IMAGE_BITMAP, (LPARAM)HPreviousSiblingBmp);
		AddToolTip(GetDlgItem(hDlg,IDC_PREVIOUSSIBLING), IDS_PREVIOUSSIBLING);
	}
/*
	//havoc Siblling
	if(NULL == HHavocBmp){
		HHavocBmp = LoadBitmap(AppInstance, MAKEINTRESOURCE(IDB_HAVOC));
	}
	if(NULL != HHavocBmp){
		SendDlgItemMessage(hDlg,IDC_HAVOC, BM_SETIMAGE,(WPARAM) IMAGE_BITMAP, (LPARAM)HHavocBmp);
	}
	//Banner Siblling
	if(NULL == HBannerBmp){
		HBannerBmp = LoadBitmap(AppInstance, MAKEINTRESOURCE(IDB_BANNER));
	}
	if(NULL != HHavocBmp){
		SendDlgItemMessage(hDlg,IDC_BANNER, BM_SETIMAGE,(WPARAM) IMAGE_BITMAP, (LPARAM)HBannerBmp);
	}
*/
}
//============================================================================================
//Bring up the navigator
void GMaxMtlDlg::DoMaterialNavigator(){
	Ip->ExecuteMAXCommand( MAXCOM_TOOLS_MTLMAPBROWSER );
}
//============================================================================================
void GMaxMtlDlg::DoNewMaterial(){
	GameMtl *new_mtl = CreateNewGameMtl();
	Game_multi_mtl = NULL;
	Reinitialize(new_mtl);
}
//============================================================================================
void GMaxMtlDlg::DoGetMaterial(){
	GameMtl* new_mtl = GetMtlFromSelection();
	if(NULL != new_mtl){// && new_mtl != TheMtl){
		Reinitialize(new_mtl);
	}
}
//============================================================================================
void GMaxMtlDlg::ApplyToSelection(GameMtl* mtl){
	theHold.Begin ();
	int selcount = Ip->GetSelNodeCount();
	for(int i(0); i < selcount; i++){
		INode* node = Ip->GetSelNode(i);
		//if(node->NumMtls() < 2){ //?
		if(NULL == Game_multi_mtl){
			node->SetMtl(mtl);
		}else{
			node->SetMtl(Game_multi_mtl);
		}
	}
	Ip->ForceCompleteRedraw(true);
	theHold.Accept(IDS_APPLYGAMEMTL);
}
//============================================================================================
void GMaxMtlDlg::DeleteNodeMaterial(INode* node, MtlBase* mtl){
	Mtl* node_mtl;
	if((node_mtl = node->GetMtl()) == mtl){
		node->SetMtl(NULL);
	}
	//Multi/sub
	if(node_mtl){
		int num_subs(node_mtl->NumSubMtls());
		if( num_subs > 0){
			for(int i(0); i < num_subs; i++){
				if(mtl == node_mtl->GetSubMtl(i)){
						node_mtl->SetSubMtl(i,NULL);
				}
			}
		}
	}
	int node_count = node->NumberOfChildren();
	for(int i(0); i < node_count; i++){
		INode* node_i = node->GetChildNode(i);
		DeleteNodeMaterial(node_i, mtl);
	}
}
//============================================================================================
void GMaxMtlDlg::DeleteMtl(){
	theHold.Begin ();
	MtlBaseLib* scene_mtls = Ip->GetSceneMtls();
	INode* root_node = Ip->GetRootNode();
	if(NULL != scene_mtls && NULL != root_node){
		DeleteNodeMaterial(root_node, TheMtl);
		//Remove from scene
		scene_mtls->Remove(TheMtl);
		//Reset
		DestroyDialog();
		GMaxMaterialDialog = NULL;
	}
	Ip->ForceCompleteRedraw(true);
	theHold.Accept(IDS_APPLYGAMEMTL);
}
//============================================================================================
void GMaxMtlDlg::DeleteAllSceneMtl(){

	MtlBaseLib* scene_mtls = Ip->GetSceneMtls();
	if(scene_mtls){
		if(!DontShowDeleteAll){
			DWORD ret;
			char msg[MAX_PATH];
			char title[MAX_PATH];
			LoadString(AppInstance, IDS_DELETE_ALLMTLS, msg, MAX_PATH);
			LoadString(AppInstance, IDS_W3DMSGCAPTION, title, MAX_PATH);
			if(IDYES == MaxMsgBox(Ip->GetMAXHWnd(), msg, title,MB_YESNO | MB_ICONINFORMATION, MAX_MB_DONTSHOWAGAIN, &ret)){
				scene_mtls->Empty();
			}
			if(ret & MAX_MB_DONTSHOWAGAIN){
				DontShowDeleteAll = true;
			}
		}else{
			scene_mtls->Empty();
			//Force Refresh navigator
			Ip->ExecuteMAXCommand( MAXCOM_TOOLS_MTLMAPBROWSER );
		}
	}
}
//============================================================================================
GameMtl* CreateNewGameMtl(){
	Interface * Ip = GetCOREInterface();
	MtlBaseLib* scene_mtls = Ip->GetSceneMtls();
	GameMtl* mtl = new GameMtl();
	char namebuf[MAX_PATH] = {0};
	LoadString(AppInstance,IDS_DEFAULTGAMEMTL_NAME, namebuf, MAX_PATH);
	TSTR str = namebuf;
	int index = scene_mtls->FindMtl(mtl);
	if(index == -1){
		char tempbuf[32];
		itoa(_Num_Of_NoNames++, tempbuf, 10);
		str += tempbuf;
		mtl->SetName(str);
	}
	scene_mtls->AddMtl(mtl);
	return mtl;
}
//============================================================================================
void GMaxMtlDlg::ShowHideControls(){
	if(NULL == Game_multi_mtl){
		ShowWindow(GetDlgItem(HwndSurfaceType,IDC_SUBMTL_SPIN), SW_HIDE);
		ShowWindow(GetDlgItem(HwndSurfaceType,IDC_SUBMTL_STATIC), SW_HIDE);
		ShowWindow(GetDlgItem(HwndSurfaceType,IDC_SUBMTL_NUM), SW_HIDE);
		ShowWindow(GetDlgItem(HwndSurfaceType,IDC_MULTIMTLNAME),SW_HIDE);
		ShowWindow(GetDlgItem(HwndSurfaceType,IDC_MULTI_STATIC),SW_HIDE);
		ShowWindow(GetDlgItem(HwndSurfaceType,IDC_NAME_STATIC),SW_SHOW);
		ShowWindow(GetDlgItem(HwndSurfaceType,IDC_NEXTSIBLING),SW_HIDE);
		ShowWindow(GetDlgItem(HwndSurfaceType,IDC_PREVIOUSSIBLING),SW_HIDE);
	}else{
		ShowWindow(GetDlgItem(HwndSurfaceType,IDC_SUBMTL_SPIN), SW_SHOW);
		ShowWindow(GetDlgItem(HwndSurfaceType,IDC_SUBMTL_STATIC), SW_SHOW);
		ShowWindow(GetDlgItem(HwndSurfaceType,IDC_SUBMTL_NUM), SW_SHOW);
		int n_subs = Game_multi_mtl->NumSubMtls();
		SendDlgItemMessage(HwndSurfaceType,IDC_SUBMTL_SPIN, UDM_SETRANGE,(WPARAM)0, MAKELONG(n_subs, 1));
		ICustEdit* pEdit = GetICustEdit(GetDlgItem(HwndSurfaceType, IDC_SUBMTL_NUM));
		pEdit->SetText(1);
		ReleaseICustEdit(pEdit);
		//SetDlgItemInt(HwndSurfaceType,IDC_SUBMTL_NUM, 1, false);
		ShowWindow(GetDlgItem(HwndSurfaceType,IDC_MULTIMTLNAME),SW_SHOW);
		ShowWindow(GetDlgItem(HwndSurfaceType,IDC_MULTI_STATIC),SW_SHOW);
		ShowWindow(GetDlgItem(HwndSurfaceType,IDC_NAME_STATIC),SW_HIDE);
		ShowWindow(GetDlgItem(HwndSurfaceType,IDC_NEXTSIBLING),SW_SHOW);
		ShowWindow(GetDlgItem(HwndSurfaceType,IDC_PREVIOUSSIBLING),SW_SHOW);
	}
}
//============================================================================================
void GMaxMtlDlg::NextSibling(){

	// (gth) MATERIAL BEING CHANGED, user clicked the "next sibling" button
	if(HwndPassCount != NULL && Game_multi_mtl){ //Only after initializations
		int num_subs = Game_multi_mtl->NumSubMtls();
		Current_Submtl_Index = ++Current_Submtl_Index % num_subs;
		Mtl * new_mtl = Game_multi_mtl->GetSubMtl(Current_Submtl_Index);

		if(NULL != new_mtl){
			if(new_mtl->ClassID() == GameMaterialClassID){// && new_mtl != TheMtl){
				Reinitialize((GameMtl*)new_mtl, false);
				ICustEdit* pEdit = GetICustEdit(GetDlgItem(HwndSurfaceType, IDC_SUBMTL_NUM));
				pEdit->SetText(Current_Submtl_Index+1);
				ReleaseICustEdit(pEdit);
			}else{
				if(!DontShowMtlType){
					DWORD ret;
					char msg[MAX_PATH];
					char title[MAX_PATH];
					LoadString(AppInstance, IDS_NOTAGAMEMTL, msg, MAX_PATH);
					LoadString(AppInstance, IDS_W3DMSGCAPTION, title, MAX_PATH);
					MaxMsgBox(HwndSurfaceType, msg, title,MB_OK | MB_ICONINFORMATION, MAX_MB_DONTSHOWAGAIN, &ret);
					if(ret & MAX_MB_DONTSHOWAGAIN){
						DontShowMtlType = true;
					}
				}
			}
		}
	}
}
//============================================================================================
void GMaxMtlDlg::PreviousSibling(){

	// (gth) MATERIAL BEING CHANGED, user clicked the "prev sibling" button

	if(HwndPassCount != NULL && Game_multi_mtl){ //Only after initializations
		int num_subs = Game_multi_mtl->NumSubMtls();
		if(Current_Submtl_Index > 0){
			Current_Submtl_Index --;
		}else{
			Current_Submtl_Index = Game_multi_mtl->NumSubMtls()-1;
		}
		
		Mtl* new_mtl = (GameMtl*)(Game_multi_mtl->GetSubMtl(Current_Submtl_Index));
		
		if(NULL != new_mtl){
			if(new_mtl->ClassID() == GameMaterialClassID){// && new_mtl != TheMtl){
				ICustEdit* pEdit = GetICustEdit(GetDlgItem(HwndSurfaceType, IDC_SUBMTL_NUM));
				pEdit->SetText(Current_Submtl_Index + 1);
				ReleaseICustEdit(pEdit);
				Reinitialize((GameMtl*)new_mtl, false);
			}else{
				if(!DontShowMtlType){
					DWORD ret;
					char msg[MAX_PATH];
					char title[MAX_PATH];
					LoadString(AppInstance, IDS_NOTAGAMEMTL, msg, MAX_PATH);
					LoadString(AppInstance, IDS_W3DMSGCAPTION, title, MAX_PATH);
					MaxMsgBox(HwndSurfaceType, msg, title,MB_OK | MB_ICONINFORMATION, MAX_MB_DONTSHOWAGAIN, &ret);
					if(ret & MAX_MB_DONTSHOWAGAIN){
						DontShowMtlType = true;
					}
				}
			}
		}
	}
}
//============================================================================================
BOOL CALLBACK GameMtlMainProc(HWND hMainDlg, UINT msg, WPARAM wParam, LPARAM lParam){
	switch(msg){
		case WM_INITDIALOG:{
				break;
			}
	}
	return false;
}
//============================================================================================
GameMtl* GetMtlFromSelection(){

	// (gth) MATERIAL BEING CHANGED, user clicked the "get from selection" button

//	bool found_gamemtl(false);
	Mtl* mtl = NULL;
	GameMtl* game_mtl = NULL;
	IMtlParams* mtl_params = NULL;
	Interface* ip = GetCOREInterface();
	int nsel;
	if((nsel = ip->GetSelNodeCount()) < 1){
		return NULL;
	}else{
		INode* node = ip->GetSelNode(0);
		mtl = node->GetMtl();
		if(NULL == mtl){// Node has no mtl
			return NULL;
		}else{
			GMaxMaterialDialog->SetMultimaterial(NULL);
			int n_subs = mtl->NumSubMtls() ;
			if(n_subs > 0){
				//case MultiMtl
				//found_gamemtl = true;
				//Set for multimaterial
				GMaxMaterialDialog->SetMultimaterial(mtl);
				for(int sub_i(0); sub_i < n_subs/* && ! found_gamemtl*/; sub_i ++){
					Mtl* sub_mtl = mtl->GetSubMtl(sub_i);
					if(sub_mtl->ClassID() == GameMaterialClassID){
						game_mtl = (GameMtl*)sub_mtl;
					}else{
						game_mtl = GMaxMaterialDialog->ConvertStdMtl(sub_mtl);
						//replace the submtl by the converted
						mtl->SetSubMtl(sub_i,game_mtl );

					}
				}
			}else{
				// No submtls
				if(mtl->ClassID() == GameMaterialClassID){
					game_mtl = (GameMtl*) mtl;
				}else{
					game_mtl = GMaxMaterialDialog->ConvertStdMtl(mtl);
				}
			}
		}
	}
	return game_mtl;
}
//============================================================================================
//============================================================================================
//============================================================================================
void GMaxMtlDlg::Launch()
{
}

void GMaxMtlDlg::SetMultiMaterialTabBySlot( int iSlot, int iSubMtlTexIndex, int iElement ) {
	SetMultiMaterialBySlot(iSlot,iSubMtlTexIndex);
}

//============================================================================================
void GMaxMtlDlg::SetMaterialTabBySlot( int iSlot, int iElement ) {
	SetMaterialBySlot(iSlot);
}

//============================================================================================
void GMaxMtlDlg::SetMultiMaterialBySlot( int iSlot, int iIndex ) {

	// (gth) MATERIAL BEING CHANGED, user double clicked a member of a multi-material in the navagator
	MtlBase * mtl_base = GetMtlEditInterface()->GetCurMtl();
	if ((mtl_base->IsMultiMtl()) && (IsMtl(mtl_base))) {
		
		Mtl * mtl = (Mtl*)mtl_base;

		// Sub material iIndex is "1-based" but the sdk is "0-based"
		if ((iIndex > 0) && (iIndex <= mtl->NumSubMtls())) {

			MtlBase * sub_mtl = ((Mtl*)mtl)->GetSubMtl(iIndex - 1); 
			if ((sub_mtl != NULL) && (sub_mtl->ClassID() == GameMaterialClassID)) {

				Game_multi_mtl = (Mtl*)mtl;
				Reinitialize((GameMtl*)(sub_mtl));

				ICustEdit* pEdit = GetICustEdit(GetDlgItem(HwndSurfaceType, IDC_SUBMTL_NUM));
				Current_Submtl_Index = iIndex-1 ;
				pEdit->SetText(iIndex);
				ReleaseICustEdit(pEdit);

			}
		}
	}
	ShowWindow(HwndEdit, SW_SHOW);
}

//============================================================================================
void GMaxMtlDlg::SetMaterialBySlot( int iSlot ){

	// (gth) MATERIAL BEING CHANGED, user double clicked a material in the navagator
	MtlBase * mtl_base = GetMtlEditInterface()->GetCurMtl();

	Class_ID clsid = mtl_base->ClassID();
	if(clsid == GameMaterialClassID){
		Game_multi_mtl = NULL;
		Reinitialize((GameMtl*)mtl_base);
	} else if ((mtl_base->IsMultiMtl()) && (IsMtl(mtl_base))) {
		SetMultiMaterialBySlot( iSlot, 1 );
	}
	ShowWindow(HwndEdit, SW_SHOW);
}


//============================================================================================
void GMaxMtlDlg::DestroyDialog()
{
	ShowWindow(HwndEdit, SW_HIDE);
	DeleteThis();
}
//============================================================================================
GameMtl* GMaxMtlDlg::ConvertStdMtl(Mtl* stdmtl){
	GameMtl* gmtl = CreateNewGameMtl();
	gmtl->Set_Ambient(0,Ip->GetTime(), stdmtl->GetAmbient());
	gmtl->Set_Diffuse(0,Ip->GetTime(), stdmtl->GetDiffuse());
	gmtl->Set_Specular(0,Ip->GetTime(), stdmtl->GetSpecular());
	gmtl->Set_Shininess(0,Ip->GetTime(), stdmtl->GetShininess());
	float opacity = 1-stdmtl->GetXParency();
	gmtl->Set_Opacity(0,Ip->GetTime(),opacity);
	Texmap* tmap = (Texmap*)stdmtl->GetActiveTexmap();
	if(tmap != NULL){
		gmtl->Set_Texture(0,0,tmap);
		gmtl->Set_Texture_Enable(0,0,true);
		gmtl->Set_Texture_Display(0,0,true);
		gmtl->Set_Texture_Frame_Rate(0,0,0);
		gmtl->Set_Texture_Frame_Count(0,0,0);
		gmtl->Set_Texture_Display(0,0,true);
		gmtl->Set_Map_Channel(0,0,1);
	}
	return gmtl;
}
#endif

