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
 *                     $Modtime:: 7/09/01 4:30p       $* 
 *                                                                                             * 
 *                    $Revision:: 50                                                        $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

// Includes.
#include "StdAfx.h"
#include "LightMap.h"
#include "LightmapPacker.h"
#include "OptionsDialog.h"
#include "StringBuilder.h"
#include "Targa.h"
#include "TextureNameNode.h"
#include "srFilter.hpp"
#include <direct.h>
#include <io.h>
#include <stdlib.h>
#include <winbase.h>


// Defines.
#define TEMPORARY_DIRECTORY_NAME		 "~L"						// Name of temporary directory used to store packed lightmaps.

#define PAGE_WIDTH										  256		// Width & height of page in texels.
#define PAGE_HEIGHT										  256		// NOTE: Elected to use the biggest texture size that is
																			// supported by the gamut of PC 3D graphics hardware in order
																			// to maximize no. of lightmaps packed per page and therefore
																			// minimize no. of texture swaps per rendered polygon.

#define PACKING_BYTES						 (1024 * 1024)		// No. of page bytes to consider for packing	the current lightmap.

#define ASSET_EXTENSION					 ".tga"					// Extension (and file type) of lightmap assets.


// Static data.
unsigned LightmapPacker::_BasePageIndex = 0;

char LightmapPacker::_Statistics [LightmapPacker::STATISTICS_COUNT][LightmapPacker::STATISTICS_STRING_SIZE] = {
	"No data available",
	"No data available",
	"No data available",
	"No data available",
	"No data available",
	"No data available",
	"No data available",
	"No data available",
	"No data available",
	"No data available"
};


/***********************************************************************************************
 * LightmapPacker::LightmapPacker --																			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/18/99    IML : Created.                                                                 * 
 *=============================================================================================*/
LightmapPacker::LightmapPacker()
{
	char			  pathname [_MAX_PATH];
	OptionsDialog options;

	// Initialize.
	FillColor		  = options.Get_Fill_Color();
	FilterError		  = pow (options.Get_Filter_Error() * 4.919e-3l, 2.5l);	// Map s.t. 50th percentile -> 0.03% RGB error.
	SampleRate		  = options.Get_Sample_Rate();
	PageBitDepth	  = options.Get_Bit_Depth();
	CurrentPageIndex = -1;
	Reset_Statistics();

	// Recreate the asset directory.
	strcpy (pathname, theApp.Working_Path());
	strcat (pathname, Asset_Directory());

	// Can the asset directory can be successfully created?
	if (_mkdir (pathname) == 0) {
		_BasePageIndex = 0;
	}

	// Create the placeholder texture name node.
	strcpy (pathname, theApp.Working_Path());
	strcat (pathname, "Placeholder.tga");
	PlaceholderTextureNameNodePtr = new TextureNameNode (pathname);
	ASSERT (PlaceholderTextureNameNodePtr != NULL);
}


/***********************************************************************************************
 * LightmapPacker::Finish --																						  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/18/99    IML : Created.                                                                 * 
 *=============================================================================================*/
LightmapPacker::Finish()
{
	// For each page...
	for (int pageindex = 0; pageindex < PagePtrs.Count(); pageindex++) {

		char  pagepathname [_MAX_PATH];
		char *errormessage;

		// Pad fill color.
		PagePtrs [pageindex]->Fill (FillColor);

		// Update statistics about the page.
		Update_Statistics (*PagePtrs [pageindex]);

		// Save the page. Report any error.
		strcpy (pagepathname, theApp.Working_Path());
		strcat (pagepathname, Lightmap_Pathname (pageindex));
		errormessage = PagePtrs [pageindex]->Save (pagepathname);
		if (errormessage != NULL) throw (errormessage);
	}
	
	_BasePageIndex += PagePtrs.Count();
	Collate_Statistics();
}


/***********************************************************************************************
 * LightmapPacker::~LightmapPacker --																			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS: Do not throw any exceptions in this (and any other) destructor because this		  *
 *				 destructor	can itself be called during the exception as part of its 'clean-up'.	  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/18/99    IML : Created.                                                                 * 
 *=============================================================================================*/
LightmapPacker::~LightmapPacker()
{
	// For each page...
	for (int pageindex = 0; pageindex < PagePtrs.Count(); pageindex++) {
		delete PagePtrs [pageindex];
	}

	delete PlaceholderTextureNameNodePtr;
}


/***********************************************************************************************
 * LightmapPacker::Submit -- Submit a triangular region of a lightmap for packing.				  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/28/00    IML : Created.                                                                * 
 *=============================================================================================*/
void LightmapPacker::Submit (PackingTriangle *principaltriangleptr, const DynamicVectorClass <Triangle> &adjtriangles)
{
	const unsigned normalgroupid		 = 0;
	const unsigned placeholdergroupid = 1;

	TrianglePacker *trianglepackerptr, *mergedpackerptr;

	ASSERT (principaltriangleptr != NULL);

	// Is there an associated texture?
	if (principaltriangleptr->TextureNameNodePtr == NULL) {

		static const float _placeholderuvs [Triangle::VERTICES_COUNT][2] =
		{
			{0.0f, 0.0f},
			{1.0f, 0.0f},
			{0.0f, 1.0f},
		};

		DynamicVectorClass <Triangle> noadjtriangles;

		// Substitute a placeholder texture.
		principaltriangleptr->TextureNameNodePtr = PlaceholderTextureNameNodePtr;
		for (unsigned v = 0; v < Triangle::VERTICES_COUNT; v++) {
			principaltriangleptr->Vertices [v].UV.X = _placeholderuvs [v][0];
			principaltriangleptr->Vertices [v].UV.Y = _placeholderuvs [v][1];
		}

		// Create a triangle packer from the modified principal triangle with no adjacent triangles.
		// NOTE: Use a 'placeholder group' ID to indicate to the triangle packer that it should not be merged with non-placeholder triangles.
		trianglepackerptr = new TrianglePacker (principaltriangleptr, noadjtriangles, placeholdergroupid, EDGE_BLEND_THICKNESS, SampleRate, FillColor);
		ASSERT (trianglepackerptr != NULL);
	
	} else {

		// Create a triangle packer from the principal and adjacent triangles.
		trianglepackerptr = new TrianglePacker (principaltriangleptr, adjtriangles, normalgroupid, EDGE_BLEND_THICKNESS, SampleRate, FillColor);
		ASSERT (trianglepackerptr != NULL);
	}

	while (trianglepackerptr != NULL) {
		mergedpackerptr	= trianglepackerptr;
		trianglepackerptr = Merge (mergedpackerptr);
	}

	TrianglePackerPtrs.Add (mergedpackerptr);

	// Statistics: each submission contains one triangular principal face.
	FaceCount++;
}


/***********************************************************************************************
 * LightmapPacker::Merge --																						  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/28/00    IML : Created.                                                                * 
 *=============================================================================================*/
TrianglePacker *LightmapPacker::Merge (TrianglePacker *trianglepackerptr)
{
	TrianglePacker *resultantpackerptr;

	// See if the triangle packer can be merged with an existing triangle packer.
	resultantpackerptr = NULL;
	for (int t = 0; t < TrianglePackerPtrs.Count(); t++) {

		TrianglePacker *mergedpackerptr;
		
		mergedpackerptr = TrianglePackerPtrs [t]->Merge (*trianglepackerptr);
		if (mergedpackerptr != NULL) {
			
			// Does the merged packer fit within the maximum page size?
			if ((mergedpackerptr->Width() <= PAGE_WIDTH) && (mergedpackerptr->Height() <= PAGE_HEIGHT)) {
	
				unsigned mergedarea, unmergedarea;
				
				// Is the merged packer smaller in area than the sum of the two unmerged packers?
				mergedarea	 = mergedpackerptr->Width() * mergedpackerptr->Height();
				unmergedarea = (trianglepackerptr->Width() * trianglepackerptr->Height()) + (TrianglePackerPtrs [t]->Width() * TrianglePackerPtrs [t]->Height());
				if (mergedarea <= unmergedarea) {

					bool success;
	
					// Replace the two unmerged packers with the merged packer.
					delete trianglepackerptr;
					delete TrianglePackerPtrs [t];
					success = TrianglePackerPtrs.Delete (t);
					ASSERT (success);
					resultantpackerptr = mergedpackerptr;
					break;
				}
			}
		}
	}
	
	return (resultantpackerptr);
}


/***********************************************************************************************
 * LightmapPacker::Pack --	Pack all the triangular regions submitted thus far.					  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/18/00    IML : Created.                                                                * 
 *=============================================================================================*/
void LightmapPacker::Pack (ProceduralTexture *proceduraltexture)
{
	TargaLoader	targaloader (SampleRate, FillColor);
	
	// For each triangle packer...
	for (int t = 0; t < TrianglePackerPtrs.Count(); t++) {

		const unsigned rescalethreshold = 16; // When snapping to even dimension, 'round down' if above this size -
														  // otherwise 'round up'.			

		TrueColorTarga	  rasterizedtarga;
		unsigned			  unscaledwidth, unscaledheight;								 
		unsigned			  clampedwidth, clampedheight;	
		unsigned			  padwidth, padheight;

		// Rasterize the triangle packer.
		TrianglePackerPtrs [t]->Rasterize (targaloader, proceduraltexture, rasterizedtarga);

		// Scale targa according to user-specified filter error. 
		unscaledwidth  = rasterizedtarga.Width();
		unscaledheight = rasterizedtarga.Height();
		rasterizedtarga.Scale (FilterError);

		// Scale targa to maximum page size (taking into account edge thickness).
		ASSERT (PAGE_WIDTH  > (EDGE_BLEND_THICKNESS * 2));
		ASSERT (PAGE_HEIGHT > (EDGE_BLEND_THICKNESS * 2));
		clampedwidth  = MIN (PAGE_WIDTH  - (EDGE_BLEND_THICKNESS * 2), rasterizedtarga.Width());
		clampedheight = MIN (PAGE_HEIGHT - (EDGE_BLEND_THICKNESS * 2), rasterizedtarga.Height());
		if ((clampedwidth < rasterizedtarga.Width()) || (clampedheight < rasterizedtarga.Height())) {
			
			// Statistics: update no. of oversize targas.
			OversizeCount++;
		}

		// Snap to even dimensions.
		// NOTE: This will reduce texel bleeding as a side effect of mip-mapping or rendering at reduced resolutions.
		ASSERT (rescalethreshold < PAGE_WIDTH);
		ASSERT (rescalethreshold < PAGE_HEIGHT);
		if (clampedwidth > rescalethreshold) {
			clampedwidth -= (clampedwidth & 0x1);
		} else {
			clampedwidth += (clampedwidth & 0x1);
		}
		if (clampedheight > rescalethreshold) {
			clampedheight -= (clampedheight & 0x1);
		} else {
			clampedheight += (clampedheight & 0x1);
		}
		if (clampedwidth != rasterizedtarga.Width() || clampedheight != rasterizedtarga.Height()) {
			rasterizedtarga.Scale (clampedwidth, clampedheight);
		}

		// Add pad texels to compensate for edge texels that may have been lost due to filtered scaling and scaling to clamp to maximum page size.
		// NOTE: Pad to a multiple of 2 to maintain 'even' dimensions.
		padwidth  = 2 * ceilf (EDGE_BLEND_THICKNESS * (1.0f - (((float) rasterizedtarga.Width())  / ((float) unscaledwidth))));
		padheight =	2 * ceilf (EDGE_BLEND_THICKNESS * (1.0f - (((float) rasterizedtarga.Height()) / ((float) unscaledheight))));
		rasterizedtarga.Pad (padwidth, padheight, FillColor, TrianglePackerPtrs [t]->Principal_Triangles());

		// Pack the targa into a lightmap page.
		Pack (rasterizedtarga, TrianglePackerPtrs [t]->Principal_Triangles());
		
		// Statistics: add targa's area before and after scaling.
		// NOTE: Although this is not desirable, it is possible for scaling to increase the targa's area.
		UnscaledTexelCount += unscaledwidth * unscaledheight;
		ScaledTexelCount	 += rasterizedtarga.Width() * rasterizedtarga.Height();

		// Update statistics about the triangle packer.
		Update_Statistics (*TrianglePackerPtrs [t]);

		// Clean-up.
		delete TrianglePackerPtrs [t];
	}

	// Reset.
	TrianglePackerPtrs.Clear();
}


/***********************************************************************************************
 * TargaLoader::TargaLoader --																					  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/30/00    IML : Created.                                                                * 
 *=============================================================================================*/
TargaLoader::TargaLoader (float samplerate, W3dRGBStruct fillcolor)
{
	const unsigned cachesize		 = 1024;	// Maximum no. of entries in cache.
	const	unsigned cleanuplistsize = 256;	// Initial size of clean-up list.

	SampleRate = samplerate;
	FillColor  = fillcolor;

	// Size the cache and the clean-up list.
	Cache.Resize (cachesize);
	CleanupList.Set_Growth_Step (cleanuplistsize);
}


/***********************************************************************************************
 * TargaLoader::~TargaLoader --																					  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/30/00    IML : Created.                                                                * 
 *=============================================================================================*/
