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
 *                 Project Name : WWPhys                                                       *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwphys/visrendercontext.cpp                  $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 5/17/01 10:42a                                              $*
 *                                                                                             *
 *                    $Revision:: 16                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   VisRenderContextClass::VisRenderContextClass -- Constructor                               *
 *   VisRenderContextClass::Set_Vis_ID -- set the currently active Vis ID                      *
 *   VisRenderContextClass::Set_Resolution -- set the vis rendering resolution                 *
 *   VisRenderContextClass::Get_Resolution -- get the current vis rendering resolution         *
 *   VisRenderContextClass::Scan_Frame_Buffer -- scan the frame buffer for visible objects     *
 *   VisRenderContextClass::Compute_2D_Bounds -- compute the 2D bounds of a 3D box             *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "visrendercontext.h"
#include "simplevec.h"

#include "win.h"
#include "rawfile.h"
#include "visrasterizer.h"


const int CLEAR_VIS_COLOR	= 0x00000000;						// Vis id for background/clear pixels
const float BACKFACE_OVERFLOW_FRACTION = 0.005f;			// max percentage of backface before overflow (rejection)

static VisRasterizerClass			_VisRasterizer;			// Instance of a vis rasterizer 


/***********************************************************************************************
**
** VisRenderContextClass Implementation
**
***********************************************************************************************/


/***********************************************************************************************
 * VisRenderContextClass::VisRenderContextClass -- Constructor                                 *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/18/2000  gth : Created.                                                                 *
 *=============================================================================================*/
VisRenderContextClass::VisRenderContextClass
(
	CameraClass & cam,
	VisTableClass & vtab
) :
	SpecialRenderInfoClass(cam,RENDER_VIS),
	VisTable(vtab)
{
	VisRasterizer = &_VisRasterizer;
	VisRasterizer->Set_Camera(&cam);
}


VisRenderContextClass::~VisRenderContextClass(void)
{
	VisRasterizer->Set_Camera(NULL);
}


/***********************************************************************************************
 * VisRenderContextClass::Set_Vis_ID -- set the currently active Vis ID                        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/18/2000  gth : Created.                                                                 *
 *=============================================================================================*/
void VisRenderContextClass::Set_Vis_ID(uint32 id)
{
	WWASSERT(id < BACKFACE_VIS_ID);
	_VisRasterizer.Set_Frontface_ID(id);
	_VisRasterizer.Set_Backface_ID((uint32)BACKFACE_VIS_ID);
}


/***********************************************************************************************
 * VisRenderContextClass::Set_Resolution -- set the vis rendering resolution                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/29/2001  gth : Created.                                                                 *
 *=============================================================================================*/
void VisRenderContextClass::Set_Resolution(int resx,int resy)
{
	_VisRasterizer.Set_Resolution(resx,resy);
}


/***********************************************************************************************
 * VisRenderContextClass::Get_Resolution -- get the current vis rendering resolution           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/29/2001  gth : Created.                                                                 *
 *=============================================================================================*/
void VisRenderContextClass::Get_Resolution(int * set_resx,int * set_resy)
{
	_VisRasterizer.Get_Resolution(set_resx,set_resy);
}


/***********************************************************************************************
 * VisRenderContextClass::Scan_Frame_Buffer -- scan the frame buffer for visible objects       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/18/2000  gth : Created.                                                                 *
 *=============================================================================================*/
void VisRenderContextClass::Scan_Frame_Buffer
(
	const AABoxClass & wrld_bbox,
	VisSampleClass * sample
)
{
	// transform wrld_bbox into view space and compute the 2d bounding rectangle
	Vector2 min_v,max_v;
	Compute_2D_Bounds(wrld_bbox,&min_v,&max_v);
	Scan_Frame_Buffer(min_v,max_v,sample);
}


/***********************************************************************************************
 * VisRenderContextClass::Scan_Frame_Buffer -- scan the frame buffer for visible objects       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/18/2000  gth : Created.                                                                 *
 *=============================================================================================*/
void VisRenderContextClass::Scan_Frame_Buffer(VisSampleClass * sample)
{
	Scan_Frame_Buffer(Vector2(0,0),Vector2(1,1),sample);
}


/***********************************************************************************************
 * VisRenderContextClass::Scan_Frame_Buffer -- scan the frame buffer for visible objects       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/18/2000  gth : Created.                                                                 *
 *=============================================================================================*/
