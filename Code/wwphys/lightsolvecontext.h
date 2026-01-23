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
 *                 Project Name : WWPhys                                                       *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwphys/lightsolvecontext.h                   $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 2/27/02 1:21p                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#ifndef LIGHTSOLVECONTEXT_H
#define LIGHTSOLVECONTEXT_H

#include "always.h"
#include "lightsolveprogress.h"

class LightSolveObserverClass;

/**
** LightSolveContextClass
** This class is a container for all of the options and current state of
** a light solve.
** Any temporary data which needs to be passed around during the solve can
** be added into this class.
*/
class LightSolveContextClass
{
public:
	LightSolveContextClass(void);
	~LightSolveContextClass(void);

	void								Enable_Filtering(bool onoff) { FilteringEnabled = onoff; }
	bool								Is_Filtering_Enabled(void) { return FilteringEnabled; }
	void								Enable_Occlusion(bool onoff) { OcclusionEnabled = onoff; }
	bool								Is_Occlusion_Enabled(void) { return OcclusionEnabled; }

	LightSolveProgressClass &	Get_Progress(void) { return Progress; }
	
	void								Set_Observer(LightSolveObserverClass * observer) { Observer = observer; }
	LightSolveObserverClass *	Get_Observer(void) { return Observer; }

	void								Update_Observer(void); 

protected:

	bool								OcclusionEnabled;
	bool								FilteringEnabled;

	LightSolveProgressClass		Progress;
	LightSolveObserverClass	*	Observer;
};

/**
** LightSolveObserverClass 
** Derive from this class and install your object in the light solve context to get
** progress callbacks periodically...
*/
class LightSolveObserverClass
{	
public:
	virtual ~LightSolveObserverClass(void) { };
	virtual void Progress_Callback(LightSolveContextClass & /* context */) { };
};

#endif //LIGHTSOLVECONTEXT_H

