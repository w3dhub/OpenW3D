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
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : LightMap                                                     *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tool $*
 *                                                                                             *
 *                      $Author:: Ian_l               $*
 *                                                                                             *
 *                     $Modtime:: 8/10/00 4:10p       $*
 *                                                                                             *
 *                    $Revision:: 2                                                         $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef _TRIANGLE_H
#define _TRIANGLE_H

// Includes.
#include "vector3.h"
#include "vector2.h"

class TextureNameNode;

class Triangle
{
	public:

		enum VerticesCountEnum {
			VERTICES_COUNT = 3	// No. of vertices in a triangle. Fixed for this class.
		};

		struct TriangleVertices
		{
			Vector3 Point;
			Vector2 UV;

			bool operator == (const TriangleVertices &t) {
				return ((Point == t.Point) && (UV == t.UV));
			}
		};

		// Equality operator.
		bool operator == (const Triangle &t) {
			return ((Normal == t.Normal) &&
					  (Vertices [0] == t.Vertices [0]) &&
					  (Vertices [1] == t.Vertices [1]) &&
					  (Vertices [2] == t.Vertices [2]));
	  	}

		// Inequality operator.
		bool operator != (const Triangle &t) {
			return (!(*this == t));
		}

		bool Is_Equivalent (const Triangle &triangle) const;
		bool Abuts (const Triangle &triangle) const;

		Vector3			   Normal;							// Face normal.
		TriangleVertices	Vertices [VERTICES_COUNT];	// Location of triangle in object space and texture space.
		TextureNameNode  *TextureNameNodePtr;			// List of textures associated with this triangle.
		unsigned				TextureID;						// ID to uniquely identify the texture.
};


/***********************************************************************************************
 * Triangle::Is_Equivalent --																						  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *  06/13/00    IML : Created.                                                                 *
 *=============================================================================================*/
inline bool Triangle::Is_Equivalent (const Triangle &triangle)	const
{
	bool foundequivalent;
	bool hasequivalent [VERTICES_COUNT];

	// Rules for equivalence are:
	// (a) Normal vectors must match
	// (b) Points must match (but may not necessarily be in the same order).

	if (Normal != triangle.Normal) return (false);

	hasequivalent [0] = false;
	hasequivalent [1] = false;
	hasequivalent [2] = false;
	for (unsigned a = 0; a < VERTICES_COUNT; a++) {
		foundequivalent = false;
		for (unsigned b = 0; b < VERTICES_COUNT; b++) {
			if (!hasequivalent [b]) {
				if (Vertices [a].Point == triangle.Vertices [b].Point) {
					hasequivalent [b] = true;
					foundequivalent = true;
					break;
				}
			}
		}
		if (!foundequivalent) return (false);
	}

	// Triangles are equivalent.
	return (true);
}


/***********************************************************************************************
 * Triangle::Abuts --																								  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *  08/03/00    IML : Created.                                                                 *
 *=============================================================================================*/
inline bool Triangle::Abuts (const Triangle &triangle) const
{
 	bool		iscoincident [VERTICES_COUNT];
	unsigned	coincidentcount;

	// Rules for abutting are:
	// (1) Exactly two points must match.

	iscoincident [0] = false;
	iscoincident [1] = false;
	iscoincident [2] = false;
	coincidentcount  = 0;
	for (unsigned a = 0; a < VERTICES_COUNT; a++) {
		for (unsigned b = 0; b < VERTICES_COUNT; b++) {
			if (!iscoincident [b]) {
				if (Vertices [a].Point == triangle.Vertices [b].Point) {
					iscoincident [b] = true;
					coincidentcount++;
					break;
				}
			}
		}
	}

	return (coincidentcount == 2);
}


class PackingTriangle : public Triangle
{
	public:

		// Public data.
		Vector2  PackedUVs [Triangle::VERTICES_COUNT];
		unsigned PackedTextureID;
};


#endif // _TRIANGLE_H
