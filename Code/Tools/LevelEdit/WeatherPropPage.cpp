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

// WeatherPropPage.cpp : implementation file
//

#include "StdAfx.h"
#include "weatherproppage.h"
#include "backgroundmgr.h"
#include "leveledit.h"
#include "sceneeditor.h"
#include "phys.h"
#include "utils.h"
#include "weathermgr.h"
#include "colorpickerdialogclass.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Defines.
#define SLIDER_RESOLUTION	100
#define SPINNER_RESOLUTION	100

/////////////////////////////////////////////////////////////////////////////
// WeatherPropPageClass property page

IMPLEMENT_DYNCREATE(WeatherPropPageClass, CPropertyPage)

WeatherPropPageClass::WeatherPropPageClass() : CPropertyPage(WeatherPropPageClass::IDD)
{
	//{{AFX_DATA_INIT(WeatherPropPageClass)
	//}}AFX_DATA_INIT
}

WeatherPropPageClass::~WeatherPropPageClass()
{
}

void WeatherPropPageClass::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(WeatherPropPageClass)
	DDX_Control(pDX, IDC_WIND_VARIABILITY, WindVariabilitySlider);
	DDX_Control(pDX, IDC_WIND_HEADING, WindHeadingSlider);
	DDX_Control(pDX, IDC_WIND_SPEED_SPIN, WindSpeedSpin);
	DDX_Control(pDX, IDC_PRECIPITATION_DENSITY_SPIN, PrecipitationDensitySpin);
	DDX_Control(pDX, IDC_LIGHTNING_DISTRIBUTION, LightningDistributionSlider);
	DDX_Control(pDX, IDC_LIGHTNING_END_DISTANCE, LightningEndDistanceSlider);
	DDX_Control(pDX, IDC_LIGHTNING_HEADING, LightningHeadingSlider);
	DDX_Control(pDX, IDC_LIGHTNING_START_DISTANCE, LightningStartDistanceSlider);
	DDX_Control(pDX, IDC_LIGHTNING_INTENSITY, LightningIntensitySlider);
		DDX_Control(pDX, IDC_FOG_START_SPIN, FogStartSpin);
		DDX_Control(pDX, IDC_FOG_END_SPIN, FogEndSpin);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(WeatherPropPageClass, CPropertyPage)
	//{{AFX_MSG_MAP(WeatherPropPageClass)
	ON_BN_CLICKED(IDC_FOG_CHECK, OnFogCheck)
	ON_BN_CLICKED(IDC_FOG_COLOR, OnFogColor)
	ON_WM_DRAWITEM()
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_PRECIPITATION_NONE, OnPrecipitationNone)
	ON_BN_CLICKED(IDC_PRECIPITATION_RAIN, OnPrecipitationRain)
	ON_BN_CLICKED(IDC_PRECIPITATION_SNOW, OnPrecipitationSnow)
	ON_BN_CLICKED(IDC_PRECIPITATION_ASH, OnPrecipitationAsh)
	ON_BN_CLICKED(IDC_LIGHTNING_NONE, OnLightningNone)
	ON_BN_CLICKED(IDC_LIGHTNING_LIGHTNING, OnLightningLightning)
	ON_BN_CLICKED(IDC_LIGHTNING_WAR_BLITZ, OnLightningWarBlitz)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// WeatherPropPageClass message handlers

