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

// DriverVersionWarning.cpp : implementation file
//

#include "StdAfx.h"
#include "WWConfig.h"
#include "videoconfigdialog.h"
#include "DriverVersionWarning.h"
#include "dx8caps.h"
#include "cpudetect.h"
#include "dx8wrapper.h"
#include "registry.h"
#include "formconv.h"
#include "locale_api.h"
#include "wwconfig_ids.h"
#include "openw3d.h"
#include "ini.h"


extern int GlobalExitValue;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static StringClass VersionWarningString;


/////////////////////////////////////////////////////////////////////////////
// DriverVersionWarning dialog


DriverVersionWarning::DriverVersionWarning(CWnd* pParent /*=NULL*/)
	: CDialog(DriverVersionWarning::IDD, pParent)
{
	//{{AFX_DATA_INIT(DriverVersionWarning)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void DriverVersionWarning::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DriverVersionWarning)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(DriverVersionWarning, CDialog)
	//{{AFX_MSG_MAP(DriverVersionWarning)
	ON_BN_CLICKED(IDC_DISABLE_DRIVER_VERSION_DIALOG_CHECKBOX, OnDisableDriverVersionDialogCheckbox)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
//
//	DriverVersionWarning message handlers
//
//	Modified: 12/06/2001 by	MML	- Retrieving strings from Locomoto file.
//
/////////////////////////////////////////////////////////////////////////////

static void Get_Driver_Version_Warning_String(StringClass& str, DX8Caps& caps)
{
	DX8Caps::DriverVersionStatusType status=caps.Get_Driver_Version_Status();

	if (status!=DX8Caps::DRIVER_STATUS_BAD) {
//		str="Driver version is GOOD";
		str = StringClass( Locale_GetString( IDS_GOOD_DRIVER )); 
		return;
	}

	StringClass driver_name(0,true);
	driver_name=caps.Get_Driver_Name();

//	str.Format(
//		"Your current video card driver version is known to cause problems with Renegade. If you encounter problems while playing Renegade, please refer to the readme.txt for instructions on how to update your video card driver.\n"
//		"\n"
//		"Driver: %s\n"
//		"Driver version: %d.%d (Status: BAD)\n",
//		driver_name,
//		caps.Get_Driver_Build_Version()/100,
//		caps.Get_Driver_Build_Version()%100);

	StringClass format_string( Locale_GetString( IDS_KNOW_PROBLEMS ));
	format_string += "\n\n";
	format_string += StringClass( Locale_GetString( IDS_DRIVER_NAME ));
	format_string += "\n";
	format_string += StringClass( Locale_GetString( IDS_DRIVER_VERSION ));
	format_string += "\n";

	str.Format( format_string,
		driver_name,
		caps.Get_Driver_Build_Version()/100,
		caps.Get_Driver_Build_Version()%100 );

	switch (caps.Get_Vendor()) {

		default:
			break;

		case DX8Caps::VENDOR_NVIDIA:
			if (driver_name=="nv4_disp.dll" || driver_name=="nvdd32.dll") {
//				str += "Recommended version: 21.81 or higher\n";
				str += StringClass( Locale_GetString( IDS_VERSION_MIN1 ));
			}
			break;

		case DX8Caps::VENDOR_ATI:
			if (driver_name=="ati2dvag.dll") {
//				str += "Recommended version: 3281 or higher\n";
				str += StringClass( Locale_GetString( IDS_VERSION_MIN2 ));
			}
			break;

		case DX8Caps::VENDOR_3DFX:
//			str += "There is no driver support for 3Dfx. Website www.x3dfx.com has some unofficial drivers.\n";
			str += StringClass( Locale_GetString( IDS_NO_DRIVER_SUPPORT ));
			break;
	}
}

/////////////////////////////////////////////////////////////////////////////
//	CheckDriverVersion

void CheckDriverVersion()
{
	//
	//	Attempt to open the registry key 
	//
	INIClass ini(W3D_CONF_FILE);
	int disabled = ini.Get_Int(W3D_SECTION_RENDER, "DriverVersionCheckDisabled", 0);
	if (disabled>=87) return;

	IDirect3D9* d3d=NULL;
	D3DCAPS9 tmp_caps;
	const D3DCAPS9* d3dcaps=NULL;
	D3DADAPTER_IDENTIFIER9 adapter_id;

	VideoConfigDialogClass* video=VideoConfigDialogClass::Get_Instance();
	if (video) {
		d3d=DX8Wrapper::_Get_D3D8();
		d3d->AddRef();
		d3dcaps=&video->Get_Current_Caps();
		adapter_id=video->Get_Current_Adapter_Identifier();
	}
	else {
		// Init D3D
		Init_D3D_To_WW3_Conversion();
		d3d=Direct3DCreate9(D3D_SDK_VERSION);		// TODO: handle failure cases...
		if (!d3d) {
			return;
		}

		// Select device. If there is already a device selected in the registry, use it.

		int current_adapter_index=D3DADAPTER_DEFAULT;

		//
		//	Load the render device settings from the config file
		//
		char device_name[256] = { 0 };
		ini.Get_String(W3D_SECTION_RENDER, VALUE_INI_RENDER_DEVICE_NAME, "", device_name, sizeof(device_name));
		int adapter_count = d3d->GetAdapterCount();
		for (int adapter_index=0; adapter_index<adapter_count; adapter_index++) {
			D3DADAPTER_IDENTIFIER9 id;
			::ZeroMemory(&id, sizeof(D3DADAPTER_IDENTIFIER9));
			HRESULT res = d3d->GetAdapterIdentifier(adapter_index,0/*D3DENUM_WHQL_LEVEL*/,&id);
			// If device ok, check if it matches the currently set adapter name
			if (res == D3D_OK) {
				StringClass name(id.Description,true);
				if (name==device_name) {
					current_adapter_index=adapter_index;
					break;
				}
			}
		}

		if (FAILED(d3d->GetDeviceCaps(
			current_adapter_index,
			D3DDEVTYPE_HAL,
			&tmp_caps))) {
			d3d->Release();
			return;
		}

		::ZeroMemory(&adapter_id, sizeof(D3DADAPTER_IDENTIFIER9));
		if (FAILED( d3d->GetAdapterIdentifier(
			current_adapter_index,
			0/*D3DENUM_WHQL_LEVEL*/,
			&adapter_id))) {
			d3d->Release();
			return;
		}

		d3dcaps=&tmp_caps;

	}

	DX8Caps caps(d3d,*d3dcaps,WW3D_FORMAT_UNKNOWN,adapter_id);
	d3d->Release();
	d3d=NULL;

	// Beta message - remember to remove!
/*	bool unknown_device=false;
	switch (caps.Get_Vendor()) {
	default:
	case DX8Caps::VENDOR_UNKNOWN:
		unknown_device=true;
		break;
	case DX8Caps::VENDOR_NVIDIA:
		unknown_device=(caps.Get_Device()==DX8Caps::DEVICE_NVIDIA_UNKNOWN);
		break;
	case DX8Caps::VENDOR_ATI:
		unknown_device=(caps.Get_Device()==DX8Caps::DEVICE_ATI_UNKNOWN);
		break;
	case DX8Caps::VENDOR_INTEL:
		unknown_device=(caps.Get_Device()==DX8Caps::DEVICE_INTEL_UNKNOWN);
		break;
	case DX8Caps::VENDOR_S3:
		unknown_device=(caps.Get_Device()==DX8Caps::DEVICE_S3_UNKNOWN);
		break;
	case DX8Caps::VENDOR_POWERVR:
		unknown_device=(caps.Get_Device()==DX8Caps::DEVICE_POWERVR_UNKNOWN);
		break;
	case DX8Caps::VENDOR_MATROX:
		unknown_device=(caps.Get_Device()==DX8Caps::DEVICE_MATROX_UNKNOWN);
		break;
	case DX8Caps::VENDOR_3DFX:
		unknown_device=(caps.Get_Device()==DX8Caps::DEVICE_3DFX_UNKNOWN);
		break;
	case DX8Caps::VENDOR_3DLABS:
		unknown_device=(caps.Get_Device()==DX8Caps::DEVICE_3DLABS_UNKNOWN);
		break;
	}

	if (unknown_device) {

		VersionWarningString=
			"Renegade doesn't recognize your videocard.\n"
			"While you will most likely be able to play\n"
			"the game without problems, we would still\n"
			"like to add your video card in our datebase.\n"
			"Please send email to jani@westwood.com and\n"
			"tell what your system configuration is. Please\n"
			"include a file called sysinfo.txt from Renegade\n"
			"folder (c:\\westwood\\renegade by default)\n"
			"\n"
			"Thanks!";
		DriverVersionWarning dlg;
		dlg.DoModal();
	}
*/
	ini.Put_Int(W3D_SECTION_RENDER, "DriverVersionCheckDisabled", 87);
	ini.Save(W3D_CONF_FILE);
	GlobalExitValue=0;

// IML: Disable driver warning message.
#if 0
	DX8Caps::DriverVersionStatusType status=caps.Get_Driver_Version_Status();
	switch (status) {
	default:
	case DX8Caps::DRIVER_STATUS_GOOD:
	case DX8Caps::DRIVER_STATUS_OK:
	case DX8Caps::DRIVER_STATUS_UNKNOWN:
		render_registry.Set_Int( "DriverVersionCheckDisabled", 87 );	// Disable checking if driver version is good
		break;
	case DX8Caps::DRIVER_STATUS_BAD:
		{
			Get_Driver_Version_Warning_String(VersionWarningString,caps);

			DriverVersionWarning dlg;
			dlg.DoModal();
		}
		break;
	}
#endif
}

void DriverVersionWarning::OnDisableDriverVersionDialogCheckbox() 
{
	// TODO: Add your control notification handler code here
	
	int is_disabled = SendDlgItemMessage (IDC_DISABLE_DRIVER_VERSION_DIALOG_CHECKBOX, BM_GETCHECK);
	INIClass ini(W3D_CONF_FILE);
	
	ini.Put_Int(W3D_SECTION_RENDER, "DriverVersionCheckDisabled", is_disabled ? 87 : 0);
	ini.Save(W3D_CONF_FILE);
		
}

INT_PTR DriverVersionWarning::DoModal()
{
	// TODO: Add your specialized code here and/or call the base class

	return CDialog::DoModal();
}


/////////////////////////////////////////////////////////////////////////////
//
//	OnInitDialog
//
//	Modified: 12/06/2001 by	MML	- Retrieving strings from Locomoto file.
//
/////////////////////////////////////////////////////////////////////////////

BOOL DriverVersionWarning::OnInitDialog() 
{
	CDialog::OnInitDialog();

	//
	// Set all the static strings for this dialog.
	//
	char string[ _MAX_PATH ];

	// Message
	SetDlgItemText(IDC_DRIVER_VERSION_WARNING_TEXT,VersionWarningString);

	// Title
	Locale_GetString( IDS_WARNING, string );
	SetWindowText((LPCTSTR) string );

	// Checkbox
	Locale_GetString( IDS_DO_NOT_SHOW_MSG_AGAIN, string );
	SetDlgItemText( IDC_DISABLE_DRIVER_VERSION_DIALOG_CHECKBOX, string );

	// Buttons
	Locale_GetString( IDS_OK, string );
	SetDlgItemText( IDOK, string );

	Locale_GetString( IDS_CANCEL, string );
	SetDlgItemText( IDCANCEL, string );

	// TODO: Add extra initialization here
	
	return true;  // return true unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return false
}

void DriverVersionWarning::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	GlobalExitValue=1;
	CDialog::OnCancel();
}

void DriverVersionWarning::OnOK() 
{
	// TODO: Add extra validation here
	
	GlobalExitValue=0;
	CDialog::OnOK();
}
