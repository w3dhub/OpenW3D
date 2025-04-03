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

// VideoConfigDialog.cpp : implementation file
//

#include "stdafx.h"
#include "wwconfig.h"
#include "videoconfigdialog.h"
#include "ww3d.h"
#include "assetmgr.h"
#include "locale_api.h"
#include "wwconfig_ids.h"
#include	"assetstatus.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static VideoConfigDialogClass* VideoConfigDialogInstance;

/////////////////////////////////////////////////////////////////////////////
//	Globals
/////////////////////////////////////////////////////////////////////////////
WW3DAssetManager *_TheAssetMgr = NULL;;


/////////////////////////////////////////////////////////////////////////////
//	Local prototypes
/////////////////////////////////////////////////////////////////////////////
int _cdecl ResolutionSortCallback (const void *elem1, const void *elem2);


/////////////////////////////////////////////////////////////////////////////
//	Constants
/////////////////////////////////////////////////////////////////////////////
enum
{
	COL_DEVICE_NAME = 0
};

/////////////////////////////////////////////////////////////////////////////
//
// VideoConfigDialogClass
//
/////////////////////////////////////////////////////////////////////////////
VideoConfigDialogClass::VideoConfigDialogClass (CWnd *pParent)
	:	CurrentBitDepth (16),
		CurrentWidth (800),
		CurrentHeight (600),
		CurrentDriverIndex (0),
		CurrentIsWindowed (false),
		CDialog(VideoConfigDialogClass::IDD, pParent)
{
	//{{AFX_DATA_INIT(VideoConfigDialogClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	WWASSERT(!VideoConfigDialogInstance);	// Only one can exist at a time!
	VideoConfigDialogInstance=this;
	CDialog::Create (VideoConfigDialogClass::IDD, pParent);
	return ;
}

VideoConfigDialogClass::~VideoConfigDialogClass ()
{
	VideoConfigDialogInstance=NULL;
}

VideoConfigDialogClass* VideoConfigDialogClass::Get_Instance ()
{
	return VideoConfigDialogInstance;
}

/////////////////////////////////////////////////////////////////////////////
//
// DoDataExchange
//
/////////////////////////////////////////////////////////////////////////////
void
VideoConfigDialogClass::DoDataExchange (CDataExchange *pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(VideoConfigDialogClass)
	DDX_Control(pDX, IDC_RESOLUTION_SLIDER, m_ResSliderCtrl);
	DDX_Control(pDX, IDC_DRIVER_LIST, m_DriverListCtrl);
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(VideoConfigDialogClass, CDialog)
	//{{AFX_MSG_MAP(VideoConfigDialogClass)
	ON_WM_DESTROY()
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_DRIVER_LIST, OnItemchangedDriverList)
	ON_CBN_SELCHANGE(IDC_BITDEPTH_COMBO, OnSelchangeBitdepthCombo)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_WINDOWED_CHECK, OnWindowedCheck)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
