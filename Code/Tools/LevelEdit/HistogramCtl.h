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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/HistogramCtl.h                                                                                                                                                                                                                                                                                                                               $Modtime:: 7/2/99 6:28p                                               $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif


#ifndef __HISTOGRAM_CTL_H
#define __HISTOGRAM_CTL_H

#include "vector.h"
#include "vector3.h"
#include <algorithm>


///////////////////////////////////////////////////////////////////
//
//	HistogramCtlClass
//
///////////////////////////////////////////////////////////////////
class HistogramCtlClass
{
	public:

		//////////////////////////////////////////////////////////////
		//	Public constructors/destructors
		//////////////////////////////////////////////////////////////
		HistogramCtlClass (void);
		virtual ~HistogramCtlClass (void);

		//////////////////////////////////////////////////////////////
		//	Public methods
		//////////////////////////////////////////////////////////////	
		void			Add_Data_Point		(float value);
		void			Reset_Data_Points (void);

		// Selection managment
		void			Get_Selection		(float &range_min, float &range_max);
		void			Set_Selection		(float range_min, float range_max);
		void			Set_Selection		(int min_pixel, int max_pixel);
		void			Clear_Selection	(void);
		
		// Control customization
		void			Set_X_Axis_Range	(float min, float max);
		void			Set_Y_Axis_Range	(float min, float max);	
		void			Set_Dimensions		(int width, int height);
		void			Set_Bk_Color		(const Vector3 &low, const Vector3 &high);
		void			Set_Color			(const Vector3 &low, const Vector3 &high);

		// Painting methods
		bool			Is_Dirty				(void) const { return m_IsDirty; }
		void			Set_Dirty			(bool dirty = true);
		void			Allow_Refresh		(bool onoff = true);
		void			Render				(HDC hdc, int x_pos, int y_pos);

	protected:

		//////////////////////////////////////////////////////////////
		//	Protected methods
		//////////////////////////////////////////////////////////////
		void			Create_DIB_Section (void);
		void			Destroy_DIB_Section (void);
		void			Paint_DIB (void);
		int			Find_Value_Index (float value);
		
	private:

		//////////////////////////////////////////////////////////////
		//	Private data types
		//////////////////////////////////////////////////////////////
		typedef struct
		{
			float min;
			float max;
		} RANGE;

		typedef struct _VALUE
		{
			// Data members
			float		value;
			int		count;

			//	For DynamicVectorClass
			_VALUE (void)								: value (0), count (0)	{ }
			_VALUE (float _value, DWORD _count) : value (_value), count (_count)	{ }
			bool operator== (const _VALUE &/* src*/)			{ return false; }
			bool operator!= (const _VALUE &/* src*/)			{ return true; }
		} VALUE;

		typedef DynamicVectorClass<VALUE> VALUE_LIST;
		
		//////////////////////////////////////////////////////////////
		//	Private member data
		//////////////////////////////////////////////////////////////		
		HBITMAP			m_hBitmap;
		unsigned char *m_pBits;
		int				m_ScanlineSize;
		int				m_BMPWidth;
		int				m_BMPHeight;
		bool				m_IsDirty;
		bool				m_AllowRefresh;
		Vector3			m_BkLowColor;
		Vector3			m_BkHighColor;
		Vector3			m_LowColor;
		Vector3			m_HighColor;
		Vector3			m_SelColor;
		Vector3			m_BkSelColor;

		RANGE				m_XAxis;
		RANGE				m_YAxis;
		RANGE				m_Selection;

		VALUE_LIST		m_ValueList;
};


inline void
HistogramCtlClass::Set_X_Axis_Range	(float min, float max)
{
	m_XAxis.min = min;
	m_XAxis.max = max;
	Set_Dirty (true);
	return ;
}


inline void
HistogramCtlClass::Set_Y_Axis_Range	(float min, float max)
{
	m_YAxis.min = min;
	m_YAxis.max = max;
	Set_Dirty (true);
	return ;
}


inline void
HistogramCtlClass::Clear_Selection	(void)
{
	m_Selection.min = -1;
	m_Selection.max = -1;
	Set_Dirty (true);
	return ;
}


inline void
HistogramCtlClass::Set_Selection	(int min_pixel, int max_pixel)
{
	float x_range	= m_XAxis.max - m_XAxis.min;

	//int start_index	= Find_Value_Index (m_XAxis.min);
	//int end_index		= Find_Value_Index (m_XAxis.max);

	/*float *x_vector	= new float[m_BMPWidth*2];
	::memset (x_vector, 0, sizeof (float) * m_BMPWidth * 2);
	
	if (m_ValueList[)
	//
	//	Loop through all the values and combine pixel-duplicates
	//
	for (int index = start_index; index <= end_index; index ++) {
		VALUE &value		= m_ValueList[index];
		int x_pos			= int((value.value / x_range) * (float)(m_BMPWidth-1));
		x_vector[x_pos]	= std::min (x_vector[x_pos], value.value);
		x_vector[x_pos*2]	= std::max (x_vector[x_pos*2], value.value);
	}	
	
	//float value1	= ((float)(min_pixel * x_range) / ((float)m_BMPWidth));
	//float value2	= ((float)(max_pixel * x_range) / ((float)m_BMPWidth));

	float value1	= x_vector[min_pixel];
	float value2	= x_vector[max_pixel*2];

	delete [] x_vector;*/

	float pixel_range = x_range / (float)m_BMPWidth;
	float left	= min_pixel * pixel_range;
	float right = (max_pixel + 1) * pixel_range;

	float value1	= m_XAxis.min + left;
	float value2	= m_XAxis.min + right;

	m_Selection.min = std::min (value1, value2);
	m_Selection.max = std::max (value1, value2);
	

	/*if (min_pixel != -1) {
		m_Selection.min = std::min (value1, value2);
	}

	if (max_pixel != -1) {
		m_Selection.max = std::max (value1, value2);
	}*/

	/*float real_min = std::min (value1, value2);
	float real_max = std::max (value1, value2);
	m_Selection.min = std::min (m_Selection.min, real_min);
	m_Selection.max = std::max (m_Selection.max, real_max);*/

	Set_Dirty (true);
	return ;
}


inline void
HistogramCtlClass::Set_Selection (float min, float max)
{
	m_Selection.min = min;
	m_Selection.max = max;
	Set_Dirty (true);
	return ;
}


inline void
HistogramCtlClass::Get_Selection (float &min, float &max)
{
	min = m_Selection.min;
	max = m_Selection.max;
	return ;
}


inline void
HistogramCtlClass::Set_Dirty (bool dirty)
{
	m_IsDirty = dirty;
	return ;
}


inline void
HistogramCtlClass::Allow_Refresh	(bool onoff)
{
	m_AllowRefresh = onoff;
	return ;
}


#endif //__HISTOGRAM_CTL_H


