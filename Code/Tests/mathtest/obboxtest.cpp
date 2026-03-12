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
 *                 Project Name : WWMath                                                       *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tests/mathtest/obboxtest.cpp                 $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 12/06/00 9:33a                                              $*
 *                                                                                             *
 *                    $Revision:: 8                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   test_obb_tri -- performs some "hard-coded" obb-tri collision tests                        *
 *   brute_force_cast_obb_tri -- binary-search method of finding the collision time for obb-tr *
 *   brute_force_obb_tri_test -- collide random obb's into random tri's                        *
 *   test_obb_obb -- performs some "hard-coded" obb-obb collision tests                        *
 *   brute_force_cast_obb_obb -- brute force function to verify collision of two obb's         *
 *   brute_force_obb_obb_test -- collide random obb's together                                 *
 *   Test_OBBoxes -- run all of the obb-obb and obb-tri tests                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "obboxtest.h"
#include "output.h"
#include "vector3.h"
#include "tri.h"
#include "obbox.h"
#include "wwmath.h"
#include "colmath.h"
#include "p_timer.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


//#define MANUAL_DEBUGGING


static Matrix3 _RotateZ45
(
	(float)WWMATH_SQRT2/2.0f,	-(float)WWMATH_SQRT2/2.0f,	0.0f,
	(float)WWMATH_SQRT2/2.0f,	 (float)WWMATH_SQRT2/2.0f,	0.0f,
							  0.0f,								0.0f,	1.0f
);

static Matrix3 _RotateZ90
(
	0.0f, -1.0f, 0.0f,
	1.0f,  0.0f, 0.0f,
	0.0f,  0.0f, 1.0f
);


/*********************************************************************
**
** OBBoxTriTestClass
** Test Data for OBBox->Triangle collision
**
*********************************************************************/
class OBBoxTriTestClass
{
public:
	OBBoxClass		Box;
	Vector3			BoxMove;
	Vector3			V0;
	Vector3			V1;
	Vector3			V2;
	Vector3			N;
	TriClass			Tri;
	float				Fraction;
	bool				StartBad;

	OBBoxTriTestClass
	(
		const Vector3 &c,					// center of box
		const Vector3 &e,					// extent of box
		const Matrix3 &b,					// basis of box
		const Vector3 &m,					// move for the box
		const Vector3 &v0,				// v0 of triangle
		const Vector3 &v1,				// v1 of triangle
		const Vector3 &v2,				// v2 of triangle
		float frac,							// expected fraction
		bool sol								// expected start solid
	)
	{
		BoxMove = m;
		V0 = v0;
		V1 = v1;
		V2 = v2;
		Fraction = frac;
		StartBad = sol;

		/*
		** Initialize the triangle
		*/
		Tri.V[0] = &V0;
		Tri.V[1] = &V1;
		Tri.V[2] = &V2;
		Tri.N = & N;
		Tri.Compute_Normal();

		/*
		** Initialize the box
		*/
		Box.Center = c;
		Box.Extent = e;
		Box.Basis = b;
	}
};

OBBoxTriTestClass Test0
(
	Vector3(0,0,0),			// box starting at origin
	Vector3(2,1,1),			// extent is 2 units along x, 1 y, 1 z
	Matrix3(1),
	Vector3(1,0,0),			// moving 5 along x axis
	Vector3(6,-3,-1),			// triangle crossing x and y extent but not colliding
	Vector3(8,-1,2),
	Vector3(9,0,-1),
	1.0f,
	false
);

OBBoxTriTestClass Test1
(
	Vector3(3,0,0),
	Vector3(1,2,1),
	Matrix3(1),
	Vector3(0,-2,0),
	Vector3(1,-3,0),
	Vector3(2,-3,5),
	Vector3(6,-3,1),
	0.5f,
	false
);