BOOL WeatherPropPageClass::OnInitDialog() 
{
	const int fogstartmax				 = 1000;
	const int fogendmax					 = 5000;
	const int precipitationdensitymax = 100;	
	const int windspeedmax				 = 100;

	float	fogstart, fogend;
	float	lightningintensity, lightningstartdistance, lightningenddistance, lightningheading, lightningdistribution;
	float	windheading, windspeed, windvariability;
	float	precipitationdensity;

	CPropertyPage::OnInitDialog();

	Make_Edit_Float_Ctrl (PrecipitationDensitySpin.GetBuddy()->m_hWnd);
	Make_Edit_Float_Ctrl (WindSpeedSpin.GetBuddy()->m_hWnd);

	//	Load the fog settings into the dialog controls.
//	CheckDlgButton (IDC_FOG_CHECK, ::Get_Scene_Editor()->Get_Fog_Enable());
	CheckDlgButton (IDC_FOG_CHECK, WeatherMgrClass::Get_Fog_Enable());
	OnFogCheck();
	FogColor	= ::Get_Scene_Editor()->Get_Fog_Color();
//	::Get_Scene_Editor ()->Get_Fog_Range (&fogstart, &fogend);
	WeatherMgrClass::Get_Fog_Range (fogstart, fogend);
	FogStartSpin.SetRange (0, fogstartmax);
	FogEndSpin.SetRange (0, fogendmax);	
	FogStartSpin.SetPos (fogstart);
	FogEndSpin.SetPos (fogend);

	// Load the lightning/war-blitz settings into the dialog controls.
	BackgroundMgrClass::Get_Lightning (lightningintensity, lightningstartdistance, lightningenddistance, lightningheading, lightningdistribution);
	if (lightningintensity > 0.0f) {
		OnLightningLightning();
	} else {
		BackgroundMgrClass::Get_War_Blitz (lightningintensity, lightningstartdistance, lightningenddistance, lightningheading, lightningdistribution);
		if (lightningintensity > 0.0f) {
			OnLightningWarBlitz();
		} else {
			OnLightningNone();
		}
	}
	LightningIntensitySlider.SetRange (0, SLIDER_RESOLUTION);
	LightningIntensitySlider.SetPos (WWMath::Float_To_Long (lightningintensity * SLIDER_RESOLUTION));
	LightningStartDistanceSlider.SetRange (0, SLIDER_RESOLUTION);
	LightningStartDistanceSlider.SetPos (WWMath::Float_To_Long (lightningstartdistance * SLIDER_RESOLUTION)); 
	LightningEndDistanceSlider.SetRange (0, SLIDER_RESOLUTION);
	LightningEndDistanceSlider.SetPos (WWMath::Float_To_Long (lightningenddistance * SLIDER_RESOLUTION)); 
	LightningHeadingSlider.SetRange (0, 359);
	LightningHeadingSlider.SetPos (WWMath::Float_To_Long (lightningheading)); 
	LightningDistributionSlider.SetRange (0, SLIDER_RESOLUTION);
	LightningDistributionSlider.SetPos (WWMath::Float_To_Long (lightningdistribution * SLIDER_RESOLUTION));
	Update_Lightning_Values();

	// Load the precipitation settings into the dialog controls.
	WeatherMgrClass::Get_Precipitation (WeatherMgrClass::PRECIPITATION_RAIN, precipitationdensity);
	if (precipitationdensity > 0.0f) {
		OnPrecipitationRain();
	} else {
		WeatherMgrClass::Get_Precipitation (WeatherMgrClass::PRECIPITATION_SNOW, precipitationdensity);
		if (precipitationdensity > 0.0f) {
			OnPrecipitationSnow();
		} else {
			WeatherMgrClass::Get_Precipitation (WeatherMgrClass::PRECIPITATION_ASH, precipitationdensity);
			if (precipitationdensity > 0.0f) {
				OnPrecipitationAsh();
			} else {
				OnPrecipitationNone();
			}
		}
	}
	PrecipitationDensitySpin.SetRange (0, precipitationdensitymax * SPINNER_RESOLUTION);
	SetWindowFloat (PrecipitationDensitySpin.GetBuddy()->m_hWnd, precipitationdensity);

	// Load the wind settings into the dialog controls.
	WeatherMgrClass::Get_Wind (windheading, windspeed, windvariability);
	WindHeadingSlider.SetRange (0, 359);
	WindHeadingSlider.SetPos (WWMath::Float_To_Long (windheading)); 
	WindSpeedSpin.SetRange (0, windspeedmax * SPINNER_RESOLUTION);
	SetWindowFloat (WindSpeedSpin.GetBuddy()->m_hWnd, windspeed);
	WindVariabilitySlider.SetRange (0, SLIDER_RESOLUTION);
	WindVariabilitySlider.SetPos (WWMath::Float_To_Long (windvariability * SLIDER_RESOLUTION));
	Update_Wind_Values();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void WeatherPropPageClass::OnFogCheck() 
{
	bool enabled = (IsDlgButtonChecked (IDC_FOG_CHECK) == 1);

	// Enable/disable fog controls.
	FogStartSpin.EnableWindow (enabled);
	FogEndSpin.EnableWindow (enabled);
	
	// Fog colors are now under programmatic control. Don't let the user edit this.
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_FOG_COLOR), false);

	::EnableWindow (::GetDlgItem (m_hWnd, IDC_FOG_START_EDIT), enabled);
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_FOG_END_EDIT), enabled);
}

