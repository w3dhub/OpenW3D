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
 *                 Project Name : LevelEdit                                                    *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/HistogramCtl.cpp                                                                                                                                                                                                                                                                                                                               $Modtime:: 7/2/99 6:28p                                               $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "StdAfx.h"
#include "HistogramCtl.H"
#include <algorithm>


/////////////////////////////////////////////////////////////////////////////
//
// HistogramCtlClass
//
/////////////////////////////////////////////////////////////////////////////
HistogramCtlClass::HistogramCtlClass (void)
	:	m_hBitmap (NULL),
		m_pBits (NULL),
		m_ScanlineSize (0),
		m_BMPWidth (0),
		m_BMPHeight (0),
		m_IsDirty (true),
		m_AllowRefresh (true),
		m_LowColor (0, 0.5F, 0),
		m_HighColor (1, 0, 0),
		m_BkLowColor (0.75F, 0.75F, 0.75F),
		m_BkHighColor (0.1F, 0.1F, 0.1F),
		m_SelColor (0, 0, 0),
		m_BkSelColor (1, 1, 1)
{
	m_XAxis.min = 0;
	m_XAxis.max = 1;

	m_YAxis.min = 0;
	m_YAxis.max = 1;

	m_Selection.min = -1;
	m_Selection.max = -1;
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// ~HistogramCtlClass
//
/////////////////////////////////////////////////////////////////////////////
HistogramCtlClass::~HistogramCtlClass (void)
{
	Destroy_DIB_Section ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Create_DIB_Section
//
/////////////////////////////////////////////////////////////////////////////
void
HistogramCtlClass::Create_DIB_Section (void)
{
	// Start fresh
	Destroy_DIB_Section ();

	//
	// Set-up the fields of the BITMAPINFOHEADER
	//	Note: Top-down DIBs use negative height in Win32.
	//
	BITMAPINFOHEADER bitmap_info = { 0 };
	bitmap_info.biSize				= sizeof (BITMAPINFOHEADER);
	bitmap_info.biWidth				= m_BMPWidth;
	bitmap_info.biHeight				= -m_BMPHeight;
	bitmap_info.biPlanes				= 1;
	bitmap_info.biBitCount			= 24;
	bitmap_info.biCompression		= BI_RGB;
	bitmap_info.biSizeImage			= ((m_BMPWidth * m_BMPHeight) * 3);
	bitmap_info.biXPelsPerMeter	= 0;
	bitmap_info.biYPelsPerMeter	= 0;
	bitmap_info.biClrUsed			= 0;
	bitmap_info.biClrImportant		= 0;

	//
	// Create a bitmap that we can access the bits directly of
	//
	HDC hscreen_dc = ::GetDC (NULL);
	m_hBitmap = ::CreateDIBSection (hscreen_dc,
											  (const BITMAPINFO *)&bitmap_info,
											  DIB_RGB_COLORS,
											  (void **)&m_pBits,
											  NULL,
											  0L);

	// Release our temporary screen DC
	::ReleaseDC (NULL, hscreen_dc);

	// Window's bitmaps are DWORD aligned, so make sure
	// we take that into account.
	int alignment_offset = (m_BMPWidth * 3) % 4;
	alignment_offset = (alignment_offset != 0) ? (4 - alignment_offset) : 0;
	m_ScanlineSize = (m_BMPWidth * 3) + alignment_offset;

	// Force a repaint next time its necessary
	Set_Dirty (true);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Destroy_DIB_Section
//
/////////////////////////////////////////////////////////////////////////////
void
HistogramCtlClass::Destroy_DIB_Section (void)
{
	if (m_hBitmap != NULL) {
		::DeleteObject (m_hBitmap);
		m_hBitmap = NULL;
		m_pBits = NULL;
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Set_Dimensions
//
/////////////////////////////////////////////////////////////////////////////
void
HistogramCtlClass::Set_Dimensions (int width, int height)
{
	if ((m_BMPWidth != width) ||
		 (m_BMPHeight != height)) {
		
		//
		//	Recreate the BMP we use
		//
		m_BMPWidth = width;
		m_BMPHeight = height;
		Create_DIB_Section ();
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Paint_DIB
//
/////////////////////////////////////////////////////////////////////////////
void
HistogramCtlClass::Paint_DIB (void)
{
	float x_range		= m_XAxis.max - m_XAxis.min;
	float y_range		= m_YAxis.max - m_YAxis.min;

	int sel_start_x	= int((m_Selection.min / x_range) * (float)(m_BMPWidth-1));
	int sel_end_x		= int((m_Selection.max / x_range) * (float)(m_BMPWidth-1));

	BYTE sel_back_red		= BYTE (m_BkSelColor.X * 255);
	BYTE sel_back_green	= BYTE (m_BkSelColor.Y * 255);
	BYTE sel_back_blue	= BYTE (m_BkSelColor.Z * 255);
	BYTE sel_red			= BYTE (m_SelColor.X * 255);
	BYTE sel_green			= BYTE (m_SelColor.Y * 255);
	BYTE sel_blue			= BYTE (m_SelColor.Z * 255);

	//
	//	Paint the background
	//
	if (m_pBits != NULL) {
		float red	= m_BkHighColor.X;
		float green	= m_BkHighColor.Y;
		float blue	= m_BkHighColor.Z;

		float red_inc		= (m_BkLowColor.X - m_BkHighColor.X) / (float)m_BMPHeight;
		float green_inc	= (m_BkLowColor.Y - m_BkHighColor.Y) / (float)m_BMPHeight;
		float blue_inc		= (m_BkLowColor.Z - m_BkHighColor.Z) / (float)m_BMPHeight;
		
		//
		//	Loop through all the pixels and set their background color.
		//
		int bmp_index = 0;
		int row_offset = m_ScanlineSize - (m_BMPWidth * 3);
		for (int row = 0; row < m_BMPHeight; row ++) {
			BYTE curr_red	= BYTE(red * 255);
			BYTE curr_green = BYTE(green * 255);
			BYTE curr_blue	= BYTE(blue * 255);

			for (int col = 0; col < m_BMPWidth; col ++) {

				if ((col >= sel_start_x) && (col <= sel_end_x)) {
					m_pBits[bmp_index ++] = sel_back_blue;
					m_pBits[bmp_index ++] = sel_back_green;
					m_pBits[bmp_index ++] = sel_back_red;
				} else {
					m_pBits[bmp_index ++] = curr_blue;
					m_pBits[bmp_index ++] = curr_green;
					m_pBits[bmp_index ++] = curr_red;
				}
			}

			red	+= red_inc;
			green	+= green_inc;
			blue	+= blue_inc;
			bmp_index += row_offset;
		}		
	}

	//
	//	Paint the columns if necessary
	//
	if ((m_pBits != NULL) && (m_ValueList.Count () > 0)) {
		int start_index	= Find_Value_Index (m_XAxis.min);
		int end_index		= Find_Value_Index (m_XAxis.max);

		int * x_vector		= new int [m_BMPWidth];
		::memset (x_vector, 0, sizeof (int) * m_BMPWidth);
		
		//
		//	Loop through all the values and combine pixel-duplicates
		//
		for (int index = start_index; index <= end_index; index ++) {
			VALUE &value		= m_ValueList[index];
			int x_pos			= int((value.value / x_range) * (float)(m_BMPWidth-1));
			x_vector[x_pos]	+= value.count;

		}

		float start_red	= m_HighColor.X;
		float start_green	= m_HighColor.Y;
		float start_blue	= m_HighColor.Z;
		float red_inc		= (m_LowColor.X - m_HighColor.X) / (float)m_BMPHeight;
		float green_inc	= (m_LowColor.Y - m_HighColor.Y) / (float)m_BMPHeight;
		float blue_inc		= (m_LowColor.Z - m_HighColor.Z) / (float)m_BMPHeight;

		//
		//	Paint each column of information
		//
		for (int x_pos = 0; x_pos < m_BMPWidth; x_pos ++) {
			if (x_vector[x_pos] > 0) {
				int height	= int(((float)(x_vector[x_pos]) / y_range) * (float)m_BMPHeight);
				height		= std::min (height, m_BMPHeight);
				int y_pos	= m_BMPHeight - height;

				//
				//	Determine the starting red, green, blue
				//
				float curr_red		= (start_red + (y_pos * red_inc));
				float curr_green	= (start_green + (y_pos * green_inc));
				float curr_blue	= (start_blue + (y_pos * blue_inc));

				//
				//	Paint the column
				//
				int bmp_index = (y_pos * m_ScanlineSize) + (x_pos * 3);
				for (int pixel = y_pos; pixel < m_BMPHeight; pixel ++) {
					
					//
					//	Plot the pixel in the BMP (note, the bits are orgainzed
					//	in an BGR format).
					//

					if ((x_pos >= sel_start_x) && (x_pos <= sel_end_x)) {
						m_pBits[bmp_index]		= sel_blue;
						m_pBits[bmp_index + 1]	= sel_green;
						m_pBits[bmp_index + 2]	= sel_red;
					} else {
						m_pBits[bmp_index]		= BYTE(curr_blue * 255);
						m_pBits[bmp_index + 1]	= BYTE(curr_green * 255);
						m_pBits[bmp_index + 2]	= BYTE(curr_red * 255);
					}

					//
					//	Increment the current cocor
					//
					curr_red		+= red_inc;
					curr_green	+= green_inc;
					curr_blue	+= blue_inc;

					// Move down to the next scanline
					bmp_index += m_ScanlineSize;
				}
			}
		}

		delete [] x_vector;
		Set_Dirty (false);
	}
	
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Add_Data_Point
//
/////////////////////////////////////////////////////////////////////////////
void
HistogramCtlClass::Add_Data_Point (float value)
{
	int start		= 0;
	int end			= m_ValueList.Count ();
	end				= std::max (end, 0);

	bool found = false;
	int index = end;
	while (end > start)
	{
		index						= start + ((end - start) >> 1);
		float curr_value		= m_ValueList[index].value;

		if (value > curr_value) {
			if (start == index) { index ++; break; }
			start = index;
		} else if (value < curr_value) {
			if (end == index) { index --; break; }
			end = index;
		} else {
			m_ValueList[index].count ++;
			found = true;
			break;
		}

	}

	ASSERT (index >=0 );

	if (found == false) {
		m_ValueList.Insert (index, VALUE (value, 1));
	}


	/*int start = 0;
	int end = m_ValueList.Count () - 1;

	//
	//	Simple binary search to find the right place
	// to insert the new data point
	//
	bool found = false;
	while ((end > start) && !found) {
		
		int index = start + ((end - start) >> 1);
		VALUE &curr_point = m_ValueList[index];
		if (index == start) {
			break;
		} else if (value > curr_point.value) {
			start = index;
		} else if (value < curr_point.value) {
			end = index;
		} else {
			curr_point.count ++;
			found = true;
		}
	}

	//
	//	If we didn't find the exact point in our search,
	//	then add the point to the list.
	//
	if (found == false) {
		m_ValueList.Insert (start, VALUE (value, 1));
	}*/

	Set_Dirty (true);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Find_Value_Index
//
/////////////////////////////////////////////////////////////////////////////
int
HistogramCtlClass::Find_Value_Index (float value)
{
	//int retval = -1;


	int start		= 0;
	int end			= m_ValueList.Count ();
	end				= std::max (end, 0);

	int index = end;
	while (end > start)
	{
		index						= start + ((end - start) >> 1);
		float curr_value		= m_ValueList[index].value;

		if (value > curr_value) {
			if (start == index) { index ++; break; }
			start = index;
		} else if (value < curr_value) {
			if (end == index) { index --; break; }
			end = index;
		} else {
			
			while ((curr_value == value) && (index > 0)) {
				curr_value = m_ValueList[--index].value;
			}

			index = (curr_value == value) ? index : index + 1;

			//m_ValueList[index].count ++;
			//found = true;
			break;
		}

	}

	ASSERT (index >=0 );

	/*if (found == false) {
		m_ValueList.Insert (index, VALUE (value, 1));
	}*/


	//
	//	Simple binary search to find the right place
	// to insert the new data point
	//
	/*int start	= 0;
	int end		= m_ValueList.Count () - 1;
	while ((end > start) && (retval == -1)) {
		
		int index = start + ((end - start) >> 1);
		VALUE &curr_point = m_ValueList[index];
		if (index == start) {
			retval = (value > curr_point.value) ? end : start;
			break;
		} else if (value > curr_point.value) {			
			start = index;
		} else if (value < curr_point.value) {
			end = index;
		} else {
			retval = index;
		}
	}*/

	/*if (retval == -1) {
		retval = index;
	}*/

	return index;
}


/////////////////////////////////////////////////////////////////////////////
//
// Render
//
/////////////////////////////////////////////////////////////////////////////
void
HistogramCtlClass::Render (HDC hdc, int x_pos, int y_pos)
{
	//
	//	Ensure we have an up-to-date graph
	//
	if (m_IsDirty) {
		Paint_DIB ();
	}

	if (m_hBitmap != NULL) {
		HDC mem_dc			= ::CreateCompatibleDC (NULL);
		HBITMAP old_bmp	= (HBITMAP)::SelectObject (mem_dc, m_hBitmap);

		//
		//	Paint our graph onto the window
		//
		::BitBlt (hdc, x_pos, y_pos, m_BMPWidth, m_BMPHeight, mem_dc, 0, 0, SRCCOPY);

		// Cleanup
		::SelectObject (mem_dc, old_bmp);
		::DeleteDC (mem_dc);
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Reset_Data_Points
//
/////////////////////////////////////////////////////////////////////////////
void
HistogramCtlClass::Reset_Data_Points (void)
{
	m_ValueList.Delete_All ();
	Set_Dirty (true);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Set_Bk_Color
//
/////////////////////////////////////////////////////////////////////////////
void
HistogramCtlClass::Set_Bk_Color
(
	const Vector3 &low,
	const Vector3 &high
)
{
	m_BkLowColor	= low;
	m_BkHighColor	= high;
	Set_Dirty (true);
	return ;
}



/////////////////////////////////////////////////////////////////////////////
//
// Set_Color
//
/////////////////////////////////////////////////////////////////////////////
void
HistogramCtlClass::Set_Color
(
	const Vector3 &low,
	const Vector3 &high
)
{
	m_LowColor	= low;
	m_HighColor	= high;
	Set_Dirty (true);
	return ;
}