OBBoxTriTestClass Test2
(
	Vector3(-3.5,-1.5,0),	// sweeping a 3x3 box along pos x and neg y
	Vector3(1.5,1.5,1.5),
	Matrix3(1),
	Vector3(4,-4,0),
	Vector3(-4,-4,-1),		// into a polygon in y-z plane
	Vector3(-2,-4,5),
	Vector3(0,-4,1),
	0.25f,						// should only move 25%
	false
);

OBBoxTriTestClass Test3
(
	Vector3(-3.5,-1.5,0) + 0.25f * Vector3(4,-4,0),
	Vector3(1.5,1.5,1.5),	// starting at end of test2's move, should be 0.0 but not StartBad!
	Matrix3(1),
	Vector3(4,-4,0),
	Vector3(-4,-4,-1),		// into a polygon in y-z plane
	Vector3(-2,-4,5),
	Vector3(0,-4,1),
	0.0f,
	false
);

OBBoxTriTestClass Test4
(
	Vector3(-3.5f,-1.5f,0),	// Same as test 2
	Vector3(1.5f,1.5f,1.5f),
	Matrix3(1),
	Vector3(4,-4,0),
	Vector3(-9,-4,-1),		// into a polygon in y-z plane *but* just barely not in the way
	Vector3(-8,-4,5),
	Vector3(-4.9f,-4,0),
	1.0f,							// should move 100%
	false
);

OBBoxTriTestClass Test5
(
	Vector3(-3.5,-1.5,0),	// Same as test 3 with box brushing polygon vertex.
	Vector3(1.5,1.5,1.5),
	Matrix3(1),
	Vector3(4,-4,0),
	Vector3(-9,-4,-1),		// into a polygon in y-z plane just touching path of box
	Vector3(-8,-4,5),
	Vector3(-4,-4,0),
	1.0f,							// should move 100%
	false
);

OBBoxTriTestClass Test6
(
	Vector3(-3.5f,-1.5f,0),	// Same as test 3 with box brushing polygon vertex.
	Vector3(1.5f,1.5f,1.5f),
	Matrix3(1),
	Vector3(4,-4,0),

	Vector3(-9,-4,-1),		// into a polygon in y-z plane just barely hitting it
	Vector3(-8,-4,5),
	Vector3(-3.999f,-4,0),	// (-4,-4,0) would just "touch" (see test5)

	0.25f,						// should move 25%
	false
);

OBBoxTriTestClass Test7
(
	Vector3(0,0,0),			// This is a case where the box starts out intersecting
	Vector3(5,5,5),
	Matrix3(1),
	Vector3(4,4,0),
	Vector3(1,4,-1),
	Vector3(2,4,5),
	Vector3(5,4,0),
	0.0f,
	true
);

OBBoxTriTestClass Test8
(
	Vector3(-2.5,2,0),		// center
	Vector3(1.5,1,1),			// extent
	Matrix3(1),					// basis
	Vector3(3,0,0),			// move

	Vector3(1,2,0),			// v0
	Vector3(3,4,5),			// v1
	Vector3(4,5,-1),			// v2
	0.66666667f,
	false
);

OBBoxTriTestClass Test9
(
	Vector3(0,0,0),		// Box with diagonal y-z length of 1, rotated 45 about z
	Vector3(WWMATH_SQRT2/2.0,WWMATH_SQRT2/2.0,1),
	_RotateZ45,
	Vector3(4,0,0),

	Vector3(3,2,-1),		// triangle blocking the move at x=3 (hitting back side)
	Vector3(3,0,1),
	Vector3(3,-2,-1),

	0.5f,						// hitting another box edge-to-face halfway through the move
	false
);

OBBoxTriTestClass * OBBoxTriTestCases[] =
{
	&Test0,
	&Test1,
	&Test2,
	&Test3,
	&Test4,
	&Test5,
	&Test6,
	&Test7,
	&Test8,
	&Test9
};


/***********************************************************************************************
 * test_obb_tri -- performs some "hard-coded" obb-tri collision tests                          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/8/99     GTH : Created.                                                                 *
 *=============================================================================================*/
