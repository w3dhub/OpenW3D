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
 *                     $Modtime:: 7/20/00 4:47p       $* 
 *                                                                                             * 
 *                    $Revision:: 3                                                         $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

// Includes.
#include "StdAfx.h"
#include "LightMap.h"
#include "PackingDialog.h"

// The following is maintained by MFC tools.
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/***********************************************************************************************
 * PackingDialog::OnInitDialog --																				  *
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
BOOL PackingDialog::OnInitDialog() 
{
  	static LV_COLUMN _column [2] = {
		{LVCF_FMT | LVCF_TEXT | LVCF_WIDTH, LVCFMT_LEFT,  0, "Statistic", 0, 0},
		{LVCF_FMT | LVCF_TEXT | LVCF_WIDTH, LVCFMT_RIGHT, 0, "Value", 0, 0}
	};

	static float _widthratio [2] = {0.55f, 0.45f};

	static char *_statisticlabels [LightmapPacker::STATISTICS_COUNT] = {
		"Page format",                          
		"No. of lightmaps processed",           
		"Percentage adjacent faces blended",		// No. adjacent faces edge blended / No. adjacent faces. NOTE: Some adjacent
																// faces cannot be edge blended because they do not have the same UV mapping
																// as the principal face.
		"Edge blend efficiency",						// Percentage texel area not dedicated to edge blending. 
		"Scaling efficiency",							// Percentage texel area reduction due to low-pass filter scaling.
		"No. of pages created",                 
		"Packing efficiency",							// Percentage texel area of page used (not padded). 
	   "Replica lightmap culling efficiency",		// Percentage texel area saved due to lightmap reuse.
		"Estimated texture swapping efficiency",	// Estimated likelihood that next face will not incur a texture swap.
		"No. of oversize lightmaps"					// No. of lightmaps that exceeded the page size limit and had to be scaled.
	};

	CListCtrl *list;
	LVITEM	  item;
	RECT		  rect;
	float		  w;

	CDialog::OnInitDialog();

	list = (CListCtrl*) GetDlgItem (IDC_PACKING_LIST);
	list->GetClientRect (&rect);
	w = (float) rect.right;
	for (unsigned c = 0; c < sizeof (_widthratio) / sizeof (float); c++) {
		
		float columnwidth;

		columnwidth = w * _widthratio [c];
		_column [c].cx  = columnwidth;
		if (columnwidth - _column [c].cx > 0.5f) _column [c].cx++;
		list->InsertColumn (c, &_column [c]);
	}

	item.mask	  = LVIF_TEXT;
	item.iSubItem = 0;
	for (unsigned i = 0; i < LightmapPacker::STATISTICS_COUNT; i++) {
		item.iItem	  = i;
		item.pszText  = _statisticlabels [i];
		list->InsertItem (&item);
		list->SetItemText (i, 1, LightmapPacker::Get_Statistic (i));
	}

	return (TRUE);
}


// The following is maintained by MFC tools.
PackingDialog::PackingDialog(CWnd* pParent /*=NULL*/)
	: CDialog(PackingDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(PackingDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void PackingDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(PackingDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(PackingDialog, CDialog)
	//{{AFX_MSG_MAP(PackingDialog)
	ON_BN_CLICKED(IDCLOSE, OnClose)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


INT_PTR PackingDialog::DoModal()
{
	// TODO: Add your specialized code here and/or call the base class
	
	return CDialog::DoModal();
}


