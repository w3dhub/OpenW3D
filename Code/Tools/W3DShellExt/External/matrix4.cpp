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
 *                 Project Name : WW3D PS2                                                     *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/W3DShellExt/External/matrix4.cpp       $*
 *                                                                                             *
 *                   Programmer : Kenny Mitchell															  *
 *																															  *
 *                   Start Date : 11/16/99																	  *
 *																															  *
 *                  Last Update : 11/16/99																	  *
 *																															  *
 *---------------------------------------------------------------------------------------------*
 * Based on Greg Hjelstrom 97 																					  *
 * Functions:                                                                                  *
 *   Matrix4::Multiply -- Multiply two Matrix4's together                                      *
 *   Matrix4::Multiply -- Multiply a Matrix3D * Matrix4                                        *
 *   Matrix4::Multiply -- Multiply a Matrix4 * Matrix3D                                        *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include <assert.h>

#include "matrix4.h"


#if 0
/***********************************************************************************************
 * Matrix4::Multiply -- Multiply two Matrix4's together                                        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *  a - first operand                                                                          *
 *  b - second operand                                                                         *
 *  res - pointer to matrix to store the result in (must not point to a or b)                  *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/13/99   gth : Created.                                                                 *
 *=============================================================================================*/
void Matrix4::Multiply(const Matrix4 &a,const Matrix4 &b,Matrix4 * res)
{
	assert(res != &a);
	assert(res != &b);

	#define ROWCOL(i,j) a[i][0]*b[0][j] + a[i][1]*b[1][j] + a[i][2]*b[2][j] + a[i][3]*b[3][j]

	(*res)[0][0] = ROWCOL(0,0);
	(*res)[0][1] = ROWCOL(0,1);
	(*res)[0][2] = ROWCOL(0,2);
	(*res)[0][3] = ROWCOL(0,3);

	(*res)[1][0] = ROWCOL(1,0);
	(*res)[1][1] = ROWCOL(1,1);
	(*res)[1][2] = ROWCOL(1,2);
	(*res)[1][3] = ROWCOL(1,3);

	(*res)[2][0] = ROWCOL(2,0);
	(*res)[2][1] = ROWCOL(2,1);
	(*res)[2][2] = ROWCOL(2,2);
	(*res)[2][3] = ROWCOL(2,3);

	(*res)[3][0] = ROWCOL(3,0);
	(*res)[3][1] = ROWCOL(3,1);
	(*res)[3][2] = ROWCOL(3,2);
	(*res)[3][3] = ROWCOL(3,3);

	#undef ROWCOL
}


/***********************************************************************************************
 * Matrix4::Multiply -- Multiply a Matrix3D * Matrix4                                          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/13/99   gth : Created.                                                                 *
 *=============================================================================================*/
void Matrix4::Multiply(const Matrix3D &a,const Matrix4 &b,Matrix4 * res)
{
	assert(res != &b);

	#define ROWCOL(i,j) a[i][0]*b[0][j] + a[i][1]*b[1][j] + a[i][2]*b[2][j] + a[i][3]*b[3][j]

	(*res)[0][0] = ROWCOL(0,0);
	(*res)[0][1] = ROWCOL(0,1);
	(*res)[0][2] = ROWCOL(0,2);
	(*res)[0][3] = ROWCOL(0,3);

	(*res)[1][0] = ROWCOL(1,0);
	(*res)[1][1] = ROWCOL(1,1);
	(*res)[1][2] = ROWCOL(1,2);
	(*res)[1][3] = ROWCOL(1,3);

	(*res)[2][0] = ROWCOL(2,0);
	(*res)[2][1] = ROWCOL(2,1);
	(*res)[2][2] = ROWCOL(2,2);
	(*res)[2][3] = ROWCOL(2,3);

	(*res)[3][0] = b[3][0];				// last row of a is 0,0,0,1
	(*res)[3][1] = b[3][1];				// this leaves the last row of b unchanged
	(*res)[3][2] = b[3][2];
	(*res)[3][3] = b[3][3];

	#undef ROWCOL
}


/***********************************************************************************************
 * Matrix4::Multiply -- Multiply a Matrix4 * Matrix3D                                          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *=============================================================================================*/