void VisRenderContextClass::Scan_Frame_Buffer
(
	const Vector2 & min,
	const Vector2 & max,
	VisSampleClass * sample
)
{
	int width,height;
	VisRasterizer->Get_Resolution(&width,&height);

	int minx = MAX(min.X * width , 1);		// ignore the far left column
	int miny = MAX(min.Y * height , 1);		// ignore the top row
	int maxx = MIN(max.X * width , width-1);
	int maxy = MIN(max.Y * height , height-1);
	int backface_count = 0;

	const uint32 * pixel_row = NULL;

	/*
	** Loop over the pixels, counting backfaces and enabling the visibility of
	** each object encountered in the buffer
	*/			
	for (int y=miny; y<maxy; y++) {

		pixel_row = VisRasterizer->Get_Pixel_Row(y,minx,maxx);
		for (int x=0; x<maxx-minx; x++) {
			if (pixel_row[x] != 0) {

				int id = pixel_row[x];
				if (id == BACKFACE_VIS_ID) {
					backface_count++;
				} else {
					VisTable.Set_Bit(id,true);
				}
			}
		}
	}

	if (sample != NULL) {
		int total_pixels = (maxx-minx)*(maxy-miny);
		float backface_fraction = (float)backface_count / (float)total_pixels;
		
		if (backface_fraction > BACKFACE_OVERFLOW_FRACTION) {

			WWDEBUG_SAY(("%s Backface Overflow ",sample->Get_Cur_Direction_Name()));
			sample->Set_Results(VIS_STATUS_BACKFACE_OVERFLOW,backface_fraction);
		
		} else {

			if (backface_count > 0) {
				WWDEBUG_SAY(("%s Backface Leak ",sample->Get_Cur_Direction_Name()));
				sample->Set_Results(VIS_STATUS_BACKFACE_LEAK,backface_fraction);
			} else {
				WWDEBUG_SAY(("%s ",sample->Get_Cur_Direction_Name()));
				sample->Set_Results(VIS_STATUS_OK,0.0f);
			}
		}
	} 
}


/***********************************************************************************************
 * VisRenderContextClass::Compute_2D_Bounds -- compute the 2D bounds of a 3D box               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/18/2000  gth : Created.                                                                 *
 *=============================================================================================*/
void VisRenderContextClass::Compute_2D_Bounds
(
	const AABoxClass &	wrld_bbox,
	Vector2 *				min_v,
	Vector2 *				max_v
)
{
	#define NUM_BOX_VERTS 8
	static float _boxverts[NUM_BOX_VERTS][3] = 
	{
		{  1.0f, 1.0f, 1.0f },		// +z ring of 4 verts
		{ -1.0f, 1.0f, 1.0f },
		{ -1.0f,-1.0f, 1.0f },
		{  1.0f,-1.0f, 1.0f },

		{  1.0f, 1.0f,-1.0f },		// -z ring of 4 verts;
		{ -1.0f, 1.0f,-1.0f },
		{ -1.0f,-1.0f,-1.0f },
		{  1.0f,-1.0f,-1.0f },
	};
	
	// rotate and project the corners of the box
	Vector3 corner[8];
	int ivert;
	for (ivert=0; ivert<NUM_BOX_VERTS; ivert++) {
		corner[ivert].X = wrld_bbox.Center.X + _boxverts[ivert][0] * wrld_bbox.Extent.X;
		corner[ivert].Y = wrld_bbox.Center.Y + _boxverts[ivert][1] * wrld_bbox.Extent.Y;
		corner[ivert].Z = wrld_bbox.Center.Z + _boxverts[ivert][2] * wrld_bbox.Extent.Z;
		
		if (Camera.Project(corner[ivert],corner[ivert]) == CameraClass::OUTSIDE_NEAR_CLIP) {
			min_v->Set(0,0);
			max_v->Set(1,1);
			return;	
		}
	}
	
	// scan for the min and max
	min_v->X = max_v->X = corner[0].X;
	min_v->Y = max_v->Y = corner[0].Y;
	for (ivert = 1; ivert<NUM_BOX_VERTS; ivert++) {
		if (min_v->X > corner[ivert].X) min_v->X = corner[ivert].X;
		if (min_v->Y > corner[ivert].Y) min_v->Y = corner[ivert].Y;
		if (max_v->X < corner[ivert].X) max_v->X = corner[ivert].X;
		if (max_v->Y < corner[ivert].Y) max_v->Y = corner[ivert].Y;
	}
	return;
}
