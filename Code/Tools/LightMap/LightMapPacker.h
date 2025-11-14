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
 *                     $Modtime:: 6/06/01 5:40p       $* 
 *                                                                                             * 
 *                    $Revision:: 35                                                        $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef _LIGHTMAPPACKER_H
#define _LIGHTMAPPACKER_H

// Includes.
#include "ProceduralTexture.h"
#include "StringBuilder.h"
#include "Triangle.h"
#include "vector.h"
#include "vector2.h"
#include "matrix3.h"
#include "listnode.h"
#include "Targa.h"
#include "w3d_file.h"
#include <srPixelConvert.hpp>
#include <srColorSurface.hpp>


// Defines.
#define UNPACKED_TEXEL_BYTE_COUNT 4	// No. of bytes for internal unpacked texel format (ARGB).


class ColorSurface : public srColorSurface
{
	public:
		ColorSurface (srPixelConvert::e_surfaceType pixelformat, short width, short height)
			: srColorSurface (pixelformat, width, height)
		{}
};


class TrueColorTarga : public Targa
{
	public:

		// Public functions.
		TrueColorTarga();
		TrueColorTarga (unsigned width, unsigned height, unsigned pixeldepth);
		TrueColorTarga (unsigned width, unsigned height, unsigned pixeldepth, const W3dRGBStruct &clearcolor);

		void		 Reformat (unsigned width, unsigned height, unsigned pixeldepth);
		void		 Reformat (unsigned width, unsigned height, unsigned pixeldepth, const W3dRGBStruct &clearcolor);

		unsigned	 Width()	const											{return (Header.Width);}                                         
		unsigned	 Height() const										{return (Header.Height);}                                        
		unsigned	 Pixel_Depth()	const									{return (Header.PixelDepth);}                                    
		void		 Scale (unsigned width, unsigned height)		{Scale (*this, width, height);}                                  
		void		 Scale (TrueColorTarga &destination)			{Scale (destination, destination.Width(), destination.Height());}
		void		 Scale (float filtererror)							{Scale (*this, filtererror);}
		void		 Transpose()											{Transpose (*this);}

		void Pad (unsigned padwidth, unsigned padheight, const W3dRGBStruct &padcolor, DynamicVectorClass <PackingTriangle*> &triangleptrs)
		{
			Pad (*this, padwidth, padheight, padcolor, triangleptrs);
		}

		char		*Load (const char *pathname);
		char		*Save (const char *pathname);
		void		 Blit (TrueColorTarga &destination, unsigned x, unsigned y);
		void		 Scale (TrueColorTarga &destination, unsigned width, unsigned height);
		int		 Compare (TrueColorTarga &comparison, float epsilon);
		int		 Compare (TrueColorTarga &comparison, unsigned x, unsigned y, float epsilon);
		bool		 Fill (const W3dRGBStruct &fillcolor);
		void		 Add (TrueColorTarga &targa);
		void		 Rasterize (TrueColorTarga &destination, const W3dRGBStruct &fillcolor, unsigned vertexcount, const W3dRGBStruct *vertexcolors, Vector2 *uvs);

		bool Get_Color (const Vector2 &t, W3dRGBStruct &color) const;
		bool Set_Color (unsigned x, unsigned y, const W3dRGBStruct &color);

	protected:
		
		enum TransposeEnum {
			UNTRANSPOSED,
			TRANSPOSED,
			TRANSPOSE_COUNT
		};

  		struct UnpackedTexelStruct {

			UnpackedTexelStruct () {}
			UnpackedTexelStruct (unsigned char r, unsigned char g, unsigned char b, unsigned char a)
			{
				Set (r, g, b, a);
			} 
			
			void Set (unsigned char r, unsigned char g, unsigned char b, unsigned char a)
			{
				Byte [0] = a;
				Byte [1] = r;
				Byte [2] = g;
				Byte [3] = b;
			}

			unsigned char Alpha() const	{return (Byte [0]);}
			unsigned char Red() const		{return (Byte [1]);}
			unsigned char Green() const	{return (Byte [2]);}
			unsigned char Blue()	const 	{return (Byte [3]);}

			// Equality operator.
			bool operator == (const UnpackedTexelStruct &t) {
				ASSERT (sizeof (unsigned int) == sizeof (Byte));
				return (*((unsigned int*) Byte) == *((unsigned int*) t.Byte));
			}