void WeatherPropPageClass::OnFogColor() 
{
	int red, green, blue;

	red	= MIN (UCHAR_MAX, FogColor.X * ((float) (UCHAR_MAX + 1)));
	green = MIN (UCHAR_MAX, FogColor.Y * ((float) (UCHAR_MAX + 1)));
	blue	= MIN (UCHAR_MAX, FogColor.Z * ((float) (UCHAR_MAX + 1)));
	if (::Show_Color_Picker (&red, &green, &blue)) {
		
		const float ooucharmaxplusone = 1.0f / (UCHAR_MAX + 1);

		FogColor.Set ((red + 0.5f) * ooucharmaxplusone, (green + 0.5f) * ooucharmaxplusone, (blue + 0.5f) * ooucharmaxplusone);

		// Repaint the fog color button.
		::InvalidateRect (::GetDlgItem (m_hWnd, IDC_FOG_COLOR), NULL, TRUE);
	}
}


void WeatherPropPageClass::OnDrawItem (int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	Vector3 color;
	UINT	  state = DFCS_BUTTONPUSH | DFCS_ADJUSTRECT;
	CRect	  rect = lpDrawItemStruct->rcItem;
	CDC	  dc;
	int	  r, g, b;	

	color = FogColor;

	// Determine what state to draw the button in (pushed or normal).
	if (lpDrawItemStruct->itemState & ODS_SELECTED) {
		state |= DFCS_PUSHED;
	}

	// Draw the button's outline.
	::DrawFrameControl (lpDrawItemStruct->hDC, rect, DFC_BUTTON, state);

	// Fill the button with the appropriate color.
	dc.Attach (lpDrawItemStruct->hDC);
	
	r = MIN (UCHAR_MAX, FogColor.X * ((float) (UCHAR_MAX + 1)));
	g = MIN (UCHAR_MAX, FogColor.Y * ((float) (UCHAR_MAX + 1)));
	b = MIN (UCHAR_MAX, FogColor.Z * ((float) (UCHAR_MAX + 1)));
	dc.FillSolidRect (&rect, RGB (r, g, b));
	dc.Detach();
	
	// Draw the focus rectangle (if necessary).
	if (lpDrawItemStruct->itemState & ODS_FOCUS) {
		::DrawFocusRect (lpDrawItemStruct->hDC, &rect);
	}
	
	CPropertyPage::OnDrawItem (nIDCtl, lpDrawItemStruct);
}


void WeatherPropPageClass::OnLightningNone() 
{
	CheckDlgButton (IDC_LIGHTNING_NONE, 1);
	CheckDlgButton (IDC_LIGHTNING_LIGHTNING, 0);
	CheckDlgButton (IDC_LIGHTNING_WAR_BLITZ, 0);
	Enable_Lightning_Controls (false);
}


void WeatherPropPageClass::OnLightningLightning() 
{
	CheckDlgButton (IDC_LIGHTNING_NONE, 0);
	CheckDlgButton (IDC_LIGHTNING_LIGHTNING, 1);
	CheckDlgButton (IDC_LIGHTNING_WAR_BLITZ, 0);
	Enable_Lightning_Controls (true);
}


void WeatherPropPageClass::OnLightningWarBlitz() 
{
	CheckDlgButton (IDC_LIGHTNING_NONE, 0);
	CheckDlgButton (IDC_LIGHTNING_LIGHTNING, 0);
	CheckDlgButton (IDC_LIGHTNING_WAR_BLITZ, 1);
	Enable_Lightning_Controls (true);
}


void WeatherPropPageClass::Enable_Lightning_Controls (bool enable)
{
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_LIGHTNING_INTENSITY), enable);
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_LIGHTNING_INTENSITY_VALUE), enable);
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_LIGHTNING_START_DISTANCE), enable);
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_LIGHTNING_START_DISTANCE_VALUE), enable);
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_LIGHTNING_END_DISTANCE), enable);
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_LIGHTNING_END_DISTANCE_VALUE), enable);
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_LIGHTNING_HEADING), enable);
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_LIGHTNING_HEADING_VALUE), enable);
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_LIGHTNING_DISTRIBUTION), enable);
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_LIGHTNING_DISTRIBUTION_VALUE), enable);
}


void WeatherPropPageClass::OnPrecipitationNone() 
{
	CheckDlgButton (IDC_PRECIPITATION_NONE, 1);
	CheckDlgButton (IDC_PRECIPITATION_RAIN, 0);
	CheckDlgButton (IDC_PRECIPITATION_SNOW, 0);
	CheckDlgButton (IDC_PRECIPITATION_ASH, 0);
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_PRECIPITATION_DENSITY_EDIT), false);
}


