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

//*****************************************************************************
//
//	Copyright (c) 2000 Westwood Studios. All Rights Reserved.
//
//	cominit.cpp
//
// Created on 30 Mar 2001 by Tom Spencer-Smith (Westwood/Vegas)
//
//	Description:
//
//	See h file.
//
//*****************************************************************************

#include "cominit.h"

#include <objbase.h>

//#include "utility.h"

// 
// Creating this instance will setup all COM stuff & do cleanup on program exit
//
static cComInit	global_com_initializer;

//---------------------------------------------------------------------------
cComInit::cComInit
(
	void
)
{
	HRESULT hres = ::CoInitialize(NULL);
	if (!SUCCEEDED(hres))
	{
		::MessageBoxA(NULL, "Unable to initialize COM.", "Error:", MB_OK | MB_ICONERROR);
		::exit(0);
	}
}

//---------------------------------------------------------------------------
cComInit::~cComInit
(
	void
)
{
	::CoUninitialize();
}

//---------------------------------------------------------------------------