void test_obb_tri(void)
{
	Print_Title("Testing OBBox->Triangle collision.");

	/*
	** Test the sweep function with a bunch of boxes and triangles
	*/
	CastResultStruct result;
	result.ComputeContactPoint = true;
	int numtests = sizeof(OBBoxTriTestCases)/sizeof(OBBoxTriTestClass *);
	unsigned cycles;
	unsigned totalcycles = 0;

	// prime the cache
	OBBoxTriTestClass * testcase = OBBoxTriTestCases[9];
	cycles = Get_CPU_Clock();

	CollisionMath::Collide(	testcase->Box,
									testcase->BoxMove,
									testcase->Tri,
									Vector3(0,0,0),
									&result);

	cycles = Get_CPU_Clock() - cycles;


	// now time and test the routine
	for (int i=0; i<numtests; i++) {

		testcase = OBBoxTriTestCases[i];

		result.Fraction = 1.0;
		result.StartBad = false;
		result.Normal.Set(0,0,0);

		cycles = Get_CPU_Clock();

		CollisionMath::Collide(	testcase->Box,
										testcase->BoxMove,
										testcase->Tri,
										Vector3(0,0,0),
										&result);

		cycles = Get_CPU_Clock() - cycles;
		totalcycles += cycles;
		if ((WWMath::Fabs(testcase->Fraction - result.Fraction) > WWMATH_EPSILON) ||
			 (testcase->StartBad != result.StartBad))
		{
			printf("test: %3d  fraction: %8.6f   cycles: %12d \tfailed!\n",i,result.Fraction,cycles);
		} else {
			printf("test: %3d  fraction: %8.6f   cycles: %12d \tpassed...\n",i,result.Fraction,cycles);
		}

	}
	printf("average cycles: %d\n",totalcycles / numtests);
	printf("\n");


	/*
	** Test a box moving down the z-axis to a polygon, then moving along
	** the x-axis along the surface of the polygon.
	*/
	OBBoxClass testbox;
	Vector3 move;
	Vector3 v0,v1,v2,n;
	TriClass testtri;

	v0.Set(0,1,0);
	v1.Set(-1,-1,0);
	v2.Set(1,1,0);

	testtri.V[0] = &v0;
	testtri.V[1] = &v1;
	testtri.V[2] = &v2;
	testtri.N = &n;
	testtri.Compute_Normal();

	testbox.Center.Set(0,0,2.363f);
	testbox.Extent.Set(1,1,1);
	move.Set(0,0,-5.0f);

	CastResultStruct cres;
	cres.StartBad = 0;
	cres.Fraction = 1.0f;
	CollisionMath::Collide(testbox,move,testtri,Vector3(0,0,0),&cres);

	printf("fraction = %f\n",cres.Fraction);

	testbox.Center += cres.Fraction * move;

	move.Set(1.0f,1.0f,0.0f);

	cres.StartBad = 0;
	cres.Fraction = 1.0f;
	CollisionMath::Collide(testbox,move,testtri,Vector3(0,0,0),&cres);

	printf("fraction = %f\n",cres.Fraction);

	testbox.Center += cres.Fraction * move;

	/*
	** Try to get a timing of Oriented_Box_Intersects_Tri versus
	** CollisionMath::Intersect
	*/
	const int REPEAT = 50;
	cycles = Get_CPU_Clock();
	for (int j=0;j<REPEAT;j++) {
		testcase = OBBoxTriTestCases[7];
		CollisionMath::Intersection_Test(testcase->Box,testcase->Tri);
	}
	cycles = Get_CPU_Clock() - cycles;
	printf("CollisionMath::Intersect - average cycles: %d\n",cycles / REPEAT);
	printf("\n");

	cycles = Get_CPU_Clock();
	for (j=0;j<REPEAT;j++) {
		testcase = OBBoxTriTestCases[7];
		Oriented_Box_Intersects_Tri(testcase->Box,testcase->Tri);
	}
	cycles = Get_CPU_Clock() - cycles;
	printf("Oriented_Box_Intersects_Tri - average cycles: %d\n",cycles / REPEAT);
	printf("\n");

}