void WeatherPropPageClass::OnPrecipitationRain() 
{
	CheckDlgButton (IDC_PRECIPITATION_NONE, 0);
	CheckDlgButton (IDC_PRECIPITATION_RAIN, 1);
	CheckDlgButton (IDC_PRECIPITATION_SNOW, 0);
	CheckDlgButton (IDC_PRECIPITATION_ASH, 0);
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_PRECIPITATION_DENSITY_EDIT), true);
}


void WeatherPropPageClass::OnPrecipitationSnow() 
{
	CheckDlgButton (IDC_PRECIPITATION_NONE, 0);
	CheckDlgButton (IDC_PRECIPITATION_RAIN, 0);
	CheckDlgButton (IDC_PRECIPITATION_SNOW, 1);
	CheckDlgButton (IDC_PRECIPITATION_ASH, 0);
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_PRECIPITATION_DENSITY_EDIT), true);
}


void WeatherPropPageClass::OnPrecipitationAsh() 
{
	CheckDlgButton (IDC_PRECIPITATION_NONE, 0);
	CheckDlgButton (IDC_PRECIPITATION_RAIN, 0);
	CheckDlgButton (IDC_PRECIPITATION_SNOW, 0);
	CheckDlgButton (IDC_PRECIPITATION_ASH, 1);
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_PRECIPITATION_DENSITY_EDIT), true);
}


void WeatherPropPageClass::OnHScroll (UINT sbcode, UINT pos, CScrollBar *scrollbar) 
{
	Update_Lightning_Values();
	Update_Wind_Values();

	CPropertyPage::OnHScroll (sbcode, pos, scrollbar);
}


void WeatherPropPageClass::Update_Lightning_Values()
{
	const unsigned buffersize = 10;
	const float		oosliderresolution = 1.0f / SLIDER_RESOLUTION;

	char buffer [buffersize];

	buffer [buffersize - 1] = '\0';

	_snprintf (buffer, buffersize - 1, "%.2f", LightningIntensitySlider.GetPos() * oosliderresolution);
	GetDlgItem (IDC_LIGHTNING_INTENSITY_VALUE)->SetWindowText (buffer);
	_snprintf (buffer, buffersize - 1, "%.2f", LightningStartDistanceSlider.GetPos() * oosliderresolution);
	GetDlgItem (IDC_LIGHTNING_START_DISTANCE_VALUE)->SetWindowText (buffer);
	_snprintf (buffer, buffersize - 1, "%.2f", LightningEndDistanceSlider.GetPos() * oosliderresolution);
	GetDlgItem (IDC_LIGHTNING_END_DISTANCE_VALUE)->SetWindowText (buffer);
	_snprintf (buffer, buffersize - 1, "%.2f", (float) LightningHeadingSlider.GetPos());
	GetDlgItem (IDC_LIGHTNING_HEADING_VALUE)->SetWindowText (buffer);
	_snprintf (buffer, buffersize - 1, "%.2f", LightningDistributionSlider.GetPos() * oosliderresolution);
	GetDlgItem (IDC_LIGHTNING_DISTRIBUTION_VALUE)->SetWindowText (buffer);
}


void WeatherPropPageClass::Update_Wind_Values()
{
	const unsigned buffersize = 10;
	const float		oosliderresolution = 1.0f / SLIDER_RESOLUTION;

	char buffer [buffersize];

	buffer [buffersize - 1] = '\0';

	_snprintf (buffer, buffersize - 1, "%.2f", (float) WindHeadingSlider.GetPos());
	GetDlgItem (IDC_WIND_HEADING_VALUE)->SetWindowText (buffer);
	_snprintf (buffer, buffersize - 1, "%.2f", WindVariabilitySlider.GetPos() * oosliderresolution);
	GetDlgItem (IDC_WIND_VARIABILITY_VALUE)->SetWindowText (buffer);
}


