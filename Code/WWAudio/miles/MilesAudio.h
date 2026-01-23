/*
**	Command & Conquer Renegade(tm)
**	Copyright 2025 Electronic Arts Inc.
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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : WWAudio.h                                                    *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/WWAudio/WWAudio.h          $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/30/02 2:47p                                               $*
 *                                                                                             *
 *                    $Revision:: 31                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __MILESAUDIO_H
#define __MILESAUDIO_H

#include "WWAudio.h"
#include <mss.h>

typedef struct _DRIVER_INFO_STRUCT
{
	char *		name;
	HPROVIDER	driver;
} DRIVER_INFO_STRUCT;


/////////////////////////////////////////////////////////////////////////////////
//	Constants
/////////////////////////////////////////////////////////////////////////////////
enum
{
	MAX_CACHE_HASH		= 256,
	CACHE_HASH_MASK	= 0x000000FF
};

/////////////////////////////////////////////////////////////////////////////////
//
//	MilesAudioClass
//
//	Main controlling entity for all music and sound effects in a game.  Used
// to:
//
//		-- Select hardware devices
//		-- Modify quality preferences
//		-- Modify global volume settings
//		-- Allocate new sounds
//		-- Cache reuseable sounds
//		-- Play music and sound effects
//
//
/////////////////////////////////////////////////////////////////////////////////
class MilesAudioClass final : public WWAudioClass
{
public:
	//////////////////////////////////////////////////////////////////////
	//	Friend classes
	//////////////////////////////////////////////////////////////////////
	friend class AudibleSoundClass;
	friend class Sound3DClass;
	friend class Listener3DClass;

	//////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////////////////
	MilesAudioClass (bool lite = false);
	~MilesAudioClass (void) override;

	//////////////////////////////////////////////////////////////////////
	//	Initialization methods
	//////////////////////////////////////////////////////////////////////

	//
	//	Note:  After call Initialize () you can begin using the library, you don't
	// need to explicity call Open_2D_Device () or Select_3D_Device ().  Those
	// methods were provided as a means of opening devices other than the default.
	//
	//	The Initialize () method defaults to a stereo, 16bit, 44100hz 2D DirectSound
	// driver and a RSX 3D provider.  If RSX isn't available, it trys A3D, then
	//	EAX, then D3DSound, then whatever driver is first available.
	//
	void					Initialize (bool stereo = true, int bits = 16, int hertz = 44100) override;
	void					Initialize (const char *registry_subkey_name) override;
	void					Shutdown (void) override;

	//////////////////////////////////////////////////////////////////////
	//	Driver methods
	//////////////////////////////////////////////////////////////////////
	HDIGDRIVER				Get_2D_Driver (void) const			{ return m_Driver2D; }
	HPROVIDER				Get_3D_Driver (void) const			{ return m_Driver3D; }
	const StringClass &	Get_3D_Driver_Name (void) const override	{ return m_Driver3DName; }

	//////////////////////////////////////////////////////////////////////
	//	2D Hardware/driver selection methods
	//////////////////////////////////////////////////////////////////////
	DRIVER_TYPE_2D		Open_2D_Device (bool stereo, int bits, int hertz) override;
	bool					Close_2D_Device (void);

	//////////////////////////////////////////////////////////////////////
	//	3D Hardware/driver selection methods
	//////////////////////////////////////////////////////////////////////

	// Device information
	int					Get_3D_Device_Count (void) const override								{ return m_Driver3DList.Count (); }
	bool					Get_3D_Device (int index, const char **info) override	{ (*info) = m_Driver3DList[index]->name; return true; }
	int					Find_3D_Device (DRIVER_TYPE_3D type);

	// Device selection
	bool					Select_3D_Device (int index);
	bool					Select_3D_Device (const char *device_name, HPROVIDER provider);
	bool					Select_3D_Device (DRIVER_TYPE_3D type);
	bool					Select_3D_Device (const char *device_name) override;
	bool					Close_3D_Device (void);

	//////////////////////////////////////////////////////////////////////
	//	Speaker configuration
	//////////////////////////////////////////////////////////////////////

	//
	//	See MSS.H for a list of speaker types
	//
	//	At the time of this documentation the speaker types were:
	//		#define AIL_3D_2_SPEAKER  0
	//		#define AIL_3D_HEADPHONE  1
	//		#define AIL_3D_SURROUND   2
	//		#define AIL_3D_4_SPEAKER  3
	//
	void					Set_Speaker_Type (int speaker_type) override;
	int					Get_Speaker_Type (void) const override;

	//////////////////////////////////////////////////////////////////////
	//	Preference settings methods
	//////////////////////////////////////////////////////////////////////
	bool					Set_Max_2D_Sample_Count (int count = DEF_2D_SAMPLE_COUNT);
	int					Get_Max_2D_Sample_Count (void) const;
	int					Get_Avail_2D_Sample_Count (void) const;

	bool					Set_Max_3D_Sample_Count (int count = DEF_3D_SAMPLE_COUNT);
	int					Get_Max_3D_Sample_Count (void) const;
	int					Get_Avail_3D_Sample_Count (void) const;

	//
	//	Reverb Support:  Only works with Create Labs EAX chipset.
	//
	float					Get_Effects_Level (void) override	{ return m_EffectsLevel; }

	//////////////////////////////////////////////////////////////////////
	//	Cache methods
	//////////////////////////////////////////////////////////////////////

	void					Flush_Cache (void) override;

	//
	//	Debug support for determine what sounds are playing on which "channels"
	//
	int						Get_2D_Sample_Count (void) const override
	{ 
		return m_2DSampleHandles.Count ();
	}
	int						Get_3D_Sample_Count (void) const override
	{
		return m_3DSampleHandles.Count ();
	}
	AudibleSoundClass *	Peek_2D_Sample (int index) override;
	AudibleSoundClass *	Peek_3D_Sample (int index) override;

protected:

	//////////////////////////////////////////////////////////////////////
	//	Protected methods
	//////////////////////////////////////////////////////////////////////
	void						Build_3D_Driver_List (void);
	void						Free_3D_Driver_List (void);
	bool						Validate_3D_Sound_Buffer (SoundBufferClass *buffer) override;
	FileClass *				Get_File (const char* filename);
	void						Return_File (FileClass *file);

	//////////////////////////////////////////////////////////////////////
	//	Handle management
	//////////////////////////////////////////////////////////////////////
	void						Allocate_2D_Handles (void);
	void						Release_2D_Handles (void);
	void						Allocate_3D_Handles (void);
	void						Release_3D_Handles (void);
	void						ReAssign_2D_Handles (void);
	void						ReAssign_3D_Handles (void);
	void						Remove_2D_Sound_Handles (void);
	void						Remove_3D_Sound_Handles (void);
	SoundHandleClass *			Get_2D_Handle (const AudibleSoundClass &sound_obj, bool streaming) override;
	SoundHandleClass *			Get_3D_Handle (const Sound3DClass &sound_obj) override;

	//////////////////////////////////////////////////////////////////////
	//	Cache methods
	//////////////////////////////////////////////////////////////////////
	SoundBufferClass *	Get_Sound_Buffer (const char *filename, bool is_3d) override;
	bool						Free_Cache_Space (int bytes);

private:
	DRIVER_TYPE_2D		Open_2D_Device (WAVEFORMAT *format = NULL);
	
	//////////////////////////////////////////////////////////////////////
	//	Registry settings
	//////////////////////////////////////////////////////////////////////
	bool					Load_From_Registry (const char *subkey_name);
	bool					Save_To_Registry (const char *subkey_name);

	SoundBufferClass *	Create_Sound_Buffer (FileClass &file, const char *string_id, bool is_3d);
	SoundBufferClass *	Find_Cached_Buffer (const char *string_id);
	bool						Cache_Buffer (SoundBufferClass *buffer, const char *string_id);
	
	//////////////////////////////////////////////////////////////////////
	//	Miles File Callbacks
	//////////////////////////////////////////////////////////////////////
	static U32 AILCALLBACK	File_Open_Callback (char const *filename, void **file_handle);
	static void AILCALLBACK	File_Close_Callback (void *file_handle);
	static S32 AILCALLBACK	File_Seek_Callback (void *file_handle, S32 offset, U32 type);
	static U32 AILCALLBACK	File_Read_Callback (void *file_handle, void *buffer, U32 bytes);

private:
	//////////////////////////////////////////////////////////////////////
	//	Private data types
	//////////////////////////////////////////////////////////////////////
	typedef struct _CACHE_ENTRY_STRUCT
	{
		char *					string_id;
		SoundBufferClass *	buffer;

		_CACHE_ENTRY_STRUCT (void)
			: string_id (0), buffer (NULL) {}

		_CACHE_ENTRY_STRUCT &operator= (const _CACHE_ENTRY_STRUCT &src) { string_id = ::strdup (src.string_id); REF_PTR_SET (buffer, src.buffer); return *this; }
		bool operator== (const _CACHE_ENTRY_STRUCT &/* src*/) { return false; }
		bool operator!= (const _CACHE_ENTRY_STRUCT &/* src*/) { return true; }
	} CACHE_ENTRY_STRUCT;

	//////////////////////////////////////////////////////////////////////
	//	Private member data
	//////////////////////////////////////////////////////////////////////
	int													m_Max2DSamples;
	int													m_Max3DSamples;
	int													m_Max2DBufferSize;
	int													m_Max3DBufferSize;

	//	Driver information
	HDIGDRIVER											m_Driver2D;
	HPROVIDER											m_Driver3D;
	HPROVIDER											m_Driver3DPseudo;
	DynamicVectorClass<DRIVER_INFO_STRUCT *>	m_Driver3DList;
	StringClass											m_Driver3DName;
	int													m_SpeakerType;

	// Available sample handles
	DynamicVectorClass<HSAMPLE>					m_2DSampleHandles;
	DynamicVectorClass<H3DSAMPLE>					m_3DSampleHandles;

	// Buffer caching
	DynamicVectorClass<CACHE_ENTRY_STRUCT>		m_CachedBuffers[MAX_CACHE_HASH];
	int													m_MaxCacheSize;
	int													m_CurrentCacheSize;

	//	Reverb support
	float													m_EffectsLevel;
	int													m_ReverbRoomType;
};


#endif //__MILESAUDIO_H