/***********************************************************************************************
 * brute_force_cast_obb_tri -- binary-search method of finding the collision time for obb-tri  *
 *                                                                                             *
 * This function doesn't really work in the general case.  Only when the endpoint of the move  *
 * is guaranteed to be inside the triangle                                                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/8/99     GTH : Created.                                                                 *
 *=============================================================================================*/
float brute_force_cast_obb_tri
(
	const OBBoxClass & box,
	const Vector3 & move,
	const TriClass & tri
)
{
	float istart = 0.0f;
	float iend = 1.0f;

	while (iend - istart > WWMATH_EPSILON/2.0f) {
		float icenter = (iend + istart) / 2.0f;
		OBBoxClass testbox = box;
		testbox.Center = box.Center + icenter*move;

		if (Oriented_Box_Intersects_Tri(testbox,tri)) {
			iend = icenter;
		} else {
			istart = icenter;
		}
	}
	return (iend + istart) / 2.0f;
}


/***********************************************************************************************
 * brute_force_obb_tri_test -- collide random obb's into random tri's                          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/8/99     GTH : Created.                                                                 *
 *=============================================================================================*/
void brute_force_obb_tri_test(int test_count)
{
	Print_Title("Brute Force Testing OBBox->Tri collision.");

	Vector3 v[3];
	Vector3 n;
	OBBoxClass box;
	TriClass tri;
	Vector3 move;

	tri.V[0] = &v[0];
	tri.V[1] = &v[1];
	tri.V[2] = &v[2];
	tri.N = &n;

	int fail_count = 0;
	int startbad_count = 0;
	int startbad_fail_count = 0;
	int bad_points = 0;

	float min_fraction = 1.0f;
	float max_fraction = 0.0f;
	float avg_fraction = 0.0f;
	float avg_fraction_error = 0.0f;
	int fraction_error_count = 0;
	float max_error = 0.0f;

	for (int i=0; i<test_count; i++) {

		// v0 is always in positive quadrant
		v[0].X = WWMath::Random_Float(0,5);
		v[0].Y = WWMath::Random_Float(0,5);
		v[0].Z = WWMath::Random_Float(0,5);

		// v1 is always +x,+y,-z
		v[1].X = WWMath::Random_Float(0,5);
		v[1].Y = WWMath::Random_Float(0,5);
		v[1].Z = -WWMath::Random_Float(0,5);

		// v2 is always -x,-y,-z
		v[2].X = -WWMath::Random_Float(0,5);
		v[2].Y = -WWMath::Random_Float(0,5);
		v[2].Z = -WWMath::Random_Float(0,5);

		tri.Compute_Normal();

		// make a random box
		box.Init_Random(0.25f,3.0f);

		// put it in a random position in a 20x20x20 cube
		box.Center.X = WWMath::Random_Float(-10.0f,10.0f);
		box.Center.Y = WWMath::Random_Float(-10.0f,10.0f);
		box.Center.Z = WWMath::Random_Float(-10.0f,10.0f);

		// make a move vector that will move the box's center to the centroid of the tri
		Vector3 new_center;
		new_center.X = (v[0].X + v[1].X + v[2].X) / 3.0f;
		new_center.Y = (v[0].Y + v[1].Y + v[2].Y) / 3.0f;
		new_center.Z = (v[0].Z + v[1].Z + v[2].Z) / 3.0f;
		move = new_center - box.Center;

		// sweep box into tri!
		CastResultStruct result;
		result.ComputeContactPoint = true;
		CollisionMath::Collide(box,move,tri,Vector3(0,0,0),&result);

		// if they started out intersecting, this test doesn't count
		if (result.StartBad) {
			startbad_count++;
			if (!Oriented_Box_Intersects_Tri(box,tri)) {
				startbad_fail_count++;
				printf("False startbad!");
			}
		}

		// if they were intersecting, never mind
		if (!result.StartBad) {

			printf(".");
			bool success = true;

			// add the fraction into the total so we get an idea
			// how far our tests are going (hopefully a good mix)
			avg_fraction += result.Fraction;
			if (result.Fraction < min_fraction) min_fraction = result.Fraction;
			if (result.Fraction > max_fraction) max_fraction = result.Fraction;

			// verify that the fraction is correct
			float realfrac = brute_force_cast_obb_tri(box,move,tri);
			if (fabs(realfrac - result.Fraction) > WWMATH_EPSILON) {
				success = false;
				float error = fabs(realfrac - result.Fraction);
				avg_fraction_error += error;
				fraction_error_count++;
				if (error > max_error) max_error = error;
			}

			// verify that they are not intersecting at the end of the move
			// if the allowed move is smaller than epsilon, we skip this and don't move
			if (result.Fraction > WWMATH_EPSILON) {
				OBBoxClass box2 = box;
				CastResultStruct second_result;
				box2.Center += /*0.9999f **/ result.Fraction * move;
				CollisionMath::Collide(box2,move,tri,Vector3(0,0,0),&second_result);

				if (second_result.StartBad) success = false;
				if ((result.Fraction < 1.0f) && (second_result.Fraction > 0.01f)) success = false;
			}

			// if something failed, do the test again to let the programmer step through...
			if (!success) {
				fail_count++;
				CastResultStruct redo_result;
				redo_result.ComputeContactPoint = true;
				CollisionMath::Collide(box,move,tri,Vector3(0,0,0),&redo_result);
			}
		}
	}
	printf("\n");
	int passes = test_count - (startbad_fail_count + fail_count);
	printf("Passed %d tests out of %d tests.\n",passes,test_count);
	printf("StartBad tests: %d  failures: %d\n",startbad_count,startbad_fail_count);

	if (fraction_error_count > 0) {
		avg_fraction_error /= (float)fraction_error_count;
	}
	avg_fraction /= (float)(test_count - startbad_count);
	printf("Largest Fraction:     %f\n",max_fraction);
	printf("Smallest Fraction:    %f\n",min_fraction);
	printf("Average Fraction:     %f\n",avg_fraction);
	printf("Average Error:        %f\n",avg_fraction_error);
	printf("Biggest Error:        %f\n",max_error);
}




