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

// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (C) 1993-1997  Microsoft Corporation.  All Rights Reserved.
//
// Modudule: ICONHDLR.CPP
//
// Purpose:  Implements the IExtractIcon and IPersistFile member
//           functions necessary to support the icon handler portion
//           of this shell extension.
//

#include "Stdafx.h"
#include "priv.h"
#include "shellext.h"

extern HINSTANCE g_DllInstance; // Handle to this DLL itself.

// *********************** IExtractIcon Implementation *************************

STDMETHODIMP CShellExt::GetIconLocation(UINT   /*uFlags*/,
                                        LPSTR  szIconFile,
                                        UINT   cchMax,
                                        int   *piIndex,
                                        UINT  */*pwFlags*/)
{
    GetModuleFileName(g_DllInstance, szIconFile, cchMax);
    *piIndex = (int)GetPrivateProfileInt("IconImage", "Index", 0, m_szFileUserClickedOn);
    return S_OK;
}


STDMETHODIMP CShellExt::Extract(LPCSTR /*pszFile*/,UINT   /*nIconIndex*/,HICON  */*phiconLarge*/,HICON  */*phiconSmall*/,UINT   /*nIconSize*/)
{
    return S_FALSE;
}


// *********************** IPersistFile Implementation ******************

STDMETHODIMP CShellExt::GetClassID(LPCLSID /*lpClassID*/)
{
    return E_FAIL;
}

STDMETHODIMP CShellExt::IsDirty()
{
    return S_FALSE;
}

STDMETHODIMP CShellExt::Load(LPCOLESTR lpszFileName, DWORD /*grfMode*/)
{
    WideCharToMultiByte(CP_ACP,0,lpszFileName,-1,m_szFileUserClickedOn, sizeof(m_szFileUserClickedOn), NULL,NULL);
    return NOERROR;
}

STDMETHODIMP CShellExt::Save(LPCOLESTR /*lpszFileName*/, BOOL /*fRemember*/)
{
    ODS("CShellExt::Save()\r\n");
    return E_FAIL;
}

STDMETHODIMP CShellExt::SaveCompleted(LPCOLESTR /*lpszFileName*/)
{
    ODS("CShellExt::SaveCompleted()\r\n");
    return E_FAIL;
}

STDMETHODIMP CShellExt::GetCurFile(LPOLESTR FAR* /*lplpszFileName*/)
{
    ODS("CShellExt::GetCurFile()\r\n");
    return E_FAIL;
}
