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
 *                     $Modtime:: 8/24/01 9:24p       $* 
 *                                                                                             * 
 *                    $Revision:: 60                                                        $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

// Includes.
#include "StdAfx.h"
#include "LightMap.h"
#include "Lightscape.h"
#include "LightMapPacker.h"
#include "OptionsDialog.h"
#include "PerlinNoise.h"
#include "StringBuilder.h"
#include "TextureNameNode.h"
#include "matrix3.h"
#include "matrix3d.h"
#include "lightexclude.h"
#include "tri.h"
#include "vector3.h"
#include "wwmath.h"
#include <float.h>
#include	<math.h>
#include <typeinfo.h>


// Static data.
LightscapeSolve *LightscapeSolve::_ActiveImporter	= NULL;


/***********************************************************************************************
 * LightscapeSolve::LightscapeSolve --																			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/27/99    IML : Created.                                                                 * 
 *=============================================================================================*/
LightscapeSolve::LightscapeSolve (const char *solvedirectoryname, const char *solvefilenamelist, CStatusBar* statusptr, const char *statusbarmessage, bool blendnoise)
	: LightmapPacker()
{
	const char *notsolutiontext =
"One or more solve files are not solution (.ls) files.";

	const int materialtexturenamegrowthstep = 256;
	const int patchfacegrowthstep				 = 1024;
	const int vertexgrowthstep					 = 2048;	
	const int lightgrowthstep					 = 32;

	static char _messagebuffer	[1024];

	CRect					 srect, trect;
	BOOL					 success;
	StringBuilder		 errormessage (_messagebuffer, sizeof (_messagebuffer));
	const char			*solvefilename;
	LtTBuilderFactory *factory = NULL;
	OptionsDialog		 options;

	// Initialize.
	SmoothingAngle			= DEG_TO_RAD (options.Get_Smoothing_Angle());					//	Convert from degrees to radians.
	SpatialTolerance		= options.Get_Spatial_Tolerance();								// No mapping required.
	FilterSharpness		= pow (options.Get_Filter_Sharpness() / 50.0l, 5.0l);		// Map from linear to exponential scale.
	
	// If noise has been requested create a Perlin Noise generator.
	if (blendnoise) {
		ProceduralTexture = new PerlinNoise;
		ASSERT (ProceduralTexture != NULL);
	} else {
		ProceduralTexture = NULL;
	}

	if (options.Get_Light_Export_Selective()) {
		LightExclusionString = options.Get_Light_Exclusion_String();
	} else {
		LightExclusionString = NULL;
	}

	statusptr->GetItemRect (0, &srect);
	statusptr->SetPaneText (0, statusbarmessage);
	statusptr->GetDC()->DrawText (statusbarmessage, -1, &trect, DT_CALCRECT);

	CRect	prect (srect.TopLeft().x + trect.Width(), srect.TopLeft().y, srect.BottomRight().x, srect.BottomRight().y);
	
	ProgressBar = new CProgressCtrl;
	ASSERT (ProgressBar != NULL);
	success = ProgressBar->Create (WS_CHILD | WS_VISIBLE | PBS_SMOOTH, prect, statusptr, 0);
	ASSERT (success);
	
	// Specify how many elements should be added to dynamic arrays when they are resized.
	MaterialTextureNames.Set_Growth_Step (materialtexturenamegrowthstep);
	PatchFaces.Set_Growth_Step (patchfacegrowthstep);
	Vertices.Set_Growth_Step (vertexgrowthstep);
	VertexIndices.Set_Growth_Step (vertexgrowthstep);
	Lights.Set_Growth_Step (lightgrowthstep);

	_ActiveImporter = this;

	try {

		char solvepathname [_MAX_PATH];
		int  totalpatchclustercount = 0;

		// Parse the files, check that they are solve files and sum the patch cluster count for the progress bar.
		solvefilename = solvefilenamelist;
		while (strlen (solvefilename) > 0) {
			strcpy (solvepathname, solvedirectoryname);
			strcat (solvepathname, solvefilename);
			factory = new LsInformationFactory;
			ASSERT (factory != NULL);
			::LtSolutionImport (solvepathname, *factory);
			if (!IsSolution) throw (notsolutiontext);
			totalpatchclustercount += PatchClusterCount;
			errormessage.Copy (factory->GetErrorMsg());
			delete factory;
			factory = NULL;
			if (strlen (errormessage.String()) > 0) throw (errormessage.String());
			
			// Advance to next solve file.
			solvefilename += strlen (solvefilename) + 1;
		}

		// Set-up the progress bar so that it can be updated for every patch cluster.
		ProgressBar->SetRange32 (0, totalpatchclustercount);
		ProgressBar->SetStep (1);

		// For each solve filename in the list...
		solvefilename = solvefilenamelist;
		while (strlen (solvefilename) > 0) {

			strcpy (solvepathname, solvedirectoryname);
			strcat (solvepathname, solvefilename);

			// Parse the Lightscape solve file and process materials.
			// NOTE: Material processing must be done first because mesh processing is dependant on it.
			//			In order to guarantee this, the solution file is parsed twice, so that there is no
			//			implicit reliance on the material data preceeding the mesh data in the file.
			factory = new LsPreparationFactory;
			ASSERT (factory != NULL);
			
			::LtSolutionImport (solvepathname, *factory);
			errormessage.Copy (factory->GetErrorMsg());
			delete factory;
			factory = NULL;
			if (strlen (errormessage.String()) > 0) throw (errormessage.String());
		
			// Sort the material-texture names by a key defined by the comparison function Compare_Material_Names().
			// NOTE: qsort MUST (and does) allow for duplicate keys.
			if (MaterialTextureNames.Count() > 0) {
				qsort (&MaterialTextureNames [0], MaterialTextureNames.Count(), sizeof (char*), Compare_Material_Texture_Names);
			}

			// Parse the Lightscape solve file and add all vertices and faces that have been
			// processed by Mesh to Texture.
			factory = new LsMainFactory;
			ASSERT (factory != NULL);
			
			::LtSolutionImport (solvepathname, *factory);
			errormessage.Copy (factory->GetErrorMsg());
			delete factory;
			factory = NULL;
			if (strlen (errormessage.String()) > 0) throw (errormessage.String());

			// Clean-up the the material-texture names. They are no longer required.
			for (int i = 0; i < MaterialTextureNames.Count(); i++) {
				delete [] MaterialTextureNames [i];
			}
			MaterialTextureNames.Clear();

			// Advance to next solve file.
			solvefilename += strlen (solvefilename) + 1;
		}

		if (Vertices.Count() > 0) {

			// Create a vertex index table and initialize the indices to identity.
			for (int v = 0; v < Vertices.Count(); v++) {
				VertexIndices.Insert (v, v);
			}

			// Sort the vertex pointers by a key defined by the comparison function Compare_Vertices().
			// NOTE: qsort MUST (and does) allow for duplicate keys.
			qsort (&VertexIndices [0], VertexIndices.Count(), sizeof (unsigned), Compare_Vertices);
		}

		// Sort the patch faces by a key defined by the comparison function Compare_Patch_Faces().
		// NOTE: qsort MUST (and does) allow for duplicate keys.
		if (PatchFaces.Count() > 0) {
			qsort (&PatchFaces [0], PatchFaces.Count(), sizeof (PatchFaceStruct), Compare_Patch_Faces);
		}

 	} catch (const char *errormessage) {
			
		// Clean-up.
		if (factory != NULL) delete factory;
		if (ProgressBar != NULL) {
			delete ProgressBar;
			ProgressBar = NULL;
		}

		// Re-throw the message to the caller.
		throw (errormessage);
	}
	
	_ActiveImporter = NULL;
	delete ProgressBar;
	ProgressBar = NULL;
}


/***********************************************************************************************
 * LightscapeSolve::Finish --																						  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/27/99    IML : Created.                                                                 * 
 *=============================================================================================*/
LightscapeSolve::Finish()
{
	LightmapPacker::Finish();
}


/***********************************************************************************************
 * LightscapeSolve::~LightscapeSolve --																		  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS: Do not throw any exceptions in this (and any other) destructor because this		  *
 *				 destructor	can itself be called during the exception as part of its 'clean-up'.	  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/27/99    IML : Created.                                                                 * 
 *=============================================================================================*/
LightscapeSolve::~LightscapeSolve()
{
	int i, l;

	// Clean-up anything that didn't get cleaned up before because an exception was thrown.
	for (i = 0; i < MaterialTextureNames.Count(); i++) {
		delete [] MaterialTextureNames [i];
	}
	_ActiveImporter = NULL;

	// Clean-up the texture names.
	for (i = 0; i < TextureNames.Length(); i++) {
		
		TextureNameNode *texturenamenode, *removalnode;

		texturenamenode = TextureNames [i];
		while (texturenamenode != NULL) {
			removalnode = texturenamenode;
			texturenamenode = texturenamenode->Next;
			delete removalnode;
		}
	}

	// Release references to lights.
	for (l = 0; l < Lights.Count(); l++) {
		Lights [l]->Release_Ref();
	}

	if (ProceduralTexture != NULL) delete ProceduralTexture;
}	


/***********************************************************************************************
 * LightscapeSolve::Add_Material_Texture_Name --															  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/27/99    IML : Created.                                                                 * 
 *=============================================================================================*/
void LightscapeSolve::Add_Material_Texture_Name (const char *materialname, const char *texturepathname)
{
	const char *invalidtexturepathname = "";

	unsigned  materialnamelength, texturepathnamelength;
	char		*name;

	// The material name must exist.
	ASSERT (materialname != NULL);

	// If the material has no associated texture use the invalid texture pathname.
	if (texturepathname == NULL) texturepathname = invalidtexturepathname;

	// Calculate length of strings including terminator.
	materialnamelength	 = strlen (materialname) + 1;
	texturepathnamelength = strlen (texturepathname) + 1;
		
	name = new char [materialnamelength + texturepathnamelength];
	ASSERT (name != NULL);

	strcpy (name, materialname);

	// Append the texture name to the material name but leave the material name's terminator
	// intact so that conventional string functions only 'see' the material name.
	strcpy (name + materialnamelength, texturepathname);

	MaterialTextureNames.Add (name);
}


