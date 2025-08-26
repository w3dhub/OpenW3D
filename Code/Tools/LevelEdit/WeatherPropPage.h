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

#if !defined(AFX_WEATHERPROPPAGE_H__BF0B86B8_3079_469D_A4B6_854192EF922C__INCLUDED_)
#define AFX_WEATHERPROPPAGE_H__BF0B86B8_3079_469D_A4B6_854192EF922C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// WeatherPropPage.h : header file
//

#include "resource.h"
#include "vector3.h"

/////////////////////////////////////////////////////////////////////////////
// WeatherPropPageClass dialog

class WeatherPropPageClass : public CPropertyPage
{
	DECLARE_DYNCREATE(WeatherPropPageClass)

// Construction
public:
	WeatherPropPageClass();
	~WeatherPropPageClass();

// Dialog Data
	//{{AFX_DATA(WeatherPropPageClass)
	enum { IDD = IDD_BACKGROUND_WEATHER };
	CSliderCtrl	WindVariabilitySlider;
	CSliderCtrl	WindHeadingSlider;
	CSpinButtonCtrl	WindSpeedSpin;
	CSpinButtonCtrl	PrecipitationDensitySpin;
	CSliderCtrl	LightningDistributionSlider;
	CSliderCtrl	LightningEndDistanceSlider;
	CSliderCtrl	LightningHeadingSlider;
	CSliderCtrl	LightningStartDistanceSlider;
	CSliderCtrl	LightningIntensitySlider;
		CSpinButtonCtrl FogStartSpin;
		CSpinButtonCtrl FogEndSpin;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(WeatherPropPageClass)
	public:
	virtual void OnOK();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(WeatherPropPageClass)
	virtual BOOL OnInitDialog();
	afx_msg void OnFogCheck();
	afx_msg void OnFogColor();
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnPrecipitationNone();
	afx_msg void OnPrecipitationRain();
	afx_msg void OnPrecipitationSnow();
	afx_msg void OnPrecipitationAsh();
	afx_msg void OnLightningNone();
	afx_msg void OnLightningLightning();
	afx_msg void OnLightningWarBlitz();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	
	void Enable_Lightning_Controls (bool enable);
	void Update_Lightning_Values();
	void Update_Wind_Values();

	Vector3 FogColor;
};


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WEATHERPROPPAGE_H__BF0B86B8_3079_469D_A4B6_854192EF922C__INCLUDED_)
