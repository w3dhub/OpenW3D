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
 *                     $Archive:: /Commando/Code/Tests/mathtest/main.cpp                      $*
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 10/03/01 12:52p                                             $*
 *                                                                                             *
 *                    $Revision:: 47                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "matrix4.h"
#include "matrix3d.h"
#include "vector3.h"
#include "euler.h"
#include "ode.h"
#include "obbox.h"
#include "plane.h"
#include "colmath.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#include "p_timer.h"
#include "output.h"
#include "odetest.h"
#include "raytest.h"
#include "aaboxtest.h"
#include "obboxtest.h"
#include "uarraytest.h"
#include "lineseg.h"
#include "mempooltest.h"
#include "tri.h"
#include "frustum.h"
#include "jan_math.h"

#include <windows.h>

const double TOLERANCE = 0.00001;
static int _Global;

void test_lookat(void);
void test_vectors(void);
void test_eulers(void);
void test_matrix3(void);
void test_ode_systems(void);
void test_quaternions(void);
void test_planes(void);
void test_animation(void);
void test_concatenation(void);
void test_sphere_intersection(void);
void test_aabox_transform(void);
void test_vector_quick_length(void);
void test_sqrt_time(void);
void test_windows(void);
void test_point_containment(void);
void test_fast_trig(void);

float gaussian_function(float x);
float integrate(float (*f)(float),float start,float end,int slices);

Matrix3D random_matrix(void);
void print_matrix(const Matrix3D & m);
void Transform_Min_Max_AABox_Brute_Force(const Matrix3D & tm,const Vector3 & min,const Vector3 & max,
													  Vector3 &	newmin,Vector3 & newmax);
void Bug_Function(const Matrix3D & A,const Matrix3D & B,Matrix3D * set_res);


/*

  This really needs to become a rigorous test of the math library...
  So far, I've only added random stuff as I find bugs...

*/
void main(void)
{
	WWMath::Init();

	Matrix4 tm(1);
	tm[0][3] = 1.0f;
	tm[1][3] = 2.0f;
	tm[2][3] = 3.0f;
	Matrix4 invtm = tm.Inverse();

	float area = integrate(gaussian_function,-10.0f,10.0f,1000);
	float e = exp(1.0f);

#if 0
	Vector3 array[3];
	
	printf("%f",array[2].Y);
	array[1] = Vector3(1,1,1);
	array[2].X = 5.0f;

	for (int i=0; i<3; i++) {
		array[i].Set(rand() & 0xFF,rand() & 0xFF,rand() & 0xFF);
	}
#endif

#if 0
	PlaneClass plane;
	plane.N.Set(rand() & 0xFF,rand() & 0xFF,rand() & 0xFF);
	plane.D = rand() & 0xFF;
	
	if (plane.In_Front(Vector3(5,5,5))) {
		printf("hi");
	} else {
		printf("a");
	}
#endif

	test_fast_trig();

#if 0
	Test_AABoxes();

	test_point_containment();
	Test_Rays();
	test_mempool();
	Test_OBBoxes();
	test_lookat();
	test_vectors();
	test_eulers();
	test_matrix3();
	test_quaternions();
	test_animation();
	test_ode_systems();
	
	Test_Rays();
	test_planes();
	test_concatenation();
	
	Test_Uarray();

	test_sphere_intersection();
	test_aabox_transform();
	test_vector_quick_length();
	test_sqrt_time();
#endif

	WWMath::Shutdown();
}

void test_fast_trig(void)
{
	Print_Title("Testing table-based trig code.");

	float max_sin_error = 0.0f;
	float max_cos_error = 0.0f;
	float max_asin_error = 0.0f;
	float max_acos_error = 0.0f;

	const float STEPS=32000.0f;
	float START = -8.0f * WWMATH_PI;
	float STOP = 8.0f * WWMATH_PI;
	float error;
	
	for (float test=START; test < STOP; test+=(STOP-START) / STEPS) {
		
		error = WWMath::Fabs(WWMath::Sin(test) - WWMath::Fast_Sin(test));
		if (error > max_sin_error) {
			max_sin_error = error;
		}

		error = WWMath::Fabs(WWMath::Cos(test) - WWMath::Fast_Cos(test));
		if (error > max_cos_error) {
			max_cos_error = error;
		}
	}

	START=-1.0f;
	STOP=1.0f;

	for (test=START; test<STOP; test+=(STOP-START)/STEPS) {
		error = WWMath::Fabs(WWMath::Asin(test) - WWMath::Fast_Asin(test));
		if (error > max_asin_error) {
			max_asin_error = error;
		}

		error = WWMath::Fabs(WWMath::Acos(test) - WWMath::Fast_Acos(test));
		if (error > max_acos_error) {
			max_acos_error = error;
		}

	}

	printf("max cos error: %f\r\n",max_cos_error);
	printf("max sin error: %f\r\n",max_sin_error);
	printf("max acos error: %f\r\n",max_acos_error);
	printf("max asin error: %f\r\n",max_asin_error);

}