			// Inequality operator.
			bool operator != (const UnpackedTexelStruct &t) {
				return (!(*this == t));
			}

			// Addition operator.
			UnpackedTexelStruct operator += (UnpackedTexelStruct t)
			{
				Byte [0] = MIN (((unsigned) Byte [0]) + ((unsigned) t.Byte [0]), (unsigned) UCHAR_MAX);
				Byte [1] = MIN (((unsigned) Byte [1]) + ((unsigned) t.Byte [1]), (unsigned) UCHAR_MAX);
				Byte [2] = MIN (((unsigned) Byte [2]) + ((unsigned) t.Byte [2]), (unsigned) UCHAR_MAX);
				Byte [3] = MIN (((unsigned) Byte [3]) + ((unsigned) t.Byte [3]), (unsigned) UCHAR_MAX);
				return (*this);
			}

			// Public data.
			unsigned char Byte [UNPACKED_TEXEL_BYTE_COUNT];
		};

		struct PointStruct {

			// Equality operator.
			bool operator == (const PointStruct &p) {
				return ((X == p.X) && (Y == p.Y));
			}

			// Inequality operator.
			bool operator != (const PointStruct &p) {
				return (!(*this == p));
			}

			// Public data.
			int X;
			int Y;
		};

		void Clear (const UnpackedTexelStruct &cleartexel);

	private:		
	
		// Inlines.
		unsigned char		  *Get_Texel (int x, int y) const;
		UnpackedTexelStruct *Unpack_Texel (unsigned char *packedtexelptr, unsigned packedbytespertexel, UnpackedTexelStruct &unpackedtexel) const;
		void						Pack_Texel (const UnpackedTexelStruct &unpackedtexel, unsigned char *packedtexelptr, unsigned packedbytespertexel);

		// Static functions.
		static srPixelConvert::e_surfaceType Pixel_Format (unsigned pixeldepth);

		void  Scale (TrueColorTarga &destination, float filtererror);
		void  Transpose (TrueColorTarga &destination);
		bool  Fill_Four_Connected (unsigned x, unsigned y, const UnpackedTexelStruct &filltexel, DynamicVectorClass <PointStruct> &fourconnectedarray);
		void  Pad (TrueColorTarga &destination, unsigned verticalpadthickness, unsigned horizontalpadthickness, const W3dRGBStruct &padcolor, DynamicVectorClass <PackingTriangle*> &triangleptrs);
};


class Page : public TrueColorTarga
{
	public:

		// Public functions.
		Page (unsigned bitdepth, const W3dRGBStruct &clearcolor);
	  ~Page ();

		bool Pack (TrueColorTarga &targa, float epsilon, DynamicVectorClass <PackingTriangle*> &triangleptrs);


		float	Packing_Efficiency() const
		{
			return (((float) UsedTexelCount) / (((unsigned) Width()) * ((unsigned) Height())));
		}
		
		float	Replica_Efficiency() const
		{
			return (((float) ReplicaTexelCount) / (((unsigned) Width()) * ((unsigned) Height())));
		}
		
	private:
		
		class Region : public GenericNode {
			
			public:

				// Public functions.
				Region () {}
				Region (const Region &region)
				{
					Set (region.X0, region.Y0, region.X1, region.Y1);
				}

				unsigned Width () {return (X1 - X0 + 1);}
				unsigned Height() {return (Y1 - Y0 + 1);}

				void Set (unsigned x0, unsigned y0, unsigned x1, unsigned y1)
				{
					X0 = x0; Y0 = y0;	X1 = x1;	Y1 = y1;
				}

				bool Accomodates (unsigned width, unsigned height, Region &targetregion);
				bool Intersects  (const Region &targetregion, unsigned &subregioncount, Region **subregions);

				// Public data.
				unsigned X0, Y0;
				unsigned X1, Y1;
		};
		
		// Private functions.
		bool Replica_Region (TrueColorTarga &targa, float epsilon, Region &replicaregion);
		bool Lowest_Cost_Region (TrueColorTarga &targa, unsigned &lowestcost, Region &lowestcostregion);
		void Insert_Region (const Region &insertionregion);
		bool Contains (const Region &testregion);
		
		// Statistics functions.
		void Reset_Statistics();

		// Private data.
		GenericList VacantRegionList;
		GenericList UsedRegionList;