void Matrix4::Multiply(const Matrix4 & a,const Matrix3D & b,Matrix4 * res)
{
	assert(res != &a);

	// ROWCOL multiplies a row of 'a' by one of the first three columns of 'b' (4th entry in b is zero)
	// ROWCOL4 multiplies a row of 'a' by the fourth column of 'b' (4th entry in b is one)

	#define ROWCOL(i,j) a[i][0]*b[0][j] + a[i][1]*b[1][j] + a[i][2]*b[2][j]
	#define ROWCOL4(i,j) a[i][0]*b[0][j] + a[i][1]*b[1][j] + a[i][2]*b[2][j] + a[i][3]

	(*res)[0][0] = ROWCOL(0,0);
	(*res)[0][1] = ROWCOL(0,1);
	(*res)[0][2] = ROWCOL(0,2);
	(*res)[0][3] = ROWCOL4(0,3);

	(*res)[1][0] = ROWCOL(1,0);
	(*res)[1][1] = ROWCOL(1,1);
	(*res)[1][2] = ROWCOL(1,2);
	(*res)[1][3] = ROWCOL4(1,3);

	(*res)[2][0] = ROWCOL(2,0);
	(*res)[2][1] = ROWCOL(2,1);
	(*res)[2][2] = ROWCOL(2,2);
	(*res)[2][3] = ROWCOL4(2,3);

	(*res)[3][0] = ROWCOL(3,0);
	(*res)[3][1] = ROWCOL(3,1);
	(*res)[3][2] = ROWCOL(3,2);
	(*res)[3][3] = ROWCOL4(3,3);

	#undef ROWCOL
	#undef ROWCOL4
}

/***********************************************************************************************
 * Set_Perspective -- Sets perspective matrix                                                 *
// Desc: Sets the passed in 4x4 matrix to a perpsective projection matrix built
//       from the field-of-view (fov, in y), aspect ratio, near plane (D),
//       and far plane (F).
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/16/99  KJM: Created.                                                                   *
 *=============================================================================================*/
void Matrix4::Set_Perspective
(
	Matrix4* m,
	float fFOV,
	float ScreenWidth,
	float ScreenHeight,
	float fNearPlane,
	float fFarPlane
)
{
	float w;
	float h;
	float Q,Q2;

	Q=DEGTORAD*0.5f;

	float c = cosf(fFOV*Q);
	float s = sinf(fFOV*Q);

	//printf("sincos %f %f\n",c,s);
	Q=0.000244140625f; // 1/4096

	w =  (ScreenWidth*Q);
	h =  (ScreenHeight*Q);

	Q = s/(1.0f - (fNearPlane/fFarPlane));
	Q2 = -Q*fNearPlane;

	m->Make_Identity();

	m->Row[0][0]= c*w;
	m->Row[1][1]= -c*h;
	m->Row[2][2]= Q;
	m->Row[2][3]= s;//1.0f;
	m->Row[3][2]= Q2;
}


void Matrix4::Set_View_Matrix
(
	Matrix4* m,
	Vector3& vFrom,
	Vector3& vAt,
	Vector3& vWorldUp
)
{
	Vector3 vView;
	Vector3 vUp;
	Vector3 vRight;
	float inv_length;
	float dot_product;

	// Get the z basis vector, which points straight ahead. This is the
	// difference from the eyepoint to the lookat point.
	vView=vAt-vFrom;

	inv_length=Inv_Sqrt(vView.Length2());

	// Normalize the z basis vector
	vView*=inv_length;

	// Get the dot product, and calculate the projection of the z basis
	// vector onto the up vector. The projection is the y basis vector.
	dot_product=Vector3::Dot_Product(vWorldUp,vView);

	// vUp = vWorldUp - fDotProduct * vView
	vUp=vWorldUp-(dot_product*vView);

	inv_length=Inv_Sqrt(vUp.Length2());

	// Normalize the y basis vector
	vUp*=inv_length;

	// The x basis vector is found simply with the cross product of the y
	// and z basis vectors
	Vector3::Cross_Product(vUp,vView,&vRight);

	// Start building the matrix. The first three rows contains the basis
	// vectors used to rotate the view to point at the lookat point
	m->Row[0][0] = vRight.x;    m->Row[0][1] = vUp.x;    m->Row[0][2] = vView.x; m->Row[0][3] = 0.0f;
	m->Row[1][0] = vRight.y;    m->Row[1][1] = vUp.y;    m->Row[1][2] = vView.y; m->Row[1][3] = 0.0f;
	m->Row[2][0] = vRight.z;    m->Row[2][1] = vUp.z;    m->Row[2][2] = vView.z; m->Row[2][3] = 0.0f;

	// Do the translation values (rotations are still about the eyepoint)
	m->Row[3][0] = - (vFrom * vRight);
	m->Row[3][1] = - (vFrom * vUp);
	m->Row[3][2] = - (vFrom * vView);
	m->Row[3][3] = 1.0f;
}


void Matrix4::Print_Matrix() const
{
	printf("%f %f %f %f\n",Row[0][0],Row[0][1],Row[0][2],Row[0][3]);
	printf("%f %f %f %f\n",Row[1][0],Row[1][1],Row[1][2],Row[1][3]);
	printf("%f %f %f %f\n",Row[2][0],Row[2][1],Row[2][2],Row[2][3]);
	printf("%f %f %f %f\n",Row[3][0],Row[3][1],Row[3][2],Row[3][3]);
}

#endif
