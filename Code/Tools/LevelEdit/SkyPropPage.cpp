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

// SkyPropPage.cpp : implementation file
//

#include "StdAfx.h"
#include "skyproppage.h"
#include "backgroundmgr.h"
#include "combat.h"
#include "LevelEdit.h"
#include "phys.h"
#include "pscene.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Defines.
#define CLOUD_SLIDER_RESOLUTION 20


/////////////////////////////////////////////////////////////////////////////
// SkyPropPageClass property page

IMPLEMENT_DYNCREATE(SkyPropPageClass, CPropertyPage)

SkyPropPageClass::SkyPropPageClass() : CPropertyPage(SkyPropPageClass::IDD)
{
	//{{AFX_DATA_INIT(SkyPropPageClass)
	//}}AFX_DATA_INIT
}

SkyPropPageClass::~SkyPropPageClass()
{
}

void SkyPropPageClass::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(SkyPropPageClass)
	DDX_Control(pDX, IDC_SKY_CLOUD_GLOOMINESS, CloudGloominessCtrl);
	DDX_Control(pDX, IDC_SKY_CLOUD_COVER, CloudCoverCtrl);
	DDX_Control(pDX, IDC_SKY_TIME_OF_DAY, TimeCtrl);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(SkyPropPageClass, CPropertyPage)
	//{{AFX_MSG_MAP(SkyPropPageClass)
	ON_BN_CLICKED(IDC_SKY_LIGHT_MOON, OnSkyLightMoon)
	ON_BN_CLICKED(IDC_SKY_LIGHT_SUN, OnSkyLightSun)
	ON_BN_CLICKED(IDC_SKY_MOON_FULL, OnSkyMoonFull)
	ON_BN_CLICKED(IDC_SKY_MOON_PARTIAL, OnSkyMoonPartial)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// SkyPropPageClass message handlers

BOOL SkyPropPageClass::OnInitDialog() 
{
	const CTime starttime (2000, 01, 01, 00, 00, 00, 0);
	const CTime	endtime	 (2000, 01, 01, 23, 59, 00, 0);

	unsigned											 hours, minutes;
	_SYSTEMTIME										 systemtime;
	BackgroundMgrClass::LightSourceTypeEnum lightsourcetype;
	SkyClass::MoonTypeEnum						 moontype;
	float												 cloudcover, gloominess;	

	CPropertyPage::OnInitDialog();
	
	BackgroundMgrClass::Get_Time_Of_Day (hours, minutes);

	// Format time display to 24hr clock.
	TimeCtrl.SetFormat ("HH:mm");
	
	TimeCtrl.SetRange (&starttime, &endtime);
	memset (&systemtime, 0, sizeof (systemtime));
	systemtime.wYear	 = 2000;
	systemtime.wMonth	 = 01;
	systemtime.wDay	 = 01;
	systemtime.wHour	 = hours;
	systemtime.wMinute = minutes;
	TimeCtrl.SetTime (&systemtime);

	lightsourcetype = BackgroundMgrClass::Get_Light_Source_Type();
	switch (lightsourcetype) {
		
		case BackgroundMgrClass::LIGHT_SOURCE_TYPE_SUN:
			OnSkyLightSun(); 
			break;

		case BackgroundMgrClass::LIGHT_SOURCE_TYPE_MOON:
			OnSkyLightMoon(); 
			break;

		default:
			ASSERT (false);
			break;
	}
	
	moontype = BackgroundMgrClass::Get_Moon_Type();
	switch (moontype) {
		
		case SkyClass::MOON_TYPE_FULL:
			OnSkyMoonFull();
			break;

		case SkyClass::MOON_TYPE_PART:
			OnSkyMoonPartial();
			break;

		default:
			ASSERT (false);
			break;
	}

	BackgroundMgrClass::Get_Clouds (cloudcover, gloominess);
	CloudCoverCtrl.SetRange (0, CLOUD_SLIDER_RESOLUTION);
	CloudCoverCtrl.SetPos (WWMath::Float_To_Long (cloudcover * CLOUD_SLIDER_RESOLUTION));
	CloudGloominessCtrl.SetRange (0, CLOUD_SLIDER_RESOLUTION);
	CloudGloominessCtrl.SetPos (WWMath::Float_To_Long (gloominess * CLOUD_SLIDER_RESOLUTION));
	
	return true;  // return true unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return false
}

void SkyPropPageClass::OnSkyLightSun() 
{
	CheckDlgButton (IDC_SKY_LIGHT_SUN, 1);
	CheckDlgButton (IDC_SKY_LIGHT_MOON, 0);
}

void SkyPropPageClass::OnSkyLightMoon() 
{
	CheckDlgButton (IDC_SKY_LIGHT_SUN, 0);
	CheckDlgButton (IDC_SKY_LIGHT_MOON, 1);
}

void SkyPropPageClass::OnSkyMoonFull() 
{
	CheckDlgButton (IDC_SKY_MOON_FULL,	  1);
	CheckDlgButton (IDC_SKY_MOON_PARTIAL, 0);
}

void SkyPropPageClass::OnSkyMoonPartial() 
{
	CheckDlgButton (IDC_SKY_MOON_FULL,	  0);
	CheckDlgButton (IDC_SKY_MOON_PARTIAL, 1);
}

void SkyPropPageClass::OnOK() 
{
	const float oocloudsliderresolution = 1.0f / CLOUD_SLIDER_RESOLUTION;

	_SYSTEMTIME	systemtime;
	float			cloudcover, gloominess;

	TimeCtrl.GetTime (&systemtime);
	BackgroundMgrClass::Set_Time_Of_Day (systemtime.wHour, systemtime.wMinute);

	if (IsDlgButtonChecked (IDC_SKY_LIGHT_SUN) == 1) {
		BackgroundMgrClass::Set_Light_Source_Type (BackgroundMgrClass::LIGHT_SOURCE_TYPE_SUN);
	} else {
		BackgroundMgrClass::Set_Light_Source_Type (BackgroundMgrClass::LIGHT_SOURCE_TYPE_MOON);
	}
	if (IsDlgButtonChecked (IDC_SKY_MOON_FULL) == 1) {
		BackgroundMgrClass::Set_Moon_Type (SkyClass::MOON_TYPE_FULL);
	} else {
		BackgroundMgrClass::Set_Moon_Type (SkyClass::MOON_TYPE_PART);
	}
	
	cloudcover = CloudCoverCtrl.GetPos() * oocloudsliderresolution;
	gloominess = CloudGloominessCtrl.GetPos() * oocloudsliderresolution;
	BackgroundMgrClass::Set_Clouds (cloudcover, gloominess);
	
	CPropertyPage::OnOK();
}