/*********************************************************************
**
** OBBoxTestClass
** Data for testing OBBox->OBBox collision detection
**
*********************************************************************/
class OBBoxTestClass
{
public:

	OBBoxClass		Box0;
	Vector3			Move0;
	OBBoxClass		Box1;
	Vector3			Move1;
	float				Fraction;
	bool				StartBad;

	OBBoxTestClass
	(
		const Vector3 & c0,		// center of box0
		const Vector3 & e0,		// extent of box0
		const Matrix3 & b0,		// basis of box0
		const Vector3 & m0,		// move for box0

		const Vector3 & c1,		// center of box1
		const Vector3 & e1,		// extent of box1
		const Matrix3 & b1,		// basis of box1
		const Vector3 & m1,		// move for box1

		float frac,					// expected fraction
		bool sol						// expected start solid
	) :
		Box0(c0,e0,b0),
		Move0(m0),
		Box1(c1,e1,b1),
		Move1(m1),
		Fraction(frac),
		StartBad(sol)
	{
	}
};


OBBoxTestClass BTest0
(
	Vector3(0,0,0),		// center
	Vector3(4,0,0),		// extent
	Matrix3(1),				// basis
	Vector3(4,0,0),		// move
	Vector3(6,0,0),		// center
	Vector3(1,1,1),		// extent
	Matrix3(1),				// basis
	Vector3(0,0,0),		// move
	0.25f,
	false
);

OBBoxTestClass BTest1
(
	Vector3(-3.5f,-1.5f,0.0f),
	Vector3(1.5f,1.5f,1.5f),
	Matrix3(1),
	Vector3(4,-4,0),

	Vector3(-5.1,-5,0),
	Vector3(1,1,1),
	Matrix3(1),
	Vector3(0,0,0),

	1.0f,						// should just barely go by (touches)
	false
);

