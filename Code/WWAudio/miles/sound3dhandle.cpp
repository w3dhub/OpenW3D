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
 *                     $Archive:: /Commando/Code/WWAudio/sound3dhandle.cpp                    $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 10/31/01 3:00p                                              $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "sound3dhandle.h"
#include "AudibleSound.h"
#include "wwprofile.h"


//////////////////////////////////////////////////////////////////////
//
//	Sound3DHandleClass
//
//////////////////////////////////////////////////////////////////////
Sound3DHandleClass::Sound3DHandleClass (void)
	: SampleHandle ((H3DSAMPLE)INVALID_MILES_HANDLE)
{
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	~Sound3DHandleClass
//
//////////////////////////////////////////////////////////////////////
Sound3DHandleClass::~Sound3DHandleClass (void)
{
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Initialize
//
//////////////////////////////////////////////////////////////////////
void
Sound3DHandleClass::Initialize (SoundBufferClass *buffer)
{
	WWPROFILE ("Sound3DHandleClass::Initialize");

	SoundHandleClass::Initialize (buffer);

	if (SampleHandle != (H3DSAMPLE)INVALID_MILES_HANDLE && Buffer != NULL) {

		//
		//	Configure the 3D sample
		//
		unsigned success = ::AIL_set_3D_sample_file (SampleHandle, Buffer->Get_Raw_Buffer ());

		int test1 = 0;
		int test2 = 0;
		Get_Sample_MS_Position (&test1, &test2);
		
		//
		//	Check for success
		//
		WWASSERT (success != 0);
		if (success == 0) {
			WWDEBUG_SAY (("WWAudio: Couldn't set 3d sample file.  Reason %s\r\n", ::AIL_last_error ()));
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
Sound3DHandleClass::Start_Sample (void)
{
	if (SampleHandle != (H3DSAMPLE)INVALID_MILES_HANDLE) {
		::AIL_start_3D_sample (SampleHandle);
	}
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Stop_Sample
//
//////////////////////////////////////////////////////////////////////
void
Sound3DHandleClass::Stop_Sample (void)
{
	if (SampleHandle != (H3DSAMPLE)INVALID_MILES_HANDLE) {
		::AIL_stop_3D_sample (SampleHandle);
	}
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Resume_Sample
//
//////////////////////////////////////////////////////////////////////
void
Sound3DHandleClass::Resume_Sample (void)
{
	if (SampleHandle != (H3DSAMPLE)INVALID_MILES_HANDLE) {
		::AIL_resume_3D_sample (SampleHandle);
	}
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	End_Sample
//
//////////////////////////////////////////////////////////////////////
void
Sound3DHandleClass::End_Sample (void)
{
	if (SampleHandle != (H3DSAMPLE)INVALID_MILES_HANDLE) {
		::AIL_end_3D_sample (SampleHandle);
	}
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Sample_Pan
//
//////////////////////////////////////////////////////////////////////
void
Sound3DHandleClass::Set_Sample_Pan (float /*pan*/)
{
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Get_Sample_Pan
//
//////////////////////////////////////////////////////////////////////
float
Sound3DHandleClass::Get_Sample_Pan (void)
{
	return 0.5f;
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Sample_Volume
//
//////////////////////////////////////////////////////////////////////
void
Sound3DHandleClass::Set_Sample_Volume (float volume)
{
	if (SampleHandle != (H3DSAMPLE)INVALID_MILES_HANDLE) {
		::AIL_set_3D_sample_volume (SampleHandle, int(volume * 127.0F));
	}
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Get_Sample_Volume
//
//////////////////////////////////////////////////////////////////////
float
Sound3DHandleClass::Get_Sample_Volume (void)
{
	float retval = 0;

	if (SampleHandle != (H3DSAMPLE)INVALID_MILES_HANDLE) {
		retval = ::AIL_3D_sample_volume (SampleHandle) / 127.0F;
	}
	return retval;
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Sample_Loop_Count
//
//////////////////////////////////////////////////////////////////////
void
Sound3DHandleClass::Set_Sample_Loop_Count (unsigned count)
{
	if (SampleHandle != (H3DSAMPLE)INVALID_MILES_HANDLE) {
		::AIL_set_3D_sample_loop_count (SampleHandle, count);
	}
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Get_Sample_Loop_Count
//
//////////////////////////////////////////////////////////////////////
unsigned
Sound3DHandleClass::Get_Sample_Loop_Count (void)
{
	unsigned retval = 0;

	if (SampleHandle != (H3DSAMPLE)INVALID_MILES_HANDLE) {
		retval = ::AIL_3D_sample_loop_count (SampleHandle);
	}
	return retval;
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Sample_MS_Position
//
//////////////////////////////////////////////////////////////////////
void
Sound3DHandleClass::Set_Sample_MS_Position (unsigned ms)
{
	if (SampleHandle != (H3DSAMPLE)INVALID_MILES_HANDLE) {

		WWASSERT (Buffer != NULL);
		unsigned bytes_per_sec = (Buffer->Get_Rate () * Buffer->Get_Bits ()) >> 3;
		unsigned bytes = (ms * bytes_per_sec) / 1000;
		bytes += (bytes & 1);
		::AIL_set_3D_sample_offset (SampleHandle, bytes);
	}
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Get_Sample_MS_Position
//
//////////////////////////////////////////////////////////////////////
void
Sound3DHandleClass::Get_Sample_MS_Position (int *len, int *pos)
{
	if (SampleHandle != (H3DSAMPLE)INVALID_MILES_HANDLE) {

		WWASSERT (Buffer != NULL);
		if (pos != NULL) {
			unsigned bytes = ::AIL_3D_sample_offset (SampleHandle);
			unsigned bytes_per_sec = (Buffer->Get_Rate () * Buffer->Get_Bits ()) >> 3;
			unsigned ms = (bytes * 1000) / bytes_per_sec;
			(*pos) = ms;
		}

		if (len != NULL) {
			unsigned bytes = ::AIL_3D_sample_length (SampleHandle);
			unsigned bytes_per_sec = (Buffer->Get_Rate () * Buffer->Get_Bits ()) >> 3;
			unsigned ms = (bytes * 1000) / bytes_per_sec;
			(*len) = ms;
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
Sound3DHandleClass::Set_Sample_User_Data (int i, void *val)
{
	if (SampleHandle != (H3DSAMPLE)INVALID_MILES_HANDLE) {
		::AIL_set_3D_object_user_data (SampleHandle, i, val);
	}
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Get_Sample_User_Data
//
//////////////////////////////////////////////////////////////////////
void *
Sound3DHandleClass::Get_Sample_User_Data (int i)
{
	void *retval = nullptr;

	if (SampleHandle != (H3DSAMPLE)INVALID_MILES_HANDLE) {
		retval = AIL_3D_object_user_data (SampleHandle, i);
	}
	return retval;
}


//////////////////////////////////////////////////////////////////////
//
//	Get_Sample_Playback_Rate
//
//////////////////////////////////////////////////////////////////////
int
Sound3DHandleClass::Get_Sample_Playback_Rate (void)
{	
	int retval = 0;

	if (SampleHandle != (H3DSAMPLE)INVALID_MILES_HANDLE) {
		retval = ::AIL_3D_sample_playback_rate (SampleHandle);
	}
	return retval;
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Sample_Playback_Rate
//
//////////////////////////////////////////////////////////////////////
void
Sound3DHandleClass::Set_Sample_Playback_Rate (int rate)
{
	if (SampleHandle != (H3DSAMPLE)INVALID_MILES_HANDLE) {
		::AIL_set_3D_sample_playback_rate (SampleHandle, rate);
	}
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Get_Sample_Pitch
//
//////////////////////////////////////////////////////////////////////
float
Sound3DHandleClass::Get_Sample_Pitch (void)
{	
	float retval = 0;

	if (SampleHandle != (H3DSAMPLE)INVALID_MILES_HANDLE) {
		retval = ::AIL_3D_sample_playback_rate (SampleHandle) / float(Buffer->Get_Rate ());
	}
	return retval;
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Sample_Pitch
//
//////////////////////////////////////////////////////////////////////
void
Sound3DHandleClass::Set_Sample_Pitch (float pitch)
{
	if (SampleHandle != (H3DSAMPLE)INVALID_MILES_HANDLE) {
		::AIL_set_3D_sample_playback_rate (SampleHandle, int(Buffer->Get_Rate () * pitch));
	}
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Miles_Handle
//
//////////////////////////////////////////////////////////////////////
void
Sound3DHandleClass::Set_Miles_Handle (void *handle)
{
	WWASSERT (SampleHandle == (H3DSAMPLE)INVALID_MILES_HANDLE);

	SampleHandle = (H3DSAMPLE)handle;
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Position
//
//////////////////////////////////////////////////////////////////////
void Sound3DHandleClass::Set_Position(const Vector3 &position)
{
	::AIL_set_3D_position (SampleHandle, -position.Y, position.Z, position.X);
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Orientation
//
//////////////////////////////////////////////////////////////////////
void Sound3DHandleClass::Set_Orientation(const Vector3 &facing, const Vector3 &up)
{
	::AIL_set_3D_orientation (SampleHandle,
										  -facing.Y,
										  facing.Z,
										  facing.X,
										  -up.Y,
										  up.Z,
										  up.X);
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Velocity
//
//////////////////////////////////////////////////////////////////////
void Sound3DHandleClass::Set_Velocity(const Vector3 &velocity)
{
	::AIL_set_3D_velocity_vector (SampleHandle, -velocity.Y, velocity.Z, velocity.X);
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Dropoff
//
//////////////////////////////////////////////////////////////////////
void Sound3DHandleClass::Set_Dropoff(float max, float min)
{
	::AIL_set_3D_sample_distances (SampleHandle, max, (min > 1.0F) ? min : 1.0F);
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Dropoff
//
//////////////////////////////////////////////////////////////////////
void Sound3DHandleClass::Set_Effect_Level(float level)
{
	::AIL_set_3D_sample_effects_level (SampleHandle, level);
}
