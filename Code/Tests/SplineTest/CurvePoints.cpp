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

#include "stdafx.h"
#include "CurvePoints.h"


float _ControlPoints[CONTROL_POINT_COUNT][4] =
{
	{ 0.0f,0.0f,0.0f,0.0f },
	{ 2.0f,1.0f,0.0f,1.0f },
	{ 4.0f,3.0f,0.0f,2.0f },
	{ 5.0f,5.0f,0.0f,3.0f },
	{ 7.0f,2.0f,0.0f,4.0f },
};

float _TCBParams[CONTROL_POINT_COUNT][3] =
{
	{ 0.0f,1.0f,0.0f },
	{ 0.0f,1.0f,0.0f },
	{ 0.0f,1.0f,0.0f },
	{ 0.0f,1.0f,0.0f },
	{ 0.0f,1.0f,0.0f },
};