OBBoxTestClass BTest2
(
	Vector3(3,0,0),
	Vector3(7,1,1),
	Matrix3(1),
	Vector3(4,-4,0),

	Vector3(9.5,0,0),
	Vector3(1,6,1),
	Matrix3(1),
	Vector3(0,0,0),

	0.0f,						// startbad
	true
);

OBBoxTestClass BTest3
(
	Vector3(0,0,0),		// Box with diagonal y-z length of 1, rotated 45 about z
	Vector3(WWMATH_SQRT2/2.0,WWMATH_SQRT2/2.0,1),
	_RotateZ45,
	Vector3(4,0,0),

	Vector3(4,0,0),		// axis-aligned box blocking the move along the x-axis
	Vector3(1,3,1),
	Matrix3(1),
	Vector3(0,0,0),

	0.5f,						// hitting another box edge-to-face halfway through the move
	false
);

OBBoxTestClass BTest4
(
	Vector3(0,0,0),		// Box with diagonal y-z length of 1, rotated 45 about z
	Vector3(WWMATH_SQRT2/2.0,WWMATH_SQRT2/2.0,1),
	_RotateZ45,
	Vector3(0,4,0),

	Vector3(0,4,0),		// axis-aligned box blocking the move along the x-axis
	Vector3(3,1,1),
	Matrix3(1),
	Vector3(0,0,0),

	0.5f,						// hitting another box edge-to-face halfway through the move
	false
);

OBBoxTestClass BTest5
(
	Vector3(0,0,0),		// Box with diagonal y-z length of 1, rotated 45 about z
	Vector3(WWMATH_SQRT2/2.0,WWMATH_SQRT2/2.0,1),
	_RotateZ45,
	Vector3(0,-4,0),

	Vector3(0,-4,0),		// axis-aligned box blocking the move along the x-axis
	Vector3(3,1,1),
	Matrix3(1),
	Vector3(0,0,0),

	0.5f,						// hitting another box edge-to-face halfway through the move
	false
);

OBBoxTestClass * OBBoxTestCases[] =
{
	&BTest0,
	&BTest1,
	&BTest2,
	&BTest3,
	&BTest4,
	&BTest5
};


/***********************************************************************************************
 * test_obb_obb -- performs some "hard-coded" obb-obb collision tests                          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/8/99     GTH : Created.                                                                 *
 *=============================================================================================*/