/***********************************************************************************************
 * LightscapeSolve::Add_Texture_Name --																		  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/16/99    IML : Created.                                                                 * 
 *=============================================================================================*/
void LightscapeSolve::Add_Texture_Name (unsigned patchindex, const char *materialname)
{
	const char *materialnotfoundtext =
"There is a face in the model that has a material name that cannot be found.";
	
	int		  length;
	char	   **materialnameptr;
	char		 *texturepathname;
	unsigned   texturepathnamelength;

	// Can the texture name data be accomodated by the texture name array?
	length = TextureNames.Length();
	if ((int) patchindex >= length) {
		
		const int growthstep = 1024;	// Add this many elements if the vector array runs out of room.

		bool success = TextureNames.Resize (patchindex + growthstep);
		ASSERT (success);

		// Null out the new elements.
		for (int i = length; i < TextureNames.Length(); i++) {
			TextureNames [i] = NULL;
		}
	}

	// NOTE: The material-texture names MUST have been sorted prior to calling bsearch().
	if (MaterialTextureNames.Count() > 0) {

		const char **keyptr;
		const char  *key;

		key    = materialname;
		keyptr = &key;

		materialnameptr = (char**) bsearch (keyptr, &MaterialTextureNames [0], MaterialTextureNames.Count(), sizeof (char*), Compare_Material_Texture_Names);
	} else {
		materialnameptr = NULL;
	}

	// If the material cannot be found then either the database has malfunctioned or the solve
	// file is corrupt.
	if (materialnameptr == NULL) {
		ASSERT (false);
		throw (materialnotfoundtext);
	}

	// Extract the texture name. 
	texturepathname		 = *materialnameptr + strlen (*materialnameptr) + 1;
	texturepathnamelength = strlen (texturepathname) + 1;

	// Add the name to the database only if it exists (has non-zero length excluding terminator).
	if (texturepathnamelength > 1) {

		TextureNameNode *node;
		
		node = new TextureNameNode (texturepathname);
		ASSERT (node != NULL);

		// Add the texture name to the database.
		node->Next = TextureNames [patchindex];

		TextureNames [patchindex] = node;
	}
}


/***********************************************************************************************
 * LightscapeSolve::Add_Light --																					  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/24/00    IML : Created.                                                                 * 
 *=============================================================================================*/
void LightscapeSolve::Add_Light (LightClass *light)
{
	Lights.Add (light);
	light->Add_Ref();
}


/***********************************************************************************************
 * LightscapeSolve::Add_Vertex --																				  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/25/00    IML : Created.                                                                 * 
 *=============================================================================================*/
bool LightscapeSolve::Add_Vertex (unsigned vertexindex, const Vector3 &p, const Vector3 &n, unsigned patchindex, const Vector2 &t)
{
	bool success;

	// If adding a M2T vertex, ensure that this is a M2T solve.
	ASSERT (Importer()->Is_M2T_Solve());

	// Has the element already been inserted?
	if (((int) vertexindex) < Vertices.Count()) {

		// Check that the new element data matches the existing data.
		success = (Vertices [vertexindex].Point == p) &&
					 (Vertices [vertexindex].FaceNormal == n) &&
					 (Vertices [vertexindex].PatchIndex == patchindex);

  	} else {

		VerticesStruct vertex;

		vertex.Point		= p;
		vertex.FaceNormal = n;
		vertex.PatchIndex = patchindex;
		vertex.ValidColor = false;
		vertex.ValidUV		= false;
		success = Vertices.Insert (vertexindex, vertex);
	}

	if (success) {
		Vertices [vertexindex].UV		 = t;	
		Vertices	[vertexindex].ValidUV = true;
	}

	return (success);
}


/***********************************************************************************************
 * LightscapeSolve::Add_Vertex --																				  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/25/00    IML : Created.                                                                 * 
 *=============================================================================================*/
bool LightscapeSolve::Add_Vertex (unsigned vertexindex, const Vector3 &p, const Vector3 &n, unsigned patchindex, const ColorVector &c)
{
	bool success;

	// Has the element already been inserted?
	if (((int) vertexindex) < Vertices.Count()) {

		// Check that the new element data matches the existing data.
		success = (Vertices [vertexindex].Point == p) &&
					 (Vertices [vertexindex].FaceNormal == n) &&
					 (Vertices [vertexindex].PatchIndex == patchindex);

  	} else {

		VerticesStruct vertex;

		vertex.Point		= p;
		vertex.FaceNormal = n;
		vertex.PatchIndex = patchindex;
		vertex.ValidColor = false;
		vertex.ValidUV		= false;
		success = Vertices.Insert (vertexindex, vertex);
	}

	if (success) {
	
		// Only add/set the color if this is not a M2T solve. 
		if (!Importer()->Is_M2T_Solve()) {
			
			// Add or set the color?
			if (Vertices [vertexindex].ValidColor) {

				const ColorVector clamp (1.0f, 1.0f, 1.0f);
					
				// Add and clamp.
				Vertices [vertexindex].Color += c;
				Vertices [vertexindex].Color.Update_Min (clamp);

			} else {

				Vertices [vertexindex].Color		 = c;
				Vertices [vertexindex].ValidColor = true;
			}
		}
	}
	return (success);
}


/***********************************************************************************************
 * LightscapeSolve::Add_Patch_Face --																			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/16/99    IML : Created.                                                                 * 
 *=============================================================================================*/
bool LightscapeSolve::Add_Patch_Face (unsigned patchfaceindex, unsigned patchindex, unsigned vertexindexcount, unsigned *vertexindices)
{
	bool success;

 	// Only 3 or 4 vertex indices are supported.
	ASSERT (vertexindexcount == 3 || vertexindexcount == 4);

	// Has the element already been inserted?
	if (((int) patchfaceindex) < PatchFaces.Count()) {

		// Check that the new element data matches the existing data.
		success = (patchindex == PatchFaces [patchfaceindex].PatchIndex) &&
					 (vertexindices [0] == PatchFaces [patchfaceindex].VertexIndices [0]) &&
					 (vertexindices [1] == PatchFaces [patchfaceindex].VertexIndices [1]) &&
					 (vertexindices [2] == PatchFaces [patchfaceindex].VertexIndices [2]); 

		if (vertexindexcount == 4) {
			success &= (vertexindices [3] == PatchFaces [patchfaceindex].VertexIndices [3]);
		}
	
	} else {

		PatchFaceStruct patchface;

		patchface.PatchIndex = patchindex;
		patchface.VertexIndices [0] = vertexindices [0];
		patchface.VertexIndices [1] = vertexindices [1];
		patchface.VertexIndices [2] = vertexindices [2];
		if (vertexindexcount == 4) {
			patchface.VertexIndices [3] = vertexindices [3];
		} else {
			patchface.VertexIndices [3] = PatchFaceStruct::VERTEX_INDEX_INVALID;
		}
		success = PatchFaces.Insert (patchfaceindex, patchface);
	}

	return (success);
}


/***********************************************************************************************
 * LightscapeSolve::Find_Vertex --																				  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/07/00    IML : Created.                                                                 * 
 *=============================================================================================*/
void LightscapeSolve::Find_Vertex (const Vector3 &point, const Vector3 *smoothingnormalptr, W3dRGBStruct &vertexcolor, SolveStatistics &solvestatistics)
{
	const int			 growthstep = 256;
	const W3dRGBStruct pink (255, 0, 255);

	DynamicVectorClass <VerticesStruct*> vertexset;
	ColorVector									 color;
	unsigned										 count;

	vertexset.Set_Growth_Step (growthstep);
	Find_Vertices (point, SpatialTolerance, vertexset);
	if (vertexset.Count() > 0) {

		color.Set (0.0f, 0.0f, 0.0f);
		count = 0;
		if (smoothingnormalptr != NULL) {
		
			// Sum the colors of all vertices that lie within the smoothing normal.
			for (int v = 0; v < vertexset.Count(); v++) {

				if (vertexset [v]->ValidColor) {

					float angle;

					// Is the face that is associated with this vertex within the smoothing angle
					// of the supplied vertex normal?
					angle = acosf (Vector3::Dot_Product (*smoothingnormalptr, vertexset [v]->FaceNormal));
					if (angle <= SmoothingAngle) {
						color += vertexset [v]->Color;
						count++;
					}
				}
			}

	  		if (count > 0) {
				color /= ((float) count);
				vertexcolor.Set (color.X, color.Y, color.Z);
				return;
			}
		}
			
		// Sum the colors of all vertices regardless of smoothing normal.
		for (int v = 0; v < vertexset.Count(); v++) {
			if (vertexset [v]->ValidColor) {
				color += vertexset [v]->Color;
				count++;
			}
		}

		if (count > 0) {
		 	color /= ((float) count);
		 	vertexcolor.Set (color.X, color.Y, color.Z);
		 	solvestatistics.Count [SolveStatistics::VERTEX_NOT_SMOOTH]++;
			return;
		} else {
		
			// There are no vertices with a valid color.	Return a place-holder color.
			vertexcolor = pink;
			solvestatistics.Count [SolveStatistics::VERTEX_NO_COLOR]++;
			return;
		}
	}
	
	//	Vertex not found. Return a place-holder color.
	vertexcolor = pink;
	solvestatistics.Count [SolveStatistics::VERTEX_NOT_FOUND]++;
	return;
}


/***********************************************************************************************
 * LightscapeSolve::Find_Triangle --																			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/07/00    IML : Created.                                                                  * 
 *=============================================================================================*/