void test_vectors(void)
{
	Print_Title("Testing matrix*vector code.");

	/*
	** Matrix * Vector (this was demonstrating a compiler bug...)
	*/
	Vector3 vec(5.0,0.0,0.0);
	Matrix3D mat(1);

	Vector3 tvec = mat*vec;
	CHECK(0,((tvec - vec).Length() < TOLERANCE));

	/*
	** Timing the difference between operator + and Vector3::Add
	*/
	Vector3 tab[16000];
	for (int i=0; i<16000; i++) {
		tab[i].Set(WWMath::Random_Float(),WWMath::Random_Float(),WWMath::Random_Float());
	}

	Vector3 c;

	unsigned int op_cycles = Get_CPU_Clock();
	for (i=0; i<16000; i++) {
		c = tab[0] + tab[i];
	}
	op_cycles = Get_CPU_Clock() - op_cycles;	
	c.Normalize();
	
	printf("Vector3::operator + cycles: %d\n",op_cycles);

	unsigned int add_cycles = Get_CPU_Clock();
	for (i=0; i<16000; i++) {
		Vector3::Add(tab[0],tab[i],&c);
	}
	add_cycles = Get_CPU_Clock() - add_cycles;
	c.Normalize();

	printf("Vector3::Add cycles: %d\n",add_cycles);
	printf("Operator/Add ration: %f\n",(float)op_cycles / (float)add_cycles);
}


void test_lookat(void)
{
	int check = 0;
	Print_Title("Testing matrix look-at code.");

	Matrix3D tm(1);
	Matrix3D invtm;

	tm.Look_At(Vector3(0,0,0),Vector3(-1,0,0),0); 
	Quaternion q = Build_Quaternion(tm);

	CHECK(check++,(fabs(q[0] - 0.5) < TOLERANCE));

	Vector3 pos;
	Vector3 tar;
	Vector3 test;

	pos.Set(0,0,0);
	tar.Set(30.0f,10.0f,3.0f);
	tm.Look_At(pos,tar,0.0f);
	tm.Get_Orthogonal_Inverse(invtm);
	test = invtm * tar;
	CHECK(check++,(Vector2(test.X,test.Y).Length() < 0.001f));

	tar.Set(1,-2,1);
	tm.Obj_Look_At(pos,tar,0.0f);
	tm.Get_Orthogonal_Inverse(invtm);
	test = invtm * tar;
	CHECK(check++,(Vector2(test.Y,test.Z).Length() < 0.001f));

}

void test_eulers(void)
{
	Print_Title("Testing euler angle conversion code.");

	/*
	** Testing Euler angles
	*/
	Matrix3D mat;
	float xrot,yrot,zrot;

	mat.Rotate_X(0.3f);
	mat.Rotate_Z(0.2f);
	mat.Rotate_Y(0.1f);
	EulerAnglesClass euler(mat,EulerOrderYXYr);
	euler.To_Matrix(mat);

	Matrix3D mat2(1);
	mat2.Rotate_Y(euler.Get_Angle(0));
	mat2.Rotate_X(euler.Get_Angle(1));
	mat2.Rotate_Y(euler.Get_Angle(2));

	CHECK(0,(((mat2[0] - mat[0]).Length() < TOLERANCE) && 
				((mat2[1] - mat[1]).Length() < TOLERANCE) && 
				((mat2[2] - mat[2]).Length() < TOLERANCE)));

	mat.Make_Identity();
	mat.Rotate_Y(DEG_TO_RADF(12.0f));
	mat.Rotate_Z(DEG_TO_RADF(25.0f));
	EulerAnglesClass euler2(mat,EulerOrderXYZr);
	xrot = euler2.Get_Angle(0);
	yrot = euler2.Get_Angle(1);
	zrot = euler2.Get_Angle(2);
	
	mat2.Make_Identity();
	mat2.Rotate_X(xrot);
	mat2.Rotate_Y(yrot);
	mat2.Rotate_Z(zrot);

	CHECK(0,(((mat2[0] - mat[0]).Length() < TOLERANCE) && 
				((mat2[1] - mat[1]).Length() < TOLERANCE) && 
				((mat2[2] - mat[2]).Length() < TOLERANCE)));
}	

