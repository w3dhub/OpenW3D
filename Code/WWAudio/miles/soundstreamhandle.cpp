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
 *                 Project Name : wwaudio                                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/WWAudio/soundstreamhandle.cpp                $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 8/23/01 4:47p                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "soundstreamhandle.h"
#include "AudibleSound.h"
#include "miles/MilesAudio.h"


//////////////////////////////////////////////////////////////////////
//
//	SoundStreamHandleClass
//
//////////////////////////////////////////////////////////////////////
SoundStreamHandleClass::SoundStreamHandleClass (void)
	: SampleHandle ((HSAMPLE)INVALID_MILES_HANDLE),
	StreamHandle ((HSTREAM)INVALID_MILES_HANDLE)
{
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	~SoundStreamHandleClass
//
//////////////////////////////////////////////////////////////////////
SoundStreamHandleClass::~SoundStreamHandleClass (void)
{
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Initialize
//
//////////////////////////////////////////////////////////////////////
void
SoundStreamHandleClass::Initialize (SoundBufferClass *buffer)
{
	SoundHandleClass::Initialize (buffer);

	if (Buffer != NULL) {
		//
		//	Create a stream from the sample handle
		//
		auto inst = reinterpret_cast<MilesAudioClass *>(WWAudioClass::Get_Instance ());
		StreamHandle = ::AIL_open_stream_by_sample (inst->Get_2D_Driver (),
								SampleHandle, buffer->Get_Filename (), 0);
	}

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Start_Sample
//
//////////////////////////////////////////////////////////////////////
void
SoundStreamHandleClass::Start_Sample (void)
{
	if (StreamHandle != (HSTREAM)INVALID_MILES_HANDLE) {
		::AIL_start_stream (StreamHandle);
	}
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Stop_Sample
//
//////////////////////////////////////////////////////////////////////
void
SoundStreamHandleClass::Stop_Sample (void)
{
	if (StreamHandle != (HSTREAM)INVALID_MILES_HANDLE) {
		::AIL_pause_stream (StreamHandle, 1);
	}
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Resume_Sample
//
//////////////////////////////////////////////////////////////////////
void
SoundStreamHandleClass::Resume_Sample (void)
{
	if (StreamHandle != (HSTREAM)INVALID_MILES_HANDLE) {
		::AIL_pause_stream (StreamHandle, 0);
	}
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	End_Sample
//
//////////////////////////////////////////////////////////////////////
void
SoundStreamHandleClass::End_Sample (void)
{
	//
	//	Stop the sample and then release our hold on the stream handle
	//
	Stop_Sample ();

	if (StreamHandle != (HSTREAM)INVALID_MILES_HANDLE) {
		::AIL_close_stream (StreamHandle);
		StreamHandle = (HSTREAM)INVALID_MILES_HANDLE;
	}
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Sample_Pan
//
//////////////////////////////////////////////////////////////////////
void
SoundStreamHandleClass::Set_Sample_Pan (float pan)
{
	if (StreamHandle != (HSTREAM)INVALID_MILES_HANDLE) {
		::AIL_set_stream_pan (StreamHandle, int(pan * 127.0F));
	}
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Get_Sample_Pan
//
//////////////////////////////////////////////////////////////////////
float
SoundStreamHandleClass::Get_Sample_Pan (void)
{
	float retval = 0;

	if (StreamHandle != (HSTREAM)INVALID_MILES_HANDLE) {
		retval = ::AIL_stream_pan (StreamHandle) / 127.0F;
	}

	return retval;
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Sample_Volume
//
//////////////////////////////////////////////////////////////////////
void
SoundStreamHandleClass::Set_Sample_Volume (float volume)
{
	if (StreamHandle != (HSTREAM)INVALID_MILES_HANDLE) {
		::AIL_set_stream_volume (StreamHandle, int(volume * 127.0F));
	}
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Get_Sample_Volume
//
//////////////////////////////////////////////////////////////////////
float
SoundStreamHandleClass::Get_Sample_Volume (void)
{
	float retval = 0;

	if (StreamHandle != (HSTREAM)INVALID_MILES_HANDLE) {
		retval = ::AIL_stream_volume (StreamHandle) / 127.0F;
	}

	return retval;
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Sample_Loop_Count
//
//////////////////////////////////////////////////////////////////////
void
SoundStreamHandleClass::Set_Sample_Loop_Count (unsigned count)
{
	if (StreamHandle != (HSTREAM)INVALID_MILES_HANDLE) {
		::AIL_set_stream_loop_block (StreamHandle, 0, -1);
		::AIL_set_stream_loop_count (StreamHandle, count);
	}
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Get_Sample_Loop_Count
//
//////////////////////////////////////////////////////////////////////
unsigned
SoundStreamHandleClass::Get_Sample_Loop_Count (void)
{
	unsigned retval = 0;

	if (StreamHandle != (HSTREAM)INVALID_MILES_HANDLE) {
		::AIL_stream_loop_count (StreamHandle);
	}

	return retval;
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Sample_MS_Position
//
//////////////////////////////////////////////////////////////////////
void
SoundStreamHandleClass::Set_Sample_MS_Position (unsigned ms)
{
	if (StreamHandle != (HSTREAM)INVALID_MILES_HANDLE) {
		::AIL_set_stream_ms_position (StreamHandle, ms);
	}
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Get_Sample_MS_Position
//
//////////////////////////////////////////////////////////////////////
void
SoundStreamHandleClass::Get_Sample_MS_Position (int *len, int *pos)
{
	if (StreamHandle != (HSTREAM)INVALID_MILES_HANDLE) {
		S32 total_ms;
		S32 current_ms;
		::AIL_stream_ms_position (StreamHandle, &total_ms, &current_ms);
		
		if (len != NULL) {
			*len = int(total_ms);
		}

		if (pos != NULL) {
			*pos = int(current_ms);
		}
	}

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Sample_User_Data
//
//////////////////////////////////////////////////////////////////////
void
SoundStreamHandleClass::Set_Sample_User_Data (int i, void *val)
{
	if (SampleHandle != (HSAMPLE)INVALID_MILES_HANDLE) {
		::AIL_set_sample_user_data (SampleHandle, i, val);
	}

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Get_Sample_User_Data
//
//////////////////////////////////////////////////////////////////////
void *
SoundStreamHandleClass::Get_Sample_User_Data (int i)
{
	void *retval = nullptr;

	if (SampleHandle != (HSAMPLE)INVALID_MILES_HANDLE) {
		retval = ::AIL_sample_user_data (SampleHandle, i);
	}

	return retval;
}


//////////////////////////////////////////////////////////////////////
//
//	Get_Sample_Playback_Rate
//
//////////////////////////////////////////////////////////////////////
int
SoundStreamHandleClass::Get_Sample_Playback_Rate (void)
{	
	int retval = 0;
	
	if (StreamHandle != (HSTREAM)INVALID_MILES_HANDLE) {
		retval = ::AIL_stream_playback_rate (StreamHandle);
	}

	return retval;
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Sample_Playback_Rate
//
//////////////////////////////////////////////////////////////////////
void
SoundStreamHandleClass::Set_Sample_Playback_Rate (int rate)
{
	if (StreamHandle != (HSTREAM)INVALID_MILES_HANDLE) {
		::AIL_set_stream_playback_rate (StreamHandle, rate);
	}

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Get_Sample_Pitch
//
//////////////////////////////////////////////////////////////////////
float
SoundStreamHandleClass::Get_Sample_Pitch (void)
{	
	float retval = 0;
	
	if (StreamHandle != (HSTREAM)INVALID_MILES_HANDLE) {
		retval = ::AIL_stream_playback_rate (StreamHandle) / float(Buffer->Get_Rate ());
	}

	return retval;
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Sample_Pitch
//
//////////////////////////////////////////////////////////////////////
void
SoundStreamHandleClass::Set_Sample_Pitch (float pitch)
{
	if (StreamHandle != (HSTREAM)INVALID_MILES_HANDLE) {
		::AIL_set_stream_playback_rate (StreamHandle, int(Buffer->Get_Rate () * pitch));
	}

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Miles_Handle
//
//////////////////////////////////////////////////////////////////////
void
SoundStreamHandleClass::Set_Miles_Handle (void *handle)
{
	SampleHandle = (HSAMPLE)handle;
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Initialize_Reverb
//
//////////////////////////////////////////////////////////////////////
void
SoundStreamHandleClass::Initialize_Reverb ()
{
	//
	//	Grab the first (and only) filter for use with our 'tinny' effect.
	//
	HPROVIDER filter;
	HPROENUM next = HPROENUM_FIRST;
	char *name = NULL;
	if (::AIL_enumerate_filters (&next, &filter, &name) == 0) {
		//
		//	Pass the filter onto the sample
		//
		::AIL_set_sample_processor (SampleHandle, DP_FILTER, filter);

		//
		//	Change the reverb's settings to simulate a 'tinny' effect.
		//
		F32 reverb_level   = 0.3F;
		F32 reverb_reflect = 0.01F;
		F32 reverb_decay   = 0.535F;
		::AIL_set_filter_sample_preference (SampleHandle,
														"Reverb level",
														&reverb_level);

		::AIL_set_filter_sample_preference (SampleHandle,
														"Reverb reflect time",
														&reverb_reflect);

		::AIL_set_filter_sample_preference (SampleHandle,
														"Reverb decay time",
														&reverb_decay);
	}
	return ;
}
