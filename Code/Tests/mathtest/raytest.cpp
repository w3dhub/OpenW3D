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
 *                 Project Name : MathTest                                                     *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tests/mathtest/raytest.cpp                   $*
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 3/17/00 9:34a                                               $*
 *                                                                                             *
 *                    $Revision:: 10                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "raytest.h"
#include "output.h"
#include "wwmath.h"
#include "tri.h"
#include "vector3.h"
#include "lineseg.h"
#include "aabox.h"
#include "obbox.h"
#include "colmath.h"
#include "p_timer.h"
#include <cinttypes>
#include <stdio.h>


void line_box_test(const LineSegClass & line,const OBBoxClass & box,CastResultStruct * res);

/************************************************************************************************
**
**  Ray - Triangle Test Cases
**
************************************************************************************************/

class RayTriTestClass
{
public:
	Vector3			P0;
	Vector3			P1;
	Vector3			V0;
	Vector3			V1;
	Vector3			V2;
	Vector3			N;
	float				Fraction;
	bool				StartBad;
	LineSegClass	LineSeg;
	TriClass			Tri;

	RayTriTestClass
	(
		const Vector3 p0,					// p0 of ray
		const Vector3 p1,					// p1 of ray
		const Vector3 v0,					// v0 of triangle
		const Vector3 v1,					// v1 of triangle
		const Vector3 v2,					// v2 of triangle
		float frac,							// expected fraction
		bool sol								// expected start solid
	) :
		P0(p0),P1(p1),V0(v0),V1(v1),V2(v2),Fraction(frac),StartBad(sol),LineSeg(P0,P1)
	{
		Tri.V[0] = &V0;
		Tri.V[1] = &V1;
		Tri.V[2] = &V2;
		Tri.N = & N;
		Tri.Compute_Normal();
	}
};


RayTriTestClass RayTriTestCases[] =
{
	RayTriTestClass
	(
		Vector3(0,0,0),
		Vector3(1,0,0),
		Vector3(1,1,1),
		Vector3(1,-1,1),
		Vector3(1,0,-1),
		1.0,
		false
	),
	RayTriTestClass				// ray going down +x, hitting a triangle at x=0.75
	(
		Vector3(0,0,0),
		Vector3(1,0,0),
		Vector3(0.5,1,1),
		Vector3(1,-1,1),
		Vector3(0.75,0,-1),
		0.75,
		false
	),
	RayTriTestClass				// ray going down the -z axis hitting an x-y triangle
	(
		Vector3(0,0,5),			// ray start
		Vector3(0,0,-5),			// ray end
		Vector3(0,1,0),			// p0
		Vector3(-1,-1,0),			// p1
		Vector3(1,-1,0),			// p2
		0.5,
		false
	),
	RayTriTestClass				// ray going down the -z axis, hitting back of an x-y triangle
	(
		Vector3(0,0,5),			// ray start
		Vector3(0,0,-5),			// ray end
		Vector3(0,1,0),			// p0
		Vector3(1,-1,0),			// p1
		Vector3(-1,-1,0),			// p2
		0.5,
		false
	),
	RayTriTestClass				// ray going down the +x axis, hitting a y-z triangle
	(
		Vector3(0,0,0),			// ray start
		Vector3(5,0,0),			// ray end
		Vector3(2,0,1),			// p0
		Vector3(2,1,-1),			// p1
		Vector3(2,-1,-1),			// p2
		2.0f / 5.0f,
		false
	),
	RayTriTestClass				// ray going down the -z axis, hitting vertex 2 of the triangle
	(
		Vector3(0,0,5),			// ray start
		Vector3(0,0,-5),			// ray end
		Vector3(0,1,0),			// p0
		Vector3(0,0,0),			// p1
		Vector3(1,0,0),			// p2
		0.5f,
		false
	),

	RayTriTestClass				// ray going down the -z axis, hitting center of edge between p0 and p2
	(
		Vector3(0,0,5),			// ray start
		Vector3(0,0,-5),			// ray end
		Vector3(-1,1,0),			// p0
		Vector3(-1,-1,0),			// p1
		Vector3(1,-1,0),			// p2
		0.5f,
		false
	),
};

/************************************************************************************************
**
**  Ray - AABox Test Cases
**
************************************************************************************************/

class RayAABoxTestClass
{
public:
	Vector3			P0;
	Vector3			P1;
	AABoxClass		Box;
	float				Fraction;
	bool				StartBad;
	LineSegClass	LineSeg;
	TriClass			Tri;