void test_obb_obb(void)
{
	Print_Title("Testing OBBox->OBBox collision.");

	CastResultStruct result;
	result.ComputeContactPoint = true;
	int numtests = sizeof(OBBoxTestCases)/sizeof(OBBoxTestClass *);
	unsigned cycles;
	unsigned totalcycles = 0;

	// prime the cache
	OBBoxTestClass * testcase = OBBoxTestCases[4];
	cycles = Get_CPU_Clock();

	CollisionMath::Collide(	testcase->Box0,
									testcase->Move0,
									testcase->Box1,
									testcase->Move1,
									&result);

	cycles = Get_CPU_Clock() - cycles;


	// now time and test the routine
	for (int i=0; i<numtests; i++) {

		testcase = OBBoxTestCases[i];

		result.Fraction = 1.0;
		result.StartBad = false;
		result.Normal.Set(0,0,0);

		cycles = Get_CPU_Clock();

		CollisionMath::Collide(	testcase->Box0,
										testcase->Move0,
										testcase->Box1,
										testcase->Move1,
										&result);

		cycles = Get_CPU_Clock() - cycles;
		totalcycles += cycles;

		if ((WWMath::Fabs(testcase->Fraction - result.Fraction) > WWMATH_EPSILON) ||
			 (testcase->StartBad != result.StartBad))
		{
			printf("test: %5d\tcycles: %12d \t\tfailed!\n",i,cycles);
		} else {
			printf("test: %5d\tcycles: %12d \t\tpassed...\n",i,cycles);
		}
	}
	printf("average cycles: %d\n",totalcycles / numtests);
	printf("\n");

	/*
	** Try to get an accurate timing, run the same (startbad) test
	** 100 times...
	*/
	const int REPEAT=50;
	cycles = Get_CPU_Clock();
	for (int j=0;j<REPEAT;j++) {
		testcase = OBBoxTestCases[0];
		result.Fraction = 1.0;
		result.StartBad = false;
		result.Normal.Set(0,0,0);
		CollisionMath::Collide(	testcase->Box0,
										testcase->Move0,
										testcase->Box1,
										testcase->Move1,
										&result);
	}
	cycles = Get_CPU_Clock() - cycles;
	printf("average cycles: %d\n",cycles / REPEAT);
	printf("\n");

	/*
	** Now, comparing Oriented_Boxes_Intersect with CollisionMath::Intersect
	*/
	cycles = Get_CPU_Clock();
	for (j=0;j<REPEAT;j++) {
		testcase = OBBoxTestCases[2];
		CollisionMath::Intersection_Test(testcase->Box0,testcase->Box1);
	}
	cycles = Get_CPU_Clock() - cycles;
	printf("CollisionMath::Intersect - average cycles: %d\n",cycles / REPEAT);
	printf("\n");

	cycles = Get_CPU_Clock();
	for (j=0;j<REPEAT;j++) {
		testcase = OBBoxTestCases[2];
		Oriented_Boxes_Intersect(testcase->Box0,testcase->Box1);
	}
	cycles = Get_CPU_Clock() - cycles;
	printf("Oriented_Boxes_Intersect - average cycles: %d\n",cycles / REPEAT);
	printf("\n");
}


/***********************************************************************************************
 * brute_force_cast_obb_obb -- brute force function to verify collision of two obb's           *
 *                                                                                             *
 * This function doesn't really work in the general case.  Only when the endpoint of the move  *
 * is guaranteed to be inside the other box...                                                 *
 *                                                                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/8/99     GTH : Created.                                                                 *
 *=============================================================================================*/
float brute_force_cast_obb_obb
(
	const OBBoxClass & box0,
	const Vector3 & move0,
	const OBBoxClass & box1
)
{
	float istart = 0.0f;
	float iend = 1.0f;

	while (iend - istart > WWMATH_EPSILON) {
		float icenter = (iend + istart) / 2.0f;
		OBBoxClass testbox = box0;
		testbox.Center = box0.Center + icenter*move0;

		if (Oriented_Boxes_Intersect(testbox,box1)) {
			iend = icenter;
		} else {
			istart = icenter;
		}
	}
	return (iend + istart) / 2.0f;
}


/***********************************************************************************************
 * brute_force_obb_obb_test -- collide random obb's together                                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/8/99     GTH : Created.                                                                 *
 *=============================================================================================*/
