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

#ifndef __WWAUDIO_H
#define __WWAUDIO_H

#include "always.h"

#include "vector.h"
#include "SoundBuffer.h"
#include "AudioEvents.h"
#include "wwstring.h"

/////////////////////////////////////////////////////////////////////////////////
// Forward declaration
/////////////////////////////////////////////////////////////////////////////////
class AudibleSoundClass;
class Sound3DClass;
class Sound2DTriggerClass;
class StreamSoundClass;
class FileClass;
class SoundSceneClass;
class SoundHandleClass;
class FileFactoryClass;
class SoundSceneObjClass;
class LogicalListenerClass;
class LogicalSoundClass;
class Matrix3D;
class INIClass;

struct WaveFormatStruct {
    unsigned short    wFormatTag;
    unsigned short    nChannels;
    unsigned   nSamplesPerSec;
    unsigned   nAvgBytesPerSec;
    unsigned short    nBlockAlign;
};

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
//  based on this comment we define our own equivalent values for this.
static constexpr int W3D_3D_2_SPEAKER = 0;
static constexpr int W3D_3D_HEADPHONE = 1;
static constexpr int W3D_3D_SURROUND  = 2;
static constexpr int W3D_3D_4_SPEAKER = 3;

/////////////////////////////////////////////////////////////////////////////////
//	Class IDs
/////////////////////////////////////////////////////////////////////////////////
typedef enum
{
	CLASSID_UNKNOWN			= 0,
	CLASSID_2D,
	CLASSID_3D,
	CLASSID_LISTENER,
	CLASSID_PSEUDO3D,
	CLASSID_2DTRIGGER,
	CLASSID_LOGICAL,
	CLASSID_FILTERED,
	CLASSID_COUNT
} SOUND_CLASSID;


/////////////////////////////////////////////////////////////////////////////////
//	Default values
/////////////////////////////////////////////////////////////////////////////////
const int DEF_2D_SAMPLE_COUNT		= 16;
const int DEF_3D_SAMPLE_COUNT		= 16;
const float DEF_MUSIC_VOL			= 1.0F;
const float DEF_SFX_VOL				= 1.0F;
const float DEF_DIALOG_VOL			= 1.0F;
const float DEF_CINEMATIC_VOL		= 1.0F;
const float DEF_FADE_TIME			= 0.5F;
const int DEF_MAX_2D_BUFFER_SIZE	= 20000;
const int DEF_MAX_3D_BUFFER_SIZE	= 100000;


/////////////////////////////////////////////////////////////////////////////////
//
//	WWAudioClass
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
class WWAudioClass
{
public:

	//////////////////////////////////////////////////////////////////////
	//	Public data types
	//////////////////////////////////////////////////////////////////////
	typedef enum
	{
		DRIVER2D_ERROR			= 0,
		DRIVER2D_DSOUND,
		DRIVER2D_WAVEOUT,
		DRIVER2D_OPENAL,
		DRIVER2D_COUNT
	} DRIVER_TYPE_2D;

	typedef enum
	{
		DRIVER3D_ERROR			= 0,
		DRIVER3D_D3DSOUND,
		DRIVER3D_EAX,
		DRIVER3D_A3D,
		DRIVER3D_RSX,
		DRIVER3D_PSEUDO,
		DRIVER3D_DOLBY,
		DRIVER3D_COUNT
	} DRIVER_TYPE_3D;

	typedef enum
	{
		PAGE_PRIMARY		= 0,
		PAGE_SECONDARY,
		PAGE_TERTIARY,
		PAGE_COUNT
	} SOUND_PAGE;

	//////////////////////////////////////////////////////////////////////
	//	Friend classes
	//////////////////////////////////////////////////////////////////////
	friend class AudibleSoundClass;
	friend class Sound3DClass;
	friend class Listener3DClass;

	//////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////////////////
	WWAudioClass(bool lite = false);
	virtual ~WWAudioClass (void);

	//////////////////////////////////////////////////////////////////////
	//	Static methods
	//////////////////////////////////////////////////////////////////////
	static WWAudioClass *	Get_Instance (void)		{ return _theInstance; }
	static WWAudioClass * Create_Instance(bool lite = false); // Implemented in derived class cpp.

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
	virtual void					Initialize (bool stereo = true, int bits = 16, int hertz = 44100) = 0;
	virtual void					Initialize (const char *registry_subkey_name) = 0;
	virtual void					Shutdown (void) = 0;

