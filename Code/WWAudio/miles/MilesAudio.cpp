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
 *                 Project Name : WWAudio                                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/WWAudio/WWAudio.cpp                          $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/30/02 2:47p                                               $*
 *                                                                                             *
 *                    $Revision:: 76                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "always.h"
#include "MilesAudio.h"
#include "wwdebug.h"
#include "Utils.h"
#include "realcrc.h"
#include "MilesSoundBuffer.h"
#include "AudibleSound.h"
#include "Sound3D.h"
#include "ww3d.h"
#include "SoundScene.h"
#include "ffactory.h"
#include "Threads.h"
#include "wwmemlog.h"
#include "wwprofile.h"
#include "ini.h"
#include "soundstreamhandle.h"
#include "sound2dhandle.h"
#include "sound3dhandle.h"

const int DEF_CACHE_SIZE			= 1024;

WWAudioClass *WWAudioClass::Create_Instance(bool lite)
{
	return new MilesAudioClass(lite);
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	MilesAudioClass
//
// 1/25/2002 12:13PM ST. Added the 'lite' flag which causes us to go through the motions of
//                       creating the object (including theInstance) so it exists but disables
//                       the call that initialises Miles and doesn't create a sound scene.
//
////////////////////////////////////////////////////////////////////////////////////////////////
MilesAudioClass::MilesAudioClass (bool lite)
	: WWAudioClass(),
		m_Driver2D (NULL),
	  m_Driver3D (NULL),
	  m_SpeakerType (0),
	  m_Driver3DPseudo (NULL),
	  m_MaxCacheSize (DEF_CACHE_SIZE * 1024),
	  m_CurrentCacheSize (0),
	  m_Max2DSamples (DEF_2D_SAMPLE_COUNT),
	  m_Max3DSamples (DEF_3D_SAMPLE_COUNT),
	  m_Max2DBufferSize (DEF_MAX_2D_BUFFER_SIZE),
	  m_Max3DBufferSize (DEF_MAX_3D_BUFFER_SIZE),
	  m_EffectsLevel (0),
	  m_ReverbRoomType (ENVIRONMENT_GENERIC)
{
	m_ForceDisable = lite;

	//
	// Start Miles Sound System
	//
	if (!lite) {
		AIL_startup ();
	}
	
	_theInstance = this;

	m_Max3DBufferSize = m_Max3DBufferSize * 2.0F;

	return;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	~MilesAudioClass
//
////////////////////////////////////////////////////////////////////////////////////////////////
MilesAudioClass::~MilesAudioClass (void)
{
	//
	//	Stop the background music
	//
	Set_Background_Music (NULL);

	//
	//	Make sure the delayed-release thread is terminated
	// before we exit (otherwise the process will crash).
	//
	WWAudioThreadsClass::End_Delayed_Release_Thread ();

	Shutdown ();

	//
	//	Free the list of logical "types".
	//
	Reset_Logical_Types ();

	return;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Flush_Cache
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
MilesAudioClass::Flush_Cache (void)
{
	// Loop through all the hash indicies
	for (int hash_index = 0; hash_index < MAX_CACHE_HASH; hash_index ++) {

		// Loop through all the buffers at this hash index and free them all
		for (int index = 0; index < m_CachedBuffers[hash_index].Count (); index ++) {
			CACHE_ENTRY_STRUCT &info = m_CachedBuffers[hash_index][index];

			// Free the buffer data
			SAFE_FREE (info.string_id);
			REF_PTR_RELEASE (info.buffer);
		}

		// Remove all the entries for this hash index
		m_CachedBuffers[hash_index].Delete_All ();
	}

	m_CurrentCacheSize = 0;
	return;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Open_2D_Device
//
////////////////////////////////////////////////////////////////////////////////////////////////
MilesAudioClass::DRIVER_TYPE_2D
MilesAudioClass::Open_2D_Device (WAVEFORMAT *format)
{
	MMSLockClass lock;

	//
	//	Store the playback settings for future reference
	//
	m_PlaybackRate		= format->nSamplesPerSec;
	m_PlaybackBits		= (format->nAvgBytesPerSec << 3) / (format->nChannels * format->nSamplesPerSec);
	m_PlaybackStereo	= bool(format->nChannels > 1);

	//
	// Assume we will open the DirectSound driver
	//
	DRIVER_TYPE_2D type = DRIVER2D_DSOUND;

	// First close the current 2D device and take
	// all the sound handles away from the sound objects.
	Close_2D_Device ();

	AIL_set_preference (AIL_LOCK_PROTECTION, NO);

	// Try to use DirectSound if possible
	S32 success = ::AIL_set_preference (DIG_USE_WAVEOUT, false);
	//WWASSERT (success == AIL_NO_ERROR);		// This assert fires if there is no sound card.

	// Open the driver
	success = ::AIL_waveOutOpen (&m_Driver2D, NULL, 0, format);

	// Do we need to switch from direct sound to waveout?
	if ((success == AIL_NO_ERROR) &&
		 (m_Driver2D != NULL) &&
		 (m_Driver2D->emulated_ds != 0)) {
		::AIL_waveOutClose (m_Driver2D);
		success = 2;
		WWDEBUG_SAY (("WWAudio: Detected 2D DirectSound emulation, switching to WaveOut.\r\n"));
   }

	// If we couldn't open the direct sound device, then use the
	// default wave out device
	if (success != AIL_NO_ERROR) {

		// Try to use the default wave out driver
		success = ::AIL_set_preference (DIG_USE_WAVEOUT, true);
		//WWASSERT (success == AIL_NO_ERROR);	// This assert fires if there is no sound card.

		// Open the driver
		success = ::AIL_waveOutOpen (&m_Driver2D, NULL, 0, reinterpret_cast<LPWAVEFORMAT>(format));
		type = (success == AIL_NO_ERROR) ? DRIVER2D_WAVEOUT : DRIVER2D_ERROR;
	}

	// Allocate all the available handles if we were successful
	if (success == AIL_NO_ERROR) {
		Allocate_2D_Handles ();
		ReAssign_2D_Handles ();
	} else {
		Close_2D_Device ();
		WWDEBUG_SAY (("WWAudio: Error initializing 2D device.\r\n"));
	}

	// Return the opened device type
	return type;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Open_2D_Device
//
////////////////////////////////////////////////////////////////////////////////////////////////
MilesAudioClass::DRIVER_TYPE_2D
MilesAudioClass::Open_2D_Device
(
	bool stereo,
	int bits,
	int hertz
)
{
	// Build a wave format structure from the params
	PCMWAVEFORMAT wave_format = { 0 };
	wave_format.wf.wFormatTag = WAVE_FORMAT_PCM;
	wave_format.wf.nChannels = stereo ? 2 : 1;
	wave_format.wf.nSamplesPerSec = hertz;
	wave_format.wf.nAvgBytesPerSec = (wave_format.wf.nChannels * wave_format.wf.nSamplesPerSec * bits) >> 3;
	wave_format.wf.nBlockAlign = (wave_format.wf.nChannels * bits) >> 3;
	wave_format.wBitsPerSample = bits;
	DRIVER_TYPE_2D type = DRIVER2D_ERROR;

	while (((type = Open_2D_Device (&wave_format.wf)) == DRIVER2D_ERROR) &&
			 (wave_format.wf.nSamplesPerSec >= 11025)) {

		//
		//	Cut the playback rate in half and try again
		//
		wave_format.wf.nSamplesPerSec = wave_format.wf.nSamplesPerSec >> 1;
		wave_format.wf.nAvgBytesPerSec = (wave_format.wf.nChannels * wave_format.wf.nSamplesPerSec * bits) >> 3;
		wave_format.wf.nBlockAlign = (wave_format.wf.nChannels * bits) >> 3;
	}

	// Pass this structure onto the function that actually opens the device
	return type;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Close_2D_Device
//
////////////////////////////////////////////////////////////////////////////////////////////////
bool
MilesAudioClass::Close_2D_Device (void)
{
	MMSLockClass lock;

	//
	//	Note:  We MUST close the 3D device when we close the 2D device...
	//
	Close_3D_Device ();

	//
	//	Free any 2D sound handles
	//
	Remove_2D_Sound_Handles ();
	Release_2D_Handles ();

	//
	// Do we have an open driver handle to close?
	//
	bool retval = false;
	if (m_Driver2D != NULL) {

		//
		// Close the driver
		//
		::AIL_waveOutClose (m_Driver2D);
		m_Driver2D = NULL;
		retval = true;
	}

	return retval;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Close_3D_Device
//
////////////////////////////////////////////////////////////////////////////////////////////////
bool
MilesAudioClass::Close_3D_Device (void)
{
	MMSLockClass lock;

	bool retval = false;

	//
	//	Remove all the handles
	//
	Remove_3D_Sound_Handles ();
	Release_3D_Handles ();

	//
	// Do we have an open driver handle to close?
	//
	if (m_Driver3D != NULL) {
		::AIL_close_3D_provider (m_Driver3D);
		m_Driver3D = NULL;
		retval = true;
	}

	return retval;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Get_Sound_Buffer
//
////////////////////////////////////////////////////////////////////////////////////////////////
SoundBufferClass *
MilesAudioClass::Get_Sound_Buffer (const char *filename, bool is_3d)
{
	WWPROFILE ("Get_Sound_Buffer");

	//
	// Try to find the buffer in our cache, otherwise create a new buffer.
	//
	SoundBufferClass *buffer = Find_Cached_Buffer (filename);
	if (buffer == NULL) {
		FileClass *file = Get_File (filename);
		if (file != NULL && file->Is_Available ()) {
			buffer = Create_Sound_Buffer (*file, filename, is_3d);
		} else {
			static int count = 0;
			if ( ++count < 10 ) {
				WWDEBUG_SAY(( "Sound \"%s\" not found\r\n", filename ));
			}
		}
		Return_File (file);
	}

	return buffer;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Find_Cached_Buffer
//
////////////////////////////////////////////////////////////////////////////////////////////////
SoundBufferClass *
MilesAudioClass::Find_Cached_Buffer (const char *string_id)
{
	WWPROFILE ("Find_Cached_Buffer");

	SoundBufferClass *sound_buffer = NULL;

	// Param OK?
	WWASSERT (string_id != NULL);
	if (string_id != NULL) {

		//
		// Determine which index in our hash table to use
		//
		int hash_index = ::CRC_Stringi (string_id) & CACHE_HASH_MASK;

		//
		// Loop through all the buffers at this hash index and try to find
		// one that matches the requested name
		//
		for (int index = 0; index < m_CachedBuffers[hash_index].Count (); index ++) {

			//
			// Is this the sound buffer we were looking for?
			//
			CACHE_ENTRY_STRUCT &info = m_CachedBuffers[hash_index][index];
			if (::stricmp (info.string_id, string_id) == 0) {
				sound_buffer = info.buffer;
				sound_buffer->Add_Ref ();
				break;
			}
		}
	}

	//
	// Return a pointer to the cached sound buffer
	//
	return sound_buffer;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Free_Cache_Space
//
////////////////////////////////////////////////////////////////////////////////////////////////
bool
MilesAudioClass::Free_Cache_Space (int bytes)
{
	int bytes_freed = 0;

	// Loop through all the hash indicies
	for (int hash_index = 0;
		  (hash_index < MAX_CACHE_HASH) && (bytes_freed < bytes);
		  hash_index ++) {

		// Loop through all the buffers at this hash index
		for (int index = 0;
			  (index < m_CachedBuffers[hash_index].Count ()) && (bytes_freed < bytes);
			  index ++) {

			// Can we free this cached buffer?
			CACHE_ENTRY_STRUCT &info = m_CachedBuffers[hash_index][index];
			if ((info.buffer != NULL) && (info.buffer->Num_Refs () == 1)) {

				// Add the size of this buffer to our count of bytes freed
				bytes_freed += info.buffer->Get_Raw_Length ();

				// Free the buffer data
				SAFE_FREE (info.string_id);
				REF_PTR_RELEASE (info.buffer);

				// Remove this entry from the hash table
				m_CachedBuffers[hash_index].Delete (index);
				index --;
			}
		}
	}

	// Make sure to recompute out current cache size
	m_CurrentCacheSize -= bytes_freed;
	WWASSERT (m_CurrentCacheSize >= 0);

	// Return true if we freed enough bytes in the cache
	return (bytes_freed >= bytes);
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Cache_Buffer
//
////////////////////////////////////////////////////////////////////////////////////////////////
bool
MilesAudioClass::Cache_Buffer
(
	SoundBufferClass *buffer,
	const char *string_id
)
{
	WWPROFILE ("Cache_Buffer");

	// Assume failure
	bool retval = false;

	// Params OK?
	WWASSERT (buffer != NULL);
	WWASSERT (string_id != NULL);
	if ((buffer != NULL) && (string_id != NULL)) {

		//
		// Attempt to free space in the cache (if needed)
		//
		/*int space_needed = (m_CurrentCacheSize + buffer->Get_Raw_Length ()) - (int)m_MaxCacheSize;
		if (space_needed > 0) {
			Free_Cache_Space (space_needed);
		}*/

		// Do we have enough space in the cache for this buffer?
		//space_needed = (m_CurrentCacheSize + buffer->Get_Raw_Length ()) - (int)m_MaxCacheSize;
		//if (space_needed <= 0) {

			//
			// Determine which index in our hash table to use
			//
			int hash_index = ::CRC_Stringi (string_id) & CACHE_HASH_MASK;

			//
			// Add this buffer to the hash table at the given index.
			//	Note:  The assignment operator caused by the Add call
			//			will add a reference to the sound buffer.
			//
			CACHE_ENTRY_STRUCT info;
			info.string_id = (char *)string_id;
			info.buffer = buffer;
			m_CachedBuffers[hash_index].Add (info);

			// Update our current cache size
			m_CurrentCacheSize += buffer->Get_Raw_Length ();
			retval = true;
		//}
	}

	if (!retval) {
		WWDEBUG_SAY (("Unable to cache sound: %s.\r\n", string_id));
	}

	// Return the true/false result code
	return retval;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Create_Sound_Buffer
//
////////////////////////////////////////////////////////////////////////////////////////////////
SoundBufferClass *
MilesAudioClass::Create_Sound_Buffer
(
	FileClass &	file,
	const char *string_id,
	bool			is_3d
)
{
	WWPROFILE ("Create_Sound_Buffer");

	SoundBufferClass *sound_buffer = NULL;

	//
	//	Determine how large this buffer can be
	//
	int max_size = is_3d ? m_Max3DBufferSize : m_Max2DBufferSize;

	//
	// Create a streaming sound buffer object if the
	// file is too large to preload.
	//
	if (file.Size () > max_size) {
		sound_buffer = new StreamSoundBufferClass;
	} else {
		sound_buffer = new SingleSoundBufferClass;
	}
	SET_REF_OWNER(sound_buffer);

	//
	// Create a new sound buffer from the provided file
	//
	bool success = sound_buffer->Load_From_File (file);
	sound_buffer->Set_Filename (string_id);
	WWASSERT (success);

	// If we were successful in creating the sound buffer, then
	// try to cache it as well, otherwise free the buffer and return NULL.
	if (success && (string_id != NULL)) {
		Cache_Buffer (sound_buffer, string_id);
	} else if (!success) {
		REF_PTR_RELEASE (sound_buffer);
	}

	// Return a pointer to the new sound buffer
	return sound_buffer;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Release_2D_Handles
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
MilesAudioClass::Release_2D_Handles (void)
{
	MMSLockClass lock;

	// Release our hold on all the samples we've allocated
	for (int index = 0; index < m_2DSampleHandles.Count (); index ++) {
		HSAMPLE sample = m_2DSampleHandles[index];
		if (sample != NULL) {
			::AIL_release_sample_handle (sample);
		}
	}

	m_2DSampleHandles.Delete_All ();
	return;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Allocate_2D_Handles
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
MilesAudioClass::Allocate_2D_Handles (void)
{
	MMSLockClass lock;

	// Start fresh
	Release_2D_Handles ();

	if (m_Driver2D != NULL) {

		// Attempt to allocate our share of 2D sample handles
		for (int index = 0; index < m_Max2DSamples; index ++) {
			HSAMPLE sample = ::AIL_allocate_sample_handle (m_Driver2D);
			if (sample != NULL) {
				::AIL_set_sample_user_data (sample, INFO_OBJECT_PTR, NULL);
				m_2DSampleHandles.Add (sample);
			}
		}

		// Record our actual number of available 2D sample handles
		m_Max2DSamples = m_2DSampleHandles.Count ();
	}

	return;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Get_2D_Sample
//
////////////////////////////////////////////////////////////////////////////////////////////
SoundHandleClass *
MilesAudioClass::Get_2D_Handle (const AudibleSoundClass &sound_obj, bool streaming)
{
	if (!WWAudioClass::Is_OK_To_Give_Handle(sound_obj)) {
		return NULL;
	}

	MMSLockClass lock;

	float lowest_priority					= sound_obj.Get_Priority ();
	float lowest_runtime_priority			= sound_obj.Get_Runtime_Priority ();
	AudibleSoundClass *lowest_pri_sound = NULL;
	HSAMPLE lowest_pri_sample				= NULL;
	HSAMPLE free_sample						= (HSAMPLE)INVALID_MILES_HANDLE;

	// Loop through all the available sample handles and try to find
	// one that isn't being used to play a sound.
	bool found = false;
	for (int index = 0; (index < m_2DSampleHandles.Count ()) && !found; index ++) {

		HSAMPLE sample = m_2DSampleHandles[index];
		if (sample != NULL) {

			// Get a pointer to the object that is currently using this sample
			AudibleSoundClass *sample_obj = (AudibleSoundClass *)::AIL_sample_user_data (sample, INFO_OBJECT_PTR);
			if (sample_obj == NULL) {

				// Return this sample handle to the caller
				free_sample = sample;
				found = true;
			} else {

				//
				//	Determine if this sound's priority is lesser then the sound we want to play.
				// This is done by comparing both the designer-specified priority and the current
				// runtime priority (which is calculated by distance to the listener).
				//
				float priority				= sample_obj->Get_Priority ();
				float runtime_priority	= sample_obj->Get_Runtime_Priority ();
				if (	(priority < lowest_priority) ||
						(priority == lowest_priority && runtime_priority <= lowest_runtime_priority))
				{
					lowest_priority			= priority;
					lowest_pri_sound			= sample_obj;
					lowest_pri_sample			= sample;
					lowest_runtime_priority = runtime_priority;
				}
			}
		}
	}

	// Steal the sample handle from the lower priority
	// sound and return the handle to the caller.
	if ((!found) && (lowest_pri_sound != NULL)) {
		lowest_pri_sound->Free_Miles_Handle ();
		free_sample = lowest_pri_sample;
	}

	if (free_sample == (HSAMPLE)INVALID_MILES_HANDLE)
	{
		return NULL;
	}

	SoundHandleClass *retval = NULL;

	if (streaming) {
		retval = new SoundStreamHandleClass;
	} else {
		retval = new Sound2DHandleClass;
	}

	retval->Set_Miles_Handle(free_sample);

	// Return the free sample handle if we found one
	return retval;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Get_3D_Sample
//
////////////////////////////////////////////////////////////////////////////////////////////////
SoundHandleClass *
MilesAudioClass::Get_3D_Handle (const Sound3DClass &sound_obj)
{
	if (!WWAudioClass::Is_OK_To_Give_Handle(sound_obj)) {
		return NULL;
	}

	MMSLockClass lock;

	float lowest_priority					= sound_obj.Get_Priority ();
	float lowest_runtime_priority			= sound_obj.Get_Runtime_Priority ();
	AudibleSoundClass *lowest_pri_sound = NULL;
	H3DSAMPLE lowest_pri_sample			= NULL;
	H3DSAMPLE free_sample					= (H3DSAMPLE)INVALID_MILES_HANDLE;


	// Loop through all the available sample handles and try to find
	// one that isn't being used to play a sound.
	bool found = false;
	for (int index = 0; (index < m_3DSampleHandles.Count ()) && !found; index ++) {

		H3DSAMPLE sample = m_3DSampleHandles[index];
		if (sample != NULL) {

			// Get a pointer to the object that is currently using this sample
			AudibleSoundClass *sample_obj = (AudibleSoundClass *)::AIL_3D_object_user_data (sample, INFO_OBJECT_PTR);
			if (sample_obj == NULL) {

				// Return this sample handle to the caller
				free_sample = sample;
				found = true;
			} else {

				//
				//	Determine if this sound's priority is lesser then the sound we want to play.
				// This is done by comparing both the designer-specified priority and the current
				// runtime priority (which is calculated by distance to the listener).
				//
				float priority				= sample_obj->Get_Priority ();
				float runtime_priority	= sample_obj->Get_Runtime_Priority ();
				if (	(priority < lowest_priority) ||
						(priority == lowest_priority && runtime_priority <= lowest_runtime_priority))
				{
					lowest_priority			= priority;
					lowest_pri_sound			= sample_obj;
					lowest_pri_sample			= sample;
					lowest_runtime_priority = runtime_priority;
				}
			}
		}
	}

	// Steal the sample handle from the lower priority
	// sound and return the handle to the caller.
	if ((!found) && (lowest_pri_sound != NULL)) {
		lowest_pri_sound->Free_Miles_Handle ();
		free_sample = lowest_pri_sample;
	}

	if (free_sample == (H3DSAMPLE)INVALID_MILES_HANDLE)
	{
		return NULL;
	}

	SoundHandleClass *retval = new Sound3DHandleClass;
	retval->Set_Miles_Handle(free_sample);

	// Return the free sample handle if we found one
	return retval;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Build_3D_Driver_List
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
MilesAudioClass::Build_3D_Driver_List (void)
{
	MMSLockClass lock;

	HPROENUM next = HPROENUM_FIRST;
	HPROVIDER provider = NULL;
	char *name = NULL;
	while (::AIL_enumerate_3D_providers (&next, &provider, &name) > 0) {

		// Can we successfully open this provider?
		if (::AIL_open_3D_provider (provider) == M3D_NOERR) {
			DRIVER_INFO_STRUCT *info = new DRIVER_INFO_STRUCT;
			info->driver = provider;
			info->name = ::strdup (name);
			m_Driver3DList.Add (info);
			::AIL_close_3D_provider (provider);
		} else {
			[[maybe_unused]] char *error_info = ::AIL_last_error ();
			WWDEBUG_SAY (("WWAudio: Unable to open %s.\r\n", name));
			WWDEBUG_SAY (("WWAudio: Reason %s.\r\n", error_info));
		}
	}

	//
	// Attempt to select one of the known drivers (in the following order).
	//
	if (	(!Select_3D_Device (DRIVER3D_PSEUDO)) &&
			(!Select_3D_Device (DRIVER3D_EAX)) &&
			(!Select_3D_Device (DRIVER3D_A3D)) &&
			(!Select_3D_Device (DRIVER3D_D3DSOUND)) &&
			(!Select_3D_Device (DRIVER3D_DOLBY)))
	{
		//
		// Couldn't select a known driver, so just use the first possible.
		//
		if (m_Driver3DList.Count () > 0) {
			Select_3D_Device (0);
		}
	}

	return;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Free_3D_Driver_List
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
MilesAudioClass::Free_3D_Driver_List (void)
{
	MMSLockClass lock;

	//
	//	Remove all the handles
	//
	Remove_3D_Sound_Handles ();
	Release_3D_Handles ();

	//
	// Loop through all the driver entries and free them all
	//
	for (int index = 0; index < m_Driver3DList.Count (); index ++) {
		DRIVER_INFO_STRUCT *info = m_Driver3DList[index];
		if (info != NULL) {

			//
			// Free the information we have stored with this driver
			//
			if (info->name != NULL) {
				::free (info->name);
			}
			delete info;
		}
	}

	if (m_Driver3D != NULL) {
		::AIL_close_3D_provider (m_Driver3D);
		m_Driver3D = NULL;
	}

	//
	// Clear the list
	//
	m_Driver3DList.Delete_All ();
	return;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Select_3D_Device
//
////////////////////////////////////////////////////////////////////////////////////////////////
bool
MilesAudioClass::Select_3D_Device (const char *device_name)
{
	bool retval = false;

	//
	// Loop through all the drivers until we've found the one we want
	//
	for (int index = 0; index < m_Driver3DList.Count (); index ++) {
		DRIVER_INFO_STRUCT *info = m_Driver3DList[index];
		if (info != NULL) {

			//
			//	Is this the device we were looking for?
			//
			if (::stricmp (info->name, device_name) == 0) {
				retval = Select_3D_Device (device_name, info->driver);
				break;
			}
		}
	}

	return retval;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Select_3D_Device
//
////////////////////////////////////////////////////////////////////////////////////////////////
bool
MilesAudioClass::Select_3D_Device (const char *device_name, HPROVIDER provider)
{
	bool retval = false;
	if ((provider != NULL) && (provider != m_Driver3D)) {

		Close_3D_Device ();

		//
		// Select this device and re-allocate all handles
		//
		if (::AIL_open_3D_provider (provider) == M3D_NOERR) {
			m_Driver3D = provider;
			m_SoundScene->Initialize ();
			Allocate_3D_Handles ();
			AIL_set_3D_speaker_type (provider, AIL_3D_2_SPEAKER);

			//
			//	Adjust the effects level to 1.0 if this is an EAX based driver
			//
			StringClass lower_name = device_name;
			lower_name.To_Lower();
			if (::strstr (device_name, "eax") != 0) {
				m_EffectsLevel = 1.0F;
			} else {
				m_EffectsLevel = 0.0F;
			}

			m_Driver3DName = device_name;
		}

		retval = true;
	}

	// Return true if we successfully selected the device
	return retval;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Select_3D_Device
//
////////////////////////////////////////////////////////////////////////////////////////////////
bool
MilesAudioClass::Select_3D_Device (int index)
{
	bool retval = false;

	//
	// Index valid?
	//
	if ((index >= 0) && (index < m_Driver3DList.Count ())) {
		Select_3D_Device (m_Driver3DList[index]->name, m_Driver3DList[index]->driver);
		WWDEBUG_SAY (("WWAudio: Selecting 3D sound device: %s.\r\n", m_Driver3DList[index]->name));
		retval = true;
	}

	//
	// Return true if we successfully selected the device
	//
	return retval;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Select_3D_Device
//
////////////////////////////////////////////////////////////////////////////////////////////////
bool
MilesAudioClass::Select_3D_Device (DRIVER_TYPE_3D type)
{
	// Return true if we successfully selected the device
	return Select_3D_Device (Find_3D_Device (type));
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Find_3D_Device
//
////////////////////////////////////////////////////////////////////////////////////////////////
int
MilesAudioClass::Find_3D_Device (DRIVER_TYPE_3D type)
{
	// Determine which substring to search for in the
	// name of the driver.
	const char *sub_string = "RSX";
	switch (type) {
		case DRIVER3D_D3DSOUND:
			sub_string = "DirectSound";
			break;

		case DRIVER3D_EAX:
			sub_string = "EAX";
			break;

		case DRIVER3D_A3D:
			sub_string = "A3D";
			break;

		case DRIVER3D_PSEUDO:
			sub_string = "Fast";
			break;

		case DRIVER3D_DOLBY:
			sub_string = "Dolby";
			break;
	}

	// Loop through all the driver entries and free them all
	int driver_index = -1;
	for (int index = 0; (index < m_Driver3DList.Count ()) && (driver_index == -1); index ++) {
		DRIVER_INFO_STRUCT *info = m_Driver3DList[index];
		if ((info != NULL) && (info->name != NULL)) {

			// Is this the driver we were looking for?
			if (::strstr (info->name, sub_string) != NULL) {
				driver_index = index;
			}
		}
	}

	// Return -1 if not found, otherwise the 0 based index
	return driver_index;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Allocate_3D_Handles
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
MilesAudioClass::Allocate_3D_Handles (void)
{
	MMSLockClass lock;

	// Start fresh
	Release_3D_Handles ();

	if (m_Driver3D != NULL) {

		// Attempt to allocate our share of 3D sample handles
		for (int index = 0; index < m_Max3DSamples; index ++) {
			H3DSAMPLE sample = ::AIL_allocate_3D_sample_handle (m_Driver3D);
			if (sample != NULL) {
				::AIL_set_3D_object_user_data (sample, INFO_OBJECT_PTR, NULL);
				m_3DSampleHandles.Add (sample);
			}
		}
	}

	return;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Release_3D_Handles
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
MilesAudioClass::Release_3D_Handles (void)
{
	MMSLockClass lock;

	//
	// Release our hold on all the samples we've allocated
	//
	for (int index = 0; index < m_3DSampleHandles.Count (); index ++) {
		H3DSAMPLE sample = m_3DSampleHandles[index];
		if (sample != NULL) {
			::AIL_release_3D_sample_handle (sample);
		}
	}

	m_3DSampleHandles.Delete_All ();
	return;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Validate_3D_Sound_Buffer
//
////////////////////////////////////////////////////////////////////////////////////////////////
bool
MilesAudioClass::Validate_3D_Sound_Buffer (SoundBufferClass *buffer)
{
	bool retval = false;

	//
	// 3D sound buffer MUST be uncompressed mono WAV data
	//
	if ((buffer != NULL) &&
		 (buffer->Get_Channels () == 1) &&
		 (buffer->Get_Type () == WAVE_FORMAT_PCM) &&
		 (!buffer->Is_Streaming ()))
	{
		retval = true;
	}

	// Return a true/false result code
	return retval;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	ReAssign_2D_Handles
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
MilesAudioClass::ReAssign_2D_Handles (void)
{
	// Loop through all the entries in the playlist
	for (int index = 0; index < m_Playlist[m_CurrPage].Count (); index ++) {
		AudibleSoundClass *sound_obj = m_Playlist[m_CurrPage][index];

		// If this is a 2D sound effect, then force it to 'get' a new
		// sound handle.
		if ((sound_obj->Get_Class_ID () == CLASSID_2D) ||
			 (sound_obj->Get_Class_ID () == CLASSID_PSEUDO3D) ||
			 (sound_obj->Get_Class_ID () == CLASSID_2DTRIGGER))
		{
			sound_obj->Free_Miles_Handle ();
			sound_obj->Allocate_Miles_Handle ();
		}
	}

	return;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	ReAssign_3D_Handles
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
MilesAudioClass::ReAssign_3D_Handles (void)
{
	// Loop through all the entries in the playlist
	for (int index = 0; index < m_Playlist[m_CurrPage].Count (); index ++) {
		AudibleSoundClass *sound_obj = m_Playlist[m_CurrPage][index];

		// If this is a 3D sound effect, then force it to 'get' a new
		// sound handle.
		if (sound_obj->Get_Class_ID () == CLASSID_3D) {
			sound_obj->Free_Miles_Handle ();
			sound_obj->Allocate_Miles_Handle ();
		}
	}

	return;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Remove_2D_Sound_Handles
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
MilesAudioClass::Remove_2D_Sound_Handles (void)
{
	//
	//	Loop over all the 2D handles
	//
	for (int index = 0; index < m_2DSampleHandles.Count (); index ++) {
		HSAMPLE sample = m_2DSampleHandles[index];
		if (sample != NULL) {

			//
			// Get a pointer to the object that is currently using this sample
			//
			AudibleSoundClass *sound_obj = (AudibleSoundClass *)::AIL_sample_user_data (sample, INFO_OBJECT_PTR);
			if (sound_obj != NULL) {
				sound_obj->Free_Miles_Handle ();
			}
		}
	}

	return;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Remove_3D_Sound_Handles
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
MilesAudioClass::Remove_3D_Sound_Handles (void)
{
	//
	//	Loop over all the 3D handles
	//
	for (int index = 0; index < m_3DSampleHandles.Count (); index ++) {
		H3DSAMPLE sample = m_3DSampleHandles[index];
		if (sample != NULL) {

			//
			// Get a pointer to the object that is currently using this sample
			//
			AudibleSoundClass *sound_obj = (AudibleSoundClass *)::AIL_3D_object_user_data (sample, INFO_OBJECT_PTR);
			if (sound_obj != NULL) {
				sound_obj->Free_Miles_Handle ();
			}
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Initialize
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
MilesAudioClass::Initialize (const char *registry_subkey_name)
{
	WWMEMLOG(MEM_SOUND);

	if (!Is_Disabled ()) {

		//
		//	Initialize the audio system from the registry settings
		//
		Load_From_Registry (registry_subkey_name);

		m_RealMusicVolume = m_MusicVolume;
		m_RealSoundVolume = m_SoundVolume;
	}

	//
	//	Register the file callbacks so we can support streaming from MIX files...
	//
	::AIL_set_file_callbacks (File_Open_Callback, File_Close_Callback,
		File_Seek_Callback, File_Read_Callback);
	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Initialize
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
MilesAudioClass::Initialize
(
	bool stereo,
	int bits,
	int hertz
)
{
	// Open the default 2D device, then build a list of 3D
	// devices and open the default.
	if (!Is_Disabled ()) {

		Open_2D_Device (stereo, bits, hertz);
		Build_3D_Driver_List ();
	}

	//
	//	Register the file callbacks so we can support streaming from MIX files...
	//
	::AIL_set_file_callbacks (File_Open_Callback, File_Close_Callback,
		File_Seek_Callback, File_Read_Callback);

	return;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Shutdown
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
MilesAudioClass::Shutdown (void)
{
	//
	//	Stop the background music
	//
	Set_Background_Music (NULL);

	//
	// Stop all sounds from playing
	//
	Flush_Playlist ();
	if (m_SoundScene != NULL) {
		m_SoundScene->Flush_Scene ();
	}

	//
	// Free all our cached sound buffers
	//
	Flush_Cache ();

	//
	// Close-out our hold on any driver resources
	//
	Remove_2D_Sound_Handles ();
	Remove_3D_Sound_Handles ();
	Release_2D_Handles ();
	Release_3D_Handles ();
	Free_3D_Driver_List ();
	SAFE_DELETE (m_SoundScene);
	Close_2D_Device ();

	//
	// Shutdown Miles Sound System
	//
	::AIL_shutdown ();
	return;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Get_File
//
////////////////////////////////////////////////////////////////////////////////////////////////
FileClass *
MilesAudioClass::Get_File (const char* filename)
{
	FileClass *file = NULL;
	if (m_FileFactory != NULL) {
		file = m_FileFactory->Get_File (filename);
	} else {
		file = _TheFileFactory->Get_File(filename);
	}

	// Return a pointer to the file
	return file;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Return_File
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
MilesAudioClass::Return_File (FileClass *file)
{
	if (m_FileFactory != NULL) {
		m_FileFactory->Return_File (file);
	} else {
		SAFE_DELETE (file);
	}

	return;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Load_From_Registry
//
////////////////////////////////////////////////////////////////////////////////////////////
bool
MilesAudioClass::Load_From_Registry (const char *subkey_name)
{
	bool retval = true;

	StringClass device_name;
	bool is_stereo = true;
	int bits = 16;
	int hertz = 44100;

	//
	//	Load the settings from the registry
	//
	if (WWAudioClass::Load_From_Registry (subkey_name, device_name, is_stereo, bits, hertz,
			m_AreSoundEffectsEnabled, m_IsMusicEnabled, m_IsDialogEnabled, m_IsCinematicSoundEnabled,
			m_SoundVolume, m_MusicVolume, m_DialogVolume, m_CinematicVolume, m_SpeakerType))
	{
		//
		//	Close any open devices
		//
		Free_3D_Driver_List ();
		Close_2D_Device ();

		//
		//	Open the 2D device as specified
		//
		Open_2D_Device (is_stereo, bits, hertz);

		//
		//	Find and open the 3D device specified
		//
		Build_3D_Driver_List ();
		Select_3D_Device (device_name);
		retval = true;

		//
		//	Select the speaker type
		//
		Set_Speaker_Type (m_SpeakerType);
	}

	m_RealMusicVolume = m_MusicVolume;
	m_RealSoundVolume = m_SoundVolume;
	return retval;
}////////////////////////////////////////////////////////////////////////////////////////////
//
//	Save_To_Registry
//
////////////////////////////////////////////////////////////////////////////////////////////
bool
MilesAudioClass::Save_To_Registry (const char *subkey_name)
{
	StringClass device_name;

	//
	// Get the name of the current 3D driver
	//
	for (int index = 0; index < m_Driver3DList.Count (); index ++) {
		DRIVER_INFO_STRUCT *info = m_Driver3DList[index];

		//
		//	Is this the device we were looking for?
		//
		if (info != NULL && info->driver == m_Driver3D) {
			device_name = info->name;
			break;
		}
	}

	//
	//	Save these settings to the registry
	//
	return WWAudioClass::Save_To_Registry (subkey_name, device_name, m_PlaybackStereo, m_PlaybackBits, m_PlaybackRate,
				m_AreSoundEffectsEnabled, m_IsMusicEnabled, m_IsDialogEnabled, m_IsCinematicSoundEnabled,
				m_SoundVolume, m_MusicVolume, m_DialogVolume, m_CinematicVolume, m_SpeakerType);
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	File_Open_Callback
//
////////////////////////////////////////////////////////////////////////////////////////////
U32 AILCALLBACK
MilesAudioClass::File_Open_Callback (char const *filename, void **file_handle)
{
	U32 retval = false;

	if (Get_Instance () != NULL) {

		//
		//	Open the file
		//
		FileClass *file = reinterpret_cast<MilesAudioClass *>(Get_Instance ())->Get_File (filename);
		if (file != NULL && file->Open ()) {
			(*file_handle) = reinterpret_cast<void *> (file);
			retval = true;
		}
	}

	return retval;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	File_Close_Callback
//
////////////////////////////////////////////////////////////////////////////////////////////
void AILCALLBACK
MilesAudioClass::File_Close_Callback (void *file_handle)
{
	if (Get_Instance () != NULL) {

		//
		//	Close the file (if necessary)
		//
		FileClass *file = reinterpret_cast<FileClass *> (file_handle);
		if (file != NULL) {
			reinterpret_cast<MilesAudioClass *>(Get_Instance ())->Return_File (file);
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	File_Seek_Callback
//
////////////////////////////////////////////////////////////////////////////////////////////
S32 AILCALLBACK
MilesAudioClass::File_Seek_Callback (void *file_handle, S32 offset, U32 type)
{
	S32 retval = 0;

	//
	//	Convert the handle to a file handle type
	//
	FileClass *file = reinterpret_cast<FileClass *> (file_handle);
	if (file != NULL) {

		//
		//	Convert the Miles seek type to one of our own
		//
		int seek_type = SEEK_CUR;
		switch (type)
		{
			case AIL_FILE_SEEK_BEGIN:
				seek_type = SEEK_SET;
				break;

			case AIL_FILE_SEEK_CURRENT:
				seek_type = SEEK_CUR;
				break;

			case AIL_FILE_SEEK_END:
				seek_type = SEEK_END;
				break;
		}

		//
		//	Perform the seek
		//
		retval = file->Seek (offset, seek_type);
	}

	return retval;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	File_Read_Callback
//
////////////////////////////////////////////////////////////////////////////////////////////
U32 AILCALLBACK
MilesAudioClass::File_Read_Callback (void *file_handle, void *buffer, U32 bytes)
{
	U32 retval = 0;

	//
	//	Convert the handle to a file handle type
	//
	FileClass *file = reinterpret_cast<FileClass *> (file_handle);
	if (file != NULL) {

		//
		//	Read the bytes from the file
		//
		retval = file->Read (buffer, bytes);
	}

	return retval;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Peek_2D_Sample
//
////////////////////////////////////////////////////////////////////////////////////////////
AudibleSoundClass *
MilesAudioClass::Peek_2D_Sample (int index)
{
	if (index < 0 || index > m_2DSampleHandles.Count ()) {
		return NULL;
	}

	MMSLockClass lock;
	AudibleSoundClass *retval = NULL;

	//
	// Try to get the sound object associated with this handle
	//
	HSAMPLE sample = m_2DSampleHandles[index];
	if (sample != NULL) {
		retval = (AudibleSoundClass *)::AIL_sample_user_data (sample, INFO_OBJECT_PTR);
	}

	return retval;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Peek_3D_Sample
//
////////////////////////////////////////////////////////////////////////////////////////////
AudibleSoundClass *
MilesAudioClass::Peek_3D_Sample (int index)
{
	if (index < 0 || index > m_3DSampleHandles.Count ()) {
		return NULL;
	}

	MMSLockClass lock;
	AudibleSoundClass *retval = NULL;

	//
	// Try to get the sound object associated with this handle
	//
	H3DSAMPLE sample = m_3DSampleHandles[index];
	if (sample != NULL) {
		retval = (AudibleSoundClass *)::AIL_3D_object_user_data (sample, INFO_OBJECT_PTR);
	}

	return retval;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Set_Speaker_Type
//
////////////////////////////////////////////////////////////////////////////////////////////
void
MilesAudioClass::Set_Speaker_Type (int speaker_type)
{
	m_SpeakerType = speaker_type;

	//
	//	Pass the new speaker type onto miles
	//
	if (m_Driver3D != NULL) {
		::AIL_set_3D_speaker_type (m_Driver3D, speaker_type);
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Get_Speaker_Type
//
////////////////////////////////////////////////////////////////////////////////////////////
int
MilesAudioClass::Get_Speaker_Type (void) const
{
	return m_SpeakerType;
}