void test_matrix3(void)
{
	Print_Title("Testing Matrix3 Inversion");
	
	/*
	** Testing Matrix3 Inversion function
	*/
	Matrix3 mat;

	mat[0][0] =  6.0f; mat[0][1] =  4.5f; mat[0][2] = -1.2f;
	mat[1][0] = -3.0f; mat[1][1] =  0.0f; mat[1][2] = -1.3f;
	mat[2][0] =  2.1f; mat[2][1] =  1.5f; mat[2][2] = 17.2f;

	Matrix3 invmat = mat.Inverse();

	Vector3 a(10.0, 20.0, 30.0);
	Vector3 b = mat * a;
	Vector3 c = invmat * b;

	CHECK(0,((a-c).Length() < TOLERANCE));

}


void test_ode_systems(void)
{
	Print_Title("Testing ODE Integrators");

	/*
	** Testing the new-style ODE solvers
	**
	** Suppose we have a point which is going to move around
	** a 2-D circle. 
	*/
	int i;
	float error = 0.0f;
	float maxerror[4] = {0,0,0,0};
	ODETestClass circle_system;

	circle_system.Point.Set(1,0,0);
	for (i=0; i<100; i++) {
		
		IntegrationSystem::Euler_Integrate(&circle_system,1.0);
		
		error = circle_system.Point.Length() - 1.0;
		if (fabs(error) > maxerror[0]) {
			maxerror[0] = fabs(error);
		}
	}
	Print("Euler max error = %f\n",maxerror[0]);

	circle_system.Point.Set(1,0,0);
	for (i=0; i<100; i++) {
		
		IntegrationSystem::Midpoint_Integrate(&circle_system,1.0);

		error = circle_system.Point.Length() - 1.0;
		if (fabs(error) > maxerror[1]) {
			maxerror[1] = fabs(error);
		}
	}
	Print("Midpoint max error = %f\n",maxerror[1]);

	circle_system.Point.Set(1,0,0);
	for (i=0; i<100; i++) {
		
		IntegrationSystem::Runge_Kutta_Integrate(&circle_system,1.0);

		error = circle_system.Point.Length() - 1.0;
		if (fabs(error) > maxerror[2]) {
			maxerror[2] = fabs(error);
		}
	}
	Print("Runge_Kutta max error = %f\n",maxerror[2]);

	circle_system.Point.Set(1,0,0);
	for (i=0; i<100; i++) {
		
		IntegrationSystem::Runge_Kutta5_Integrate(&circle_system,1.0);

		error = circle_system.Point.Length() - 1.0;
		if (fabs(error) > maxerror[3]) {
			maxerror[3] = fabs(error);
		}
	}
	Print("Runge_Kutta5 max error = %f\n",maxerror[3]);

}