	//////////////////////////////////////////////////////////////////////
	//	Driver methods
	//////////////////////////////////////////////////////////////////////
	virtual const StringClass &	Get_3D_Driver_Name (void) const	= 0;

	//////////////////////////////////////////////////////////////////////
	//	2D Hardware/driver selection methods
	//////////////////////////////////////////////////////////////////////
	virtual DRIVER_TYPE_2D		Open_2D_Device (bool stereo, int bits, int hertz) = 0;
	int					Get_Playback_Rate (void) const		{ return m_PlaybackRate; }
	int					Get_Playback_Bits (void) const		{ return m_PlaybackBits; }
	bool				Get_Playback_Stereo (void) const		{ return m_PlaybackStereo; }

	//////////////////////////////////////////////////////////////////////
	//	3D Hardware/driver selection methods
	//////////////////////////////////////////////////////////////////////

	// Device information
	virtual int					Get_3D_Device_Count (void) const = 0;
	virtual bool					Get_3D_Device (int index, const char **info) = 0;

	// Device selection
	virtual bool					Select_3D_Device (const char *device_name) = 0;

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
	virtual void					Set_Speaker_Type (int speaker_type) = 0;
	virtual int					Get_Speaker_Type (void) const = 0;

	//////////////////////////////////////////////////////////////////////
	//	Registry settings
	//////////////////////////////////////////////////////////////////////
	bool					Load_From_Registry (const char *subkey_name, StringClass &device_name, bool &is_stereo, int &bits, int &hertz, bool &sound_enabled, bool &music_enabled, bool &dialog_enabled, bool &cinematic_sound_enabled, float &sound_volume, float &music_volume, float &dialog_volume, float &cinematic_volume, int &speaker_types);
	bool					Save_To_Registry (const char *subkey_name, const StringClass &device_name, bool is_stereo, int bits, int hertz, bool sound_enabled, bool music_enabled, bool dialog_enabled, bool cinematic_sound_enabled, float sound_volume, float music_volume, float dialog_volume, float cinematic_volume, int speaker_type);

	//////////////////////////////////////////////////////////////////////
	//	Default settings
	//////////////////////////////////////////////////////////////////////
	void					Load_Default_Volume (int &defaultmusicvolume, int &defaultsoundvolume, int &defaultdialogvolume, int &defaultcinematicvolume);

	//////////////////////////////////////////////////////////////////////
	//	File interface methods
	//////////////////////////////////////////////////////////////////////

	//
	//	Note:  The user is responsible for freeing this file factory, the
	// sound library does not.
	//
	void					Set_File_Factory (FileFactoryClass *ffactory)			{ m_FileFactory = ffactory; }

	//
	//	Reverb Support:  Only works with Create Labs EAX chipset.
	//
	virtual float					Get_Effects_Level (void) = 0;

	//////////////////////////////////////////////////////////////////////
	//	Volume methods
	//////////////////////////////////////////////////////////////////////
	void					Set_Sound_Effects_Volume (float volume = DEF_SFX_VOL);
	float					Get_Sound_Effects_Volume (void) const						{ return m_SoundVolume; }

	void					Set_Music_Volume (float volume = DEF_MUSIC_VOL);
	float					Get_Music_Volume (void) const									{ return m_MusicVolume; }

	void					Set_Dialog_Volume (float volume = DEF_DIALOG_VOL);
	float					Get_Dialog_Volume (void) const								{ return m_DialogVolume; }

	void					Set_Cinematic_Volume (float volume = DEF_CINEMATIC_VOL);
	float					Get_Cinematic_Volume (void) const							{ return m_CinematicVolume; }

	void					Allow_Sound_Effects (bool onoff = true);
	bool					Are_Sound_Effects_On (void) const							{ return m_AreSoundEffectsEnabled; }

	void					Allow_Music (bool onoff = true);
	bool					Is_Music_On (void) const										{ return m_IsMusicEnabled; }

	void					Allow_Dialog (bool onoff = true);
	bool					Is_Dialog_On (void) const										{ return m_IsDialogEnabled; }

