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
#include "FFMpegBuffer.h"

std::unordered_map<ALuint, AudibleSoundClass *> OpenALHandleClass::SampleUsers;

//////////////////////////////////////////////////////////////////////
//
//	OpenALHandleClass
//
//////////////////////////////////////////////////////////////////////
OpenALHandleClass::OpenALHandleClass()
	: SampleHandle(INVALID_OAL_HANDLE),
	SampleBufferIndex(0),
	SampleLoopCount(0),
	SampleUnqueuedTime(0.0f),
	SampleEnded(true)
{
	alGenBuffers(OAL_BUFFER_COUNT, SampleBuffers);

	if(alGetError() != AL_NO_ERROR) {
		WWDEBUG_SAY(("Failed to generate OpenAL buffer.\n"));
	}
}


//////////////////////////////////////////////////////////////////////
//
//	~OpenALHandleClass
//
//////////////////////////////////////////////////////////////////////
OpenALHandleClass::~OpenALHandleClass()
{
	// Unbind any buffers before deleting the object.
	alSourcei(SampleHandle, AL_BUFFER, AL_NONE);
	alDeleteBuffers(OAL_BUFFER_COUNT, SampleBuffers);
}


//////////////////////////////////////////////////////////////////////
//
//	Initialize
//
//////////////////////////////////////////////////////////////////////
void
OpenALHandleClass::Initialize(SoundBufferClass *buffer)
{
	SoundHandleClass::Initialize(buffer);
	SampleLoopCount = 0;
	// Stop source and unbind any existing buffers from this source.
	alSourceStop(SampleHandle);
	alSourcei(SampleHandle, AL_BUFFER, AL_NONE);
}


//////////////////////////////////////////////////////////////////////
//
//	Start_Sample
//
//////////////////////////////////////////////////////////////////////
void
OpenALHandleClass::Start_Sample()
{
	SampleEnded = false;
	reinterpret_cast<FFMpegBufferClass *>(Buffer)->Reset_Buffer();
	OpenALHandleClass::Queue_Audio();
	alSourcePlay(SampleHandle);

	if(alGetError() != AL_NO_ERROR) {
		WWDEBUG_SAY(("Couldn't play source.\n"));
	}
}


//////////////////////////////////////////////////////////////////////
//
//	Stop_Sample
//
//////////////////////////////////////////////////////////////////////
void
OpenALHandleClass::Stop_Sample()
{
	alSourcePause(SampleHandle);

	if(alGetError() != AL_NO_ERROR) {
		WWDEBUG_SAY(("Couldn't pause source.\n"));
	}
}


//////////////////////////////////////////////////////////////////////
//
//	Resume_Sample
//
//////////////////////////////////////////////////////////////////////
void
OpenALHandleClass::Resume_Sample()
{
	alSourcePlay(SampleHandle);

	if(alGetError() != AL_NO_ERROR) {
		WWDEBUG_SAY(("Couldn't resume source.\n"));
	}
}


