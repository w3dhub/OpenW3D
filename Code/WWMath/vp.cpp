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
 *                 Project Name : wwmath                                                       *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/WWMath/vp.cpp                                $*
 *                                                                                             *
 *                        Author:: Hector Yee                                                  *
 *                                                                                             *
 *                     $Modtime:: 6/27/01 4:16p                                               $*
 *                                                                                             *
 *                    $Revision:: 11                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*/

#include "vp.h"
#include "vector2.h"
#include "vector3.h"
#include "vector4.h"
#include "matrix3d.h"
#include "matrix4.h"
#include "wwdebug.h"
#include "cpudetect.h"
#include <memory.h>

void VectorProcessorClass::Prefetch(void* /* address */)
{
	/* FIXME: implement using intrinsics */
}

void VectorProcessorClass::Transform (Vector3* dst,const Vector3 *src, const Matrix3D& mtx, const int count)
{
	/* FIXME: implement using intrinsics */

	for (int i=0; i<count; i++)
	{
		dst[i]=mtx*src[i];
	}
}

void VectorProcessorClass::Transform(Vector4* dst,const Vector3 *src, const Matrix4& matrix, const int count)
{
	if (count<=0) return;

	int i;

	for (i=0; i<count; i++)
	{
		dst[i]=matrix*src[i];
	}
}

void VectorProcessorClass::Copy(Vector2 *dst, const Vector2 *src, int count)
{
	if (count<=0) return;
	memcpy(dst,src,sizeof(Vector2)*count);
}

void VectorProcessorClass::Copy(unsigned *dst, const unsigned *src, int count)
{
	if (count<=0) return;
	memcpy(dst,src,sizeof(unsigned)*count);
}

void VectorProcessorClass::Copy(Vector3 *dst, const Vector3 *src, int count)
{
	if (count<=0) return;
	memcpy(dst,src,sizeof(Vector3)*count);
}

void VectorProcessorClass::Copy(Vector4 *dst, const Vector4 *src, int count)
{
	if (count<=0) return;
	memcpy(dst,src,sizeof(Vector4)*count);
}

void VectorProcessorClass::Copy(Vector4 *dst,const Vector3 *src, const float * srca, const int count)
{
	if (count<=0) return;
	int i;

	for (i=0; i<count; i++)
	{
		dst[i].X=src[i].X;
		dst[i].Y=src[i].Y;
		dst[i].Z=src[i].Z;
		dst[i].W=srca[i];
	}
}

void VectorProcessorClass::Copy(Vector4 *dst,const Vector3 *src, const float srca, const int count)
{
	if (count<=0) return;
	int i;

	for (i=0; i<count; i++)
	{
		dst[i].X=src[i].X;
		dst[i].Y=src[i].Y;
		dst[i].Z=src[i].Z;
		dst[i].W=srca;
	}
}

void VectorProcessorClass::Copy(Vector4 *dst,const Vector3 &src, const float * srca, const int count)
{
	if (count<=0) return;
	int i;

	for (i=0; i<count; i++)
	{
		dst[i].X=src.X;
		dst[i].Y=src.Y;
		dst[i].Z=src.Z;
		dst[i].W=srca[i];
	}
}

void VectorProcessorClass::CopyIndexed (unsigned *dst,const unsigned *src, const unsigned int *index, int count)
{
	if (count<=0) return;
	int i;

	for (i=0; i<count; i++)
	{
		dst[i]=src[index[i]];
	}
}

void VectorProcessorClass::CopyIndexed (Vector2 *dst,const Vector2 *src, const unsigned int *index, int count)
{
	if (count<=0) return;
	int i;

	for (i=0; i<count; i++)
	{
		dst[i]=src[index[i]];
	}
}

void VectorProcessorClass::CopyIndexed (Vector3 *dst,const Vector3 *src, const unsigned int *index, int count)
{
	if (count<=0) return;
	int i;

	for (i=0; i<count; i++)
	{
		dst[i]=src[index[i]];
	}
}

void VectorProcessorClass::CopyIndexed (Vector4 *dst,const Vector4 *src, const unsigned int *index, int count)
{
	if (count<=0) return;
	int i;

	for (i=0; i<count; i++)
	{
		dst[i]=src[index[i]];
	}
}

void VectorProcessorClass::CopyIndexed(unsigned char* dst, const unsigned char* src, const unsigned int *index, int count)
{
	if (count<=0) return;
	int i;

	for (i=0; i<count; i++)
	{
		dst[i]=src[index[i]];
	}
}

void VectorProcessorClass::CopyIndexed(float* dst, float* src, const unsigned int *index, int count)
{
	if (count<=0) return;
	int i;

	for (i=0; i<count; i++)
	{
		dst[i]=src[index[i]];
	}
}

void VectorProcessorClass::Clamp(Vector4 *dst,const Vector4 *src, const float min, const float max, const int count)
{
	if (count<=0) return;
	int i;

	for (i=0; i<count; i++)
	{
		dst[i].X=(src[i].X<min)?min:src[i].X;
		dst[i].X=(src[i].X>max)?max:src[i].X;

		dst[i].Y=(src[i].Y<min)?min:src[i].Y;
		dst[i].Y=(src[i].Y>max)?max:src[i].Y;

		dst[i].Z=(src[i].Z<min)?min:src[i].Z;
		dst[i].Z=(src[i].Z>max)?max:src[i].Z;

		dst[i].W=(src[i].W<min)?min:src[i].W;
		dst[i].W=(src[i].W>max)?max:src[i].W;
	}
}

void VectorProcessorClass::Clear(Vector3*dst, const int count)
{
	if (count<=0) return;
	memset(dst,0,sizeof(Vector3)*count);
}


void VectorProcessorClass::Normalize(Vector3 *dst, const int count)
{
	if (count<=0) return;
	int i;

	for (i=0; i<count; i++)
		dst[i].Normalize();
}

void VectorProcessorClass::MinMax(Vector3 *src, Vector3 &min, Vector3 &max, const int count)
{
	if (count<=0) return;
	min=*src;
	max=*src;

	int i;

	for (i=1; i<count; i++)
	{
		min.X=MIN(min.X,src[i].X);
		min.Y=MIN(min.Y,src[i].Y);
		min.Z=MIN(min.Z,src[i].Z);

		max.X=MAX(max.X,src[i].X);
		max.Y=MAX(max.Y,src[i].Y);
		max.Z=MAX(max.Z,src[i].Z);
	}
}

void VectorProcessorClass::MulAdd(float * dest,float multiplier,float add,int count)
{
	for (int i=0; i<count; i++) {
		dest[i] = dest[i] * multiplier + add;
	}
}

void VectorProcessorClass::DotProduct(float *dst, const Vector3 &a, const Vector3 *b,const int count)
{
	for (int i=0; i<count; i++)
		dst[i]=Vector3::Dot_Product(a,b[i]);
}

void VectorProcessorClass::ClampMin(float *dst, float *src, const float min, const int count)
{
	for (int i=0; i<count; i++)
		dst[i]=(src[i]>min?src[i]:min);
}

void VectorProcessorClass::Power(float *dst, float *src, const float pow, const int count)
{
	for (int i=0; i<count; i++)
		dst[i]=powf(src[i],pow);
}