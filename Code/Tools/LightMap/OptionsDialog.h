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
 *                     $Modtime:: 9/08/00 6:44p       $* 
 *                                                                                             * 
 *                    $Revision:: 8                                                         $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if !defined(AFX_OPTIONSDIALOG_H__23794761_9166_11D3_A321_009027447394__INCLUDED_)
#define AFX_OPTIONSDIALOG_H__23794761_9166_11D3_A321_009027447394__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


// Includes.
#include "w3d_file.h"


class OptionsDialog : public CDialog
{
// Construction
public:
	OptionsDialog (CWnd *parent = NULL);

	float			 Get_Spatial_Tolerance()		{return (SpatialTolerance * 0.0001f);}				// x 0.0001 metres.
	unsigned		 Get_Smoothing_Angle()			{return (SmoothingAngle);}
	unsigned		 Get_Filter_Sharpness()			{return (FilterSharpness);}
	W3dRGBStruct Get_Fill_Color()					{return (FillColor);}
	float			 Get_Sample_Rate()				{return (SampleRate / 10.0f);}
	unsigned		 Get_Filter_Error()				{return (FilterError);}
	unsigned		 Get_Bit_Depth()					{return (BitDepth);}
	bool			 Get_Light_Export_Selective()	{return (LightExportSelective);}
	const char	*Get_Light_Exclusion_String() {return (LPCTSTR (LightExclusionString));}

// Dialog Data
	//{{AFX_DATA(OptionsDialog)
	enum { IDD = IDD_OPTIONS };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(OptionsDialog)
	public:
	virtual INT_PTR DoModal();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(OptionsDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnChangeSpatialTolerance();
	afx_msg void OnUpdateSpatialTolerance();
	afx_msg void OnChangeFillColorRed();
   afx_msg void OnChangeFillColorGreen();
	afx_msg void OnChangeFillColorBlue();
	afx_msg void OnUpdateFillColorRed();
	afx_msg void OnUpdateFillColorGreen();
	afx_msg void OnUpdateFillColorBlue();
	afx_msg void On16BitsPerPixel();
	afx_msg void On24BitsPerPixel();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnLightExportSelective();
	afx_msg void OnChangeLightExclusionString();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:

	void Initialize_Slider (int sliderid, int start, int end, int value);
	void Initialize_Spinner (int spinnerid, int start, int end, int value);
	void Set_Text (int textid, const char *controlstring, int value);
	void Set_Text (int textid, const char *controlstring, float value);

	unsigned		  SpatialTolerance;		// Vertex smoothing spatial tolerance.
	unsigned		  SmoothingAngle;			// Vertex smoothing angle (in degrees).
	unsigned		  FilterSharpness;		// Filter sharpness (or tightness).
	unsigned		  BitDepth;					// Lightmap bit depth (pixel format).
	W3dRGBStruct  FillColor;				// Color used to pad unused texels in lightmaps.
	unsigned		  SampleRate;
	unsigned		  FilterError;			  	// Scale lightmaps to maximum filter error.
	bool			  LightExportSelective;
	CString		  LightExclusionString;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPTIONSDIALOG_H__23794761_9166_11D3_A321_009027447394__INCLUDED_)
