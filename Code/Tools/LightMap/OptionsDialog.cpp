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
 ***                            Confidential - Westwood Studios                              *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : LightMap                                                     * 
 *                                                                                             * 
 *                     $Archive:: /Commando/Code/Tool $* 
 *                                                                                             * 
 *                      $Author:: Ian_l               $* 
 *                                                                                             * 
 *                     $Modtime:: 9/08/00 6:37p       $* 
 *                                                                                             * 
 *                    $Revision:: 7                                                         $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

// Includes.
#include "StdAfx.h"
#include "LightMap.h"
#include "OptionsDialog.h"
#include "StringBuilder.h"

// The following is maintained by MFC tools.
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Defines.
#define OPTIONS_SECTION_NAME					"Options"
#define SPATIAL_TOLERANCE_NAME				"Spatial Tolerance"
#define SMOOTHING_ANGLE_NAME					"Smoothing Angle"
#define FILTER_SHARPNESS_NAME					"Filter Sharpness"
#define FILL_COLOR_RED_NAME					"Fill Color Red"
#define FILL_COLOR_GREEN_NAME					"Fill Color Green"
#define FILL_COLOR_BLUE_NAME					"Fill Color Blue"
#define SCALE_FACTOR_NAME						"Scale Factor"
#define SAMPLE_RATE_NAME						"Sample Rate"
#define FILTER_ERROR_NAME						"Filter Error"
#define BIT_DEPTH_NAME							"Bit Depth"
#define LIGHT_EXPORT_SELECTIVE_NAME			"Light Export Selective"
#define LIGHT_EXCLUSION_STRING_NAME			"Light Exclusion String"
#define PERCENT_SLIDER_TICK_COUNT			101
#define SMOOTHING_ANGLE_TICK_COUNT			181
#define MIN_SAMPLE_RATE							1
#define MAX_SAMPLE_RATE							9999	
#define SAMPLE_RATE_VALUE_CONTROL_STRING	"%4.1f"	


/***********************************************************************************************
 * OptionsDialog::OptionsDialog -- Constructor																  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/1/99    IML : Created.                                                                  * 
 *=============================================================================================*/
OptionsDialog::OptionsDialog (CWnd *parent)
	: CDialog (OptionsDialog::IDD, parent)
{
	SpatialTolerance		= AfxGetApp()->GetProfileInt (OPTIONS_SECTION_NAME, SPATIAL_TOLERANCE_NAME, 1);
	SmoothingAngle			= AfxGetApp()->GetProfileInt (OPTIONS_SECTION_NAME, SMOOTHING_ANGLE_NAME, 89);
	FilterSharpness		= AfxGetApp()->GetProfileInt (OPTIONS_SECTION_NAME, FILTER_SHARPNESS_NAME, 50);
	FillColor.R				= AfxGetApp()->GetProfileInt (OPTIONS_SECTION_NAME, FILL_COLOR_RED_NAME, 0);
	FillColor.G				= AfxGetApp()->GetProfileInt (OPTIONS_SECTION_NAME, FILL_COLOR_GREEN_NAME, _UI8_MAX);
	FillColor.B				= AfxGetApp()->GetProfileInt (OPTIONS_SECTION_NAME, FILL_COLOR_BLUE_NAME, 0);
	SampleRate				= AfxGetApp()->GetProfileInt (OPTIONS_SECTION_NAME, SAMPLE_RATE_NAME,	50);
	FilterError				= AfxGetApp()->GetProfileInt (OPTIONS_SECTION_NAME, FILTER_ERROR_NAME, 50);
	BitDepth					= AfxGetApp()->GetProfileInt (OPTIONS_SECTION_NAME, BIT_DEPTH_NAME, 16);
	LightExportSelective = AfxGetApp()->GetProfileInt (OPTIONS_SECTION_NAME, LIGHT_EXPORT_SELECTIVE_NAME, 1) > 0 ? true : false;
	LightExclusionString = AfxGetApp()->GetProfileString (OPTIONS_SECTION_NAME, LIGHT_EXCLUSION_STRING_NAME, "@");
}


void OptionsDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(OptionsDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(OptionsDialog, CDialog)
	//{{AFX_MSG_MAP(OptionsDialog)
	ON_WM_HSCROLL()
	ON_EN_CHANGE(IDC_SPATIAL_TOLERANCE, OnChangeSpatialTolerance)
	ON_EN_UPDATE(IDC_SPATIAL_TOLERANCE, OnUpdateSpatialTolerance)
	ON_EN_CHANGE(IDC_FILL_COLOR_RED, OnChangeFillColorRed)
	ON_EN_CHANGE(IDC_FILL_COLOR_GREEN, OnChangeFillColorGreen)
	ON_EN_CHANGE(IDC_FILL_COLOR_BLUE, OnChangeFillColorBlue)
	ON_EN_UPDATE(IDC_FILL_COLOR_RED, OnUpdateFillColorRed)
	ON_EN_UPDATE(IDC_FILL_COLOR_GREEN, OnUpdateFillColorGreen)
	ON_EN_UPDATE(IDC_FILL_COLOR_BLUE, OnUpdateFillColorBlue)
	ON_BN_CLICKED(IDC_16_BITS_PER_PIXEL, On16BitsPerPixel)
	ON_BN_CLICKED(IDC_24_BITS_PER_PIXEL, On24BitsPerPixel)
	ON_WM_VSCROLL()
	ON_BN_CLICKED(IDC_LIGHT_EXPORT_SELECTIVE, OnLightExportSelective)
	ON_EN_CHANGE(IDC_LIGHT_EXCLUSION_STRING, OnChangeLightExclusionString)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/***********************************************************************************************
 * OptionsDialog::OnInitDialog --																				  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/2/99    IML : Created.                                                                 * 
 *=============================================================================================*/
BOOL OptionsDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();

	// Initialize.
	SetDlgItemInt (IDC_SPATIAL_TOLERANCE, SpatialTolerance, false);
  	Initialize_Slider (IDC_SMOOTHING_ANGLE, 0, SMOOTHING_ANGLE_TICK_COUNT - 1, SmoothingAngle);
	Set_Text (IDC_SMOOTHING_ANGLE_VALUE, "%d", (int) SmoothingAngle);
  	Initialize_Slider (IDC_FILTER_SHARPNESS, 0, PERCENT_SLIDER_TICK_COUNT - 1, FilterSharpness);
	Set_Text (IDC_FILTER_SHARPNESS_VALUE, "%d", (int) FilterSharpness);
	SetDlgItemInt (IDC_FILL_COLOR_RED,   FillColor.R, false);
	SetDlgItemInt (IDC_FILL_COLOR_GREEN, FillColor.G, false);
	SetDlgItemInt (IDC_FILL_COLOR_BLUE,  FillColor.B, false);
	Initialize_Spinner (IDC_SAMPLE_RATE, MIN_SAMPLE_RATE, MAX_SAMPLE_RATE, SampleRate); 
	Set_Text (IDC_SAMPLE_RATE_VALUE, SAMPLE_RATE_VALUE_CONTROL_STRING, Get_Sample_Rate());
  	Initialize_Slider (IDC_FILTER_ERROR, 0, PERCENT_SLIDER_TICK_COUNT - 1, FilterError);
	Set_Text (IDC_FILTER_ERROR_VALUE, "%d", (int) FilterError);

	// Initialize bit depth option.
	switch (BitDepth) {

		case 16:
			CheckDlgButton (IDC_16_BITS_PER_PIXEL, 1);
			break;

		case 24:
			CheckDlgButton (IDC_24_BITS_PER_PIXEL, 1);
			break;
			
		default:
			ASSERT (false);
			break;
	}
	CheckDlgButton (IDC_LIGHT_EXPORT_SELECTIVE, LightExportSelective);
	GetDlgItem (IDC_LIGHT_EXCLUSION_STRING)->SetWindowText (LightExclusionString);
	GetDlgItem (IDC_LIGHT_EXCLUSION_STRING)->EnableWindow (LightExportSelective);

	return true;  // return true unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return false
}


/***********************************************************************************************
 * OptionsDialog::DoModal --																						  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/2/99    IML : Created.                                                                 * 
 *=============================================================================================*/