TargaLoader::~TargaLoader()
{
	// Delete the targas in the clean-up list.
	for (int c = 0; c < CleanupList.Count(); c++) {
		ASSERT (CleanupList [c] != NULL);
		delete CleanupList [c];
	}	
}


/***********************************************************************************************
 * TargaLoader::Load	--																								  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/08/00    IML : Created.                                                                * 
 *=============================================================================================*/
TrueColorTarga *TargaLoader::Load (const Triangle &triangle)
{
	TrueColorTarga *targaptr;
	unsigned			 index; 		

	index = Index (triangle.TextureID); 
	
	// Is there a cache miss?
	if ((Cache [index].Ptr == NULL) || (Cache [index].ID != triangle.TextureID)) {

		char				 *errormessage;
		TextureNameNode *pathnamenodeptr;
		Vector2			  t;
		Vector3			  p;
		float				  length, localsamplerate, scalefactor;

		// There must be a texture pathname.
		pathnamenodeptr = triangle.TextureNameNodePtr;
		ASSERT (pathnamenodeptr != NULL);

		targaptr = new TrueColorTarga;
		ASSERT (targaptr != NULL);

		// Read the first targa asset file. Report any error.
		errormessage = targaptr->Load (pathnamenodeptr->TextureName);
		if (errormessage != NULL) throw (errormessage);

		// Replace fill texel with padding. If the targa is all fill color substitute the replacement targa.
		targaptr->Fill (FillColor);

		// Read all remaining targa asset files and add them to the first.
		pathnamenodeptr = pathnamenodeptr->Next;
		while (pathnamenodeptr != NULL) {
				
			TrueColorTarga additivetarga;

			errormessage = additivetarga.Load (pathnamenodeptr->TextureName);
			if (errormessage != NULL) throw (errormessage);
			
			// Replace fill texel with padding.
			additivetarga.Fill (FillColor);

			targaptr->Add (additivetarga);

			// Advance to next targa asset.
			pathnamenodeptr = pathnamenodeptr->Next;
		}

		// Calculate the sample rate of the targa (no. of texels per unit length).
		// NOTE: A typical length unit is metres (it does not matter which it is).
		t = triangle.Vertices [1].UV - triangle.Vertices [0].UV;
		t.Scale (targaptr->Width(), targaptr->Height());
		p = triangle.Vertices [1].Point - triangle.Vertices [0].Point;
		length = p.Length();

		// Degenerate triangles are not allowed!
		ASSERT (length > 0.0f);

		localsamplerate = t.Length() / length;

		// In the event that the targa is sampled, maximize sampling accuracy by scaling
		//	to match the global sample rate.
		// NOTE: There is no advantage to scaling-up.
		scalefactor = SampleRate / localsamplerate;
		if (scalefactor < 1.0f) {
			targaptr->Scale (targaptr->Width() * scalefactor, targaptr->Height() * scalefactor);
		}

		// Add the existing targa to the clean-up list for removal later (in the destructor).
		// NOTE: It cannot be deleted yet because another object may be referencing it.
		if (Cache [index].Ptr != NULL) {
			CleanupList.Add (Cache [index].Ptr);
		}

		// Update the cache entry.
		Cache [index].ID  = triangle.TextureID;
		Cache [index].Ptr = targaptr;

	} else {
		
		// Cache hit.
		targaptr = Cache [index].Ptr;
	}

	return (targaptr);
}


/***********************************************************************************************
 * LightmapPacker::Pack --	Pack a targa into a lightmap page.											  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/19/00    IML : Created.                                                                * 
 *=============================================================================================*/
void LightmapPacker::Pack (TrueColorTarga &targa, DynamicVectorClass <PackingTriangle*> &triangleptrs)
{
	const float packingerror = FilterError * 0.5f;

	bool packed;

	// Attempt to pack the lightmap in the current page.
	if (CurrentPageIndex != -1) {
		packed = PagePtrs [CurrentPageIndex]->Pack (targa, packingerror, triangleptrs);
	} else {
		packed = false;
	}

	if (!packed) {

		const unsigned packingwindowsize = PACKING_BYTES / ((PAGE_WIDTH * PAGE_HEIGHT * PageBitDepth) / CHAR_BIT);
		
		int firstpage, lastpage;

		// Iterate over all existing pages (except the current page) that are in the packing window.
		firstpage = PagePtrs.Count() - 1;
		lastpage  = MAX (firstpage - ((int) packingwindowsize) + 1, 0);
		for (int pageindex = firstpage; pageindex >= lastpage; pageindex--) {
			if (pageindex != CurrentPageIndex) {
				packed = PagePtrs [pageindex]->Pack (targa, packingerror, triangleptrs);
				if (packed) {
					CurrentPageIndex = pageindex;
					break;
				}
			}
		}

		// If still not packed then create a new page.
		if (!packed) {
				
			Page *pageptr;

			pageptr = new Page (PageBitDepth, FillColor);
			ASSERT (pageptr != NULL);
			PagePtrs.Add (pageptr);
			packed = pageptr->Pack (targa, packingerror, triangleptrs);

			// The first lightmap packed into a page must always succeed.
			ASSERT (packed);

			CurrentPageIndex = PagePtrs.Count() - 1;
		}

		// The current page index has changed so this will incur a texture swap.
		TextureSwapCount++;
	}

	LightmapCount++;

	// Store the page index in the triangles.
	for (int t = 0; t < triangleptrs.Count(); t++) {
		triangleptrs [t]->PackedTextureID = CurrentPageIndex;
	}
}


/***********************************************************************************************
 * LightmapPacker::Lightmap_Pathname --																		  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/18/99    IML : Created.                                                                 * 
 *=============================================================================================*/
const char *LightmapPacker::Lightmap_Pathname (unsigned pageindex)
{
	static char _pathname [_MAX_PATH];

	char filename [_MAX_FNAME];

	// Build pathname of lightmap based on lightmap's index.
	strcpy (_pathname, Asset_Directory());
	_ultoa (_BasePageIndex + pageindex, filename, 10);
	strcat (filename, ASSET_EXTENSION);
	strcat (_pathname, filename);
	return (_pathname);
}


/***********************************************************************************************
 * LightmapPacker::Asset_Directory --																			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/15/00    IML : Created.                                                                * 
 *=============================================================================================*/
const char *LightmapPacker::Asset_Directory()
{
	static bool _called = false;
	static char _assetdirectory [_MAX_PATH]; 

	// Optimization: only required to build asset directory once because it is invariant over
	// lifetime of application.
	if (!_called) {
		strcpy (_assetdirectory, TEMPORARY_DIRECTORY_NAME);
		strcat (_assetdirectory, theApp.Instance_Name()); 
		strcat (_assetdirectory, "\\");
		_called = true;
	}
	return (_assetdirectory);
}


/***********************************************************************************************
 * LightmapPacker::Asset_Directory --																			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/18/99    IML : Created.                                                                 * 
 *=============================================================================================*/
const char *LightmapPacker::Asset_Directory (const char *filename)
{
	static char _pathname [_MAX_PATH];

	strcpy (_pathname, filename);
	strcat (_pathname, "+\\");
	return (_pathname);
}


/***********************************************************************************************
 * LightmapPacker::Delete_Assets --																				  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/18/99    IML : Created.                                                                 * 
 *=============================================================================================*/
void LightmapPacker::Delete_Assets()
{
	char			pathname [_MAX_PATH];
	long			handle;
	_finddata_t	fileinfo;

	// Attempt to delete every file in the asset directory.
	strcpy (pathname, theApp.Working_Path());
	strcat (pathname, Asset_Directory());
	strcat (pathname, "*");
	strcat (pathname, ASSET_EXTENSION);
	handle = _findfirst (pathname, &fileinfo);
	if (handle != -1) {

		do {
			strcpy (pathname, theApp.Working_Path());
			strcat (pathname, Asset_Directory());
			strcat (pathname, fileinfo.name);
			DeleteFile (pathname);
		
		} while (_findnext (handle, &fileinfo) == 0);
	}
	
	// Clean-up.
	_findclose (handle);

	// Attempt to remove the asset directory.
	strcpy (pathname, theApp.Working_Path());
	strcat (pathname, Asset_Directory());
	_rmdir (pathname);
}


/***********************************************************************************************
 * LightmapPacker::Copy_Assets --																				  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/18/99    IML : Created.                                                                 * 
 *=============================================================================================*/
void LightmapPacker::Copy_Assets (const char *pathname)
{
	char			savedrivename [_MAX_DRIVE];
	char			savedirectoryname [_MAX_DIR];
	char			savefilename [_MAX_FNAME];
	char			loadpathname [_MAX_PATH];
	char			savepath [_MAX_PATH];
	char			savepathname [_MAX_PATH];
	long			handle;
	_finddata_t	fileinfo;
	
	// Attempt to copy every file in the load directory to the save directory.
	strcpy (loadpathname, theApp.Working_Path());
	strcat (loadpathname, Asset_Directory());
	strcat (loadpathname, "*");
	strcat (loadpathname, ASSET_EXTENSION);
	handle = _findfirst (loadpathname, &fileinfo);
	if (handle != -1) {

		// Now we know that there are assets so create the save directory.
		_splitpath (pathname, savedrivename, savedirectoryname, savefilename, NULL);
		strcpy (savepath, savedrivename);
		strcat (savepath, savedirectoryname);
		strcat (savepath, Asset_Directory (savefilename));
		_mkdir (savepath);

		do {
			strcpy (loadpathname, theApp.Working_Path());
			strcat (loadpathname, Asset_Directory());
			strcat (loadpathname, fileinfo.name);
			strcpy (savepathname, savepath);
			strcat (savepathname, fileinfo.name);
		  	CopyFile (loadpathname, savepathname, false);
		
		} while (_findnext (handle, &fileinfo) == 0);
	}

	// Clean-up.
	_findclose (handle);
}


/***********************************************************************************************
 * LightmapPacker::Reset_Statistics --																			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/18/99    IML : Created.                                                                 * 
 *=============================================================================================*/
void LightmapPacker::Reset_Statistics()
{
	FaceCount				= 0;
	LightmapCount			= 0;
	AdjacentFaceCount		= 0;
	BlendedFaceCount		= 0;
	EdgeBlendAreaSum		= 0.0;
	UnscaledTexelCount	= 0;
	ScaledTexelCount		= 0;	
	ReplicaEfficiencySum	= 0.0;
	PackingEfficiencySum	= 0.0;
	TextureSwapCount		= 0;
	OversizeCount			= 0;
	AllFillColorCount		= 0;
}


/***********************************************************************************************
 * LightmapPacker::Update_Statistics --																		  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/18/99    IML : Created.                                                                 * 
 *=============================================================================================*/
void LightmapPacker::Update_Statistics (const Page &page)
{
	PackingEfficiencySum += page.Packing_Efficiency();
	ReplicaEfficiencySum	+= page.Replica_Efficiency();
}


/***********************************************************************************************
 * LightmapPacker::Update_Statistics --																		  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/31/00    IML : Created.                                                                * 
 *=============================================================================================*/
void LightmapPacker::Update_Statistics (const TrianglePacker &trianglepacker)
{
	AdjacentFaceCount += trianglepacker.Adjacent_Face_Count();
	BlendedFaceCount	+= trianglepacker.Blended_Face_Count();
	EdgeBlendAreaSum	+= trianglepacker.Edge_Blend_Area();
}


/***********************************************************************************************
 * LightmapPacker::Collate_Statistics --																		  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/18/99    IML : Created.                                                                 * 
 *=============================================================================================*/