void WeatherPropPageClass::OnOK() 
{
	const float oosliderresolution = 1.0f / SLIDER_RESOLUTION;
	
	float	lightningintensity, lightningstartdistance, lightningenddistance, lightningheading, lightningdistribution;
	float	windheading, windspeed, windvariability;
	float	precipitationdensity;

	// Configure fog system.
//	::Get_Scene_Editor()->Set_Fog_Enable (IsDlgButtonChecked (IDC_FOG_CHECK) == 1);
	WeatherMgrClass::Set_Fog_Enable (IsDlgButtonChecked (IDC_FOG_CHECK) == 1);
	::Get_Scene_Editor()->Set_Fog_Color (FogColor);
//	::Get_Scene_Editor()->Set_Fog_Range (FogStartSpin.GetPos(), FogEndSpin.GetPos());
	WeatherMgrClass::Set_Fog_Range (FogStartSpin.GetPos(), FogEndSpin.GetPos());

	// Configure lightning/war-blitz.
	lightningintensity	  = LightningIntensitySlider.GetPos() * oosliderresolution;
	lightningstartdistance = LightningStartDistanceSlider.GetPos() * oosliderresolution;
	lightningenddistance	  = LightningEndDistanceSlider.GetPos() * oosliderresolution;
	lightningheading		  = (float) LightningHeadingSlider.GetPos();
	lightningdistribution  = LightningDistributionSlider.GetPos() * oosliderresolution;
	if (IsDlgButtonChecked (IDC_LIGHTNING_NONE) == 1) {
		BackgroundMgrClass::Set_Lightning_Intensity (0.0f);
		BackgroundMgrClass::Set_War_Blitz (0.0f);
	} else {
		if (IsDlgButtonChecked (IDC_LIGHTNING_LIGHTNING) == 1) {
			BackgroundMgrClass::Set_Lightning (lightningintensity, lightningstartdistance, lightningenddistance, lightningheading, lightningdistribution); 
			BackgroundMgrClass::Set_War_Blitz (0.0f);
		} else {
			WWASSERT (IsDlgButtonChecked (IDC_LIGHTNING_WAR_BLITZ) == 1);
			BackgroundMgrClass::Set_Lightning_Intensity (0.0f);
			BackgroundMgrClass::Set_War_Blitz (lightningintensity, lightningstartdistance, lightningenddistance, lightningheading, lightningdistribution);
		}
	}

	// Configure wind.
	windheading		 = WindHeadingSlider.GetPos();
	windspeed		 = GetWindowFloat (WindSpeedSpin.GetBuddy()->m_hWnd, true);
	windvariability = WindVariabilitySlider.GetPos() * oosliderresolution;
	WeatherMgrClass::Set_Wind (windheading, windspeed, windvariability);

	// Configure precipitation.
	precipitationdensity = GetWindowFloat (PrecipitationDensitySpin.GetBuddy()->m_hWnd, true);
	if (IsDlgButtonChecked (IDC_PRECIPITATION_NONE) == 1) {
		WeatherMgrClass::Set_Precipitation (WeatherMgrClass::PRECIPITATION_RAIN, 0.0f);
		WeatherMgrClass::Set_Precipitation (WeatherMgrClass::PRECIPITATION_SNOW, 0.0f);
		WeatherMgrClass::Set_Precipitation (WeatherMgrClass::PRECIPITATION_ASH,  0.0f);
	} else {
		if (IsDlgButtonChecked (IDC_PRECIPITATION_RAIN) == 1) {
			WeatherMgrClass::Set_Precipitation (WeatherMgrClass::PRECIPITATION_RAIN, precipitationdensity);
			WeatherMgrClass::Set_Precipitation (WeatherMgrClass::PRECIPITATION_SNOW, 0.0f);
			WeatherMgrClass::Set_Precipitation (WeatherMgrClass::PRECIPITATION_ASH,  0.0f);
		} else {
			if (IsDlgButtonChecked (IDC_PRECIPITATION_SNOW) == 1) {
				WeatherMgrClass::Set_Precipitation (WeatherMgrClass::PRECIPITATION_RAIN, 0.0f);
				WeatherMgrClass::Set_Precipitation (WeatherMgrClass::PRECIPITATION_SNOW, precipitationdensity);
				WeatherMgrClass::Set_Precipitation (WeatherMgrClass::PRECIPITATION_ASH,  0.0f);
			} else {
				WWASSERT (IsDlgButtonChecked (IDC_PRECIPITATION_ASH) == 1);
				WeatherMgrClass::Set_Precipitation (WeatherMgrClass::PRECIPITATION_RAIN, 0.0f);
				WeatherMgrClass::Set_Precipitation (WeatherMgrClass::PRECIPITATION_SNOW, 0.0f);
				WeatherMgrClass::Set_Precipitation (WeatherMgrClass::PRECIPITATION_ASH,  precipitationdensity);
			}
		}
	}

	CPropertyPage::OnOK();
}
