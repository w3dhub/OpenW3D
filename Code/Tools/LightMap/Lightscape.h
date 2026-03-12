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
 *                     $Modtime:: 7/03/01 3:07p       $*
 *                                                                                             *
 *                    $Revision:: 38                                                        $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef _LIGHTSCAPE_H
#define _LIGHTSCAPE_H

// Includes.
#include "Chunk.h"
#include "LightMapPacker.h"
#include "Triangle.h"
#include "arraylistof.h"
#include "light.h"
#include "lsbasebuilders.h"
#include "ltttypes.h"
#include "rmap.h"
#include "vector.h"
#include "vector2.h"
#include "vector3.h"
#include "vector3i.h"
#include "wwmath.h"
#include "w3d_file.h"


class TextureNameNode;

class SolveStatistics {

	public:

		enum SolveStatisticEnum {
			VERTEX_NOT_FOUND,
			VERTEX_NO_COLOR,
			FACE_NOT_FOUND,
			FACE_NO_LIGHTMAP,
			VERTEX_NOT_SMOOTH,
			FACE_AMBIGUOUS,
			FACE_DEGENERATE,
			FACE_UNDERSIZED,
			EDGE_MATCH,
			ALL_FILL_COLOR,
			NULL_TEXTURE,
			SOLVE_STATISTIC_COUNT
		};

		SolveStatistics ()
		{
			VertexCount = 0;
			FaceCount	= 0;
			for (unsigned s = 0; s < SOLVE_STATISTIC_COUNT; s++) Count [s] = 0;
		}

		SolveStatistics (unsigned vertexcount, unsigned facecount)
		{
			VertexCount	= vertexcount;
			FaceCount	= facecount;
			for (unsigned s = 0; s < SOLVE_STATISTIC_COUNT; s++) Count [s] = 0;
		}

		bool Valid_Vertex_Solve() const {
			return (((Count [VERTEX_NOT_FOUND] == 0) || (Count [VERTEX_NOT_FOUND] < VertexCount)) && (Count [VERTEX_NO_COLOR] == 0));
		}

		bool Valid_Lightmap_Solve() const {
			return (((Count [FACE_NOT_FOUND] == 0) || (Count [FACE_NOT_FOUND] < FaceCount)) && (Count [FACE_NO_LIGHTMAP] == 0));
		}

		bool operator == (unsigned c) {

			unsigned sum;

			sum = 0;
			for (unsigned s = 0; s < SOLVE_STATISTIC_COUNT; s++) sum += Count [s];
			return (sum == c);
		}

		unsigned	VertexCount;
		unsigned FaceCount;
		unsigned Count [SOLVE_STATISTIC_COUNT];
};


// Class wrapper for floating point color vectors.
class ColorVector : public Vector3
{
	public:

		ColorVector () : Vector3() {}
		ColorVector (float r, float g, float b) : Vector3 (r, g, b) {}

		void Set (float r, float g, float b)
		{
			Vector3::Set (r, g, b);
		}

		void Set (const LtTRGBColor &irradiance, const RadianceMap &radiancemap)
		{
			const double oopi = 1.0 / M_PI;

			X = irradiance.GetR() * oopi;
			Y = irradiance.GetG() * oopi;
			Z = irradiance.GetB() * oopi;
			radiancemap.ToColor (X, Y, Z);
		}
};


class LightscapeSolve : public LightmapPacker
{
	public:

		// Member functions.
		LightscapeSolve (const char *solvedirectoryname, const char *solvefilenamelist, CStatusBar* statusptr, const char *statusbarmessage, bool blendnoise = false);
		Finish();
	  ~LightscapeSolve ();

		// Database ammendment functions.
	   void Set_Is_Solution (bool issolution)		{IsSolution = issolution;}
		void Set_Brightness (float brightness)		{Brightness = brightness;}
		void Set_Contrast (float contrast)			{Contrast = contrast;}
		void Set_Is_Daylight (bool isdaylight)		{IsDaylight = isdaylight;}
		void Set_Is_Exterior (bool isexterior)		{IsExterior = isexterior;}
		void Set_Patch_Cluster_Count (int count)	{PatchClusterCount = count;}
		void Set_Is_M2T_Solve (bool ism2tsolve)	{IsM2TSolve = ism2tsolve;}
		void Add_Material_Texture_Name (const char *materialname, const char *texturename);
		void Add_Texture_Name (unsigned patchindex, const char *materialname);
		void Add_Light (LightClass *light);
		bool Add_Vertex (unsigned vertexindex, const Vector3 &p, const Vector3 &n, unsigned patchindex, const Vector2 &t);
		bool Add_Vertex (unsigned vertexindex, const Vector3 &p, const Vector3 &n, unsigned patchindex, const ColorVector &c);
		bool Add_Patch_Face (unsigned patchfaceindex, unsigned patchindex, unsigned vertexindexcount, unsigned *vertexindices);