		// Statistics counters.
		unsigned AssetCount;
		unsigned	UsedTexelCount;
		unsigned	ReplicaTexelCount;
};


class TargaLoader {

	public:

		// Public functions.	
		TargaLoader (float samplerate, W3dRGBStruct fillcolor);
	  ~TargaLoader();
	
		TrueColorTarga *Load (const Triangle &triangle);
	
	private:
	
		struct TargaCacheStruct {
			
			TargaCacheStruct()
			{
				Ptr = NULL;
			}
			
			~TargaCacheStruct()
			{
				if (Ptr != NULL) delete Ptr;
			}

			// Equality operator.
			bool operator == (const TargaCacheStruct &t) {
				return ((ID == t.ID) && (Ptr == t.Ptr));
			}

			// Inequality operator.
			bool operator != (const TargaCacheStruct &t) {
				return (!(*this == t));
			}

			// Data.
			unsigned			 ID;
			TrueColorTarga *Ptr;
		};

		// Private functions.
		TargaLoader() {}
		
		unsigned Index (unsigned id)
		{
			return (id % ((unsigned) Cache.Length()));
		}

		// Private data.
		VectorClass			 <TargaCacheStruct> Cache;			// A simple array of targacache objects. 
		DynamicVectorClass <TrueColorTarga*>  CleanupList;	// List of targas that have been displaced from the cache but cannot
																			// yet be deleted because they may still be referenced.	
		float											  SampleRate;	// No. of texels per unit length.
		W3dRGBStruct								  FillColor;	// Color to replace during fill operations.	
};


class TrianglePacker {

	public:

		TrianglePacker (const PackingTriangle *principaltriangleptr, const DynamicVectorClass <Triangle> &adjtriangles, unsigned groupid, unsigned edgeblendthickness, float samplerate, W3dRGBStruct fillcolor);

		DynamicVectorClass <PackingTriangle*> &Principal_Triangles() {return (PrincipalTriangles);}
		
		// Query functions. 
		unsigned	Width() const	{return (UpperBound.U - LowerBound.U);}
		unsigned	Height() const {return (UpperBound.V - LowerBound.V);}

		// Operations.
		TrianglePacker *Merge (const TrianglePacker &trianglepacker);
		void Rasterize (TargaLoader &targaloader, ProceduralTexture *proceduraltexture, TrueColorTarga &rasterizedtarga);

		// Statistics.
		unsigned Adjacent_Face_Count() const 
		{
			return (AdjacentTriangles [ADJACENT_PROJECTION_COMMON].Count() +
					  AdjacentTriangles [ADJACENT_PROJECTION_VALID].Count() +
					  AdjacentTriangles [ADJACENT_PROJECTION_NONE].Count());
		}

		unsigned Blended_Face_Count() const
		{
			return (AdjacentTriangles [ADJACENT_PROJECTION_COMMON].Count() +
					  AdjacentTriangles [ADJACENT_PROJECTION_VALID].Count());
		}

		float	Edge_Blend_Area()	const;

	private:

		enum ProjectionEnum {
			PROJECTION_Y_POSITIVE,
			PROJECTION_Y_NEGATIVE,
			PROJECTION_Z_POSITIVE,
			PROJECTION_Z_NEGATIVE,
			PROJECTION_X_POSITIVE,
			PROJECTION_X_NEGATIVE,
			PROJECTION_COUNT
		};

		enum AdjacentProjectionEnum {
			ADJACENT_PROJECTION_COMMON,	// Adjacent triangles that use the same projection as this object.
			ADJACENT_PROJECTION_VALID,		// Adjacent triangles that map to a different projection but have non-zero area if object's projection is used.
			ADJACENT_PROJECTION_NONE,		// Adjacent triangles that cannot be projected using object's projection.
			ADJACENT_PROJECTION_COUNT
		};

		class ProjectionTriangle {
		  
			public:

				// Public functions.
				ProjectionTriangle() {}
				ProjectionTriangle (const Vector3 *points, const Vector2 *sourceuvs, const TrueColorTarga *sourcetargaptr, const Vector2 *projectionuvs);
		
				// Public data.
				Vector3			 Points [Triangle::VERTICES_COUNT];
				Vector2			 SourceUVs [Triangle::VERTICES_COUNT];
				TrueColorTarga *SourceTargaPtr;
				Vector2			 ProjectionUVs [Triangle::VERTICES_COUNT];
		};

