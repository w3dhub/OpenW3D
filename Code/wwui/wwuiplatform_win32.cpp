/*
**	Command & Conquer Renegade(tm)
**	Copyright 2026 OpenW3D Developers
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

#include "wwuiplatform.h"


////////////////////////////////////////////////////////////////
//
//	Get_Keyboard_State
//
////////////////////////////////////////////////////////////////
void
WWUIPlatform::Get_Keyboard_State (uint8_t state[256])
{
	::GetKeyboardState (state);
}


////////////////////////////////////////////////////////////////
//
//	Is_Key_Down
//
////////////////////////////////////////////////////////////////
bool
WWUIPlatform::Is_Key_Down (int vk)
{
	return (::GetAsyncKeyState (vk) < 0);
}


////////////////////////////////////////////////////////////////
//
//	Get_Wheel_Scroll_Lines
//
////////////////////////////////////////////////////////////////
int
WWUIPlatform::Get_Wheel_Scroll_Lines (void)
{
	UINT scrolllines = 0;
	SystemParametersInfo (SPI_GETWHEELSCROLLLINES, 0, &scrolllines, 0);
	return int(scrolllines);
}
