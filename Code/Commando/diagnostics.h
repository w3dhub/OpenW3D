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

//
// Filename:     diagnostics.h
// Project:      Commando
// Author:       Tom Spencer-Smith
// Date:         Dec 1998
// Description:  
//
//-----------------------------------------------------------------------------
#if defined(_MSV_VER)
#pragma once
#endif

#ifndef DIAGNOSTICS_H
#define DIAGNOSTICS_H

//
// This class is for displaying the state of variables on the screen.
// Add anything you need to monitor to the Render method.
//

#include "bittype.h"

class Render2DTextClass;
class Font3DInstanceClass;

//-----------------------------------------------------------------------------
class cDiagnostics
{
   public:
      static void		Init(void);
      static void		Close(void);
      static void		Render(void);

   private:
		static void		Show_Object_Tally(void);
		static void		Add_Diagnostic(const char *format, ...);

		static Render2DTextClass	*	PRenderer;
		static Font3DInstanceClass *	PFont;
		static float						DiagnosticX;
		static float						DiagnosticY;
};

//-----------------------------------------------------------------------------
#endif // DIAGNOSTICS_H