void test_quaternions(void)
{
	Print_Title("Testing Quaternions");
	int ci = 0;

	Quaternion q,p,pnew;
	Matrix3 tm;
	Vector3 vnew;
	
	// Testing rotation about the Z axis
	q = Axis_To_Quat(Vector3(0,0,1),DEG_TO_RAD(45.0f));
	p.Set(1.0,0.0,0.0,0.0);
	
	pnew = q*p*Conjugate(q);

	vnew.Set(pnew.X,pnew.Y,pnew.Z);
	CHECK(ci++,((vnew - Vector3(0.707f,0.707f,0.0f)).Length() < 0.01f));

	tm = Build_Matrix3(q);
	vnew = tm * Vector3(p.X,p.Y,p.Z);
	CHECK(ci++,((vnew - Vector3(0.707f,0.707f,0.0f)).Length() < 0.01f));


	// Testing rotation about the Y axis
	q = Axis_To_Quat(Vector3(0,1,0),DEG_TO_RADF(45.0f));
	p.Set(0.0f,0.0f,1.0f,0.0f);
	
	pnew = q*p*Conjugate(q);

	vnew.Set(pnew.X,pnew.Y,pnew.Z);
	CHECK(ci++,((vnew - Vector3(0.707f,0.0f,0.707f)).Length() < 0.01f));

	tm = Build_Matrix3(q);
	vnew = tm * Vector3(p.X,p.Y,p.Z);
	CHECK(ci++,((vnew - Vector3(0.707f,0.0f,0.707f)).Length() < 0.01f));

	// Testing rotation about the X axis
	q = Axis_To_Quat(Vector3(1,0,0),DEG_TO_RADF(45.0f));
	p.Set(0.0f,1.0f,0.0f,0.0f);
	
	pnew = q*p*Conjugate(q);

	vnew.Set(pnew.X,pnew.Y,pnew.Z);
	CHECK(ci++,((vnew - Vector3(0.0f,0.707f,0.707f)).Length() < 0.01f));

	tm = Build_Matrix3(q);
	vnew = tm * Vector3(p.X,p.Y,p.Z);
	CHECK(ci++,((vnew - Vector3(0.0f,0.707f,0.707f)).Length() < 0.01f));


	// Testing Rigid Body Angular momentum:
	Vector3 testpoint(1.0,0.0,0.0);
	Vector3 newpoint;
	Quaternion orientation(0,0,0,1);
	Quaternion omega(0.0,0.0,DEG_TO_RAD(45.0f),0.0);

	// Euler integrate for 1 second at 45 deg/s
	for (int i=0; i<100; i++) {
		
		// deriviative of orientation:
		Quaternion dorient = 0.5 * omega * orientation;

		// dt
		dorient = 0.01f * dorient;

		// new orientation
		orientation = orientation + dorient;
	
		orientation.Normalize();
	}

	// transform test point (should be 0.707,0.707,0):
	tm = Build_Matrix3(orientation);
	newpoint = tm * testpoint;

	CHECK(ci++,((newpoint - Vector3(0.707f,0.707f,0.0f)).Length() < 0.01f));


	// Get ready for some speed tests
	unsigned cycles;
	Quaternion input_quats[256];
	Quaternion output_quat;
	Matrix3D input_tms[256];
	Vector3 input_vec(4.5f,-3.7f,2.3f);
	Quaternion input_quat(input_vec.X,input_vec.Y,input_vec.Z,0.0f);
	Vector3 output_vec;
	Matrix3D output_tm;

	for (i=0; i<256; i++) {
		input_quats[i].Randomize();
		input_tms[i] = Build_Matrix3D(input_quats[i]);
	}

	// test speed of quaternion multiplication:
	cycles = Get_CPU_Clock();
	for (i=0; i<256; i++) {
		output_quat = input_quats[i] * input_quats[(i+5) & 0xFF];
	}
	cycles = Get_CPU_Clock() - cycles;
	Print("quaternion multiply cycles:                %d\n",cycles / 256);

	// test speed of building a matrix from euler angles
	cycles = Get_CPU_Clock();
	for (i=0; i<256; i++) {
		output_tm.Make_Identity();
		output_tm.Rotate_Z(0.123f);
		output_tm.Rotate_Y(-0.342f);
		output_tm.Rotate_X(1.23f);
	}
	cycles = Get_CPU_Clock() - cycles;
	Print("building matrix cycles:                    %d\n",cycles / 256);
	

	// test speed of matrix-vector multiplication
	cycles = Get_CPU_Clock();
	for (i=0; i<256; i++) {
		output_vec = input_tms[i] * input_vec;
	}
	cycles = Get_CPU_Clock() - cycles;
	Print("matrix-vector cycles:                      %d\n",cycles / 256);

	// test speed of convert matrix, then matrix-vector multiplication
	cycles = Get_CPU_Clock();
	for (i=0; i<256; i++) {
		tm = Build_Matrix3D(input_quats[i]);
		output_vec = tm * input_vec;
	}
	cycles = Get_CPU_Clock() - cycles;
	Print("convert-matrix-vector cycles:              %d\n",cycles / 256);
	
	// test speed of naive quat-vector transform
	cycles = Get_CPU_Clock();
	for (i=0; i<256; i++) {
		output_quat = input_quats[i] * input_quat * Conjugate(input_quats[i]);
	}
	cycles = Get_CPU_Clock() - cycles;
	Print("naive quat-vector cycles:                  %d\n",cycles / 256);

	// test speed of quat-vector transform
	cycles = Get_CPU_Clock();
	for (i=0; i<256; i++) {
		output_vec = input_quats[i].Rotate_Vector(input_vec);
	}
	cycles = Get_CPU_Clock() - cycles;
	Print("fast quat-vector cycles:                   %d\n",cycles / 256);

	// test accuracy of the quat-vector transform
	Vector3 verify_vec;
	Quaternion tmp0,tmp1;
	bool success = true;

	for (i=0; i<256; i++) {
		output_vec = input_quats[i].Rotate_Vector(input_vec);
		verify_vec = Build_Matrix3D(input_quats[i]) * input_vec;
		
		tmp0.X = input_vec.X;
		tmp0.Y = input_vec.Y;
		tmp0.Z = input_vec.Z;
		tmp0.W = 0.0f;
		tmp1 = input_quats[i] * tmp0 * Conjugate(input_quats[i]);

		float delta = (output_vec - verify_vec).Length();
		if (delta > 0.001f) success = false;
	}
	CHECK(ci++,(success));
}