//////////////////////////////////////////////////////////////////////
//
//	End_Sample
//
//////////////////////////////////////////////////////////////////////
void
OpenALHandleClass::End_Sample()
{
	//
	//	Stop the sample and then release our hold on the stream handle
	//
	SampleEnded = true;
	alSourceStop(SampleHandle);

	if(alGetError() != AL_NO_ERROR) {
		WWDEBUG_SAY(("Couldn't stop source.\n"));
	}

	// Unbind any buffers.
	alSourcei(SampleHandle, AL_BUFFER, AL_NONE);
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Sample_Pan
//
//////////////////////////////////////////////////////////////////////
void
OpenALHandleClass::Set_Sample_Pan(float pan)
{
	ALfloat location[3] = {pan, 0.0F, 0.0F };
	alSourcefv(SampleHandle, AL_POSITION, location);

	if(alGetError() != AL_NO_ERROR) {
		WWDEBUG_SAY(("Couldn't set source pan.\n"));
	}
}


//////////////////////////////////////////////////////////////////////
//
//	Get_Sample_Pan
//
//////////////////////////////////////////////////////////////////////
float
OpenALHandleClass::Get_Sample_Pan()
{
	ALfloat location[3] = { 0 };
	alGetSourcefv(SampleHandle, AL_POSITION, location);

	if(alGetError() != AL_NO_ERROR) {
		WWDEBUG_SAY(("Couldn't get source pan.\n"));
	}

	return location[0];
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Sample_Volume
//
//////////////////////////////////////////////////////////////////////
void
OpenALHandleClass::Set_Sample_Volume(float volume)
{
	alSourcef(SampleHandle, AL_GAIN, volume);

	if(alGetError() != AL_NO_ERROR) {
		WWDEBUG_SAY(("OpenALHandleClass::Set_Sample_Volume couldn't set source gain.\n"));
	}
}


//////////////////////////////////////////////////////////////////////
//
//	Get_Sample_Volume
//
//////////////////////////////////////////////////////////////////////
float
OpenALHandleClass::Get_Sample_Volume()
{
	ALfloat state = 0.0F;
	alGetSourcef(SampleHandle, AL_GAIN, &state);

	if(alGetError() != AL_NO_ERROR) {
		return 0.0F;
	}

	return state;
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Sample_Loop_Count
//
//////////////////////////////////////////////////////////////////////
void
OpenALHandleClass::Set_Sample_Loop_Count(unsigned count)
{
	WWDEBUG_SAY(("Stream %s requested to loop %u times.\n", Buffer->Get_Filename(), count));
	// count 0 is special and is supposed to mean infinite... best we can do is UINT_MAX or "lots".
	SampleLoopCount = count == 0 ? UINT_MAX : count;
}


//////////////////////////////////////////////////////////////////////
//
//	Get_Sample_Loop_Count
//
//////////////////////////////////////////////////////////////////////
unsigned
OpenALHandleClass::Get_Sample_Loop_Count()
{
	return SampleLoopCount;
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Sample_MS_Position
//
//////////////////////////////////////////////////////////////////////
void
OpenALHandleClass::Set_Sample_MS_Position(unsigned /* ms */)
{
	// TODO
}

void
OpenALHandleClass::Update_Position()
{
	// Corrects the Samples that have been unqueued in the case the audio has looped.
	while (SampleUnqueuedTime > Buffer->Get_Duration()) {
		SampleUnqueuedTime -= Buffer->Get_Duration() / 1000.0f;
	}
}

//////////////////////////////////////////////////////////////////////
//
//	Get_Sample_MS_Position
//
//////////////////////////////////////////////////////////////////////
void
OpenALHandleClass::Get_Sample_MS_Position(int *len, int *pos)
{
	if (len != nullptr) {
		*len = static_cast<int>(Buffer->Get_Duration());
	}

	if (pos != nullptr) {
		Update_Position();
		ALfloat current_position;
		alGetSourcef(SampleHandle, AL_SEC_OFFSET, &current_position);
		*pos = static_cast<int>((SampleUnqueuedTime + current_position) * 1000.0f);
	}
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Sample_User_Data
//
//////////////////////////////////////////////////////////////////////
void
OpenALHandleClass::Set_Sample_User_Data(int /* i */, void *val)
{
	ALint state;
	alGetSourcei(SampleHandle, AL_SOURCE_STATE, &state);

	if(alGetError() == AL_NO_ERROR) {
		Set_Sample_User(SampleHandle, static_cast<AudibleSoundClass *>(val));
	}
}


//////////////////////////////////////////////////////////////////////
//
//	Get_Sample_User_Data
//
//////////////////////////////////////////////////////////////////////
void *
OpenALHandleClass::Get_Sample_User_Data(int /* i */)
{
	void *retval = nullptr;
	ALint state;
	alGetSourcei(SampleHandle, AL_SOURCE_STATE, &state);

	if(alGetError() == AL_NO_ERROR) {
		retval = Get_Sample_User(SampleHandle);
	}

	return retval;
}


//////////////////////////////////////////////////////////////////////
//
//	Get_Sample_Pitch
//
//////////////////////////////////////////////////////////////////////
float
OpenALHandleClass::Get_Sample_Pitch()
{	
	ALfloat pitch;
	alGetSourcef(SampleHandle, AL_PITCH, &pitch);

	if(alGetError() != AL_NO_ERROR) {
		WWDEBUG_SAY(("Failed to retrieve OpenAL source pitch.\n"));
	}

	return pitch;
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Sample_Pitch_Factor
//
//////////////////////////////////////////////////////////////////////
void
OpenALHandleClass::Set_Sample_Pitch(float pitch)
{
	alSourcef(SampleHandle, AL_PITCH, pitch);

	if(alGetError() != AL_NO_ERROR) {
		WWDEBUG_SAY(("Failed to set OpenAL source pitch.\n"));
	}
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Miles_Handle
//
//////////////////////////////////////////////////////////////////////
void
OpenALHandleClass::Set_Miles_Handle(void *handle)
{
	SampleHandle = ALuint(reinterpret_cast<uintptr_t>(handle));
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Position
//
//////////////////////////////////////////////////////////////////////
void OpenALHandleClass::Set_Position(const Vector3 &position)
{
	// TODO, Confirm that OpenAL has Z negated in comparison to Miles .
	alSource3f(SampleHandle, AL_POSITION, -position.Y, position.Z, -position.X);
	
	if(alGetError() != AL_NO_ERROR) {
		WWDEBUG_SAY(("Failed to set OpenAL source position.\n"));
	}
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Orientation
//
//////////////////////////////////////////////////////////////////////
void OpenALHandleClass::Set_Orientation(const Vector3 &facing, const Vector3 &up)
{
	ALfloat orientation[6] = {
		-facing.Y,
		facing.Z,
		-facing.X,
		-up.Y,
		up.Z,
		-up.X,
	};

	alSourcefv(SampleHandle, AL_ORIENTATION, orientation);
	
	if(alGetError() != AL_NO_ERROR) {
		WWDEBUG_SAY(("Failed to set OpenAL source orientation.\n"));
	}
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Velocity
//
//////////////////////////////////////////////////////////////////////
void OpenALHandleClass::Set_Velocity(const Vector3 &velocity)
{
	// TODO, Confirm that OpenAL has Z negated in comparison to Miles .
	alSource3f(SampleHandle, AL_VELOCITY, -velocity.Y, velocity.Z, -velocity.X);
	
	if(alGetError() != AL_NO_ERROR) {
		WWDEBUG_SAY(("Failed to set OpenAL source velocity.\n"));
	}
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Dropoff
//
//////////////////////////////////////////////////////////////////////
void OpenALHandleClass::Set_Dropoff(float max, float min)
{
	alSourcef(SampleHandle, AL_MAX_DISTANCE, max);

	if(alGetError() != AL_NO_ERROR) {
		WWDEBUG_SAY(("Failed to set OpenAL source max distance.\n"));
	}

	alSourcef(SampleHandle, AL_REFERENCE_DISTANCE, (min > 1.0F) ? min : 1.0F);

	if(alGetError() != AL_NO_ERROR) {
		WWDEBUG_SAY(("Failed to set OpenAL source reference distance.\n"));
	}
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Dropoff
//
//////////////////////////////////////////////////////////////////////
void OpenALHandleClass::Set_Effect_Level(float /* level */)
{
	// TODO, figure out appropriate code for OpenAL.
}


//////////////////////////////////////////////////////////////////////
//
//	Initialize_Reverb
//
//////////////////////////////////////////////////////////////////////
void OpenALHandleClass::Initialize_Reverb()
{
	// TODO, figure out appropriate code for OpenAL.
}

void OpenALHandleClass::Queue_Audio()
{
	if (SampleEnded) {
		return;
	}

	// Get current position before unqueuing.
	ALfloat current_position;
	alGetSourcef(SampleHandle, AL_SEC_OFFSET, &current_position);

	// Unqueue any finished buffers.
	ALint processed;
	alGetSourcei(SampleHandle, AL_BUFFERS_PROCESSED, &processed);
	while(processed > 0) {
			ALuint buffer;
			alSourceUnqueueBuffers(SampleHandle, 1, &buffer);

			// Track how many bytes we have processed.
			ALfloat new_position;
			alGetSourcef(SampleHandle, AL_SEC_OFFSET, &new_position);
			SampleUnqueuedTime += new_position - current_position;
			processed--;
	}

	// Check if we can buffer any more data at this time.
	ALint num_queued;
	alGetSourcei(SampleHandle, AL_BUFFERS_QUEUED, &num_queued);
	if(num_queued >= OAL_BUFFER_COUNT) {
			return;
	}

	// We have a valid buffer and need to loop at least once, attempt to read and buffer some audio.
	if(Buffer != NULL && SampleLoopCount != 0)
	{
		bool more_data = reinterpret_cast<FFMpegBufferClass *>(Buffer)->Refresh_Buffer();

		alGetError();
		alBufferData(
			SampleBuffers[SampleBufferIndex],
			OpenALAudioClass::Get_AL_Format(Buffer->Get_Channels(),
			Buffer->Get_Bits()),
			Buffer->Get_Raw_Buffer(),
			Buffer->Get_Raw_Length(),
			Buffer->Get_Rate());
		
		if(alGetError() != AL_NO_ERROR) {
			WWDEBUG_SAY(("Failed to buffer data for streaming sample\n"));
			return;
		}

		alSourceQueueBuffers(SampleHandle, 1, &SampleBuffers[SampleBufferIndex]);
		
		if(alGetError() != AL_NO_ERROR) {
			WWDEBUG_SAY(("Failed to bind buffer for streaming sample\n"));
			return;
		}
		
		// Check if we are intentionally paused rather than stopped.
		ALint state;
		alGetSourcei(SampleHandle, AL_SOURCE_STATE, &state);

		if (state != AL_PAUSED) {
			// Incase sample is so short it finished before Queue_Audio was called again. No op if already playing.
			alSourcePlay(SampleHandle);
		}

		++SampleBufferIndex;
		if(SampleBufferIndex >= OAL_BUFFER_COUNT) {
			SampleBufferIndex = 0;
		}

		if(!more_data) {
			--SampleLoopCount;
			reinterpret_cast<FFMpegBufferClass *>(Buffer)->Reset_Buffer();
		}
	}
}