		class SampleSurface {

			public:

				// Public functions.
				SampleSurface (unsigned width, unsigned height, ProceduralTexture *blendtexture = NULL);
				
				bool Sample (const Vector2 &samplepoint, const ProjectionTriangle &projectiontriangle, unsigned priority);
				bool Sample (float alpha, float beta, const ProjectionTriangle &projectiontriangle, unsigned priority);
				
				bool Get_Color (unsigned x, unsigned y, W3dRGBStruct &color)
				{
					SampleStruct *sampleptr;

					ASSERT ((x < Width) && (y < Height));
					sampleptr = Surface + (y * Width + x);
					if (sampleptr->Count > 0) {
			
						float		oocount;
						unsigned	r, g, b;

						oocount = 1.0f / sampleptr->Count;
						r = sampleptr->Red * oocount;
						g = sampleptr->Green * oocount;
						b = sampleptr->Blue * oocount;
						color.Set ((uint8) MIN (r, UCHAR_MAX), (uint8) MIN (g, UCHAR_MAX), (uint8) MIN (b, UCHAR_MAX));
						return (true);
					} else {
						return (false);
					}
				}

				unsigned Priority (unsigned x, unsigned y)
				{
					SampleStruct *sampleptr;

					ASSERT ((x < Width) && (y < Height));
					sampleptr = Surface + (y * Width + x);
					return (sampleptr->Priority);
				}

				float Sampling_Ratio() {return (((float) SampledTexelCount) / ((float) (Width * Height)));}

			private:

				struct SampleStruct {
					unsigned	Red;
					unsigned	Green;
					unsigned	Blue;
					unsigned Count;
					unsigned	Priority;
				};

				SampleSurface() {}

				unsigned				 Width;
				unsigned				 Height;
				unsigned				 SampledTexelCount;	
				SampleStruct		*Surface;
				ProceduralTexture *BlendTexture;	
		};

		// Private functions.
							TrianglePacker() {}
							TrianglePacker (const TrianglePacker &trianglepacker);	// Copy constructor.
		void				Set_Bounds();
		bool				Can_Project (const Vector3 &normal);
		Vector2			Project (const Vector3 &point) const;
//		void				Get_Projection_Triangle (const Triangle &triangle, ProjectionTriangleStruct &projectiontriangle);
		bool				Sample (const Vector2 &samplepoint, const ProjectionTriangle &projectiontriangle, W3dRGBStruct &color);

		static ProjectionEnum Get_Projection (const Vector3 &normal);

		// Private data.
		ProjectionEnum								  Projection;												// Texture projection to be used for rasterization.
		unsigned										  GroupID;	
		unsigned										  EdgeBlendThickness;	
		float											  SampleRate;												// No. of texels per unit length.
		W3dRGBStruct								  FillColor;													// Color to replace during fill operations.	
		DynamicVectorClass <PackingTriangle*> PrincipalTriangles;										// Set of triangles that will be rasterized in their entirety (ie. not edge blended).
		DynamicVectorClass <Triangle>			  AdjacentTriangles [ADJACENT_PROJECTION_COUNT];	// Set of triangles that will be edge-blended.
		Vector2										  LowerBound;												// Lower bound of all principal triangle points projected onto the texel plane.
		Vector2										  UpperBound;												// Upper bound of all principal triangle points projected onto the texel plane.
};


class LightmapPacker {

	public:
	
		enum ThicknessEnum {
			EDGE_BLEND_THICKNESS = 2	// No. of texels to pad around every packed map.
												// NOTE: The edge blend thickness is designed to prevent
												// bleeding between adjacent maps under mip-mapping. The
												// no, of allowable mip-maps is dependant upon this
												// thickness value.
		};

		enum StatisticsEnum {

			STATISTICS_PAGE_FORMAT,
			STATISTICS_LIGHTMAPS_PROCESSED,
			STATISTICS_ADJACENT_FACE_BLEND_PERCENTAGE,
			STATISTICS_EDGE_BLEND_EFFICIENCY,
			STATISTICS_SCALING_EFFICIENCY,
			STATISTICS_PAGES_CREATED,
			STATISTICS_PACKING_EFFICIENCY,
			STATISTICS_CULLING_EFFICIENCY,
			STATISTICS_TEXTURE_SWAP_EFFICIENCY,
			STATISTICS_OVERSIZE_LIGHTMAPS,
			STATISTICS_COUNT,

