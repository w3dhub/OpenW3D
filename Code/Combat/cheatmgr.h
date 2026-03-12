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
 *                 Project Name : combat                                                       *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Combat/cheatmgr.h          $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 12/06/01 11:35a                                             $*
 *                                                                                             *
 *                    $Revision:: 5                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __CHEATMGR_H
#define __CHEATMGR_H

#include "gametype.h"


//////////////////////////////////////////////////////////////////////
//
//	CheatMgrClass
//
//////////////////////////////////////////////////////////////////////
class CheatMgrClass
{
public:

	///////////////////////////////////////////////////////////////////
	//	Public flags
	///////////////////////////////////////////////////////////////////
	enum
	{
		CHEAT_INVULNERABILITY	= 1,
		CHEAT_INFINITE_AMMO		= 2,
		CHEAT_ALL_WEAPONS		= 4,

		ALL_CHEATS				= 0x07,

	};

	///////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	///////////////////////////////////////////////////////////////////
	CheatMgrClass  (void);
	virtual ~CheatMgrClass (void);

	///////////////////////////////////////////////////////////////////
	//	Public methods
	///////////////////////////////////////////////////////////////////

	//
	//	Singleton access
	//
	static CheatMgrClass *	Get_Instance (void)		{ return _TheInstance; }

	//
	//	Cheat control
	//
	virtual void	Enable_Cheat (int cheat, bool onoff)	{ if (onoff) Flags |= cheat; else Flags &= (~cheat); HistoryFlags |= Flags;  }
	virtual bool	Is_Cheat_Enabled (int /*cheat*/) const	{ return false; }
	virtual bool	Is_Cheat_Set (int /*cheat*/) const		{ return false; }

	virtual bool	Was_Cheat_Used (int /*cheat*/) const	{ return false; }
	virtual void	Update_History (void)						{ HistoryFlags |= Flags; }
	virtual void	Reset_History (void)							{ HistoryFlags = Flags; }
	virtual int		Get_History (void) const					{ return HistoryFlags; }
	virtual void	Set_History (int flags)						{ HistoryFlags = flags; }

	//
	//	Notifications
	//
	virtual void	Apply_Cheats (void) {}

protected:

	///////////////////////////////////////////////////////////////////
	//	Protected methods
	///////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////
	//	Protected member data
	///////////////////////////////////////////////////////////////////
	int	Flags;
	int	HistoryFlags;

	//
	//	Static member data
	//
	static CheatMgrClass *_TheInstance;
};


#endif //__CHEATMGR_H
