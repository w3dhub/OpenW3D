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
 *                     $Archive:: /Commando/Code/Tools/ChunkView/ChunkViewDoc.cpp             $*
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


// ChunkViewDoc.cpp : implementation of the CChunkViewDoc class
//

#include "stdafx.h"
#include "ChunkView.h"

#include "ChunkViewDoc.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChunkViewDoc

IMPLEMENT_DYNCREATE(CChunkViewDoc, CDocument)

BEGIN_MESSAGE_MAP(CChunkViewDoc, CDocument)
	//{{AFX_MSG_MAP(CChunkViewDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChunkViewDoc construction/destruction

CChunkViewDoc::CChunkViewDoc() :
	m_pCurChunk(NULL)
{
	// TODO: add one-time construction code here

}

CChunkViewDoc::~CChunkViewDoc()
{
}

BOOL CChunkViewDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return false;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return true;
}



/////////////////////////////////////////////////////////////////////////////
// CChunkViewDoc serialization

void CChunkViewDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
	}
	else
	{
		m_ChunkFileImage.Load(ar.m_strFileName);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CChunkViewDoc diagnostics

#ifdef _DEBUG
void CChunkViewDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CChunkViewDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CChunkViewDoc commands

const ChunkFileImageClass & CChunkViewDoc::Get_File_Image(void)
{
	return m_ChunkFileImage;
}

void CChunkViewDoc::Set_Cur_Chunk(ChunkImageClass * cur_chunk)
{
	// The ChunkTreeView calls this when the user clicks on a particular chunk
	if (m_pCurChunk != cur_chunk) {
		m_pCurChunk = cur_chunk;
	}
}

const ChunkImageClass *	CChunkViewDoc::Get_Cur_Chunk(void)
{
	return m_pCurChunk;
}