			STATISTICS_STRING_SIZE = 32	// Size of statistic string in bytes.	
		};

		 LightmapPacker();
		 Finish();
	   ~LightmapPacker();
		
		void					 Submit (PackingTriangle *principaltriangleptr, const DynamicVectorClass <Triangle> &adjtriangles);
		TrianglePacker		*Merge (TrianglePacker *trianglepackerptr);
		void					 Pack (ProceduralTexture *proceduraltexture = NULL);
		void					 Pack (TrueColorTarga &targa, DynamicVectorClass <PackingTriangle*> &triangleptrs);

		static const char *Lightmap_Pathname (unsigned pageindex);
		static const char *Asset_Directory();
		static const char *Asset_Directory (const char *filename);
		static void			 Delete_Assets();
		static void			 Copy_Assets (const char *pathname);
		static const char *Get_Statistic (unsigned index) {ASSERT (index < STATISTICS_COUNT); return (_Statistics [index]);}

	private:

		// Statistics functions.
		void Reset_Statistics();
		void Update_Statistics (const Page &page);
		void Update_Statistics (const TrianglePacker &trianglepacker);
		void Collate_Statistics();
		
		W3dRGBStruct		  FillColor;				// Color used to pad unused texels in lightmaps. 
		float					  ScaleFactor;				// Global lightmap scaling	factor (0.0...1.0).
		float					  FilterError;				// Maximum allowed variance between a color component of original
																// lightmap	and that of packed lightmap, expressed as a fraction
																// of color component's total range ie. a setting of 0.0 will
																// allow no variance (packed lightmap must match original) and
																// a setting of 1.0 allows complete variance.
		float					  SampleRate;				// Sample rate used by rasterizers (such as Edge_Blend()) in texels per unit length.
		unsigned				  PageBitDepth;			// Bit depth of page	texels.


		int					  CurrentPageIndex;		// Current page (-1 indicates no page exists).
		
		DynamicVectorClass <TrianglePacker*>  TrianglePackerPtrs;
		DynamicVectorClass <Page*>				  PagePtrs;
		TextureNameNode							 *PlaceholderTextureNameNodePtr;

		// Statistics counters.
		unsigned  FaceCount;
		unsigned	 LightmapCount;
		unsigned	 AdjacentFaceCount;
		unsigned	 BlendedFaceCount;	
		double	 EdgeBlendAreaSum;
		int64_t	 UnscaledTexelCount;
		int64_t	 ScaledTexelCount;
		double	 PackingEfficiencySum;
		double	 ReplicaEfficiencySum;
		unsigned	 TextureSwapCount;
		unsigned  OversizeCount;
		unsigned  AllFillColorCount;

		static unsigned _BasePageIndex;
		static char		 _Statistics [STATISTICS_COUNT][STATISTICS_STRING_SIZE];
};


/***********************************************************************************************
 * TrueColorTarga::Get_Color --																					  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/27/00    IML : Created.                                                                * 
 *=============================================================================================*/
inline bool TrueColorTarga::Get_Color (const Vector2 &t, W3dRGBStruct &color)	const
{
	int						x, y;
	unsigned char		  *texelptr;
	UnpackedTexelStruct  unpackedtexel;
	
	x = t.U * Width();
	y = t.V * Height();
	texelptr = Get_Texel (x, y);
	if (texelptr == NULL) return (false);
	Unpack_Texel (texelptr, TGA_BytesPerPixel (Pixel_Depth()), unpackedtexel);
	color.Set (unpackedtexel.Red(), unpackedtexel.Green(), unpackedtexel.Blue());
	return (true);
}


/***********************************************************************************************
 * TrueColorTarga::Set_Color --																					  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/27/00    IML : Created.                                                                * 
 *=============================================================================================*/
inline bool TrueColorTarga::Set_Color (unsigned x, unsigned y, const W3dRGBStruct &color)
{
	UnpackedTexelStruct unpackedtexel (color.R, color.G, color.B, 0);

	unsigned char *texelptr;

	texelptr = Get_Texel (x, y);
	if (texelptr != NULL) {
		Pack_Texel (unpackedtexel, texelptr, TGA_BytesPerPixel (Pixel_Depth()));
		return (true);
	} else {
		return (false);
	}
}


/***********************************************************************************************
 * TrueColorTarga::Get_Texel --																					  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/27/99    IML : Created.                                                                 * 
 *=============================================================================================*/