	RayAABoxTestClass
	(
		const Vector3 p0,					// p0 of ray
		const Vector3 p1,					// p1 of ray
		const Vector3 center,			// center of the box
		const Vector3 extent,			// extent of the box
		float frac,							// expected fraction
		bool sol								// expected start solid
	) :
		P0(p0),P1(p1),Box(center,extent),Fraction(frac),StartBad(sol),LineSeg(P0,P1)
	{
	}
};

RayAABoxTestClass RayAABoxTestCases[] =
{
	RayAABoxTestClass
	(
		Vector3(5,0,0),					// p0 of ray
		Vector3(0,0,0),					// p1 of ray
		Vector3(0,0,0),					// center of the box
		Vector3(1,1,1),					// extent of the box
		4.0f / 5.0f,						// expected fraction
		false
	),
	RayAABoxTestClass
	(
		Vector3(-2,-5,0),					// p0 of ray
		Vector3(0,0,0),					// p1 of ray
		Vector3(0,0,0),					// center of the box
		Vector3(2,2,2),					// extent of the box
		3.0f / 5.0f,						// expected fraction
		false
	),
	RayAABoxTestClass
	(
		Vector3(-2,5,0),					// p0 of ray
		Vector3(0,0,0),					// p1 of ray
		Vector3(0,0,0),					// center of the box
		Vector3(10,1,1),					// extent of the box
		4.0f / 5.0f,						// expected fraction
		false
	),
	RayAABoxTestClass						// ray just clips the corner of the box (in x-y plane)
	(
		Vector3(2,0,0),					// p0 of ray
		Vector3(0,2,0),					// p1 of ray
		Vector3(0,0,0),					// center of the box
		Vector3(1,1,1),					// extent of the box
		0.5f,									// expected fraction
		false
	),
	RayAABoxTestClass						// ray misses box
	(
		Vector3(1.01f,-3,0),				// p0 of ray
		Vector3(1.01f,3,0),				// p1 of ray
		Vector3(0,0,0),					// center of the box
		Vector3(1,1,1),					// extent of the box
		1.0f,									// expected fraction
		false
	),
};


void Test_Rays(void)
{
	CastResultStruct result;
	CastResultStruct result_check;
	uint64_t cycles;
	int numtests;
	int i;

	/*
	** Test the Cast_Ray function with a bunch of ray-triangle pairs.
	*/
	Print_Title("Testing ray-triangle intersection.");

	numtests = sizeof(RayTriTestCases)/sizeof(RayTriTestClass);
	for (i=0; i<numtests; i++) {

		RayTriTestClass * testcase = &(RayTriTestCases[i]);

		result.Fraction = 1.0;
		result.StartBad = false;
		result.Normal.Set(0,0,0);

		cycles = Get_CPU_Clock();
		CollisionMath::Collide(testcase->LineSeg,testcase->Tri,&result);
		cycles = Get_CPU_Clock() - cycles;

		printf("Ray -> Tri cycles: %" PRIu64 "\n", cycles);

		CHECK(i,(fabs(testcase->Fraction - result.Fraction) < 0.001f));
	}

	/*
	** Test the Cast_Ray function with a bunch of ray-aabox pairs.
	*/
	Print_Title("Testing ray-aabox intersection.");

	numtests = sizeof(RayAABoxTestCases)/sizeof(RayAABoxTestClass);
	for (i=0; i<numtests; i++) {

		RayAABoxTestClass * testcase = &(RayAABoxTestCases[i]);

		result.Fraction = 1.0;
		result.StartBad = false;
		result.Normal.Set(0,0,0);

		cycles = Get_CPU_Clock();
		CollisionMath::Collide(testcase->LineSeg,testcase->Box,&result);
		cycles = Get_CPU_Clock() - cycles;

		printf("Ray -> AABox cycles: %" PRIu64 "\n", cycles);

		CHECK(i,(fabs(testcase->Fraction - result.Fraction) < 0.001f));
	}

	/*
	** Test the Cast_Ray function on some random oriented boxes
	*/
	Print_Title("Testing ray-obbox intersection.");
	for (i=0; i<30; i++) {

		result.Fraction = 1.0;
		result.StartBad = false;
		result.Normal.Set(0,0,0);

		// create a random box
		OBBoxClass box;
		box.Init_Random(1.0f,2.75f);

		// create a random line
		LineSegClass line;
		line.Set_Random(Vector3(-3,-3,-3),Vector3(3,3,3));

		// use the ray-box test
		cycles = Get_CPU_Clock();
		CollisionMath::Collide(line,box,&result);
		cycles = Get_CPU_Clock() - cycles;

		// double-check the result
		result_check.Fraction = 1.0f;
		result_check.StartBad = false;
		result_check.Normal.Set(0,0,0);
		line_box_test(line,box,&result_check);

		// print what happened
		bool passed = false;
		if (fabs(result_check.Fraction - result.Fraction) < 0.001f) {
			passed = true;
		}

		printf("test %3d  cycles: %6" PRIu64 "\tfraction: %1.8f\t",i,cycles,result.Fraction);
		if (passed) {
			printf("passed\n");
		} else if (result.StartBad) {
			printf("(start inside)\n");
		} else {
			printf("<<failed!>>\n");
		}
	}


	/*
	** Test the Overlap_Test function on some random AABoxes
	*/
	uint64_t total_tests = 0;
	uint64_t total_cycles = 0;
	uint64_t outside_tests = 0;
	uint64_t outside_cycles = 0;
	uint64_t overlap_tests = 0;
	uint64_t overlap_cycles = 0;

	CollisionMath::OverlapType overlap;
	Print_Title("Testing ray-aabox overlap.");
	for (i=0; i<10000; i++) {

		// create a random box
		AABoxClass box;
		box.Init_Random(-1.0f,1.0f,1.0f,2.0f);

		// create a random line
		const float DIMENSION = 5.0f;
		LineSegClass line;
		line.Set_Random(Vector3(-DIMENSION,-DIMENSION,-DIMENSION),Vector3(DIMENSION,DIMENSION,DIMENSION));

		// use the overlap_test
		cycles = Get_CPU_Clock();
		overlap = CollisionMath::Overlap_Test(box,line);
		cycles = Get_CPU_Clock() - cycles;

		// print what happened
		printf("test %3d  cycles: %6" PRIu64 "\tfraction: %1.8f\t",i,cycles,result.Fraction);
		if (overlap == CollisionMath::OUTSIDE) {
			printf("outside\n");
		} else if (overlap == CollisionMath::INSIDE) {
			printf("inside\n");
		} else {
			printf("overlapped\n");
		}

		// stats
		total_tests++;
		total_cycles += cycles;
		if (overlap == CollisionMath::OUTSIDE) {
			outside_tests++;
			outside_cycles += cycles;
		} else {
			overlap_tests++;
			overlap_cycles += cycles;
		}
	}

	printf("Total Tests:              %" PRIu64 "\n",total_tests);
	printf("Average Cycles:           %f\n",(float)total_cycles / (float)total_tests);
	printf("Outside Tests:            %" PRIu64 "\n",outside_tests);
	printf("Average Outside Cycles:   %f\n",(float)outside_cycles / (float)outside_tests);
	printf("Overlap Tests:            %" PRIu64 "\n",overlap_tests);
	printf("Average Overlap Cycles:   %f\n",(float)overlap_cycles / (float)overlap_tests);

}