void test_animation(void)
{
	Print_Title("Testing animation speed");
	int ci = 0;
	
	///////////////////////////////////////////////////////////////////////
	// Animation calculation for a node consists of the following steps:
	//
	// - concatenate the root transform and the base pose transform
	// - grab the translation (possible lerp), concatenate it in
	// - grab the rotation (possible slerp), concatenate it in
	///////////////////////////////////////////////////////////////////////	
	int i;
	unsigned cycles;
	float percentage = 0.5f;
	
	Matrix3D root_tm;
	Matrix3D base_tm;
	Matrix3D tm;
	Matrix3D tmp;

	float euler_x0 = (float)DEG_TO_RAD(24.0f);
	float euler_y0 = (float)DEG_TO_RAD(30.0f);
	float euler_z0 = (float)DEG_TO_RAD(-12.0f);

	float euler_x1 = (float)DEG_TO_RAD(44.0f);
	float euler_y1 = (float)DEG_TO_RAD(23.0f);
	float euler_z1 = (float)DEG_TO_RAD(0.0f);

	float tx0 = 10.0f;
	float ty0 = -3.0f;
	float tz0 = 2.0f;

	float tx1 = 13.0f;
	float ty1 = -4.0f;
	float tz1 = 1.0f;

	Quaternion q0,q1;

	// initialization
	root_tm.Make_Identity();
	root_tm.Rotate_X(DEG_TO_RAD(12.0f));
	base_tm.Make_Identity();
	base_tm.Rotate_Z(DEG_TO_RAD(-3.0f));

	tmp.Make_Identity();
	tmp.Rotate_X(euler_x0);
	tmp.Rotate_Y(euler_y0);
	tmp.Rotate_Z(euler_z0);
	q0 = Build_Quaternion(tmp);

	tmp.Make_Identity();
	tmp.Rotate_X(euler_x1);
	tmp.Rotate_Y(euler_y1);
	tmp.Rotate_Z(euler_z1);
	q1 = Build_Quaternion(tmp);

	// anim method, Translation base and euler angles:
	cycles = Get_CPU_Clock();
	for (i=0; i<256; i++) {
		tm = root_tm;
		tm.Translate(tx0,ty1,tz0);

		Vector3 trans0(tx0,ty0,tz0);
		Vector3 trans1(tx1,ty1,tz1);
		tm.Translate((1.0 - percentage) * trans0 + (percentage) * trans1);

		tm.Rotate_X((1.0 - percentage) * euler_x0 + (percentage) * euler_x1);
		tm.Rotate_Y((1.0 - percentage) * euler_y0 + (percentage) * euler_y1);
		tm.Rotate_Z((1.0 - percentage) * euler_z0 + (percentage) * euler_z1);
	}
	cycles = Get_CPU_Clock() - cycles;
	printf("Translation Base + Euler animation cycles:            %d\n",cycles / 256);

	// anim method, Translation base and quaternions:
	cycles = Get_CPU_Clock();
	for (i=0; i<256; i++) {
		tm = root_tm;
		tm.Translate(tx0,ty1,tz0);

		Vector3 trans0(tx0,ty0,tz0);
		Vector3 trans1(tx1,ty1,tz1);
		tm.Translate((1.0 - percentage) * trans0 + (percentage) * trans1);

		Quaternion q;
		Slerp(q,q0,q1,percentage);
		Matrix3D::Multiply(tm,Build_Matrix3D(q),&tm);
	}
	cycles = Get_CPU_Clock() - cycles;
	printf("Translation Base + Quaternion animation cycles:       %d\n",cycles / 256);

	
	// anim method, Matrix3D base and euler angles:
	cycles = Get_CPU_Clock();
	for (i=0; i<256; i++) {
	
		Matrix3D::Multiply(root_tm,base_tm,&tm);

		Vector3 trans0(tx0,ty0,tz0);
		Vector3 trans1(tx1,ty1,tz1);
		tm.Translate((1.0 - percentage) * trans0 + (percentage) * trans1);

		tm.Rotate_X((1.0 - percentage) * euler_x0 + (percentage) * euler_x1);
		tm.Rotate_Y((1.0 - percentage) * euler_y0 + (percentage) * euler_y1);
		tm.Rotate_Z((1.0 - percentage) * euler_z0 + (percentage) * euler_z1);
	}
	cycles = Get_CPU_Clock() - cycles;
	printf("Matrix3D Base + Euler animation cycles:               %d\n",cycles / 256);


	// anim method, Matrix3D base and quaternions:
	cycles = Get_CPU_Clock();
	for (i=0; i<256; i++) {
	
		Matrix3D::Multiply(root_tm,base_tm,&tm);

		Vector3 trans0(tx0,ty0,tz0);
		Vector3 trans1(tx1,ty1,tz1);
		tm.Translate((1.0 - percentage) * trans0 + (percentage) * trans1);

		Quaternion q;
		Slerp(q,q0,q1,percentage);
		Matrix3D::Multiply(tm,Build_Matrix3D(q),&tm);
	}
	cycles = Get_CPU_Clock() - cycles;
	printf("Matrix3D Base + Quaternion animation cycles:          %d\n",cycles / 256);

	printf("\n");

}