inline unsigned char *TrueColorTarga::Get_Texel (int x, int y) const
{
	// Check for (x, y) out of range.
	if ((x < 0) || (x >= ((int) Width ()))) return (NULL);
	if ((y < 0) || (y >= ((int) Height()))) return (NULL);

	return ((unsigned char*) GetImage()) + (((Width() * y) + x) * TGA_BytesPerPixel (Pixel_Depth()));
}


/***********************************************************************************************
 * TrueColorTarga::Unpack_Texel --																				  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/27/99    IML : Created.                                                                 * 
 *=============================================================================================*/
inline TrueColorTarga::UnpackedTexelStruct *TrueColorTarga::Unpack_Texel (unsigned char *packedtexelptr, unsigned packedbytespertexel, UnpackedTexelStruct &unpackedtexel) const
{
	switch (packedbytespertexel) {

		case 4:
			
			// Packed format same as unpacked format.
			ASSERT (sizeof (UnpackedTexelStruct) == 4);
			unpackedtexel = *((UnpackedTexelStruct*) packedtexelptr);
			break;

		case 3:
			
			// Unpack to ordering ARGB. Set Alpha to zero.
			unpackedtexel.Byte [0] = 0;							// Alpha
			unpackedtexel.Byte [1] = *(packedtexelptr + 0); // Red
			unpackedtexel.Byte [2] = *(packedtexelptr + 1); // Green
			unpackedtexel.Byte [3] = *(packedtexelptr + 2); // Blue
			break;

		case 2:
			{			
				static unsigned char _alpha [2] = {0x00, 0xff};

				unsigned short packedtexel = *((unsigned short*) packedtexelptr);

				// Unpack to ordering ARGB. Bit replicate	Alpha.
				unpackedtexel.Byte [0] = _alpha [packedtexel >> 15];				// Alpha
				unpackedtexel.Byte [1] = ((packedtexel & 0x7c00) >> 7) | 0x3;	// Red
				unpackedtexel.Byte [2] = ((packedtexel & 0x03e0) >> 2) | 0x3;	// Green
				unpackedtexel.Byte [3] = ((packedtexel & 0x001f) << 3) | 0x3;	// Blue
			}
			break;

		default:
			
			// Unrecognized byte count.
			ASSERT (0);
			break;
	}
	return (&unpackedtexel);
}				


/***********************************************************************************************
 * TrueColorTarga::Pack_Texel --																					  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/27/99    IML : Created.                                                                 * 
 *=============================================================================================*/
inline void TrueColorTarga::Pack_Texel (const UnpackedTexelStruct &unpackedtexel, unsigned char *packedtexelptr, unsigned packedbytespertexel)
{
	switch (packedbytespertexel) {

		case 4:
			*(packedtexelptr + 0) = unpackedtexel.Byte [0];
			*(packedtexelptr + 1) = unpackedtexel.Byte [1];
			*(packedtexelptr + 2) = unpackedtexel.Byte [2];
			*(packedtexelptr + 3) = unpackedtexel.Byte [3];
			break;

		case 3:
			
			// Pack to ordering RGB. Lose Alpha.
			*(packedtexelptr + 0) = unpackedtexel.Byte [1];
			*(packedtexelptr + 1) = unpackedtexel.Byte [2];
			*(packedtexelptr + 2) = unpackedtexel.Byte [3];
			break;

		case 2:
			{
				unsigned a, r, g, b;

				//	Pack to ordering ARGB. Round Alpha, Red, Green, Blue.
				a =  unpackedtexel.Byte [0] >> 7;
				r =  MIN (0x1f, (unpackedtexel.Byte [1] >> 3) + ((unpackedtexel.Byte [1] & 0x4) >> 2));
				g =  MIN (0x1f, (unpackedtexel.Byte [2] >> 3) + ((unpackedtexel.Byte [2] & 0x4) >> 2));
				b =  MIN (0x1f, (unpackedtexel.Byte [3] >> 3) + ((unpackedtexel.Byte [3] & 0x4) >> 2));
				*((unsigned short*) packedtexelptr) = ((a << 15) | (r << 10) | (g << 5) | b);
			}
			break;

		default:
			
			// Unrecognized byte count.
			ASSERT (0);
			break;
	}
}				


#endif // LIGHTMAPPACKER_H