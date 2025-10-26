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
//  MODULE:   ctxmenu.cpp
//
//  PURPOSE:   Implements the IContextMenu member functions necessary to support
//             the context menu portioins of this shell extension.  Context menu
//             shell extensions are called when the user right clicks on a file
//             (of the type registered for the shell extension--see SHELLEXT.REG
//             for details on the registry entries.  In this sample, the relevant
//             files are of type .W3D) in the Explorer, or selects the File menu 
//             item.
//

#include "Stdafx.h"
#include "priv.h"
#include "shellext.h"


//
//  FUNCTION: CShellExt::QueryContextMenu(HMENU, UINT, UINT, UINT, UINT)
//
//  PURPOSE: Called by the shell just before the context menu is displayed.
//           This is where you add your specific menu items.
//
//  PARAMETERS:
//    hMenu      - Handle to the context menu
//    indexMenu  - Index of where to begin inserting menu items
//    idCmdFirst - Lowest value for new menu ID's
//    idCmtLast  - Highest value for new menu ID's
//    uFlags     - Specifies the context of the menu event
//
//  RETURN VALUE:
//
//
//  COMMENTS:
//

STDMETHODIMP CShellExt::QueryContextMenu(HMENU hMenu,
                                         UINT indexMenu,
                                         UINT idCmdFirst,
                                         UINT idCmdLast,
                                         UINT uFlags){
    UINT idCmd = idCmdFirst;
    char szMenuText[64];
    BOOL bAppendItems=true;
    if ((uFlags & 0x000F) == CMF_NORMAL){  //Check == here, since CMF_NORMAL=0
        lstrcpy(szMenuText, "&Convert to P3D");
    } else{
		 if (uFlags & CMF_VERBSONLY){
            lstrcpy(szMenuText, "&Convert to P3D");
		 }else{
			  if (uFlags & CMF_EXPLORE){
					lstrcpy(szMenuText, "&Convert to P3D");
			  }else{
				  if (uFlags & CMF_DEFAULTONLY){
						bAppendItems = false;
				  }else{
						char szTemp[32];
						wsprintf(szTemp, "uFlags==>%d\r\n", uFlags);
						bAppendItems = false;
				  }
			  }
		 }
	 }
	if (bAppendItems){
		InsertMenu(hMenu, indexMenu++, MF_SEPARATOR|MF_BYPOSITION, 0, NULL);
		InsertMenu(hMenu, indexMenu,MF_STRING|MF_BYPOSITION,idCmd++, szMenuText);
		return ResultFromShort(idCmd-idCmdFirst); //Must return number of menu items we added.
   }
   return NOERROR;
}
STDMETHODIMP CShellExt::InvokeCommand(LPCMINVOKECOMMANDINFO lpcmi)
{
	HRESULT hr = E_INVALIDARG;

    //If HIWORD(lpcmi->lpVerb) then we have been called programmatically
    //and lpVerb is a command that should be invoked.  Otherwise, the shell
    //has called us, and LOWORD(lpcmi->lpVerb) is the menu ID the user has
    //selected.  Actually, it's (menu ID - idCmdFirst) from QueryContextMenu().
	if (!HIWORD(lpcmi->lpVerb)){
        UINT idCmd = LOWORD(lpcmi->lpVerb);
        switch (idCmd){
				case 0:{
                hr = DoW3DMenu1(lpcmi->hwnd,lpcmi->lpDirectory,lpcmi->lpVerb,lpcmi->lpParameters, lpcmi->nShow);
                break;
				}
        }
    }
    return hr;
}
STDMETHODIMP CShellExt::GetCommandString(UINT idCmd,
                                         UINT uFlags,
                                         UINT FAR *reserved,
                                         LPSTR pszName,
                                         UINT cchMax){
    switch (idCmd){
        case 0:
            lstrcpy(pszName, "New menu item number 1");
            break;

	 }
    return NOERROR;
}

STDMETHODIMP CShellExt::DoW3DMenu1(HWND hParent,
                                   LPCSTR pszWorkingDir,
                                   LPCSTR pszCmd,
                                   LPCSTR pszParam,
                                   int iShowCmd){
    MessageBox(hParent, "Not Implemented !", "Sorry !", MB_OK);
    return NOERROR;
}