void LightmapPacker::Collate_Statistics()
{
	unsigned	pagecount;
	float		adjacentfaceblendfraction, edgeblendefficiency, scalingefficiency, packingefficiency, cullingefficiency, swappingefficiency;
	
	// Build statistics string.
	adjacentfaceblendfraction = (AdjacentFaceCount > 0) ? ((float) BlendedFaceCount) / ((float) AdjacentFaceCount) : 0.0f;
	edgeblendefficiency		  = (UnscaledTexelCount > 0) ? 1.0f - (EdgeBlendAreaSum / UnscaledTexelCount) : 0.0f;
	scalingefficiency			  = (UnscaledTexelCount > 0) ? 1.0f - (((float) ScaledTexelCount) / ((float) UnscaledTexelCount)) : 0.0f;
	pagecount					  = PagePtrs.Count();
	packingefficiency			  = (pagecount > 0) ? PackingEfficiencySum / pagecount : 0.0f;
	cullingefficiency			  = (pagecount > 0) ? ReplicaEfficiencySum / pagecount : 0.0f;
	swappingefficiency		  = (FaceCount > 0) ? 1.0f - (((float)TextureSwapCount) / FaceCount) : 0.0f;

	_snprintf (_Statistics [STATISTICS_PAGE_FORMAT], sizeof (_Statistics [STATISTICS_PAGE_FORMAT]) - 1,  "%d x %d x %dbpp", PAGE_WIDTH, PAGE_HEIGHT, PageBitDepth);
	_snprintf (_Statistics [STATISTICS_LIGHTMAPS_PROCESSED], sizeof (_Statistics [STATISTICS_LIGHTMAPS_PROCESSED]) - 1, "%d", LightmapCount);
	_snprintf (_Statistics [STATISTICS_ADJACENT_FACE_BLEND_PERCENTAGE], sizeof (_Statistics [STATISTICS_ADJACENT_FACE_BLEND_PERCENTAGE]) - 1, "%.1f%%", adjacentfaceblendfraction * 100.0f);
	_snprintf (_Statistics [STATISTICS_EDGE_BLEND_EFFICIENCY], sizeof (_Statistics [STATISTICS_EDGE_BLEND_EFFICIENCY]) - 1, "%.1f%%", edgeblendefficiency * 100.0f);
	_snprintf (_Statistics [STATISTICS_SCALING_EFFICIENCY], sizeof (_Statistics [STATISTICS_SCALING_EFFICIENCY]) - 1, "%.1f%%", scalingefficiency * 100.0f);
	_snprintf (_Statistics [STATISTICS_PAGES_CREATED], sizeof (_Statistics [STATISTICS_PAGES_CREATED]) - 1, "%d", pagecount);
	_snprintf (_Statistics [STATISTICS_PACKING_EFFICIENCY], sizeof (_Statistics [STATISTICS_PACKING_EFFICIENCY]) - 1, "%.1f%%", packingefficiency * 100.0f);
	_snprintf (_Statistics [STATISTICS_CULLING_EFFICIENCY], sizeof (_Statistics [STATISTICS_CULLING_EFFICIENCY]) - 1, "%.1f%%", cullingefficiency * 100.0f);
	_snprintf (_Statistics [STATISTICS_TEXTURE_SWAP_EFFICIENCY], sizeof (_Statistics [STATISTICS_TEXTURE_SWAP_EFFICIENCY]) - 1, "%.1f%%", swappingefficiency * 100.0f);
	_snprintf (_Statistics [STATISTICS_OVERSIZE_LIGHTMAPS], sizeof (_Statistics [STATISTICS_TEXTURE_SWAP_EFFICIENCY]) - 1, "%d", OversizeCount);
}


/***********************************************************************************************
 * TrianglePacker::TrianglePacker --																			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/28/00    IML : Created.                                                                * 
 *=============================================================================================*/
TrianglePacker::TrianglePacker (const PackingTriangle *principaltriangleptr, const DynamicVectorClass <Triangle> &adjtriangles, unsigned groupid, unsigned edgeblendthickness, float samplerate, W3dRGBStruct fillcolor)
{
	Projection			 = Get_Projection (principaltriangleptr->Normal);
	GroupID				 = groupid;	
	EdgeBlendThickness = edgeblendthickness;
	SampleRate			 = samplerate;
	FillColor			 = fillcolor;

	// Add the principal triangle.
	PrincipalTriangles.Add ((PackingTriangle*) principaltriangleptr);

	// Add the adjacent triangles, sorting them into the three groups COMMON, VALID & NONE.
	for (int a = 0; a < adjtriangles.Count(); a++) {
		if (Get_Projection (adjtriangles [a].Normal) == Projection) {
			AdjacentTriangles [ADJACENT_PROJECTION_COMMON].Add (adjtriangles [a]);
		} else {
			if (Can_Project (adjtriangles [a].Normal)) {
				AdjacentTriangles [ADJACENT_PROJECTION_VALID].Add (adjtriangles [a]);
			} else {
				AdjacentTriangles [ADJACENT_PROJECTION_NONE].Add (adjtriangles [a]);
			}	
		}
	}

	// The triangle sets have been modifed - therefore set bounds.
	Set_Bounds();
}


/***********************************************************************************************
 * TrianglePacker::TrianglePacker --																			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/28/00    IML : Created.                                                                * 
 *=============================================================================================*/
TrianglePacker::TrianglePacker (const TrianglePacker &trianglepacker)
{
	int t, a;

	Projection			 = trianglepacker.Projection;
	GroupID				 = trianglepacker.GroupID;	
	EdgeBlendThickness = trianglepacker.EdgeBlendThickness;
	SampleRate			 = trianglepacker.SampleRate;
	FillColor			 = trianglepacker.FillColor;
	LowerBound			 = trianglepacker.LowerBound;
	UpperBound			 = trianglepacker.UpperBound;

	// Copy the principal triangles.
	for (t = 0; t < trianglepacker.PrincipalTriangles.Count(); t++) {
		PrincipalTriangles.Add (trianglepacker.PrincipalTriangles [t]);
	}

	// Copy the adjacent triangles.
	for (a = 0; a < ADJACENT_PROJECTION_COUNT; a++) {
		for (t = 0; t < trianglepacker.AdjacentTriangles [a].Count(); t++) {
			AdjacentTriangles [a].Add (trianglepacker.AdjacentTriangles [a] [t]);
		}
	}
}
	

/***********************************************************************************************
 * TrianglePacker::Reset --																						  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/28/00    IML : Created.                                                                * 
 *=============================================================================================*/
void TrianglePacker::Set_Bounds()
{
	const Vector2 edge (EdgeBlendThickness, EdgeBlendThickness);

	Vector2  lowerbound (+FLT_MAX, +FLT_MAX);
	Vector2  upperbound (-FLT_MAX, -FLT_MAX);

	// Calculate the smallest bounding rectangle that will contain the projected principal triangles.
	for (int t = 0; t < PrincipalTriangles.Count(); t++) {
		for (int v = 0; v < Triangle::VERTICES_COUNT; v++) {
			
			Vector2 uv;

			uv = Project (PrincipalTriangles [t]->Vertices [v].Point);
			lowerbound.Update_Min (uv);
			upperbound.Update_Max (uv);
		}
	}

	// Increase the bounds by the edge thickness to account for edge blending and snap to integer coordinates.
	lowerbound -= edge;
	upperbound += edge;
	LowerBound.Set (floorf (lowerbound.U), floorf (lowerbound.V));
	UpperBound.Set (ceilf  (upperbound.U), ceilf  (upperbound.V));
}


/***********************************************************************************************
 * TrianglePacker::Edge_Blend_Area --																			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/28/00    IML : Created.                                                                * 
 *=============================================================================================*/
float TrianglePacker::Edge_Blend_Area() const
{
	float		principalareasum;
	unsigned width, height;

	principalareasum = 0.0f;
	for (int t = 0; t < PrincipalTriangles.Count(); t++) {

		Vector2 o, a, b;

		o = Project (PrincipalTriangles [t]->Vertices [0].Point);
		a = Project (PrincipalTriangles [t]->Vertices [1].Point) - o;
		b = Project (PrincipalTriangles [t]->Vertices [2].Point) - o;
		principalareasum += 0.5f * WWMath::Fabs (a.U * b.V - b.U * a.V);
	}
	width  = Width();
	height = Height();
	ASSERT ((width * height) >= principalareasum);
	return ((width * height) - principalareasum);
}


/***********************************************************************************************
 * TrianglePacker::Merge --																						  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/28/00    IML : Created.                                                                * 
 *=============================================================================================*/
TrianglePacker *TrianglePacker::Merge (const TrianglePacker &trianglepacker)
{
	TrianglePacker *mergedpackerptr;

	mergedpackerptr = NULL;
	
	// Do both triangle packers have the same projection, group ID, edge blend thickness, sample rate, and fill color?
	// If not, then they are not compatible packers and cannot be merged.
	if ((Projection == trianglepacker.Projection) &&
		 (GroupID == trianglepacker.GroupID) &&
		 (EdgeBlendThickness == trianglepacker.EdgeBlendThickness) &&
		 (SampleRate == trianglepacker.SampleRate) &&
		 (FillColor == trianglepacker.FillColor)) {

		// For each principal triangle in triangle packer...
		for (int p = 0; p < trianglepacker.PrincipalTriangles.Count(); p++) {

			// For each principal triangle in this...
			for (int q = 0; q < PrincipalTriangles.Count(); q++) {

				// Does one principal triangle abut the other?
				if (trianglepacker.PrincipalTriangles [p]->Abuts (*PrincipalTriangles [q])) {

					// Copy this.
				 	mergedpackerptr = new TrianglePacker (*this);
					ASSERT (mergedpackerptr != NULL);
					break;
				}
			}

			if (mergedpackerptr != NULL) break;
		}

		if (mergedpackerptr != NULL)  {

			// For each principal triangle in triangle packer...
			for (int p = 0; p < trianglepacker.PrincipalTriangles.Count(); p++) {

				// Add it to the merged packer.
				mergedpackerptr->PrincipalTriangles.Add (trianglepacker.PrincipalTriangles [p]);
			}

			// Merge the adjacent triangles (do not allow duplicates).
			for (int a = 0; a < ADJACENT_PROJECTION_COUNT; a++) {
				for (int t = 0; t < trianglepacker.AdjacentTriangles [a].Count(); t++) {
				
					const Triangle *triangleptr;
					bool				 foundadjacent;

					triangleptr = &(trianglepacker.AdjacentTriangles [a][t]);
					foundadjacent = false;
					for (int m = 0; m < AdjacentTriangles [a].Count(); m++) {
						if (triangleptr->Is_Equivalent ((AdjacentTriangles [a]) [m])) {
							foundadjacent = true;
							break;
						}
					}
				
					if (!foundadjacent) {
						mergedpackerptr->AdjacentTriangles [a].Add ((trianglepacker.AdjacentTriangles [a]) [t]);
					}
				}
			}

			// The triangle sets have been modifed - therefore set bounds.
			mergedpackerptr->Set_Bounds();
		}
	}
	
	return (mergedpackerptr);
}


/***********************************************************************************************
 * TrianglePacker::Get_Projection --																			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/19/00    IML : Created.                                                                * 
 *=============================================================================================*/
TrianglePacker::ProjectionEnum TrianglePacker::Get_Projection (const Vector3 &normal)
{
	float x, y, z;

	x = WWMath::Fabs (normal.X);
	y = WWMath::Fabs (normal.Y);
	z = WWMath::Fabs (normal.Z);

	if (y >= x) {
		if (y >= z) {
			if (normal.Y >= 0.0f) {
				return (PROJECTION_Y_POSITIVE);
			} else {
				return (PROJECTION_Y_NEGATIVE);
			}
		} else {
			if (normal.Z >= 0.0f) {
				return (PROJECTION_Z_POSITIVE);
			} else {
				return (PROJECTION_Z_NEGATIVE);
			}
		}
	} else {
		if (x >= z) {
			if (normal.X >= 0.0f) {
				return (PROJECTION_X_POSITIVE);
			} else {
				return (PROJECTION_X_NEGATIVE);
			}
		} else {
			if (normal.Z >= 0.0f) {
				return (PROJECTION_Z_POSITIVE);
			} else {
				return (PROJECTION_Z_NEGATIVE);
			}
		}
	}
}


/***********************************************************************************************
 * TrianglePacker::Can_Project --																				  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/19/00    IML : Created.                                                                * 
 *=============================================================================================*/
bool TrianglePacker::Can_Project (const Vector3 &normal)
{
	static const float _projectionnormals [PROJECTION_COUNT][3] =
	{{ 0.0f,  1.0f,  0.0f},
	 { 0.0f, -1.0f,  0.0f},
	 { 0.0f,  0.0f,  1.0f},
	 { 0.0f,  0.0f, -1.0f},
	 { 1.0f,  0.0f,  0.0f},
	 {-1.0f,  0.0f,  0.0f}
	};

	const float maxprojectionangle = (80.0f * WWMATH_PI) / 180.0f;	// Max. allowed angle between triangle and projection plane. 
	
	Vector3 projectionnormal;
	float	  angle;

	projectionnormal.Set (_projectionnormals [Projection][0], _projectionnormals [Projection][1], _projectionnormals [Projection][2]);
	angle = acosf (Vector3::Dot_Product (normal, projectionnormal));
	return (angle < maxprojectionangle);
}


/***********************************************************************************************
 * TrianglePacker::Project --																						  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/19/00    IML : Created.                                                                * 
 *=============================================================================================*/
Vector2 TrianglePacker::Project (const Vector3 &point) const
{
	Vector2 t;

	switch (Projection) {

		case PROJECTION_Y_POSITIVE:
		case PROJECTION_Y_NEGATIVE:
			t.Set (point.X, point.Z);
			break;

		case PROJECTION_Z_POSITIVE:
		case PROJECTION_Z_NEGATIVE:
			t.Set (point.X, point.Y);
			break;

		case PROJECTION_X_POSITIVE:
		case PROJECTION_X_NEGATIVE:
			t.Set (point.Y, point.Z);
			break;
	}
	t *= SampleRate;
	return (t);
}


/***********************************************************************************************
 * TrianglePacker::Rasterize --																					  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/08/00    IML : Created.                                                                * 
 *=============================================================================================*/
