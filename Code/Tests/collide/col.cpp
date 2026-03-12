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

#include "col.h"
#include "matrix3.h"
#include "vector3.h"



BoxClass::BoxClass(Box box)
{
	Center.X = box.center[0];
	Center.Y = box.center[1];
	Center.Z = box.center[2];

	Extent.X = box.extent[0];
	Extent.Y = box.extent[1];
	Extent.Z = box.extent[2];

	Velocity.X = box.velocity[0];
	Velocity.Y = box.velocity[1];
	Velocity.Z = box.velocity[2];

	Basis[0][0] = box.basis[0][0];
	Basis[0][1] = box.basis[0][1];
	Basis[0][2] = box.basis[0][2];

	Basis[1][0] = box.basis[1][0];
	Basis[1][1] = box.basis[1][1];
	Basis[1][2] = box.basis[1][2];

	Basis[2][0] = box.basis[2][0];
	Basis[2][1] = box.basis[2][1];
	Basis[2][2] = box.basis[2][2];
}

//---------------------------------------------------------------------------
IntersectType Boxes_Intersect(const BoxClass & box0, const BoxClass & box1,float dt)
{
	// memoized values for Dot_Product(box0.Basis[i],box1.Basis[j]),
	Matrix3 AB;

	// calculate difference of centers
	Vector3 C = box1.Center - box0.Center;
	Vector3 V = box1.Velocity - box0.Velocity;

	float ra, rb, rsum, u0, u1;

	/////////////////////////////////////////////////////////////////////////
	// L = A0
	//
	// Projecting the two boxes onto Box0's X axis.  If their intervals
	// on this line do not intersect, the boxes are not intersecting!
	// Each of the tests in this function work in a similar way.
	/////////////////////////////////////////////////////////////////////////
	AB[0][0] = Dot_Product(box0.Basis[0],box1.Basis[0]);
	AB[0][1] = Dot_Product(box0.Basis[0],box1.Basis[1]);
	AB[0][2] = Dot_Product(box0.Basis[0],box1.Basis[2]);

	ra = FABS(box0.Extent[0]);
	rb = FABS(box1.Extent[0]*AB[0][0])+FABS(box1.Extent[1]*AB[0][1])+FABS(box1.Extent[2]*AB[0][2]);
	rsum = ra+rb;

	// u0 = projected distance between the box centers at t0
	// u1 = projected distance between the box centers at t1
	u0 = Dot_Product(C,box0.Basis[0]);
	u1 = u0+dt*Dot_Product(V,box0.Basis[0]);

	if ((u0 > rsum && u1 > rsum) || (u0 < -rsum && u1 < -rsum) )
		return itA0;


	/////////////////////////////////////////////////////////////////////////
	// L = A1
	// Separating Axis is Box0's Y axis
	/////////////////////////////////////////////////////////////////////////
	AB[1][0] = Dot_Product(box0.Basis[1],box1.Basis[0]);
	AB[1][1] = Dot_Product(box0.Basis[1],box1.Basis[1]);
	AB[1][2] = Dot_Product(box0.Basis[1],box1.Basis[2]);

	ra = FABS(box0.Extent[1]);
	rb = FABS(box1.Extent[0]*AB[1][0])+FABS(box1.Extent[1]*AB[1][1])+FABS(box1.Extent[2]*AB[1][2]);
	rsum = ra+rb;

	u0 = Dot_Product(C,box0.Basis[1]);
	u1 = u0+dt*Dot_Product(V,box0.Basis[1]);
	if ((u0 > rsum && u1 > rsum) || (u0 < -rsum && u1 < -rsum) )
		return itA1;


	/////////////////////////////////////////////////////////////////////////
	// L = A2
	// Separating Axis is Box0's Y axis
	/////////////////////////////////////////////////////////////////////////
	AB[2][0] = Dot_Product(box0.Basis[2],box1.Basis[0]);
	AB[2][1] = Dot_Product(box0.Basis[2],box1.Basis[1]);
	AB[2][2] = Dot_Product(box0.Basis[2],box1.Basis[2]);

	ra = FABS(box0.Extent[2]);
	rb = FABS(box1.Extent[0]*AB[2][0])+FABS(box1.Extent[1]*AB[2][1])+FABS(box1.Extent[2]*AB[2][2]);
	rsum = ra+rb;

	u0 = Dot_Product(C,box0.Basis[2]);
	u1 = u0+dt*Dot_Product(V,box0.Basis[2]);
	if ((u0 > rsum && u1 > rsum) || (u0 < -rsum && u1 < -rsum) )
		return itA2;


	/////////////////////////////////////////////////////////////////////////
	// L = B0
	// Separating Axis is Box1's X axis
	/////////////////////////////////////////////////////////////////////////
	ra = FABS(box0.Extent[0]*AB[0][0])+FABS(box0.Extent[1]*AB[1][0])+FABS(box0.Extent[2]*AB[2][0]);
	rb = FABS(box1.Extent[0]);
	rsum = ra+rb;

	u0 = Dot_Product(C,box1.Basis[0]);
	u1 = u0+dt*Dot_Product(V,box1.Basis[0]);
	if ((u0 > rsum && u1 > rsum) || (u0 < -rsum && u1 < -rsum) )
		return itB0;


	/////////////////////////////////////////////////////////////////////////
	// L = B1
	// Separating Axis is Box1's Y axis
	/////////////////////////////////////////////////////////////////////////
	ra = FABS(box0.Extent[0]*AB[0][1])+FABS(box0.Extent[1]*AB[1][1])+FABS(box0.Extent[2]*AB[2][1]);
	rb = FABS(box1.Extent[1]);

	rsum = ra+rb;
	u0 = Dot_Product(C,box1.Basis[1]);
	u1 = u0+dt*Dot_Product(V,box1.Basis[1]);
	if ((u0 > rsum && u1 > rsum) || (u0 < -rsum && u1 < -rsum) )
		return itB1;


	/////////////////////////////////////////////////////////////////////////
	// L = B2
	// Separating Axis is Box1's Z axis
	/////////////////////////////////////////////////////////////////////////
	ra = FABS(box0.Extent[0]*AB[0][2])+FABS(box0.Extent[1]*AB[1][2])+FABS(box0.Extent[2]*AB[2][2]);
	rb = FABS(box1.Extent[2]);

	rsum = ra+rb;
	u0 = Dot_Product(C,box1.Basis[2]);
	u1 = u0+dt*Dot_Product(V,box1.Basis[2]);
	if ((u0 > rsum && u1 > rsum) || (u0 < -rsum && u1 < -rsum) )
		return itB2;


	/////////////////////////////////////////////////////////////////////////
	// None of the aligned axes turned out to be separating axes.  Now
	// we check all combinations of cross products of the two boxes axes.
	/////////////////////////////////////////////////////////////////////////
	Matrix3 Ainv = box0.Basis.Transpose();
	Matrix3 coeff = AB.Transpose();

	// difference of centers in box0's-basis
	Vector3 d0, d1, product;
	d0 = Ainv * C;
	d1 = Ainv * V;
	product = dt*d1;
	d1 = d0 + product;


	/////////////////////////////////////////////////////////////////////////
	// L = A0xB0
	/////////////////////////////////////////////////////////////////////////
	ra = FABS(box0.Extent[1]*coeff[0][2])+FABS(box0.Extent[2]*coeff[0][1]);
	rb = FABS(box1.Extent[1]*coeff[2][0])+FABS(box1.Extent[2]*coeff[1][0]);

	rsum = ra+rb;
	u0 = d0[2]*coeff[1][0]-d0[1]*coeff[2][0];
	u1 = d1[2]*coeff[1][0]-d1[1]*coeff[2][0];
	if ((u0 > rsum && u1 > rsum) || (u0 < -rsum && u1 < -rsum) )
		return itA0B0;


	/////////////////////////////////////////////////////////////////////////
	// L = A0xB1
	/////////////////////////////////////////////////////////////////////////
	ra = FABS(box0.Extent[1]*coeff[1][2])+FABS(box0.Extent[2]*coeff[1][1]);
	rb = FABS(box1.Extent[0]*coeff[2][0])+FABS(box1.Extent[2]*coeff[0][0]);

	rsum = ra+rb;
	u0 = d0[2]*coeff[1][1]-d0[1]*coeff[1][2];
	u1 = d1[2]*coeff[1][1]-d1[1]*coeff[1][2];
	if ((u0 > rsum && u1 > rsum) || (u0 < -rsum && u1 < -rsum) )
		return itA0B1;


	/////////////////////////////////////////////////////////////////////////
	// L = A0xB2
	/////////////////////////////////////////////////////////////////////////
	ra = FABS(box0.Extent[1]*coeff[2][2])+FABS(box0.Extent[2]*coeff[2][1]);
	rb = FABS(box1.Extent[0]*coeff[1][0])+FABS(box1.Extent[1]*coeff[0][0]);

	rsum = ra+rb;
	u0 = d0[2]*coeff[2][1]-d0[1]*coeff[2][2];
	u1 = d1[2]*coeff[2][1]-d1[1]*coeff[2][2];
	if ((u0 > rsum && u1 > rsum) || (u0 < -rsum && u1 < -rsum) )
		return itA0B2;


	/////////////////////////////////////////////////////////////////////////
	// L = A1xB0
	/////////////////////////////////////////////////////////////////////////
	ra = FABS(box0.Extent[0]*coeff[0][2])+FABS(box0.Extent[2]*coeff[0][0]);
	rb = FABS(box1.Extent[1]*coeff[2][1])+FABS(box1.Extent[2]*coeff[1][1]);

	rsum = ra+rb;
	u0 = d0[0]*coeff[0][2]-d0[2]*coeff[0][0];
	u1 = d1[0]*coeff[0][2]-d1[2]*coeff[0][0];
	if ((u0 > rsum && u1 > rsum) || (u0 < -rsum && u1 < -rsum) )
		return itA1B0;


	/////////////////////////////////////////////////////////////////////////
	// L = A1xB1
	/////////////////////////////////////////////////////////////////////////
	ra = FABS(box0.Extent[0]*coeff[1][2])+FABS(box0.Extent[2]*coeff[1][0]);
	rb = FABS(box1.Extent[0]*coeff[2][1])+FABS(box1.Extent[2]*coeff[0][1]);

	rsum = ra+rb;
	u0 = d0[0]*coeff[1][2]-d0[2]*coeff[1][0];
	u1 = d1[0]*coeff[1][2]-d1[2]*coeff[1][0];
	if ((u0 > rsum && u1 > rsum) || (u0 < -rsum && u1 < -rsum) )
		return itA1B1;


	/////////////////////////////////////////////////////////////////////////
	// L = A1xB2
	/////////////////////////////////////////////////////////////////////////
	ra = FABS(box0.Extent[0]*coeff[2][2])+FABS(box0.Extent[2]*coeff[2][0]);
	rb = FABS(box1.Extent[0]*coeff[1][1])+FABS(box1.Extent[1]*coeff[0][1]);

	rsum = ra+rb;
	u0 = d0[0]*coeff[2][2]-d0[2]*coeff[2][0];
	u1 = d1[0]*coeff[2][2]-d1[2]*coeff[2][0];
	if ((u0 > rsum && u1 > rsum) || (u0 < -rsum && u1 < -rsum) )
		return itA1B2;


	/////////////////////////////////////////////////////////////////////////
	// L = A2xB0
	/////////////////////////////////////////////////////////////////////////
	ra = FABS(box0.Extent[0]*coeff[0][1])+FABS(box0.Extent[1]*coeff[0][0]);
	rb = FABS(box1.Extent[1]*coeff[2][2])+FABS(box1.Extent[2]*coeff[1][2]);

	rsum = ra+rb;
	u0 = d0[1]*coeff[0][0]-d0[0]*coeff[0][1];
	u1 = d1[1]*coeff[0][0]-d1[0]*coeff[0][1];
	if ((u0 > rsum && u1 > rsum) || (u0 < -rsum && u1 < -rsum) )
		return itA2B0;


	/////////////////////////////////////////////////////////////////////////
	// L = A2xB1
	/////////////////////////////////////////////////////////////////////////
	ra = FABS(box0.Extent[0]*coeff[1][1])+FABS(box0.Extent[1]*coeff[1][0]);
	rb = FABS(box1.Extent[0]*coeff[2][2])+FABS(box1.Extent[2]*coeff[0][2]);

	rsum = ra+rb;
	u0 = d0[1]*coeff[1][0]-d0[0]*coeff[1][1];
	u1 = d1[1]*coeff[1][0]-d1[0]*coeff[1][1];
	if ((u0 > rsum && u1 > rsum) || (u0 < -rsum && u1 < -rsum) )
		return itA2B1;


	/////////////////////////////////////////////////////////////////////////
	// L = A2xB2
	/////////////////////////////////////////////////////////////////////////
	ra = FABS(box0.Extent[0]*coeff[2][1])+FABS(box0.Extent[1]*coeff[2][0]);
	rb = FABS(box1.Extent[0]*coeff[1][2])+FABS(box1.Extent[1]*coeff[0][2]);

	rsum = ra+rb;
	u0 = d0[1]*coeff[2][0]-d0[0]*coeff[2][1];
	u1 = d1[1]*coeff[2][0]-d1[0]*coeff[2][1];
	if ((u0 > rsum && u1 > rsum) || (u0 < -rsum && u1 < -rsum) )
		return itA2B2;

	return itIntersects;
}



