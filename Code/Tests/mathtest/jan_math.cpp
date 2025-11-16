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

/*

	This file contains some math functions that Jan worked on while visiting westwood
	between Meltdown and Siggraph 2001.

*/


#include <iostream>
#include <iomanip>
#include <cmath>


#include "p_timer.h"
#include "wwmath.h"


using namespace std;

#undef for
#define for if(false); else for

/*		(1-u)(1-u)(1-u)
		---------------				1/6			0/6
			   6

		3*u*u*u - 6u*u +4
		---------------				4/6			1/6
			   6

		-3*u*u*u + 3*u*u + 3*u + 1
		---------------				1/6			4/6
			   6

		u*u*u
		---------------				0/6			1/6
			   6
*/

const double pi = 3.1415926535897932384626433832795;


const int SINTABLESIZE = 16;

float sinTable[SINTABLESIZE+1][3];
/*
float sinTable[SINTABLESIZE+1][3] =
{
              0.f,  0.0327249329938f,  0.0654498139838f,
 0.0980171403296f,   0.130584524375f,    0.16299423692f,
  0.195090322016f,    0.22718650856f,   0.258968953094f,
  0.290284677254f,   0.321600570763f,   0.352449632798f,
  0.382683432365f,   0.412917427848f,   0.442536062284f,
  0.471396736826f,        0.5002577f,   0.528360611467f,
   0.55557023302f,   0.582780177536f,   0.609096734804f,
  0.634393284164f,   0.659690191419f,   0.683966978964f,
  0.707106781187f,   0.730247032377f,   0.752250161646f,
  0.773010453363f,    0.79377112016f,   0.813288873188f,
  0.831469612303f,   0.849650815039f,   0.866495135964f,
  0.881921264348f,   0.897347904711f,   0.911356511164f,
  0.923879532511f,   0.936403079986f,   0.947441092993f,
  0.956940335732f,   0.966440181792f,   0.974401217255f,
  0.980785280403f,   0.987169903631f,   0.991977366409f,
  0.995184726672f,   0.998392578191f,     1.0000003005f,
              1.f,     1.0000003005f,   0.998392578191f,
};
*/
void init_bez_sin()
{
	for(int i=0; i<SINTABLESIZE+1; i++)
	{
		double x = sin(pi/2*(i*3)/(SINTABLESIZE*3));
		double y = sin(pi/2*(i*3+1)/(SINTABLESIZE*3)); // * 1.00053f;
		double z = sin(pi/2*(i*3+2)/(SINTABLESIZE*3)); // * 1.00053f;
		double w = sin(pi/2*(i*3+3)/(SINTABLESIZE*3));

		double mse = 4.0;

		double my = 1.0;
		double mz = 1.0;


// * nu2 * nu;
// nu2;
// u * nu;
// * u2 * u;

		double wy = 4.0;
		
		for(double dy=1.000178; dy<1.000537; dy+=0.0000001)
		{
			bool b = false;
			double wz = 4.0;
			for(double dz=1.000534; dz<1.000715; dz+=0.0000001)
			{

	
				double by = x * 8.0 / 27.0 + 
							y * dy * 12.0 / 27.0 +
							z * dz * 6.0 / 27.0 +
							w / 27.0;

				double bz = x / 27.0 + 
							y * dy * 6.0 / 27.0 +
							z * dz * 12.0 / 27.0 +
							w * 8.0 / 27.0;

				if(y*dy < 0.0 || y*dy > 1.0)
					continue;
				if(z*dz < 0.0 || z*dz > 1.0)
					continue;

				double yerr, zerr;

//				if(by > y)
//					yerr = by/y;
//				else
					yerr = y/by;

//				if(bz > z)
//					zerr = bz/z;
//				else
					zerr = z/bz;

				yerr -= 1.0;
				zerr -= 1.0;

				double se = yerr*yerr + zerr*zerr;

				if(mse > se)
				{
					wy = se;
					wz = se;
					mse = se;
					my = dy;
					mz = dz;
					b = false;
				}
				else
				{
//					if(se > wy)
//						b = true;
//					if(se > wz)
//						break;
				}
			}
//			if(b)
//				break;
		}

		sinTable[i][0] = float(x);
		sinTable[i][1] = float(y * my);
		sinTable[i][2] = float(z * mz);

		cout << setprecision(12);
		cout << setw(16) << x << "f, " << setw(16) << y*my << "f, " << setw(16) << z*mz << "f," << endl;
	}
}

const int ACOSTABLESIZE = 32;

//float acosTable[ACOSTABLESIZE+1][3];

