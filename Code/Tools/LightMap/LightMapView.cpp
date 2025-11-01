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
 *                     $Modtime:: 7/17/01 3:17p       $* 
 *                                                                                             * 
 *                    $Revision:: 27                                                        $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

// Includes.
#include "StdAfx.h"
#include "LightMap.h"
#include "InsertSolveDialog.h"
#include "LightMapDoc.h"
#include "LightMapView.h"
#include "OptionsDialog.h"
#include "PackingDialog.h"
#include "SelectionDialog.h"
#include "StringBuilder.h"


// The following is maintained by MFC tools.
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(LightMapView, CListView)

BEGIN_MESSAGE_MAP(LightMapView, CListView)
	//{{AFX_MSG_MAP(LightMapView)
	ON_WM_CREATE()
	ON_COMMAND(ID_INSERT_SOLVE, OnInsertSolve)
	ON_UPDATE_COMMAND_UI(ID_INSERT_SOLVE, OnUpdateInsertSolve)
	ON_COMMAND(ID_TOOLS_OPTIONS, OnToolsOptions)
	ON_COMMAND(ID_TOOLS_PACKING, OnToolsPacking)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_PACKING, OnUpdateToolsPacking)
	ON_WM_LBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


class MeshDialog : public CDialog
{
	public:
		MeshDialog (UINT nIDTemplate, CWnd* pParentWnd, const char *meshname, const char *anomalies)
			: CDialog (nIDTemplate, pParentWnd)
		{
			MeshName  = meshname;
			Anomalies = anomalies;
		}

	protected:
		virtual BOOL OnInitDialog();

	private:
		const char *MeshName;
		const char *Anomalies;
};


// Static data.
LightMapDoc *LightMapView::_Document = NULL;


