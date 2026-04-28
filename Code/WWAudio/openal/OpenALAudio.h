/*
**	Command & Conquer Renegade(tm)
**	Copyright 2026 OpenW3D Contributors.
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

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __OPENALAUDIO_H
#define __OPENALAUDIO_H

#include "WWAudio.h"
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#include <string.h>

class FFMpegBufferClass;

enum
{
	MAX_CACHE_HASH = 256,
	CACHE_HASH_MASK = 0x000000FF
};

class OpenALAudioClass final : public WWAudioClass
{
public:
	OpenALAudioClass(bool lite);
	~OpenALAudioClass() override;
	void Initialize (bool stereo = true, int bits = 16, int hertz = 44100) override;
	void Initialize (const char *registry_subkey_name) override;
	void Shutdown() override;
	const StringClass & Get_3D_Driver_Name() const override { return m_DriverName; }
	DRIVER_TYPE_2D Open_2D_Device (bool stereo, int bits, int hertz) override;
	int Get_3D_Device_Count() const override { return 1; }
	bool Get_3D_Device (int /* index */, const char **info) override   { (*info) = m_DriverName; return true; }
	bool Select_3D_Device (const char * /* device_name */) override { return true; }
	void Set_Speaker_Type (int speaker_type) override;
	int Get_Speaker_Type() const override { return m_SpeakerType; }
	float Get_Effects_Level() override { return 0.0F; }
	void Flush_Cache() override;
	int Get_2D_Sample_Count() const override { return m_2DSampleHandles.Count();}
	int Get_3D_Sample_Count() const override { return m_3DSampleHandles.Count();}
	AudibleSoundClass *Peek_2D_Sample (int index) override;
	AudibleSoundClass *Peek_3D_Sample (int index) override;
	bool Validate_3D_Sound_Buffer (SoundBufferClass *buffer) override { return buffer != nullptr && buffer->Get_Channels () == 1; }
	SoundHandleClass *Get_2D_Handle (const AudibleSoundClass &sound_obj, bool streaming) override;
	SoundHandleClass *Get_3D_Handle (const Sound3DClass &sound_obj) override;
	SoundBufferClass *Get_Sound_Buffer (const char *filename, bool is_3d) override;

	static ALenum Get_AL_Format(unsigned channels, unsigned bits)
	{
		if (channels == 1 && bits == 8) {
			return AL_FORMAT_MONO8;
		}

		if (channels == 1 && bits == 16){
			return AL_FORMAT_MONO16;
		}

		if (channels == 1 && bits == 32) {
			return AL_FORMAT_MONO_FLOAT32;
		}

		if (channels == 2 && bits == 8) {
			return AL_FORMAT_STEREO8;
		}

		if (channels == 2 && bits == 16) {
			return AL_FORMAT_STEREO16;
		}

		if (channels == 2 && bits == 32) {
			return AL_FORMAT_STEREO_FLOAT32;
		}

		WWDEBUG_SAY(("Unknown OpenAL format: %u channels, %u bits per sample", channels, bits));
		return AL_FORMAT_MONO8;
	}
	
private:
	void Close_2D_Device();
  
	void Allocate_Handles();
	void Release_Handles();
	void ReAssign_Handles();
	void Remove_Handles();
	SoundBufferClass *Find_Cached_Buffer(const char *string_id);
	bool Cache_Buffer(SoundBufferClass *buffer, const char *string_id);

private:
	typedef struct _CACHE_ENTRY_STRUCT
	{
		char *string_id;
		SoundBufferClass *buffer;

		_CACHE_ENTRY_STRUCT(void)
			: string_id(nullptr), buffer(nullptr) {}

		_CACHE_ENTRY_STRUCT &operator=(const _CACHE_ENTRY_STRUCT &src)
		{
			string_id = ::strdup(src.string_id);
			REF_PTR_SET(buffer, src.buffer);
			return *this;
		}
		bool operator==(const _CACHE_ENTRY_STRUCT &/* src */) { return false; }
		bool operator!=(const _CACHE_ENTRY_STRUCT &/* src */) { return true; }
	} CACHE_ENTRY_STRUCT;

	DynamicVectorClass<ALuint> m_2DSampleHandles;
	DynamicVectorClass<ALuint> m_3DSampleHandles;
	DynamicVectorClass<CACHE_ENTRY_STRUCT> m_CachedBuffers[MAX_CACHE_HASH];
	StringClass m_DriverName;
	ALCdevice *m_alcDevice;
	ALCcontext *m_alcContext;
	int m_SpeakerType;
	int m_CurrentCacheSize;
};

#endif /* __OPENALAUDIO_H */