void TrianglePacker::Rasterize (TargaLoader &targaloader, ProceduralTexture *proceduraltexture, TrueColorTarga &rasterizedtarga)
{
	const unsigned pixeldepth						 = 24;
	const unsigned sqrtforwardsamplespertexel  = 4;
	const unsigned sqrtbackwardsamplespertexel = 4;
	const unsigned	principalpriority				 = 2;
	const unsigned adjacentpriority				 = 1;

	// There must be at least one principal triangle to rasterize.
	ASSERT (PrincipalTriangles.Count() >= 1);

	unsigned				  width, height;	
	SampleSurface		 *forwardsurfaceptr, *backwardsurfaceptr;	
	int					  t;
	unsigned				  projectiontrianglecount;
	float					  right, top;	
	ProjectionTriangle *projectiontriangles;
	Vector3				  points [Triangle::VERTICES_COUNT];	
	Vector2				  sourceuvs [Triangle::VERTICES_COUNT];
	Vector2				  projectionuvs [Triangle::VERTICES_COUNT];
	TrueColorTarga		 *sourcetargaptr;	
	float					  oosqrtsamplespertexel;	
	Vector2				  samplepoint;
	int					  sampledt;
	unsigned				  x, y, u, v;
	float					  oowidth, ooheight;	

	width  = Width();
	height = Height();

	// Allocate a sample surface to store destination->source (backward) samples.
	backwardsurfaceptr = new SampleSurface (width, height, proceduraltexture);
	ASSERT (backwardsurfaceptr != NULL);

	// Allocate enough projection triangles to cover the principal triangles and those triangles that can be projected.
	projectiontriangles = new ProjectionTriangle [PrincipalTriangles.Count() + AdjacentTriangles [ADJACENT_PROJECTION_COMMON].Count() + AdjacentTriangles [ADJACENT_PROJECTION_VALID].Count()];
	ASSERT (projectiontriangles != NULL);

	// For each principal triangle...
	projectiontrianglecount = PrincipalTriangles.Count();
	for (t = 0; t < (int) projectiontrianglecount; t++) {
		for (int v = 0; v < Triangle::VERTICES_COUNT; v++) {
			points [v]			= PrincipalTriangles [t]->Vertices [v].Point;
			sourceuvs [v]		= PrincipalTriangles [t]->Vertices [v].UV;
			projectionuvs [v] = Project (PrincipalTriangles [t]->Vertices [v].Point) - LowerBound;
		}
		sourcetargaptr = targaloader.Load (*PrincipalTriangles [t]);
		projectiontriangles [t] = ProjectionTriangle (points, sourceuvs, sourcetargaptr, projectionuvs);
	}

	// For each adjacent triangle that is common or valid...
	right = width;
	top   = height;
	for (int a = ADJACENT_PROJECTION_COMMON; a <= ADJACENT_PROJECTION_VALID; a++) {
		for (t = 0; t < AdjacentTriangles [a].Count(); t++) {

			unsigned outcode, outcodeu, outcodev;

			// Test the triangle for trivial rejection.
			outcode = 0xf;
			for (int v = 0; v < Triangle::VERTICES_COUNT; v++) {
				projectionuvs [v] = Project (AdjacentTriangles [a] [t].Vertices [v].Point) - LowerBound;
				outcodeu = (projectionuvs [v].U < 0.0f) | ((projectionuvs [v].U >= right) << 1);
				outcodev = (projectionuvs [v].V < 0.0f) | ((projectionuvs [v].V >= top) << 1);
				outcode &= (outcodeu | (outcodev << 2));
			}
			
			// Should the triangle be accepted (ie. not trivially rejected)?
			if (outcode == 0) {
				for (int v = 0; v < Triangle::VERTICES_COUNT; v++) {
					points [v]	  = AdjacentTriangles [a] [t].Vertices [v].Point;
					sourceuvs [v] = AdjacentTriangles [a] [t].Vertices [v].UV;
				}
				sourcetargaptr = targaloader.Load (AdjacentTriangles [a] [t]);
				projectiontriangles [projectiontrianglecount] = ProjectionTriangle (points, sourceuvs, sourcetargaptr, projectionuvs);
				projectiontrianglecount++;
			}
		}
	}	

	// Sample the projection triangles into the backward surface.
	oosqrtsamplespertexel = 1.0f / sqrtbackwardsamplespertexel;
	sampledt	= 0;
	for (y = 0;	y < height; y++) {
		for (x = 0;	x < width; x++) {
			for (v = 0; v < sqrtbackwardsamplespertexel; v++) {
				samplepoint.V = y + (v * oosqrtsamplespertexel);
				for (u = 0; u < sqrtbackwardsamplespertexel; u++) {
					samplepoint.U = x + (u * oosqrtsamplespertexel);

					if (sampledt < PrincipalTriangles.Count()) {
						if (backwardsurfaceptr->Sample (samplepoint, projectiontriangles [sampledt], principalpriority)) goto nextsample;
					}
									
					for (t = 0; t < PrincipalTriangles.Count(); t++) {
						if (t != sampledt) {
							if (backwardsurfaceptr->Sample (samplepoint, projectiontriangles [t], principalpriority)) {
			 					sampledt = t;
			 					goto nextsample;
			 				}
			 			}
					}  
							  			
					if ((sampledt >= PrincipalTriangles.Count()) && (sampledt < (int) projectiontrianglecount)) {
						if (backwardsurfaceptr->Sample (samplepoint, projectiontriangles [sampledt], adjacentpriority)) goto nextsample;
					}

					for (t = PrincipalTriangles.Count(); t < (int) projectiontrianglecount; t++) {
						if (t != sampledt) {
							if (backwardsurfaceptr->Sample (samplepoint, projectiontriangles [t], adjacentpriority)) {
								sampledt = t;
								goto nextsample;
							}
						}
					}

					nextsample:;
				}
			}
		}
	}

	// If less than 50% of the texels were sampled in the backward rasterization prepare a forward rasterization.
	forwardsurfaceptr = NULL;
	if (backwardsurfaceptr->Sampling_Ratio() < 0.5f) {

		// Allocate a sample surface to store source->destination (forward) samples.
		forwardsurfaceptr = new SampleSurface (width, height, proceduraltexture);
		ASSERT (forwardsurfaceptr != NULL);

		for (t = 0; t < PrincipalTriangles.Count(); t++) {

			Vector2  d, d0, d1, d2;
			float		projectionarea, maxlength, pseudoarea;
			unsigned sqrtsamplecount;
			float		oosqrtsamplecount;
			unsigned a, b;
			float		alpha, beta;

			// Calculate the sample rate based on the triangle's projected area.
			// NOTE: If the area is small relative to its longest edge use the edge length instead.
			d  = projectionuvs [2] - projectionuvs [0];
			d0 = projectionuvs [1] - projectionuvs [0];
			d1 = projectionuvs [2] - projectionuvs [1];
			d2 = projectionuvs [0] - projectionuvs [2];
			projectionarea = 0.5f * WWMath::Fabs (d.U * d0.V - d0.U * d.V);
			maxlength = (float) WWMath::Sqrt (MAX (MAX (d0.Length2(), d1.Length2()), d2.Length2()));
			pseudoarea = MAX (projectionarea, maxlength);
			sqrtsamplecount = ceilf (WWMath::Sqrt (sqrtforwardsamplespertexel * sqrtforwardsamplespertexel * pseudoarea));
			
			// Sample the projection triangle into the forward surface.
			oosqrtsamplecount = 1.0f / sqrtsamplecount;
			for (b = 0; b < sqrtsamplecount; b++) {
				beta = b * oosqrtsamplecount;
				for (a = 0; a < sqrtsamplecount; a++) {
					alpha = a * oosqrtsamplecount;
					if (alpha + beta <= 1.0f) {
						forwardsurfaceptr->Sample (alpha, beta, projectiontriangles [t], principalpriority);
					}
				}
			}
		}
	}

	// Resize the targa which holds the resulting rasterization.
	rasterizedtarga.Reformat (width, height, pixeldepth);

	// Iterate over the backward (and possibly forward) surface and translate it into the rasterized targa.
	for (y = 0; y < height; y++) {
		
		unsigned miny, maxy, minx, maxx;
		bool		inrangeofprincipal;
		unsigned	a, b;

		miny = MAX ((int) 0, ((int) y) - ((int) EdgeBlendThickness));
		maxy = MIN (height - 1, y + EdgeBlendThickness);
		for (x = 0; x < width; x++) {

			W3dRGBStruct color;
			
			minx = MAX ((int) 0, ((int) x) - ((int) EdgeBlendThickness));
			maxx = MIN (width - 1, x + EdgeBlendThickness);

			// Find out if this texel is within edge blend texels of a texel that has a 'principal sample'.
			// NOTE: If so then this texel can potentially be read and therefore must contain valid data.
			//			Otherwise fill color will suffice.
			inrangeofprincipal = false;
			for (b = miny; b <= maxy; b++) {
				for (a = minx; a <= maxx; a++) {
					if (backwardsurfaceptr->Priority (a, b) >= principalpriority) {
						inrangeofprincipal = true;
						break;
					}
					if (forwardsurfaceptr != NULL) {
						if (forwardsurfaceptr->Priority (a, b) >= principalpriority) {
							inrangeofprincipal = true;
							break;
						}
					}
				}
				if (inrangeofprincipal) break;
			}
			
			// Is texel within range of 'principal texel'?
			if (inrangeofprincipal) {
				
				// Does the backward surface's texel have a sample?
				if (backwardsurfaceptr->Get_Color (x, y, color)) {
					rasterizedtarga.Set_Color (x, y, color);
				} else {
					if (forwardsurfaceptr != NULL) {
						if (forwardsurfaceptr->Get_Color (x, y, color)) {
							rasterizedtarga.Set_Color (x, y, color);
						} else {
							rasterizedtarga.Set_Color (x, y, FillColor);
						}
					} else {
						rasterizedtarga.Set_Color (x, y, FillColor);
					}
				}

			} else {
				rasterizedtarga.Set_Color (x, y, FillColor);
			}
		}
	}

	// Pad any fill color texels.
	rasterizedtarga.Fill (FillColor);

	// Set the packed UV's of each principal triangle to normalized projection UV's.
	oowidth  = 1.0f / width;
	ooheight	= 1.0f / height;
	for (t = 0; t < PrincipalTriangles.Count(); t++) {
		for (unsigned v = 0; v < Triangle::VERTICES_COUNT; v++) {

			Vector2 uv;
			
			uv = Project (PrincipalTriangles [t]->Vertices [v].Point) - LowerBound;
			uv.Scale (oowidth, ooheight);
			PrincipalTriangles [t]->PackedUVs [v] = uv;
		}
	}

	// Clean-up.
	if (forwardsurfaceptr != NULL) delete forwardsurfaceptr;
	delete [] projectiontriangles;
	delete backwardsurfaceptr;
}


/***********************************************************************************************
 * ProjectionTriangle::ProjectionTriangle --																	  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/08/00    IML : Created.                                                                * 
 *=============================================================================================*/
TrianglePacker::ProjectionTriangle::ProjectionTriangle (const Vector3 *points, const Vector2 *sourceuvs, const TrueColorTarga *sourcetargaptr, const Vector2 *projectionuvs)
{
	Points [0]			= points [0];
	Points [1]			= points [1] - points [0];
	Points [2]			= points [2] - points [0];
	SourceUVs [0]		= sourceuvs [0];
	SourceUVs [1]		= sourceuvs [1] - sourceuvs [0];
	SourceUVs [2]		= sourceuvs [2] - sourceuvs [0];
	ProjectionUVs [0] = projectionuvs [0];
	ProjectionUVs [1] = projectionuvs [1] - projectionuvs [0];
	ProjectionUVs [2] = projectionuvs [2] - projectionuvs [0];
	SourceTargaPtr		= (TrueColorTarga*) sourcetargaptr;
}