void test_planes(void)
{
	int ci=0;

	Print_Title("Testing PlaneClass");
	PlaneClass p0(Vector3(-1,0,0),-1);
	PlaneClass p1(Vector3(1,0,0),0);
	PlaneClass p2(Vector3(1,0,0),5);
	PlaneClass p3(Vector3(1,0,0),-5);
	
	SphereClass s0(Vector3(0,0,0),0.5f);
	SphereClass s1(Vector3(0,0,0),1.1f);

	CHECK(ci++,(p0.In_Front(s0)));
	CHECK(ci++,!(p0.In_Front(s1)));
	CHECK(ci++,(p0.In_Front_Or_Intersecting(s1)));

	CHECK(ci++,!(p1.In_Front(s0)));
	CHECK(ci++,!(p1.In_Front(s1)));
	CHECK(ci++,(p1.In_Front_Or_Intersecting(s0)));
	CHECK(ci++,(p1.In_Front_Or_Intersecting(s1)));

	CHECK(ci++,!(p2.In_Front_Or_Intersecting(s0)));
	CHECK(ci++,!(p2.In_Front_Or_Intersecting(s1)));

}

void test_concatenation(void)
{
	Print_Title("Testing Matrix Concatenation Code");
	unsigned operator_cycles;
	unsigned concatenate_cycles;
	unsigned i;
	Matrix3D a,b;
	Matrix3D res1,res2;
	bool ok;

	a.Make_Identity();
	a.Rotate_X(20.0f);
	b.Make_Identity(); 
	b.Rotate_X(20.0f);
	
	res1 = a*b;
	print_matrix(res1);	
	
	// Result being placed into a third matrix
	printf("\nResult being placed into third matrix c = a*b\n\n");
	for (i=0; i<10; i++) {
		a = random_matrix();
		b = random_matrix();
	
		operator_cycles = Get_CPU_Clock();
		res1 = a*b;
		operator_cycles = Get_CPU_Clock() - operator_cycles;

		concatenate_cycles = Get_CPU_Clock();
		Matrix3D::Multiply(a,b,&res2);
		concatenate_cycles = Get_CPU_Clock() - concatenate_cycles;

		ok = ( (((res1[0] - res2[0]).Length()) < 0.001f) &&
				 (((res1[1] - res2[1]).Length()) < 0.001f) &&
				 (((res1[2] - res2[2]).Length()) < 0.001f) );

		printf("Test %3d   op_cycles: %d\tc_cycles: %d\t",i,operator_cycles,concatenate_cycles);
		if (ok) { 
			printf("passed\n");
		} else {
			printf("<<FAILED!>>\n");
		}
	}

	// result being placed into one of the operands.
	printf("\nResult being placed into one of the operands a = a*b\n\n");
	Matrix3D a2,b2;
	for (i=0; i<10; i++) {
		a = random_matrix();
		b = random_matrix();
		a2 = a;
		b2 = b;

		operator_cycles = Get_CPU_Clock();
		a = a*b;
		operator_cycles = Get_CPU_Clock() - operator_cycles;

		concatenate_cycles = Get_CPU_Clock();
		Matrix3D::Multiply(a2,b2,&a2);
		concatenate_cycles = Get_CPU_Clock() - concatenate_cycles;

		bool ok = ( (((res1[0] - res2[0]).Length()) < 0.001f) &&
					   (((res1[1] - res2[1]).Length()) < 0.001f) &&
					   (((res1[2] - res2[2]).Length()) < 0.001f) );
		printf("Test %3d   op_cycles: %d\tc_cycles: %d\t",i+10,operator_cycles,concatenate_cycles);
		if (ok) { 
			printf("passed\n");
		} else {
			printf("<<FAILED!>>\n");
		}
	}
}

void print_matrix(const Matrix3D & m)
{
	for (int row = 0;row < 3; row++) {
		printf("%f   %f   %f   %f\n",m[row][0],m[row][1],m[row][2],m[row][3]);
	}
	printf("\n");
}

Matrix3D random_matrix(void)
{	
	Quaternion q;
	q.X = WWMath::Random_Float();
	q.Y = WWMath::Random_Float();
	q.Z = WWMath::Random_Float();
	q.W = WWMath::Random_Float() + 0.0001f;
	q.Normalize();

	Matrix3D m = Build_Matrix3D(q);
	m.Set_Translation(Vector3(WWMath::Random_Float(),WWMath::Random_Float(),WWMath::Random_Float()));

	return m;
}

