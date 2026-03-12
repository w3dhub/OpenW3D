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

#include "matrix3.h"
#include "vector3.h"
#include "cvec.h"

typedef enum
{
	itIntersects,
	itA0, itA1, itA2,
	itB0, itB1, itB2,
	itA0B0, itA0B1, itA0B2,
	itA1B0, itA1B1, itA1B2,
	itA2B0, itA2B1, itA2B2
}
IntersectType;


typedef struct
{
	Vector center;
	Vector basis[3];
	Vector extent;
	Vector velocity;
}
Box;

class BoxClass
{
public:

	BoxClass(Box box);

	Vector3	Center;
	Vector3	Extent;
	Vector3	Velocity;
	Matrix3	Basis;
};


IntersectType Boxes_Intersect(const BoxClass & box0, const BoxClass & box1);
IntersectType Boxes_Intersect(const BoxClass & box0, const BoxClass & box1,float dt);
IntersectType BoxesIntersect(float dt, const Box & box0, const Box & box1);