void LightscapeSolve::Find_Triangle (const Vector3 *points, const Vector3 &normal, PackingTriangle &triangle, SolveStatistics &solvestatistics)
{
	const unsigned notextureid	= 0xffffffff;	// Arbitrary ID used to indicate that there is no associated texture.

	VerticesStruct trianglevertices [Triangle::VERTICES_COUNT];
	FaceResultEnum	faceresult;	
	bool				notexture;
	unsigned			v;	

	ASSERT (points != NULL);

	// First, attempt a match with the face itself.
	switch (Face_Size (points, Triangle::VERTICES_COUNT)) {

		case FACE_UNDERSIZED:
		case FACE_NOT_UNDERSIZED:
			Find_Triangle (points, normal, trianglevertices, faceresult);
			break;

		case FACE_DEGENERATE:
			solvestatistics.Count [SolveStatistics::FACE_DEGENERATE]++;
			faceresult = FACE_NOT_FOUND;
			break;
	}

	notexture = false;
	switch (faceresult) {

		case FACE_AMBIGUOUS:
			solvestatistics.Count [SolveStatistics::FACE_AMBIGUOUS]++;
			// No break.

		case FACE_FOUND:
		{
			TextureNameNode *texturenamenodeptr;
			bool				  validuvs;

			// Check that texture exists and that all UV's are valid.
			texturenamenodeptr = (TextureNameNode*) Texture_Name (trianglevertices [0].PatchIndex);
			if (texturenamenodeptr != NULL) {
				validuvs = true;
				for (v = 0; v < Triangle::VERTICES_COUNT; v++) {
					validuvs &= trianglevertices [v].ValidUV;
				}
				if (validuvs) {
					triangle.TextureNameNodePtr = texturenamenodeptr;
					triangle.TextureID			 = trianglevertices [0].PatchIndex;
					triangle.Normal				 = trianglevertices [0].FaceNormal;
					for (v = 0; v < Triangle::VERTICES_COUNT; v++) {
						triangle.Vertices [v].Point = trianglevertices [v].Point;
						triangle.Vertices [v].UV	 = trianglevertices [v].UV;
					}
					break;
				}
			}
			notexture = true;
			solvestatistics.Count [SolveStatistics::FACE_NO_LIGHTMAP]++;
			break;
		}

		case FACE_NOT_FOUND:
			notexture = true;
			solvestatistics.Count [SolveStatistics::FACE_NOT_FOUND]++;
			break;
	}

	if (notexture) {

		// Return the original triangle with no associated texture.
		triangle.Normal = normal;
		for (v = 0; v < Triangle::VERTICES_COUNT; v++) {
			triangle.Vertices [v].Point = points [v];
		}
		triangle.TextureNameNodePtr = NULL;
		triangle.TextureID			 = notextureid;
	}
}


/***********************************************************************************************
 * LightscapeSolve::Submit_Triangle --																			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/07/00    IML : Created.                                                                * 
 *=============================================================================================*/
void LightscapeSolve::Submit_Triangle (PackingTriangle &triangle)
{			
	DynamicVectorClass <Triangle> adjtriangles;
	
	adjtriangles.Set_Growth_Step (32);
	Find_Adjacent_Triangles (triangle, adjtriangles);
	Submit (&triangle, adjtriangles);
}


/***********************************************************************************************
 * LightscapeSolve::Find_Adjacent_Triangles --																  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *  6/05/00    IML : Created.                                                                  * 
 *=============================================================================================*/
void LightscapeSolve::Find_Adjacent_Triangles (const PackingTriangle &principaltriangle, DynamicVectorClass <Triangle> &adjtriangles)
{
	DynamicVectorClass <VerticesStruct*> spatialvertexset;

	// For each principal triangle vertex...
	for (unsigned v = 0; v < Triangle::VERTICES_COUNT; v++) {

		// Find all vertices that lie within spatial tolerance of this principal triangle vertex.
		spatialvertexset.Clear();
		Find_Vertices (principaltriangle.Vertices [v].Point, SpatialTolerance, spatialvertexset);
		for (int s = 0; s < spatialvertexset.Count(); s++) {
			
			Vector3 normal;
			float   angle;

			// Is the face that is associated with this spatial vertex within the smoothing angle of the principal triangle's normal?
			normal = spatialvertexset [s]->FaceNormal;				
			angle  = acosf (Vector3::Dot_Product (principaltriangle.Normal, normal));
			if (angle <= SmoothingAngle) {

				TextureNameNode *texturenamenodeptr;
				unsigned			  textureid;	

				// Is the texture name valid?
				texturenamenodeptr = (TextureNameNode*) Texture_Name (spatialvertexset [s]->PatchIndex);
				textureid			 = spatialvertexset [s]->PatchIndex;	
				if (texturenamenodeptr != NULL) {

					PatchFaceStruct p, *patchfaceptr;

					// Find a patch face in the database with patch index that matches the patch index of the spatial vertex.
					p.PatchIndex = spatialvertexset [s]->PatchIndex;
					patchfaceptr = (PatchFaceStruct*) bsearch (&p, &PatchFaces [0], PatchFaces.Count(), sizeof (PatchFaceStruct), Compare_Patch_Faces);
	
					// Does a patch exist?
					if (patchfaceptr != NULL) {

						// Step backwards in the database to the first patch that matches.
						// NOTE: bsearch() does not necessarily return the first key in the array that matches	when there are duplicate keys.
						while (patchfaceptr > &PatchFaces [0]) {
							if (Compare_Patch_Faces (&p, patchfaceptr - 1) != 0) break;
							patchfaceptr--;
						}

						// Step forwards in the database while there is a patch match.
						while ((patchfaceptr < &PatchFaces [0] + PatchFaces.Count()) && (Compare_Patch_Faces (&p, patchfaceptr) == 0)) {

							unsigned trianglecount;

							// Is this patch face a triangle or a quadrilateral?
							trianglecount = (patchfaceptr->VertexIndices [PatchFaceStruct::VERTICES_PER_PATCH_FACE - 1] == PatchFaceStruct::VERTEX_INDEX_INVALID) ? 1 : 2;

							// For each triangle...
							for (unsigned t = 0; t < trianglecount; t++) {

								const unsigned _vertexindex [2][3] = {{0, 1, 2}, {0, 2, 3}};

								bool		validuvs;
								unsigned v;

								validuvs = true;
								for (v = 0; v < Triangle::VERTICES_COUNT; v++) {
									validuvs &= Vertices [patchfaceptr->VertexIndices [_vertexindex [t][v]]].ValidUV;
								}	
							
								if (validuvs) {
								
									Triangle triangle;
									bool		hasequivalent;

									triangle.TextureNameNodePtr = texturenamenodeptr;
									triangle.TextureID			 = textureid;
									triangle.Normal				 = normal;	
									for (v = 0; v < Triangle::VERTICES_COUNT; v++) {
										triangle.Vertices [v].Point = Vertices [patchfaceptr->VertexIndices [_vertexindex [t][v]]].Point;
										triangle.Vertices [v].UV	 = Vertices [patchfaceptr->VertexIndices [_vertexindex [t][v]]].UV;
									}
										 
									// Linearly search the existing adjacent triangles to see if this triangle already exists.
									hasequivalent = false;
									for (int a = 0; a < adjtriangles.Count(); a++) {
										if (triangle.Is_Equivalent (adjtriangles [a])) {
											hasequivalent = true;
											break;
										}
									}
									if (!hasequivalent) adjtriangles.Add (triangle);
								}
							}

							// Advance to next patch face.
							patchfaceptr++;
						}
					}
				}
			}
		}
	}
}


/***********************************************************************************************
 * LightscapeSolve::Find_Triangle --																			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/27/99    IML : Created.                                                                 * 
 *=============================================================================================*/
void LightscapeSolve::Find_Triangle (const Vector3 *points, const Vector3 &facenormal, VerticesStruct *vertices, FaceResultEnum &faceresult)
{
	const unsigned attemptcount = 14; // No. of binary search attempts (see note below).

	ASSERT (points != NULL);

	// Attempt to find the face with zero tolerance.
	Find_Triangle (points, 0.0f, true, &facenormal, vertices, faceresult);
		
	// If the face was not found then attempt to find a face by iterating over a range of 
	// tolerances using a binary search technique.
	if (faceresult == FACE_NOT_FOUND) {
			
		bool  faceambiguous;
		float maxtolerance = SpatialTolerance;
		float mintolerance = 0.00f;
		float	tolerance;
			
		tolerance = (maxtolerance + mintolerance) * 0.5f;
			
		// NOTE: The tolerance will be increased or decreased, depending upon whether the
		// current result is 'not found' or 'ambiguous'. On each iteration the tolerance
		// range (max tolerance - min tolerance) is halved. Thus, after n iterations, the
		// tolerance range will be (max tolerance - min tolerance) / 2 ^ n. I have purposely
		// set n, the number of attempts, to yield a range at the limit of floating point
		// accuracy.
		faceambiguous = false;
		for (unsigned attempt = 0; attempt < attemptcount; attempt++) {
			Find_Triangle (points, tolerance, false, NULL, vertices, faceresult);
			if (faceresult == FACE_AMBIGUOUS) {
					
				// Flag that an ambiguous result occured.
				faceambiguous = true;

				// Decrease the tolerance.
				maxtolerance = tolerance;
				tolerance = (tolerance + mintolerance) * 0.5f;
			} else {
				if (faceresult == FACE_NOT_FOUND) {
						
					// Increase the tolerance.
					mintolerance = tolerance;
					tolerance = (tolerance + maxtolerance) * 0.5f;
				} else {
					
					// The face has been found.
					ASSERT (faceresult == FACE_FOUND);
					break;
				}
			}
		}

		// If an ambiguous result occured on ANY iteration then the maximum tolerance is the
		// smallest tolerance at which the ambiguity exists. Try to resolve the ambiguity
		// using the centroid and normal tests.
		if ((faceresult != FACE_FOUND) && faceambiguous) {
			Find_Triangle (points, maxtolerance, true, &facenormal, vertices, faceresult);
		}
	}
}	


/***********************************************************************************************
 * LightscapeSolve::Find_Triangle --																			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/27/99    IML : Created.                                                                 * 
 *=============================================================================================*/