void test_sphere_intersection(void)
{
	Print_Title("Testing Sphere-LineSegment Intersection Code");
	SphereClass sphere(Vector3(0,0,0),1.0f);
	LineSegClass seg(Vector3(2,1,0),Vector3(0,0,0));
	CastResultStruct res;

	if (CollisionMath::Collide(seg,sphere,&res)) {
		printf("Test 1 Passed\n");
	} else {
		printf("Test 1 Failed\n");
	}
}

void test_aabox_transform(void)
{
	int ci = 0;
	Print_Title("Testing AABox Transformation");

	Vector3 vmin(-10,-10,-10);
	Vector3 vmax(10,10,10);
	Matrix3D tm(Matrix3D::RotateZ90);

	Vector3 newmin,newmax;
	tm.Transform_Min_Max_AABox(vmin,vmax,&newmin,&newmax);

	CHECK(ci++,	((newmin - Vector3(-10,-10,-10)).Length() < TOLERANCE) && 
					((newmax - Vector3(10,10,10)).Length() < TOLERANCE));

	vmin.Set(-1,-10,-1);
	vmax.Set(1,10,1);
	tm = Matrix3D::RotateZ90;	
	tm.Transform_Min_Max_AABox(vmin,vmax,&newmin,&newmax);

	CHECK(ci++,	((newmin - Vector3(-10,-1,-1)).Length() < TOLERANCE) && 
					((newmax - Vector3(10,1,1)).Length() < TOLERANCE));


	vmin.Set(0,0,0);
	vmax.Set(5,3,2);
	tm.Make_Identity();
	tm.Rotate_Z(DEG_TO_RAD(45.0));
	tm.Transform_Min_Max_AABox(vmin,vmax,&newmin,&newmax);

	Vector3 center(0,0,0);
	Vector3 extent(1,1,1);
	Vector3 newcenter,newextent;
	tm.Make_Identity();
	tm.Rotate_Z(DEG_TO_RAD(45.0f));
	tm.Transform_Center_Extent_AABox(center,extent,&newcenter,&newextent);
}


void test_vector_quick_length(void)
{
	const int LONGITUDE_SAMPLES = 64;
	const int LATITUDE_SAMPLES = 32;

	float error;
	float max_error = 0.0f;
	float avg_error = 0.0f;

	double longitude;
	double latitude;

	for (int long_index=0; long_index<LONGITUDE_SAMPLES; long_index++) {
		
		longitude = 2*WWMATH_PI*((float)long_index/(float)LONGITUDE_SAMPLES);

		for (int lat_index=0; lat_index<LATITUDE_SAMPLES; lat_index++) {

			latitude = -WWMATH_PI/2.0f + WWMATH_PI*((float)lat_index/(float)LATITUDE_SAMPLES);
			
			Matrix3D tm(1);
			tm.Rotate_Z(longitude);
			tm.Rotate_Y(latitude);
			Vector3 vec = tm * Vector3(1,0,0);

			float real_length = vec.Length();
			float fake_length = vec.Quick_Length();

			error = fabs(fake_length - real_length);
			avg_error += error;
			if (error > max_error) {
				max_error = error;
			}
		}
	}

	avg_error = avg_error / (LONGITUDE_SAMPLES * LATITUDE_SAMPLES);
	printf("Vector3::Quick_Length Average Error: %f\n",avg_error);
}