		// Database query functions.
		float					 Get_Brightness()			  {return (Brightness);}
		float					 Get_Contrast()			  {return (Contrast);}
		bool					 Is_Daylight()				  {return (IsDaylight);}
		bool					 Is_Exterior()				  {return (IsExterior);}
		bool					 Is_M2T_Solve()			  {return (IsM2TSolve);}
		float					 Filter_Sharpness()		  {return (FilterSharpness);}
		ProceduralTexture *Procedural_Texture()	  {return (ProceduralTexture);}
		unsigned				 Light_Count()				  {return (Lights.Count());}
		LightClass			*Get_Light (unsigned l)	  {return (Lights [l]);}
		const char			*Light_Exclusion_String() {return (LightExclusionString);}

		void Find_Vertex (const Vector3 &point, const Vector3 *smoothingnormalptr, W3dRGBStruct &vertexcolor, SolveStatistics &solvestatistics);
		void Find_Triangle (const Vector3 *points, const Vector3 &normal, PackingTriangle &triangle, SolveStatistics &solvestatistics);
		void Submit_Triangle (PackingTriangle &triangle);

		const TextureNameNode *Texture_Name (unsigned patchindex);

		void Pack();

		// Update the progress bar.
		void Step_It()				{ProgressBar->StepIt();}

		// Static functions.
		static LightscapeSolve *Importer() {return (_ActiveImporter);}
		static const char		  *Asset_Directory() {return (LightmapPacker::Asset_Directory());}
		static const char		  *Asset_Directory (const char *filename) {return (LightmapPacker::Asset_Directory (filename));}
		static void					Copy_Assets (const char *pathname) {LightmapPacker::Copy_Assets (pathname);}

	private:

		// Result of finding a face.
		enum FaceResultEnum {
			FACE_NOT_FOUND,
			FACE_AMBIGUOUS,
			FACE_FOUND
		};

		// Area of the face.
		enum FaceSizeEnum {
			FACE_DEGENERATE,
			FACE_UNDERSIZED,
			FACE_NOT_UNDERSIZED
		};

		struct VerticesStruct {

			// Equality operator.
			bool operator == (const VerticesStruct &v) {
				return ((Point == v.Point) &&
						  (FaceNormal == v.FaceNormal) &&
						  (PatchIndex == v.PatchIndex) &&
						  (ValidColor == v.ValidColor) &&
						  (Color == v.Color) &&
						  (ValidUV == v.ValidUV) &&
						  (UV == v.UV));
			}

			// Inequality operator.
			bool operator != (const VerticesStruct &v) {
				return (!(*this == v));
			}

			Vector3		Point;
			Vector3		FaceNormal;
			unsigned		PatchIndex;
			bool			ValidColor;
			ColorVector	Color;
			bool			ValidUV;
			Vector2		UV;
   	};

		struct PatchFaceStruct {

			enum VerticesPerPatchFace {
				VERTICES_PER_PATCH_FACE = 4
			};

			// To indicate whether a patch face vertex index is valid.
			enum VertexIndexValidity {
				VERTEX_INDEX_INVALID = -1
			};

			// Equality operator.
			bool operator == (const PatchFaceStruct &p) {
				return ((PatchIndex == p.PatchIndex) &&
						  (VertexIndices [0] == p.VertexIndices [0]) &&
						  (VertexIndices [1] == p.VertexIndices [1]) &&
						  (VertexIndices [2] == p.VertexIndices [2]) &&
						  (VertexIndices [3] == p.VertexIndices [3]));
			}

			// Inequality operator.
			bool operator != (const PatchFaceStruct &p) {
				return (!(*this == p));
			}

			unsigned PatchIndex;
			unsigned	VertexIndices [VERTICES_PER_PATCH_FACE];
		};

