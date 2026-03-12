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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/HitTestInfo.h                $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 9/29/99 4:39p                                               $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __HITTESTINFO_H
#define __HITTESTINFO_H


class NodeClass;
class NodeFunctionClass;
class VisGenPointClass;


/////////////////////////////////////////////////////////////////
//
// Structures and typedefs
//
/////////////////////////////////////////////////////////////////
typedef struct _HITTESTINFO
{
	enum
	{
		Node		= 0,
		Function,
		VisPoint
	} Type;

	union
	{
		NodeClass *				node;
		NodeFunctionClass *	function;
		VisGenPointClass	*	vis_point;
	};

	_HITTESTINFO (NodeClass *new_node)					{ Type = Node; node = new_node; }
	_HITTESTINFO (NodeFunctionClass *new_function)	{ Type = Function; function = new_function; }
	_HITTESTINFO (VisGenPointClass *new_vis_point)	{ Type = VisPoint; vis_point = new_vis_point; }

} HITTESTINFO;


#endif //__HITTESTINFO_H
