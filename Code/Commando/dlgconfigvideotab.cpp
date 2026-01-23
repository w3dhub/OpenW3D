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
 *                 Project Name : commando                                                     *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Commando/dlgconfigvideotab.cpp               $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/07/02 6:29p                                               $*
 *                                                                                             *
 *                    $Revision:: 10                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "renegadedialog.h"
#include "dlgconfigvideotab.h"
#include "ww3d.h"
#include "sliderctrl.h"
#include "dx8wrapper.h"
#include "cardinalspline.h"
#include "string_ids.h"


////////////////////////////////////////////////////////////////
//	Static member initialization
////////////////////////////////////////////////////////////////
int DlgConfigVideoTabClass::GammaLevel		  = GAMMA_SLIDER_DEFAULT;
int DlgConfigVideoTabClass::BrightnessLevel = BRIGHTNESS_SLIDER_DEFAULT;
int DlgConfigVideoTabClass::ContrastLevel	  = CONTRAST_SLIDER_DEFAULT;


////////////////////////////////////////////////////////////////
//
//	DlgConfigVideoTabClass
//
////////////////////////////////////////////////////////////////
DlgConfigVideoTabClass::DlgConfigVideoTabClass (void)	:
	ChildDialogClass (GetRenegadeDialog(RenegadeDialogID::IDD_CONFIG_VIDEO)),
	UpdateGamma (true)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	~DlgConfigVideoTabClass
//
////////////////////////////////////////////////////////////////
DlgConfigVideoTabClass::~DlgConfigVideoTabClass (void)
{
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_Init_Dialog
//
////////////////////////////////////////////////////////////////
void
DlgConfigVideoTabClass::On_Init_Dialog (void)
{
	//
	//	Get the name of the render device
	//
	int curr_device					= WW3D::Get_Render_Device ();
	StringClass ascii_device_name	= WW3D::Get_Render_Device_Name (curr_device);

	WideStringClass device_name;
	device_name.Convert_From (ascii_device_name);

	//
	//	Get the current resolution
	//
	int width			= 0;
	int height			= 0;
	int bits				= 0;
	bool is_windowed	= false;
	WW3D::Get_Device_Resolution (width, height, bits, is_windowed);

	WideStringClass resolution;
	resolution.Format (U_CHAR("%d x %d"), width, height);

	//
	//	Build a string with the current bit depth
	//
	WideStringClass bit_depth;
	bit_depth.Format (U_CHAR("%d"), bits);

	//
	//	Put the data into the dialog
	//
	Set_Dlg_Item_Text (IDC_DISPLAY_DRIVER, device_name);
	Set_Dlg_Item_Text (IDC_RESOLUTION,		resolution);
	Set_Dlg_Item_Text (IDC_BIT_DEPTH,		bit_depth);
	Enable_Dlg_Item (IDC_DISPLAY_DRIVER,	false);
	Enable_Dlg_Item (IDC_RESOLUTION,			false);
	Enable_Dlg_Item (IDC_BIT_DEPTH,			false);

	SliderCtrlClass *slider;

	// To avoid partial (and incorrect) gamma updates, disable it until all values have been correctly set.
	UpdateGamma = false;

	//
	//	Configure the gamma slider
	//
	slider = (SliderCtrlClass *)Get_Dlg_Item (IDC_GAMMA_SLIDER);
	slider->Set_Range (GAMMA_SLIDER_MIN, GAMMA_SLIDER_MAX);
	slider->Set_Pos (GammaLevel);

	//
	//	Configure the brightness slider
	//
	slider = (SliderCtrlClass *)Get_Dlg_Item (IDC_BRIGHTNESS_SLIDER);
	slider->Set_Range (BRIGHTNESS_SLIDER_MIN, BRIGHTNESS_SLIDER_MAX);
	slider->Set_Pos (BrightnessLevel);

	//
	//	Configure the contrast slider
	//
	slider = (SliderCtrlClass *)Get_Dlg_Item (IDC_CONTRAST_SLIDER);
	slider->Set_Range (CONTRAST_SLIDER_MIN, CONTRAST_SLIDER_MAX);
	
	// Now the gamma can take effect.
	UpdateGamma = true;
	slider->Set_Pos (ContrastLevel);

	ChildDialogClass::On_Init_Dialog ();
	return ;
}


////////////////////////////////////////////////////////////////
//
//	On_End_Dialog
//
////////////////////////////////////////////////////////////////
void
DlgConfigVideoTabClass::On_Destroy (void)
{
	GammaLevel		 = ((SliderCtrlClass *)Get_Dlg_Item (IDC_GAMMA_SLIDER))->Get_Pos();
	BrightnessLevel = ((SliderCtrlClass *)Get_Dlg_Item (IDC_BRIGHTNESS_SLIDER))->Get_Pos();
	ContrastLevel	 = ((SliderCtrlClass *)Get_Dlg_Item (IDC_CONTRAST_SLIDER))->Get_Pos();
}


////////////////////////////////////////////////////////////////
//
//	On_SliderCtrl_Pos_Changed
//
////////////////////////////////////////////////////////////////
void
DlgConfigVideoTabClass::On_SliderCtrl_Pos_Changed
(
	SliderCtrlClass *	slider_ctrl,
	int					ctrl_id,
	int					/* new_pos */
)
{
	const unichar_t *formatstring = U_CHAR("%.2f");
		
	WideStringClass settingstring;

	if (UpdateGamma) {
	
		int g, b, c;	
		
	 	g = ((SliderCtrlClass *)Get_Dlg_Item (IDC_GAMMA_SLIDER))->Get_Pos();
		b = ((SliderCtrlClass *)Get_Dlg_Item (IDC_BRIGHTNESS_SLIDER))->Get_Pos();
		c = ((SliderCtrlClass *)Get_Dlg_Item (IDC_CONTRAST_SLIDER))->Get_Pos();
		Update_Gamma (g, b, c);
	}

	switch (ctrl_id) {

		case IDC_GAMMA_SLIDER:
			settingstring.Format (formatstring, Gamma_Scale (slider_ctrl->Get_Pos()));
			Set_Dlg_Item_Text (IDC_GAMMA_SETTING, settingstring);
			break;
	
		case IDC_BRIGHTNESS_SLIDER:
			settingstring.Format (formatstring, Gamma_Scale (slider_ctrl->Get_Pos()));
			Set_Dlg_Item_Text (IDC_BRIGHTNESS_SETTING, settingstring);
			break;
	
		case IDC_CONTRAST_SLIDER:
			settingstring.Format (formatstring, Gamma_Scale (slider_ctrl->Get_Pos()));
			Set_Dlg_Item_Text (IDC_CONTRAST_SETTING, settingstring);
			break;

		default:
			// Do nothing.
			break;
	}

	return ;
}


