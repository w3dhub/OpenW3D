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

#include "OpenALHandle.h"

#include "AudibleSound.h"
#include "OpenALAudio.h"
#include "wwdebug.h"

#include <algorithm>
#include <cstdint>
#include <cstring>

std::unordered_map<ALuint, AudibleSoundClass *> OpenALHandleClass::SampleUsers;

//////////////////////////////////////////////////////////////////////
//
//	OpenALHandleClass
//
//////////////////////////////////////////////////////////////////////
OpenALHandleClass::OpenALHandleClass()
	: SampleHandle(INVALID_OAL_HANDLE),
	  SampleBuffers{ 0 },
	  SampleBufferIndex(0),
	  SampleLoopCount(1),
	  SamplePositionMs(0),
	  StreamBufferLength{ 0 },
	  SampleEnded(true),
	  StreamEOF(false)
{
	alGenBuffers(OAL_BUFFER_COUNT, SampleBuffers);
	if (alGetError() != AL_NO_ERROR) {
		WWDEBUG_SAY(("Failed to generate OpenAL buffers.\n"));
	}

	StreamScratch.resize(32768);
}

OpenALHandleClass::~OpenALHandleClass()
{
	Reset_Source();
	if (SampleHandle != INVALID_OAL_HANDLE) {
		Set_Sample_User(SampleHandle, nullptr);
	}
	alDeleteBuffers(OAL_BUFFER_COUNT, SampleBuffers);
}

void OpenALHandleClass::Set_Miles_Handle(void *handle)
{
	SampleHandle = ALuint(reinterpret_cast<uintptr_t>(handle));
}

void OpenALHandleClass::Initialize(SoundBufferClass *buffer)
{
	Reset_Source();
	SoundHandleClass::Initialize(buffer);
	SampleLoopCount = 1;
	SampleEnded = true;
	StreamEOF = false;
	SamplePositionMs = 0;
	SampleBufferIndex = 0;
	std::fill(StreamBufferLength, StreamBufferLength + OAL_BUFFER_COUNT, 0u);
	Clear_Static_Buffer();

	if (Buffer != nullptr && !Buffer->Is_Streaming()) {
		Upload_Static_Buffer();
	}
}

void OpenALHandleClass::Start_Sample()
{
	if (Buffer == nullptr) {
		return;
	}

	Reset_Source();
	SampleEnded = false;

	if (Buffer->Is_Streaming()) {
		if (!Stream.Open(Buffer->Get_Filename())) {
			SampleEnded = true;
			return;
		}

		ALint queued = 0;
		while (queued < OAL_BUFFER_COUNT && Queue_Stream_Buffer(SampleBuffers[SampleBufferIndex])) {
			SampleBufferIndex = (SampleBufferIndex + 1) % OAL_BUFFER_COUNT;
			++queued;
		}

		if (queued == 0) {
			SampleEnded = true;
			Stream.Close();
			return;
		}
	} else {
		Upload_Static_Buffer();
	}

	alSourcePlay(SampleHandle);
	if (alGetError() != AL_NO_ERROR) {
		WWDEBUG_SAY(("Couldn't play source.\n"));
	}
}

void OpenALHandleClass::Stop_Sample()
{
	alSourcePause(SampleHandle);
	if (alGetError() != AL_NO_ERROR) {
		WWDEBUG_SAY(("Couldn't pause source.\n"));
	}
}

void OpenALHandleClass::Resume_Sample()
{
	if (!SampleEnded) {
		alSourcePlay(SampleHandle);
		if (alGetError() != AL_NO_ERROR) {
			WWDEBUG_SAY(("Couldn't resume source.\n"));
		}
	}
}

void OpenALHandleClass::End_Sample()
{
	SampleEnded = true;
	Reset_Source();
}

void OpenALHandleClass::Set_Sample_Pan(float pan)
{
	ALfloat location[3] = { pan, 0.0F, 0.0F };
	alSourcefv(SampleHandle, AL_POSITION, location);
	if (alGetError() != AL_NO_ERROR) {
		WWDEBUG_SAY(("Couldn't set source pan.\n"));
	}
}