		// Member functions.
		void Find_Adjacent_Triangles (const PackingTriangle &principaltriangle, DynamicVectorClass <Triangle> &adjtriangles);
		void Find_Triangle (const Vector3 *points, const Vector3 &facenormal, VerticesStruct *vertices, FaceResultEnum &faceresult);
		void Find_Triangle (const Vector3 *points, float tolerance, bool centroidtest, const Vector3 *facenormal, VerticesStruct *vertices, FaceResultEnum &faceresult);
		void Find_Vertices (const Vector3 &point, float tolerance, DynamicVectorClass <VerticesStruct*> &vertices);

		// Miscellaneous convenience functions.
		FaceSizeEnum  Face_Size (const Vector3 *points, unsigned count);
		unsigned		  Longest_Edge (const Vector3 *points, unsigned count, bool *usededges);

		float	Manhatten_Distance (const Vector3 &v)
		{
			return (WWMath::Fabs (v.X) + WWMath::Fabs (v.Y) + WWMath::Fabs (v.Z));
		}

		// Comparison functions.
		static int Compare_Material_Texture_Names (const void *materialname0, const void *materialname1);
		static int Compare_Vertices (const void *vertexindexptr0, const void *vertexindexptr1);
		static int Compare_Patch_Faces (const void *patchface0, const void *patchface1);

		// Member data.
		float												 SmoothingAngle;
		float												 SpatialTolerance;
		float												 FilterSharpness;
		bool												 IsSolution;						// Is the currently imported file a valid solve file?
		int												 PatchClusterCount;				// Total patch cluster count over all parsed solve files.
		float												 Brightness;
		float												 Contrast;
		bool												 IsDaylight;
		bool												 IsExterior;
		bool												 IsM2TSolve;
		ProceduralTexture								*ProceduralTexture;
		CProgressCtrl									*ProgressBar;
		DynamicVectorClass <char *>				 MaterialTextureNames;			// <material name, texture pathname> pairs for all materials in the solve.
		DynamicVectorClass <TextureNameNode*>	 TextureNames;						// <texture pathname> for all patches in the solve.
		DynamicVectorClass <VerticesStruct>		 Vertices;							// <point, uv coordinate, normal, material index> tuples for all vertices in the solve.
		DynamicVectorClass <unsigned>				 VertexIndices;					// Indices into Vertices.
		DynamicVectorClass <PatchFaceStruct>	 PatchFaces;						// <patch index, face> pairs for all faces in the solve.
		DynamicVectorClass <LightClass*>			 Lights;								// All active lights in the solve.
		const char										*LightExclusionString;			// Do not add lights that contain this string.

		// Static data.
		static LightscapeSolve *_ActiveImporter;	// Object currently importing (reading a solution file) (otherwise NULL).
};


class LsInformationFactory : public LtTBuilderFactory
{
	protected:
		virtual LtTInfoBuilderApi		 *OnGetInfoBuilder();
		virtual LtTParameterBuilderApi *OnGetParameterBuilder();
};


class LsPreparationFactory : public LtTBuilderFactory
{
	protected:
		virtual LtTInfoBuilderApi		 *OnGetInfoBuilder();
		virtual LtTParameterBuilderApi *OnGetParameterBuilder();
		virtual LtTMaterialBuilderApi  *OnGetMaterialBuilder();
		virtual LtTLampBuilderApi		 *OnGetLampBuilder();
		virtual LtTMeshBuilderApi		 *OnGetMeshBuilder();
};


class LsMainFactory : public LtTBuilderFactory
{
	public:

		LsMainFactory();

		unsigned Patch_Face_Count()		{return (PatchFaceIndex);}
		void		Patch_Face_Increment()	{PatchFaceIndex++;}
		unsigned Vertex_Count()				{return (VertexIndex);}
		void		Vertex_Increment()		{VertexIndex++;}
		unsigned Patch_Count()				{return (PatchIndex);}
		void		Patch_Increment()			{PatchIndex++; LightscapeSolve::Importer()->Step_It();}

	protected:

		virtual LtTInfoBuilderApi		 *OnGetInfoBuilder();
		virtual LtTParameterBuilderApi *OnGetParameterBuilder();
		virtual LtTMeshBuilderApi		 *OnGetMeshBuilder();

	private:

		unsigned PatchFaceIndex;
		unsigned VertexIndex;
		unsigned PatchIndex;
};


class LsInfoBuilder : public LtTBaseInfoBuilder
{
	protected:
	   LsInfoBuilder (LtTBuilderFactory *factory);
		LtTBool Finish();

	friend class LsInformationFactory;
	friend class LsPreparationFactory;
	friend class LsMainFactory;
};


