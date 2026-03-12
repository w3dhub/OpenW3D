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

#if !defined(AFX_VISWINDOWDIALOG_H__AB6CCD3A_88BB_4D66_AA1C_9B6E69C99409__INCLUDED_)
#define AFX_VISWINDOWDIALOG_H__AB6CCD3A_88BB_4D66_AA1C_9B6E69C99409__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"


/////////////////////////////////////////////////////////////////////////////
//	Forward declarations
/////////////////////////////////////////////////////////////////////////////
class VisRasterizerClass;


/////////////////////////////////////////////////////////////////////////////
//
// VisWindowDialogClass
//
/////////////////////////////////////////////////////////////////////////////
class VisWindowDialogClass : public CDialog
{
// Construction
public:
	VisWindowDialogClass (CWnd* pParent = NULL);
	~VisWindowDialogClass (void);

// Dialog Data
	//{{AFX_DATA(VisWindowDialogClass)
	enum { IDD = IDD_VIS_WINDOW };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(VisWindowDialogClass)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(VisWindowDialogClass)
	afx_msg void OnPaint();
	virtual BOOL OnInitDialog();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	///////////////////////////////////////////////////////////////////
	//	Public methods
	///////////////////////////////////////////////////////////////////
	void		Create (void);
	void		Update_Display (VisRasterizerClass &rasterizer);
	BOOL		OnToolTipNeedText(UINT id, NMHDR * pNMHDR, LRESULT * pResult);
	BOOL		PreTranslateMessage(MSG* pMsg);


public:

	///////////////////////////////////////////////////////////////////
	//	Public methods
	///////////////////////////////////////////////////////////////////
	void		Free_DIB_Section (void);
	void		Create_DIB_Section (int width, int height);
	void		Paint_Display (void);

private:

	///////////////////////////////////////////////////////////////////
	// Private methods
	///////////////////////////////////////////////////////////////////
	int				Hit_Test(CPoint point) const;
	unsigned int	Id_To_Pixel(unsigned int id) const;
	unsigned int	Pixel_To_Id(unsigned int pixel) const;

	///////////////////////////////////////////////////////////////////
	//	Private member data
	///////////////////////////////////////////////////////////////////
	HDC				MemDC;
	HBITMAP			Bitmap;
	BYTE *			BitmapBits;
	int				Width;
	int				Height;
	unsigned			CurToolTipVisId;
	CToolTipCtrl	ToolTip;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VISWINDOWDIALOG_H__AB6CCD3A_88BB_4D66_AA1C_9B6E69C99409__INCLUDED_)
