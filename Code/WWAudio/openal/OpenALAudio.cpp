/*
**	Command & Conquer Renegade(tm)
**	Copyright 2025 OpenW3D Contributors.
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

#include "OpenALAudio.h"
#include "FFmpegBuffer.h"
#include "OpenALHandle.h"
#include "SoundScene.h"
#include "Threads.h"
#include "Utils.h"
#include "ffactory.h"
#include "realcrc.h"
#include "wwfile.h"

#include <cstring>

#define LOAD_ALC_PROC(N) N = reinterpret_cast<decltype(N)>(alcGetProcAddress(m_alcDevice, #N))

// Debug callback for OpenAL errors
static void AL_APIENTRY Debug_Callback(ALenum /* source */, ALenum /* type */, ALuint /* id */,
	ALenum severity, ALsizei /* length */, [[maybe_unused]] const ALchar* message, void* /* userParam */ ) AL_API_NOEXCEPT17
{
	switch (severity)
	{
	case AL_DEBUG_SEVERITY_HIGH_EXT:
		WWDEBUG_SAY(("OpenAL Error: %s\n", message));
		break;
	case AL_DEBUG_SEVERITY_MEDIUM_EXT:
		WWDEBUG_SAY(("OpenAL Warning: %s\n", message));
		break;
	case AL_DEBUG_SEVERITY_LOW_EXT:
		WWDEBUG_SAY(("OpenAL Info: %s\n", message));
		break;
	default:
		WWDEBUG_SAY(("OpenAL Message: %s\n", message));
		break;
	}
}

WWAudioClass *WWAudioClass::Create_Instance(bool lite)
{
	return new OpenALAudioClass(lite);
}

OpenALAudioClass::OpenALAudioClass(bool lite)
	:
		WWAudioClass(lite),
		m_DriverName("OpenAL 3D Audio"),
		m_alcDevice(nullptr),
		m_alcContext(nullptr),
		m_SpeakerType(W3D_3D_2_SPEAKER),
		m_CurrentCacheSize(0)
{
	_theInstance = this;
}
	
OpenALAudioClass::~OpenALAudioClass()
{
	//
	//	Stop the background music
	//
	Set_Background_Music (NULL);

	//
	//	Make sure the delayed-release thread is terminated
	// before we exit (otherwise the process will crash).
	//
	WWAudioThreadsClass::End_Delayed_Release_Thread();

	Shutdown();
}


void OpenALAudioClass::Initialize(bool stereo, int bits, int hertz)
{
	if (!Is_Disabled()) {
		Open_2D_Device(stereo, bits, hertz);
		m_SoundScene->Initialize();
	}
	
	m_RealMusicVolume = m_MusicVolume;
	m_RealSoundVolume = m_SoundVolume;
}

void OpenALAudioClass::Initialize(const char *registry_subkey_name)
{
	StringClass device_name;
	bool is_stereo = true;
	int bits = 16;
	int hertz = 44100;
	int speaker_type = m_SpeakerType;

	if (WWAudioClass::Load_From_Registry (registry_subkey_name, device_name, is_stereo, bits, hertz,
			m_AreSoundEffectsEnabled, m_IsMusicEnabled, m_IsDialogEnabled, m_IsCinematicSoundEnabled,
			m_SoundVolume, m_MusicVolume, m_DialogVolume, m_CinematicVolume, speaker_type))
	{
		Set_Speaker_Type(speaker_type);
		Initialize(is_stereo, bits, hertz);
	}
}

void OpenALAudioClass::Shutdown()
{
	//
	//	Stop the background music
	//
	Set_Background_Music(NULL);

	//
	// Stop all sounds from playing
	//
	Flush_Playlist();
	if (m_SoundScene != NULL) {
		m_SoundScene->Flush_Scene();
	}

	//
	// Free all our cached sound buffers
	//
	Flush_Cache();

	//
	// Close-out our hold on any driver resources
	//
	Remove_Handles();
	Release_Handles();
	delete m_SoundScene;
	m_SoundScene = nullptr;
	Close_2D_Device();
}

