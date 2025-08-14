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

#ifndef WINBLOWS_HEADER
#define WINBLOWS_HEADER

#include <windows.h>
#include <windowsx.h>
#include <stdlib.h>

#include"wstypes.h"

  extern HINSTANCE Global_instance;
  extern LPSTR     Global_commandline;
  extern int       Global_commandshow;

  extern int       main(int argc, char *argv[]);

  int              Print_WM(UINT wm,char *out);

#endif