float OpenALHandleClass::Get_Sample_Pan()
{
	ALfloat location[3] = { 0.0F, 0.0F, 0.0F };
	alGetSourcefv(SampleHandle, AL_POSITION, location);
	if (alGetError() != AL_NO_ERROR) {
		WWDEBUG_SAY(("Couldn't get source pan.\n"));
	}

	return location[0];
}

void OpenALHandleClass::Set_Sample_Volume(float volume)
{
	alSourcef(SampleHandle, AL_GAIN, volume);
	if (alGetError() != AL_NO_ERROR) {
		WWDEBUG_SAY(("OpenALHandleClass::Set_Sample_Volume couldn't set source gain.\n"));
	}
}

float OpenALHandleClass::Get_Sample_Volume()
{
	ALfloat state = 0.0F;
	alGetSourcef(SampleHandle, AL_GAIN, &state);
	if (alGetError() != AL_NO_ERROR) {
		return 0.0F;
	}

	return state;
}

void OpenALHandleClass::Set_Sample_Loop_Count(unsigned count)
{
	WWDEBUG_SAY(("Sample %s requested to loop %u times.\n", Buffer != nullptr ? Buffer->Get_Filename() : "<null>", count));
	SampleLoopCount = count == 0 ? UINT_MAX : count;
	if (Buffer != nullptr && !Buffer->Is_Streaming()) {
		alSourcei(SampleHandle, AL_LOOPING, count == 0 ? AL_TRUE : AL_FALSE);
	}
}

unsigned OpenALHandleClass::Get_Sample_Loop_Count()
{
	return SampleLoopCount;
}

void OpenALHandleClass::Set_Sample_MS_Position(unsigned ms)
{
	if (Buffer == nullptr) {
		return;
	}

	if (Buffer->Is_Streaming()) {
		ALint state = AL_STOPPED;
		alGetSourcei(SampleHandle, AL_SOURCE_STATE, &state);
		Reset_Source();
		if (!Stream.Open(Buffer->Get_Filename()) || !Stream.Seek_MS(ms)) {
			SampleEnded = true;
			return;
		}

		SampleEnded = false;
		SamplePositionMs = std::min(ms, Buffer->Get_Duration());
		ALint queued = 0;
		while (queued < OAL_BUFFER_COUNT && Queue_Stream_Buffer(SampleBuffers[SampleBufferIndex])) {
			SampleBufferIndex = (SampleBufferIndex + 1) % OAL_BUFFER_COUNT;
			++queued;
		}
		if (queued == 0) {
			SampleEnded = true;
			return;
		}
		if (state == AL_PLAYING) {
			alSourcePlay(SampleHandle);
		}
	} else {
		alSourcef(SampleHandle, AL_SEC_OFFSET, static_cast<ALfloat>(ms) / 1000.0F);
	}
}

void OpenALHandleClass::Get_Sample_MS_Position(int *len, int *pos)
{
	if (len != nullptr) {
		*len = Buffer != nullptr ? static_cast<int>(Buffer->Get_Duration()) : 0;
	}

	if (pos != nullptr) {
		ALfloat current_position = 0.0F;
		alGetSourcef(SampleHandle, AL_SEC_OFFSET, &current_position);
		unsigned int current_ms = static_cast<unsigned int>(current_position * 1000.0F);
		if (Buffer != nullptr && Buffer->Is_Streaming()) {
			current_ms += SamplePositionMs;
			if (Buffer->Get_Duration() > 0) {
				current_ms %= Buffer->Get_Duration();
			}
		}
		*pos = static_cast<int>(current_ms);
	}
}

void OpenALHandleClass::Set_Sample_User_Data(int /* i */, void *val)
{
	Set_Sample_User(SampleHandle, static_cast<AudibleSoundClass *>(val));
}

void *OpenALHandleClass::Get_Sample_User_Data(int /* i */)
{
	return Get_Sample_User(SampleHandle);
}

float OpenALHandleClass::Get_Sample_Pitch()
{
	ALfloat pitch = 1.0F;
	alGetSourcef(SampleHandle, AL_PITCH, &pitch);
	if (alGetError() != AL_NO_ERROR) {
		WWDEBUG_SAY(("Failed to retrieve OpenAL source pitch.\n"));
	}

	return pitch;
}