WWAudioClass::DRIVER_TYPE_2D OpenALAudioClass::Open_2D_Device(bool stereo, int bits, int hertz)
{
	//
	// Only OpenAL driver can be used by this backend though this return is never checked by the engine.
	//
	DRIVER_TYPE_2D type = DRIVER2D_OPENAL;

	// First close the current 2D device and take
	// all the sound handles away from the sound objects.
	Close_2D_Device();

	m_PlaybackStereo = stereo;
	m_PlaybackBits = bits;
	m_PlaybackRate = hertz;

	m_alcDevice = alcOpenDevice(nullptr);
	if (m_alcDevice == nullptr) {
		WWDEBUG_SAY(("Failed to open ALC device"));
		return DRIVER2D_ERROR;
	}
	
	ALCint attributes[6];
	int attr_index = 0;
	attributes[attr_index++] = ALC_FREQUENCY;
	attributes[attr_index++] = m_PlaybackRate;

#ifdef ALC_SOFT_HRTF
	if (alcIsExtensionPresent(m_alcDevice, "ALC_SOFT_HRTF")) {
		attributes[attr_index++] = ALC_HRTF_SOFT;
		attributes[attr_index++] = (m_SpeakerType == W3D_3D_HEADPHONE) ? ALC_TRUE : ALC_FALSE;
	}
#endif

	attributes[attr_index] = 0;
	m_alcContext = alcCreateContext(m_alcDevice, attributes);

	if (m_alcContext == nullptr) {
		WWDEBUG_SAY(("Failed to create ALC context"));
		type = DRIVER2D_ERROR;
	}

	if (!alcMakeContextCurrent(m_alcContext)) {
		WWDEBUG_SAY(("Failed to make ALC context current"));
		type = DRIVER2D_ERROR;
	}

	// Allocate all the available handles if we were successful
	if (type != DRIVER2D_ERROR) {
		if (alcIsExtensionPresent(m_alcDevice, "ALC_EXT_debug")) {
			LPALDEBUGMESSAGECALLBACKEXT alDebugMessageCallbackEXT;
			LOAD_ALC_PROC(alDebugMessageCallbackEXT);
			alEnable(AL_DEBUG_OUTPUT_EXT);
			alDebugMessageCallbackEXT(Debug_Callback, nullptr);
		}

		alDistanceModel(AL_LINEAR_DISTANCE_CLAMPED);
		if (alGetError() != AL_NO_ERROR) {
			WWDEBUG_SAY(("Failed to set OpenAL distance model.\n"));
		}

		Allocate_Handles();
		ReAssign_Handles();
	} else {
		Close_2D_Device();
		WWDEBUG_SAY (("WWAudio: Error initializing 2D device.\r\n"));
	}

	return type;
}

void OpenALAudioClass::Close_2D_Device()
{
	//
	//	Free any sound handles
	//
	Remove_Handles();
	Release_Handles();

	alcMakeContextCurrent(nullptr);

	if (m_alcContext) {
		alcDestroyContext(m_alcContext);
		m_alcContext = nullptr;
	}

	if (m_alcDevice) {
		alcCloseDevice(m_alcDevice);
		m_alcDevice = nullptr;
	}
}

void OpenALAudioClass::Allocate_Handles()
{
	m_2DSampleHandles.Resize(DEF_2D_SAMPLE_COUNT);
	alGenSources(DEF_2D_SAMPLE_COUNT, &m_2DSampleHandles[0]);

	if(alGetError() == AL_NO_ERROR) {
		m_2DSampleHandles.Set_Active(DEF_2D_SAMPLE_COUNT);

		for (int i = 0; i < m_2DSampleHandles.Count(); ++i) {
			alSourcei(m_2DSampleHandles[i], AL_SOURCE_RELATIVE, AL_TRUE);
			alSourcef(m_2DSampleHandles[i], AL_ROLLOFF_FACTOR, 0.0F);
			alSourcef(m_2DSampleHandles[i], AL_REFERENCE_DISTANCE, 1.0F);
			alSourcef(m_2DSampleHandles[i], AL_MAX_DISTANCE, 1.0F);

			if (alGetError() != AL_NO_ERROR) {
				WWDEBUG_SAY(("Configuring 2D OpenAL source failed for sample %d.", i));
			}
		}
	} else {
		WWDEBUG_SAY(("Allocate_2D_Handles failed."));
	}

	m_3DSampleHandles.Resize(DEF_3D_SAMPLE_COUNT);
	alGenSources(DEF_3D_SAMPLE_COUNT, &m_3DSampleHandles[0]);

	if(alGetError() == AL_NO_ERROR) {
		m_3DSampleHandles.Set_Active(DEF_3D_SAMPLE_COUNT);
		
		for (int i = 0; i < m_3DSampleHandles.Count(); ++i) {
			alSourcei(m_3DSampleHandles[i], AL_SOURCE_RELATIVE, AL_FALSE);
			alSourcef(m_3DSampleHandles[i], AL_ROLLOFF_FACTOR, 1.0F);

			if (alGetError() != AL_NO_ERROR) {
				WWDEBUG_SAY(("Configuring 3D OpenAL source failed for sample %d.", i));
			}
		}
	} else {
		WWDEBUG_SAY(("Allocate_2D_Handles failed."));
	}
}