void LightscapeSolve::Find_Triangle (const Vector3 *points, float tolerance, bool centroidtest, const Vector3 *facenormal, VerticesStruct *vertices, FaceResultEnum &faceresult)
{
	const int growthstep = 256;	

	DynamicVectorClass <VerticesStruct*> vertexset [Triangle::VERTICES_COUNT];
	DynamicVectorClass <VerticesStruct*> *candidatevertexset = &vertexset [0];
	unsigned patchindex;
	int		n, c, v;
	bool		success;

	ASSERT (points != NULL);

	// Create a set of vertex indices that correspond to the zeroth point.
	candidatevertexset->Set_Growth_Step (growthstep);
	Find_Vertices (points [0], tolerance, *candidatevertexset);

	// Remove any duplicate patches from the candidate set.
	c = 0;
	while (c < candidatevertexset->Count()) {
		patchindex = (*candidatevertexset) [c]->PatchIndex;
		v = c + 1;
		while (v < candidatevertexset->Count()) {
			if (patchindex == (*candidatevertexset) [v]->PatchIndex) {
				success = candidatevertexset->Delete (v);
				ASSERT (success);
			} else {
				v++;
			}
		}
		c++;
	}

	// For all other points...
	for (n = 1; n < Triangle::VERTICES_COUNT; n++) {
		
		// Create a set of vertex indices that correspond to n'th <point, normal> pair.
		vertexset [n].Set_Growth_Step (growthstep);
		Find_Vertices (points [n], tolerance, vertexset [n]);

		// Delete all indexes in the candidate index set that have materials that DO NOT occur in the vertex index set.
		c = 0;
		while (c < candidatevertexset->Count()) {
			
			bool found;

			patchindex = (*candidatevertexset) [c]->PatchIndex; 
			found = false;
			for (v = 0; v < vertexset [n].Count(); v++) {
				if (patchindex == vertexset [n][v]->PatchIndex) {
					found = true;
					break;
				}
			}
			if (!found) {
				success = candidatevertexset->Delete (c);
				ASSERT (success);
			} else {
				c++;
			}
		}
	}

	// If there is more than one candidate and the centroid test can be applied then apply it.
	if ((candidatevertexset->Count() > 1) && centroidtest) {
			
		// If there is a patch-face database...
		if (PatchFaces.Count() > 0) {

			// Attempt to eliminate the ambiguity by applying a centroid test.
			Vector3 centroid;

			centroid = points [0];
			for (n = 1; n < Triangle::VERTICES_COUNT; n++) {
				centroid += points [n];
			}
			centroid /= (float) Triangle::VERTICES_COUNT;

			// For each candidate...
			c = 0;
			while (c < candidatevertexset->Count()) {

				bool				  contained;	
				PatchFaceStruct  p;
				PatchFaceStruct *patchfaceptr;
				unsigned			  trianglecount;	

				patchindex = (*candidatevertexset) [c]->PatchIndex;
				contained  = false;
					
				// Find a face in the array that matches the patch.
				p.PatchIndex = patchindex;
				patchfaceptr = (PatchFaceStruct*) bsearch (&p, &PatchFaces [0], PatchFaces.Count(), sizeof (PatchFaceStruct), Compare_Patch_Faces);
	
				if (patchfaceptr != NULL) {
	 
					// Step backwards in the array to the first patch that matches.
					// NOTE: bsearch() does not necessarily return the first key in the array that matches	when there are duplicate keys.
					while (patchfaceptr > &PatchFaces [0]) {
						if (Compare_Patch_Faces (&p, patchfaceptr - 1) != 0) break;
						patchfaceptr--;
					}

					// Step forwards in the array while there is a patch match.
					// Find all vertices whose points and normals approximately match the requested normal and add their patch indices to the patch array.
					while ((patchfaceptr < &PatchFaces [0] + PatchFaces.Count()) && (Compare_Patch_Faces (&p, patchfaceptr) == 0)) {
						
						// NOTE: The candidate point may not match the any of the material face 
						// points because Lightscape may have changed the topology. Thus it is
						// necessary to search ALL of the faces that have this material.

						// For each triangle in the patch face...
						// NOTE: Define the first triangle to be the points 0-1-2. If the patch face is a quadrilateral,
						//			then define the second triangle to be the points 0-2-3.
						trianglecount = (patchfaceptr->VertexIndices [3] == PatchFaceStruct::VERTEX_INDEX_INVALID) ? 1 : 2;
						for (unsigned t = 0; t < trianglecount; t++) {

							// Create a triangle from the material face and test it against the centroid for containment.
							TriClass triangle;
							Vector3	normal;

							triangle.V [0] = &Vertices [patchfaceptr->VertexIndices [0]].Point;
							triangle.V [1] = &Vertices [patchfaceptr->VertexIndices [1 + t]].Point;
							triangle.V [2] = &Vertices [patchfaceptr->VertexIndices [2 + t]].Point;
							triangle.N		= &normal;
							triangle.Compute_Normal();

							if (triangle.Contains_Point (centroid)) {
								contained = true;
								break;
							}
						}

						if (contained) break;

						// Advance to next patch face.
						patchfaceptr++;
					}
				}

				if (!contained) {
					success = candidatevertexset->Delete (c);
					ASSERT (success);
				} else {
					c++;
				}
			}
		}
	}

	// If there is still more than one candidate and the normal exists then apply the normal test.
	if ((candidatevertexset->Count() > 1) && (facenormal != NULL)) {

		float largestdp;
		
		// Find the smallest angle (largest dot product) between the vertex face normal and the target face normal.
		largestdp = (*facenormal) * ((*candidatevertexset) [0]->FaceNormal);
		for (c = 1; c < candidatevertexset->Count(); c++) {
			largestdp = MAX (largestdp, (*facenormal) * ((*candidatevertexset) [c]->FaceNormal));
		}
	
		// Delete all candidates that do not have the largest dot product.
		c = 0;
		while (c < candidatevertexset->Count()) {

			float dp = (*facenormal) * ((*candidatevertexset) [c]->FaceNormal);
	
			if (dp < largestdp) {
				success = candidatevertexset->Delete (c);
				ASSERT (success);
			} else {
				c++;
			}	
		}
	}

	// Now if there are no candidates then return failure.
	if	(candidatevertexset->Count() == 0) {
		faceresult = FACE_NOT_FOUND;
	} else {

		unsigned candidateindex = 0;

		// NOTE: If there is >1 candidate at this point the result is still ambiguous.
		// If the face normal has been supplied, select the candidate with closest
		// face normal - otherwise select the zeroth candidate.
		if (facenormal != NULL) {

			float mindistance = FLT_MAX;

			for (n = 0; n < candidatevertexset->Count(); n++) {
				
				float distance;

				distance = Manhatten_Distance (*facenormal - (*candidatevertexset) [n]->FaceNormal);
				if (distance < mindistance) {
					mindistance = distance;
					candidateindex = n;
				}
			}
		}
		
		// For the zeroth point...
		patchindex = (*candidatevertexset) [candidateindex]->PatchIndex;
		vertices [0] = *((*candidatevertexset) [candidateindex]);

		// For all other points...
		for (n = 1; n < Triangle::VERTICES_COUNT; n++) {
					
			bool assigned = false;
						
			for (v = 0; v < vertexset [n].Count(); v++) {
				if (vertexset [n][v]->PatchIndex == patchindex) {
					vertices [n] = *(vertexset [n][v]);
					assigned = true;
					break;
				}
			}
			ASSERT (assigned);
		}

		// Flag if the result was ambiguous.
		if	(candidatevertexset->Count() > 1) {
			faceresult = FACE_AMBIGUOUS;
		} else {
			
			// There must be exactly one candidate.  
			ASSERT (candidatevertexset->Count() == 1);
			faceresult = FACE_FOUND;
		}
	}
}


/***********************************************************************************************
 * LightscapeSolve::Find_Vertices --																			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/27/99    IML : Created.                                                                 * 
 *=============================================================================================*/
void LightscapeSolve::Find_Vertices (const Vector3 &point, float tolerance, DynamicVectorClass <VerticesStruct*> &vertices)
{
	if (VertexIndices.Count() > 0) {

		unsigned *vertexindexptr = NULL;
		int		 b0, b1, b;
		float		 x;

		// There should be a 1-1 mapping between vertex indices and vertices.
		ASSERT (VertexIndices.Count() == Vertices.Count());

		// Find a vertex that matches point to within tolerance using binary search technique.
		b0 = 0;
		b1 = VertexIndices.Count() - 1;
		do {

			b = (b0 + b1) >> 1;
			x = Vertices [VertexIndices [b]].Point.X;
			if (point.X - x < -tolerance) {
				b1 = b - 1;
			} else {
				if (point.X - x > tolerance) {
					b0 = b + 1;
				} else {
					vertexindexptr = &VertexIndices [b];
					break;
				}
			}
	
		} while (b0 <= b1);

		// Step backwards in the array to the first vertex that meets the match criteria defined by Compare_Vertices().
		// NOTE: Above search did not necessarily return the first key in the array that matches when there are duplicate keys.
		if (vertexindexptr != NULL) {
			while (vertexindexptr > &VertexIndices [0]) {
				x = Vertices [*(vertexindexptr - 1)].Point.X;
				if (point.X - x > tolerance) break;
				vertexindexptr--;
			}

			// Step forwards in the array while there is an approximate vertex match.
			// Add all vertices that are valid and whose points match the requested point within tolerance.
			while (vertexindexptr < &VertexIndices [0] + VertexIndices.Count()) {
				
				VerticesStruct *vertexptr;

				vertexptr = &Vertices [*vertexindexptr];
				if (point.X - vertexptr->Point.X < -tolerance) break;
				if (Manhatten_Distance (vertexptr->Point - point) <= tolerance) {
					vertices.Add (vertexptr);
				}

				// Advance to next vertex index.
				vertexindexptr++;
			}
		}
	}
}


