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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/HeightWatcher.h              $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 5/27/00 3:35p                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __HEIGHT_WATCHER_H
#define __HEIGHT_WATCHER_H

#include <algorithm>

//////////////////////////////////////////////////////////////////////////
//
//	HeightWatcherClass
//
//////////////////////////////////////////////////////////////////////////
class HeightWatcherClass
{
public:

	////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	////////////////////////////////////////////////////////////////////
	HeightWatcherClass (void)
		:	m_MinZPos (0),
			m_MaxZPos (0),
			m_CurrMinZPos (0),
			m_CurrMaxZPos (0) {}

	////////////////////////////////////////////////////////////////////
	//	Public methods
	////////////////////////////////////////////////////////////////////
	void	Initialize (float min_z, float max_z, float curr_z);
	void	Add_Upper_Bound (float z_pos);
	void	Add_Lower_Bound (float z_pos);
	bool	Is_Current_Pos_Valid (float curr_z_pos);

private:

	////////////////////////////////////////////////////////////////////
	//	Private member data
	////////////////////////////////////////////////////////////////////
	float	m_MinZPos;
	float	m_MaxZPos;
	float m_CurrMinZPos;
	float	m_CurrMaxZPos;
};


//////////////////////////////////////////////////////////////////////////
//	Initialize
//////////////////////////////////////////////////////////////////////////
inline void
HeightWatcherClass::Initialize (float min_z, float max_z, float curr_z)
{
	m_MinZPos = min_z;
	m_MaxZPos = max_z;
	m_CurrMinZPos = curr_z;
	m_CurrMaxZPos = curr_z;
	return ;
}

//////////////////////////////////////////////////////////////////////////
//	Add_Upper_Bound
//////////////////////////////////////////////////////////////////////////
inline void
HeightWatcherClass::Add_Upper_Bound (float z_pos)
{
	m_MaxZPos = std::min (m_MaxZPos, z_pos);
	return ;
}

//////////////////////////////////////////////////////////////////////////
//	Add_Lower_Bound
//////////////////////////////////////////////////////////////////////////
inline void
HeightWatcherClass::Add_Lower_Bound (float z_pos)
{
	m_MinZPos = std::max (m_MinZPos, z_pos);
	return ;
}

//////////////////////////////////////////////////////////////////////////
//	Is_Current_Pos_Valid
//////////////////////////////////////////////////////////////////////////
inline bool
HeightWatcherClass::Is_Current_Pos_Valid (float curr_z_pos)
{
	m_CurrMaxZPos = std::max (curr_z_pos, m_CurrMaxZPos);
	m_CurrMinZPos = std::min (curr_z_pos, m_CurrMinZPos);
	return	(m_CurrMaxZPos >= m_MinZPos) && (m_CurrMaxZPos <= m_MaxZPos) &&
				(m_CurrMinZPos >= m_MinZPos) && (m_CurrMinZPos <= m_MaxZPos);
}


#endif //__HEIGHT_WATCHER_H