	void					Allow_Cinematic_Sound (bool onoff = true);
	bool					Is_Cinematic_Sound_On (void) const							{ return m_IsCinematicSoundEnabled; }

	void					Enable_New_Sounds (bool onoff)			{ m_AreNewSoundsEnabled = onoff; }
	bool					Are_New_Sounds_Enabled (void)	const		{ return m_AreNewSoundsEnabled; }

	void					Temp_Disable_Audio (bool onoff);

	//////////////////////////////////////////////////////////////////////
	//	Update methods
	//////////////////////////////////////////////////////////////////////
	void					On_Frame_Update (unsigned int milliseconds = 0);

	//////////////////////////////////////////////////////////////////////
	//	Callback methods
	//////////////////////////////////////////////////////////////////////
	void					Register_EOS_Callback (LPFNEOSCALLBACK callback, uint32 user_param);
	void					UnRegister_EOS_Callback (LPFNEOSCALLBACK callback);

	void					Register_Text_Callback (LPFNTEXTCALLBACK callback, uint32 user_param);
	void					UnRegister_Text_Callback (LPFNTEXTCALLBACK callback);

	void					Fire_Text_Callback (AudibleSoundClass *sound_obj, const StringClass &text);

	//////////////////////////////////////////////////////////////////////
	//	Sound allocation methods
	//////////////////////////////////////////////////////////////////////

	//
	//	Note:  We differentiate between a 'music' object and
	// a 'sound effect' for volume control.  A music object
	// is simply a sound effect with a 'SOUND_TYPE' of TYPE_MUSIC.
	// When the user adjusts the 'music' volume, we loop through
	// the sound effects marked 'music' and update their volume.
	// Same for 'sound effect' volume.
	//

	//
	//	Note:  The sound data these objects support can be PCM WAV,
	// ADPCM WAV, VOC, or MP3.
	//
	AudibleSoundClass *		Create_Sound_Effect (const char *filename);
	
	//
	//	Note:  3D sound effects should be inserted into the SoundScene.
	// They should not be manually played/stopped/etc.
	//
	//	True 3D sound effects require mono, uncompressed, WAV data.
	// If the supplied data is not in this format the sound will
	// become a pseudo-3D sound.
	//
	//	Pseudo-3D sounds are not true 3D sounds.  They have the
	// same properties as 3D 'sound effects' but do not use 3D
	// hardware, are not restricted to mono, uncompressed, WAV data,
	// and do not calculate doppler and reverb effects.
	//
	Sound3DClass *		Create_3D_Sound (const char *filename, int classid_hint = CLASSID_3D);
	

	//////////////////////////////////////////////////////////////////////
	//	Background music support
	//////////////////////////////////////////////////////////////////////
	void						Set_Background_Music (const char *filename);
	void						Fade_Background_Music (const char *filename, int fade_out_time, int fade_in_time);

	const char *			Get_Background_Music_Name (void)						{ return m_BackgroundMusicName; }
	AudibleSoundClass *	Peek_Background_Music (void)							{ return m_BackgroundMusic; }

	//////////////////////////////////////////////////////////////////////
	//	Logical-sound related methods
	//////////////////////////////////////////////////////////////////////
	LogicalSoundClass *		Create_Logical_Sound (void);
	LogicalListenerClass *	Create_Logical_Listener (void);

	//
	//	Logical type identification for use with the definition system
	//
	void							Add_Logical_Type (int id, const char* display_name);
	void							Reset_Logical_Types (void);
	int							Get_Logical_Type_Count (void) const	{ return m_LogicalTypes.Count (); }
	int							Get_Logical_Type (int index, StringClass &name);

	//////////////////////////////////////////////////////////////////////
	//	Definition related methods
	//////////////////////////////////////////////////////////////////////