//---------------------------------------------------------------------------

IntersectType Boxes_Intersect(const BoxClass & box0, const BoxClass & box1)
{
	// memoized values for Dot_Product(box0.Basis[i],box1.Basis[j]),
	Matrix3 AB;

	// calculate difference of centers
	Vector3 C = box1.Center - box0.Center;

	float ra, rb, rsum, u0;

	/////////////////////////////////////////////////////////////////////////
	// L = A0
	//
	// Projecting the two boxes onto Box0's X axis.  If their intervals
	// on this line do not intersect, the boxes are not intersecting!
	// Each of the tests in this function work in a similar way.
	/////////////////////////////////////////////////////////////////////////
	AB[0][0] = Dot_Product(box0.Basis[0],box1.Basis[0]);
	AB[0][1] = Dot_Product(box0.Basis[0],box1.Basis[1]);
	AB[0][2] = Dot_Product(box0.Basis[0],box1.Basis[2]);

	ra = FABS(box0.Extent[0]);
	rb = FABS(box1.Extent[0]*AB[0][0])+FABS(box1.Extent[1]*AB[0][1])+FABS(box1.Extent[2]*AB[0][2]);
	rsum = ra+rb;

	// u0 = projected distance between the box centers at t0
	// u1 = projected distance between the box centers at t1
	u0 = Dot_Product(C,box0.Basis[0]);

	if ((u0 > rsum) || (u0 < -rsum))
		return itA0;


	/////////////////////////////////////////////////////////////////////////
	// L = A1
	// Separating Axis is Box0's Y axis
	/////////////////////////////////////////////////////////////////////////
	AB[1][0] = Dot_Product(box0.Basis[1],box1.Basis[0]);
	AB[1][1] = Dot_Product(box0.Basis[1],box1.Basis[1]);
	AB[1][2] = Dot_Product(box0.Basis[1],box1.Basis[2]);

	ra = FABS(box0.Extent[1]);
	rb = FABS(box1.Extent[0]*AB[1][0])+FABS(box1.Extent[1]*AB[1][1])+FABS(box1.Extent[2]*AB[1][2]);
	rsum = ra+rb;

	u0 = Dot_Product(C,box0.Basis[1]);
	if ((u0 > rsum) || (u0 < -rsum))
		return itA1;


	/////////////////////////////////////////////////////////////////////////
	// L = A2
	// Separating Axis is Box0's Y axis
	/////////////////////////////////////////////////////////////////////////
	AB[2][0] = Dot_Product(box0.Basis[2],box1.Basis[0]);
	AB[2][1] = Dot_Product(box0.Basis[2],box1.Basis[1]);
	AB[2][2] = Dot_Product(box0.Basis[2],box1.Basis[2]);

	ra = FABS(box0.Extent[2]);
	rb = FABS(box1.Extent[0]*AB[2][0])+FABS(box1.Extent[1]*AB[2][1])+FABS(box1.Extent[2]*AB[2][2]);
	rsum = ra+rb;

	u0 = Dot_Product(C,box0.Basis[2]);
	if ((u0 > rsum) || (u0 < -rsum))
		return itA2;


	/////////////////////////////////////////////////////////////////////////
	// L = B0
	// Separating Axis is Box1's X axis
	/////////////////////////////////////////////////////////////////////////
	ra = FABS(box0.Extent[0]*AB[0][0])+FABS(box0.Extent[1]*AB[1][0])+FABS(box0.Extent[2]*AB[2][0]);
	rb = FABS(box1.Extent[0]);
	rsum = ra+rb;

	u0 = Dot_Product(C,box1.Basis[0]);
	if ((u0 > rsum) || (u0 < -rsum))
		return itB0;


	/////////////////////////////////////////////////////////////////////////
	// L = B1
	// Separating Axis is Box1's Y axis
	/////////////////////////////////////////////////////////////////////////
	ra = FABS(box0.Extent[0]*AB[0][1])+FABS(box0.Extent[1]*AB[1][1])+FABS(box0.Extent[2]*AB[2][1]);
	rb = FABS(box1.Extent[1]);

	rsum = ra+rb;
	u0 = Dot_Product(C,box1.Basis[1]);
	if ((u0 > rsum) || (u0 < -rsum))
		return itB1;


	/////////////////////////////////////////////////////////////////////////
	// L = B2
	// Separating Axis is Box1's Z axis
	/////////////////////////////////////////////////////////////////////////
	ra = FABS(box0.Extent[0]*AB[0][2])+FABS(box0.Extent[1]*AB[1][2])+FABS(box0.Extent[2]*AB[2][2]);
	rb = FABS(box1.Extent[2]);

	rsum = ra+rb;
	u0 = Dot_Product(C,box1.Basis[2]);
	if ((u0 > rsum) || (u0 < -rsum))
		return itB2;


	/////////////////////////////////////////////////////////////////////////
	// None of the aligned axes turned out to be separating axes.  Now
	// we check all combinations of cross products of the two boxes axes.
	/////////////////////////////////////////////////////////////////////////
	Matrix3 Ainv = box0.Basis.Transpose();
	Matrix3 coeff = AB.Transpose();

	// difference of centers in box0's-basis
	Vector3 d0, d1, product;
	d0 = Ainv * C;


	/////////////////////////////////////////////////////////////////////////
	// L = A0xB0
	/////////////////////////////////////////////////////////////////////////
	ra = FABS(box0.Extent[1]*coeff[0][2])+FABS(box0.Extent[2]*coeff[0][1]);
	rb = FABS(box1.Extent[1]*coeff[2][0])+FABS(box1.Extent[2]*coeff[1][0]);

	rsum = ra+rb;
	u0 = d0[2]*coeff[1][0]-d0[1]*coeff[2][0];
	if ((u0 > rsum) || (u0 < -rsum))
		return itA0B0;


	/////////////////////////////////////////////////////////////////////////
	// L = A0xB1
	/////////////////////////////////////////////////////////////////////////
	ra = FABS(box0.Extent[1]*coeff[1][2])+FABS(box0.Extent[2]*coeff[1][1]);
	rb = FABS(box1.Extent[0]*coeff[2][0])+FABS(box1.Extent[2]*coeff[0][0]);

	rsum = ra+rb;
	u0 = d0[2]*coeff[1][1]-d0[1]*coeff[1][2];
	if ((u0 > rsum) || (u0 < -rsum))
		return itA0B1;


	/////////////////////////////////////////////////////////////////////////
	// L = A0xB2
	/////////////////////////////////////////////////////////////////////////
	ra = FABS(box0.Extent[1]*coeff[2][2])+FABS(box0.Extent[2]*coeff[2][1]);
	rb = FABS(box1.Extent[0]*coeff[1][0])+FABS(box1.Extent[1]*coeff[0][0]);

	rsum = ra+rb;
	u0 = d0[2]*coeff[2][1]-d0[1]*coeff[2][2];
	if ((u0 > rsum) || (u0 < -rsum))
		return itA0B2;


	/////////////////////////////////////////////////////////////////////////
	// L = A1xB0
	/////////////////////////////////////////////////////////////////////////
	ra = FABS(box0.Extent[0]*coeff[0][2])+FABS(box0.Extent[2]*coeff[0][0]);
	rb = FABS(box1.Extent[1]*coeff[2][1])+FABS(box1.Extent[2]*coeff[1][1]);

	rsum = ra+rb;
	u0 = d0[0]*coeff[0][2]-d0[2]*coeff[0][0];
	if ((u0 > rsum) || (u0 < -rsum))
		return itA1B0;


	/////////////////////////////////////////////////////////////////////////
	// L = A1xB1
	/////////////////////////////////////////////////////////////////////////
	ra = FABS(box0.Extent[0]*coeff[1][2])+FABS(box0.Extent[2]*coeff[1][0]);
	rb = FABS(box1.Extent[0]*coeff[2][1])+FABS(box1.Extent[2]*coeff[0][1]);

	rsum = ra+rb;
	u0 = d0[0]*coeff[1][2]-d0[2]*coeff[1][0];
	if ((u0 > rsum) || (u0 < -rsum))
		return itA1B1;


	/////////////////////////////////////////////////////////////////////////
	// L = A1xB2
	/////////////////////////////////////////////////////////////////////////
	ra = FABS(box0.Extent[0]*coeff[2][2])+FABS(box0.Extent[2]*coeff[2][0]);
	rb = FABS(box1.Extent[0]*coeff[1][1])+FABS(box1.Extent[1]*coeff[0][1]);

	rsum = ra+rb;
	u0 = d0[0]*coeff[2][2]-d0[2]*coeff[2][0];
	if ((u0 > rsum) || (u0 < -rsum))
		return itA1B2;


	/////////////////////////////////////////////////////////////////////////
	// L = A2xB0
	/////////////////////////////////////////////////////////////////////////
	ra = FABS(box0.Extent[0]*coeff[0][1])+FABS(box0.Extent[1]*coeff[0][0]);
	rb = FABS(box1.Extent[1]*coeff[2][2])+FABS(box1.Extent[2]*coeff[1][2]);

	rsum = ra+rb;
	u0 = d0[1]*coeff[0][0]-d0[0]*coeff[0][1];
	if ((u0 > rsum) || (u0 < -rsum))
		return itA2B0;


	/////////////////////////////////////////////////////////////////////////
	// L = A2xB1
	/////////////////////////////////////////////////////////////////////////
	ra = FABS(box0.Extent[0]*coeff[1][1])+FABS(box0.Extent[1]*coeff[1][0]);
	rb = FABS(box1.Extent[0]*coeff[2][2])+FABS(box1.Extent[2]*coeff[0][2]);

	rsum = ra+rb;
	u0 = d0[1]*coeff[1][0]-d0[0]*coeff[1][1];
	if ((u0 > rsum) || (u0 < -rsum))
		return itA2B1;


	/////////////////////////////////////////////////////////////////////////
	// L = A2xB2
	/////////////////////////////////////////////////////////////////////////
	ra = FABS(box0.Extent[0]*coeff[2][1])+FABS(box0.Extent[1]*coeff[2][0]);
	rb = FABS(box1.Extent[0]*coeff[1][2])+FABS(box1.Extent[1]*coeff[0][2]);

	rsum = ra+rb;
	u0 = d0[1]*coeff[2][0]-d0[0]*coeff[2][1];
	if ((u0 > rsum) || (u0 < -rsum))
		return itA2B2;

	return itIntersects;
}