float acosTable[ACOSTABLESIZE+1][3] =
{
	0, 0, 0,
	0, 0, 0,
	0, 0, 0,
	0, 0, 0,
	0, 0, 0,
	0, 0, 0,
	0, 0, 0,
	0, 0, 0,
   2.09439510239f,    2.07033956061f,    2.04678164613f,
   2.02361292154f,    2.00044498842f,    1.97766674853f,
   1.95519310129f,    1.93271994669f,    1.91055213001f,
   1.88862003072f,    1.86668845355f,    1.84499333386f,
   1.82347658194f,    1.80196011523f,     1.7806223216f,
   1.75941271297f,    1.73820326172f,    1.71712229397f,
   1.69612415796f,    1.67512622765f,    1.65421121018f,
   1.63333708859f,    1.61246303408f,    1.59162963139f,
   1.57079632679f,    1.54996304584f,    1.52912960772f,
     1.508255565f,    1.48738138962f,    1.46646655946f,
   1.44546849563f,    1.42447034954f,    1.40338938542f,
   1.38217994062f,    1.36097042076f,    1.33963248005f,
   1.31811607165f,    1.29659929615f,    1.27490425438f,
   1.25297262287f,    1.23104063033f,    1.20887262684f,
    1.1863995523f,    1.16392587108f,    1.14114765337f,
   1.11797973205f,    1.09481095108f,    1.07125317716f,
    1.0471975512f,        1.0231405f,   0.998586616212f,
	0, 0, 0,
	0, 0, 0,
	0, 0, 0,
	0, 0, 0,
	0, 0, 0,
	0, 0, 0,
	0, 0, 0,
	0, 0, 0
};

double calc_cos_plus_45(double cx)
{
	double sx = sqrt(1-cx*cx);
	cx = cx*cos(pi/4) - sx*sin(pi/4);

	return cx;
}

void init_bez_acos()
{
	for(int i=0; i<ACOSTABLESIZE; i++)
	{
//		if(i < ACOSTABLESIZE/4 || i > ACOSTABLESIZE/4*3)
//			continue;
		double x = acos(float(i*3)/(ACOSTABLESIZE*3)*2-1);
		double y = acos(float(i*3+1)/(ACOSTABLESIZE*3)*2-1); // * 1.00053f;
		double z = acos(float(i*3+2)/(ACOSTABLESIZE*3)*2-1); // * 1.00053f;

		double w;
		if(i != ACOSTABLESIZE-1)
			w = acos(float(i*3+3)/(ACOSTABLESIZE*3)*2-1);
		else
			w = 0.0;//-acos(float(i*3+1)/(ACOSTABLESIZE*3)*2-1);

		double mse = 4.0;

		double my = 1.0;
		double mz = 1.0;

		double wy = 4.0;
		
		for(double dy=0.9996944; dy<=1.0006926; dy+=16.0/8388608)
		{
			bool b = false;
			double wz = 4.0;
			for(double dz=0.9997872; dz<=1.0011092; dz+=16.0/8388608)
			{
				double by = x * 8.0 / 27.0 + 
							y * dy * 12.0 / 27.0 +
							z * dz * 6.0 / 27.0 +
							w / 27.0;

				double bz = x / 27.0 + 
							y * dy * 6.0 / 27.0 +
							z * dz * 12.0 / 27.0 +
							w * 8.0 / 27.0;

				if(y*dy < 0.0 || y*dy > pi)
					continue;
				if(z*dz < 0.0 || z*dz > pi)
					continue;

				double yerr, zerr;

//				if(by > y)
//					yerr = by/y;
//				else
					yerr = y/by;

//				if(bz > z)
//					zerr = bz/z;
//				else
					zerr = z/bz;

				yerr -= 1.0;
				zerr -= 1.0;

				double se = yerr*yerr + zerr*zerr;

				if(mse > se)
				{
					wy = se;
					wz = se;
					mse = se;
					my = dy;
					mz = dz;
					b = false;
				}
				else
				{
//					if(se > wy)
//						b = true;
//					if(se > wz)
//						break;
				}
			}
//			if(b)
//				break;
		}

		acosTable[i][0] = float(x);
		acosTable[i][1] = float(y * my);
		acosTable[i][2] = float(z * mz);

		cout << setprecision(12);
		cout << setw(16) << x << "f, " << setw(16) << y*my << "f, " << setw(16) << z*mz << "f," << endl;
//		cout << my << " " << mz << endl;
	}

	acosTable[ACOSTABLESIZE][0] = 0.f;
}