void OpenALHandleClass::Set_Sample_Pitch(float pitch)
{
	alSourcef(SampleHandle, AL_PITCH, pitch);
	if (alGetError() != AL_NO_ERROR) {
		WWDEBUG_SAY(("Failed to set OpenAL source pitch.\n"));
	}
}

void OpenALHandleClass::Set_Position(const Vector3 &position)
{
	alSource3f(SampleHandle, AL_POSITION, -position.Y, position.Z, position.X);
	if (alGetError() != AL_NO_ERROR) {
		WWDEBUG_SAY(("Failed to set OpenAL source position.\n"));
	}
}

void OpenALHandleClass::Set_Orientation(const Vector3 &facing, const Vector3 &up)
{
	ALfloat orientation[6] = {
		-facing.Y,
		facing.Z,
		facing.X,
		-up.Y,
		up.Z,
		up.X,
	};

	alSourcefv(SampleHandle, AL_ORIENTATION, orientation);
	if (alGetError() != AL_NO_ERROR) {
		WWDEBUG_SAY(("Failed to set OpenAL source orientation.\n"));
	}
}

void OpenALHandleClass::Set_Velocity(const Vector3 &velocity)
{
	alSource3f(SampleHandle, AL_VELOCITY, -velocity.Y, velocity.Z, velocity.X);
	if (alGetError() != AL_NO_ERROR) {
		WWDEBUG_SAY(("Failed to set OpenAL source velocity.\n"));
	}
}

void OpenALHandleClass::Set_Dropoff(float max, float min)
{
	alSourcef(SampleHandle, AL_MAX_DISTANCE, max);
	if (alGetError() != AL_NO_ERROR) {
		WWDEBUG_SAY(("Failed to set OpenAL source max distance.\n"));
	}

	alSourcef(SampleHandle, AL_REFERENCE_DISTANCE, (min > 1.0F) ? min : 1.0F);
	if (alGetError() != AL_NO_ERROR) {
		WWDEBUG_SAY(("Failed to set OpenAL source reference distance.\n"));
	}
}

void OpenALHandleClass::Set_Effect_Level(float /* level */)
{
}

void OpenALHandleClass::Initialize_Reverb()
{
}

void OpenALHandleClass::Queue_Audio()
{
	if (SampleEnded || Buffer == nullptr || !Buffer->Is_Streaming()) {
		return;
	}

	ALint processed = 0;
	alGetSourcei(SampleHandle, AL_BUFFERS_PROCESSED, &processed);
	while (processed > 0) {
		ALuint buffer = 0;
		alSourceUnqueueBuffers(SampleHandle, 1, &buffer);
		if (alGetError() != AL_NO_ERROR) {
			break;
		}

		for (ALsizei index = 0; index < OAL_BUFFER_COUNT; ++index) {
			if (SampleBuffers[index] == buffer) {
				SamplePositionMs += Buffer_Duration_MS(StreamBufferLength[index]);
				StreamBufferLength[index] = 0;
				break;
			}
		}

		if (Queue_Stream_Buffer(buffer)) {
			// Buffer was reused immediately.
		}
		--processed;
	}

	ALint queued = 0;
	alGetSourcei(SampleHandle, AL_BUFFERS_QUEUED, &queued);
	if (queued == 0 && StreamEOF) {
		SampleEnded = true;
		Stream.Close();
		return;
	}

	ALint state = AL_STOPPED;
	alGetSourcei(SampleHandle, AL_SOURCE_STATE, &state);
	if (queued > 0 && state == AL_STOPPED) {
		alSourcePlay(SampleHandle);
	}
}

