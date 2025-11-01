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

// UIThread.cpp : implementation file
//

#include "StdAfx.h"
#include "LevelEdit.h"
#include "UIThread.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// UIThreadClass

IMPLEMENT_DYNCREATE(UIThreadClass, CWinThread)

UIThreadClass::UIThreadClass()
{
}

UIThreadClass::~UIThreadClass()
{
}

BOOL UIThreadClass::InitInstance()
{
	// TODO:  perform and per-thread initialization here
	return true;
}

int UIThreadClass::ExitInstance()
{
	// TODO:  perform any per-thread cleanup here
	return CWinThread::ExitInstance();
}

BEGIN_MESSAGE_MAP(UIThreadClass, CWinThread)
	//{{AFX_MSG_MAP(UIThreadClass)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// UIThreadClass message handlers
