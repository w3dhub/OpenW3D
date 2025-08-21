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

// AudioConfigDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "WWConfig.h"
#include "audioconfigdialog.h"
#include "WWAudio.h"
#include "locale_api.h"
#include "wwconfig_ids.h"
#include "../../Combat/specialbuilds.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
//	Constants
/////////////////////////////////////////////////////////////////////////////
enum
{
	COL_DRIVER_NAME	= 0,
};

/*
#ifdef MULTIPLAYERDEMO
const char *RENEGADE_SUB_KEY_NAME_AUDIO = "Software\\Westwood\\RenegadeMPDemo\\Sound";
#else
const char *RENEGADE_SUB_KEY_NAME_AUDIO = "Software\\Westwood\\Renegade\\Sound";
#endif // MULTIPLAYERDEMO
*/

#if	defined(FREEDEDICATEDSERVER)
const char *RENEGADE_SUB_KEY_NAME_AUDIO = "Software\\Westwood\\RenegadeFDS\\Sound";
#elif defined(MULTIPLAYERDEMO)
const char *RENEGADE_SUB_KEY_NAME_AUDIO = "Software\\Westwood\\RenegadeMPDemo\\Sound";
#elif defined(BETACLIENT)
const char *RENEGADE_SUB_KEY_NAME_AUDIO = "Software\\Westwood\\RenegadeBeta\\Sound";
#else
const char *RENEGADE_SUB_KEY_NAME_AUDIO = "Software\\Westwood\\Renegade\\Sound";
#endif