	//
	//	Sound creation methods
	//
	int						Create_Instant_Sound (int definition_id, const Matrix3D &tm, RefCountClass *user_obj = nullptr, uint32 user_data = 0, int classid_hint = CLASSID_3D);
	int						Create_Instant_Sound (const char *def_name, const Matrix3D &tm, RefCountClass *user_obj = nullptr, uint32 user_data = 0, int classid_hint = CLASSID_3D);
	AudibleSoundClass *	Create_Continuous_Sound (int definition_id, RefCountClass *user_obj = nullptr, uint32 user_data = 0, int classid_hint = CLASSID_3D);
	AudibleSoundClass *	Create_Continuous_Sound (const char *def_name, RefCountClass *user_obj = nullptr, uint32 user_data = 0, int classid_hint = CLASSID_3D);
	AudibleSoundClass *	Create_Sound (int definition_id, RefCountClass *user_obj = nullptr, uint32 user_data = 0, int classid_hint = CLASSID_3D);
	AudibleSoundClass *	Create_Sound (const char *def_name, RefCountClass *user_obj = nullptr, uint32 user_data = 0, int classid_hint = CLASSID_3D);

	//////////////////////////////////////////////////////////////////////
	//	Sound object lookup
	//////////////////////////////////////////////////////////////////////
	SoundSceneObjClass *	Find_Sound_Object (uint32 sound_obj_id);

	//////////////////////////////////////////////////////////////////////
	//	Sound scene methods (for 3D sounds)
	//////////////////////////////////////////////////////////////////////
	SoundSceneClass *	Get_Sound_Scene (void) const		{ return m_SoundScene; }

	//////////////////////////////////////////////////////////////////////
	//	Cache methods
	//////////////////////////////////////////////////////////////////////

	virtual void					Flush_Cache (void) = 0;

	//////////////////////////////////////////////////////////////////////
	//	Play control methods
	//////////////////////////////////////////////////////////////////////
	bool					Simple_Play_2D_Sound_Effect (const char *filename, float priority = 1.0F, float volume = DEF_SFX_VOL);
	
	//////////////////////////////////////////////////////////////////////
	//	Playlist methods
	//////////////////////////////////////////////////////////////////////
	bool						Add_To_Playlist (AudibleSoundClass *sound);
	bool						Remove_From_Playlist (AudibleSoundClass *sound);
	int						Get_Playlist_Count (void) const			{ return m_Playlist[m_CurrPage].Count (); }
	AudibleSoundClass *	Get_Playlist_Entry (int index) const;
	AudibleSoundClass *	Peek_Playlist_Entry (int index) const	{ return m_Playlist[m_CurrPage][index]; }
	void						Flush_Playlist (void);
	void						Flush_Playlist (SOUND_PAGE page);
	bool						Is_Sound_In_Playlist (AudibleSoundClass *sound_obj);

	//////////////////////////////////////////////////////////////////////
	//	Virtual channel access
	//////////////////////////////////////////////////////////////////////
	bool					Acquire_Virtual_Channel (AudibleSoundClass *sound_obj, int channel_index);
	void					Release_Virtual_Channel (AudibleSoundClass *sound_obj, int channel_index);

	//////////////////////////////////////////////////////////////////////
	//	Sound "page" access
	//////////////////////////////////////////////////////////////////////

	//
	//	Note:  Sound "pages" are simply sets of sounds.  The primary page
	// is the full set of 3D sounds (in the scene) and 2D sounds that
	// are started when the primary page is active.
	//	The secondary page is only the set of 2D sounds that are started
	// when the secondary page is active.
	//
	void					Set_Active_Sound_Page (SOUND_PAGE page);
	SOUND_PAGE			Get_Active_Sound_Page (void)					{ return m_CurrPage; }

	void					Push_Active_Sound_Page (SOUND_PAGE page);
	void					Pop_Active_Sound_Page (void);

	//////////////////////////////////////////////////////////////////////
	//	Debug methods
	//////////////////////////////////////////////////////////////////////
	bool						Is_Disabled (void) const;

	//
	//	Debug support for determine what sounds are playing on which "channels"
	//
	virtual int						Get_2D_Sample_Count (void) const = 0;
	virtual int						Get_3D_Sample_Count (void) const = 0;
	virtual AudibleSoundClass *	Peek_2D_Sample (int index) = 0;
	virtual AudibleSoundClass *	Peek_3D_Sample (int index) = 0;

	// Um somtimes you need to get rid of all the completed sounds without
	// being in the update render function and without totally shutting down
	// the sound system.  This is primarily because completed (non static) sounds
	// still may have a reference to the object they're attached to.
	void					Free_Completed_Sounds (void);

protected:
	void						Reprioritize_Playlist (void);

