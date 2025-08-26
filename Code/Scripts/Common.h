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

/******************************************************************************
*
* FILE
*     Common.h
*
* DESCRIPTION
*     Common scripting functions and definitions.
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*
* VERSION INFO
*     $Author: Rich_d $
*     $Revision: 12 $
*     $Modtime: 6/13/00 11:57a $
*     $Archive: /Commando/Code/Scripts/Common.h $
*
******************************************************************************/

#ifndef _COMMON_H_
#define _COMMON_H_

#include "customevents.h"
#include "scripts.h"
#include "DPrint.h"
#include "groupcontrol.h"
#include "group.h"
#include "vector3.h"
#include "wwmath.h"
#include <string.h>
#include <assert.h>

class Vector3;

Vector3 RandomVector3(float xRange, float yRange, float zRange);

#endif // _COMMON_H_