bool OpenALHandleClass::Queue_Stream_Buffer(ALuint buffer)
{
	if (Buffer == nullptr || !Buffer->Is_Streaming() || StreamEOF) {
		return false;
	}

	unsigned int read = Stream.Read(StreamScratch.data(), static_cast<unsigned int>(StreamScratch.size()));
	while (read == 0 && Stream.Is_EOF()) {
		if (Is_Infinite_Loop()) {
			Stream.Rewind();
		} else if (SampleLoopCount > 1) {
			--SampleLoopCount;
			Stream.Rewind();
		} else {
			StreamEOF = true;
			return false;
		}

		read = Stream.Read(StreamScratch.data(), static_cast<unsigned int>(StreamScratch.size()));
	}

	if (read == 0) {
		return false;
	}

	alBufferData(
		buffer,
		OpenALAudioClass::Get_AL_Format(Buffer->Get_Channels(), Buffer->Get_Bits()),
		StreamScratch.data(),
		read,
		Buffer->Get_Rate());
	if (alGetError() != AL_NO_ERROR) {
		WWDEBUG_SAY(("Failed to buffer data for streaming sample.\n"));
		StreamEOF = true;
		return false;
	}

	alSourceQueueBuffers(SampleHandle, 1, &buffer);
	if (alGetError() != AL_NO_ERROR) {
		WWDEBUG_SAY(("Failed to queue streaming sample buffer.\n"));
		StreamEOF = true;
		return false;
	}

	for (ALsizei index = 0; index < OAL_BUFFER_COUNT; ++index) {
		if (SampleBuffers[index] == buffer) {
			StreamBufferLength[index] = read;
			break;
		}
	}

	return true;
}

void OpenALHandleClass::Reset_Source()
{
	if (SampleHandle == INVALID_OAL_HANDLE) {
		return;
	}

	alSourceStop(SampleHandle);

	ALint queued = 0;
	alGetSourcei(SampleHandle, AL_BUFFERS_QUEUED, &queued);
	while (queued > 0) {
		ALuint buffer = 0;
		alSourceUnqueueBuffers(SampleHandle, 1, &buffer);
		if (alGetError() != AL_NO_ERROR) {
			break;
		}
		--queued;
	}

	Clear_Static_Buffer();
	Reset_Stream_State();
}

void OpenALHandleClass::Upload_Static_Buffer()
{
	if (Buffer == nullptr || Buffer->Is_Streaming() || Buffer->Get_Raw_Length() == 0) {
		return;
	}

	alBufferData(
		SampleBuffers[0],
		OpenALAudioClass::Get_AL_Format(Buffer->Get_Channels(), Buffer->Get_Bits()),
		Buffer->Get_Raw_Buffer(),
		Buffer->Get_Raw_Length(),
		Buffer->Get_Rate());
	if (alGetError() != AL_NO_ERROR) {
		WWDEBUG_SAY(("Failed to buffer static sample.\n"));
		return;
	}

	alSourcei(SampleHandle, AL_BUFFER, SampleBuffers[0]);
	alSourcei(SampleHandle, AL_LOOPING, Is_Infinite_Loop() ? AL_TRUE : AL_FALSE);
}

void OpenALHandleClass::Clear_Static_Buffer()
{
	alSourcei(SampleHandle, AL_BUFFER, AL_NONE);
	alSourcei(SampleHandle, AL_LOOPING, AL_FALSE);
}

void OpenALHandleClass::Reset_Stream_State()
{
	Stream.Close();
	SampleBufferIndex = 0;
	SamplePositionMs = 0;
	StreamEOF = false;
	std::fill(StreamBufferLength, StreamBufferLength + OAL_BUFFER_COUNT, 0u);
}

unsigned int OpenALHandleClass::Buffer_Duration_MS(unsigned int bytes) const
{
	if (Buffer == nullptr || Buffer->Get_Rate() == 0 || Buffer->Get_Channels() == 0 || Buffer->Get_Bits() == 0) {
		return 0;
	}

	const unsigned int bytes_per_second = Buffer->Get_Rate() * Buffer->Get_Channels() * (Buffer->Get_Bits() / 8);
	return bytes_per_second == 0 ? 0 : static_cast<unsigned int>((static_cast<uint64_t>(bytes) * 1000ULL) / bytes_per_second);
}

void OpenALHandleClass::Set_Sample_User(ALuint handle, AudibleSoundClass *user)
{
	if (handle == INVALID_OAL_HANDLE) {
		return;
	}

	if (user == nullptr) {
		SampleUsers.erase(handle);
	} else {
		SampleUsers[handle] = user;
	}
}

AudibleSoundClass *OpenALHandleClass::Get_Sample_User(ALuint handle)
{
	const auto iter = SampleUsers.find(handle);
	return iter == SampleUsers.end() ? nullptr : iter->second;
}