/***********************************************************************************************
 * TrueColorTarga::TrueColorTarga --																			  *
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
TrueColorTarga::TrueColorTarga()
	: Targa()
{
	const unsigned width		  = 1;
	const unsigned height	  = 1;
	const unsigned pixeldepth = 24;

	// Check that width, height and pixel depth will fit into the Targa internal storage types.
	ASSERT (width  <= SHRT_MAX);
	ASSERT (height <= SHRT_MAX);
	ASSERT (pixeldepth <= SCHAR_MAX); 
	
	mFlags			  = TGAF_IMAGE;
	Header.ImageType = TGA_TRUECOLOR;
	
	Reformat (width, height, pixeldepth);
}


/***********************************************************************************************
 * TrueColorTarga::TrueColorTarga --																			  *
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
TrueColorTarga::TrueColorTarga (unsigned width, unsigned height, unsigned pixeldepth)
	: Targa()
{
	// Check that width, height and pixel depth will fit into the Targa internal storage types.
	ASSERT (width  <= SHRT_MAX);
	ASSERT (height <= SHRT_MAX);
	ASSERT (pixeldepth <= SCHAR_MAX); 
	
	mFlags			  = TGAF_IMAGE;
	Header.ImageType = TGA_TRUECOLOR;
	
	Reformat (width, height, pixeldepth);
}


/***********************************************************************************************
 * TrueColorTarga::TrueColorTarga --																			  *
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
TrueColorTarga::TrueColorTarga (unsigned width, unsigned height, unsigned pixeldepth, const W3dRGBStruct &clearcolor)
	: Targa()
{
	// Check that width, height and pixel depth will fit into the Targa internal storage types.
	ASSERT (width  <= SHRT_MAX);
	ASSERT (height <= SHRT_MAX);
	ASSERT (pixeldepth <= SCHAR_MAX); 
	
	mFlags			  = TGAF_IMAGE;
	Header.ImageType = TGA_TRUECOLOR;
	
	Reformat (width, height, pixeldepth, clearcolor);
}


/***********************************************************************************************
 * TrueColorTarga::Reformat --																					  *
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
void TrueColorTarga::Reformat (unsigned width, unsigned height, unsigned pixeldepth)
{
	size_t size;

	Header.Width		= width;
	Header.Height		= height;
	Header.PixelDepth = pixeldepth;

	size = width * height * TGA_BytesPerPixel (pixeldepth);
	if (size > 0) {

		// NOTE: Use malloc() or realloc() here because destructor will use free().
		mImage = (char*) realloc (mImage, size);
		ASSERT (mImage != NULL);
	}
} 


/***********************************************************************************************
 * TrueColorTarga::Reformat --																					  *
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
void TrueColorTarga::Reformat (unsigned width, unsigned height, unsigned pixeldepth, const W3dRGBStruct &clearcolor)
{
	UnpackedTexelStruct cleartexel (clearcolor.R, clearcolor.G, clearcolor.B, 0);

	Reformat (width, height, pixeldepth);
	Clear (cleartexel);
}


/***********************************************************************************************
 * TrueColorTarga::Pixel_Format --																				  *
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
srPixelConvert::e_surfaceType TrueColorTarga::Pixel_Format (unsigned pixeldepth)
{
	// Map a pixel depth (in bits) to a Targa supported pixel format.
	switch (pixeldepth) {

		case 16:
			return (srPixelConvert::ARGB1555);

		case 24:
			return (srPixelConvert::RGB888);

		case 32:
			return (srPixelConvert::ARGB8888);

		default:
			ASSERT (false);
			break;
	}

	ASSERT (false);
	return (srPixelConvert::e_surfaceType (0));
}


/***********************************************************************************************
 * TrueColorTarga::Clear --																						  *
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
void TrueColorTarga::Clear (const TrueColorTarga::UnpackedTexelStruct &cleartexel)
{
	const unsigned bytespertexel = TGA_BytesPerPixel (Pixel_Depth());

	unsigned char *texelptr;

	for (unsigned y = 0; y < Height(); y++) {
		texelptr = ((unsigned char*) GetImage()) + (Width() * y * bytespertexel);
		for (unsigned x = 0; x < Width(); x++) {
			Pack_Texel (cleartexel, texelptr, bytespertexel);
			texelptr += bytespertexel;
		}
	}
}


/***********************************************************************************************
 * TrueColorTarga::Load --																							  *
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
char *TrueColorTarga::Load (const char *pathname)
{
	const char *loaderrormessage = "The image file ""%s"" cannot be loaded (load error: %d).\
 Please ensure that the image file exists and is in TGA (Truevision TARGA) format.";

	const char *formaterrormessage = "The image file ""%s"" is not in a suitable TGA format.\
 Please ensure that the image file is in true color format (16, 24 or 32 bit).";

	static char _messagebuffer [256];

	long			   error;
	bool			   valid;
	StringBuilder  errormessage (_messagebuffer, sizeof (_messagebuffer));
	
	error = Targa::Load (pathname, TGAF_IMAGE);
	if (error != 0) {
		errormessage.Copy (loaderrormessage, pathname, error);
		return (errormessage.String());
	}

	// Check for a specific subset of the available .TGA texel formats which are suitable for
	// true color targas (ie. true color, not paletted, not monochrome).
	valid  = (Header.ImageType == TGA_TRUECOLOR) || (Header.ImageType == TGA_TRUECOLOR_ENCODED);
	valid &= (Header.PixelDepth == 16) || (Header.PixelDepth == 24) || (Header.PixelDepth == 32);
	if (!valid) {
		errormessage.Copy (formaterrormessage, pathname);
		return (errormessage.String());
	}

	// No error message.
	return (NULL);
}


/***********************************************************************************************
 * TrueColorTarga::Save --																							  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/18/99    IML : Created.                                                                 * 
 *=============================================================================================*/
char *TrueColorTarga::Save (const char *pathname)
{
	const char *saveerrormessage = "Cannot save the image file ""%s"" (save error: %d).";

	static char _messagebuffer [256];

	long			   error;
	StringBuilder  errormessage (_messagebuffer, sizeof (_messagebuffer));

	error = Targa::Save (pathname, TGAF_IMAGE);
	if (error != 0) {
		errormessage.Copy (saveerrormessage, pathname, error);
		return (errormessage.String());
	}

	// No error message.
	return (NULL);
}


/***********************************************************************************************
 * TrueColorTarga::Blit --																							  *
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
void TrueColorTarga::Blit (TrueColorTarga &destination, unsigned x, unsigned y)
{
	unsigned			sourcebytespertexel, destbytespertexel;
	unsigned			maxc, maxr;
	unsigned char *sourceptr, *destptr;

	sourcebytespertexel = TGA_BytesPerPixel (Pixel_Depth()); 
	destbytespertexel   = TGA_BytesPerPixel (destination.Pixel_Depth());

	// Currently, a true color targa cannot blit to itself.
	ASSERT (this != &destination);

	ASSERT (GetImage() != NULL);
	ASSERT (destination.GetImage() != NULL);

	// Sanity checks.
	if ((x >= destination.Width()) || (y >= destination.Height())) return;

	// Clip to destination.
	if (x + Width() <= destination.Width()) {
		maxc = Width();
	} else {
		maxc = destination.Width() - x;
	}
	if (y + Height() <= destination.Height()) {
		maxr = Height();
	} else {
		maxr = destination.Height() - y;
	}

	// For each row...
	for (unsigned r = 0; r < maxr; r++) {
		sourceptr = ((unsigned char*) GetImage()) + (Width() * r * sourcebytespertexel);
		destptr = ((unsigned char*) destination.GetImage()) + (((destination.Width() * (y + r)) + x) * destbytespertexel);

		// For each column...
		for (unsigned c = 0; c < maxc; c++) {

			UnpackedTexelStruct unpackedtexel;
			
			Pack_Texel (*Unpack_Texel (sourceptr, sourcebytespertexel, unpackedtexel), destptr, destbytespertexel);

			// Advance to next texel.
			sourceptr += sourcebytespertexel;
			destptr   += destbytespertexel;
		}
	}
}


/***********************************************************************************************
 * TrueColorTarga::Scale --																						  *
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
void TrueColorTarga::Scale (TrueColorTarga &destination, unsigned width, unsigned height)
{
	size_t		 size, newsize;
	ColorSurface surface (Pixel_Format (Pixel_Depth()), Width(), Height());
	SRBOOL		 success;

	// Image data must exist.
	ASSERT (GetImage() != NULL);

	// Currently, source and destination pixel depths must match.
	ASSERT (Pixel_Depth() == destination.Pixel_Depth());

	// Clamp width, height.
	if (width < 1)  width  = 1;
	if (height < 1) height = 1;

	// Define highest quality filter.
	surface.setFilter (&srBSplineFilter);

	// Enable clamping.
	surface.setHClampMode (true); 
	surface.setVClampMode (true); 

	// Copy the targa source texel data to the Surrender surface.
	size = Width() * Height() * TGA_BytesPerPixel (Pixel_Depth());
	memcpy (surface.getDataPtr(), GetImage(), size);

	// Scale.
	success = surface.rescale (width, height);
	ASSERT (success);
	
	// Resize the destination to match that of the rescaled Surrender surface.
	destination.Reformat (width, height, destination.Pixel_Depth());

	// Copy the rescaled Surrender surface to the targa destination.
	newsize = width * height * TGA_BytesPerPixel (destination.Pixel_Depth());
	memcpy (destination.GetImage(), surface.getDataPtr(), newsize);
}


/***********************************************************************************************
 * TrueColorTarga::Scale --																						  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/11/99    IML : Created.                                                                *
 *=============================================================================================*/
#define AVERAGE(a, b) \
  (((a) + (b)) / 2) + (((a) + (b)) % 2)

#define BINARY_CHOP \
	if (result == -1) { \
		maxdimension = dimension; \
		dimension = AVERAGE (dimension, mindimension); \
		if (maxdimension == dimension) break; \
	} else {	\
		if (result == 1) { \
			mindimension = dimension; \
			dimension = AVERAGE (dimension, maxdimension); \
			if (mindimension == dimension) break; \
		} else {	\
			break; \
		} \
	} \

void TrueColorTarga::Scale (TrueColorTarga &destination, float error)
{
	const unsigned attemptcount = 8;

	unsigned			 attempt, maxdimension, mindimension, dimension;
	int				 result;
	unsigned			 width, height;
	TrueColorTarga *scaledtargaptr;

	// Calculate width.
	maxdimension = Width();
	mindimension = 1;

	dimension = AVERAGE (maxdimension, mindimension);
	for (attempt = 0; attempt < attemptcount; attempt++) {
		scaledtargaptr = new TrueColorTarga (dimension, Height(), Pixel_Depth());
		ASSERT (scaledtargaptr != NULL);
		Scale (*scaledtargaptr);
		result = Compare (*scaledtargaptr, error);
		delete scaledtargaptr;
		BINARY_CHOP;
 	}
	width = dimension;

	// Calculate height.
	maxdimension = Height();
	mindimension = 1;

	dimension = AVERAGE (maxdimension, mindimension);
	for (attempt = 0; attempt < attemptcount; attempt++) {
		scaledtargaptr = new TrueColorTarga (Width(), dimension, Pixel_Depth());
		ASSERT (scaledtargaptr != NULL);
		Scale (*scaledtargaptr);
		result = Compare (*scaledtargaptr, error);
		delete scaledtargaptr;
		BINARY_CHOP;
	}
	height = dimension;

	// Scale targa. 
	Scale	(destination, width, height);
}
#undef AVERAGE
#undef BINARY_CHOP


/***********************************************************************************************
 * TrueColorTarga::Transpose --																					  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/07/99    IML : Created.                                                                * 
 *=============================================================================================*/
void TrueColorTarga::Transpose (TrueColorTarga &destination)
{
	unsigned			bytespertexel;
	size_t			size;
	unsigned			sourcestride;
	unsigned char *sourceptr, *destinationptr, *stagingbuffer;

	// Image data must exist.
	ASSERT (GetImage() != NULL);

	// Currently, source and destination pixel depths must match.
	ASSERT (Pixel_Depth() == destination.Pixel_Depth());

	bytespertexel = TGA_BytesPerPixel (Pixel_Depth());
	size = Width() * Height() * bytespertexel;
	
	stagingbuffer = new unsigned char [size];
	ASSERT (stagingbuffer != NULL);

	// Write transposed image data to staging buffer.
	sourcestride	= Width() * bytespertexel;
	destinationptr = stagingbuffer;
	for (unsigned y = 0; y < Width(); y++) {
		sourceptr = ((unsigned char*) GetImage()) + (y * bytespertexel);
		for (unsigned x = 0; x < Height(); x++) {
			for (unsigned b = 0; b < bytespertexel; b++) {
				*destinationptr++ = *(sourceptr + b);
			}
			sourceptr += sourcestride;
		}
	}

	// Resize the destination to match that of the transposed source.
	destination.Reformat (Height(), Width(), destination.Pixel_Depth());

	// Copy transposed image data to destination.
	memcpy (destination.GetImage(), stagingbuffer, size);

	// Clean-up.
	delete [] stagingbuffer;
}	


/***********************************************************************************************
 * TrueColorTarga::Compare --																						  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/11/99    IML : Created.                                                                *
 *=============================================================================================*/
int TrueColorTarga::Compare (TrueColorTarga &comparison, float epsilon)
{
	unsigned			 bytespertexel, comparisonbytespertexel;
	int	 			 epsilondelta;
	float				 xscale, yscale;	
	int				 result;	
	unsigned char	*imageptr;

	ASSERT ((epsilon >= 0.0f) && (epsilon <= 1.0f));

	bytespertexel				= (size_t) TGA_BytesPerPixel (Pixel_Depth());
	comparisonbytespertexel = (size_t) TGA_BytesPerPixel (comparison.Pixel_Depth());
	epsilondelta				= MAX (0, epsilon * 255.0f);
	result						= -1;

	// Calculate destination:source scaling factors.
	xscale = ((float) comparison.Width())  / Width();
	yscale = ((float) comparison.Height()) / Height();

	// For each texel in this targa measure the delta for the corresponding texel in the comparison targa.
	imageptr = (unsigned char*) GetImage();
	for (unsigned y = 0; y < Height(); y++) {

		unsigned	ys;

		// NOTE: Position sample point at center of texel.
		ys = MIN ((unsigned) ((y + 0.5f) * yscale), (unsigned) comparison.Height() - 1);

		for (unsigned x = 0; x < Width(); x++) {

			unsigned					xs;
			unsigned char		  *comparisonimageptr;
			int					   delta;
			UnpackedTexelStruct  texel, comparisontexel;

			// NOTE: Position sample point at center of texel.
			xs = MIN ((unsigned) ((x + 0.5f) * xscale), (unsigned) comparison.Width() - 1);

			comparisonimageptr = ((unsigned char*) comparison.GetImage()) + (((ys * comparison.Width()) + xs) * comparisonbytespertexel);
			Unpack_Texel (imageptr, bytespertexel, texel);
			Unpack_Texel (comparisonimageptr, comparisonbytespertexel, comparisontexel);

			delta = abs (((int) texel.Byte [0]) - ((int) comparisontexel.Byte [0]));
			for (unsigned b = 1; b < sizeof (texel.Byte); b++) {
				delta = MAX (delta, abs (((int) texel.Byte [b]) - ((int) comparisontexel.Byte [b])));
			}

			if (delta > epsilondelta) {
	
				// Return result.
				return (1);
			}

			if (delta == epsilondelta) {
				result = 0;
			}
			
			// Advance.
			imageptr += bytespertexel;
		}
	}

	// Return result.
	return (result);
}


