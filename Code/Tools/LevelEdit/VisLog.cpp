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
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/VisLog.cpp                   $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 5/22/01 9:15a                                               $*
 *                                                                                             *
 *                    $Revision:: 5                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "StdAfx.h"
#include "VisLog.H"
#include "Utils.h"
#include "ChunkIO.H"

enum
{

	LOG_CHUNK_VIS							= 0x00000000,
		
		LOG_CHUNK_VIS_HEADER				= 0x00000100,
			LEV_CHUNK_VIS_POINT_INFO	= 0x00000102,
};


typedef struct
{
	uint32	version;
	uint32	count;
	uint32	reserved[4];
} VIS_ERROR_HEADER;

/*typedef struct
{
	Matrix3D	transform;
	float		backface_fraction;
	int		status[6];
	int		direction_bits;
	int		current_direction;
	uint32	Reserved[4];
} VIS_ERROR_INFO;*/

static const uint32 CURRENT_VERSION	= 0x00010000;


//////////////////////////////////////////////////////////////////////////////////
//
//	VisLogClass
//
//////////////////////////////////////////////////////////////////////////////////
VisLogClass::VisLogClass (void)
{
	m_ErrorList.Set_Growth_Step (5000);
	return ;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	~VisLogClass
//
//////////////////////////////////////////////////////////////////////////////////
VisLogClass::~VisLogClass (void)
{
	Reset_Log ();
	return ;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Reset_Log
//
//////////////////////////////////////////////////////////////////////////////////
void
VisLogClass::Reset_Log (void)
{
	//
	//	Delete all the vis samples
	//
	for (int index = 0; index < m_ErrorList.Count (); index ++) {
		VisSampleClass *vis_sample = m_ErrorList[index];
		SAFE_DELETE (vis_sample);
	}

	m_ErrorList.Delete_All ();
	return ;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Log_Sample
//
//////////////////////////////////////////////////////////////////////////////////
void
VisLogClass::Log_Sample (const VisSampleClass &vis_sample)
{
	//
	//	If this sample is either a leak or an overflow, then log it
	// for reference later.
	//
	/*if (	vis_sample.Status == VIS_STATUS_BACKFACE_LEAK ||
			vis_sample.Status == VIS_STATUS_BACKFACE_OVERFLOW ||
			vis_sample.Status == VIS_STATUS_OK) {*/
	
		//
		//	Add this sample to our list
		//
	if (m_ErrorList.Count () < 5000) {
			VisSampleClass *new_sample = new VisSampleClass (vis_sample);
			m_ErrorList.Add (new_sample);
	}
	//}
	
	return ;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Load
//
//////////////////////////////////////////////////////////////////////////////////
bool
VisLogClass::Load (ChunkLoadClass &chunk_load)
{
	bool retval = chunk_load.Open_Chunk ();
	if (retval) {		
		bool close_both = false;

		//
		//	Which chunk is this?
		//
		switch (chunk_load.Cur_Chunk_ID ())
		{
			case LOG_CHUNK_VIS:
				close_both = chunk_load.Open_Chunk ();

			case LOG_CHUNK_VIS_HEADER:
			{
				VIS_ERROR_HEADER header = { 0 };
				retval &= (chunk_load.Read (&header, sizeof (header)) == sizeof (header));

				//
				//	Read the list of points from the chunk
				//
				for (uint32 index = 0; (index < header.count) && retval; index ++) {				
					VisSampleClass sample;
					retval &= sample.Load (chunk_load);
					
					//
					//	Add this sample to the log
					//						
					if (retval) {						
						Log_Sample (sample);
					}
				}
			}
			break;

			default:
				ASSERT (0);
				break;
		}
		
		//
		//	Close the chunks
		//
		chunk_load.Close_Chunk ();
		if (close_both) {
			chunk_load.Close_Chunk ();
		}
	}

	return retval;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Save
//
//////////////////////////////////////////////////////////////////////////////////
bool
VisLogClass::Save (ChunkSaveClass &chunk_save)
{
	bool retval = false;
	
	chunk_save.Begin_Chunk (LOG_CHUNK_VIS);
	chunk_save.Begin_Chunk (LOG_CHUNK_VIS_HEADER);

	//
	//	Write the header out to the chunk
	//
	VIS_ERROR_HEADER header = { 0 };
	header.version = CURRENT_VERSION;
	header.count	= m_ErrorList.Count ();
	retval = (chunk_save.Write (&header, sizeof (header)) == sizeof (header));

	//
	//	Now write each of the points to the chunk
	//
	for (int index = 0; (index < m_ErrorList.Count ()) && retval; index ++) {
		VisSampleClass *sample  = m_ErrorList[index];
		retval &= sample->Save (chunk_save);
	}

	chunk_save.End_Chunk ();
	chunk_save.End_Chunk ();
	return retval;
}

