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
 *                 Project Name : ChunkView                                                    *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/ChunkView/ChunkDataView.cpp            $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 9/28/99 9:48a                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


// ChunkDataView.cpp : implementation file
//

#include "stdafx.h"
#include "ChunkView.h"
#include "ChunkDataView.h"
#include "ChunkViewDoc.h"
#include "ChunkFileImage.h"
#include "HexToString.h"
#include <assert.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



/////////////////////////////////////////////////////////////////////////////
// CChunkDataView

IMPLEMENT_DYNCREATE(CChunkDataView, CListView)

CChunkDataView::CChunkDataView() :
	WordSize(WORD_SIZE_BYTE),
	DisplayMode(DISPLAY_MODE_HEX)
{
}

CChunkDataView::~CChunkDataView()
{
}


BEGIN_MESSAGE_MAP(CChunkDataView, CListView)
	//{{AFX_MSG_MAP(CChunkDataView)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChunkDataView drawing

void CChunkDataView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// CChunkDataView diagnostics

#ifdef _DEBUG
void CChunkDataView::AssertValid() const
{
	CListView::AssertValid();
}

void CChunkDataView::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CChunkDataView message handlers

void CChunkDataView::OnInitialUpdate() 
{
	CListView::OnInitialUpdate();
	
	CListCtrl &list = GetListCtrl();
	int flags = list.GetStyle();
	flags |= LVS_REPORT;
	SetWindowLong(list.GetSafeHwnd(), GWL_STYLE, flags);

	//list.SetFont ();
	::SendMessage (list, WM_SETFONT, (WPARAM)GetStockObject (ANSI_FIXED_FONT), 0L);
}

void CChunkDataView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	if (this == pSender) return;

	// Get the document and currently selected chunk
	CListCtrl &list = GetListCtrl();
	CChunkViewDoc * doc= (CChunkViewDoc *)GetDocument();
	const ChunkImageClass * chunk = doc->Get_Cur_Chunk();

	// Reset the list
	list.DeleteAllItems();	

	// Rebuild the list view
	if (chunk != NULL) {
		Display_Chunk(chunk);
	}
}

void CChunkDataView::Display_Chunk(const ChunkImageClass * chunk)
{
	CListCtrl &list = GetListCtrl();
	CChunkViewDoc * doc= (CChunkViewDoc *)GetDocument();
	
	if (chunk->Get_Data() == NULL) {

		Display_Chunk_Sub_Chunks(chunk);

	} else {
		
		switch (DisplayMode) {
			case DISPLAY_MODE_HEX:
				Display_Chunk_Hex(chunk);
				break;

			case DISPLAY_MODE_MICROCHUNKS:
				Display_Chunk_Micro_Chunks(chunk);
				break;
		};
	}
}

void CChunkDataView::Display_Chunk_Sub_Chunks(const ChunkImageClass * chunk)
{
	char _buf[256];
	Reset_Columns();
	CListCtrl &list = GetListCtrl();

	static LV_COLUMN IDColumn = { LVCF_TEXT | LVCF_WIDTH | LVCF_FMT, LVCFMT_LEFT, 160, "Chunk ID", 0,0 };
	static LV_COLUMN LengthColumn = { LVCF_TEXT | LVCF_WIDTH | LVCF_FMT, LVCFMT_LEFT, 160, "Chunk Length", 0,0 };
	list.InsertColumn(0, &IDColumn);
	list.InsertColumn(1, &LengthColumn);

	for (int i=0; i<chunk->Get_Child_Count(); i++) {
		const ChunkImageClass * child = chunk->Get_Child(i);
		
		sprintf(_buf,"0x%08X",child->Get_ID());
		int list_item = list.InsertItem(i, _buf);
		
		sprintf(_buf,"0x%08X",child->Get_Length());
		list.SetItemText(list_item, 1, _buf);
	}
}

void CChunkDataView::Display_Chunk_Hex(const ChunkImageClass * chunk)
{
	Reset_Columns();
	CListCtrl &list = GetListCtrl();

	static LV_COLUMN HexColumn = { LVCF_TEXT | LVCF_WIDTH | LVCF_FMT, LVCFMT_LEFT, 750, "Hex Dump", 0,0 };
	list.InsertColumn(0, &HexColumn);

	HexToStringClass * hexconverter = Create_Hex_Converter(chunk->Get_Data(),chunk->Get_Length());
	assert(hexconverter != NULL);
	
	int rowcounter = 0;
	while (!hexconverter->Is_Done()) {
		list.InsertItem(rowcounter++,hexconverter->Get_Next_Line());
	}

	Destroy_Hex_Converter(hexconverter);
}

void CChunkDataView::Display_Chunk_Micro_Chunks(const ChunkImageClass * chunk)
{
	Reset_Columns();
	CListCtrl &list = GetListCtrl();

	static LV_COLUMN IDColumn = { LVCF_TEXT | LVCF_WIDTH | LVCF_FMT, LVCFMT_LEFT, 48, "ID", 0,0 };
	static LV_COLUMN LengthColumn = { LVCF_TEXT | LVCF_WIDTH | LVCF_FMT, LVCFMT_LEFT, 48, "Size", 0,0 };
	static LV_COLUMN DataColumn = { LVCF_TEXT | LVCF_WIDTH | LVCF_FMT, LVCFMT_LEFT, 750, "Micro Chunk Data", 0,0 };
	list.InsertColumn(0, &IDColumn);
	list.InsertColumn(1, &LengthColumn);
	list.InsertColumn(2, &DataColumn);

	int rowcounter = 0;
	const uint8 * workptr = chunk->Get_Data();
	static char _buf[256];

	while (workptr < chunk->Get_Data() + chunk->Get_Length()) {
		
		uint8 micro_id = *workptr++;
		uint8 micro_size = *workptr++;

		// Add a line for the id
		CString tmp_string;
		tmp_string.Format("%02x",micro_id);
		int list_item = list.InsertItem(rowcounter++, tmp_string);

		// Set the size (second column)
		tmp_string.Format("%02x",micro_size);
		list.SetItemText(list_item, 1, tmp_string);

		// Set the first line of hex data
		HexToStringClass * hexconverter = Create_Hex_Converter(workptr,micro_size);
		tmp_string = hexconverter->Get_Next_Line();
		list.SetItemText(list_item, 2, tmp_string);
		
		while (!hexconverter->Is_Done()) {
			tmp_string = hexconverter->Get_Next_Line();
			list_item = list.InsertItem(rowcounter++, "");
			list.SetItemText(list_item, 2, tmp_string);
		}
		
		workptr += micro_size;
		Destroy_Hex_Converter(hexconverter);
	}
}

void CChunkDataView::Reset_Columns(void)
{
	CListCtrl &list = GetListCtrl();

	BOOL hascolumns = true;
	while (hascolumns) {
		hascolumns = list.DeleteColumn(0);
	}
}

HexToStringClass * CChunkDataView::Create_Hex_Converter(const uint8 * data,const uint32 size)
{
	HexToStringClass * hexconv = NULL;
	
	switch(WordSize) {
		case WORD_SIZE_LONG:
			hexconv = new HexToStringLongClass(data,size);
			break;
		case WORD_SIZE_SHORT:
			hexconv = new HexToStringShortClass(data,size);
			break;
		default:
			hexconv = new HexToStringByteClass(data,size);
			break;
	}
	return hexconv;
}

void CChunkDataView::Destroy_Hex_Converter(HexToStringClass * hexconv)
{
	assert(hexconv != NULL);
	delete hexconv;
}