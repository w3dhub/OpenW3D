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
 *                     $Archive:: /Commando/Code/wwphys/vehicledazzle.h                       $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 10/29/01 4:07p                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef VEHICLEDAZZLE_H
#define VEHICLEDAZZLE_H

#include "always.h"

class RenderObjClass;
class DazzleRenderObjClass;
class VehiclePhysClass;

/**
** VehicleDazzleClass
** This object controls a dazzle for a vehicle.  These are used to implement headlight dazzles,
** brakelight dazzles, and blinking dazzles on vehicles.
*/
class VehicleDazzleClass
{
public:

	VehicleDazzleClass(void);
	~VehicleDazzleClass(void);

	void								Set_Model(DazzleRenderObjClass * model);
	void								Set_Time_Of_Day(float time);
	void								Pre_Render_Update(VehiclePhysClass * parent);

	static bool						Is_Vehicle_Dazzle(RenderObjClass * model);

private:

	enum { HEADLIGHT_TYPE = 0, BRAKELIGHT_TYPE, BLINKLIGHT_TYPE, NONE = -1};
	static int						Determine_Type(RenderObjClass * model);

	int								Type;
	DazzleRenderObjClass *		Model;
	float								BlinkRate;		//(blinks per minute)

	unsigned int					CreationTime;

	// Not Implemented
	VehicleDazzleClass(const VehicleDazzleClass & that);
	VehicleDazzleClass & operator = (const VehicleDazzleClass & that);
};


#endif // VEHICLEDAZZLE_H