/***********************************************************************************************
 * TrueColorTarga::Compare --																						  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/11/99    IML : Created.                                                                *
 *=============================================================================================*/
int TrueColorTarga::Compare (TrueColorTarga &comparison, unsigned x, unsigned y, float epsilon)
{
	unsigned			 bytespertexel, comparisonbytespertexel;
	int	 			 epsilondelta;
	int				 result;	
	unsigned char	*imageptr, *comparisonptr;

	ASSERT ((epsilon >= 0.0f) && (epsilon <= 1.0f));

	bytespertexel				= (size_t) TGA_BytesPerPixel (Pixel_Depth());
	comparisonbytespertexel = (size_t) TGA_BytesPerPixel (comparison.Pixel_Depth());
	epsilondelta				= MAX (0, epsilon * 255.0f);
	result						= -1;

	// This targa must be contained by comparison targa. 
	if (x + Width()  >= comparison.Width())  return (1);
	if (y + Height() >= comparison.Height()) return (1);

	// For each row...
	for (unsigned r = 0; r < Height(); r++) {
		imageptr		  = ((unsigned char*) GetImage()) + (Width() * r * bytespertexel);
		comparisonptr = ((unsigned char*) comparison.GetImage()) + (((comparison.Width() * (y + r)) + x) * comparisonbytespertexel);

		// For each column...
		for (unsigned c = 0; c < Width(); c++) {

			UnpackedTexelStruct texel, comparisontexel;
			int					  delta;

			Unpack_Texel (imageptr, bytespertexel, texel);
			Unpack_Texel (comparisonptr, comparisonbytespertexel, comparisontexel);

			delta = abs (((int) texel.Byte [0]) - ((int) comparisontexel.Byte [0]));
			for (unsigned b = 1; b < sizeof (texel.Byte); b++) {
				delta = MAX (delta, abs (((int) texel.Byte [b]) - ((int) comparisontexel.Byte [b])));
			}

			if (delta > epsilondelta) {
	
				// Return result.
				return (1);
			}

			if (delta == epsilondelta) {
				result = 0;
			}
			
			// Advance.
			imageptr += bytespertexel;
			comparisonptr += comparisonbytespertexel;
		}
	}

	// Return result.
	return (result);
}


/***********************************************************************************************
 * TrueColorTarga::Fill --																							  *
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
bool TrueColorTarga::Fill (const W3dRGBStruct &fillcolor)
{
	const unsigned growthstep = 2048;
	const UnpackedTexelStruct filltexel (fillcolor.R, fillcolor.G, fillcolor.B, 0); 

	DynamicVectorClass <PointStruct> *fourconnectedarrays;
	unsigned									 index;
	bool										 allfillcolor;	

	// Allocate dynamic arrays and initialize.
	fourconnectedarrays = new DynamicVectorClass <PointStruct> [2];
	ASSERT (fourconnectedarrays != NULL);
	fourconnectedarrays [0].Set_Growth_Step (growthstep);
	fourconnectedarrays [1].Set_Growth_Step (growthstep);

	// Step thru all texels and pad all existing four-connected texels, and add new four-connected texel coordinates to dynamic array.
	index = 0;
	allfillcolor = true;
	for (unsigned y = 0; y < Height(); y++) {
		for (unsigned x = 0; x < Width(); x++) {
			allfillcolor &= Fill_Four_Connected (x, y, filltexel, fourconnectedarrays [index]);
		}
	}

	if (!allfillcolor) {

		// While there are still unpadded texels...
		while (fourconnectedarrays [index].Count() > 0) {

			// For each four connected texel... 
			for (int e = 0; e < fourconnectedarrays [index].Count(); e++) {
				Fill_Four_Connected ((fourconnectedarrays [index])[e].X, (fourconnectedarrays [index])[e].Y, filltexel, fourconnectedarrays [index ^ 1]);
			}
			fourconnectedarrays [index].Clear();
		
			// Toggle dynamic array in which to place new four-connected texel coordinates. 
			index ^= 1;
		}
	}

	// Clean-up.
	delete [] fourconnectedarrays;
	
	return (allfillcolor);
}	


/***********************************************************************************************
 * TrueColorTarga::Fill_Four_Connected --																		  *
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
bool TrueColorTarga::Fill_Four_Connected (unsigned x, unsigned y, const UnpackedTexelStruct &filltexel, DynamicVectorClass <PointStruct> &fourconnectedarray)
{
	const unsigned	fourconnectedcount = 4;
  	const unsigned	bytespertexel		 = TGA_BytesPerPixel (Pixel_Depth());

	static const int offsetx [fourconnectedcount] = {-1, +1,  0,  0};
	static const int offsety [fourconnectedcount] = { 0,  0, -1, +1};
	
	UnpackedTexelStruct  unpackedtexel;
	unsigned char		  *texelptr;

	// Is texel (x, y) the fill color (ie. not already filled)?  
	texelptr = Get_Texel (x, y);
	ASSERT (texelptr != NULL);
	Unpack_Texel (texelptr, bytespertexel, unpackedtexel);
	if (unpackedtexel == filltexel) {
		
		unsigned	adj;
		bool		fourconnected [fourconnectedcount];
		unsigned	fillcount, r, g, b;

		// Initialize.
		fillcount = 0;
		r = g = b = 0;

		// Fill the texel by averaging its non-fill color four-connected neighbors.
		for (adj = 0; adj < fourconnectedcount; adj++) {

			int				adjx, adjy;
			unsigned char *adjtexelptr;

			fourconnected [adj] = false;
			adjx = ((int) x) + offsetx [adj];
			adjy = ((int) y) + offsety [adj];
			adjtexelptr	= Get_Texel (adjx, adjy);
			if (adjtexelptr != NULL) {
				Unpack_Texel (adjtexelptr, bytespertexel, unpackedtexel);
				if (unpackedtexel == filltexel) {
					fourconnected [adj] = true;
				} else {
					r += unpackedtexel.Red();
					g += unpackedtexel.Green();
					b += unpackedtexel.Blue();
					fillcount++;
				}
			}
		}

		if (fillcount > 0) {
		
			float oofillcount;

			// Fill texel (x, y) with average of neighboring non-fill colors.
			oofillcount = 1.0f / fillcount;
			r = MIN (r * oofillcount, UCHAR_MAX);
			g = MIN (g * oofillcount, UCHAR_MAX);
			b = MIN (b * oofillcount, UCHAR_MAX);
			UnpackedTexelStruct texel (r, g, b, 0);
			Pack_Texel (texel, texelptr, bytespertexel);
			
			// Add the coordinates of any neighboring fill colors.
			for (adj = 0; adj < fourconnectedcount; adj++) {
				if (fourconnected [adj]) {
				
					PointStruct p;

					p.X = ((int) x) + offsetx [adj];
					p.Y = ((int) y) + offsety [adj];
					fourconnectedarray.Add (p);
				}
			}
		}
		return (true);
	} else {
		return (false);
	}
}


/***********************************************************************************************
 * TrueColorTarga::Add --																							  *
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
void TrueColorTarga::Add (TrueColorTarga &targa)
{
	unsigned	width, height;
	unsigned thisbytespertexel, targabytespertexel;

	// Scale both targas to maximum width & height of both. 
	width  = MAX (Width(),  targa.Width());
	height = MAX (Height(), targa.Height());
	Scale (width, height);
	targa.Scale (width, height);

	// Step thru both targas and add the pixel data. Store result in this targa.  
	// For each row...
	thisbytespertexel	 = TGA_BytesPerPixel (Pixel_Depth()); 
	targabytespertexel = TGA_BytesPerPixel (targa.Pixel_Depth());
	for (unsigned r = 0; r < height; r++) {
	
		unsigned char *thisptr, *targaptr;
		
		thisptr  = ((unsigned char*) GetImage()) + (width * r * thisbytespertexel);
		targaptr = ((unsigned char*) targa.GetImage()) + (width * r * targabytespertexel);

		// For each column...
		for (unsigned c = 0; c < width; c++) {

			UnpackedTexelStruct thisunpackedtexel, targaunpackedtexel;
			
			Unpack_Texel (thisptr, thisbytespertexel, thisunpackedtexel);
			Unpack_Texel (targaptr, targabytespertexel, targaunpackedtexel);
			
			thisunpackedtexel += targaunpackedtexel;
			Pack_Texel (thisunpackedtexel, thisptr, thisbytespertexel);

			// Advance to next texel.
			thisptr  += thisbytespertexel;
			targaptr += targabytespertexel;
		}
	}
}	


/***********************************************************************************************
 * TrueColorTarga::Rasterize --																					  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/22/00    IML : Created.                                                                * 
 *=============================================================================================*/
void TrueColorTarga::Rasterize (TrueColorTarga &destination, const W3dRGBStruct &fillcolor, unsigned vertexcount, const W3dRGBStruct *vertexcolors, Vector2 *vertexuvs)
{
	const unsigned edgecount = 3;
	const static float _vertexuvs [edgecount][2] =
	{
		{1.0f, 0.0f},
		{0.0f, 0.0f},
		{0.0f, 1.0f}
	};
	
	const UnpackedTexelStruct filltexel (fillcolor.R, fillcolor.G, fillcolor.B, 0);

	const unsigned	sqrtsamplecountminusone	 = 3;
	const float		oosqrtsamplecountminusone = 1.0f / sqrtsamplecountminusone;
	const unsigned	mindimension = 3;	

	Vector2				   edges [edgecount];
	float					   longestlength2, r, w, h;	
	unsigned				   longestedge;
	Vector2				   i, j;	
	float						oowidthminustwo, ooheightminustwo;
	unsigned				   bytespertexel;	
	unsigned char		  *destptr;			
	W3dRGBStruct		   colors [edgecount];	
	float					   oowidth, ooheight;
	unsigned					n;

	// UV's must define a triangle.
	ASSERT (vertexcount == edgecount);

	// Define edges of source triangle.
	edges [0] = vertexuvs [1] - vertexuvs [0];
	edges [1] = vertexuvs [2] - vertexuvs [1];
	edges [2] = vertexuvs [0] - vertexuvs [2];

	// The destination triangle is a right-angled triangle that occupies one half of the
	// destination targa. Size the destination targa usinh the following rules:
	// (a) length of longest edge equals length of targa's diagonal (hypotenuse of destination triangle)
	// (b) ratio of lengths of remaining two edges of source triangle equals ratio of dimensions of targa.

	// Identify the longest edge.
	longestlength2 = 0.0f;
	for (unsigned e = 0; e < edgecount; e++) {
		
		float length2 = edges [e].Length2();

		if (length2 > longestlength2) {
			longestedge		= e;
			longestlength2 = length2;
		}
	}

	// Calculate width and height of destination targa.
	r = edges [(longestedge + 1) % edgecount].Length() / edges [(longestedge + 2) % edgecount].Length();
	w = WWMath::Sqrt (longestlength2	/ (1.0f + r));
	h = w / r;

	// Resize the destination targa.
	destination.Reformat (MAX (mindimension, (unsigned) (w * Width())), MAX (mindimension, (unsigned) (h * Height())), Pixel_Depth());

	// Set up a coordinate system to define the source triangle in terms of the destination triangle.
	i = -edges [(longestedge + 1) % edgecount];
	j =  edges [(longestedge + 2) % edgecount];

	// Define colors that correspond to each vertex;
	for (n = 0; n < vertexcount; n++) {
		if (vertexcolors != NULL) {
			colors [n] = vertexcolors [(n + longestedge) % edgecount];
		} else {
			colors [n] = fillcolor;
		}
	}

	// Iterate over the destination triangle, sampling the source triangle at regular intervals.
	// Also insert a one pixel wide filtered edge comprising interpolated vertex colors.
	// For each destination texel...
	bytespertexel	  = TGA_BytesPerPixel (Pixel_Depth());
	oowidthminustwo  = 1.0f / (destination.Width()  - 2);
	ooheightminustwo = 1.0f / (destination.Height() - 2);
	destptr			  = (unsigned char*) destination.GetImage();
	for (unsigned v = 0; v < destination.Height(); v++) {
		for (unsigned u = 0; u < destination.Width(); u++) {
			
			unsigned				  validsamplecount;
			float					  r, g, b;	
			float					  fu, fv;
			float					  oovalidsamplecount;	
			UnpackedTexelStruct averagetexel;
			float					  clamped;	

			// For given destination texel subsample the source triangle.
			validsamplecount = 0;
			r = g = b		  = 0.0f;
			for (unsigned sv = 0; sv <= sqrtsamplecountminusone; sv++) {
				
				fv = (v + (sv * oosqrtsamplecountminusone) - 1.0f) * ooheightminustwo;
				for (unsigned su = 0; su <= sqrtsamplecountminusone; su++) {
				
					fu = (u + (su * oosqrtsamplecountminusone) - 1.0f) * oowidthminustwo;

					// Which region does (fu, fv) fall in?
					clamped = false;
					if (fu < 0.0f) {
						fu = 0.0f;
						clamped = true;
					} else {
						if (fu > 1.0f) {
							fu = 1.0f;
							clamped = true;
						}
					}

					if (fv < 0.0f) {
						fv = 0.0f;
						clamped = true;
					} else {
						if (fv > 1.0f) {
							fv = 1.0f;
							clamped = true;
						}
					}
					
					if ((fu + fv) >= 1.0f) {
						
						float ood = 1.0f / (fu + fv);

						fu = fu * ood;
						fv = fv * ood;
						clamped = true;
					}
					
					if (!clamped) {
			
						Vector2				   p;
						unsigned				   x, y;
						UnpackedTexelStruct  unpackedtexel;
						unsigned char		  *sourceptr;

						// Calculate source coordinates.
						p = vertexuvs [(longestedge + 2) % 3] + (fu * i) + (fv * j);
						x = (unsigned) MIN (MAX ((int) (p.X * Width()),  0), ((int) Width())  - 1);
						y = (unsigned) MIN (MAX ((int) (p.Y * Height()), 0), ((int) Height()) - 1);

						// Sample the source.
						sourceptr = ((unsigned char*) GetImage()) + (((Width() * y) + x) * bytespertexel);
						Unpack_Texel (sourceptr, bytespertexel, unpackedtexel);
						r += unpackedtexel.Red();
						g += unpackedtexel.Green();
						b += unpackedtexel.Blue();

					} else {

						// Calculate source color.
						r += 0.5f * ((colors [1].R * fu) + (colors [2].R * (1.0f - fu)) + (colors [2].R * (1.0f - fv)) + (colors [0].R * fv));
						g += 0.5f * ((colors [1].G * fu) + (colors [2].G * (1.0f - fu)) + (colors [2].G * (1.0f - fv)) + (colors [0].G * fv));
						b += 0.5f * ((colors [1].B * fu) + (colors [2].B * (1.0f - fu)) + (colors [2].B * (1.0f - fv)) + (colors [0].B * fv));
					}
					validsamplecount++;
				}
			}

			// One or more samples must have been taken.
			ASSERT (validsamplecount > 0);

			// Destination texel = average of samples.
			oovalidsamplecount = 1.0f / validsamplecount;
			averagetexel.Set (r * oovalidsamplecount, g * oovalidsamplecount, b * oovalidsamplecount, 0);
			Pack_Texel (averagetexel, destptr, bytespertexel);

			// Advance.
			destptr += bytespertexel;
		}
	}

	// Replace source UV's with destination coordinates for caller.
	oowidth  = 1.0f / destination.Width();
	ooheight = 1.0f / destination.Height();
	for (n = 0; n < vertexcount; n++) {
		vertexuvs [n].Set (_vertexuvs [((edgecount - 1 - longestedge) + n) % edgecount][0], _vertexuvs [((edgecount - 1 - longestedge) + n) % edgecount][1]);
		
		// Scale and translate UV's to accomodate one texel wide filtered edge.
		vertexuvs [n].U *= (destination.Width()  - 2) * oowidth;
		vertexuvs [n].V *= (destination.Height() - 2) * ooheight;
		vertexuvs [n].U += oowidth;
		vertexuvs [n].V += ooheight;
	}
}


