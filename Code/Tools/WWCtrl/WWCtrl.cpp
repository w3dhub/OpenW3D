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

// WWCtrl.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include <afxdllx.h>
#include "WWCtrl.H"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
//
// Declarations
//
extern void RegisterColorPicker (HINSTANCE hinst);
extern void RegisterColorBar (HINSTANCE hinst);

HINSTANCE _hinstance = NULL;

static AFX_EXTENSION_MODULE WWCtrlDLL = { NULL, NULL };

/////////////////////////////////////////////////////////////
//
//	DllMain
//
/////////////////////////////////////////////////////////////
extern "C" int APIENTRY
DllMain (HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	// Remove this if you use lpReserved
	UNREFERENCED_PARAMETER(lpReserved);

	if (dwReason == DLL_PROCESS_ATTACH)
	{
		TRACE0("WWCTRL.DLL Initializing!\n");
		_hinstance = hInstance;

		// Extension DLL one-time initialization
		if (!AfxInitExtensionModule(WWCtrlDLL, hInstance))
			return 0;

		// Insert this DLL into the resource chain
		// NOTE: If this Extension DLL is being implicitly linked to by
		//  an MFC Regular DLL (such as an ActiveX Control)
		//  instead of an MFC application, then you will want to
		//  remove this line from DllMain and put it in a separate
		//  function exported from this Extension DLL.  The Regular DLL
		//  that uses this Extension DLL should then explicitly call that
		//  function to initialize this Extension DLL.  Otherwise,
		//  the CDynLinkLibrary object will not be attached to the
		//  Regular DLL's resource chain, and serious problems will
		//  result.

		RegisterColorPicker (hInstance);
		RegisterColorBar (hInstance);

		new CDynLinkLibrary(WWCtrlDLL);
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		TRACE0("WWCTRL.DLL Terminating!\n");
		// Terminate the library before destructors are called
		AfxTermExtensionModule(WWCtrlDLL);
	}
	return 1;   // ok
}


/////////////////////////////////////////////////////////////
//
//	Initialize_WWCtrl
//
/////////////////////////////////////////////////////////////
__declspec (dllexport) void
Initialize_WWCtrl (void)
{
	return ;
}