void OpenALAudioClass::Set_Speaker_Type(int speaker_type)
{
	switch (speaker_type) {
		case W3D_3D_2_SPEAKER:
		case W3D_3D_HEADPHONE:
		case W3D_3D_SURROUND:
		case W3D_3D_4_SPEAKER:
			m_SpeakerType = speaker_type;
			break;

		default:
			m_SpeakerType = W3D_3D_2_SPEAKER;
			break;
	}
}

void OpenALAudioClass::Release_Handles()
{
	if (m_2DSampleHandles.Count() != 0) {
		for (int index = 0; index < m_2DSampleHandles.Count(); ++index) {
			OpenALHandleClass::Set_Sample_User(m_2DSampleHandles[index], nullptr);
		}
		alDeleteSources(m_2DSampleHandles.Count(), &m_2DSampleHandles[0]);

		if(alGetError() != AL_NO_ERROR) {
			WWDEBUG_SAY(("Release_2D_Handles an error occured while freeing handles."));
		}
		
		m_2DSampleHandles.Delete_All();
	}

	if (m_3DSampleHandles.Count() != 0) {
		for (int index = 0; index < m_3DSampleHandles.Count(); ++index) {
			OpenALHandleClass::Set_Sample_User(m_3DSampleHandles[index], nullptr);
		}
		alDeleteSources(m_3DSampleHandles.Count(), &m_3DSampleHandles[0]);

		if(alGetError() != AL_NO_ERROR) {
			WWDEBUG_SAY(("Release_2D_Handles an error occured while freeing handles."));
		}
		
		m_3DSampleHandles.Delete_All();
	}
}

void OpenALAudioClass::ReAssign_Handles()
{
	for (int index = 0; index < m_Playlist[m_CurrPage].Count (); index ++) {
		AudibleSoundClass *sound_obj = m_Playlist[m_CurrPage][index];

		// If this is a 2D sound effect, then force it to 'get' a new
		// sound handle.
		if ((sound_obj->Get_Class_ID () == CLASSID_2D) ||
			 (sound_obj->Get_Class_ID () == CLASSID_PSEUDO3D) ||
			 (sound_obj->Get_Class_ID () == CLASSID_2DTRIGGER) ||
			 (sound_obj->Get_Class_ID () == CLASSID_3D))
		{
			sound_obj->Free_Miles_Handle();
			sound_obj->Allocate_Miles_Handle();
		}
	}
}

void OpenALAudioClass::Remove_Handles()
{
	for (int index = 0; index < m_2DSampleHandles.Count (); index ++) {
		ALint state;
		ALuint sample = m_2DSampleHandles[index];
		alGetSourcei(sample, AL_SOURCE_STATE, &state);
		
		if (alGetError() == AL_NO_ERROR) {
			//
			// Get a pointer to the object that is currently using this sample
			//
			AudibleSoundClass *sound_obj = OpenALHandleClass::Get_Sample_User(sample);
			if (sound_obj != NULL) {
				sound_obj->Free_Miles_Handle ();
			}
		}
	}

	for (int index = 0; index < m_3DSampleHandles.Count (); index ++) {
		ALint state;
		ALuint sample = m_3DSampleHandles[index];
		alGetSourcei(sample, AL_SOURCE_STATE, &state);

		if (alGetError() == AL_NO_ERROR) {
			AudibleSoundClass *sound_obj = OpenALHandleClass::Get_Sample_User(sample);
			if (sound_obj != NULL) {
				sound_obj->Free_Miles_Handle ();
			}
		}
	}
}

