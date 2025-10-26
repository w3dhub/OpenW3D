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

#include "clipboard.h"

HINSTANCE hInstance;

static BOOL controlsInit = false;

#define DLLEXPORT __declspec(dllexport)

//----------------------------------------------------------------------------
// DllMain
//----------------------------------------------------------------------------

BOOL WINAPI DllMain
(
	HINSTANCE hinstDLL,
	ULONG,
	LPVOID
)
{
	hInstance = hinstDLL;

	if ( ! controlsInit )
	{
		controlsInit = true;
		
		InitCustomControls(hInstance);		// jaguar controls
		InitCommonControls();				// initialize Chicago controls
	}
			
	return true;
}

//----------------------------------------------------------------------------
// LibDescription
//----------------------------------------------------------------------------

DLLEXPORT const TCHAR * LibDescription()
{
	return _T("Animation Key Clipboard Utility");
}

//----------------------------------------------------------------------------
// LibNumberClasses
//----------------------------------------------------------------------------

DLLEXPORT int LibNumberClasses()
{
	return 1;
}

//----------------------------------------------------------------------------
// LibClassDesc
//----------------------------------------------------------------------------

DLLEXPORT ClassDesc * LibClassDesc(int i)
{
	return ClipboardDesc ();
}

//----------------------------------------------------------------------------
// LibVersion
//----------------------------------------------------------------------------

DLLEXPORT ULONG LibVersion()
{
	return VERSION_3DSMAX;
}
