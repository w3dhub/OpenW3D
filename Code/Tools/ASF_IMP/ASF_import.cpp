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

//----------------------------------------------------------------------------
// asf_import.cpp
//
// Acclaim Skeleton File import module
//
// James McNeill
//
// Created October 1996
//
// Copyright (c) 1996 Westwood Studios
//----------------------------------------------------------------------------

#include <Max.h>
#include <istdplug.h>
#include <splshape.h>
#include <dummy.h>

#include "asf_resource.h"
#include "asf_data.h"
#include "read_asf.h"
#include "exception.h"

static HINSTANCE   hInstance;

static TCHAR *     GetString ( int id )
{
	static TCHAR buf[256];
	if (hInstance)
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
}

static int MessageBox ( int s1, int s2, int option = MB_OK )
{
	TSTR str1(GetString(s1));
	TSTR str2(GetString(s2));
	return MessageBox(GetActiveWindow(), str1, str2, option);
}

static int Alert ( int s1, int s2 = IDS_LIB_SHORT_DESC, int option = MB_OK )
{
	return MessageBox(s1, s2, option);
}

//----------------------------------------------------------------------------
// ASF_Import
//----------------------------------------------------------------------------

class ASF_Import : public SceneImport
{
public:

					ASF_Import();
					~ASF_Import();

	int				ExtCount();			// Number of extensions supported
	const TCHAR *	Ext(int n);			// Extension #n
	const TCHAR *	LongDesc();			// Long ASCII description
	const TCHAR *	ShortDesc();		// Short ASCII description
	const TCHAR *	AuthorName();		// ASCII Author name
	const TCHAR *	CopyrightMessage();	// ASCII Copyright message
	const TCHAR *	OtherMessage1();	// Other message #1
	const TCHAR *	OtherMessage2();	// Other message #2
	unsigned int	Version();			// Version number * 100
	void			ShowAbout(HWND);	// Show DLL's "About..." box

	int				DoImport
	(
		const TCHAR *  name,
		ImpInterface * i,
		Interface *    gi,
		BOOL				suppressPrompts=false
	);
};

//----------------------------------------------------------------------------
// DllMain
//----------------------------------------------------------------------------

static int         controlsInit = false;

BOOL WINAPI        DllMain
(
    HINSTANCE      hinstDLL,
	ULONG          fdwReason,
	LPVOID         lpvReserved
)
{
	hInstance = hinstDLL;

	if ( !controlsInit )
	{
		controlsInit = true;
		InitCustomControls(hInstance);
		InitCommonControls();
	}
	switch(fdwReason)
	{
		case DLL_PROCESS_ATTACH: break;
		case DLL_THREAD_ATTACH:  break;
		case DLL_THREAD_DETACH:  break;
		case DLL_PROCESS_DETACH: break;
	}

	return true;
}


//----------------------------------------------------------------------------
// ASF_ClassDesc
//----------------------------------------------------------------------------

class ASF_ClassDesc : public ClassDesc
{
public:
	int 			IsPublic()     { return 1; }
	void *			Create(BOOL loading = false) { return new ASF_Import; }
	const TCHAR *	ClassName()    { return GetString(IDS_SHORT_DESC); }
	SClass_ID		SuperClassID() { return SCENE_IMPORT_CLASS_ID; }
	Class_ID		ClassID()      { return Class_ID(0x74975aa6, 0x1810323f); }
	const TCHAR* 	Category()     { return GetString(IDS_CATEGORY);  }
};

static ASF_ClassDesc ASF_desc;

//----------------------------------------------------------------------------
// This is the interface to Jaguar:
//----------------------------------------------------------------------------

__declspec( dllexport ) const TCHAR *    LibDescription()
{
	return GetString(IDS_LIB_LONG_DESC);
}

__declspec( dllexport ) int    LibNumberClasses()
{
	return 1;
}

__declspec( dllexport ) ClassDesc *    LibClassDesc(int i)
{
	switch(i)
	{
	case  0: return & ASF_desc; break;
	default: return          0; break;
	}
}

// Return version so can detect obsolete DLLs
__declspec( dllexport ) ULONG    LibVersion()
{
	return VERSION_3DSMAX;
}

//
// ASF import module functions follow:
//

ASF_Import::ASF_Import() {}
ASF_Import::~ASF_Import() {}

int    ASF_Import::ExtCount()
{
	return 1;
}

// Extensions supported for import/export modules

const TCHAR *      ASF_Import::Ext(int n)
{
	if ( n == 0 )	return _T("ASF");
	else			return _T("");
}

const TCHAR *      ASF_Import::LongDesc()
{
	return GetString(IDS_LONG_DESC);
}
	
const TCHAR *      ASF_Import::ShortDesc()
{
	return GetString(IDS_SHORT_DESC);
}

const TCHAR *      ASF_Import::AuthorName()
{
	return GetString(IDS_AUTHOR_NAME);
}

const TCHAR *      ASF_Import::CopyrightMessage()
{
	return GetString(IDS_COPYRIGHT);
}

const TCHAR *      ASF_Import::OtherMessage1()
{
	return _T("");
}

const TCHAR *      ASF_Import::OtherMessage2()
{
	return _T("");
}

unsigned int       ASF_Import::Version()
{
	return 100;
}

void               ASF_Import::ShowAbout(HWND hWnd)
{
}

//----------------------------------------------------------------------------
// asf_load
//----------------------------------------------------------------------------

static int         asf_load
(
	const TCHAR *  filename,
	ImpInterface * iface,
	Interface *    gi
)
{
	// Load the skeleton definition file.

	try
	{
		Skeleton_Class skeleton ( filename, iface, gi );
	}
	catch ( const Parse_Error & error )
	{
		MessageBox ( GetActiveWindow (), error.message (), "Parse error",
			MB_OK );
		return -1;
	}

	// Create a matching skeleton in 3DS Max.

	return 1;
}

//----------------------------------------------------------------------------
// ASF_Import::DoImport
//----------------------------------------------------------------------------

int                ASF_Import::DoImport
(
	const TCHAR *  filename,
	ImpInterface * iface,
	Interface *    gi,
	BOOL 
)
{
	int status;

	status = asf_load ( filename, iface, gi );
	if(status == 0)
		status = IMPEXP_CANCEL;	

	return (status <= 0) ? IMPEXP_FAIL : status;
}