/***********************************************************************************************
 * LightscapeSolve::Face_Size --																					  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/17/99    IML : Created.                                                                 * 
 *=============================================================================================*/
LightscapeSolve::FaceSizeEnum LightscapeSolve::Face_Size (const Vector3 *points, unsigned count)
{
	const float undersizedarea = 0.0016f;	// An area << the area of a lightmap lumel (assuming
														// that lumel density is less than 625 lumels/metre).
	float area;

	// Test for degeneracy.
	// NOTE: Assume that the points are ordered (for triangles this is guaranteed). 
	for (unsigned n = 0; n < count; n++) {
		if (points [n] == points [(n + 1) % count]) return (FACE_DEGENERATE);
	}	

	// Currently, the area calculation requires that the polygon is a triangle.
	ASSERT (count == 3);
	
	// Test for a small area. 
	area = 0.5f * Vector3::Cross_Product (points [1] - points [0], points [2] - points [0]).Length();
	if (area < undersizedarea) return (FACE_UNDERSIZED);
	return (FACE_NOT_UNDERSIZED);
}


/***********************************************************************************************
 * LightscapeSolve::Longest_Edge --																				  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/17/99    IML : Created.                                                                 * 
 *=============================================================================================*/
unsigned LightscapeSolve::Longest_Edge (const Vector3 *points, unsigned count, bool *usededges)
{
	float		maxlength = -1.0f;
	float		length;
	unsigned edgeindex;

	for (unsigned e = 0;	e < count; e++) {
		if (!usededges [e]) {
			length = (points [(e + 1) % count] - points [e]).Length();
			if (length > maxlength) {
				maxlength = length;
				edgeindex = e;
			}
		}
	}
	
	// An edge must have been found.
	ASSERT (maxlength >= 0.0f);
	usededges [edgeindex] = true;
	return (edgeindex);
}


/***********************************************************************************************
 * LightscapeSolve::Texture_Name --																				  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/27/99    IML : Created.                                                                 * 
 *=============================================================================================*/
const TextureNameNode *LightscapeSolve::Texture_Name (unsigned patchindex)
{
	// If the array element does not exist yet then the name has not been set.
	if ((int) patchindex >= TextureNames.Length()) return (NULL);
	return TextureNames [patchindex];
}


/***********************************************************************************************
 * LightscapeSolve::Compare_Material_Texture_Names --														  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/27/99    IML : Created.                                                                 * 
 *=============================================================================================*/
int LightscapeSolve::Compare_Material_Texture_Names (const void *materialname0, const void *materialname1)
{
	return (strcmp (*((const char**) materialname0), *((const char**) materialname1)));
}


/***********************************************************************************************
 * LightscapeSolve::Compare_Vertices																			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/27/99    IML : Created.                                                                 * 
 *=============================================================================================*/
int LightscapeSolve::Compare_Vertices (const void *vertexindexptr0, const void *vertexindexptr1)
{
	float x0, x1;

	ASSERT (_ActiveImporter != NULL);

	x0 = _ActiveImporter->Vertices [*((unsigned*) vertexindexptr0)].Point.X;
	x1 = _ActiveImporter->Vertices [*((unsigned*) vertexindexptr1)].Point.X;

	if (x0 < x1) {
		return (-1);
	} else {
		if (x0 > x1) {
			return (1);
		} else {
			return (0);
		}
	}
}


/***********************************************************************************************
 * LightscapeSolve::Compare_Patch_Faces --																	  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/17/99    IML : Created.                                                                 * 
 *=============================================================================================*/
int LightscapeSolve::Compare_Patch_Faces (const void *patchface0, const void *patchface1)
{
	unsigned patchindex0 = ((PatchFaceStruct*) patchface0)->PatchIndex;
	unsigned patchindex1 = ((PatchFaceStruct*) patchface1)->PatchIndex;

	if (patchindex0 < patchindex1) {
		return (-1);
	} else {
		if (patchindex0 > patchindex1) {
			return (1);
		} else {
			return (0);
		}
	}
}


/***********************************************************************************************
 * LightscapeSolve::Pack --																						  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/07/00    IML : Created.                                                                 * 
 *=============================================================================================*/
void LightscapeSolve::Pack()
{
	LightmapPacker::Pack (ProceduralTexture);
}


/***********************************************************************************************
 * LsInformationFactory::OnGet*Builder --																		  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/16/99    IML : Created.                                                                 * 
 *=============================================================================================*/
LtTInfoBuilderApi* LsInformationFactory::OnGetInfoBuilder()
{
	return (new LsInfoBuilder (this));
}

LtTParameterBuilderApi* LsInformationFactory::OnGetParameterBuilder()
{
	return (new LsParameterBuilder (this));
}


/***********************************************************************************************
 * LsPreparationFactory::OnGet*Builder --																		  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/27/99    IML : Created.                                                                 * 
 *=============================================================================================*/
LtTInfoBuilderApi *LsPreparationFactory::OnGetInfoBuilder()
{
	return (new LsInfoBuilder (this));
}

LtTParameterBuilderApi *LsPreparationFactory::OnGetParameterBuilder()
{
	return (new LsParameterBuilder (this));
}

LtTMaterialBuilderApi *LsPreparationFactory::OnGetMaterialBuilder()
{
	return (new LsMaterialBuilder (this));
}

LtTLampBuilderApi *LsPreparationFactory::OnGetLampBuilder()
{
	return (new LsLampBuilder (this));
}

LtTMeshBuilderApi *LsPreparationFactory::OnGetMeshBuilder()
{
	return (new LsMeshInquirer (this));
}


/***********************************************************************************************
 * LsMainFactory::LsMainFactory --																				  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/23/99    IML : Created.                                                                 * 
 *=============================================================================================*/
LsMainFactory::LsMainFactory()
	: LtTBuilderFactory()
{
	PatchFaceIndex = 0;
	VertexIndex		= 0;
	PatchIndex		= 0;
}


/***********************************************************************************************
 * LsMainFactory::OnGet*Builder --																				  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/23/99    IML : Created.                                                                 * 
 *=============================================================================================*/
LtTInfoBuilderApi* LsMainFactory::OnGetInfoBuilder()
{
	return (new LsInfoBuilder (this));
}

LtTParameterBuilderApi* LsMainFactory::OnGetParameterBuilder()
{
	return (new LsParameterBuilder (this));
}

LtTMeshBuilderApi* LsMainFactory::OnGetMeshBuilder()
{
	return (new LsMeshBuilder (this));
}


/***********************************************************************************************
 * LsInfoBuilder::LsInfoBuilder --																				  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/17/99    IML : Created.                                                                 * 
 *=============================================================================================*/
LsInfoBuilder::LsInfoBuilder (LtTBuilderFactory *factory)
	: LtTBaseInfoBuilder (factory)
{
	// Do any special initialization here.
}


/***********************************************************************************************
 * LsInfoBuilder::Finish --																						  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/17/99    IML : Created.                                                                 * 
 *=============================================================================================*/
LtTBool LsInfoBuilder::Finish()
{
	// Read any info values here.
	LightscapeSolve::Importer()->Set_Is_Solution (IsSolution() ? true : false);
	LightscapeSolve::Importer()->Set_Patch_Cluster_Count (GetNumGroups());
	LightscapeSolve::Importer()->Set_Brightness (GetBrightness());
	LightscapeSolve::Importer()->Set_Contrast (GetContrast());
	
	// Return success.
	return (true);
}


/***********************************************************************************************
 * LsParameterBuilder::LsParameterBuilder --																	  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/17/99    IML : Created.                                                                 * 
 *=============================================================================================*/
LsParameterBuilder::LsParameterBuilder (LtTBuilderFactory *factory)
	: LtTBaseParameterBuilder (factory)
{
	// Do any special initialization here.
}


/***********************************************************************************************
 * LsParameterBuilder::Finish --																					  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/17/99    IML : Created.                                                                 * 
 *=============================================================================================*/
LtTBool LsParameterBuilder::Finish()
{
	// Read any parameter values here.
	LightscapeSolve::Importer()->Set_Is_Daylight ((GetFlags() & LT_PROCESS_DAYLIGHT) != 0); 
	LightscapeSolve::Importer()->Set_Is_Exterior ((GetFlags() & LT_PROCESS_EXTERIOR) != 0); 

	// Return success.
	return (true);
}


/***********************************************************************************************
 * LsMateriaBuilder::LsMaterialBuilder --																		  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/27/99    IML : Created.                                                                 * 
 *=============================================================================================*/
LsMaterialBuilder::LsMaterialBuilder (LtTBuilderFactory *factory)
	: LtTBaseMaterialBuilder (factory)
{
	// Do any special initialization here.
}


/***********************************************************************************************
 * LsMateriaBuilder::Finish --																					  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/27/99    IML : Created.                                                                 * 
 *=============================================================================================*/
LtTBool LsMaterialBuilder::Finish()
{
	ASSERT (LightscapeSolve::Importer() != NULL);
	LightscapeSolve::Importer()->Add_Material_Texture_Name (GetName(), GetTextureName());	
	
	// Return success.
	return (true);
}


/***********************************************************************************************
 * LsLampBuilder::LsLampBuilder --																				  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/24/00    IML : Created.                                                                 * 
 *=============================================================================================*/
LsLampBuilder::LsLampBuilder (LtTBuilderFactory *factory)
	: LtTBaseLampBuilder (factory)
{
	// Do any special initialization here.
}


/***********************************************************************************************
 * LsLampBuilder::Finish --																						  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/24/00    IML : Created.                                                                 * 
 *=============================================================================================*/
