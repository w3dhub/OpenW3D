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
 *                     $Archive:: /Commando/Code/Tools/ChunkView/ChunkDataView.h              $*
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


#if !defined(AFX_CHUNKDATAVIEW_H__FD83E2D7_72AE_11D3_BB4D_00902742EA14__INCLUDED_)
#define AFX_CHUNKDATAVIEW_H__FD83E2D7_72AE_11D3_BB4D_00902742EA14__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ChunkDataView.h : header file
//

#include "bittype.h"

class ChunkImageClass;
class HexToStringClass;


/////////////////////////////////////////////////////////////////////////////
// CChunkDataView view

class CChunkDataView : public CListView
{
protected:
	CChunkDataView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CChunkDataView)

// Attributes
public:

	enum WordSizeType
	{
		WORD_SIZE_BYTE		= 0,
		WORD_SIZE_SHORT	= 1,
		WORD_SIZE_LONG		= 2
	};

	enum DisplayModeType
	{
		DISPLAY_MODE_HEX	= 0,
		DISPLAY_MODE_MICROCHUNKS,
	};

	void					Set_Word_Size(WordSizeType wordsize)				{ WordSize = wordsize; OnUpdate(nullptr,0,nullptr); }
	WordSizeType		Get_Word_Size(void)										{ return WordSize; }
	void					Set_Display_Mode(DisplayModeType displaymode)	{ DisplayMode = displaymode; OnUpdate(nullptr,0,nullptr); }
	DisplayModeType	Get_Display_Mode(void)									{ return DisplayMode; }

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChunkDataView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CChunkDataView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	void						Display_Chunk(const ChunkImageClass * chunk);
	void						Display_Chunk_Sub_Chunks(const ChunkImageClass * chunk);
	void						Display_Chunk_Hex(const ChunkImageClass * chunk);
	void						Display_Chunk_Micro_Chunks(const ChunkImageClass * chunk);
	void						Reset_Columns(void);
	HexToStringClass *	Create_Hex_Converter(const uint8 * data,const uint32 size);
	void						Destroy_Hex_Converter(HexToStringClass * hexconv);

	DisplayModeType		DisplayMode;
	WordSizeType			WordSize;

	// Generated message map functions
protected:
	//{{AFX_MSG(CChunkDataView)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHUNKDATAVIEW_H__FD83E2D7_72AE_11D3_BB4D_00902742EA14__INCLUDED_)
