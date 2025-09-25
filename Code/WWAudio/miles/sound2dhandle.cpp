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
 *                     $Archive:: /Commando/Code/WWAudio/sound2dhandle.cpp        $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 10/31/01 3:02p                                              $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "sound2dhandle.h"
#include "AudibleSound.h"
#include "wwprofile.h"


//////////////////////////////////////////////////////////////////////
//
//	Sound2DHandleClass
//
//////////////////////////////////////////////////////////////////////
Sound2DHandleClass::Sound2DHandleClass (void)
	: SampleHandle ((HSAMPLE)INVALID_MILES_HANDLE)
{
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	~Sound2DHandleClass
//
//////////////////////////////////////////////////////////////////////
Sound2DHandleClass::~Sound2DHandleClass (void)
{
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Initialize
//
//////////////////////////////////////////////////////////////////////
void
Sound2DHandleClass::Initialize (SoundBufferClass *buffer)
{
	WWPROFILE ("Sound2DHandleClass::Initialize");

	SoundHandleClass::Initialize (buffer);

	if (SampleHandle != (HSAMPLE)INVALID_MILES_HANDLE) {

		//
		// Make sure this handle is fresh
		//
		::AIL_init_sample (SampleHandle);

		//
		// Pass the actual sound data onto the sample
		//
		if (Buffer != NULL) {
			::AIL_set_named_sample_file (SampleHandle, (char *)Buffer->Get_Filename (),
					Buffer->Get_Raw_Buffer (), Buffer->Get_Raw_Length (), 0);
		}
	}
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Start_Sample
//
//////////////////////////////////////////////////////////////////////
void
Sound2DHandleClass::Start_Sample (void)
{
	if (SampleHandle != (HSAMPLE)INVALID_MILES_HANDLE) {
		::AIL_start_sample (SampleHandle);
	}
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Stop_Sample
//
//////////////////////////////////////////////////////////////////////
void
Sound2DHandleClass::Stop_Sample (void)
{
	if (SampleHandle != (HSAMPLE)INVALID_MILES_HANDLE) {
		::AIL_stop_sample (SampleHandle);
	}
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Resume_Sample
//
//////////////////////////////////////////////////////////////////////
void
Sound2DHandleClass::Resume_Sample (void)
{
	if (SampleHandle != (HSAMPLE)INVALID_MILES_HANDLE) {
		::AIL_resume_sample (SampleHandle);
	}
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	End_Sample
//
//////////////////////////////////////////////////////////////////////
void
Sound2DHandleClass::End_Sample (void)
{
	if (SampleHandle != (HSAMPLE)INVALID_MILES_HANDLE) {
		::AIL_end_sample (SampleHandle);
	}
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Sample_Pan
//
//////////////////////////////////////////////////////////////////////
void
Sound2DHandleClass::Set_Sample_Pan (float pan)
{
	if (SampleHandle != (HSAMPLE)INVALID_MILES_HANDLE) {
		::AIL_set_sample_pan (SampleHandle, int(pan * 127.0F));
	}
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Get_Sample_Pan
//
//////////////////////////////////////////////////////////////////////
float
Sound2DHandleClass::Get_Sample_Pan (void)
{
	float retval = 0;

	if (SampleHandle != (HSAMPLE)INVALID_MILES_HANDLE) {
		retval = ::AIL_sample_pan (SampleHandle) / 127.0F;
	}
	return retval;
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Sample_Volume
//
//////////////////////////////////////////////////////////////////////
void
Sound2DHandleClass::Set_Sample_Volume (float volume)
{
	if (SampleHandle != (HSAMPLE)INVALID_MILES_HANDLE) {
		::AIL_set_sample_volume (SampleHandle, int(volume * 127.0F));
	}
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Get_Sample_Volume
//
//////////////////////////////////////////////////////////////////////
float
Sound2DHandleClass::Get_Sample_Volume (void)
{
	float retval = 0;

	if (SampleHandle != (HSAMPLE)INVALID_MILES_HANDLE) {
		retval = ::AIL_sample_volume (SampleHandle) / 127.0F;
	}
	return retval;
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Sample_Loop_Count
//
//////////////////////////////////////////////////////////////////////
void
Sound2DHandleClass::Set_Sample_Loop_Count (unsigned count)
{
	if (SampleHandle != (HSAMPLE)INVALID_MILES_HANDLE) {
		::AIL_set_sample_loop_count (SampleHandle, count);
	}
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Get_Sample_Loop_Count
//
//////////////////////////////////////////////////////////////////////
unsigned
Sound2DHandleClass::Get_Sample_Loop_Count (void)
{
	unsigned retval = 0;

	if (SampleHandle != (HSAMPLE)INVALID_MILES_HANDLE) {
		retval = ::AIL_sample_loop_count (SampleHandle);
	}
	return retval;
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Sample_MS_Position
//
//////////////////////////////////////////////////////////////////////
void
Sound2DHandleClass::Set_Sample_MS_Position (unsigned ms)
{
	if (SampleHandle != (HSAMPLE)INVALID_MILES_HANDLE) {
		::AIL_set_sample_ms_position (SampleHandle, ms);
	}
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Get_Sample_MS_Position
//
//////////////////////////////////////////////////////////////////////
void
Sound2DHandleClass::Get_Sample_MS_Position (int *len, int *pos)
{
	if (SampleHandle != (HSAMPLE)INVALID_MILES_HANDLE) {
		S32 total_ms;
		S32 current_ms;
		::AIL_sample_ms_position (SampleHandle, &total_ms, &current_ms);
		
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
Sound2DHandleClass::Set_Sample_User_Data (int i, void *val)
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
Sound2DHandleClass::Get_Sample_User_Data (int i)
{
	void * retval = nullptr;

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
Sound2DHandleClass::Get_Sample_Playback_Rate (void)
{	
	int retval = 0;

	if (SampleHandle != (HSAMPLE)INVALID_MILES_HANDLE) {
		retval = ::AIL_sample_playback_rate (SampleHandle);
	}
	return retval;
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Sample_Playback_Rate
//
//////////////////////////////////////////////////////////////////////
void
Sound2DHandleClass::Set_Sample_Playback_Rate (int rate)
{
	if (SampleHandle != (HSAMPLE)INVALID_MILES_HANDLE) {
		::AIL_set_sample_playback_rate (SampleHandle, rate);
	}
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Get_Sample_Pitch
//
//////////////////////////////////////////////////////////////////////
float
Sound2DHandleClass::Get_Sample_Pitch (void)
{	
	float retval = 0;

	if (SampleHandle != (HSAMPLE)INVALID_MILES_HANDLE) {
		retval = ::AIL_sample_playback_rate (SampleHandle) / float(Buffer->Get_Rate ());
	}
	return retval;
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Sample_Pitch
//
//////////////////////////////////////////////////////////////////////
void
Sound2DHandleClass::Set_Sample_Pitch (float pitch)
{
	if (SampleHandle != (HSAMPLE)INVALID_MILES_HANDLE) {
		::AIL_set_sample_playback_rate (SampleHandle,  int(Buffer->Get_Rate () * pitch));
	}
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Miles_Handle
//
//////////////////////////////////////////////////////////////////////
void
Sound2DHandleClass::Set_Miles_Handle (void *handle)
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
Sound2DHandleClass::Initialize_Reverb ()
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
