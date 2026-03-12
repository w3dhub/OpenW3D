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
 *                     $Archive:: /Commando/Code/Tests/mathtest/uarraytest.cpp                $*
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 3/17/00 9:08a                                               $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "uarraytest.h"
#include "output.h"
#include "vector3.h"
#include "uarray.h"


Vector3 InputVectors[] =
{
	Vector3(0,0,0),
	Vector3(1,0,0),
	Vector3(1,0,0),
	Vector3(1,0,0),
	Vector3(1,0,0),
	Vector3(1,0,0),
	Vector3(5.4f,2.3f,4.5f),
	Vector3(0,0,0),
	Vector3(5.4f,2.3f,4.5f),
};


class VectorHasherClass : public HashCalculatorClass<Vector3>
{
public:

	virtual bool	Items_Match(const Vector3 & a, const Vector3 & b)
	{
		// looking for an exact match for normals...
		return ((a.X == b.X) && (a.Y == b.Y) && (a.Z == b.Z));
	}

	virtual void	Compute_Hash(const Vector3 & item)
	{
		HashVal = (int)(item.X*12345.6f + item.Y*1714.38484f + item.Z*27561.3f)&1023;
	}

	virtual int		Num_Hash_Bits(void)
	{
		return 12;  // 12bit hash value.
	}

	virtual int		Num_Hash_Values(void)
	{
		return 1;	// only one hash value for normals, require *exact* match
	}

	virtual int		Get_Hash_Value(int index)
	{
		return HashVal;
	}

private:

	int HashVal;

};

void Test_Uarray(void)
{

	VectorHasherClass vectorhasher;
	UniqueArrayClass<Vector3> UniqueVectors(4,4,&vectorhasher);

	int num_input_vectors = sizeof(InputVectors) / sizeof(Vector3);
	int add_index;

	for (int vi=0; vi<num_input_vectors; vi++) {
		add_index = UniqueVectors.Add(InputVectors[vi]);
	}
}