void brute_force_obb_obb_test(int test_count)
{
	Print_Title("Brute Force Testing OBBox->OBBox collision.");

	OBBoxClass box0;
	OBBoxClass box1;
	Vector3 move0;

	int fail_count = 0;
	int startbad_count = 0;
	int startbad_fail_count = 0;

	float avg_fraction = 0.0f;
	float avg_fraction_error = 0.0f;
	int fraction_error_count = 0;
	float max_error = 0.0f;
	float min_fraction = 1.0f;
	float max_fraction = 0.0f;

	for (int i=0; i<test_count; i++) {

		box0.Init_Random(0.25f,3.0f);
		box1.Init_Random(0.25f,3.0f);

		box0.Center.X = WWMath::Random_Float(-10.0f,10.0f);
		box0.Center.Y = WWMath::Random_Float(-10.0f,10.0f);
		box0.Center.Z = WWMath::Random_Float(-10.0f,10.0f);
		box1.Center.Set(0,0,0);

		Vector3 newcenter;
		newcenter.X = WWMath::Random_Float(-0.12f,0.12f);	// new center must be inside other box
		newcenter.Y = WWMath::Random_Float(-0.12f,0.12f);
		newcenter.Z = WWMath::Random_Float(-0.12f,0.12f);
		move0 = newcenter - box0.Center;

		// sweep box0 into box1
		CastResultStruct result;
		result.ComputeContactPoint = true;
		CollisionMath::Collide(box0,move0,box1,Vector3(0,0,0),&result);

		// if they were intersecting, verify that
		if (result.StartBad) {
			startbad_count++;
			if (!Oriented_Boxes_Intersect(box0,box1)) {
				startbad_fail_count++;
				printf("False startbad!");
			}
		}

		// if they weren't intersecting, verify the allowed move
		if (!result.StartBad) {

			bool success = true;

			// add the fraction into the total so we get an idea
			// how far our tests are going (hopefully a good mix)
			avg_fraction += result.Fraction;
			if (result.Fraction < min_fraction) min_fraction = result.Fraction;
			if (result.Fraction > max_fraction) max_fraction = result.Fraction;

			// verify that the fraction is correct
			float realfrac = brute_force_cast_obb_obb(box0,move0,box1);
			if (fabs(realfrac - result.Fraction) > WWMATH_EPSILON) {
				success = false;
				float error = fabs(realfrac - result.Fraction);
				avg_fraction_error += error;
				fraction_error_count++;
				if (error > max_error) max_error = error;
			}

			// verify that they are not intersecting now
			// if the allowed move is smaller than epsilon, we skip this and don't move
			if (result.Fraction > WWMATH_EPSILON) {

				OBBoxClass box2 = box0;
				box2.Center += (1.0f - WWMATH_EPSILON) * result.Fraction * move0;

				CastResultStruct second_result;
				CollisionMath::Collide(box2,move0,box1,Vector3(0,0,0),&second_result);

				if (second_result.StartBad) {
					success = false;
#ifdef MANUAL_DEBUGGING
					_asm int 0x03;
					while (!success) {
						CastResultStruct move_result;
						CollisionMath::Collide(box0,move0,box1,Vector3(0,0,0),&move_result);
						second_result.Reset();
						CollisionMath::Collide(box2,move0,box1,Vector3(0,0,0),&second_result);
					}
#endif
				}
				if ((result.Fraction < 1.0f) && (second_result.Fraction > 0.01f)) success = false;
			}

			// if something failed, do the test again to let the programmer step through...
			if (!success) {
#ifdef MANUAL_DEBUGGING
				_asm int 0x03;
				while (!success) {
					fail_count++;
					CastResultStruct redo_result;
					redo_result.ComputePoint = true;
					CollisionMath::Collide(box0,move0,box1,Vector3(0,0,0),&redo_result);
				}
#endif
				printf("x");
			} else {
				printf(".");
			}
		}
	}
	printf("\n");

	int passes = test_count - (startbad_fail_count + fail_count);
	printf("Passed %d tests out of %d tests.\n",passes,test_count);
	printf("StartBad tests: %d  failures: %d\n",startbad_count,startbad_fail_count);

	if (fraction_error_count) {
		avg_fraction_error /= (float)fraction_error_count;
	}
	avg_fraction /= (float)(test_count - startbad_count);

	printf("Largest Fraction:   %f\n",max_fraction);
	printf("Smallest Fraction:  %f\n",min_fraction);
	printf("Average Fraction:   %f\n",avg_fraction);
	printf("Average Error:      %f\n",avg_fraction_error);
	printf("Biggest Error:      %f\n",max_error);
}



/***********************************************************************************************
 * Test_OBBoxes -- run all of the obb-obb and obb-tri tests                                    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/8/99     GTH : Created.                                                                 *
 *=============================================================================================*/
void Test_OBBoxes(void)
{
	const int TESTCOUNT = 100;
	srand(time(NULL));

	test_obb_tri();
	test_obb_obb();
	brute_force_obb_tri_test(TESTCOUNT);
	brute_force_obb_obb_test(TESTCOUNT);
}



