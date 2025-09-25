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

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __NULLAUDIO_H
#define __NULLAUDIO_H

#include "WWAudio.h"

class NullAudioClass final : public WWAudioClass
{
public:
	NullAudioClass(bool lite)
		:
			WWAudioClass(true),
			m_DriverName("NullAudio")
	{
		_theInstance = this;
	}
	~NullAudioClass (void) override {}
	void Initialize (bool stereo = true, int bits = 16, int hertz = 44100) override {}
	void Initialize (const char *registry_subkey_name) override {}
	void Shutdown (void) override {}
	const StringClass & Get_3D_Driver_Name (void) const override { return m_DriverName; }
	DRIVER_TYPE_2D Open_2D_Device (bool stereo, int bits, int hertz) override { return DRIVER2D_ERROR; }
	int Get_3D_Device_Count (void) const override { return 0; }
	bool Get_3D_Device (int index, const char **info) override   { (*info) = m_DriverName; return true; }
	bool Select_3D_Device (const char *device_name) override { return false; }
	void Set_Speaker_Type (int speaker_type) override {}
	int Get_Speaker_Type (void) const override { return W3D_3D_2_SPEAKER; }
	float Get_Effects_Level (void) override { return 0.0F; }
	void Flush_Cache (void) override {}
	int Get_2D_Sample_Count (void) const override { return 0;}
	int Get_3D_Sample_Count (void) const override { return 0;}
	AudibleSoundClass *Peek_2D_Sample (int index) override { return nullptr;}
	AudibleSoundClass *Peek_3D_Sample (int index) override { return nullptr;}
	bool Validate_3D_Sound_Buffer (SoundBufferClass *buffer) override { return false; }
	SoundHandleClass *Get_2D_Handle (const AudibleSoundClass &sound_obj, bool streaming) override { return nullptr; }
	SoundHandleClass *Get_3D_Handle (const Sound3DClass &sound_obj) override { return nullptr; }
	SoundBufferClass *Get_Sound_Buffer (const char *filename, bool is_3d) override { return nullptr; }

private:
	StringClass m_DriverName;
};

#endif /* __NULLAUDIO_H */