void OpenALAudioClass::Flush_Cache()
{
	for (int hash_index = 0; hash_index < MAX_CACHE_HASH; hash_index ++) {
		for (int index = 0; index < m_CachedBuffers[hash_index].Count (); index ++) {
			CACHE_ENTRY_STRUCT &info = m_CachedBuffers[hash_index][index];
			SAFE_FREE(info.string_id);
			REF_PTR_RELEASE(info.buffer);
		}
		m_CachedBuffers[hash_index].Delete_All ();
	}

	m_CurrentCacheSize = 0;
}

AudibleSoundClass *OpenALAudioClass::Peek_2D_Sample (int index)
{
	if (index < 0 || index > m_2DSampleHandles.Count ()) {
		return NULL;
	}

	AudibleSoundClass *retval = NULL;

	//
	// Try to get the sound object associated with this handle
	//
	ALint state;
	ALuint sample = m_2DSampleHandles[index];
	alGetSourcei(sample, AL_SOURCE_STATE, &state);
	if (alGetError() == AL_NO_ERROR) {
		retval = OpenALHandleClass::Get_Sample_User(sample);
	}

	return retval;
}

AudibleSoundClass *OpenALAudioClass::Peek_3D_Sample (int index)
{
	if (index < 0 || index > m_3DSampleHandles.Count ()) {
		return NULL;
	}

	AudibleSoundClass *retval = NULL;

	//
	// Try to get the sound object associated with this handle
	//
	ALint state;
	ALuint sample = m_3DSampleHandles[index];
	alGetSourcei(sample, AL_SOURCE_STATE, &state);
	if (alGetError() == AL_NO_ERROR) {
		retval = OpenALHandleClass::Get_Sample_User(sample);
	}

	return retval;
}