void test_sqrt_time(void)
{
	Print_Title("Timing some built-in functions");

	int i;
	unsigned int time;
	const int SAMPLES = 100;
	float results[SAMPLES];

	time = 0;
	for (i=0; i<SAMPLES; i++) {
		float f = WWMath::Random_Float() * 20000.0f;
		unsigned int cycles = Get_CPU_Clock();
		float val = sqrt(f);	
		val+=sqrt(val);
		time += Get_CPU_Clock() - cycles;
		results[i] = val;
	}
	for (i=0; i<SAMPLES; i++) _Global+= results[i];		// keeping the compiler from optimizing this away
	printf("sqrt average cycles: %d\n",time / SAMPLES);

	time = 0;
	for (i=0; i<SAMPLES; i++) {
		float f = WWMath::Random_Float() * 6.28f;
		unsigned int cycles = Get_CPU_Clock();
		float val = sin(f);	
		time += Get_CPU_Clock() - cycles;
		results[i] = val;
	}
	for (i=0; i<SAMPLES; i++) _Global+= results[i];
	printf("sin average cycles: %d\n",time / SAMPLES);

	time = 0;
	for (i=0; i<SAMPLES; i++) {
		float f = WWMath::Random_Float() * 6.28f;
		unsigned int cycles = Get_CPU_Clock();
		float val = cos(f);	
		time += Get_CPU_Clock() - cycles;
		results[i] = val;
	}
	for (i=0; i<SAMPLES; i++) _Global+= results[i];
	printf("cos average cycles: %d\n",time / SAMPLES);

	// Time 100 multiplies
	time = Get_CPU_Clock();
	for (i=0; i<SAMPLES; i++) {
		float val = results[i]*results[SAMPLES-i-1];	
		results[i] = val;
	}
	time = Get_CPU_Clock() - time;
	for (i=0; i<SAMPLES; i++) _Global+= results[i];
	printf("multiply cycles: %d\n",time);


	// Time 100 adds
	time = Get_CPU_Clock();
	for (i=0; i<SAMPLES; i++) {
		float val = results[i]+results[SAMPLES-i-1];	
		results[i] = val;
	}
	time = Get_CPU_Clock() - time;
	for (i=0; i<SAMPLES; i++) _Global+= results[i];
	printf("add cycles: %d\n",time);


	// Time 100 multiplies
	time = Get_CPU_Clock();
	for (i=0; i<SAMPLES; i++) {
		float val = results[i]*results[SAMPLES-i-1];	
		results[i] = val;
	}
	time = Get_CPU_Clock() - time;
	for (i=0; i<SAMPLES; i++) _Global+= results[i];
	printf("multiply cycles: %d\n",time);

	// Time 100 adds
	time = Get_CPU_Clock();
	for (i=0; i<SAMPLES; i++) {
		float val = results[i]+results[SAMPLES-i-1];	
		results[i] = val;
	}
	time = Get_CPU_Clock() - time;
	for (i=0; i<SAMPLES; i++) _Global+= results[i];
	printf("add cycles: %d\n",time);

	
}


void Transform_Min_Max_AABox_Brute_Force
(
	const Matrix3D &	tm,
	const Vector3 &	min,
	const Vector3 &	max,
	Vector3 &			newmin,
	Vector3 &			newmax
)
{
	int i;
	Vector3 pts[8];

	pts[0].Set(min.X,min.Y,min.Z);
	pts[1].Set(min.X,max.Y,min.Z);
	pts[2].Set(max.X,max.Y,min.Z);
	pts[3].Set(max.X,min.Y,min.Z);

	pts[4].Set(min.X,min.Y,max.Z);
	pts[5].Set(min.X,max.Y,max.Z);
	pts[6].Set(max.X,max.Y,max.Z);
	pts[7].Set(max.X,min.Y,max.Z);

	for (i=0; i<8; i++) {
		pts[i] = tm * pts[i];
	}

	newmin = pts[0];
	newmax = pts[0];

	for (i=1; i<8; i++) {
		if (newmin.X >= pts[i].X) newmin.X = pts[i].X;
		if (newmin.Y >= pts[i].Y) newmin.Y = pts[i].Y;
		if (newmin.Z >= pts[i].Z) newmin.Z = pts[i].Z;
		
		if (newmax.X <= pts[i].X) newmax.X = pts[i].X;
		if (newmax.Y <= pts[i].Y) newmax.Y = pts[i].Y;
		if (newmax.Z <= pts[i].Z) newmax.Z = pts[i].Z;
	}
}


void test_point_containment(void)
{
	TriClass tri0,tri1;
	Vector3 normal0,normal1;
	Vector3 points[4];
	points[0].Set(10.6891f,23.5262f,-2.50006f);
	points[1].Set(1.0f,37.7164f,-2.50006f);
	points[2].Set(10.6566f,30.7858f,-2.50006f);
	points[3].Set(1.0f,16.0f,-2.50006f);
	
	for (int vi=0; vi<3; vi++) {
		tri0.V[vi] = &(points[vi]);
		tri1.V[vi] = &(points[vi+1]);
	}
	tri0.N = &normal0;
	tri1.N = &normal1;

	tri0.Compute_Normal();
	tri1.Compute_Normal();

	bool c0 = tri0.Contains_Point(Vector3(7.9999f,27.1883f,-2.50006f));
	bool c1 = tri1.Contains_Point(Vector3(7.9999f,27.1883f,-2.50006f));

	tri0.V[0] = &(points[1]);
	tri0.V[1] = &(points[0]);
	tri0.V[2] = &(points[2]);

	tri0.Compute_Normal();

	bool alternate_contains = tri0.Contains_Point(Vector3(7.9999f,27.1883f,-2.50006f));
}


float gaussian_function(float x)
{
	return exp(-x*x);
}

float integrate(float (*f)(float),float start,float end,int slices)
{
	float sum = 0.0f;
	float slice_width = (end-start)/slices;
	
	for (float sample = start; sample < end; sample += slice_width) {
		
		sum += f(sample) * slice_width;

	}

	return sum;
}
