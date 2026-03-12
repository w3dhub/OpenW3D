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
 *                     $Archive:: /Commando/Code/Tools/ChunkView/ChunkViewDoc.h               $*
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


// ChunkViewDoc.h : interface of the CChunkViewDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_CHUNKVIEWDOC_H__53C69A6A_72AB_11D3_BB4D_00902742EA14__INCLUDED_)
#define AFX_CHUNKVIEWDOC_H__53C69A6A_72AB_11D3_BB4D_00902742EA14__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ChunkFileImage.h"


class CChunkViewDoc : public CDocument
{
protected: // create from serialization only
	CChunkViewDoc();
	DECLARE_DYNCREATE(CChunkViewDoc)

// Attributes
public:

// Operations
public:

	const ChunkFileImageClass &	Get_File_Image(void);
	const ChunkImageClass *			Get_Cur_Chunk(void);
	void									Set_Cur_Chunk(ChunkImageClass * cur_chunk);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChunkViewDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CChunkViewDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

	ChunkFileImageClass		m_ChunkFileImage;
	ChunkImageClass *			m_pCurChunk;

// Generated message map functions
protected:
	//{{AFX_MSG(CChunkViewDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHUNKVIEWDOC_H__53C69A6A_72AB_11D3_BB4D_00902742EA14__INCLUDED_)
