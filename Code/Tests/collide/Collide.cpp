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

#include <math.h>
#include "matrix3d.h"
#include "vector3.h"
#include "col.h"
#include "cvec.h"
#include "mpu.h"


void benchmark_transformations(void);

//---------------------------------------------------------------------------



#include <fstream.h>
#include <stdlib.h>
#define FRAND (-1.0f+2.0f*rand()/float(RAND_MAX))
ofstream ostr("data1.txt");
void Rotate3D (float line[3], float angle, float rotate[3][3])
{
	int row, col, mid;
	float I[3][3], A[3][3], A2[3][3];
    float sn, omcs;

    /* identity matrix */
    I[0][0] = 1;  I[0][1] = 0;  I[0][2] = 0;
    I[1][0] = 0;  I[1][1] = 1;  I[1][2] = 0;
    I[2][0] = 0;  I[2][1] = 0;  I[2][2] = 1;

    /* infinitesimal rotation about line */
    A[0][0] = 0;         A[0][1] = +line[2];  A[0][2] = -line[1];
    A[1][0] = -line[2];  A[1][1] = 0;         A[1][2] = +line[0];
    A[2][0] = +line[1];  A[2][1] = -line[0];  A[2][2] = 0;

    /* A2 = A*A */
    for (row = 0; row < 3; row++)
    	for (col = 0; col < 3; col++) {
        	A2[row][col] = 0;
            for (mid = 0; mid < 3; mid++)
                A2[row][col] += A[row][mid]*A[mid][col];
        }

    sn = float(sin(angle));
    omcs = float(1.0-cos(angle));

    /* rotation is I+sin(angle)*A+[1-cos(angle)]*A*A */
    for (row = 0; row < 3; row++)
	   	for (col = 0; col < 3; col++)
	       	rotate[row][col] = I[row][col]+sn*A[row][col]+omcs*A2[row][col];
}