/***********************************************************************************************
 * LightMapView::LightMapView --																					  *
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
LightMapView::LightMapView()
{
	MeshIndexTable = NULL;	
}


/***********************************************************************************************
 * LightMapView::~LightMapView --																				  *
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
LightMapView::~LightMapView()
{
	if (MeshIndexTable != NULL) delete [] MeshIndexTable;
}


/***********************************************************************************************
 * LightMapView::OnInsertSolve --																				  *
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
void LightMapView::OnInsertSolve()
{
	// NOTE: Specify initial path to be that of current open document.
	InsertSolveDialog insertsolvedialog (GetDocument()->GetPathName());

	if (insertsolvedialog.DoModal() == IDOK) {
		
		char *inclusionstring;
		
		if (!insertsolvedialog.Apply_Selective()) {
			inclusionstring = NULL;
		} else {
			inclusionstring = insertsolvedialog.Inclusion_String();
		}
		GetDocument()->Insert_Solve (insertsolvedialog.Directory_Name(), insertsolvedialog.Filename_List(), inclusionstring, insertsolvedialog.Invert_Selection(), insertsolvedialog.Blend_Noise());
	}
}


/***********************************************************************************************
 * LightMapView::OnUpdateInsertSolve --																		  *
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
void LightMapView::OnUpdateInsertSolve (CCmdUI *cmdui) 
{
	cmdui->Enable (GetDocument()->Can_Insert_Solve());
}


/***********************************************************************************************
 * LightMapView::OnCreate --																						  *
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
int LightMapView::OnCreate (LPCREATESTRUCT lpCreateStruct) 
{
  	static LV_COLUMN _meshname			= {LVCF_TEXT | LVCF_FMT, LVCFMT_LEFT, 0, "Mesh Name", 0, 0};
	static LV_COLUMN _meshanomalies	= {LVCF_TEXT | LVCF_FMT, LVCFMT_LEFT, 0, "Mesh Anomalies", 0, 0};
	static LV_COLUMN _solveanomalies	= {LVCF_TEXT | LVCF_FMT, LVCFMT_LEFT, 0, "Solve Anomalies", 0, 0};
	static LV_COLUMN _vertexsolve	 	= {LVCF_TEXT | LVCF_FMT, LVCFMT_LEFT, 0, "Vertex Solve", 0, 0};
	static LV_COLUMN _lightmapsolve	= {LVCF_TEXT | LVCF_FMT, LVCFMT_LEFT, 0, "Lightmap Solve", 0, 0};

	CListCtrl &list = GetListCtrl();
	long		 flags = list.GetStyle();

	if (CListView::OnCreate(lpCreateStruct) == -1) return (-1);

	// Enable report style for the list view.
	flags |= LVS_REPORT | LVS_NOSORTHEADER;
	SetWindowLong (list.GetSafeHwnd(), GWL_STYLE, flags);
	list.SetExtendedStyle (LVS_EX_ONECLICKACTIVATE | LVS_EX_UNDERLINEHOT | LVS_EX_GRIDLINES); 
	
	list.InsertColumn (0, &_meshname);
	list.InsertColumn (1, &_meshanomalies);
	list.InsertColumn (2, &_solveanomalies);
	list.InsertColumn (3, &_vertexsolve);
	list.InsertColumn (4, &_lightmapsolve);

	return (0);
}


/***********************************************************************************************
 * LightMapView::OnUpdate --																						  *
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
void LightMapView::OnUpdate (CView* pSender, LPARAM lHint, CObject* pHint)
{
	static float	  _widthratio [] = {0.18f, 0.25f, 0.25f, 0.16f, 0.16f};
	static LV_COLUMN _column		  = {LVCF_WIDTH, 0, 0, 0, 0, 0};

	unsigned			meshindex;
	RECT				rect;
	float				w;
	CListCtrl	  &list = GetListCtrl();
	LightMapDoc   *document;
	LVITEM		   item;
	StringBuilder  text (256);

	document	= GetDocument();

	// If there are meshes to display...
	if (document->Mesh_Count() > 0) {

		// Create a mesh index table that will place the mesh indices in alphabetical order of mesh name.
		if (MeshIndexTable != NULL) delete [] MeshIndexTable;
		MeshIndexTable = new unsigned [document->Mesh_Count()];
		ASSERT (MeshIndexTable != NULL);
		for (meshindex = 0; meshindex < document->Mesh_Count(); meshindex++) {
			MeshIndexTable [meshindex] = meshindex;
		}
		_Document = document;
		qsort (MeshIndexTable, document->Mesh_Count(), sizeof (unsigned), Compare_Names);
		_Document = NULL;

		list.DeleteAllItems();

		item.mask	  = LVIF_TEXT;
		item.iSubItem = 0;
		for (meshindex = 0; meshindex < document->Mesh_Count(); meshindex++) {

			unsigned remappedmeshindex;

			item.iItem			= meshindex;
			remappedmeshindex = MeshIndexTable [meshindex];

			item.pszText  = (char*) document->Mesh_Name (remappedmeshindex);

			list.InsertItem (&item);
			list.SetItemText (meshindex, 1, document->Mesh_Anomalies_String (remappedmeshindex, false, text));
			list.SetItemText (meshindex, 2, document->Solve_Anomalies_String (remappedmeshindex, false, text));
			list.SetItemText (meshindex, 3, document->Vertex_Solve_Status_String (remappedmeshindex, text));
			list.SetItemText (meshindex, 4, document->Lightmap_Solve_Status_String (remappedmeshindex, text));
		}
	}

	// Set the column widths.
	GetClientRect (&rect);
	w = (float) rect.right;
	for (unsigned c = 0; c < sizeof (_widthratio) / sizeof (float); c++) {
		
		float columnwidth;

		columnwidth = w * _widthratio [c];
		_column.cx  = columnwidth;
		if (columnwidth - _column.cx > 0.5f) _column.cx++;
		list.SetColumn (c, &_column);
	}
}


/***********************************************************************************************
 * LightMapView::OnToolsOptions --																				  *
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
void LightMapView::OnToolsOptions() 
{
	OptionsDialog options;

	options.DoModal();
}


/***********************************************************************************************
 * LightMapView::OnToolsPacking --																				  *
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
void LightMapView::OnToolsPacking() 
{
	PackingDialog packing;

	packing.DoModal();
}


/***********************************************************************************************
 * LightMapView::OnToolsPacking --																				  *
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
void LightMapView::OnUpdateToolsPacking (CCmdUI *cmdui) 
{
	cmdui->Enable (GetDocument()->Solve_Inserted());
}


/***********************************************************************************************
 * LightMapView::OnLButtonDown --																				  *
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
void LightMapView::OnLButtonDown (UINT flags, CPoint point)
{
	LVHITTESTINFO hittest;

	// Call default handler.
	CListView::OnLButtonDown (flags, point);

	if (MeshIndexTable != NULL) {

		hittest.pt = point;
		GetListCtrl().SubItemHitTest (&hittest);
		if ((hittest.iItem >= 0) && (hittest.iSubItem == 0)) {

			unsigned			meshindex = MeshIndexTable [hittest.iItem];
			LightMapDoc	  *document = GetDocument();
			StringBuilder  string (16384), substring (8192);

			string.Copy (document->Mesh_Anomalies_String (meshindex, true, substring));
			string.Concatenate (document->Solve_Anomalies_String (meshindex, true, substring));

			MeshDialog mesh (IDD_MESH_STATUS, this, document->Mesh_Name (meshindex), string.String());

			mesh.DoModal();
		}
	}
}


/***********************************************************************************************
 * LightMapView::CompareNames --																					  *
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
int LightMapView::Compare_Names (const void *index0, const void *index1)
{
	ASSERT (_Document != NULL);
	return (strcmp (_Document->Mesh_Name (*((unsigned*) index0)), _Document->Mesh_Name (*((unsigned*) index1))));
}		


/***********************************************************************************************
 * MeshDialog::OnInitDialog --																					  *
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
BOOL MeshDialog::OnInitDialog()
{
	StringBuilder string (256);

	// Allow the base class to process this message.
	CDialog::OnInitDialog();

	string.Copy (MeshName);
	string.Concatenate (" - Anomalies");
	SetWindowText (string.String());
	GetDlgItem (IDC_MESH_STATUS_TEXT)->SetWindowText (Anomalies);
	return (true);
}


// The following is maintained by MFC tools.
BOOL LightMapView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	// the CREATESTRUCT cs
	return CListView::PreCreateWindow(cs);
}


#ifdef _DEBUG
void LightMapView::AssertValid() const
{
	CListView::AssertValid();
}

void LightMapView::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}

LightMapDoc* LightMapView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(LightMapDoc)));
	return (LightMapDoc*)m_pDocument;
}
#endif //_DEBUG
