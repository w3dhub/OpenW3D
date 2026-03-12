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
 *                     $Archive:: /Commando/Code/Tests/mathtest/aaboxtest.cpp                 $*
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 3/13/00 4:33p                                               $*
 *                                                                                             *
 *                    $Revision:: 9                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "aaboxtest.h"
#include "output.h"
#include "vector3.h"
#include "tri.h"
#include "aabox.h"
#include "wwmath.h"
#include "colmath.h"
#include "p_timer.h"
#include <stdio.h>


class AABoxTriTestClass
{
public:
	AABoxClass		Box;
	Vector3			BoxMove;
	Vector3			V0;
	Vector3			V1;
	Vector3			V2;
	Vector3			N;
	TriClass			Tri;
	float				Fraction;
	bool				StartBad;

	AABoxTriTestClass
	(
		const Vector3 c,					// center of box
		const Vector3 e,					// extent of box
		const Vector3 m,					// move for the box
		const Vector3 v0,					// v0 of triangle
		const Vector3 v1,					// v1 of triangle
		const Vector3 v2,					// v2 of triangle
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
	}
};

AABoxTriTestClass Test0
(
	Vector3(0,0,0),			// box starting at origing
	Vector3(2,1,1),			// 2 units along x, 1 y, 1 z
	Vector3(1,0,0),			// moving 5 along x axis
	Vector3(6,-3,-1),			// triangle crossing x and y extent but not colliding
	Vector3(8,-1,2),
	Vector3(9,0,-1),
	1.0f,
	false
);

AABoxTriTestClass Test1
(
	Vector3(3,0,0),
	Vector3(1,2,1),
	Vector3(0,-2,0),
	Vector3(1,-3,0),
	Vector3(2,-3,5),
	Vector3(6,-3,1),
	0.5f,
	false
);

AABoxTriTestClass Test2
(
	Vector3(-3.5,-1.5,0),	// sweeping a 3x3 box along pos x and neg y
	Vector3(1.5,1.5,1.5),
	Vector3(4,-4,0),
	Vector3(-4,-4,-1),		// into a polygon in y-z plane
	Vector3(-2,-4,5),
	Vector3(0,-4,1),
	0.25f,						// should only move 25%
	false
);

AABoxTriTestClass Test3
(
	Vector3(-3.5,-1.5,0) + 0.25f * Vector3(4,-4,0),
	Vector3(1.5,1.5,1.5),	// starting at end of test2's move, should be 0.0 but not StartBad!
	Vector3(4,-4,0),
	Vector3(-4,-4,-1),		// into a polygon in y-z plane
	Vector3(-2,-4,5),
	Vector3(0,-4,1),
	0.0f,
	false
);

AABoxTriTestClass Test4
(
	Vector3(-3.5f,-1.5f,0),	// Same as test 2
	Vector3(1.5f,1.5f,1.5f),
	Vector3(4,-4,0),
	Vector3(-9,-4,-1),		// into a polygon in y-z plane *but* just barely not in the way
	Vector3(-8,-4,5),
	Vector3(-4.001f,-4,0),
	1.0f,							// should move 100%
	false
);

AABoxTriTestClass Test5
(
	Vector3(-3.5,-1.5,0),	// Same as test 3 with box brushing polygon vertex.
	Vector3(1.5,1.5,1.5),
	Vector3(4,-4,0),
	Vector3(-9,-4,-1),		// into a polygon in y-z plane just touching path of box
	Vector3(-8,-4,5),
	Vector3(-4,-4,0),
	1.0f,							// should move 100%
	false
);

AABoxTriTestClass Test6
(
	Vector3(-3.5,-1.5,0),	// Same as test 3 with box brushing polygon vertex.
	Vector3(1.5,1.5,1.5),
	Vector3(4,-4,0),
	Vector3(-9,-4,-1),		// into a polygon in y-z plane just touching path of box
	Vector3(-8,-4,5),
	Vector3(-4,-4,0),
	1.0f,							// should move 100%
	false
);

AABoxTriTestClass Test7
(
	Vector3(0,0,0),			// This is a case where the box starts out intersecting
	Vector3(5,5,5),
	Vector3(4,4,0),
	Vector3(1,4,-1),
	Vector3(2,4,5),
	Vector3(5,4,0),
	0.0f,
	true
);

AABoxTriTestClass Test8
(
	Vector3(-2.5,2,0),
	Vector3(1.5,1,1),
	Vector3(3,0,0),
	Vector3(1,2,0),
	Vector3(3,4,5),
	Vector3(4,5,-1),
	0.66666667f,
	true
);

AABoxTriTestClass * AABoxTriTestCases[] =
{
	&Test0,
	&Test1,
	&Test2,
	&Test3,
	&Test4,
	&Test5,
	&Test6,
	&Test7,
	&Test8
};


void Test_AABoxes(void)
{
	int i;
	Print_Title("Testing AABox volume sweeping.");

	/*
	** Test the sweep function with a bunch of boxes and triangles
	*/
	CastResultStruct result;
	int numtests = sizeof(AABoxTriTestCases)/sizeof(AABoxTriTestClass *);
	unsigned cycles;

	// prime the cache
	AABoxTriTestClass * testcase = AABoxTriTestCases[4];
	cycles = Get_CPU_Clock();
	CollisionMath::Collide(testcase->Box,testcase->BoxMove,testcase->Tri,&result);
	cycles = Get_CPU_Clock() - cycles;


	// Now time and test the routine.  Repeating and averaging time for each test
	float * cycle_counts = new float[numtests];
	for (i=0; i<numtests;i++) {
		cycle_counts[i] = 0.0f;
	}

	int num_repetitions = 50;
	for (int repeat = 0; repeat < num_repetitions; repeat++) {
		for (i=0; i<numtests; i++) {

			testcase = AABoxTriTestCases[i];

			result.Fraction = 1.0;
			result.StartBad = false;
			result.Normal.Set(0,0,0);

			cycles = Get_CPU_Clock();
			CollisionMath::Collide(testcase->Box,testcase->BoxMove,testcase->Tri,&result);
			cycles = Get_CPU_Clock() - cycles;

			printf("AABox -> Tri cycles: %d\n",cycles);
			cycle_counts[i] += cycles;

			if ((WWMath::Fabs(testcase->Fraction - result.Fraction) > WWMATH_EPSILON) ||
				 (testcase->StartBad != result.StartBad))
			{
				printf("test %d failed!\n",i);
			} else {
				printf("test %d passed...\n",i);
			}
		}
	}
	printf("\n");

	for (i=0; i<numtests;i++) {
		cycle_counts[i] /= (float)num_repetitions;
		printf("Test%d Average Cycles: %f\r\n",i,cycle_counts[i]);
	}

	/*
	** Test a box moving down the z-axis to a polygon, then moving along
	** the x-axis along the surface of the polygon.
	*/
	AABoxClass testbox;
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
	CollisionMath::Collide(testbox,move,testtri,&cres);

	printf("fraction = %f\n",cres.Fraction);

	testbox.Center += cres.Fraction * move;

	move.Set(1.0f,1.0f,0.0f);

	cres.StartBad = 0;
	cres.Fraction = 1.0f;
	CollisionMath::Collide(testbox,move,testtri,&cres);

	printf("fraction = %f\n",cres.Fraction);

	testbox.Center += cres.Fraction * move;

}