void main ()
{

	Box box0, box1;

	// create box0
	box0.center[0] = 0.0f;
	box0.center[1] = 0.0f;
	box0.center[2] = 0.0f;
	
	box0.basis[0][0] = 1.0f;
	box0.basis[0][1] = 0.0f;
	box0.basis[0][2] = 0.0f;
	box0.basis[1][0] = 0.0f;
	box0.basis[1][1] = 1.0f;
	box0.basis[1][2] = 0.0f;
	box0.basis[2][0] = 0.0f;
	box0.basis[2][1] = 0.0f;
	box0.basis[2][2] = 1.0f;

	box0.extent[0] = 4.0f;
	box0.extent[1] = 4.0f;
	box0.extent[2] = 4.0f;

	box0.velocity[0] = 0.0f;
	box0.velocity[1] = 0.0f;
	box0.velocity[2] = 0.0f;


	// create box1
	box1.center[0] = 0.0f;
	box1.center[1] = -10.0f;
	box1.center[2] = 20.0f;
	
	float line[3] = { 0.0f, 0.707f, 0.707f };
	float length = float(sqrt(line[0]*line[0]+line[1]*line[1]+line[2]*line[2]));
	line[0] /= length;
	line[1] /= length;
	line[2] /= length;
	float angle = (float)DEG_TO_RAD(45.0f);
	Rotate3D(line,angle,box1.basis);
/*
	box1.basis[0][0] = 1.0f;
	box1.basis[0][1] = 0.0f;
	box1.basis[0][2] = 0.0f;
	box1.basis[1][0] = 0.0f;
	box1.basis[1][1] = 1.0f;
	box1.basis[1][2] = 0.0f;
	box1.basis[2][0] = 0.0f;
	box1.basis[2][1] = 0.0f;
	box1.basis[2][2] = 1.0f;
*/
	box1.extent[0] = 10.0f;
	box1.extent[1] = 4.0f;
	box1.extent[2] = 4.0f;

	box1.velocity[0] = 0.0f;
	box1.velocity[1] = 0.0f;
	box1.velocity[2] = 0.0f;

	BoxClass mybox0(box0);
	BoxClass mybox1(box1);
	unsigned int high;
	unsigned int cycles0;
	unsigned int cycles1;
	unsigned int cycles2;

	while (box1.center[2] > -20.0f) {

		cycles0 = Get_CPU_Clock(high);
		IntersectType type = BoxesIntersect(1.0f,box0,box1);
		cycles0 = Get_CPU_Clock(high) - cycles0;

		cycles1 = Get_CPU_Clock(high);
		IntersectType mytype = Boxes_Intersect(mybox0,mybox1,1.0f);
		cycles1 = Get_CPU_Clock(high) - cycles1;

		cycles2 = Get_CPU_Clock(high);
		IntersectType mytype2 = Boxes_Intersect(mybox0,mybox1);
		cycles2 = Get_CPU_Clock(high) - cycles2;

		cout << cycles0 << "   "<< cycles1 << "   " << cycles2 << "   " << type << "  " << mytype<< "  " << mytype2 << endl;
		
		box1.center[2] -= 1.0f;
		mybox1.Center[2] -= 1.0f;

	}
	
//	if (type == itIntersects) {
//		ostr << "type = " << type << endl;
//	}

	benchmark_transformations();

#if 0


	float line[3] = { FRAND, FRAND, FRAND };
	float length = float(sqrt(line[0]*line[0]+line[1]*line[1]+line[2]*line[2]));
	line[0] /= length;
	line[1] /= length;
	line[2] /= length;
	float angle = FRAND;
	Rotate3D(line,angle,box0.basis);

	box0.extent[0] = 1.0f;
	box0.extent[1] = 1.0f;
	box0.extent[2] = 1.0f;
	box0.velocity[0] = 0.0f;
	box0.velocity[1] = 0.0f;
	box0.velocity[2] = 0.0f;

	for (int i = 0; i < 100; i++)
	{
		box1.center[0] = 2.0f;
		box1.center[1] = 0.0f;
		box1.center[2] = 0.0f;

		line[0] = FRAND;
		line[1] = FRAND;
		line[2] = FRAND;
		length = float(sqrt(line[0]*line[0]+line[1]*line[1]+line[2]*line[2]));
		line[0] /= length;
		line[1] /= length;
		line[2] /= length;
		angle = FRAND;
		Rotate3D(line,angle,box1.basis);

		box1.extent[0] = 1.0f;
		box1.extent[1] = 1.0f;
		box1.extent[2] = 1.0f;
		box1.velocity[0] = 0.0f;
		box1.velocity[1] = 0.0f;
		box1.velocity[2] = 0.0f;

		IntersectType type = BoxesIntersect(1.0f,box0,box1);
		ostr << "i = " << i << ' ' << "type = " << type << endl;
	}
#endif

	
	/*
	** box0
	*/
	box0.center[0] = 6.1978f;
	box0.center[1] = 2.6640f;
	box0.center[2] = 0.840f;
	
	box0.extent[0] = 0.1341f;
	box0.extent[1] = 0.320672f;
	box0.extent[2] = 0.840f;

	box0.velocity[0] = box0.velocity[1] = box0.velocity[2] = 0.0f;

	box0.basis[0][0] = 0.857709f;
	box0.basis[0][1] = -0.514136f;
	box0.basis[0][2] = 0.0f;

	box0.basis[1][0] = 0.514136f;
	box0.basis[1][1] = 0.857709f;
	box0.basis[1][2] = 0.0f;
	
	box0.basis[2][0] = 0.0f;
	box0.basis[2][1] = 0.0f;
	box0.basis[2][2] = 1.0f;

	/*
	** box1
	*/
	box1.center[0] = 10.0f;
	box1.center[1] = 4.0f;
	box1.center[2] = 2.8f;

	box1.extent[0] = 4.5f;
	box1.extent[1] = 1.966f;
	box1.extent[2] = 2.868f;

	box1.velocity[0] = box1.velocity[1] = box1.velocity[2] = 0.0f;

	box1.basis[0][0] = 1.0f;
	box1.basis[0][1] = 0.0f;
	box1.basis[0][2] = 0.0f;
	box1.basis[1][0] = 0.0f;
	box1.basis[1][1] = 1.0f;
	box1.basis[1][2] = 0.0f;
	box1.basis[2][0] = 0.0f;
	box1.basis[2][1] = 0.0f;
	box1.basis[2][2] = 1.0f;

	IntersectType type = BoxesIntersect(1.0f,box0,box1);


}



void benchmark_transformations(void)
{ 
	unsigned int high;
	unsigned int cycles0;
	unsigned int cycles1;

	/*
	** Testing speed of the matrix library...
	*/
	Vector v = { 4.0f, -2.5f, 100.4f };
	float mat[3][3];

	float line[3] = { 0.0f, 0.0f, 1.0f };
	float length = float(sqrt(line[0]*line[0]+line[1]*line[1]+line[2]*line[2]));
	line[0] /= length;
	line[1] /= length;
	line[2] /= length;
	float angle = (float)DEG_TO_RAD(45.0f);
	Rotate3D(line,angle,mat);


	Vector3 myv(4.0,-2.5,100.4);
	Matrix3 mymat;

	mymat[0][0] = mat[0][0];
	mymat[0][1] = mat[0][1];
	mymat[0][2] = mat[0][2];

	mymat[1][0] = mat[1][0];
	mymat[1][1] = mat[1][1];
	mymat[1][2] = mat[1][2];

	mymat[2][0] = mat[2][0];
	mymat[2][1] = mat[2][1];
	mymat[2][2] = mat[2][2];

	Vector res;
	Vector3 myres;

	cycles0 = Get_CPU_Clock(high);
	MultiplyVM (v,mat,res);
	cycles0 = Get_CPU_Clock(high) - cycles0;

	cycles1 = Get_CPU_Clock(high);
	myres = mymat * myv;
	cycles1 = Get_CPU_Clock(high) - cycles1;

	cout << "c cycles = " << cycles0 << endl;
	cout << "c++ cycles = " << cycles1 << endl;
}

