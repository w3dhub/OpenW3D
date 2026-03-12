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
 *                     $Archive:: /Commando/Code/wwphys/phunits.h                             $*
 *                                                                                             *
 *                       Author:: Greg_h                                                       *
 *                                                                                             *
 *                     $Modtime:: 9/22/99 9:45a                                               $*
 *                                                                                             *
 *                    $Revision:: 5                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef PHUNITS_H
#define PHUNITS_H

/*
** The purpose of this header file is to define macros and functions which
** accept data in various units and convert it to the internal units used
** by the physics system (metric probably).
**
** Examples:
** (1) defining a velocity of 10mph:
** vel = PhysicsUnits::Miles(10.0f) / PhysicsUnits::Hours(1.0f);
**
** (2) defining a mass of 10 grams:
** mass = PhysicsUnits::Grams(10.0f);
**
** (3) defining a force of 60 Newtons (kg*m/s^2):
** hmmm... ;-)
*/

#define INCHES_TO_METERS			(0.0254f)
#define FEET_TO_METERS				(INCHES_TO_METERS * 12.0f)
#define YARDS_TO_METERS				(INCHES_TO_METERS * 36.0f)
#define MILES_TO_METERS				(FEET_TO_METERS * 5280.0f)

#define PHYSICS_INCHES(x)			(INCHES_TO_METERS * (x))
#define PHYSICS_FEET(x)				(FEET_TO_METERS * (x))
#define PHYSICS_YARDS(x)			(YARDS_TO_METERS * (x))
#define PHYSICS_MILES(x)			(MILES_TO_METERS * (x))
#define PHYSICS_MILLIMETERS(x)	((x) / 1000.0f)
#define PHYSICS_CENTIMETERS(x)	((x) / 100.0f)
#define PHYSICS_DECIMETERS(x)		((x) / 10.0f)
#define PHYSICS_METERS(x)			((x))
#define PHYSICS_KILOMETERS(x)		((x) * 1000.0f)
#define PHYSICS_SECONDS(x)			((x))
#define PHYSICS_MINUTES(x)			((x) * 60.0f)
#define PHYSICS_HOURS(x)			((x) * 60.0f * 60.0f)
#define PHYSICS_GRAMS(x)			((x) * 1000.0f)
#define PHYSICS_KILOGRAMS(x)		((x))
#define PHYSICS_NEWTONS(x)			((x))


class PhysicsUnits
{
	// Distance Units:
	static float Inches(float inches) { return  INCHES_TO_METERS * inches; }
	static float Feet(float feet) { return FEET_TO_METERS * feet; }
	static float Yards(float yards) { return YARDS_TO_METERS * yards; }
	static float Miles(float miles) { return MILES_TO_METERS * miles; }
	static float Millimeters(float mm) { return mm / 1000.0f; }
	static float Centimeters(float cm) { return cm / 100.0f; }
	static float Decimeters(float dm) { return  dm / 10.0f; }
	static float Meters(float m) { return m; }
	static float Kilometers(float km) { return km * 1000.0f; }

	// Time Units:
	static float Seconds(float s) { return s; }
	static float Minutes(float m) { return m * 60.0f; }
	static float Hours(float h) { return h * 60.0f * 60.0f; }

	// Mass Units:
	static float Grams(float g) { return g * 1000.0f; }
	static float Kilograms(float kg) { return kg; }

	// Force Units:
	static float Newtons(float n) { return n; }
};

#endif /*PHUNITS_H*/
