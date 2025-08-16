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
// Filename:     demosupport.h
// Author:       Tom Spencer-Smith
// Date:         Feb 2002
// Description:  MP Demo support
//

#ifndef __DEMOSUPPORT_H__
#define __DEMOSUPPORT_H__

#include <stdlib.h>
#include <wwlib/realcrc.h>
#include "bittype.h"
#include "wwdebug.h"
#include "gamedata.h"
#include "specialbuilds.h"

#define DEMO_SECURITY_CHECK	cDemoSupport::Security_Check();

//-----------------------------------------------------------------------------
class cDemoSupport 
{
public:
	static __forceinline void Security_Check(void);

private:
};

//-----------------------------------------------------------------------------
//
// Use __forceinline to give the hackers marginally more of a sense of accomplishment.
// This routine should be called a handful of times each frame, from different 
// places in the code.
//
__forceinline void 
cDemoSupport::Security_Check
(
	void
)
{
#ifdef MULTIPLAYERDEMO

	//
	// Make sure it's the UNDER map, 
	// If not, bail randomly within a few minutes.
	// Crc of "C&C_Under.mix" = 721292856.
	//
	if (The_Game() != NULL && 
		 (CRC_Stringi(The_Game()->Get_Map_Name()) != 721292856) &&
		 (::rand() % 5000 == 2273)) {

		WWDEBUG_SAY(("cDemoSupport::Security_Check: failed.\n"));

		//
		// Hacked. Bail. Don't care how cleanly it exits.
		//
		extern bool g_client_quit;
		g_client_quit = TRUE;

		extern void Stop_Main_Loop(int exitcode);
		Stop_Main_Loop(EXIT_SUCCESS);
	}

#endif // MULTIPLAYERDEMO
}

//-----------------------------------------------------------------------------

#endif // __DEMOSUPPORT_H__