/////////////////////////////////////////////////////////////////////////////
//
// AudioConfigDialogClass
//
/////////////////////////////////////////////////////////////////////////////
AudioConfigDialogClass::AudioConfigDialogClass(CWnd* pParent /*=NULL*/)
	: CDialog(AudioConfigDialogClass::IDD, pParent)
{
	//{{AFX_DATA_INIT(AudioConfigDialogClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	CDialog::Create (AudioConfigDialogClass::IDD, pParent);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// DoDataExchange
//
/////////////////////////////////////////////////////////////////////////////
void
AudioConfigDialogClass::DoDataExchange (CDataExchange *pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(AudioConfigDialogClass)
	DDX_Control(pDX, IDC_SOUND_EFFECTS_SLIDER, m_SoundVolSlider);
	DDX_Control(pDX, IDC_MUSIC_SLIDER, m_MusicVolSlider);
	DDX_Control(pDX, IDC_DIALOG_SLIDER, m_DialogVolSlider);
	DDX_Control(pDX, IDC_CINEMATIC_SLIDER, m_CinematicVolSlider);
	DDX_Control(pDX, IDC_DRIVER_LIST, m_ListCtrl);
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(AudioConfigDialogClass, CDialog)
	//{{AFX_MSG_MAP(AudioConfigDialogClass)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
//
// OnInitDialog
//
//	Modified: 12/06/2001 by	MML	- Retrieving strings from Locomoto file.
/////////////////////////////////////////////////////////////////////////////
BOOL
AudioConfigDialogClass::OnInitDialog (void) 
{
	char string [_MAX_PATH];
	
	CDialog::OnInitDialog ();

	//
	// Set all the static strings for this dialog.
	//
	Locale_GetString( IDS_DRIVER, string );
	SetDlgItemText( IDC_DRIVER, string );

	Locale_GetString( IDS_VOLUME, string );
	SetDlgItemText( IDC_VOLUME, string );

	Locale_GetString( IDS_SOUND_EFFECTS, string );
	SetDlgItemText( IDC_SOUND_EFFECTS_CHECK, string );

	Locale_GetString( IDS_MUSIC, string );
	SetDlgItemText( IDC_MUSIC_CHECK, string );

	Locale_GetString( IDS_DIALOG, string );
	SetDlgItemText( IDC_DIALOG_CHECK, string );

	Locale_GetString( IDS_CINEMATIC, string);
	SetDlgItemText( IDC_CINEMATIC_CHECK, string );

	Locale_GetString( IDS_QUALITY, string );
	SetDlgItemText( IDC_QUALITY, string ); 
	SendDlgItemMessage (IDC_QUALITY_COMBO, CB_RESETCONTENT, 0, 0);
	SendDlgItemMessage (IDC_QUALITY_COMBO, CB_ADDSTRING, 0, (LPARAM)Locale_GetString (IDS_8_BIT, string));
	SendDlgItemMessage (IDC_QUALITY_COMBO, CB_ADDSTRING, 0, (LPARAM)Locale_GetString (IDS_16_BIT, string));

	Locale_GetString( IDS_PLAYBACK_RATE, string );
	SetDlgItemText( IDC_PLAYBACK_RATE, string );
	SendDlgItemMessage (IDC_RATE_COMBO, CB_RESETCONTENT, 0, 0);
	SendDlgItemMessage (IDC_RATE_COMBO, CB_ADDSTRING, 0, (LPARAM)Locale_GetString (IDS_11_KHZ, string));
	SendDlgItemMessage (IDC_RATE_COMBO, CB_ADDSTRING, 0, (LPARAM)Locale_GetString (IDS_22_KHZ, string));
	SendDlgItemMessage (IDC_RATE_COMBO, CB_ADDSTRING, 0, (LPARAM)Locale_GetString (IDS_44_KHZ, string));

	Locale_GetString( IDS_SPEAKER_SETUP, string );
	SetDlgItemText( IDC_SPEAKER_SETUP, string );
	SendDlgItemMessage (IDC_SPEAKER_COMBO, CB_RESETCONTENT, 0, 0);
	SendDlgItemMessage (IDC_SPEAKER_COMBO, CB_ADDSTRING, 0, (LPARAM)Locale_GetString (IDS_2_SPEAKER, string));
	SendDlgItemMessage (IDC_SPEAKER_COMBO, CB_ADDSTRING, 0, (LPARAM)Locale_GetString (IDS_HEADPHONE, string));
	SendDlgItemMessage (IDC_SPEAKER_COMBO, CB_ADDSTRING, 0, (LPARAM)Locale_GetString (IDS_SURROUND_SOUND, string));
	SendDlgItemMessage (IDC_SPEAKER_COMBO, CB_ADDSTRING, 0, (LPARAM)Locale_GetString (IDS_4_SPEAKER, string));

	Locale_GetString( IDS_STEREO, string );
	SetDlgItemText( IDC_STEREO_CHECK, string );

	//
	//	Startup the audio library
	//
	new WWAudioClass;
	WWAudioClass::Get_Instance ()->Initialize ();
	
	//
	//	Read the audio library's settings from the registry
	//
	StringClass device_name;
	bool	is_stereo 	  = true;
	int	bits	  		  = 16;
	int	hertz	  		  = 44100;
	bool	sound_on  	  = true;
	bool	music_on  	  = true;
	bool	dialog_on 	  = true;
	bool  cinematic_on  = true;
	float	sound_vol	  = 1.0F;
	float	music_vol	  = 1.0F;
	float	dialog_vol	  = 1.0F;
	float	cinematic_vol = 1.0F;
	int	speaker_type  = 0;

	WWAudioClass::Get_Instance ()->Load_From_Registry (RENEGADE_SUB_KEY_NAME_AUDIO,
												device_name, is_stereo, bits, hertz, sound_on,
												music_on, dialog_on, cinematic_on, sound_vol,
												music_vol, dialog_vol, cinematic_vol, speaker_type);

	//
	//	Setup the sound volume controls
	//
	m_SoundVolSlider.SetRange (0, 100);
	m_SoundVolSlider.SetPos (static_cast<int>(sound_vol * 100));
	SendDlgItemMessage (IDC_SOUND_EFFECTS_CHECK, BM_SETCHECK, (WPARAM)sound_on);

	//
	//	Setup the music volume controls
	//
	m_MusicVolSlider.SetRange (0, 100);
	m_MusicVolSlider.SetPos (static_cast<int>(music_vol * 100));
	SendDlgItemMessage (IDC_MUSIC_CHECK, BM_SETCHECK, (WPARAM)music_on);

	//
	//	Setup the dialog volume controls
	//
	m_DialogVolSlider.SetRange (0, 100);
	m_DialogVolSlider.SetPos (static_cast<int>(dialog_vol * 100));
	SendDlgItemMessage (IDC_DIALOG_CHECK, BM_SETCHECK, (WPARAM)dialog_on);

	//
	//	Setup the cinematic volume controls
	//
	m_CinematicVolSlider.SetRange (0, 100);
	m_CinematicVolSlider.SetPos (static_cast<int>(cinematic_vol * 100));
	SendDlgItemMessage (IDC_CINEMATIC_CHECK, BM_SETCHECK, (WPARAM)cinematic_on);

	//
	//	Check the stereo box if necessary
	//
	SendDlgItemMessage (IDC_STEREO_CHECK, BM_SETCHECK, (WPARAM)is_stereo);

	//
	//	Select the appropriate quality combobox entry
	//
	switch (bits)
	{
		case 8:
			SendDlgItemMessage (IDC_QUALITY_COMBO, CB_SETCURSEL, (WPARAM)0);
			break;

		default:
		case 16:
			SendDlgItemMessage (IDC_QUALITY_COMBO, CB_SETCURSEL, (WPARAM)1);
			break;
	}


	//
	//	Select the appropriate kHz combobox entry
	//
	switch (hertz)
	{
		case 11025:
			SendDlgItemMessage (IDC_RATE_COMBO, CB_SETCURSEL, (WPARAM)0);
			break;

		case 22050:
			SendDlgItemMessage (IDC_RATE_COMBO, CB_SETCURSEL, (WPARAM)1);
			break;

		default:
		case 44100:
			SendDlgItemMessage (IDC_RATE_COMBO, CB_SETCURSEL, (WPARAM)2);
			break;
	}

	// Select the appropriate speaker setup combobox entry
	switch (speaker_type) {

		default:
		case 0:
			SendDlgItemMessage (IDC_SPEAKER_COMBO, CB_SETCURSEL, (WPARAM)0);
			break;

		case 1:
			SendDlgItemMessage (IDC_SPEAKER_COMBO, CB_SETCURSEL, (WPARAM)1);
			break;
		
		case 2:
			SendDlgItemMessage (IDC_SPEAKER_COMBO, CB_SETCURSEL, (WPARAM)2);
			break;
	
		case 3:
			SendDlgItemMessage (IDC_SPEAKER_COMBO, CB_SETCURSEL, (WPARAM)3);
			break;
	}

	//
	//	Configure the list control
	//
	m_ListCtrl.SetExtendedStyle (m_ListCtrl.GetExtendedStyle () | LVS_EX_FULLROWSELECT);
	m_ListCtrl.InsertColumn (COL_DRIVER_NAME, "Driver Name");

	//
	//	Size the columns
	//
	CRect rect;
	m_ListCtrl.GetClientRect (&rect);
	int width = rect.Width () - ::GetSystemMetrics (SM_CXVSCROLL);
	m_ListCtrl.SetColumnWidth (0, width);

	//
	//	Loop over all the drivers
	//
	bool selected_default = false;
	int driver_count = WWAudioClass::Get_Instance ()->Get_3D_Device_Count ();
	for (int index = 0; index < driver_count; index ++) {
		
		//
		//	Get information about this sound driver
		//
		WWAudioClass::DRIVER_INFO_STRUCT *driver_info = NULL;
		if (WWAudioClass::Get_Instance ()->Get_3D_Device (index, &driver_info)) {
			
			//
			//	Add an entry to the list for this driver
			//
			int item_index = m_ListCtrl.InsertItem (0xFF, driver_info->name);
			if (item_index >= 0) {
				m_ListCtrl.SetItemData (item_index, (DWORD_PTR)driver_info->driver);

				//
				//	Select this entry if its the default
				//
				if (::lstrcmpi (device_name, driver_info->name) == 0) {
					m_ListCtrl.SetItemState (item_index, LVIS_SELECTED, LVIS_SELECTED);
					selected_default = true;
				}
			}
		}
	}

	//
	//	Select the first entry by default (if necessary)
	//
	if (selected_default == false) {
		m_ListCtrl.SetItemState (0, LVIS_SELECTED, LVIS_SELECTED);
	}

	//
	//	Update the enabled state of the volume sliders
	//
	Update_Slider_Enable_State ();	

	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnDestroy
//
/////////////////////////////////////////////////////////////////////////////
void
AudioConfigDialogClass::OnDestroy (void)
{
	//
	//	Shutdown the audio library
	//
	WWAudioClass::Get_Instance ()->Shutdown ();
	delete WWAudioClass::Get_Instance ();

	CDialog::OnDestroy ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Apply_Changes
//
/////////////////////////////////////////////////////////////////////////////
void
AudioConfigDialogClass::Apply_Changes (void)
{
	StringClass device_name;
	int hertz			  = 44100;
	int bits				  = 16;
	int speaker_type	  = 0;	
	bool is_stereo		  = true;
	bool sound_on		  = true;
	bool music_on		  = true;
	bool dialog_on		  = true;
	bool cinematic_on	  = true;
	float sound_vol	  = 1.0F;
	float music_vol	  = 1.0F;
	float dialog_vol	  = 1.0F;
	float cinematic_vol = 1.0F;

	//
	//	Get the volume settings
	//
	sound_vol	  = m_SoundVolSlider.GetPos () / 100.0F;
	music_vol	  = m_MusicVolSlider.GetPos () / 100.0F;
	dialog_vol	  = m_DialogVolSlider.GetPos () / 100.0F;
	cinematic_vol = m_CinematicVolSlider.GetPos() / 100.0F;

	//
	//	Get the volume controls
	//
	sound_on		 = (SendDlgItemMessage (IDC_SOUND_EFFECTS_CHECK, BM_GETCHECK) == 1);
	music_on		 = (SendDlgItemMessage (IDC_MUSIC_CHECK, BM_GETCHECK) == 1);
	dialog_on	 = (SendDlgItemMessage (IDC_DIALOG_CHECK, BM_GETCHECK) == 1);
	cinematic_on = (SendDlgItemMessage (IDC_CINEMATIC_CHECK, BM_GETCHECK) == 1);

	//
	//	Get the name of the selected device
	//
	int selected_item = m_ListCtrl.GetNextItem (-1, LVNI_ALL | LVNI_SELECTED);
	if (selected_item >= 0) {
		device_name = (const char *)m_ListCtrl.GetItemText (selected_item, 0);
	}

	//
	//	Get the stereo flag from the dialog
	//
	is_stereo = bool(SendDlgItemMessage (IDC_STEREO_CHECK, BM_GETCHECK) == 1);

	//
	//	Get the playback bit rate from the dialog
	//
	int quality_cursel = SendDlgItemMessage (IDC_QUALITY_COMBO, CB_GETCURSEL);
	if (quality_cursel == 0) {
		bits = 8;
	} else if (quality_cursel == 1) {
		bits = 16;
	}

	//
	//	Get the playback rate from the controls
	//
	int rate_cursel = SendDlgItemMessage (IDC_RATE_COMBO, CB_GETCURSEL);
	if (rate_cursel == 0) {
		hertz = 11025;
	} else if (rate_cursel == 1) {
		hertz = 22050;
	} else if (rate_cursel == 2) {
		hertz = 44100;
	}

	// Get the speaker setup from the controls.
	speaker_type = SendDlgItemMessage (IDC_SPEAKER_COMBO, CB_GETCURSEL);

	//
	//	Store these settings in the registry
	//
	WWAudioClass::Get_Instance ()->Save_To_Registry (RENEGADE_SUB_KEY_NAME_AUDIO,
												device_name, is_stereo, bits, hertz,
												sound_on, music_on, dialog_on, cinematic_on,
												sound_vol, music_vol, dialog_vol, cinematic_vol, speaker_type);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// WindowProc
//
/////////////////////////////////////////////////////////////////////////////
LRESULT
AudioConfigDialogClass::WindowProc
(
	UINT		message,
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
// Update_Slider_Enable_State
//
/////////////////////////////////////////////////////////////////////////////
void
AudioConfigDialogClass::Update_Slider_Enable_State (void)
{
	BOOL sound_enabled	  = IsDlgButtonChecked (IDC_SOUND_EFFECTS_CHECK);
	BOOL music_enabled	  = IsDlgButtonChecked (IDC_MUSIC_CHECK);
	BOOL dialog_enabled	  = IsDlgButtonChecked (IDC_DIALOG_CHECK);
	BOOL cinematic_enabled = IsDlgButtonChecked (IDC_CINEMATIC_CHECK);

	//
	//	Enable the slider's based on the state of the check boxes
	//
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_SOUND_EFFECTS_SLIDER), sound_enabled);
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_MUSIC_SLIDER), music_enabled);
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_DIALOG_SLIDER), dialog_enabled);
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_CINEMATIC_SLIDER), cinematic_enabled);

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnCommand
//
/////////////////////////////////////////////////////////////////////////////
BOOL
AudioConfigDialogClass::OnCommand (WPARAM wParam, LPARAM lParam)
{
	switch (LOWORD (wParam)) {

		//
		//	Update the enabled state of the volume sliders
		//
		case IDC_SOUND_EFFECTS_CHECK:
		case IDC_MUSIC_CHECK:
		case IDC_DIALOG_CHECK:
		case IDC_CINEMATIC_CHECK:
			Update_Slider_Enable_State();
			break;
	}
	
	return CDialog::OnCommand (wParam, lParam);
}