float bez_sin(float x)
{

	x *= (SINTABLESIZE*4)/(pi*2);

	int		ix0 = int(floor(x));
	float	u = x - ix0;
	float	nu = 1.f-u;

	float sign = 1.f;

	ix0 &= SINTABLESIZE*4-1;

	if(ix0 >= SINTABLESIZE*2)
	{
		sign = -1.f;
		ix0 -= SINTABLESIZE*2;
	}

	if(ix0 >= SINTABLESIZE)
	{
		ix0 = (SINTABLESIZE-1)-(ix0-SINTABLESIZE);

		float t = u;
		u = nu;
		nu = t;
	}

	int ix1 = ix0+1;
//	ix0 &= SINTABLESIZE-1;
//	ix1 &= SINTABLESIZE-1;

	float u2 = u*u;
	float nu2 = nu*nu;

	float r0 = sinTable[ix0][0] * nu2 * nu;
	float r1 = sinTable[ix0][1] * 3 * u * nu2;
	float r2 = sinTable[ix0][2] * 3 * u2 * nu;
	float r3 = sinTable[ix1][0] * u2 * u;

	r0 += r1;
	r2 += r3;

	return (r0+r2) * sign;
}

float bez_acos(float x)
{
//	double sx = sqrt(1-x*x);
//	x = x*cos(pi/4) - sx*sin(pi/4);
	x = calc_cos_plus_45(x);

	x += 1.f;
	x *= ACOSTABLESIZE/2;

	int		ix0 = int(floor(x));

	float	u = x - ix0;
	float	nu = 1.f-u;

	if(ix0 < 0)
		return pi-pi/4;			// -infinite...
	if(ix0 >= ACOSTABLESIZE)
		return 0.f;

	int ix1 = ix0+1;

	float u2 = u*u;
	float nu2 = nu*nu;

	float r0 = acosTable[ix0][0] * nu2 * nu;
	float r1 = acosTable[ix0][1] * 3 * u * nu2;
	float r2 = acosTable[ix0][2] * 3 * u2 * nu;
	float r3 = acosTable[ix1][0] * u2 * u;

	r0 += r1;
	r2 += r3;

	return (r0+r2)-pi/4;
}


float mcos(float c, float m)
{
	float ac = acos(c);

	float c1 = cos(ac*m);
	float c2 = cos(ac*(1-m));

	float c3 = c * cos(m);

	return c1;
}


float cheb_asin(float x)
{
	float table[] =
	{
		1.101460576f,
		+.5764093225f,
		-.09735417608f,
		+.003083774398f,
		-.000844493744f,
		+.005722363752f,
		-.005962135984f
	};

	float r = table[0] + table[1]*x;

	r -= sqrt(1-x*x);

	x = 2.f*x-1.f;

	float x2 = x;

	x *= x;

	for(int i=2; i<sizeof(table)/sizeof(table[0]); i++)
	{
		r += x * table[i];
		x *= x2;
	}

	return r;
}

float taylor_acos(float x)
{
/*
		(1)						/(2*3),
		(3)						/(2*4*5),
		(3*5)					/(2*4*6*7),
		(3*5*7)					/(2*4*6*8*9),
		(3*5*7*9)				/(2*4*6*8*10*11),
		(3*5*7*9*11)			/(2*4*6*8*10*12*13),
		(3*5*7*9*11*13)			/(2*4*6*8*10*12*14*15),
		(3*5*7*9*11*13*15)		/(2*4*6*8*10*12*14*16*17),
		(3*5*7*9*11*13*15*17)	/(2*4*6*8*10*12*14*16*18*19),
*/
	
	float table[] =
	{
		1.f,
		float(1)						/(2*3),
		float(3)						/(2*4*5),
		float(5)						/(2*4*2*7),
		float(5*7)						/(2*4*2*8*9),
		float(7*9)						/(2*4*2*8*2*11),
		float(7*9*11)					/(2*4*2*8*2*12*13),
		float(9*11*13)					/(2*4*2*8*2*12*2*15),
		float(9*11*13*15)				/(2*4*2*8*2*12*2*16*17),
		float(11*13*15*17)				/(2*4*2*8*2*12*2*16*2*19),
	};


//	x = sqrt((1.f+x)/2.f);

	bool flip = false;

	if(x > 0.707106781186547524400844362104849f)
	{
		x = 0.707106781186547524400844362104849f - (x-0.707106781186547524400844362104849f);
		flip = true;
	}
	

	float r = 1.57079632679489661923132169163975f;
	float x2 = x*x;

	for(int i=0; i<sizeof(table)/sizeof(table[0]); i++)
	{
		r -= x * table[i];
		x *= x2;
	}

	if(flip)
	{
		r = 1.57079632679489661923132169163975f-r;
	}

	return r;
}