int OptionsDialog::DoModal()
{
	int result;

	result = CDialog::DoModal();
	if (result == IDOK) {
		AfxGetApp()->WriteProfileInt (OPTIONS_SECTION_NAME, SPATIAL_TOLERANCE_NAME, SpatialTolerance);
		AfxGetApp()->WriteProfileInt (OPTIONS_SECTION_NAME, SMOOTHING_ANGLE_NAME, SmoothingAngle);
		AfxGetApp()->WriteProfileInt (OPTIONS_SECTION_NAME, FILTER_SHARPNESS_NAME, FilterSharpness);
		AfxGetApp()->WriteProfileInt (OPTIONS_SECTION_NAME, FILL_COLOR_RED_NAME, FillColor.R);
		AfxGetApp()->WriteProfileInt (OPTIONS_SECTION_NAME, FILL_COLOR_GREEN_NAME, FillColor.G);
		AfxGetApp()->WriteProfileInt (OPTIONS_SECTION_NAME, FILL_COLOR_BLUE_NAME, FillColor.B);
		AfxGetApp()->WriteProfileInt (OPTIONS_SECTION_NAME, SAMPLE_RATE_NAME, SampleRate);
		AfxGetApp()->WriteProfileInt (OPTIONS_SECTION_NAME, FILTER_ERROR_NAME, FilterError);
		AfxGetApp()->WriteProfileInt (OPTIONS_SECTION_NAME, BIT_DEPTH_NAME, BitDepth);
		AfxGetApp()->WriteProfileInt (OPTIONS_SECTION_NAME, LIGHT_EXPORT_SELECTIVE_NAME, LightExportSelective);
		AfxGetApp()->WriteProfileString (OPTIONS_SECTION_NAME, LIGHT_EXCLUSION_STRING_NAME, LightExclusionString);
	}
	return (result);
}


/***********************************************************************************************
 * OptionsDialog::OnChangeSpatialTolerance --																  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/03/00    IML : Created.                                                                * 
 *=============================================================================================*/
void OptionsDialog::OnChangeSpatialTolerance() 
{
	SpatialTolerance = GetDlgItemInt (IDC_SPATIAL_TOLERANCE, NULL, false);
}

void OptionsDialog::OnUpdateSpatialTolerance() 
{
	const unsigned maxtolerance = 10000;

	unsigned v = GetDlgItemInt (IDC_SPATIAL_TOLERANCE, NULL, false);
	if (v > maxtolerance) SetDlgItemInt (IDC_SPATIAL_TOLERANCE, maxtolerance, false);
}


/***********************************************************************************************
 * OptionsDialog::OnChangeFillColorRed --																		  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/16/99    IML : Created.                                                                * 
 *=============================================================================================*/
void OptionsDialog::OnChangeFillColorRed() 
{
	FillColor.R = GetDlgItemInt (IDC_FILL_COLOR_RED, NULL, false);
}

void OptionsDialog::OnUpdateFillColorRed() 
{
	unsigned v = GetDlgItemInt (IDC_FILL_COLOR_RED, NULL, false);
	if (v > _UI8_MAX) SetDlgItemInt (IDC_FILL_COLOR_RED, _UI8_MAX, false);
}


/***********************************************************************************************
 * OptionsDialog::OnChangeFillColorGreen --																	  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/16/99    IML : Created.                                                                * 
 *=============================================================================================*/
void OptionsDialog::OnChangeFillColorGreen() 
{
	FillColor.G = GetDlgItemInt (IDC_FILL_COLOR_GREEN, NULL, false);
}

void OptionsDialog::OnUpdateFillColorGreen() 
{
	unsigned v = GetDlgItemInt (IDC_FILL_COLOR_GREEN, NULL, false);
	if (v > _UI8_MAX) SetDlgItemInt (IDC_FILL_COLOR_GREEN, _UI8_MAX, false);
}



/***********************************************************************************************
 * OptionsDialog::OnChangeFillColorBlue --																	  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/16/99    IML : Created.                                                                * 
 *=============================================================================================*/
void OptionsDialog::OnChangeFillColorBlue() 
{
	FillColor.B = GetDlgItemInt (IDC_FILL_COLOR_BLUE, NULL, false);
}

void OptionsDialog::OnUpdateFillColorBlue() 
{
	unsigned v = GetDlgItemInt (IDC_FILL_COLOR_BLUE, NULL, false);
	if (v > _UI8_MAX) SetDlgItemInt (IDC_FILL_COLOR_BLUE, _UI8_MAX, false);
}


/***********************************************************************************************
 * OptionsDialog::On16BitsPerPixel --																			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/2/99    IML : Created.                                                                 * 
 *=============================================================================================*/
void OptionsDialog::On16BitsPerPixel() 
{
	BitDepth = 16;
}


/***********************************************************************************************
 * OptionsDialog::On24BitsPerPixel --																			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/2/99    IML : Created.                                                                 * 
 *=============================================================================================*/
void OptionsDialog::On24BitsPerPixel() 
{
	BitDepth = 24;
}


/***********************************************************************************************
 * OptionsDialog::OnHScroll --																					  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/2/99    IML : Created.                                                                 * 
 *=============================================================================================*/