class LsParameterBuilder : public LtTBaseParameterBuilder
{
	protected:
	   LsParameterBuilder (LtTBuilderFactory *factory);
		LtTBool Finish();

	friend class LsInformationFactory;
	friend class LsPreparationFactory;
	friend class LsMainFactory;
};


class LsMaterialBuilder : public LtTBaseMaterialBuilder
{
	protected:
		LsMaterialBuilder (LtTBuilderFactory *factory);
		LtTBool Finish();

   friend class LsPreparationFactory;
};


class LsLampBuilder : public LtTBaseLampBuilder
{
	protected:
		LsLampBuilder (LtTBuilderFactory *factory);
		LtTBool Finish();

	friend class LsPreparationFactory;
};


class LsMeshInquirer : public LtTBaseMeshBuilder
{
	protected:
		LsMeshInquirer (LtTBuilderFactory *factory);
		LtTBool Finish();

	friend class LsPreparationFactory;
};


class LsMeshBuilder : public LtTBaseMeshBuilder
{
	protected:
		 LsMeshBuilder (LtTBuilderFactory *factory);
		~LsMeshBuilder();

		LtTBool			Finish();
		void				SetFaces (const int facecount, const LtTFace *v);
		int				GetFaceCount() const;
		const LtTFace *GetFaces() const;

		LsMainFactory *FactoryPtr;

	private:

		struct FaceVertexStruct {

			// Equality operator.
			bool operator == (const FaceVertexStruct &fv) {
				return ((Point == fv.Point) && (Color == fv.Color) && (Weight == fv.Weight));
			}

			// Inequality operator.
			bool operator != (const FaceVertexStruct &fv) {
				return (!(*this == fv));
			}

			Vector3		Point;
			ColorVector Color;
			float			Weight;
		};

		LtTFace		  *Faces;
		int				FaceCount;
		int				AllocatedCount;

   friend class LsMainFactory;
	friend class VertexUser;
};


class VertexUser
{
	public:
		 VertexUser (const LsMeshBuilder &meshbuilder);
		~VertexUser () {delete [] UsedVertexIndices;}

		bool Is_Used (unsigned vertexindex)
		{
			ASSERT (vertexindex < Count);
			return (UsedVertexIndices [vertexindex] != UNUSED);
		}

		void Set_Index (unsigned vertexindex, unsigned usedvertexindex)
		{
			ASSERT (Is_Used (vertexindex));
			UsedVertexIndices [vertexindex] = usedvertexindex;
		}

		unsigned Get_Index (unsigned vertexindex)
		{
			ASSERT (vertexindex < Count);

			// This vertex must have previously been set.
			ASSERT (UsedVertexIndices [vertexindex] >= 0);

			return (UsedVertexIndices [vertexindex]);
		}

	private:

		enum VertexUsage {
			UNUSED = -2,
			USED	 = -1
		};

		VertexUser() {}

		unsigned	 Count;
		int		*UsedVertexIndices;
};


class LightscapeMeshSolve
{
	public:
		 LightscapeMeshSolve (LightscapeSolve &solve, ChunkClass &trianglechunk, ChunkClass &vertexchunk);
		~LightscapeMeshSolve();

		W3dRGBStruct				 Vertex_Color (unsigned vertexindex) const;
		unsigned						 Vertex_Color_Count() const	{return (VertexCount);}
		const char					*Lightmap_Pathname (unsigned lightmapindex) const;
		unsigned						 Lightmap_Index (unsigned faceindex) const;
		W3dTexCoordStruct			 Lightmap_UV (unsigned facevertexindex) const;
	   unsigned						 Lightmap_Count() const {return (LightmapCount);}
		const SolveStatistics	*Get_Statistics() const {return (&Statistics);}

	private:

		unsigned			  VertexCount;						// No. of vertices in mesh.
		unsigned			  FaceCount;						// No. of faces in mesh.
		unsigned			  FaceVertexCount;				// No. of face-vertices in mesh.
		unsigned			  LightmapCount;					// No. of lightmaps used by mesh.

		W3dRGBStruct	 *VertexColors;					// Array of vertex colors.
		Vector2 			 *FaceVertexUVs;					// Array to map a face-vertex index to a UV coordinate.
		unsigned			 *FaceRemapLightmapIndices;	// Array to map a face to a zero-based lightmap index.
		unsigned			 *LightmapIndices;				// Array to map a zero-based lightmap index to a 'real' lightmap index.

		SolveStatistics  Statistics;						// Statistics counters.
};


#endif // LIGHTSCAPE_H
