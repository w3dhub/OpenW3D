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
 *                     $Modtime:: 9/08/00 5:09p       $* 
 *                                                                                             * 
 *                    $Revision:: 2                                                         $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef PERLIN_NOISE_H
#define PERLIN_NOISE_H

// Includes.
#include "ProceduralTexture.h"
#include "vector3.h"

// Defines.
#define NOISE_TABLE_SIZE	256					  	  // NOTE: Must be a power of 2.	
#define NOISE_TABLE_MASK	NOISE_TABLE_SIZE - 1

class PerlinNoise : public ProceduralTexture {

	public:

		PerlinNoise();
		float Value (const Vector3 &point);

	protected:

		float SmoothStep (float x) {return (x * x * (3.0f - (2.0f * x)));}
		float Noise (int x, int y, int z);	
		float Noise (const Vector3 &point);

	private:

		static float _NoiseTable [NOISE_TABLE_SIZE];
};


#endif // PERLIN_NOISE_H