/***********************************************************************************************
 * TrueColorTarga::Pad --																							  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/22/00    IML : Created.                                                                * 
 *=============================================================================================*/
void TrueColorTarga::Pad (TrueColorTarga &destination, unsigned padwidth, unsigned padheight, const W3dRGBStruct &padcolor, DynamicVectorClass <PackingTriangle*> &triangleptrs)
{
	unsigned				  w, h;	
	float					  oobw, oobh;	
	UnpackedTexelStruct padtexel (padcolor.R, padcolor.G, padcolor.B, 0);
	Matrix3				  uvmatrix;

	// Optimization: was a non-zero pad thickness specified?
	if ((padwidth > 0) || (padheight > 0)) {

		// Round pad width and height to even no. to ensure that there is an equal amount of padding on each side.
		padwidth  += (padwidth  & 0x1);
		padheight += (padheight & 0x1);

		// Calculate scaling and translation factors.
		w	  = Width();
		h	  = Height();
		oobw = 1.0f / (w + padwidth);
		oobh = 1.0f / (h + padheight);

		if (this == &destination) {

			TrueColorTarga	targacopy (w, h, Pixel_Depth());

			Blit (targacopy, 0, 0);
		
			// Resize to match that of the padded targa.
			Reformat (w + padwidth, h + padheight, Pixel_Depth());

			// Clear to pad color.
			Clear (padtexel);
		
			// Blit copy to this targa. Offset by pad thickness.
			targacopy.Blit (*this, padwidth >> 1, padheight >> 1);

		} else {

			// Resize the destination to match that of the padded targa.
			destination.Reformat (w + padwidth, h + padheight, Pixel_Depth());

			// Clear destination to pad color.
			destination.Clear (padtexel);

			// Blit this to destination. Offset by pad thickness.
			Blit (destination, padwidth >> 1, padheight >> 1);
		}

		// Pad pad color.
		destination.Fill (padcolor);

		// Calculate scaling and translation matrix.
		uvmatrix [0].Set (w * oobw, 0.0f, 0.0f);
		uvmatrix [1].Set (0.0f, h * oobh, 0.0f);
		uvmatrix [2].Set ((padwidth >> 1) * oobw, (padheight >> 1) * oobh, 1.0f);
		
		// Transpose transform matrix.
		// NOTE: This matrix is set up for post multiplication (ie. V' = M X V, matrix M, vector V).
		uvmatrix = uvmatrix.Transpose();

		// Apply transform to all triangles...
		for (int t = 0; t < triangleptrs.Count(); t++) {
			for (unsigned v = 0; v < Triangle::VERTICES_COUNT; v++) {

				Vector3 uv;

				// NOTE: Create a homogeneous vector (3-components) from the UV vector so that it can be transformed by the matrix.
				uv.Set (triangleptrs [t]->PackedUVs [v].X, triangleptrs [t]->PackedUVs [v].Y, 1.0f);
				uv = uvmatrix * uv;
				triangleptrs [t]->PackedUVs [v].Set (uv.X, uv.Y); 
			}
		}
	}
}


/***********************************************************************************************
 * TrianglePacker::SampleSurface::SampleSurface --															  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/12/00    IML : Created.                                                                * 
 *=============================================================================================*/
TrianglePacker::SampleSurface::SampleSurface (unsigned width, unsigned height, ProceduralTexture *blendtexture)
{
	SampleStruct *sampleptr;

	Width  = width;
	Height = height;
	SampledTexelCount = 0;
	
	Surface = new SampleStruct [width * height];
	ASSERT (Surface != NULL);

	// Initialize surface.
	sampleptr = Surface;
	for (unsigned s = 0; s < width * height; s++) {
		sampleptr->Red		  = 0;
		sampleptr->Green	  = 0;
		sampleptr->Blue	  = 0;
		sampleptr->Count	  = 0;
		sampleptr->Priority = 0;
		sampleptr++;
	}

	BlendTexture = blendtexture;
}


/***********************************************************************************************
 * TrianglePacker::SampleSurface::Sample --																	  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/12/00    IML : Created.                                                                * 
 *=============================================================================================*/
bool TrianglePacker::SampleSurface::Sample (const Vector2 &samplepoint, const ProjectionTriangle &projectiontriangle, unsigned priority)
{
	const float epsilon = 0.0001f;

	int	  u, v;	
	Vector2 t0, t1, t2;
	float	  alpha, beta;

	u = floorf (samplepoint.U);
	v = floorf (samplepoint.V);

	// NOTE: Sample point must be in range.
	if ((u < 0) || (u >= (int) Width))  return (false);
	if ((v < 0) || (v >= (int) Height)) return (false);

	// Use the baricentric method to determine if the sample point lies inside
	//	the triangle (see Graphics Gems I p390). 
	t0 = samplepoint - projectiontriangle.ProjectionUVs [0];
	t1 = projectiontriangle.ProjectionUVs [1];
	t2 = projectiontriangle.ProjectionUVs [2];
	if (WWMath::Fabs (t1.U) <= epsilon) {
		beta  = t0.U / t2.U;
		if (beta <= 0.0f || beta >= 1.0f) return (false);
		alpha = (t0.V - (beta * t2.V)) / t1.V;
	} else {
		beta  = (t0.V * t1.U - t0.U * t1.V) / (t2.V * t1.U - t2.U * t1.V);
		if (beta <= 0.0f || beta >= 1.0f) return (false);
		alpha = (t0.U - beta * t2.U) / t1.U;
	}

	// Does the projection triangle contain the sample point? 
	if ((alpha > 0.0f) && ((alpha + beta) < 1.0f)) {
										
		Vector2		  uv;
		W3dRGBStruct  color;
		SampleStruct *sampleptr;

		// Use the baricentric coordinates (alpha, beta) to calculate the texture coordinates.
		uv = (alpha * projectiontriangle.SourceUVs [1]) + (beta * projectiontriangle.SourceUVs [2]) + projectiontriangle.SourceUVs [0];
		if (!projectiontriangle.SourceTargaPtr->Get_Color (uv, color)) return (false);

		// Should the texel color be blended with a procedural texture?
		if (BlendTexture != NULL) {

			Vector3 p;
			float	  v;

			p = (alpha * projectiontriangle.Points [1]) + (beta * projectiontriangle.Points [2]) + projectiontriangle.Points [0];
			v = BlendTexture->Value (p);
			color.Set ((uint8) (color.R * v), (uint8) (color.G * v), (uint8) (color.B * v));
		}
		
		// Add the sample.
		sampleptr = Surface + (v * Width + u);
		sampleptr->Red	  += color.R;
		sampleptr->Green += color.G;
		sampleptr->Blue  += color.B;
		if (sampleptr->Count == 0) SampledTexelCount++;
		sampleptr->Count++;

		// Update priority.
		if (priority > sampleptr->Priority) sampleptr->Priority = priority;

		return (true);
	} else {
		return (false);
	}
}


/***********************************************************************************************
 * TrianglePacker::SampleSurface::Sample --																	  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/12/00    IML : Created.                                                                * 
 *=============================================================================================*/
bool TrianglePacker::SampleSurface::Sample (float alpha, float beta, const ProjectionTriangle &projectiontriangle, unsigned priority)
{
	Vector2		  samplepoint;
	int			  u, v;
	Vector2		  uv;
	W3dRGBStruct  color;
	SampleStruct *sampleptr;

	// Use the baricentric coordinates (alpha, beta) to calculate the sample point.
	samplepoint = (alpha * projectiontriangle.ProjectionUVs [1]) + (beta * projectiontriangle.ProjectionUVs [2]) + projectiontriangle.ProjectionUVs [0];
	u = floorf (samplepoint.U);
	v = floorf (samplepoint.V);

	// NOTE: Sample point must be in range.
	if ((u < 0) || (u >= (int) Width))  return (false);
	if ((v < 0) || (v >= (int) Height)) return (false);

	// Use the baricentric coordinates (alpha, beta) to calculate the texture coordinates.
	uv = (alpha * projectiontriangle.SourceUVs [1]) + (beta * projectiontriangle.SourceUVs [2]) + projectiontriangle.SourceUVs [0];
	if (!projectiontriangle.SourceTargaPtr->Get_Color (uv, color)) return (false);

	// Should the texel color be blended with a procedural texture?
	if (BlendTexture != NULL) {

		Vector3 p;
		float	  v;

		p = (alpha * projectiontriangle.Points [1]) + (beta * projectiontriangle.Points [2]) + projectiontriangle.Points [0];
		v = BlendTexture->Value (p);
		color.Set ((uint8) (color.R * v), (uint8) (color.G * v), (uint8) (color.B * v));
	}

	// Add the sample.
	sampleptr = Surface + (v * Width + u);
	sampleptr->Red	  += color.R;
	sampleptr->Green += color.G;
	sampleptr->Blue  += color.B;
	if (sampleptr->Count == 0) SampledTexelCount++;
	sampleptr->Count++;

	// Update priority.
	if (priority > sampleptr->Priority) sampleptr->Priority = priority;

	return (true);
}