//
// OnInitDialog
//
//	Modified: 12/06/2001 by	MML	- Retrieving strings from Locomoto file.
/////////////////////////////////////////////////////////////////////////////
BOOL
VideoConfigDialogClass::OnInitDialog (void) 
{
	char string [_MAX_PATH];

	CDialog::OnInitDialog ();	

	//
	// Set all the static strings for this dialog.
	//
	Locale_GetString( IDS_DRIVER, string );
	SetDlgItemText( IDC_DRIVER, string );

	Locale_GetString( IDS_DISPLAY, string );
	SetDlgItemText( IDC_STATIC2, string );

	Locale_GetString( IDS_RESOLUTION, string );
	SetDlgItemText( IDC_RESOLUTION, string );

	Locale_GetString( IDS_COLOR_DEPTH, string );
	SetDlgItemText( IDC_BIT_DEPTH, string );

	Locale_GetString( IDS_WINDOWED_MODE, string );
	SetDlgItemText( IDC_WINDOWED_CHECK, string );

	//
	//	Initialize the engine
	//
	_TheAssetMgr = new WW3DAssetManager;
	WW3D::Init (m_hWnd);
	AssetStatusClass::Peek_Instance()->Enable_Reporting (false);

	//
	//	Load the render device settings from the registry
	//
	char device_name[256] = { 0 };
	int texture_depth = 0;
	int width = 0;
	int height = 0;
	int bit_depth = 0;
	int windowed = 0;
	if (WW3D::Registry_Load_Render_Device (RENEGADE_SUB_KEY_NAME_RENDER, device_name, 256,
					width, height, bit_depth, windowed, texture_depth))
	{
		if (width != -1) {
			CurrentWidth = width;
		}

		if (height != -1) {
			CurrentHeight = height;
		}

		if (bit_depth != -1) {
			CurrentBitDepth = bit_depth;
		}

		if (windowed != -1) {
			CurrentIsWindowed = bool(windowed == 1);
		}
	}
	
	//
	//	Set the extended styles for the list control
	//
	m_DriverListCtrl.SetExtendedStyle (m_DriverListCtrl.GetExtendedStyle () | LVS_EX_FULLROWSELECT);

	//
	//	Configure the columns
	//
	m_DriverListCtrl.InsertColumn (COL_DEVICE_NAME, "Device Name");	

	//
	//	Populate the render device list control
	//
	bool found_default_entry = false;
	int driver_count = WW3D::Get_Render_Device_Count ();	
	for (int index = 0; index < driver_count; index ++) {
		const RenderDeviceDescClass &device_desc = WW3D::Get_Render_Device_Desc (index);

		//
		//	Build a human readable name for this driver
		//
		CString driver_name;
		driver_name.Format ("%s - %s", device_desc.Get_Device_Name (), device_desc.Get_Hardware_Name ());

		//
		//	Add this driver to the list control
		//
		int item_index = m_DriverListCtrl.InsertItem (0xFF, driver_name);
		if (item_index >= 0) {
			m_DriverListCtrl.SetItemData (item_index, index);
		}

		//
		//	Check to see if this is the default entry
		//
		const char *render_device_name = WW3D::Get_Render_Device_Name (index);
		if (::lstrcmpi (device_name, render_device_name) == 0) {
			m_DriverListCtrl.SetItemState (item_index, LVIS_SELECTED, LVIS_SELECTED);
			found_default_entry = true;
		}
	}

	//
	//	Set the "windowed mode" checkbox
	//
	SendDlgItemMessage (IDC_WINDOWED_CHECK, BM_SETCHECK, (WPARAM)CurrentIsWindowed);

	//
	//	Select the first entry for deafult (if necessary)
	//
	if (found_default_entry == false) {
		m_DriverListCtrl.SetItemState (0, LVIS_SELECTED, LVIS_SELECTED);
	} 

	//
	//	Size the columns
	//
	CRect rect;
	m_DriverListCtrl.GetClientRect (&rect);
	int col_width = rect.Width () - ::GetSystemMetrics (SM_CXVSCROLL);
	m_DriverListCtrl.SetColumnWidth (0, col_width);

	//
	//	Build the resolution list for the currently selected device
	//
	Update_Display_Settings ();	

	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnDestroy
//
/////////////////////////////////////////////////////////////////////////////
void
VideoConfigDialogClass::OnDestroy (void)
{
	//
	//	Shutdown the engine
	//
	WW3D::Shutdown ();
	delete _TheAssetMgr;
	_TheAssetMgr = NULL;

	CDialog::OnDestroy ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Update_Display_Settings
//
/////////////////////////////////////////////////////////////////////////////
void
VideoConfigDialogClass::Update_Display_Settings (void)
{
	//
	//	Get the selected device
	//
	int selected_index = m_DriverListCtrl.GetNextItem (-1, LVNI_ALL | LVNI_SELECTED);
	if (selected_index < 0) {
		return ;
	}

	//
	//	Lookup the device description for the selected device
	//
	CurrentDriverIndex = (int)m_DriverListCtrl.GetItemData (selected_index);
	const RenderDeviceDescClass &driver_info = WW3D::Get_Render_Device_Desc (CurrentDriverIndex);
	ResolutionList = driver_info.Enumerate_Resolutions ();
	CurrentCaps=driver_info.Get_Caps();
	CurrentAdapterIdentifier=driver_info.Get_Adapter_Identifier();

	//
	//	Update the dialog controls
	//
	Update_Color_Combo ();
	Update_Resolution_Slider ();
	Select_Default_Resolution ();	
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Update_Resolution_Slider
//
/////////////////////////////////////////////////////////////////////////////
void
VideoConfigDialogClass::Update_Resolution_Slider (void)
{
	//
	//	Lookup the device description for the selected device
	//
	const RenderDeviceDescClass &driver_info = WW3D::Get_Render_Device_Desc (CurrentDriverIndex);
	ResolutionList = driver_info.Enumerate_Resolutions ();

	//
	//	Filter out any resolutions that don't fit the current
	// bit depth
	//
	int index;
	for (index = 0; index < ResolutionList.Count (); index ++) {
		const ResolutionDescClass &res_desc = ResolutionList[index];

		//
		//	Remove this resolution if it doesn't fit the current
		// bit depth
		//
		if (res_desc.BitDepth != CurrentBitDepth || res_desc.Width < 640 || res_desc.Height < 480) {
			ResolutionList.Delete (index);
			index --;
		}
	}

	//
	//	Sort the entries
	//
	if (ResolutionList.Count () > 0) {
		::qsort (&ResolutionList[0], ResolutionList.Count (), sizeof (ResolutionList[0]), ResolutionSortCallback);
	}

	//
	//	Configure the slider control
	//
	m_ResSliderCtrl.ClearTics ();
	m_ResSliderCtrl.SetRange (0, ResolutionList.Count () - 1);

	//
	//	Setup the tick marks
	//
	for (index = 0; index < ResolutionList.Count (); index ++) {
		m_ResSliderCtrl.SetTic (index);
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Update_Color_Combo
//
/////////////////////////////////////////////////////////////////////////////
void
VideoConfigDialogClass::Update_Color_Combo (void)
{
	//
	//	Start fresh
	//
	SendDlgItemMessage (IDC_BITDEPTH_COMBO, CB_RESETCONTENT);

	//
	//	Loop over all the resolutions and add the different
	// bit-depths to the combo box
	//
	bool selected = false;
	for (int index = 0; index < ResolutionList.Count (); index ++) {
		
		const ResolutionDescClass &res_desc = ResolutionList[index];

		char	  string [_MAX_PATH];
		CString color_string;

		//
		//	Make a display string out of the bit depth
		//
		Locale_GetString (IDS_BIT, string);
		color_string.Format ("%d %s", res_desc.BitDepth, string);

		//
		//	Check to see if this string is already in the combobox
		//
		int item_index = SendDlgItemMessage (IDC_BITDEPTH_COMBO, CB_FINDSTRINGEXACT, 0, (LPARAM)(LPCTSTR)color_string);
		if (item_index == CB_ERR) {

			//
			//	Add this entry to the combobox
			//
			int item_index = SendDlgItemMessage (IDC_BITDEPTH_COMBO, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)color_string);
			if (item_index != CB_ERR) {
				SendDlgItemMessage (IDC_BITDEPTH_COMBO, CB_SETITEMDATA, (WPARAM)item_index, (LPARAM)res_desc.BitDepth);

				//
				//	Select this entry if its the current bit depth
				//
				if (res_desc.BitDepth == CurrentBitDepth) {
					SendDlgItemMessage (IDC_BITDEPTH_COMBO, CB_SETCURSEL, (WPARAM)item_index);
					selected = true;
				}
			}
		}
	}

	//
	//	Select the first entry by default
	//
	if (!selected) {
		SendDlgItemMessage (IDC_BITDEPTH_COMBO, CB_SETCURSEL, 0);
		CurrentBitDepth = SendDlgItemMessage (IDC_BITDEPTH_COMBO, CB_GETITEMDATA, 0);
		if (CurrentBitDepth <= 0) {
			CurrentBitDepth = 16;
		}		
	}
		
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Select_Default_Resolution
//
/////////////////////////////////////////////////////////////////////////////
void
VideoConfigDialogClass::Select_Default_Resolution (void)
{
	int best_selection		= 0;
	float best_width_err		= 1000.0F;
	float best_height_err	= 1000.0F;
	
	//
	//	Loop over all the available resolutions and try to select
	// the one that most closely matches the desired resolution
	//
	for (int index = 0; index < ResolutionList.Count (); index ++) {
		const ResolutionDescClass &res_desc = ResolutionList[index];

		//
		//	Calculate how far off this resolution is from the one
		// we want to select
		//
		float width_err	= WWMath::Fabs(((float)CurrentWidth / (float)res_desc.Width) - 1.0F);
		float height_err	= WWMath::Fabs(((float)CurrentHeight / (float)res_desc.Height) - 1.0F);

		//
		//	Is this the closest match so far?
		//
		if (width_err <= best_width_err && height_err <= best_height_err) {
			best_width_err		= width_err;
			best_height_err	= height_err;
			best_selection		= index;
		}
	}

	//
	//	Select the closest match
	//
	m_ResSliderCtrl.SetPos (best_selection);
	Update_Resolution_Text ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Update_Resolution_Text
//
/////////////////////////////////////////////////////////////////////////////
void
VideoConfigDialogClass::Update_Resolution_Text (void)
{
	int res_index = m_ResSliderCtrl.GetPos ();
	if (res_index >= 0) {
		
		//
		//	Make a display string for the current resolution
		//
		const ResolutionDescClass &res_desc = ResolutionList[res_index];
		CString res_string;
		res_string.Format ("%d x %d", res_desc.Width, res_desc.Height);

		//
		//	Set the text of the dialog control
		//
		SetDlgItemText (IDC_RESOLUTION_STATIC, res_string);

		//
		//	Store the current width and height
		//
		CurrentWidth = res_desc.Width;
		CurrentHeight = res_desc.Height;
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnItemchangedDriverList
//
/////////////////////////////////////////////////////////////////////////////
void
VideoConfigDialogClass::OnItemchangedDriverList
(
	NMHDR *		pNMHDR,
	LRESULT *	pResult
)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	(*pResult) = 0;

	//
	//	If the selection state changed, then update the resolution
	// list for this device
	//
	if (pNMListView->uChanged & LVIF_STATE) {
		Update_Display_Settings ();
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// ResolutionSortCallback
//
/////////////////////////////////////////////////////////////////////////////
int _cdecl
ResolutionSortCallback (const void *elem1, const void *elem2)
{
	const ResolutionDescClass *res1 = ((const ResolutionDescClass *)elem1);
	const ResolutionDescClass *res2 = ((const ResolutionDescClass *)elem2);

	int retval = 0;

	//
	//	Sort based on bit-depth first
	//
	if (res1->BitDepth < res2->BitDepth) {
		retval = -1;
	} else if (res1->BitDepth > res2->BitDepth) {
		retval = 1;
	} else {

		//
		//	Sort based on width next
		//
		if (res1->Width < res2->Width) {
			retval = -1;
		} else if (res1->Width > res2->Width) {
			retval = 1;
		} else {

			//
			//	Sort based on height last
			//
			if (res1->Height < res2->Height) {
				retval = -1;
			} else if (res1->Height > res2->Height) {
				retval = 1;
			}			
		}
	}

	return retval;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnSelchangeBitdepthCombo
//
/////////////////////////////////////////////////////////////////////////////
void
VideoConfigDialogClass::OnSelchangeBitdepthCombo (void)
{
	int index = SendDlgItemMessage (IDC_BITDEPTH_COMBO, CB_GETCURSEL);
	if (index != CB_ERR) {

		//
		//	Update the current bit depth
		//
		CurrentBitDepth = SendDlgItemMessage (IDC_BITDEPTH_COMBO, CB_GETITEMDATA, (WPARAM)index);
		if (CurrentBitDepth <= 0) {
			CurrentBitDepth = 16;
		}		

		//
		//	Update the dialog controls
		//
		Update_Resolution_Slider ();
		Select_Default_Resolution ();
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnHScroll
//
/////////////////////////////////////////////////////////////////////////////
void
VideoConfigDialogClass::OnHScroll (UINT nSBCode, UINT nPos, CScrollBar *pScrollBar)
{
	CDialog::OnHScroll (nSBCode, nPos, pScrollBar);

	//
	//	Simply update the display text on the dialog to reflect the new resolution
	//
	Update_Resolution_Text ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Apply_Changes
//
/////////////////////////////////////////////////////////////////////////////
void
VideoConfigDialogClass::Apply_Changes (void)
{
	//
	//	Save the changes to the registry
	//
	WW3D::Registry_Save_Render_Device (RENEGADE_SUB_KEY_NAME_RENDER, CurrentDriverIndex,
		CurrentWidth, CurrentHeight, CurrentBitDepth, CurrentIsWindowed, WW3D::Get_Texture_Bitdepth());
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// WindowProc
//
/////////////////////////////////////////////////////////////////////////////
LRESULT
VideoConfigDialogClass::WindowProc
(
	UINT	message,
	WPARAM	wParam,
	LPARAM	lParam
)
{
	if (message == (WM_USER + 101)) {
		Apply_Changes ();
	}

	return CDialog::WindowProc(message, wParam, lParam);
}


/////////////////////////////////////////////////////////////////////////////
//
// OnWindowedCheck
//
/////////////////////////////////////////////////////////////////////////////
void
VideoConfigDialogClass::OnWindowedCheck (void)
{
	CurrentIsWindowed = (SendDlgItemMessage (IDC_WINDOWED_CHECK, BM_GETCHECK) == 1);
	return ;
}