SoundHandleClass *OpenALAudioClass::Get_2D_Handle (const AudibleSoundClass &sound_obj, bool /* streaming */)
{
	if (!WWAudioClass::Is_OK_To_Give_Handle(sound_obj)) {
		return NULL;
	}

	float lowest_priority = sound_obj.Get_Priority ();
	float lowest_runtime_priority = sound_obj.Get_Runtime_Priority ();
	AudibleSoundClass *lowest_pri_sound = NULL;
	ALuint lowest_pri_sample = OpenALHandleClass::INVALID_OAL_HANDLE;
	ALuint free_sample = OpenALHandleClass::INVALID_OAL_HANDLE;

	// Loop through all the available sample handles and try to find
	// one that isn't being used to play a sound.
	bool found = false;
	for (int index = 0; (index < m_2DSampleHandles.Count ()) && !found; index ++) {
		ALint state;
		ALuint sample = m_2DSampleHandles[index];
		alGetSourcei(sample, AL_SOURCE_STATE, &state);

		if (alGetError() == AL_NO_ERROR && state != AL_PLAYING) {
			// Get a pointer to the object that is currently using this sample
			AudibleSoundClass *sample_obj = OpenALHandleClass::Get_Sample_User(sample);

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
				float priority = sample_obj->Get_Priority ();
				float runtime_priority = sample_obj->Get_Runtime_Priority ();
				if ((priority < lowest_priority) || (priority == lowest_priority && runtime_priority <= lowest_runtime_priority)) {
					lowest_priority = priority;
					lowest_pri_sound = sample_obj;
					lowest_pri_sample = sample;
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

	if (free_sample == OpenALHandleClass::INVALID_OAL_HANDLE) {
		return NULL;
	}

	SoundHandleClass *retval = new OpenALHandleClass;
	retval->Set_Miles_Handle(reinterpret_cast<void *>(uintptr_t(free_sample)));

	// Return the free sample handle if we found one
	return retval;
}

SoundHandleClass *OpenALAudioClass::Get_3D_Handle (const Sound3DClass &sound_obj)
{
	if (!WWAudioClass::Is_OK_To_Give_Handle(sound_obj)) {
		return NULL;
	}

	float lowest_priority = sound_obj.Get_Priority ();
	float lowest_runtime_priority = sound_obj.Get_Runtime_Priority ();
	AudibleSoundClass *lowest_pri_sound = NULL;
	ALuint lowest_pri_sample = OpenALHandleClass::INVALID_OAL_HANDLE;
	ALuint free_sample = OpenALHandleClass::INVALID_OAL_HANDLE;


	// Loop through all the available sample handles and try to find
	// one that isn't being used to play a sound.
	bool found = false;
	for (int index = 0; (index < m_3DSampleHandles.Count ()) && !found; index ++) {
		ALint state;
		ALuint sample = m_3DSampleHandles[index];
		alGetSourcei(sample, AL_SOURCE_STATE, &state);

		if (alGetError() == AL_NO_ERROR && state != AL_PLAYING) {
			// Get a pointer to the object that is currently using this sample
			AudibleSoundClass *sample_obj = OpenALHandleClass::Get_Sample_User(sample);
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
				float priority = sample_obj->Get_Priority ();
				float runtime_priority = sample_obj->Get_Runtime_Priority ();
				if ((priority < lowest_priority) || (priority == lowest_priority && runtime_priority <= lowest_runtime_priority)) {
					lowest_priority = priority;
					lowest_pri_sound = sample_obj;
					lowest_pri_sample = sample;
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

	if (free_sample == OpenALHandleClass::INVALID_OAL_HANDLE) {
		return NULL;
	}

	SoundHandleClass *retval = new OpenALHandleClass;
	retval->Set_Miles_Handle(reinterpret_cast<void *>(uintptr_t(free_sample)));

	// Return the free sample handle if we found one
	return retval;
}

SoundBufferClass *OpenALAudioClass::Get_Sound_Buffer (const char *filename, bool is_3d)
{
	if (filename == nullptr || *filename == '\0') {
		return nullptr;
	}

	FileClass *file = _TheFileFactory != nullptr ? _TheFileFactory->Get_File(filename) : nullptr;
	if (file == nullptr || !file->Is_Available()) {
		static int count = 0;
		if (++count < 10) {
			WWDEBUG_SAY(("Sound \"%s\" not found\r\n", filename));
		}
		if (file != nullptr) {
			_TheFileFactory->Return_File(file);
		}
		return nullptr;
	}

	const int file_size = file->Size();
	_TheFileFactory->Return_File(file);

	const int max_size = is_3d ? DEF_MAX_3D_BUFFER_SIZE * 2 : DEF_MAX_2D_BUFFER_SIZE;
	const bool streaming = file_size > max_size;

	if (!streaming) {
		SoundBufferClass *cached_buffer = Find_Cached_Buffer(filename);
		if (cached_buffer != nullptr) {
			return cached_buffer;
		}
	}

	FFMpegBufferClass *sound_buffer = new FFMpegBufferClass;
	SET_REF_OWNER(sound_buffer);

	//
	// Create a new sound buffer from the provided file
	//
	bool success = sound_buffer->Load_From_File (filename, streaming);
	WWASSERT (success);

	// If we were successful in creating the sound buffer, then
	// return it, otherwise free the buffer and return NULL.
	if (success && !streaming) {
		Cache_Buffer(sound_buffer, filename);
	} else if (!success) {
		REF_PTR_RELEASE (sound_buffer);
	}

	// Return a pointer to the new sound buffer
	return sound_buffer;
}

SoundBufferClass *OpenALAudioClass::Find_Cached_Buffer(const char *string_id)
{
	SoundBufferClass *sound_buffer = nullptr;
	WWASSERT(string_id != nullptr);

	if (string_id != nullptr) {
		const int hash_index = ::CRC_Stringi(string_id) & CACHE_HASH_MASK;
		for (int index = 0; index < m_CachedBuffers[hash_index].Count(); index++) {
			CACHE_ENTRY_STRUCT &info = m_CachedBuffers[hash_index][index];
			if (::stricmp(info.string_id, string_id) == 0) {
				sound_buffer = info.buffer;
				sound_buffer->Add_Ref();
				break;
			}
		}
	}

	return sound_buffer;
}

bool OpenALAudioClass::Cache_Buffer(SoundBufferClass *buffer, const char *string_id)
{
	bool retval = false;
	WWASSERT(buffer != nullptr);
	WWASSERT(string_id != nullptr);

	if (buffer != nullptr && string_id != nullptr && !buffer->Is_Streaming()) {
		const int hash_index = ::CRC_Stringi(string_id) & CACHE_HASH_MASK;
		CACHE_ENTRY_STRUCT info;
		info.string_id = const_cast<char *>(string_id);
		info.buffer = buffer;
		m_CachedBuffers[hash_index].Add(info);
		m_CurrentCacheSize += buffer->Get_Raw_Length();
		retval = true;
	}

	if (!retval) {
		WWDEBUG_SAY(("Unable to cache sound: %s.\r\n", string_id != nullptr ? string_id : "<null>"));
	}

	return retval;
}