/***********************************************************************************************
 * Page::Page --																										  *
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
Page::Page (unsigned bitdepth, const W3dRGBStruct &clearcolor)
	: TrueColorTarga (PAGE_WIDTH, PAGE_HEIGHT, bitdepth)
{
	const UnpackedTexelStruct cleartexel (clearcolor.R, clearcolor.G, clearcolor.B, 0);

	Region *regionptr;
	
	// Clear this page to a recognizable color.
	Clear (cleartexel);

	// Create a single region for the entire page.
	regionptr = new Region;
	ASSERT (regionptr != NULL);
	regionptr->Set (0, 0, PAGE_WIDTH - 1, PAGE_HEIGHT - 1);
	VacantRegionList.Add_Head (regionptr);

	Reset_Statistics();
}


/***********************************************************************************************
 * Page::Page --																										  *
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
Page::~Page()
{
	// Delete the used region list.
	while (UsedRegionList.First_Valid()) {
		delete UsedRegionList.First_Valid();
	}

	// Delete the region list.
	while (VacantRegionList.First_Valid()) {
		delete VacantRegionList.First_Valid();
	}
}


/***********************************************************************************************
 * Page::Reset_Statisitcs --																						  *
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
void Page::Reset_Statistics()
{
	AssetCount			= 0;
	UsedTexelCount		= 0;
	ReplicaTexelCount	= 0;
}


/***********************************************************************************************
 * Page::Pack --																										  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/19/00    IML : Created.                                                                * 
 *=============================================================================================*/
bool Page::Pack (TrueColorTarga &targa, float epsilon, DynamicVectorClass <PackingTriangle*> &triangleptrs)
{
	TrueColorTarga *targaptr [TRANSPOSE_COUNT];
	bool				 replica, packed;
	TransposeEnum	 transposed;
	unsigned			 cost;
	Region			 targetregion;	

	// Targa must fit on this page.
	ASSERT (targa.Width()  <= Width());
	ASSERT (targa.Height() <= Height());

	targaptr [UNTRANSPOSED] = &targa;
	
	// Create a transposed version of the targa.
	targaptr [TRANSPOSED] = new TrueColorTarga (targaptr [UNTRANSPOSED]->Width(), targaptr [UNTRANSPOSED]->Height(), targaptr [UNTRANSPOSED]->Pixel_Depth());
	ASSERT (targaptr [TRANSPOSED] != NULL);
	targaptr [UNTRANSPOSED]->Blit (*targaptr [TRANSPOSED], 0, 0);
	targaptr [TRANSPOSED]->Transpose();

	// Attempt to find a replica targa on this page.
	transposed = (targaptr [UNTRANSPOSED]->Width() >= targaptr [UNTRANSPOSED]->Height()) ? UNTRANSPOSED : TRANSPOSED;

	replica = Replica_Region (*targaptr [transposed], epsilon, targetregion);
	if (!replica) {
		transposed = (transposed == UNTRANSPOSED) ? TRANSPOSED : UNTRANSPOSED;
		replica = Replica_Region (*targaptr [transposed], epsilon, targetregion);
	}

	if (replica) {
		
	 	// Update statistics.
		ReplicaTexelCount += targaptr [transposed]->Width() * targaptr [transposed]->Height();

	} else {
  
		// Attempt to pack targa with width >= height first. 
		transposed = (targaptr [UNTRANSPOSED]->Width() >= targaptr [UNTRANSPOSED]->Height()) ? UNTRANSPOSED : TRANSPOSED;

		// Can the targa be packed?
		packed = Lowest_Cost_Region (*targaptr [transposed], cost, targetregion);
		
		// If not packed then transpose the targa and attempt to pack it again.
		if (!packed) {
			transposed = (transposed == UNTRANSPOSED) ? TRANSPOSED : UNTRANSPOSED;
			packed = Lowest_Cost_Region (*targaptr [transposed], cost, targetregion);
		}

		if (packed) {
	
			// Region must be aligned on an even boundary. This will reduce texel bleeding as a result of mip-mapping.
			ASSERT ((targetregion.X0 & 0x1) == 0x0);
			ASSERT ((targetregion.Y0 & 0x1) == 0x0);
			ASSERT ((targetregion.X1 & 0x1) == 0x1);
			ASSERT ((targetregion.Y1 & 0x1) == 0x1);

			Insert_Region (targetregion);

	 		// Blit the targa onto this page.
	 		targaptr [transposed]->Blit (*this, targetregion.X0, targetregion.Y0);

	 		// Update statistics.
	 		UsedTexelCount += targaptr [transposed]->Width() * targaptr [transposed]->Height();
		}
	}

	// Transform the UV's to reflect the packing.
	if (replica || packed) {

		float	  su, sv;	// Scaling factors of the targa within this page.
		float	  tu, tv;	// Translation of the targa within this page.
		Matrix3 uvmatrix;

		// Calculate scaling and translation factors.
		su = ((float) (targaptr [UNTRANSPOSED]->Width())) / Width();
		sv = ((float) (targaptr [UNTRANSPOSED]->Height())) / Height();
		tu = ((float) targetregion.X0) / Width();
		tv = ((float) targetregion.Y0) / Height();

		// Was the targa transposed?
		if (transposed == TRANSPOSED) {

			// Define scaling/rotation vectors in transform matrix.
			uvmatrix [0].Set (0.0f, su  , 0.0f);
			uvmatrix [1].Set (sv  , 0.0f, 0.0f);

		} else {

			// Define scaling/rotation vectors in transform matrix.
			uvmatrix [0].Set (su  , 0.0f, 0.0f);
			uvmatrix [1].Set (0.0f, sv  , 0.0f);
		}
				
		// Define translation vector in transform matrix.
		uvmatrix [2].Set (tu, tv, 1.0f);

		// Transpose transform matrix.
		// NOTE: Thus matrix is set up for post multiplication (ie. V' = M X V, matrix M, vector V).
		uvmatrix = uvmatrix.Transpose();

	 	// Iterate over all triangles and transform their packed UV's.
		for (int t = 0; t < triangleptrs.Count(); t++) {
			for (unsigned v = 0; v < Triangle::VERTICES_COUNT; v++) {

				Vector3 uv;

				// NOTE: Create a homogeneous vector (3-components) from the UV vector so that it can be transformed by the matrix.
				uv.Set (triangleptrs [t]->PackedUVs [v].X, triangleptrs [t]->PackedUVs [v].Y, 1.0f);
				uv = uvmatrix * uv;
				triangleptrs [t]->PackedUVs [v].Set (uv.X, uv.Y); 
			}
		}

		// Update statistics.
		AssetCount++;
	}

	// Clean-up.
	delete targaptr [TRANSPOSED];

	// Return success.
	return (replica || packed);
}


/***********************************************************************************************
 * Page::Replica_Region --																							  *
 *																															  *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/17/99    IML : Created.                                                                * 
 *=============================================================================================*/
bool Page::Replica_Region (TrueColorTarga &targa, float epsilon, Region &replicaregion)
{
	bool		 replica;
	Region	*regionptr;
	unsigned  regionwidth, regionheight;
	
	// Iterate over all used regions to see if one matches the targa to within tolerance.
	replica = false;
	regionptr = (Region*) UsedRegionList.First_Valid();
	while (regionptr != NULL) {

		regionwidth  = regionptr->X1 - regionptr->X0 + 1;
		regionheight = regionptr->Y1 - regionptr->Y0 + 1;
	
		// If regions are the same size...
		if ((regionwidth == targa.Width()) && (regionheight == targa.Height())) {

			// If regions are deemed equal to within epsilon...		
			if (targa.Compare (*this, regionptr->X0, regionptr->Y0, epsilon) <= 0) {
				replica = true;
				replicaregion = *regionptr;
				break;
			}
		}
		
		// Next region.
		regionptr = (Region*) regionptr->Next_Valid();
	}

	return (replica);
}


/***********************************************************************************************
 * Page::Lowest_Cost_Region --																					  *
 *																															  *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/8/99    IML : Created.                                                                 * 
 *=============================================================================================*/
bool Page::Lowest_Cost_Region (TrueColorTarga &targa, unsigned &lowestcost, Region &lowestcostregion)
{
	unsigned  width, height;
	bool		 packed;
	unsigned  mintotalsubregioncount;
	Region	*regionptr;

	// Find a vacant region in the page that will incur the lowest cost if it contains the targa asset.
	width  = targa.Width();
	height = targa.Height();
	packed = false;
	mintotalsubregioncount = UINT_MAX;
	regionptr = (Region*) VacantRegionList.First_Valid();
	while (regionptr != NULL) {

		Region candidateregion;
		
		// Does the asset fit inside the region?
		if (regionptr->Accomodates (width, height, candidateregion)) {

			unsigned	 totalsubregioncount, subregioncount;
			Region	*fragmentregionptr, *subregions;

			fragmentregionptr = (Region*) VacantRegionList.First_Valid();
			totalsubregioncount = 0;
			while (fragmentregionptr != NULL) {

				// Calculate the fragmentation count for the fragment region.
				if (fragmentregionptr->Intersects (candidateregion, subregioncount, &subregions)) {
					totalsubregioncount += subregioncount;
				}

				// Next region.
				fragmentregionptr = (Region*) fragmentregionptr->Next_Valid();
			}

			// Is this the minimum total subregion count?
			if (totalsubregioncount < mintotalsubregioncount) {
				packed = true;
				lowestcostregion = candidateregion;
				mintotalsubregioncount = totalsubregioncount;
			}
		}
		
		// Next region.
		regionptr = (Region*) regionptr->Next_Valid();
	}

	lowestcost = mintotalsubregioncount;
	
	// Can the asset be packed on this page?
	return (packed);
}


/***********************************************************************************************
 * Page::Insert_Region --																							  *
 *																															  *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/8/99    IML : Created.                                                                 * 
 *=============================================================================================*/
void Page::Insert_Region (const Region &insertionregion)
{
	Region *regionptr;
						
	// First, add the insertion region to the used region list.
	regionptr = new Region (insertionregion);
	ASSERT (regionptr != NULL);
	UsedRegionList.Add_Head (regionptr);

	// Walk down the vacant list. If any region intersects the target region, fragment it into
	// 0 to 4 sub-regions, depending on the relative location of the clipped region.
	regionptr = (Region*) VacantRegionList.First_Valid();
	while (regionptr != NULL) {

		unsigned  subregioncount;
		Region	*subregions, *removalptr;

		// If there is an intersection between the region and target region...
		if (regionptr->Intersects (insertionregion, subregioncount, &subregions)) {
		
			// Remove the original region, advance to next region.
			removalptr = regionptr;
			regionptr  = (Region*) regionptr->Next_Valid();
			delete removalptr;
				
			// Add the subregions to the head of the list. 
			// NOTE: Adding to the head of the list will ensure that they will not get re-processed by the current 'list walker'.
			for (int s = ((int) subregioncount) - 1; s >= 0; s--) {
			
				// Only add the subregion if it is not contained by another region ie. avoid redundancy.
				if (!Contains (subregions [s])) {
						
					Region *newregionptr;
						
					newregionptr = new Region (subregions [s]);
					ASSERT (newregionptr != NULL);
					VacantRegionList.Add_Head (newregionptr);
				}
			}		
						
		} else {

			// Advance to next region.
			regionptr = (Region*) regionptr->Next_Valid();
		}
	}
}


/***********************************************************************************************
 * Page::Region::Accomodates --																					  *
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
bool Page::Region::Accomodates (unsigned width, unsigned height, Region &targetregion)
{
	if ((width <= Width()) && (height <= Height())) {

		// Locate the target region in the top-left corner of this region.
		targetregion.Set (X0, Y0, X0 + width - 1, Y0 + height - 1);
		return (true);
	} else {
		return (false);
	}
}


/***********************************************************************************************
 * Page::Region::Intersects --																					  *
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
bool Page::Region::Intersects (const Region &targetregion, unsigned &subregioncount, Region **subregions)
{
	const unsigned maxsubregioncount = 4; // Maximum no. of fragmented subregions.

	static Region _subregions [maxsubregioncount];

	unsigned s;

	// First, test the target region for trivial rejection against the left, right, top and bottom
	// sides of this region respectively.
	if (targetregion.X1 < X0) return (false);
	if (targetregion.X0 > X1) return (false);
	if (targetregion.Y1 < Y0) return (false);
	if (targetregion.Y0 > Y1) return (false);

	// Now it is known that the target region intersects this region in some way.
	// Count and generate the subregions between the target region and this region.
	s = 0;
	if (targetregion.X0 > X0) {
		_subregions [s].Set (X0, Y0, targetregion.X0 - 1, Y1);
		s++;
	}
	if (targetregion.X1 < X1) {
		_subregions [s].Set (targetregion.X1 + 1, Y0, X1, Y1);
		s++;
	}
	if (targetregion.Y0 > Y0) {
		_subregions [s].Set (X0, Y0, X1, targetregion.Y0 - 1);
		s++;
	}
	if (targetregion.Y1 < Y1) {
		_subregions [s].Set (X0, targetregion.Y1 + 1, X1, Y1);
		s++;
	}
	subregioncount = s;
	*subregions	= _subregions;
	return (true);
}


/***********************************************************************************************
 * Page::Region::Contains --																						  *
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
bool Page::Contains (const Region &testregion)
{
	Region *regionptr;

	// Step down the vacant list until a region is found that contains the test region or end of list.
	regionptr = (Region*) VacantRegionList.First_Valid();
	while (regionptr != NULL) {
		if ((testregion.X0 >= regionptr->X0) && (testregion.X1 <= regionptr->X1) && (testregion.Y0 >= regionptr->Y0) && (testregion.Y1 <= regionptr->Y1)) {
			
			// Test region is contained by a region in the list.
			return (true);
		}
		regionptr = (Region*) regionptr->Next_Valid();
	}

	// Test region is not contained by a region in the list.
	return (false);
}

