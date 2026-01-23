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
 *                 Project Name : commando                                                    *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Commando/dlgconfigaudiotab.cpp                              $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/25/02 7:12p                                                $*
 *                                                                                             *
 *                    $Revision:: 11                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "renegadedialog.h"
#include "dlgconfigaudiotab.h"
#include "sliderctrl.h"
#include "WWAudio.h"
#include "comboboxctrl.h"
#include "listctrl.h"
#include "_globals.h"
#include "string_ids.h"
#include "translatedb.h"
#include "AudibleSound.h"
#include "dialogmgr.h"


////////////////////////////////////////////////////////////////
//
//	DlgConfigAudioTabClass
//
////////////////////////////////////////////////////////////////
DlgConfigAudioTabClass::DlgConfigAudioTabClass (void)	:
	InitialDeviceIndex (0),
	InitialIsStereo (true),
	InitialHertz (44100),
	InitialBits (16),
	SoundVolumeTestSound (NULL),
	MusicVolumeTestSound (NULL),
	DialogVolumeTestSound (NULL),
	CinematicVolumeTestSound (NULL),
	SoundVolumeTestSoundStartTime (0),
	MusicVolumeTestSoundStartTime (0),
	DialogVolumeTestSoundStartTime (0),
	CinematicVolumeTestSoundStartTime (0),
	ChildDialogClass (GetRenegadeDialog(RenegadeDialogID::IDD_CONFIG_AUDIO))
{
	SoundVolumeTestSound = WWAudioClass::Get_Instance ()->Create_Sound_Effect ("laser_rifle_fire_01.wav");
	if (SoundVolumeTestSound != NULL) {
		SoundVolumeTestSound->Set_Loop_Count (0);
		SoundVolumeTestSound->Set_Type (AudibleSoundClass::TYPE_SOUND_EFFECT);
	}

	MusicVolumeTestSound = WWAudioClass::Get_Instance ()->Create_Sound_Effect ("sakura battle theme.mp3");
	if (MusicVolumeTestSound != NULL) {
		MusicVolumeTestSound->Set_Type (AudibleSoundClass::TYPE_MUSIC);
	}

	DialogVolumeTestSound = WWAudioClass::Get_Instance ()->Create_Sound_Effect ("m00s1_s1s1gbmg_snd.wav");
	if (DialogVolumeTestSound != NULL) {
		DialogVolumeTestSound->Set_Loop_Count (0);
		DialogVolumeTestSound->Set_Type (AudibleSoundClass::TYPE_DIALOG);
	}

	CinematicVolumeTestSound = WWAudioClass::Get_Instance ()->Create_Sound_Effect ("00-n000e.wav");
	if (CinematicVolumeTestSound != NULL) {
		CinematicVolumeTestSound->Set_Loop_Count (0);
		CinematicVolumeTestSound->Set_Type (AudibleSoundClass::TYPE_CINEMATIC);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	~DlgConfigAudioTabClass
//
////////////////////////////////////////////////////////////////
DlgConfigAudioTabClass::~DlgConfigAudioTabClass (void)
{
	if (SoundVolumeTestSound != NULL) {
		SoundVolumeTestSound->Stop ();
		REF_PTR_RELEASE (SoundVolumeTestSound);
	}

	if (MusicVolumeTestSound != NULL) {
		MusicVolumeTestSound->Stop ();
		REF_PTR_RELEASE (MusicVolumeTestSound);
	}
	
	if (DialogVolumeTestSound != NULL) {
		DialogVolumeTestSound->Stop ();
		REF_PTR_RELEASE (DialogVolumeTestSound);
	}

	if (CinematicVolumeTestSound != NULL) {
		CinematicVolumeTestSound->Stop ();
		REF_PTR_RELEASE (CinematicVolumeTestSound);
	}

	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Init_Dialog
//
////////////////////////////////////////////////////////////////
void
DlgConfigAudioTabClass::On_Init_Dialog (void)
{
	ChildDialogClass::On_Init_Dialog ();

	//
	//	Read the audio library's settings from the registry
	//	
	bool is_stereo			= WWAudioClass::Get_Instance ()->Get_Playback_Stereo ();	
	float sound_vol		= WWAudioClass::Get_Instance ()->Get_Sound_Effects_Volume ();
	float music_vol		= WWAudioClass::Get_Instance ()->Get_Music_Volume ();
	float dialog_vol		= WWAudioClass::Get_Instance ()->Get_Dialog_Volume ();
	float cinematic_vol	= WWAudioClass::Get_Instance ()->Get_Cinematic_Volume ();
	bool sound_on			= WWAudioClass::Get_Instance ()->Are_Sound_Effects_On ();
	bool music_on			= WWAudioClass::Get_Instance ()->Is_Music_On ();
	bool dialog_on			= WWAudioClass::Get_Instance ()->Is_Dialog_On ();
	bool cinematic_on		= WWAudioClass::Get_Instance ()->Is_Cinematic_Sound_On ();
		
	SliderCtrlClass *snd_vol_slider = (SliderCtrlClass *)Get_Dlg_Item (IDC_SOUND_EFFECTS_SLIDER);
	SliderCtrlClass *mus_vol_slider = (SliderCtrlClass *)Get_Dlg_Item (IDC_MUSIC_SLIDER);
	SliderCtrlClass *dia_vol_slider = (SliderCtrlClass *)Get_Dlg_Item (IDC_DIALOG_SLIDER);
	SliderCtrlClass *cin_vol_slider = (SliderCtrlClass *)Get_Dlg_Item (IDC_CINEMATIC_SLIDER);	
	
	//
	//	Setup the sound volume controls
	//
	Check_Dlg_Button (IDC_SOUND_EFFECTS_CHECK, sound_on);
	if (snd_vol_slider != NULL) {
		snd_vol_slider->Set_Range (0, 100);
		snd_vol_slider->Set_Pos (static_cast<int>(sound_vol * 100), false);
	}

	//
	//	Setup the music volume controls
	//
	Check_Dlg_Button (IDC_MUSIC_CHECK, music_on);
	if (mus_vol_slider != NULL) {
		mus_vol_slider->Set_Range (0, 100);
		mus_vol_slider->Set_Pos (static_cast<int>(music_vol * 100), false);
	}

	//
	//	Setup the dialog volume controls
	//
	Check_Dlg_Button (IDC_DIALOG_CHECK, dialog_on);
	if (dia_vol_slider != NULL) {
		dia_vol_slider->Set_Range (0, 100);
		dia_vol_slider->Set_Pos (static_cast<int>(dialog_vol * 100), false);
	}

	//
	//	Setup the dialog volume controls
	//
	Check_Dlg_Button (IDC_CINEMATIC_CHECK, cinematic_on);
	if (cin_vol_slider != NULL) {
		cin_vol_slider->Set_Range (0, 100);
		cin_vol_slider->Set_Pos (static_cast<int>(cinematic_vol * 100), false);
	}	

	//
	//	Check the stereo box if necessary
	//
	Check_Dlg_Button (IDC_STEREO_CHECK, is_stereo);
	InitialIsStereo = is_stereo;

	//
	//	Configure the comboboxes and list control
	//	
	Configure_Quality_Combobox ();
	Configure_Rate_Combobox ();
	Configure_Speaker_Combobox ();
	Configure_Driver_List ();
	
	//
	//	Update the enabled state of the volume sliders
	//
	Update_Slider_Enable_State ();	
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Update_Slider_Enable_State
//
/////////////////////////////////////////////////////////////////////////////
void
DlgConfigAudioTabClass::Update_Slider_Enable_State (void)
{
	bool sound_enabled		= Is_Dlg_Button_Checked (IDC_SOUND_EFFECTS_CHECK);
	bool music_enabled		= Is_Dlg_Button_Checked (IDC_MUSIC_CHECK);
	bool dialog_enabled		= Is_Dlg_Button_Checked (IDC_DIALOG_CHECK);
	bool cinematic_enabled	= Is_Dlg_Button_Checked (IDC_CINEMATIC_CHECK);

	//
	//	Enable the slider's based on the state of the check boxes
	//
	Enable_Dlg_Item (IDC_SOUND_EFFECTS_SLIDER,	sound_enabled);
	Enable_Dlg_Item (IDC_MUSIC_SLIDER,				music_enabled);
	Enable_Dlg_Item (IDC_DIALOG_SLIDER,				dialog_enabled);
	Enable_Dlg_Item (IDC_CINEMATIC_SLIDER,			cinematic_enabled);	

	//
	//	Update the audio itself
	//
	WWAudioClass::Get_Instance ()->Allow_Sound_Effects (sound_enabled);
	WWAudioClass::Get_Instance ()->Allow_Music (music_enabled);
	WWAudioClass::Get_Instance ()->Allow_Dialog (dialog_enabled);
	WWAudioClass::Get_Instance ()->Allow_Cinematic_Sound (cinematic_enabled);
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Configure_Rate_Combobox
//
//////////////////////////////////////////////////////////////////////
void	
DlgConfigAudioTabClass::Configure_Rate_Combobox (void)
{
	ComboBoxCtrlClass *combo_box = (ComboBoxCtrlClass *)Get_Dlg_Item (IDC_RATE_COMBO);
	if (combo_box == NULL) {
		return ;
	}

	//
	//	Add an entry for each locale to the combobox
	//
	combo_box->Add_String (TRANSLATE (IDS_SND_RATE_11));
	combo_box->Add_String (TRANSLATE (IDS_SND_RATE_22));
	combo_box->Add_String (TRANSLATE (IDS_SND_RATE_44));

	InitialHertz = WWAudioClass::Get_Instance ()->Get_Playback_Rate ();
	switch (InitialHertz)
	{
		case 11025:
			combo_box->Set_Curr_Sel (0);
			break;

		case 22050:
			combo_box->Set_Curr_Sel (1);
			break;

		default:
		case 44100:
			combo_box->Set_Curr_Sel (2);
			break;
	}

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Configure_Speaker_Combobox
//
//////////////////////////////////////////////////////////////////////
void	
DlgConfigAudioTabClass::Configure_Speaker_Combobox (void)
{
	ComboBoxCtrlClass *combo_box = (ComboBoxCtrlClass *)Get_Dlg_Item (IDC_SPEAKER_SETUP_COMBO);
	if (combo_box == NULL) {
		return ;
	}

	//
	//	Add an entry for each speaker type to the combobox
	//
	combo_box->Add_String (TRANSLATE (IDS_MENU_SPEAKER_2));
	combo_box->Add_String (TRANSLATE (IDS_MENU_SPEAKER_HEADPHONE));
	combo_box->Add_String (TRANSLATE (IDS_MENU_SPEAKER_SURROUND));
	combo_box->Add_String (TRANSLATE (IDS_MENU_SPEAKER_4));

	//
	//	Select the current setting
	//
	int type = WWAudioClass::Get_Instance ()->Get_Speaker_Type ();
	switch (type)
	{
		default:
		case W3D_3D_2_SPEAKER:
			combo_box->Set_Curr_Sel (0);
			break;

		case W3D_3D_HEADPHONE:
			combo_box->Set_Curr_Sel (1);
			break;

		case W3D_3D_SURROUND:
			combo_box->Set_Curr_Sel (2);
			break;

		case W3D_3D_4_SPEAKER:
			combo_box->Set_Curr_Sel (3);
			break;
	}

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Configure_Quality_Combobox
//
//////////////////////////////////////////////////////////////////////
void	
DlgConfigAudioTabClass::Configure_Quality_Combobox (void)
{
	ComboBoxCtrlClass *combo_box = (ComboBoxCtrlClass *)Get_Dlg_Item (IDC_QUALITY_COMBO);
	if (combo_box == NULL) {
		return ;
	}

	//
	//	Add an entry for each locale to the combobox
	//
	combo_box->Add_String (TRANSLATE (IDS_SND_8_BIT));
	combo_box->Add_String (TRANSLATE (IDS_SND_16_BIT));

	InitialBits = WWAudioClass::Get_Instance ()->Get_Playback_Bits ();	

	//
	//	Select the appropriate quality combobox entry
	//
	switch (InitialBits)
	{
		case 8:
			combo_box->Set_Curr_Sel (0);
			break;

		default:
		case 16:
			combo_box->Set_Curr_Sel (1);
			break;
	}

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	On_Command
//
//////////////////////////////////////////////////////////////////////
void	
DlgConfigAudioTabClass::On_Command (int ctrl_id, int message_id, unsigned int param)
{
	switch (ctrl_id)
	{
		case IDC_SOUND_EFFECTS_CHECK:
		case IDC_MUSIC_CHECK:
		case IDC_DIALOG_CHECK:
		case IDC_CINEMATIC_CHECK:
			Update_Slider_Enable_State ();
			break;

		case IDC_DEFAULTS_BUTTON:
			Set_Default_Volumes ();
			break;
	}

	ChildDialogClass::On_Command (ctrl_id, message_id, param);
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Configure_Driver_List
//
//////////////////////////////////////////////////////////////////////
void	
DlgConfigAudioTabClass::Configure_Driver_List (void)
{
	ListCtrlClass *list_ctrl = (ListCtrlClass *)Get_Dlg_Item (IDC_DRIVER_LIST);
	if (list_ctrl == NULL) {
		return ;
	}

	//
	//	Get the name of the current 3D driver
	//
	StringClass device_name = WWAudioClass::Get_Instance ()->Get_3D_Driver_Name ();

	//
	//	Configure the column
	//
	list_ctrl->Add_Column (TRANSLATE (IDS_SND_DRIVER_COL), 1.0F, Vector3 (1, 1, 1));

	//
	//	Loop over all the drivers
	//
	bool selected_default = false;
	int driver_count = WWAudioClass::Get_Instance ()->Get_3D_Device_Count ();
	for (int index = 0; index < driver_count; index ++) {
		
		//
		//	Get information about this sound driver
		//
		const char *driver_info = NULL;
		if (WWAudioClass::Get_Instance ()->Get_3D_Device (index, &driver_info)) {

			WideStringClass wide_driver_name;
			wide_driver_name.Convert_From (driver_info);

			//
			//	Add an entry to the list for this driver
			//
			int item_index = list_ctrl->Insert_Entry (index, wide_driver_name);
			if (item_index >= 0) {
				list_ctrl->Set_Entry_Data (item_index, 0, (uintptr_t)0);

				//
				//	Select this entry if its the default
				//
				if (::stricmp (device_name, driver_info) == 0) {
					list_ctrl->Set_Curr_Sel (item_index);
					selected_default = true;
				}
			}
		}
	}

	//
	//	Select the first entry by default (if necessary)
	//
	if (selected_default == false) {
		list_ctrl->Set_Curr_Sel (0);
	}

	//
	//	Record the initial device index so we don't have
	// to recreate the device if its not necessary
	//
	InitialDeviceIndex = list_ctrl->Get_Curr_Sel ();		
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// On_Apply
//
/////////////////////////////////////////////////////////////////////////////
bool
DlgConfigAudioTabClass::On_Apply (void)
{
	StringClass device_name;
	int hertz				= 44100;
	int bits					= 16;
	int speaker_type		= 0;
	bool is_stereo			= true;
	float sound_vol		= 1.0F;
	float music_vol		= 1.0F;
	float dialog_vol		= 1.0F;
	float cinematic_vol	= 1.0F;
	bool sound_on			= true;
	bool music_on			= true;
	bool dialog_on			= true;
	bool cinematic_on		= true;

	SliderCtrlClass *snd_vol_slider = (SliderCtrlClass *)Get_Dlg_Item (IDC_SOUND_EFFECTS_SLIDER);
	SliderCtrlClass *mus_vol_slider = (SliderCtrlClass *)Get_Dlg_Item (IDC_MUSIC_SLIDER);
	SliderCtrlClass *dia_vol_slider = (SliderCtrlClass *)Get_Dlg_Item (IDC_DIALOG_SLIDER);
	SliderCtrlClass *cin_vol_slider = (SliderCtrlClass *)Get_Dlg_Item (IDC_CINEMATIC_SLIDER);

	//
	//	Get the volume settings
	//
	sound_vol		= snd_vol_slider->Get_Pos () / 100.0F;
	music_vol		= mus_vol_slider->Get_Pos () / 100.0F;
	dialog_vol		= dia_vol_slider->Get_Pos () / 100.0F;
	cinematic_vol	= cin_vol_slider->Get_Pos () / 100.0F;

	//
	//	Setup the music volume controls
	//
	sound_on			= Is_Dlg_Button_Checked (IDC_SOUND_EFFECTS_CHECK);
	music_on			= Is_Dlg_Button_Checked (IDC_MUSIC_CHECK);
	dialog_on		= Is_Dlg_Button_Checked (IDC_DIALOG_CHECK);
	cinematic_on	= Is_Dlg_Button_Checked (IDC_CINEMATIC_CHECK);
	
	//
	//	Get the name of the selected device
	//	
	ListCtrlClass *list_ctrl = (ListCtrlClass *)Get_Dlg_Item (IDC_DRIVER_LIST);
	int device_index = list_ctrl->Get_Curr_Sel ();
	if (device_index >= 0) {
		WideStringClass wide_device_name = list_ctrl->Get_Entry_Text (device_index, 0);
		wide_device_name.Convert_To (device_name);
	}

	//
	//	Get the stereo flag from the dialog
	//
	is_stereo = Is_Dlg_Button_Checked (IDC_STEREO_CHECK);

	//
	//	Get the playback bit rate from the dialog
	//
	ComboBoxCtrlClass *combo_box = (ComboBoxCtrlClass *)Get_Dlg_Item (IDC_QUALITY_COMBO);
	int quality_cursel = combo_box->Get_Curr_Sel ();
	if (quality_cursel == 0) {
		bits = 8;
	} else if (quality_cursel == 1) {
		bits = 16;
	}

	//
	//	Get the playback rate from the controls
	//
	combo_box = (ComboBoxCtrlClass *)Get_Dlg_Item (IDC_RATE_COMBO);
	int rate_cursel = combo_box->Get_Curr_Sel ();
	if (rate_cursel == 0) {
		hertz = 11025;
	} else if (rate_cursel == 1) {
		hertz = 22050;
	} else if (rate_cursel == 2) {
		hertz = 44100;
	}

	//
	//	Get the playback rate from the controls
	//
	combo_box = (ComboBoxCtrlClass *)Get_Dlg_Item (IDC_SPEAKER_SETUP_COMBO);
	speaker_type = combo_box->Get_Curr_Sel ();

	//
	//	Store these settings in the registry
	//
	WWAudioClass::Get_Instance ()->Save_To_Registry (APPLICATION_SUB_KEY_NAME_SOUND,
												device_name, is_stereo, bits, hertz, sound_on,	
												music_on, dialog_on, cinematic_on, sound_vol, music_vol,
												dialog_vol, cinematic_vol, speaker_type);

	//
	//	Don't change drivers unless necessary
	//
	if (InitialBits != bits || InitialHertz != hertz || InitialDeviceIndex != device_index || is_stereo != InitialIsStereo) {
		WWAudioClass::Get_Instance ()->Open_2D_Device (is_stereo, bits, hertz);
		WWAudioClass::Get_Instance ()->Select_3D_Device (device_name);
	}

	//
	//	Apply the simple changes
	//
	WWAudioClass::Get_Instance ()->Set_Sound_Effects_Volume (sound_vol);
	WWAudioClass::Get_Instance ()->Set_Music_Volume (music_vol);
	WWAudioClass::Get_Instance ()->Set_Dialog_Volume (dialog_vol);
	WWAudioClass::Get_Instance ()->Set_Cinematic_Volume (cinematic_vol);
	WWAudioClass::Get_Instance ()->Allow_Sound_Effects (sound_on);
	WWAudioClass::Get_Instance ()->Allow_Music (music_on);
	WWAudioClass::Get_Instance ()->Allow_Dialog (dialog_on);
	WWAudioClass::Get_Instance ()->Allow_Cinematic_Sound (cinematic_on);
	WWAudioClass::Get_Instance ()->Set_Speaker_Type (speaker_type);
	return true;
}


/////////////////////////////////////////////////////////////////////////////
//
// On_SliderCtrl_Pos_Changed
//
/////////////////////////////////////////////////////////////////////////////
void
DlgConfigAudioTabClass::On_SliderCtrl_Pos_Changed
(
	SliderCtrlClass *	slider_ctrl,
	int					ctrl_id,
	int					/* new_pos */
)
{
	if (ctrl_id == IDC_SOUND_EFFECTS_SLIDER) {
		
		//
		//	Update the sound effects volume...
		//
		float sound_vol = slider_ctrl->Get_Pos () / 100.0F;
		WWAudioClass::Get_Instance ()->Set_Sound_Effects_Volume (sound_vol);

		//
		//	Start the sound effect sound playing
		//
		if (	SoundVolumeTestSound != NULL &&
				(SoundVolumeTestSound->Is_Playing () == false || ((DialogMgrClass::Get_Time () - SoundVolumeTestSoundStartTime) > 150)))
		{			
			SoundVolumeTestSound->Stop ();
			SoundVolumeTestSound->Set_Loop_Count (0);
			SoundVolumeTestSound->Play ();

			//
			//	Record a time stamp for this sound...
			//
			SoundVolumeTestSoundStartTime = DialogMgrClass::Get_Time ();
		}

	} else if (ctrl_id == IDC_MUSIC_SLIDER) {

		//
		//	Update the music volume...
		//
		float music_vol = slider_ctrl->Get_Pos () / 100.0F;
		WWAudioClass::Get_Instance ()->Set_Music_Volume (music_vol);

		//
		//	Start the music sound playing
		//
		if (MusicVolumeTestSound != NULL && MusicVolumeTestSound->Is_Playing () == false) {
			MusicVolumeTestSound->Play ();
		}

		//
		//	Record a time stamp for this sound...
		//
		MusicVolumeTestSoundStartTime = DialogMgrClass::Get_Time ();
	} else if (ctrl_id == IDC_DIALOG_SLIDER) {
		
		//
		//	Update the dialog volume...
		//
		float dialog_vol = slider_ctrl->Get_Pos () / 100.0F;
		WWAudioClass::Get_Instance ()->Set_Dialog_Volume (dialog_vol);

		//
		//	Start the dialog effect sound playing
		//
		if (DialogVolumeTestSound != NULL) {
			DialogVolumeTestSound->Set_Loop_Count (0);
			DialogVolumeTestSound->Play ();

			//
			//	Record a time stamp for this sound...
			//
			DialogVolumeTestSoundStartTime = DialogMgrClass::Get_Time ();
		}

	} else if (ctrl_id == IDC_CINEMATIC_SLIDER) {
		
		//
		//	Update the dialog volume...
		//
		float cinematic_vol = slider_ctrl->Get_Pos () / 100.0F;
		WWAudioClass::Get_Instance ()->Set_Cinematic_Volume (cinematic_vol);

		//
		//	Start the dialog effect sound playing
		//
		if (CinematicVolumeTestSound != NULL) {
			CinematicVolumeTestSound->Set_Loop_Count (0);
			CinematicVolumeTestSound->Play ();

			//
			//	Record a time stamp for this sound...
			//
			CinematicVolumeTestSoundStartTime = DialogMgrClass::Get_Time ();
		}
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// On_Frame_Update
//
/////////////////////////////////////////////////////////////////////////////
void
DlgConfigAudioTabClass::On_Frame_Update (void)
{
	int curr_time = DialogMgrClass::Get_Time ();

	//
	//	Stop the sound volume test sound if necessary
	//
	if (SoundVolumeTestSound != NULL && (curr_time - SoundVolumeTestSoundStartTime) > 500) {
		SoundVolumeTestSound->Set_Loop_Count (1);
	}

	//
	//	Stop the sound volume test sound if necessary
	//
	if (MusicVolumeTestSound != NULL && (curr_time - MusicVolumeTestSoundStartTime) > 2000) {
		MusicVolumeTestSound->Stop ();
	}

	//
	//	Stop the dialog test sound if necessary
	//
	if (DialogVolumeTestSound != NULL && (curr_time - DialogVolumeTestSoundStartTime) > 2000) {
		DialogVolumeTestSound->Set_Loop_Count (1);
	}

	//
	//	Stop the cinematic test sound if necessary
	//
	if (CinematicVolumeTestSound != NULL && (curr_time - CinematicVolumeTestSoundStartTime) > 2000) {
		CinematicVolumeTestSound->Set_Loop_Count (1);
	}
	
	ChildDialogClass::On_Frame_Update ();	
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Set_Default_Volumes
//
/////////////////////////////////////////////////////////////////////////////
void
DlgConfigAudioTabClass::Set_Default_Volumes (void)
{
	int defaultmusicvolume, defaultsoundvolume, defaultdialogvolume, defaultcinematicvolume;

	SliderCtrlClass *snd_vol_slider = (SliderCtrlClass *)Get_Dlg_Item (IDC_SOUND_EFFECTS_SLIDER);
	SliderCtrlClass *mus_vol_slider = (SliderCtrlClass *)Get_Dlg_Item (IDC_MUSIC_SLIDER);
	SliderCtrlClass *dia_vol_slider = (SliderCtrlClass *)Get_Dlg_Item (IDC_DIALOG_SLIDER);
	SliderCtrlClass *cin_vol_slider = (SliderCtrlClass *)Get_Dlg_Item (IDC_CINEMATIC_SLIDER);	
  
	// IML: Get the default settings from the audio system.
	WWAudioClass::Get_Instance()->Load_Default_Volume (defaultmusicvolume, defaultsoundvolume, defaultdialogvolume, defaultcinematicvolume);

	snd_vol_slider->Set_Pos (defaultsoundvolume);
	mus_vol_slider->Set_Pos (defaultmusicvolume);
	dia_vol_slider->Set_Pos (defaultdialogvolume);
	cin_vol_slider->Set_Pos (defaultcinematicvolume);
	
	return ;
}