/*
taylor acos

                         3      5      7       9       11
              %PI       X    3 X    5 X    35 X    63 X
              --- - X - -- - ---- - ---- - ----- - ------ 
               2        6     40    112    1152     2816



taylor(sin(x),x,0,9);	

                           3    5      7       9
                          X    X      X       X
                      X - -- + --- - ---- + ------ + . . .
                          3!    5!    7!     9!


taylor cos

                            2    4    6      8
                           X    X    X      X
                       1 - -- + -- - --- + ----- + . . .
                           2!   4!    6!     8!

*/
int intChop(const float& f)
{
    int a	= *reinterpret_cast<const int*>(&f);		// take bit pattern of float into a register
    int sign	= (a>>31);								// sign = 0xFFFFFFFF if original value is negative, 0 if positive
    int mantissa	= (a&((1<<23)-1))|(1<<23);			// extract mantissa and add the hidden bit
    int exponent	= ((a&0x7fffffff)>>23)-127;			// extract the exponent
    int r	= (unsigned int(mantissa)<<8)>>(31-exponent);	// ((1<<exponent)*mantissa)>>24 -- (we know that mantissa > (1<<24))
    return ((r ^ (sign)) - sign ) &~ (exponent>>31);	// add original sign. If exponent was negative, make return value 0.
}

int intFloor (const float& f)
{
	int a			= *reinterpret_cast<const int*>(&f);			// take bit pattern of float into a register
	int sign		= (a>>31);										// sign = 0xFFFFFFFF if original value is negative, 0 if positive
	a&=0x7fffffff;													// we don't need the sign any more

	int exponent	= (a>>23)-127;									// extract the exponent
	int expsign	= ~(exponent>>31);									// 0xFFFFFFFF if exponent is positive, 0 otherwise
	int imask		= ( (1<<(31-(exponent))))-1;					// mask for true integer values
	int mantissa	= (a&((1<<23)-1));								// extract mantissa (without the hidden bit)
	int r			= (unsigned int(mantissa|(1<<23))<<8)>>(31-exponent);	// ((1<<exponent)*(mantissa|hidden bit))>>24 -- (we know that mantissa > (1<<24))

	r = ((r & expsign) ^ (sign)) + ((!((mantissa<<8)&imask)&(expsign^((a-1)>>31)))&sign);	// if (fabs(value)<1.0) value = 0; copy sign; if (value < 0 && value==(int)(value)) value++;
	return r;
}

const int CACHE_TRASH_BUFFER_SIZE = 4024000;
int * CacheTrashBuffer = NULL;
volatile int ReadVar;

void trash_the_cache(void)
{
	// read a million random bytes and overwrite them with a new value
	for (int j=0; j<1024000; j++) {
		int address = rand() % CACHE_TRASH_BUFFER_SIZE;
		ReadVar = CacheTrashBuffer[address];
		CacheTrashBuffer[address] = ReadVar+1;
		//cout<<ReadVar;
	}
}		


int jan_main(int argc, char* argv[])
{
//	init_bez_sin();
	init_bez_acos();

	CacheTrashBuffer = new int[CACHE_TRASH_BUFFER_SIZE];
	for (int i=0; i<CACHE_TRASH_BUFFER_SIZE; i++) {
		CacheTrashBuffer[i] = rand();
	}

	for(int i=0; i<10; i++)
	{
//		float foo = i/float(47) * pi * 2;
		float foo = float(i)/64.f;
		float r0 = 0.0f;
		float r1 = 0.0f; 
		float r2 = 0.0f;
		unsigned int acos_cycles = 0;
		unsigned int bez_cycles = 0;
		unsigned int table_cycles = 0;
		unsigned int acos_sum = 0;
		unsigned int bez_sum = 0;
		unsigned int table_sum = 0;
		const int SAMPLE_COUNT = 20;

		{
			for (int j=0; j<SAMPLE_COUNT; j++) {
				foo = WWMath::Random_Float();	
				acos_cycles = Get_CPU_Clock();
				r0 = acos(foo);
				acos_sum += Get_CPU_Clock() - acos_cycles;	
				trash_the_cache();
			}
		}
		

		{
			for (int j=0; j<SAMPLE_COUNT; j++) {
				foo = WWMath::Random_Float();	
				bez_cycles = Get_CPU_Clock();
				r1 = bez_acos(foo);
				bez_sum += Get_CPU_Clock() - bez_cycles;	
				trash_the_cache();
			}
		}

		{
			for (int j=0; j<SAMPLE_COUNT; j++) {
				foo = WWMath::Random_Float();	
				table_cycles = Get_CPU_Clock();
				r2 = WWMath::Fast_Acos(foo);
				table_sum += Get_CPU_Clock() - table_cycles;	
				trash_the_cache();
			}
		}

		
//		cout << "x: " << setw(8) <<  foo << " sin(x): " << setw(8) << r0 << " bez_sin(x): " << setw(8) << r1 << " ratio: " << setw(8) << r0-r1 << endl;
//		cout << "x: " << setw(8) <<  foo << " acos(x): " << setw(8) << r0 << " bez_acos(x): " << setw(8) << r1 << " ratio: " << setw(8) << r0/r1 << endl;
		cout << "acos clocks: "<<acos_sum<<" bez clocks: "<<bez_sum<<" table clocks: "<<table_sum << endl;
	}

	delete CacheTrashBuffer;

	return 0;
}

