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
 *                 Project Name : LevelEdit                                                    *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/ProgressUI.h                 $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 12/07/99 5:51p                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __PROGRESS_UI_H
#define __PROGRESS_UI_H


///////////////////////////////////////////////////////////////////////
//
//	ProgressUIClass
//
///////////////////////////////////////////////////////////////////////
class ProgressUIClass
{
public:

	////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////////
	ProgressUIClass (void)				{ }
	virtual ~ProgressUIClass (void)	{ }

	////////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////////

	virtual void			Show (bool /* show */)	{ }
	virtual void			Close (void)		{ }

	virtual void			Set_Status_Text (LPCTSTR /* text */)			{ }
	virtual void			Set_Progress_Percent (float /* percent */)	{ }
};


///////////////////////////////////////////////////////////////////////
//
//	ProgressUIMgrClass
//
///////////////////////////////////////////////////////////////////////
class ProgressUIMgrClass
{
public:

	////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////////
	//ProgressUIMgrClass (void)				{ }
	//virtual ~ProgressUIMgrClass (void)	{ }

	////////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////////

	static void			Set_Status_Text (LPCTSTR text);
	static void			Set_Progress_Percent (float percent);

	static ProgressUIClass *	Get_Current_Progress_UI (void);
	static void						Set_Current_Progress_UI (ProgressUIClass *progress_ui);

private:

	////////////////////////////////////////////////////////////////////
	//	Static member data
	////////////////////////////////////////////////////////////////////
	static ProgressUIClass *	_CurrentProgressUI;
};


/////////////////////////////////////////////////////////////////////
//	Get_Current_Progress_UI
/////////////////////////////////////////////////////////////////////
inline ProgressUIClass *
ProgressUIMgrClass::Get_Current_Progress_UI (void)
{
	return _CurrentProgressUI;
}


/////////////////////////////////////////////////////////////////////
//	Set_Current_Progress_UI
/////////////////////////////////////////////////////////////////////
inline void
ProgressUIMgrClass::Set_Current_Progress_UI (ProgressUIClass *progress_ui)
{
	_CurrentProgressUI = progress_ui;
	return ;
}


#endif //__PROGRESS_UI_H