LtTBool LsLampBuilder::Finish()
{
		const char *unknownlighttext =
"There is a light in the solve with unknown type.";

	const	Vector3 oodistancesquaredfactors (0.0f, 0.0f, 1.0f);
	const Vector3 zaxis (0.0f, 0.0f, 1.0f);
	const ColorVector black (0.0f, 0.0f, 0.0f);
	const ColorVector white (1.0f, 1.0f, 1.0f);

	float			 dir [3][3];
	LightClass	*light;
	Vector3		 position;
	ColorVector	 color;
	Matrix3D		 transform;

	ASSERT (LightscapeSolve::Importer() != NULL);

	// If the light name begins with a '@' skip it - it is not intended for export.
	if (LightscapeSolve::Importer()->Light_Exclusion_String() != NULL) {
		if (strstr (GetName(), LightscapeSolve::Importer()->Light_Exclusion_String())) return (true);
	}

	// Skip skylight - it cannot be meaningfully translated into a LightClass object.
	if ((GetType() == LT_LAMP_DAYLIGHT) && (strcmp (GetName(), "Skylight") == 0)) return (true);

	// If the lamp does not store direct illumination and has not been raytraced then
	// it has not been included in the solve - skip it.
	if ((GetFlags() & LT_LAMP_CAST_NO_DIRECT) && !(GetFlags() & LT_LAMP_RAY_TRACE_DIRECT)) return (true);

	// If this light has non-zero intensity then add the light to the solve database.
	if (GetIntensity() > 0.0f) {
	
		// NOTE 0: Lightscape uses a right-handed coordinate system.
		// NOTE 1: For directional lights (including sunlight), multiplying the direction matrix
		//			  by (0, 0, 1) will yield the direction of the light rays.
		GetDirection (dir);
		Matrix3 direction (dir [0][0], dir [0][1], dir [0][2],
								 dir [1][0], dir [1][1], dir [1][2],
								 dir [2][0], dir [2][1], dir [2][2]);

		// Create light of appropriate type.
		switch (GetType()) {

			case LT_LAMP_ISOTROPIC:		// Spherical equal intensity distribution.

				light = new LightClass (LightClass::POINT);
				ASSERT (light != NULL);
				break;
		
			case LT_LAMP_DIFFUSE:		// Hemispherical distribution with intensity proportional to the cosine to the direction of light.
			
				// Interpret this light as a spotlight with a 180 degree spot angle.
				light = new LightClass (LightClass::SPOT);
				ASSERT (light != NULL);
				light->Set_Spot_Angle (WWMATH_PI);
				light->Set_Spot_Direction (zaxis);
				
				// For efficiency, use unity as exponent.
				light->Set_Spot_Exponent (1.0f);
				break;
			
			case LT_LAMP_SPOTLIGHT:		// Spotlight distribution. Intensity at beam angle is one-half maximum intensity. Intensity is truncated to 0 at field angle.
			
				light = new LightClass (LightClass::SPOT);
				ASSERT (light != NULL);
				
				// NOTE: Field angle is defined as angle swept from center of beam to outside.
				light->Set_Spot_Angle (GetFieldAngle() * 2.0f);
				light->Set_Spot_Direction (zaxis);

				// For efficiency, use unity as exponent.
				light->Set_Spot_Exponent (1.0f);
				break;

			case LT_LAMP_GENERAL:		// General distribution is determined from table if intensities in given directions. 
			
				{
					float angle;

					// Get the widest angle - this is normally, but not always, the field angle.
					angle = MAX (GetBeamAngle(), GetFieldAngle()); 
					if (angle > WWMATH_PI / 2.0f) {

						// Interpret light as an isometric light.
						light = new LightClass (LightClass::POINT);
						ASSERT (light != NULL);

					} else {
				
				  		// Interpret light as a spotlight.
						light = new LightClass (LightClass::SPOT);
						ASSERT (light != NULL);
				
						light->Set_Spot_Angle (angle * 2.0f);
						light->Set_Spot_Direction (zaxis);
						
						// For efficiency, use unity as exponent.
						light->Set_Spot_Exponent (1.0f);
					}
				}
				break;

			case LT_LAMP_DAYLIGHT:		// Special value used to flag daylight when import an LS file.

				light = new LightClass (LightClass::DIRECTIONAL);
				ASSERT (light != NULL);
				break;

			default:
				ASSERT (false);
				throw (unknownlighttext);
				break;
		}

		// Set transform matrix that defines direction and position of light source.
		position.Set (GetLocation().GetX(), GetLocation().GetY(), GetLocation().GetZ());
		transform.Set (direction, position);
		light->Set_Transform (transform);

		// Set intensity.
		light->Set_Intensity (1.0f);
		
		RadianceMap	radiancemap (LightscapeSolve::Importer()->Get_Brightness(), LightscapeSolve::Importer()->Get_Contrast(), LightscapeSolve::Importer()->Is_Daylight(), LightscapeSolve::Importer()->Is_Exterior());
		float i = GetIntensity();
		LtTRGBColor irradiance (i * GetFilterColor().GetR(), i * GetFilterColor().GetG(), i * GetFilterColor().GetB());
		
		// Set colors.
		color.Set (irradiance, radiancemap);
		
		// Clamp to white.
		color.Update_Min (white);

		light->Set_Ambient (black);
		light->Set_Diffuse (color);
		light->Set_Specular (color);

		// Set attenuation model.
		if (GetType() != LT_LAMP_DAYLIGHT) {

			const float oocutoff = 1.0f / 0.025f;	// Reciprocal of light intensity	at far attenuation.

			// NOTE: Convert Lightscape 1/d^2 attenuation to an equivalent MAX type light.
			light->Set_Flag (LightClass::NEAR_ATTENUATION, false);
			light->Set_Near_Attenuation_Range (0.0f, 0.0f);
			light->Set_Flag (LightClass::FAR_ATTENUATION, true);

			// Calculate attenuation range.
			// The far attenuation is calculated as the distance at which intensity falls to a given 
			// ratio (cut-off), assuming an attenuation of c/d^2, where c = monochrome intensity of light source.
			float d = sqrt (oocutoff * color.Length());
			light->Set_Far_Attenuation_Range (d * 0.25f, d);
		
		} else {
		
			light->Set_Flag (LightClass::NEAR_ATTENUATION, false);
			light->Set_Flag (LightClass::FAR_ATTENUATION, false);
		}

		// Miscellaneous attributes.
		light->Enable_Shadows ((GetFlags() & LT_LAMP_CAST_NO_SHADOWS) ? false : true);

		LightscapeSolve::Importer()->Add_Light (light);
		
		// Clean-up.
		light->Release_Ref();
	}

	// Return success.
	return (true);
}


/***********************************************************************************************
 * LsMeshInquirer::LsMeshInquirer --																			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   04/19/00    IML : Created.                                                                 * 
 *=============================================================================================*/
LsMeshInquirer::LsMeshInquirer (LtTBuilderFactory *factory)
	: LtTBaseMeshBuilder (factory)
{
	// Assume initially that the solve is NOT an M2T solve.
	LightscapeSolve::Importer()->Set_Is_M2T_Solve (false);
}


/***********************************************************************************************
 * LsMeshInquirer::LsMeshInquirer --																			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   04/19/00    IML : Created.                                                                 * 
 *=============================================================================================*/
LtTBool LsMeshInquirer::Finish()
{
	if (GetProperties() & LT_MESH_RAWTEXTURE) {
		LightscapeSolve::Importer()->Set_Is_M2T_Solve (true);
	}

	// Return success.
	return (true);
}


/***********************************************************************************************
 * LsMeshBuilder::LsMeshBuilder --																				  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/23/99    IML : Created.                                                                 * 
 *=============================================================================================*/
LsMeshBuilder::LsMeshBuilder (LtTBuilderFactory *factory)
	: LtTBaseMeshBuilder (factory)
{
	ASSERT (strcmp (typeid (*factory).name(), "class LsMainFactory") == 0);
	FactoryPtr		= (LsMainFactory*) factory;
	Faces				= NULL;
	FaceCount		= 0;
	AllocatedCount = 0;
}


/***********************************************************************************************
 * LsMeshBuilder::~LsMeshBuilder --																				  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/16/99    IML : Created.                                                                 * 
 *=============================================================================================*/
LsMeshBuilder::~LsMeshBuilder()
{
	if (Faces != NULL) delete [] Faces;
}


/***********************************************************************************************
 * LsMeshBuilder::SetFaces --																						  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/16/99    IML : Created.                                                                 * 
 *=============================================================================================*/
void LsMeshBuilder::SetFaces (const int facecount, const LtTFace *faces)
{
	int topcount, f, g;
   
	// Count the number of faces in the top-level hierarchy.
	topcount = 0; 
	for (f = 0; f < facecount; f++) {
		topcount++;
		f += faces [f].GetDescendents();
   }

	// If the block allocated for faces is not big enough delete it and allocate a new one.
	if (topcount > AllocatedCount) {
		if (Faces != NULL) delete [] Faces;
		Faces = new LtTFace [topcount];
		ASSERT (Faces != NULL);
		AllocatedCount = topcount;
	}
	
	// Copy the faces in the top-level hierarchy into the block allocated for faces.
	g = 0;
	for (f = 0; f < facecount; f++) {
		Faces [g] = faces [f];
		g++;
		f += faces [f].GetDescendents();
	}

   ASSERT (g == topcount);
	FaceCount = topcount;
}


/***********************************************************************************************
 * LsMeshBuilder::GetFaceCount --																				  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/16/99    IML : Created.                                                                 * 
 *=============================================================================================*/
int LsMeshBuilder::GetFaceCount() const
{
	return (FaceCount);
}


/***********************************************************************************************
 * LsMeshBuilder::GetFaces --																						  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/16/99    IML : Created.                                                                 * 
 *=============================================================================================*/
const LtTFace* LsMeshBuilder::GetFaces() const
{
	return (Faces);
}


/***********************************************************************************************
 * LsMeshBuilder::Finish --  Called by the Lightscape framework after each patch cluster read. *
 *                                                                                             *
 * Called by the Lightscape framework after each patch cluster read from a .ls file. The file  *
 * read is initiated by a LtSolutionImport() call. The clustering and the mesh structure is	  *
 * entirely under the control of Lightscape. Each patch cluster may or may not reference a	  *
 * texture and may or may not contain a radiosity mesh. If the .ls file has been produced by	  *
 * Lightscape's 'Mesh to Texture' tool (M2T) with the "Generate Illumination Map" option		  *
 * enabled then the material definition will (should) reference a lightmap texture. 			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/23/99    IML : Created.                                                                 * 
 *=============================================================================================*/