void line_box_test(const LineSegClass & line,const OBBoxClass & box,CastResultStruct * res)
{
	Vector3 point[8];

	point[0] = box.Center + box.Basis * Vector3( box.Extent.X, box.Extent.Y, box.Extent.Z);
	point[1] = box.Center + box.Basis * Vector3(-box.Extent.X, box.Extent.Y, box.Extent.Z);
	point[2] = box.Center + box.Basis * Vector3(-box.Extent.X,-box.Extent.Y, box.Extent.Z);
	point[3] = box.Center + box.Basis * Vector3( box.Extent.X,-box.Extent.Y, box.Extent.Z);

	point[4] = box.Center + box.Basis * Vector3( box.Extent.X, box.Extent.Y,-box.Extent.Z);
	point[5] = box.Center + box.Basis * Vector3(-box.Extent.X, box.Extent.Y,-box.Extent.Z);
	point[6] = box.Center + box.Basis * Vector3(-box.Extent.X,-box.Extent.Y,-box.Extent.Z);
	point[7] = box.Center + box.Basis * Vector3( box.Extent.X,-box.Extent.Y,-box.Extent.Z);

	static int triverts[12][3] =
	{
		{ 0,1,2 },
		{ 0,2,3 },
		{ 4,5,1 },
		{ 4,1,0 },
		{ 1,5,6 },
		{ 1,6,2 },
		{ 3,2,6 },
		{ 3,6,7 },
		{ 4,0,3 },
		{ 4,3,7 },
		{ 4,7,6 },
		{ 4,6,5 }
	};

	// first, check if ray starts inside box?

	// now, check for collision with each triangle
	for (int i=0; i<12; i++) {

		TriClass tri;
		Vector3 normal;

		tri.V[0] = &point[triverts[i][0]];
		tri.V[1] = &point[triverts[i][1]];
		tri.V[2] = &point[triverts[i][2]];
		tri.N = &normal;
		tri.Compute_Normal();

		CollisionMath::Collide(line,tri,res);
	}
}
