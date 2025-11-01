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

/* $Header: /Commando/Code/Tools/ViewTrans/dllmain.cpp 2     1/16/98 5:02p Greg_h $ */
/*********************************************************************************************** 
 ***                            Confidential - Westwood Studios                              *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : Matrix Viewer Utility                                        * 
 *                                                                                             * 
 *                    File Name : DLLMAIN.CPP                                                  * 
 *                                                                                             * 
 *                   Programmer : Greg Hjelstrom                                               * 
 *                                                                                             * 
 *                   Start Date : 02/25/97                                                     * 
 *                                                                                             * 
 *                  Last Update : February 25, 1997 [GH]                                       * 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 *   DllMain -- Windows DLL initialization                                                     * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include <Max.h>
#include "viewtrans.h"


/*
** Globals
*/

int controlsInit = false;


/*********************************************************************************************** 
 * DllMain -- Windows DLL initialization                                                       * 
 *                                                                                             * 
 * This function is called by Windows when the DLL is loaded.  This                            *
 * function may also be called many times during time critical operations					   *
 * like rendering.  Therefore developers need to be careful what they						   *
 * do inside this function.  In the code below, note how after the DLL is					   *
 * loaded the first time only a few statements are executed.								   *
 *																							   *
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   02/25/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved) 
{	
	// Hang on to this DLL's instance handle.
	hInstance = hinstDLL;

	if (! controlsInit) {
		controlsInit = true;
		
		// Initialize MAX's custom controls
		InitCustomControls(hInstance);

		// Initialize Win95 controls
		InitCommonControls();
	}
	
	return(true);
}

__declspec(dllexport) const TCHAR *
LibDescription() { return _T("Transformation Matrix Viewer"); }

__declspec(dllexport) int 
LibNumberClasses() { return 1; }

__declspec(dllexport) ClassDesc* 
LibClassDesc(int i) { return &UtilityDesc; }

__declspec(dllexport) ULONG 
LibVersion() { return VERSION_3DSMAX; }