LtTBool LsMeshBuilder::Finish()
{
	const char *invalidmappingtext =
"There is a face in the model with an invalid texture mapping.	Please ensure that the\
 Mesh to Texture option called 'Convert each surface to a single texture per surface' was\
 used.";

	const char *notnormalizedtext = 
"There is a vertex in the model with a valid texture mapping but a non-normalized texture\
 coordinate.";

	const char *unmatchedmodeltext =
"The solve files do not contain the same model information. For any 2 solve files, the first\
 n patches must match, where n is the smallest patch count of the two files.";

	VertexUser usedvertices (*this);
	bool		  success;

	ASSERT (LightscapeSolve::Importer() != NULL);
	
	// Add the vertices to the Lightscape solve database.
	// If this mesh is an M2T patch...
	if (GetProperties() & LT_MESH_RAWTEXTURE) { 

		int			  vertexindex;
		LtTVector	  lsv;					// Lightscape scratch vertex.
		Vector3		  v;						// Scratch vertex. 
		Vector3		  origin, du, dv;		// Definition of texture projection.
		Vector2		  length;				//
		bool			  normalized;	
		LtTUnitVector lsn;
		Vector3		  n;

		// Lightscape definition of texture projection.
		LtTTextureProjection lsprojection = GetTextureProjection();
		
		// If the texture mapping is invalid (ie. not orthographic)...
		if ((lsprojection.GetFlags() & LT_PROJECTION_TYPE_MASK) != LT_PROJECTION_ORTHOGRAPHIC) {
			throw (invalidmappingtext);
		}
			
		// Define origin and texture vectors for a planar mapping.
		lsv = lsprojection.GetOrigin();
		origin.Set (lsv.GetX(), lsv.GetY(), lsv.GetZ());
		lsv = lsprojection.GetDu();
		du.Set (lsv.GetX(), lsv.GetY(), lsv.GetZ());
		lsv = lsprojection.GetDv();
		dv.Set (lsv.GetX(), lsv.GetY(), lsv.GetZ());

		// Normalize the texture vectors.
		length.U = du.Length();
		length.V = dv.Length();
		du /= length.U;
		dv /= length.V;

		// Step thru the vertices, calculate UV from the texture projection and add the information
		// to the Lightscape solve database.
		GetPlane (lsn);
		n.Set (lsn.GetX(), lsn.GetY(), lsn.GetZ());
		for (vertexindex = 0; vertexindex < GetVertexCount(); vertexindex++) {

			// Add only if the vertex is referenced by a face.
			if (usedvertices.Is_Used (vertexindex)) {

				LtTPoint	lsp = GetVertices() [vertexindex];
				
				Vector3 p (lsp.GetX(), lsp.GetY(), lsp.GetZ());
				Vector2 t;

				v = p - origin;
				t.U = (du * v) / length.U;
				t.V = (dv * v) / length.V;

				// UV's must be normalized (in the range 0..1) in order to reference the lightmap properly.
				normalized  = ((t.U >= 0.0f) && (t.U <= 1.0f));
				normalized &= ((t.V >= 0.0f) && (t.V <= 1.0f));
				if (!normalized) throw (notnormalizedtext);
					
				// Assign an index for this vertex which will uniquely identify it in the vertex database.
				usedvertices.Set_Index (vertexindex, FactoryPtr->Vertex_Count());

				// Add the vertex to the database.
				success = LightscapeSolve::Importer()->Add_Vertex (FactoryPtr->Vertex_Count(), p, n, FactoryPtr->Patch_Count(), t);
				if (!success) throw (unmatchedmodeltext);

				// Advance vertex index counter.
				FactoryPtr->Vertex_Increment();
			}
		}

		// Find the texture name that corresponds to the material name and add it for this patch.
		LightscapeSolve::Importer()->Add_Texture_Name (FactoryPtr->Patch_Count(), GetMaterial());

	} else {

		const double									filtersharpness = LightscapeSolve::Importer()->Filter_Sharpness();

		ProceduralTexture							  *blendtexture;
		DynamicVectorClass <FaceVertexStruct>  facevertices;
		double										  *distanceratios;			
		int											   vertexindex;
		int												facevertexindex;
		RadianceMap									   radiancemap (LightscapeSolve::Importer()->Get_Brightness(), LightscapeSolve::Importer()->Get_Contrast(), LightscapeSolve::Importer()->Is_Daylight(), LightscapeSolve::Importer()->Is_Exterior());
		LtTPoint										   lsp;
		LtTRGBColor									   irradiance;
		LtTUnitVector								   lsn;
		Vector3										   n;	

		blendtexture = LightscapeSolve::Importer()->Procedural_Texture();

		// NOTE: This mesh is not an M2T patch so it must instead contain valid vertex color information.
		// Build an array of face vertices initialized to their respective vertex colors.
		for (vertexindex = 0; vertexindex < GetVertexCount(); vertexindex++) {
			if (usedvertices.Is_Used (vertexindex)) {

				FaceVertexStruct facevertex;
				
				lsp = GetVertices() [vertexindex];
				irradiance = GetIrradiances() [vertexindex];
				facevertex.Point.Set (lsp.GetX(), lsp.GetY(), lsp.GetZ());
				facevertex.Color.Set (irradiance, radiancemap);
			
			 	// Should the vertex color be blended with a sample from a procedural texture?
				if (blendtexture != NULL) {
					facevertex.Color *= blendtexture->Value (facevertex.Point);
				}	
				
				facevertex.Weight = 1.0f;
				facevertices.Add (facevertex);
			}
		}

		// There must be at least one face vertex.
		ASSERT (facevertices.Count() > 0);

		// For every non-face vertex calculate its color contribution to each face vertex.
		distanceratios = new double [facevertices.Count()];
		ASSERT (distanceratios != NULL);

		for (vertexindex = 0; vertexindex < GetVertexCount(); vertexindex++) {
			if (!usedvertices.Is_Used (vertexindex)) {

				Vector3		point;
				ColorVector color;
				double		d, s;

				lsp = GetVertices() [vertexindex];
				irradiance = GetIrradiances() [vertexindex];
				point.Set (lsp.GetX(), lsp.GetY(), lsp.GetZ());
				color.Set (irradiance, radiancemap);

				// Should the vertex color be blended with a sample from a procedural texture?
				if (blendtexture != NULL) {
					color *= blendtexture->Value (point);
				}	

				d = (point - facevertices [0].Point).Length();
				distanceratios [0] = s = 1.0f;
				for (facevertexindex = 1; facevertexindex < facevertices.Count(); facevertexindex++) {
					distanceratios [facevertexindex] = d / (point - facevertices [facevertexindex].Point).Length();
				
					// Raise the distance ratio to the user specified filter sharpness.
					distanceratios [facevertexindex] = pow (distanceratios [facevertexindex], filtersharpness);
					
					s += distanceratios [facevertexindex];
				}

				for (facevertexindex = 0; facevertexindex < facevertices.Count(); facevertexindex++) {

					float weight;

					weight = distanceratios [facevertexindex] / s;
					facevertices [facevertexindex].Color  += color * weight;
					facevertices [facevertexindex].Weight += weight;
				}
			}
		}

		// Add the face vertices to the solve database.
		GetPlane (lsn);
		n.Set (lsn.GetX(), lsn.GetY(), lsn.GetZ());
		facevertexindex = 0;
		for (vertexindex = 0; vertexindex < GetVertexCount(); vertexindex++) {
			if (usedvertices.Is_Used (vertexindex)) {

				ColorVector c = facevertices [facevertexindex].Color;
				c /= facevertices [facevertexindex].Weight;
			
				// Assign an index for this vertex which will uniquely identify it in the vertex database.
				usedvertices.Set_Index (vertexindex, FactoryPtr->Vertex_Count());
				
				// Add the vertex to the database.
				success = LightscapeSolve::Importer()->Add_Vertex (FactoryPtr->Vertex_Count(), facevertices [facevertexindex].Point, n, FactoryPtr->Patch_Count(), c);
				if (!success) {
					delete [] distanceratios;
					throw (unmatchedmodeltext);
				}

				// Advance the vertex index counters.
				FactoryPtr->Vertex_Increment();
				facevertexindex++;
			}
		}
		
		// Clean-up.
		delete [] distanceratios;
	}

	// Add the faces to the Lightscape solve database.
	for (int faceindex = 0; faceindex < GetFaceCount(); faceindex++) {

		const unsigned facevertexindexcount = 4;

		LtTFace  face;
		int		vertexindex, facevertexcount;
		unsigned	facevertexindices [facevertexindexcount];

		ASSERT (faceindex >= 0 && faceindex < GetFaceCount());
		face = GetFaces() [faceindex];

		// If the face is not a triangle then it must be a quadrilateral. It is safe to assume
		// that these are the only polygon types that Lightscape supports.
		facevertexcount = face.IsTriangle() ? 3 : 4;

		for (int	v = 0; v < facevertexcount; v++) {
			vertexindex = face.GetVert (v);
			ASSERT (vertexindex >= 0 && vertexindex < GetVertexCount());
			facevertexindices [v] = usedvertices.Get_Index (vertexindex);
		}	
		
		success = LightscapeSolve::Importer()->Add_Patch_Face (FactoryPtr->Patch_Face_Count(), FactoryPtr->Patch_Count(), facevertexcount, facevertexindices);
		if (!success) throw (unmatchedmodeltext);

		// Advance patch face counter.
		FactoryPtr->Patch_Face_Increment();
	}

	// Advance the patch index counter.
	FactoryPtr->Patch_Increment();

	// Return success.
	return (true);
}


/***********************************************************************************************
 * VertexUser::VertexUser --																						  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/05/00    IML : Created.                                                                 * 
 *=============================================================================================*/