	// The backend needs to implement this as for example miles only likes pcm wave
	// but other backends might have different constraints.
	virtual bool						Validate_3D_Sound_Buffer (SoundBufferClass *buffer) = 0;

	//////////////////////////////////////////////////////////////////////
	//	Handle management
	//////////////////////////////////////////////////////////////////////
	virtual SoundHandleClass *Get_2D_Handle (const AudibleSoundClass &sound_obj, bool streaming) = 0;
	virtual SoundHandleClass *Get_3D_Handle (const Sound3DClass &sound_obj) = 0;
	bool						Is_OK_To_Give_Handle (const AudibleSoundClass &sound_obj);

	//////////////////////////////////////////////////////////////////////
	//	Dialog/Fade methods
	//////////////////////////////////////////////////////////////////////
	void						Update_Fade (void);
	void						Internal_Set_Sound_Effects_Volume (float volume);
	void						Internal_Set_Music_Volume (float volume);

	//////////////////////////////////////////////////////////////////////
	//	Cache methods
	//////////////////////////////////////////////////////////////////////
	virtual SoundBufferClass *	Get_Sound_Buffer (const char *filename, bool is_3d) = 0;
	
protected:
	typedef struct _LOGICAL_TYPE_STRUCT
	{
		StringClass				display_name;
		int						id;

		_LOGICAL_TYPE_STRUCT (void)
			: id (0) {}

		_LOGICAL_TYPE_STRUCT (int _id, const char* name)
			:	display_name (name), id (_id) {}

		bool operator== (const _LOGICAL_TYPE_STRUCT &/* src*/) { return false; }
		bool operator!= (const _LOGICAL_TYPE_STRUCT &/* src*/) { return true; }
	} LOGICAL_TYPE_STRUCT;


	//////////////////////////////////////////////////////////////////////
	//	Private constants
	//////////////////////////////////////////////////////////////////////
	typedef enum
	{
		FADE_NONE	= 0,
		FADE_IN,
		FADE_OUT,
		FADED_OUT,
	}	FADE_TYPE;

	//////////////////////////////////////////////////////////////////////
	//	Static member data
	//////////////////////////////////////////////////////////////////////
	static WWAudioClass *						_theInstance;

	// Callback lists
	AudioCallbackListClass<LPFNEOSCALLBACK>	m_EOSCallbackList;
	AudioCallbackListClass<LPFNTEXTCALLBACK>	m_TextCallbackList;

	// Logical type management
	DynamicVectorClass<LOGICAL_TYPE_STRUCT>	m_LogicalTypes;

	DynamicVectorClass<SOUND_PAGE> m_PageStack;
	DynamicVectorClass<AudibleSoundClass *>	m_Playlist[PAGE_COUNT];
	DynamicVectorClass<AudibleSoundClass *>	m_CompletedSounds;

	//	Virtual channel support
	DynamicVectorClass<AudibleSoundClass *>	m_VirtualChannels;

	StringClass m_BackgroundMusicName;
	AudibleSoundClass * m_BackgroundMusic;

	FileFactoryClass *m_FileFactory;
	INIClass *AudioIni;
	SoundSceneClass *m_SoundScene;
	SOUND_PAGE m_CurrPage;

	FADE_TYPE m_FadeType;
	float	m_NonDialogFadeTime;
	float	m_FadeTimer;

	float	m_MusicVolume;
	float	m_SoundVolume;
	float	m_RealMusicVolume;
	float	m_RealSoundVolume;
	float	m_DialogVolume;
	float	m_CinematicVolume;
	int		m_PlaybackRate;
	int		m_PlaybackBits;
	bool	m_PlaybackStereo;
	bool	m_IsMusicEnabled;
	bool	m_IsDialogEnabled;
	bool	m_IsCinematicSoundEnabled;
	bool	m_AreSoundEffectsEnabled;
	bool	m_AreNewSoundsEnabled;
	bool	m_ForceDisable;
	bool	m_CachedIsMusicEnabled;
	bool	m_CachedIsDialogEnabled;
	bool	m_CachedIsCinematicSoundEnabled;
	bool	m_CachedAreSoundEffectsEnabled;
};


#endif //__WWAUDIO_H
