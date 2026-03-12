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
 *                     $Archive:: /Commando/Code/Tests/mathtest/mempooltest.cpp               $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 7/29/99 6:01p                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "mempool.h"
#include <stdio.h>


class B : public AutoPoolClass<B,5>
{
public:
//	B(void) : AutoPoolClass<B,5>() { /*x = 2; y = 3;*/ }
	int	x;
	int	y;
};

class D
{
public:
	D(void);
	~D(void);

	char * ptr;
	int x;
};

D::D(void)
{
	ptr = new char[500];
	x = 7;
}
D::~D(void)
{
	delete[] ptr;
	x = 2;
}

DEFINE_AUTO_POOL(B,5);
ObjectPoolClass<D,5> DAllocator;



void test_mempool(void)
{
	// allocate and delete a single 'B'
	B * test0 = new B;
	delete test0;

	// allocate and delete a single 'D' make sure constructor
	// and destructor are called.
	D * test1 = DAllocator.Allocate_Object();
	DAllocator.Free_Object(test1);

#define	NUM_OBJECTS		6
	B * Test[ NUM_OBJECTS ];
	int i;
	for ( i = 0; i < NUM_OBJECTS; i++ ) 		Test[i] = new B;
	for ( i = 0; i < NUM_OBJECTS; i++ ) 		printf( "%d) %p\n", i, Test[i] );
	for ( i = 0; i < NUM_OBJECTS; i++ )			delete Test[i];
	for ( i = 0; i < NUM_OBJECTS; i++ ) 		Test[i] = new B;
	for ( i = 0; i < NUM_OBJECTS; i++ ) 		printf( "%d) %p\n", i, Test[i] );
	for ( i = 0; i < NUM_OBJECTS; i++ )			delete Test[i];
}

