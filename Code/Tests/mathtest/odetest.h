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
 *                 Project Name : WWMath Test Program                                          *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tests/mathtest/odetest.h                     $*
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 6/29/99 11:29a                                              $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#ifndef ODETEST_H
#define ODETEST_H


#ifndef ALWAYS_H
#include "always.h"
#endif

#ifndef VECTOR3_H
#include "vector3.h"
#endif

#ifndef ODE_H
#include "ode.h"
#endif



class ODETestClass : public ODESystemClass
{

public:

	ODETestClass(void) : Point(1.0,0.0,0.0), AngVel(0.5) { }

	Vector3				Point;
	double				AngVel;

	virtual void 	Get_State(StateVectorClass & set_state)
	{
		set_state.Add(Point.X);
		set_state.Add(Point.Y);
		set_state.Add(Point.Z);
	}

	virtual int		Set_State(const StateVectorClass & new_state,int start_index)
	{
		Point.X = new_state[start_index++];
		Point.Y = new_state[start_index++];
		Point.Z = new_state[start_index++];
		return start_index;
	}

	virtual int		Compute_Derivatives(float t,StateVectorClass * test_state,StateVectorClass * set_derivs,int index)
	{
		if (test_state != NULL) {
			Set_State(*test_state,index);
		}

		Vector3 Vel;
		Vector3::Cross_Product(Vector3(0,0,AngVel) , Point , &Vel);

		(*set_derivs)[index++] = Vel[0];
		(*set_derivs)[index++] = Vel[1];
		(*set_derivs)[index++] = Vel[2];

		return index;
	}
};


#endif /*ODETEST_H*/
