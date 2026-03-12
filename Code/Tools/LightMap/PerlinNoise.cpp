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
 *                     $Modtime:: 2/14/01 2:21p       $*
 *                                                                                             *
 *                    $Revision:: 3                                                         $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include <limits.h>
#include "PerlinNoise.h"
#include "Random.h"
#include "WWMath.h"

// Static data.
float PerlinNoise::_NoiseTable [NOISE_TABLE_SIZE];


/***********************************************************************************************
 * PerlinNoise::PerlinNoise --																					  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/05/00    IML : Created.                                                                *
 *=============================================================================================*/
PerlinNoise::PerlinNoise()
{
	static bool _initialized = false;

	if (!_initialized) {

		Random3Class  randomnumber (0x3f198440, 0x92012324);
		float			 *noisetableptr;

		// Initialize the noise table with random values.
		noisetableptr = &_NoiseTable [0];
		for (unsigned i = 0; i < NOISE_TABLE_SIZE; i++) {
			*noisetableptr = randomnumber (0, 10000) * 0.0001f;
			noisetableptr++;
		}

		_initialized = true;
	}
}


/***********************************************************************************************
 * PerlinNoise::Value --																							  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/05/00    IML : Created.                                                                *
 *=============================================================================================*/
float PerlinNoise::Noise (int x, int y, int z)
{
	static unsigned char _permute [NOISE_TABLE_SIZE] = {
		225, 155, 210, 108, 175, 199, 221, 144, 203, 116,  70, 213,  69, 158,  33, 252,
		  5,  82, 173, 133, 222, 139, 174,  27,   9,  71,  90, 246,  75, 130,  91, 191,
		169, 138,   2, 151, 194, 235,  81,   7,  25, 113, 228, 159, 205, 253, 134, 142,
		248,  65, 224, 217,  22, 121, 229,  63,  89, 103,  96, 104, 156,  17, 201, 129,
		 36,   8, 165, 110, 237, 117, 231,  56, 132, 211, 152,  20, 181, 111, 239, 218,
		170, 163,  51, 172, 157,  47,  80, 212, 176, 250,  87,  49,  99, 242, 136, 189,
		162, 115,  44,  43, 124,  94, 150,  16, 141, 247,  32,  10, 198, 223, 255,  72,
		 53, 131,  84,  57, 220, 197,  58,  50, 208,  11, 241,  28,   3, 192,  62, 202,
		 18, 215, 153,  24,  76,  41,  15, 179,  39,  46,  55,   6, 128, 167,  23, 188,
		106,  34, 187, 140, 164,  73, 112, 182, 244, 195, 227,  13,  35,  77, 196, 185,
		 26, 200, 226, 119,  31, 123, 168, 125, 249,  68, 183, 230, 177, 135, 160, 180,
		 12,   1, 243, 148, 102, 166,  38, 238, 251,  37, 240, 126,  64,  74, 161,  40,
		184, 149, 171, 178, 101,  66,  29,  59, 146,  61, 254, 107,  42,  86, 154,   4,
		236, 232, 120,  21, 233, 209,  45,  98, 193, 114,  78,  19, 206,  14, 118, 127,
		 48,  79, 147,  85,  30, 207, 219,  54,  88, 234, 190, 122,  95,  67, 143, 109,
		137, 214, 145,  93,  92, 100, 245,   0, 216, 186,  60,  83, 105,  97, 204,  52
	};

	#define PERMUTE(x) _permute [(x) & (NOISE_TABLE_MASK)]

	unsigned ux, uy, uz;

	ux = x + INT_MAX + 1;
	uy = y + INT_MAX + 1;
	uz = z + INT_MAX + 1;

	return (_NoiseTable [PERMUTE ((ux) + PERMUTE ((uy) + PERMUTE (uz)))]);

	#undef PERMUTE
}


/***********************************************************************************************
 * PerlinNoise::Noise --																							  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/05/00    IML : Created.                                                                *
 *=============================================================================================*/
float PerlinNoise::Noise (const Vector3 &point)
{
	int	ix, iy, iz;
	float xf, yf, zf;
	float a, b, c, d, e, f, g, h;
	float	x0, x1, x2, x3;
	float y0, y1;

	ix = floorf (point.X);
	iy = floorf (point.Y);
	iz = floorf (point.Z);

	xf = SmoothStep (point.X - ix);
	yf = SmoothStep (point.Y - iy);
	zf = SmoothStep (point.Z - iz);

	a  = Noise (ix,	  iy,		 iz);
	b  = Noise (ix + 1, iy,		 iz);
	c  = Noise (ix + 1, iy + 1, iz);
	d  = Noise (ix,	  iy + 1, iz);
	e  = Noise (ix,	  iy,		 iz + 1);
	f  = Noise (ix + 1, iy,		 iz + 1);
	g  = Noise (ix + 1, iy + 1, iz + 1);
	h  = Noise (ix,	  iy + 1, iz + 1);

	x0 = WWMath::Lerp (a, b, xf);
	x1 = WWMath::Lerp (d, c, xf);
	x2 = WWMath::Lerp (e, f, xf);
	x3 = WWMath::Lerp (h, g, xf);

	y0 = WWMath::Lerp (x0, x1, yf);
	y1 = WWMath::Lerp (x2, x3, yf);

	return (WWMath::Lerp (y0, y1, zf));
}


/***********************************************************************************************
 * PerlinNoise::Value --																							  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/05/00    IML : Created.                                                                *
 *=============================================================================================*/
float PerlinNoise::Value (const Vector3 &point)
{
	const unsigned octavecount	= 5;

	float value, amplitudesum, frequency;

	value			 = 0.0f;
	amplitudesum = 0.0f;
	frequency	 = 0.5f;
	for (unsigned octave = 0; octave < octavecount; octave++) {

		float oofrequency;

		oofrequency = 1.0f / frequency;
		value += Noise (point * frequency) * oofrequency;
		amplitudesum += oofrequency;
		frequency *= 2.17f;
	}

	// Normalize.
	return (value * (1.0f / amplitudesum));
}