VertexUser::VertexUser (const LsMeshBuilder &meshbuilder)
{
	LtTFace  face;
	unsigned v, vertexindex, facevertexcount;

	Count = meshbuilder.GetVertexCount();
	UsedVertexIndices = new int [Count];
	ASSERT (UsedVertexIndices != NULL);

	// Initialize.	All vertices unused.
	for (v = 0; v < Count; v++) {
		UsedVertexIndices [v] = UNUSED;
	}

	// For each face...
	for (int faceindex = 0; faceindex < meshbuilder.GetFaceCount(); faceindex++) {

		face = meshbuilder.GetFaces() [faceindex];

		// If the face is not a triangle then it must be a quadrilateral. It is safe to assume
		// that these are the only polygon types that Lightscape supports.
		facevertexcount = face.IsTriangle() ? 3 : 4;

		// For each vertex in the face, flag that the vertex is referenced by a face.
		for (v = 0; v < facevertexcount; v++) {
			vertexindex = face.GetVert (v);
			ASSERT (vertexindex >= 0 && vertexindex < Count);
			UsedVertexIndices [vertexindex] = USED;
		}
	}
}


/***********************************************************************************************
 * LightscapeMeshSolve::LightscapeMeshSolve --																  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/30/99    IML : Created.                                                                 * 
 *=============================================================================================*/
LightscapeMeshSolve::LightscapeMeshSolve (LightscapeSolve &solve, ChunkClass &trianglechunk, ChunkClass &vertexchunk)
	: Statistics (vertexchunk.Get_Size() / sizeof (W3dVectorStruct), trianglechunk.Get_Size() / sizeof (W3dTriStruct))
{
	const unsigned verticesperface = 3;	// No. of vertices in a triangle.

	PackingTriangle *triangles;
	W3dTriStruct	 *w3dfaces;
	W3dVectorStruct *w3dvertices;
	unsigned			  f, v;

	// Calculate no. of faces and face-vertices from triangle chunk.
	VertexCount		 =	vertexchunk.Get_Size() / sizeof (W3dVectorStruct);
	FaceCount		 = trianglechunk.Get_Size() / sizeof (W3dTriStruct);
	FaceVertexCount = FaceCount * verticesperface;

	// Allocate data tables for this mesh.
	VertexColors = new W3dRGBStruct [VertexCount];
	ASSERT (VertexColors != NULL);
	FaceVertexUVs = new Vector2 [FaceVertexCount];
	ASSERT (FaceVertexUVs != NULL);
	FaceRemapLightmapIndices = new unsigned [FaceCount];
	ASSERT (FaceRemapLightmapIndices != NULL);
	LightmapIndices = new unsigned [FaceCount];
	ASSERT (LightmapIndices != NULL);

	triangles = new PackingTriangle [FaceCount];
	ASSERT (triangles != NULL);

	w3dfaces	= (W3dTriStruct*) trianglechunk.Get_Data();
	ASSERT (w3dfaces != NULL);
	w3dvertices = (W3dVectorStruct*) vertexchunk.Get_Data();
	ASSERT (w3dvertices != NULL);

	// For each vertex in the vertex chunk...
	for (unsigned vertexindex = 0; vertexindex < VertexCount; vertexindex++) {

		const Vector3	  zero (0.0f, 0.0f, 0.0f);

		Vector3			 *smoothingnormalptr;
		Vector3			  point;
		W3dVectorStruct *w3dvertexptr;
		W3dRGBStruct	  vertexcolor;

		// Iterate over the faces and calculate a smoothing normal for this vertex based on
		// the faces that reference this vertex.
		smoothingnormalptr = NULL;
		for (f = 0; f < FaceCount; f++) {
			for (v = 0; v < verticesperface; v++) {
				if (w3dfaces [f].Vindex [v] == vertexindex) {
					
					Vector3 facenormal;

					// Do not consider face normals that are zero (face is degenerate).  
					facenormal.Set (w3dfaces [f].Normal.X, w3dfaces	[f].Normal.Y, w3dfaces [f].Normal.Z);
					if (facenormal != zero) smoothingnormalptr = &facenormal;
					break;
				}
			}
			if (smoothingnormalptr != NULL) break;
		}
		
		w3dvertexptr = w3dvertices + vertexindex;
		point.Set (w3dvertexptr->X, w3dvertexptr->Y, w3dvertexptr->Z);
		
		solve.Find_Vertex (point, smoothingnormalptr, vertexcolor, Statistics);
		VertexColors [vertexindex] = vertexcolor;
	}

	// For each face in the w3d triangle chunk, match it up with a face in the Lightscape solve database.
	for (f = 0; f < FaceCount; f++) {

		Vector3			 points [verticesperface];
		Vector3			 facenormal;
		Vector2			 uvs [verticesperface];
		PackingTriangle triangle;

		// Extract points and face normal from vertex chunk.
		for (v = 0; v < verticesperface; v++) {

			W3dVectorStruct *w3dvertexptr;
			unsigned			  vindex;	
			
			vindex = w3dfaces [f].Vindex [v];
			w3dvertexptr = w3dvertices + vindex;
			points [v].Set (w3dvertexptr->X, w3dvertexptr->Y, w3dvertexptr->Z);
		}
		facenormal.Set (w3dfaces [f].Normal.X, w3dfaces [f].Normal.Y, w3dfaces [f].Normal.Z);
		solve.Find_Triangle (points, facenormal, triangles [f], Statistics);
	}

	if (Statistics.Valid_Lightmap_Solve()) {
	
		unsigned	lightmapindex, remaplightmapindex;	
		bool		found;	

		// Submit the triangles for packing.
		for (f = 0; f < FaceCount; f++) {
			solve.Submit_Triangle (triangles [f]);
		}

		// Now pack.
		solve.Pack();

		// Extract the lightmap UV's from the triangles.
		v = 0;
		for (f = 0; f < FaceCount; f++) {
			FaceVertexUVs [v + 0] = triangles [f].PackedUVs [0];
			FaceVertexUVs [v + 1] = triangles [f].PackedUVs [1];
			FaceVertexUVs [v + 2] = triangles [f].PackedUVs [2];
			v += verticesperface;
		}

		// The face-lightmap indices are currently non-zero based, unordered, non-contiguous indices
		//	ie. an unknown list of indexes that have been returned from the packing process.
		// The mesh requires face-lightmap indices that are zero-based, ordered, contiguous indices
		// so remap the indices to create face-remaplightmap indices. Also, create a table of 'real'
		// lightmap indices that will be indexed by the face-remaplightmap table.
		remaplightmapindex = 0;
		for (f = 0; f < FaceCount; f++) {

			int g;

			lightmapindex = triangles [f].PackedTextureID;

			// Look backwards thru the face-lightmap table to see if this index has already occured?
			found = false;
			g		= ((int) f) - 1;
			while (g >= 0) {
				if (triangles [g].PackedTextureID == lightmapindex) {
					found = true;
					break;
				}
				g--;
			}

			// Was the index found?
			if (found) {
				FaceRemapLightmapIndices [f] = FaceRemapLightmapIndices [g];
			} else {
				FaceRemapLightmapIndices [f] = remaplightmapindex;
				ASSERT (remaplightmapindex < FaceCount);
				LightmapIndices [remaplightmapindex] = lightmapindex;
				remaplightmapindex++;
			}
		}
		
		ASSERT (remaplightmapindex > 0);
		LightmapCount = remaplightmapindex;

	} else {
		LightmapCount = 0;
	}

	// Clean-up.
	delete [] triangles;
}


/***********************************************************************************************
 * LightscapeMeshSolve::~LightscapeMeshSolve --																  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/30/99    IML : Created.                                                                 * 
 *=============================================================================================*/
LightscapeMeshSolve::~LightscapeMeshSolve()
{
	// Clean-up.
	delete [] VertexColors;
	delete [] LightmapIndices;
	delete [] FaceRemapLightmapIndices;
	delete [] FaceVertexUVs;
}


/***********************************************************************************************
 * LightscapeMeshSolve::Vertex_Color --																		  *	
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/06/00    IML : Created.                                                                 * 
 *=============================================================================================*/
W3dRGBStruct LightscapeMeshSolve::Vertex_Color (unsigned vertexindex) const
{
	ASSERT (Statistics.Valid_Vertex_Solve());
	ASSERT (vertexindex < VertexCount);
	return (VertexColors [vertexindex]);
}


/***********************************************************************************************
 * LightscapeMeshSolve::LightscapeMeshSolve --																  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/30/99    IML : Created.                                                                 * 
 *=============================================================================================*/
const char *LightscapeMeshSolve::Lightmap_Pathname (unsigned lightmapindex) const
{
	ASSERT (Statistics.Valid_Lightmap_Solve());
	ASSERT (lightmapindex < FaceCount);
	return (LightscapeSolve::Lightmap_Pathname (LightmapIndices [lightmapindex]));
}


/***********************************************************************************************
 * LightscapeMeshSolve::LightscapeMeshSolve --																  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/30/99    IML : Created.                                                                 * 
 *=============================================================================================*/
unsigned	LightscapeMeshSolve::Lightmap_Index (unsigned faceindex) const
{
	ASSERT (Statistics.Valid_Lightmap_Solve());
	ASSERT (faceindex < FaceCount);
	return (FaceRemapLightmapIndices [faceindex]);
}


/***********************************************************************************************
 * LightscapeMeshSolve::LightscapeMeshSolve --																  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/30/99    IML : Created.                                                                 * 
 *=============================================================================================*/
W3dTexCoordStruct LightscapeMeshSolve::Lightmap_UV (unsigned facevertexindex) const
{
	W3dTexCoordStruct w3duv;
	
	ASSERT (Statistics.Valid_Lightmap_Solve());
	ASSERT (facevertexindex < FaceVertexCount);
	
	w3duv.U = FaceVertexUVs [facevertexindex].U;
	w3duv.V = FaceVertexUVs [facevertexindex].V;
	return (w3duv);
}