void OptionsDialog::OnHScroll (UINT sbcode, UINT pos, CScrollBar *scrollbar) 
{
	int controlid = scrollbar->GetDlgCtrlID();

	CSliderCtrl *slider = (CSliderCtrl*) GetDlgItem (controlid);
	ASSERT (slider != NULL);
	switch (controlid) {

		case IDC_SMOOTHING_ANGLE:
			SmoothingAngle = slider->GetPos();
			Set_Text (IDC_SMOOTHING_ANGLE_VALUE, "%d", (int) SmoothingAngle);
			break;

		case IDC_FILTER_SHARPNESS:
			FilterSharpness = slider->GetPos();
			Set_Text (IDC_FILTER_SHARPNESS_VALUE, "%d", (int) FilterSharpness);
			break;

		case IDC_FILTER_ERROR:
			FilterError = slider->GetPos();
			Set_Text (IDC_FILTER_ERROR_VALUE, "%d", (int) FilterError);
			break;

		default:
			break;
	}
	
	CDialog::OnHScroll (sbcode, pos, scrollbar);
}


/***********************************************************************************************
 * OptionsDialog::OnVScroll --																					  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/16/00    IML : Created.                                                                * 
 *=============================================================================================*/
void OptionsDialog::OnVScroll (UINT sbcode, UINT pos, CScrollBar *scrollbar) 
{
	int controlid = scrollbar->GetDlgCtrlID();

	CSpinButtonCtrl *spinner = (CSpinButtonCtrl*) GetDlgItem (controlid);
	ASSERT (spinner != NULL);
	switch (controlid) {

		case IDC_SAMPLE_RATE:
			SampleRate = spinner->GetPos() & 0xffff;
			Set_Text (IDC_SAMPLE_RATE_VALUE, SAMPLE_RATE_VALUE_CONTROL_STRING, Get_Sample_Rate());
			break;

		default:
			break;
	}

	CDialog::OnVScroll (sbcode, pos, scrollbar);
}


/***********************************************************************************************
 * OptionsDialog::OnLightExportSelective --																	  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/16/00    IML : Created.                                                                * 
 *=============================================================================================*/
void OptionsDialog::OnLightExportSelective() 
{
	LightExportSelective = !LightExportSelective;
	GetDlgItem (IDC_LIGHT_EXCLUSION_STRING)->EnableWindow (LightExportSelective);
}


/***********************************************************************************************
 * OptionsDialog::OnChangeLightExclusionString --															  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/16/00    IML : Created.                                                                * 
 *=============================================================================================*/
void OptionsDialog::OnChangeLightExclusionString() 
{
	GetDlgItem (IDC_LIGHT_EXCLUSION_STRING)->GetWindowText (LightExclusionString);
}


/***********************************************************************************************
 * OptionsDialog::Initialize_Slider --																			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/2/99    IML : Created.                                                                 * 
 *=============================================================================================*/
void OptionsDialog::Initialize_Slider (int sliderid, int start, int end, int value)
{
	CSliderCtrl *slider;

	slider = (CSliderCtrl*) GetDlgItem (sliderid);
	ASSERT (slider != NULL);
	slider->SetRange (start, end);
	slider->SetPos (value);
}	


/***********************************************************************************************
 * OptionsDialog::Initialize_Spinner --																		  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/2/99    IML : Created.                                                                 * 
 *=============================================================================================*/
void OptionsDialog::Initialize_Spinner (int spinnerid, int start, int end, int value)
{
	CSpinButtonCtrl *spinner;
	
	spinner = (CSpinButtonCtrl*) GetDlgItem (spinnerid);
	ASSERT (spinner != NULL);
	spinner->SetRange (start, end);
	spinner->SetPos (value);
}


/***********************************************************************************************
 * OptionsDialog::Set_Text --																						  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/2/99    IML : Created.                                                                 * 
 *=============================================================================================*/
void OptionsDialog::Set_Text (int textid, const char *controlstring, int value)
{
	StringBuilder text (32);

	text.Copy (controlstring, value);
	GetDlgItem (textid)->SetWindowText (text.String());
}


/***********************************************************************************************
 * OptionsDialog::Set_Text --																						  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/2/99    IML : Created.                                                                 * 
 *=============================================================================================*/
void OptionsDialog::Set_Text (int textid, const char *controlstring, float value)
{
	StringBuilder text (32);

	text.Copy (controlstring, value);
	GetDlgItem (textid)->SetWindowText (text.String());
}
