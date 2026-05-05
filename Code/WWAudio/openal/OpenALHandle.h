/*
**	Command & Conquer Renegade(tm)
**	Copyright 2025 Electronic Arts Inc.
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

#ifndef __OPENALHANDLE_H
#define __OPENALHANDLE_H

#include "FFMpegBuffer.h"
#include "soundhandle.h"

#include <AL/al.h>
#include <unordered_map>
#include <vector>

//////////////////////////////////////////////////////////////////////
//
//	OpenALHandleClass
//
//////////////////////////////////////////////////////////////////////
class OpenALHandleClass final : public SoundHandleClass
{
	friend class OpenALAudioClass;
	static constexpr ALsizei OAL_BUFFER_COUNT = 4;
	static constexpr ALuint INVALID_OAL_HANDLE = UINT_MAX;

public:
	OpenALHandleClass();
	~OpenALHandleClass();

	void Set_Miles_Handle(void *handle) override;
	void Initialize(SoundBufferClass *buffer) override;
	void Start_Sample() override;
	void Stop_Sample() override;
	void Resume_Sample() override;
	void End_Sample() override;
	void Set_Sample_Pan(float pan) override;
	float Get_Sample_Pan() override;
	void Set_Sample_Volume(float volume) override;
	float Get_Sample_Volume() override;
	void Set_Sample_Loop_Count(unsigned count) override;
	unsigned Get_Sample_Loop_Count() override;
	void Set_Sample_MS_Position(unsigned ms) override;
	void Get_Sample_MS_Position(int *len, int *pos) override;
	void Set_Sample_User_Data(int i, void *val) override;
	void *Get_Sample_User_Data(int i) override;
	float Get_Sample_Pitch() override;
	void Set_Sample_Pitch(float pitch) override;

	void Set_Position(const Vector3 &position) override;
	void Set_Orientation(const Vector3 &facing, const Vector3 &up) override;
	void Set_Velocity(const Vector3 &velocity) override;
	void Set_Dropoff(float max, float min) override;
	void Set_Effect_Level(float level) override;
	void Initialize_Reverb() override;

	void Queue_Audio() override;

private:
	bool Queue_Stream_Buffer(ALuint buffer);
	void Reset_Source();
	void Upload_Static_Buffer();
	void Clear_Static_Buffer();
	void Reset_Stream_State();
	unsigned int Buffer_Duration_MS(unsigned int bytes) const;
	bool Is_Infinite_Loop() const { return SampleLoopCount == UINT_MAX; }

	static void Set_Sample_User(ALuint handle, AudibleSoundClass *user = nullptr);
	static AudibleSoundClass *Get_Sample_User(ALuint handle);

private:
	static std::unordered_map<ALuint, AudibleSoundClass *> SampleUsers;

	ALuint SampleHandle;
	ALuint SampleBuffers[OAL_BUFFER_COUNT];
	unsigned SampleBufferIndex;
	unsigned SampleLoopCount;
	unsigned SamplePositionMs;
	unsigned StreamBufferLength[OAL_BUFFER_COUNT];
	bool SampleEnded;
	bool StreamEOF;
	FFMpegAudioStreamClass Stream;
	std::vector<unsigned char> StreamScratch;
};

#endif //__OPENALHANDLE_H
