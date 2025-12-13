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
#include "WWAudio.h"
#include "AudibleSound.h"
#include "ini.h"
#include "LogicalSound.h"
#include "registry.h"
#include "Sound3D.h"
#include "SoundChunkIDs.h"
#include "SoundPseudo3D.h"
#include "SoundScene.h"
#include "ww3d.h"
#include "wwprofile.h"
#include "wwmemlog.h"
#include "ini.h"
#include "openw3d.h"
#include <algorithm>

////////////////////////////////////////////////////////////////////////////////////////////////
//	Config value names
////////////////////////////////////////////////////////////////////////////////////////////////
static const char VALUE_INI_IS_STEREO[]			= "Stereo";
static const char VALUE_INI_BITS[]				= "Bits";
static const char VALUE_INI_HERTZ[]				= "Hertz";
static const char VALUE_INI_DEVICE_NAME[]		= "DeviceName";
static const char VALUE_INI_MUSIC_ENABLED[]		= "MusicEnabled";
static const char VALUE_INI_SOUND_ENABLED[]		= "SoundEnabled";
static const char VALUE_INI_DIALOG_ENABLED[]	= "DialogEnabled";
static const char VALUE_INI_CINEMATIC_ENABLED[]	= "CinematicEnabled";
static const char VALUE_INI_MUSIC_VOL[]			= "MusicVolume";
static const char VALUE_INI_SOUND_VOL[]			= "SoundVolume";
static const char VALUE_INI_DIALOG_VOL[]		= "DialogVolume";
static const char VALUE_INI_CINEMATIC_VOL[]		= "CinematicVolume";
static const char VALUE_INI_SPEAKER_TYPE[]		= "SpeakerType";

////////////////////////////////////////////////////////////////////////////////////////////////
//	INI names
////////////////////////////////////////////////////////////////////////////////////////////////
static const char WWAUDIO_INI_FILENAME[]				= "WWAudio.ini";
static const char WWAUDIO_INI_RELATIVE_PATHNAME[]	= "Data\\WWAudio.ini";
static const char INI_DEFAULT_VOLUME_SECTION[]		= "Default Volume";
static const char INI_MUSIC_VOLUME_ENTRY[]			= "MUSIC_VOLUME"; 
static const char INI_SOUND_VOLUME_ENTRY[]			= "SOUND_VOLUME";
static const char INI_DIALOG_VOLUME_ENTRY[]			= "DIALOG_VOLUME";
static const char INI_CINEMATIC_VOLUME_ENTRY[]		= "CINEMATIC_VOLUME";

static constexpr int	MAX_VIRTUAL_CHANNELS				= 100;

////////////////////////////////////////////////////////////////////////////////////////////////
//	Static member initialization
////////////////////////////////////////////////////////////////////////////////////////////////
WWAudioClass *WWAudioClass::_theInstance = NULL;

WWAudioClass::WWAudioClass(bool lite)
 : 
	  m_BackgroundMusic (NULL),
	  m_FileFactory (NULL),
		AudioIni(NULL),
	  m_SoundScene (NULL),
	  m_CurrPage (PAGE_PRIMARY),
	  m_FadeType (FADE_NONE),
	  m_NonDialogFadeTime (DEF_FADE_TIME),
	  m_FadeTimer (0),
	  m_MusicVolume (DEF_MUSIC_VOL),
	  m_SoundVolume (DEF_SFX_VOL),
	  m_RealMusicVolume (DEF_MUSIC_VOL),
	  m_RealSoundVolume (DEF_SFX_VOL),
	  m_PlaybackRate (44100),
	  m_PlaybackBits (16),
	  m_PlaybackStereo (true),
	  m_IsMusicEnabled (true),
	  m_IsDialogEnabled (true),
	  m_IsCinematicSoundEnabled (true),
	  m_AreSoundEffectsEnabled (true),
	  m_AreNewSoundsEnabled (true),
		m_ForceDisable(lite),
	  m_CachedIsMusicEnabled (true),
	  m_CachedIsDialogEnabled (true),
	  m_CachedIsCinematicSoundEnabled (true),
	  m_CachedAreSoundEffectsEnabled (true)
{

	//
	// Set some default values
	//
	Set_Sound_Effects_Volume ();
	Set_Music_Volume ();

	//
	//	Allocate the virtual channels
	//
	for (int index = 0; index < MAX_VIRTUAL_CHANNELS; index ++) {
		m_VirtualChannels.Add (NULL);
	}

	// Create a new sound scene to manage our 3D sounds...
	if (!lite) {
		m_SoundScene = new SoundSceneClass;
	}
}

WWAudioClass::~WWAudioClass (void)
{ 
	delete AudioIni;
	_theInstance = NULL;
}

bool
WWAudioClass::Is_OK_To_Give_Handle (const AudibleSoundClass &sound_obj)
{
	bool is_ok = false;
	AudibleSoundClass::SOUND_TYPE type = sound_obj.Get_Type ();
	if (m_AreNewSoundsEnabled) {
		if (((type == AudibleSoundClass::TYPE_SOUND_EFFECT) && m_AreSoundEffectsEnabled) ||
			 ((type == AudibleSoundClass::TYPE_MUSIC) && m_IsMusicEnabled) ||
			 ((type == AudibleSoundClass::TYPE_DIALOG) && m_IsDialogEnabled) ||
			 ((type == AudibleSoundClass::TYPE_CINEMATIC) && m_IsCinematicSoundEnabled))
		{
			is_ok = true;
		}
	}
	return is_ok;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Internal_Set_Sound_Effects_Volume
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
WWAudioClass::Internal_Set_Sound_Effects_Volume (float volume)
{
	m_SoundVolume = volume;
	m_SoundVolume = std::min (1.0F, m_SoundVolume);
	m_SoundVolume = std::max (0.0F, m_SoundVolume);

	// Update all the currently playing 'Sound Effects' to
	// reflect this new volume
	for (int index = 0; index < m_Playlist[m_CurrPage].Count (); index ++) {
		AudibleSoundClass *sound_obj = m_Playlist[m_CurrPage][index];
		if (sound_obj->Get_Type () == AudibleSoundClass::TYPE_SOUND_EFFECT) {
			sound_obj->Update_Volume ();
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Internal_Set_Music_Volume
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
WWAudioClass::Internal_Set_Music_Volume (float volume)
{
	m_MusicVolume = volume;
	m_MusicVolume = std::min (1.0F, m_MusicVolume);
	m_MusicVolume = std::max (0.0F, m_MusicVolume);

	// Update all currently playing music to
	// reflect this new volume
	for (int index = 0; index < m_Playlist[m_CurrPage].Count (); index ++) {
		AudibleSoundClass *sound_obj = m_Playlist[m_CurrPage][index];
		if (sound_obj->Get_Type () == AudibleSoundClass::TYPE_MUSIC) {
			sound_obj->Update_Volume ();
		}
	}

	return;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Allow_Sound_Effects
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
WWAudioClass::Allow_Sound_Effects (bool onoff)
{
	//
	// Is the state changing?
	//
	if (m_AreSoundEffectsEnabled != onoff) {
		m_AreSoundEffectsEnabled = onoff;

		//
		// Update all the currently playing 'Sound Effects' to
		// reflect this new state.
		//
		if (m_AreSoundEffectsEnabled) {

			for (int page = 0; page < PAGE_COUNT; page ++) {
				Push_Active_Sound_Page ((WWAudioClass::SOUND_PAGE)page);
				for (int index = 0; index < m_Playlist[page].Count (); index ++) {
					AudibleSoundClass *sound_obj = m_Playlist[page][index];
					if (sound_obj->Get_Type () == AudibleSoundClass::TYPE_SOUND_EFFECT) {
						sound_obj->Allocate_Miles_Handle ();
					}
				}
				Pop_Active_Sound_Page ();
			}
		} else {

			for (int page = 0; page < PAGE_COUNT; page ++) {
				for (int index = 0; index < m_Playlist[page].Count (); index ++) {
					AudibleSoundClass *sound_obj = m_Playlist[page][index];
					if (sound_obj->Get_Type () == AudibleSoundClass::TYPE_SOUND_EFFECT) {
						sound_obj->Free_Miles_Handle ();
					}
				}
			}
		}
	}

	return;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Allow_Music
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
WWAudioClass::Allow_Music (bool onoff)
{
	// Is the state changing?
	if (m_IsMusicEnabled != onoff) {
		m_IsMusicEnabled = onoff;

		//
		// Update all the currently playing 'music tracks' to
		// reflect this new state.
		//
		if (m_IsMusicEnabled) {

			for (int page = 0; page < PAGE_COUNT; page ++) {
				Push_Active_Sound_Page ((WWAudioClass::SOUND_PAGE)page);
				for (int index = 0; index < m_Playlist[page].Count (); index ++) {
					AudibleSoundClass *sound_obj = m_Playlist[page][index];
					if (sound_obj->Get_Type () == AudibleSoundClass::TYPE_MUSIC) {
						sound_obj->Stop (false);
						sound_obj->Play ();
					}
				}
				Pop_Active_Sound_Page ();
			}

		} else {

			for (int page = 0; page < PAGE_COUNT; page ++) {
				for (int index = 0; index < m_Playlist[page].Count (); index ++) {
					AudibleSoundClass *sound_obj = m_Playlist[page][index];
					if (sound_obj->Get_Type () == AudibleSoundClass::TYPE_MUSIC) {
						sound_obj->Free_Miles_Handle ();
					}
				}
			}
		}
	}

	return;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Allow_Dialog
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
WWAudioClass::Allow_Dialog (bool onoff)
{
	// Is the state changing?
	if (m_IsDialogEnabled != onoff) {
		m_IsDialogEnabled = onoff;

		//
		// Update all the currently playing 'dialog' to
		// reflect this new state.
		//
		if (m_IsDialogEnabled) {

			for (int page = 0; page < PAGE_COUNT; page ++) {
				Push_Active_Sound_Page ((WWAudioClass::SOUND_PAGE)page);
				for (int index = 0; index < m_Playlist[page].Count (); index ++) {
					AudibleSoundClass *sound_obj = m_Playlist[page][index];
					if (sound_obj->Get_Type () == AudibleSoundClass::TYPE_DIALOG) {
						sound_obj->Stop (false);
						sound_obj->Play ();
					}
				}
				Pop_Active_Sound_Page ();
			}

		} else {

			for (int page = 0; page < PAGE_COUNT; page ++) {
				for (int index = 0; index < m_Playlist[page].Count (); index ++) {
					AudibleSoundClass *sound_obj = m_Playlist[page][index];
					if (sound_obj->Get_Type () == AudibleSoundClass::TYPE_DIALOG) {
						sound_obj->Free_Miles_Handle ();
					}
				}
			}
		}
	}

	return;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Allow_Cinematic_Sound
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
WWAudioClass::Allow_Cinematic_Sound (bool onoff)
{
	// Is the state changing?
	if (m_IsCinematicSoundEnabled != onoff) {
		m_IsCinematicSoundEnabled = onoff;

		//
		// Update all the currently playing 'dialog' to
		// reflect this new state.
		//
		if (m_IsCinematicSoundEnabled) {

			for (int page = 0; page < PAGE_COUNT; page ++) {
				Push_Active_Sound_Page ((WWAudioClass::SOUND_PAGE)page);
				for (int index = 0; index < m_Playlist[page].Count (); index ++) {
					AudibleSoundClass *sound_obj = m_Playlist[page][index];
					if (sound_obj->Get_Type () == AudibleSoundClass::TYPE_CINEMATIC) {
						sound_obj->Stop (false);
						sound_obj->Play ();
					}
				}
				Pop_Active_Sound_Page ();
			}

		} else {

			for (int page = 0; page < PAGE_COUNT; page ++) {
				for (int index = 0; index < m_Playlist[page].Count (); index ++) {
					AudibleSoundClass *sound_obj = m_Playlist[page][index];
					if (sound_obj->Get_Type () == AudibleSoundClass::TYPE_CINEMATIC) {
						sound_obj->Free_Miles_Handle ();
					}
				}
			}
		}
	}

	return;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Set_Dialog_Volume
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
WWAudioClass::Set_Dialog_Volume (float volume)
{
	m_DialogVolume = volume;
	m_DialogVolume = std::min (1.0F, m_DialogVolume);
	m_DialogVolume = std::max (0.0F, m_DialogVolume);

	// Update all the currently playing 'Dialog' to
	// reflect this new volume
	for (int index = 0; index < m_Playlist[m_CurrPage].Count (); index ++) {
		AudibleSoundClass *sound_obj = m_Playlist[m_CurrPage][index];
		if (sound_obj->Get_Type () == AudibleSoundClass::TYPE_DIALOG) {
			sound_obj->Update_Volume ();
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Set_Cinematic_Volume
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
WWAudioClass::Set_Cinematic_Volume (float volume)
{
	m_CinematicVolume = volume;
	m_CinematicVolume = std::min (1.0F, m_CinematicVolume);
	m_CinematicVolume = std::max (0.0F, m_CinematicVolume);

	//
	// Update all the currently playing cinematic-counds to
	// reflect this new volume
	//
	for (int index = 0; index < m_Playlist[m_CurrPage].Count (); index ++) {
		AudibleSoundClass *sound_obj = m_Playlist[m_CurrPage][index];
		if (sound_obj->Get_Type () == AudibleSoundClass::TYPE_CINEMATIC) {
			sound_obj->Update_Volume ();
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Set_Sound_Effects_Volume
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
WWAudioClass::Set_Sound_Effects_Volume (float volume)
{
	m_RealSoundVolume = volume;
	m_RealSoundVolume = std::min (1.0F, m_RealSoundVolume);
	m_RealSoundVolume = std::max (0.0F, m_RealSoundVolume);

	Internal_Set_Sound_Effects_Volume (m_RealSoundVolume);
	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Set_Music_Volume
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
WWAudioClass::Set_Music_Volume (float volume)
{
	m_RealMusicVolume = volume;
	m_RealMusicVolume = std::min (1.0F, m_RealMusicVolume);
	m_RealMusicVolume = std::max (0.0F, m_RealMusicVolume);

	Internal_Set_Music_Volume (m_RealMusicVolume);
	return;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	On_Frame_Update
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
WWAudioClass::On_Frame_Update (unsigned int milliseconds)
{
	//
	// Free any sounds we completed last frame
	//
	Free_Completed_Sounds ();

	//
	// Calculate the time in ms since the last frame
	//
	unsigned int time_delta = milliseconds;
	if (time_delta == 0) {
		time_delta = WW3D::Get_Frame_Time ();
	}

	//
	//	Update the sound scene as necessary
	//
	if (m_CurrPage == PAGE_PRIMARY && m_SoundScene != NULL) {
		m_SoundScene->On_Frame_Update (milliseconds);
		m_SoundScene->Collect_Logical_Sounds ();
	}

	//int dialog_count = 0;

	//
	// Loop through all the entries in the playlist
	//
	for (int index = 0; index < m_Playlist[m_CurrPage].Count (); index ++) {

		//
		// Update this sound object
		//
		AudibleSoundClass *sound_obj = m_Playlist[m_CurrPage][index];
		sound_obj->On_Frame_Update (time_delta);

		//
		//	Is this an important piece of dialog?
		//
		/*if (	!sound_obj->Is_Sound_Culled() &&
				sound_obj->Get_Type () == AudibleSoundClass::TYPE_DIALOG &&
				sound_obj->Get_Priority () > 0.5F)
		{
			dialog_count ++;
		}*/
	}

	//
	//	Fade sound fx and music when there's important dialog playing.
	//
	/*if (dialog_count == 0) {
		Fade_Non_Dialog_In ();
	} else {
		Fade_Non_Dialog_Out ();
	}*/

	//
	//	Update any fading we have going on
	//
	//Update_Fade ();
	return;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Register_EOS_Callback
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
WWAudioClass::Register_EOS_Callback (LPFNEOSCALLBACK callback, uint32 user_param)
{
	m_EOSCallbackList.Add_Callback (callback, user_param);
	return;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	UnRegister_EOS_Callback
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
WWAudioClass::UnRegister_EOS_Callback (LPFNEOSCALLBACK callback)
{
	m_EOSCallbackList.Remove_Callback (callback);
	return;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Register_Text_Callback
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
WWAudioClass::Register_Text_Callback (LPFNTEXTCALLBACK callback, uint32 user_param)
{
	m_TextCallbackList.Add_Callback (callback, user_param);
	return;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	UnRegister_Text_Callback
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
WWAudioClass::UnRegister_Text_Callback (LPFNTEXTCALLBACK callback)
{
	m_TextCallbackList.Remove_Callback (callback);
	return;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Fire_Text_Callback
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
WWAudioClass::Fire_Text_Callback (AudibleSoundClass *sound_obj, const StringClass &text)
{
	if (text.Get_Length () > 0) {

		//
		//	Loop over all the text-callbacks that have been registered
		//
		for (int index = 0; index < m_TextCallbackList.Count (); index ++) {
			uint32 user_data				= 0;
			LPFNTEXTCALLBACK callback	= m_TextCallbackList.Get_Callback (index, &user_data);
			if (callback != NULL) {

				//
				//	Fire the notification
				//
				(*callback) (sound_obj, text, user_data);
			}
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Set_Background_Music
//
////////////////////////////////////////////////////////////////////////////////////////////
void
WWAudioClass::Set_Background_Music (const char *filename)
{
	//
	//	Stop the background music
	//
	if (m_BackgroundMusic != NULL) {
		m_BackgroundMusic->Stop ();
		REF_PTR_RELEASE (m_BackgroundMusic);
	}

	m_BackgroundMusicName = filename;
	if (filename != NULL) {

		//
		//	Create the sound
		//
		m_BackgroundMusic = Create_Sound_Effect (filename);
		if (m_BackgroundMusic != NULL) {

			//
			//	Configure the sound and start playing it
			//
			m_BackgroundMusic->Set_Priority (1.0F);
			m_BackgroundMusic->Set_Runtime_Priority (1.0F);
			m_BackgroundMusic->Set_Loop_Count (INFINITE_LOOPS);
			m_BackgroundMusic->Set_Type (AudibleSoundClass::TYPE_MUSIC);
			m_BackgroundMusic->Cull_Sound (false);
			m_BackgroundMusic->Play ();
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Fade_Background_Music
//
////////////////////////////////////////////////////////////////////////////////////////////
void
WWAudioClass::Fade_Background_Music (const char *filename, int fade_out_time, int fade_in_time)
{
	//
	//	Fade-out the background music (as necessary)
	//
	if (m_BackgroundMusic != NULL) {
		m_BackgroundMusic->Fade_Out (fade_out_time);
		REF_PTR_RELEASE (m_BackgroundMusic);
	}

	m_BackgroundMusicName = filename;
	if (filename != NULL) {

		//
		//	Create the sound
		//
		m_BackgroundMusic = Create_Sound_Effect (filename);
		if (m_BackgroundMusic != NULL) {

			//
			//	Configure the sound and start playing it
			//
			m_BackgroundMusic->Set_Priority (1.0F);
			m_BackgroundMusic->Set_Runtime_Priority (1.0F);
			m_BackgroundMusic->Set_Loop_Count (INFINITE_LOOPS);
			m_BackgroundMusic->Set_Type (AudibleSoundClass::TYPE_MUSIC);
			m_BackgroundMusic->Cull_Sound (false);
			m_BackgroundMusic->Fade_In (fade_in_time);
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Create_Logical_Sound
//
////////////////////////////////////////////////////////////////////////////////////////////
LogicalSoundClass *
WWAudioClass::Create_Logical_Sound (void)
{
	return new LogicalSoundClass;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Create_Logical_Listener
//
////////////////////////////////////////////////////////////////////////////////////////////
LogicalListenerClass *
WWAudioClass::Create_Logical_Listener (void)
{
	return new LogicalListenerClass;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Add_Logical_Type
//
////////////////////////////////////////////////////////////////////////////////////////////
void
WWAudioClass::Add_Logical_Type (int id, const char* display_name)
{
	m_LogicalTypes.Add (LOGICAL_TYPE_STRUCT (id, display_name));
	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Reset_Logical_Types
//
////////////////////////////////////////////////////////////////////////////////////////////
void
WWAudioClass::Reset_Logical_Types (void)
{
	m_LogicalTypes.Delete_All ();
	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Get_Logical_Type
//
////////////////////////////////////////////////////////////////////////////////////////////
int
WWAudioClass::Get_Logical_Type (int index, StringClass &name)
{
	int type_id = 0;

	WWASSERT (index >= 0 && index < m_LogicalTypes.Count ());
	if (index >= 0 && index < m_LogicalTypes.Count ()) {
		type_id	= m_LogicalTypes[index].id;
		name		= m_LogicalTypes[index].display_name;
	}

	return type_id;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Create_Sound_Effect
//
////////////////////////////////////////////////////////////////////////////////////////////////
AudibleSoundClass *
WWAudioClass::Create_Sound_Effect (const char *filename)
{
	WWPROFILE ("Create_Sound_Effect");

	// Assume failure
	AudibleSoundClass *sound_obj = NULL;
	if (!Is_Disabled()) {
		// Param OK?
		WWASSERT (filename != NULL);
		if (filename != NULL) {
			
			// Try to find the buffer in our cache, otherwise create a new buffer.
			SoundBufferClass *buffer = Get_Sound_Buffer (filename, false);

			if (buffer != NULL) {
				sound_obj = NEW_REF(AudibleSoundClass, ());
				sound_obj->Set_Buffer(buffer);
				REF_PTR_RELEASE(buffer);
			} else {
				WWDEBUG_SAY(( "Sound %s not found\r\n", filename ));
			}

		}
	}

	// Return a pointer to the sound effect
	return sound_obj;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Create_3D_Sound
//
////////////////////////////////////////////////////////////////////////////////////////////////
Sound3DClass *
WWAudioClass::Create_3D_Sound
(
	const char *	filename,
	int				classid_hint
)
{
	WWPROFILE ("Create_3D_Sound (filename)");
	WWMEMLOG(MEM_SOUND);

	// Assume failure
	Sound3DClass *sound_obj = NULL;
	if (!Is_Disabled()) {

		// Param OK?
		WWASSERT (filename != NULL);
		if (filename != NULL) {

			// Try to find the buffer in our cache, otherwise create a new buffer.
			SoundBufferClass *buffer = Get_Sound_Buffer (filename, true);

			//
			// What type of sound object should we create? A true 3D sound or one of
			// our pseudo-3d sounds?  (volume and panning only)
			//
			if (	classid_hint == CLASSID_PSEUDO3D ||
					!Validate_3D_Sound_Buffer(buffer))
			{
				sound_obj = new SoundPseudo3DClass;
				sound_obj->Set_Buffer (buffer);
			} else if (buffer != NULL) {
				sound_obj = new Sound3DClass;
				sound_obj->Set_Buffer (buffer);
			} else {
				static int count = 0;
				if ( ++count < 10 ) {
					WWDEBUG_SAY(( "Sound File not Found \"%s\"\r\n", filename ));
				}
			}

			REF_PTR_RELEASE (buffer);
		}
	}

	// Return a pointer to the sound effect
	return sound_obj;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Create_Sound
//
////////////////////////////////////////////////////////////////////////////////////////////////
AudibleSoundClass *
WWAudioClass::Create_Sound
(
	int				definition_id,
	RefCountClass *user_obj,
	uint32			user_data,
	int				classid_hint
)
{
	WWPROFILE ("Create_Sound");
	AudibleSoundClass *sound = NULL;

	//
	//	Find the definition
	//
	DefinitionClass *definition = DefinitionMgrClass::Find_Definition (definition_id);
	if (definition != NULL ) {

		//
		//	Make sure this is really a sound definition
		//
		WWASSERT (definition->Get_Class_ID () == CLASSID_SOUND);
		if (definition->Get_Class_ID () == CLASSID_SOUND) {
			AudibleSoundDefinitionClass *sound_def = reinterpret_cast<AudibleSoundDefinitionClass *> (definition);

			//
			//	Create an instance of the sound
			//
			sound = sound_def->Create_Sound (classid_hint);
			if (sound != NULL) {
				sound->Set_User_Data (user_obj, user_data);
			}
		}
	}

	return sound;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Create_Sound
//
////////////////////////////////////////////////////////////////////////////////////////////////
AudibleSoundClass *
WWAudioClass::Create_Sound
(
	const char *	def_name,
	RefCountClass *user_obj,
	uint32			user_data,
	int				classid_hint
)
{
	WWPROFILE ("Create_Sound");
	AudibleSoundClass *sound = NULL;

	//
	//	Find the definition
	//
	DefinitionClass *definition = DefinitionMgrClass::Find_Typed_Definition (def_name, CLASSID_SOUND, true);
	if (definition != NULL ) {

		//
		//	Make sure this is really a sound definition
		//
		WWASSERT (definition->Get_Class_ID () == CLASSID_SOUND);
		if (definition->Get_Class_ID () == CLASSID_SOUND) {
			AudibleSoundDefinitionClass *sound_def = reinterpret_cast<AudibleSoundDefinitionClass *> (definition);

			//
			//	Create an instance of the sound
			//
			sound = sound_def->Create_Sound (classid_hint);
			if (sound != NULL) {
				sound->Set_User_Data (user_obj, user_data);
			}
		}
	}

	return sound;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Create_Continuous_Sound
//
////////////////////////////////////////////////////////////////////////////////////////////////
AudibleSoundClass *
WWAudioClass::Create_Continuous_Sound
(
	int				definition_id,
	RefCountClass *user_obj,
	uint32			user_data,
	int				classid_hint
)
{
	WWPROFILE ("Create_Continuous_Sound");

	//
	//	Create an instance of the sound and play it
	//
	AudibleSoundClass *sound = Create_Sound (definition_id, user_obj, user_data, classid_hint);
	if (sound != NULL) {

		if (sound->Get_Loop_Count () != INFINITE_LOOPS) {
			WWDEBUG_SAY (("Audio Error:  Creating a continuous sound with a finite loop count!\r\n"));
		}
	}

	return sound;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Create_Instant_Sound
//
////////////////////////////////////////////////////////////////////////////////////////////////
int
WWAudioClass::Create_Instant_Sound
(
	int					definition_id,
	const Matrix3D &	tm,
	RefCountClass *	user_obj,
	uint32				user_data,
	int					classid_hint
)
{
	WWPROFILE ("Create_Instant_Sound");

	int sound_id = 0;

	//
	//	Create an instance of the sound and play it
	//
	AudibleSoundClass *sound = Create_Sound (definition_id, user_obj, user_data, classid_hint);
	if (sound != NULL) {

		if (sound->Get_Loop_Count () == INFINITE_LOOPS) {
			WWDEBUG_SAY (("Audio Error:  Creating an instant sound %s with an infinite loop count!\r\n",sound->Get_Definition()->Get_Name()));
		}

		sound_id = sound->Get_ID ();
		sound->Set_Transform (tm);
		sound->Add_To_Scene ();
		sound->Release_Ref ();
	}

	return sound_id;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Create_Continuous_Sound
//
////////////////////////////////////////////////////////////////////////////////////////////////
AudibleSoundClass *
WWAudioClass::Create_Continuous_Sound
(
	const char *	def_name,
	RefCountClass *user_obj,
	uint32			user_data,
	int				classid_hint
)
{
	WWPROFILE ("Create_Continuous_Sound");

	//
	//	Create an instance of the sound and play it
	//
	AudibleSoundClass *sound = Create_Sound (def_name, user_obj, user_data, classid_hint);
	if (sound != NULL) {

		if (sound->Get_Loop_Count () != INFINITE_LOOPS) {
			WWDEBUG_SAY (("Audio Error:  Creating a continuous sound with a finite loop count!\r\n"));
		}

	}

	return sound;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Create_Instant_Sound
//
////////////////////////////////////////////////////////////////////////////////////////////////
int
WWAudioClass::Create_Instant_Sound
(
	const char *		def_name,
	const Matrix3D &	tm,
	RefCountClass *	user_obj,
	uint32				user_data,
	int					classid_hint
)
{
	WWPROFILE ("Create_Instant_Sound");
	int sound_id = 0;

	//
	//	Create an instance of the sound and play it
	//
	AudibleSoundClass *sound = Create_Sound (def_name, user_obj, user_data, classid_hint);
	if (sound != NULL) {

		if (sound->Get_Loop_Count () == INFINITE_LOOPS) {
			WWDEBUG_SAY (("Audio Error:  Creating an instant sound %s with an infinite loop count!\r\n",sound->Get_Definition()->Get_Name()));
		}

		sound_id = sound->Get_ID ();
		sound->Set_Transform (tm);
		sound->Add_To_Scene ();
		sound->Release_Ref ();
	}

	return sound_id;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Simple_Play_2D_Sound_Effect
//
////////////////////////////////////////////////////////////////////////////////////////////////
bool
WWAudioClass::Simple_Play_2D_Sound_Effect
(
	const char *filename,
	float priority,
	float volume
)
{
	bool retval = false;
	AudibleSoundClass *sound = Create_Sound_Effect (filename);
	if (sound != NULL) {
		sound->Set_Priority (priority);
		sound->Set_Loop_Count (1);
		sound->Set_Volume(volume);
		sound->Play ();
		sound->Release_Ref ();
		sound = NULL;
		retval = true;
	}

	return retval;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Flush_Playlist
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
WWAudioClass::Flush_Playlist (SOUND_PAGE page)
{
	//
	// Loop through all the entries in this playlist
	//
	for (int index = 0; index < m_Playlist[page].Count (); index ++) {
		AudibleSoundClass *sound_obj = m_Playlist[page][index];
		if (sound_obj != NULL) {
			sound_obj->Stop ();
			sound_obj->Remove_From_Scene ();
		}
		//REF_PTR_RELEASE (sound_obj);
	}

	//
	// Now, make sure to free any completed sounds
	//
	Free_Completed_Sounds ();

	//
	// Free the list structure
	//
	m_Playlist[page].Delete_All ();
	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Flush_Playlist
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
WWAudioClass::Flush_Playlist (void)
{
	Flush_Playlist (PAGE_PRIMARY);
	Flush_Playlist (PAGE_SECONDARY);
	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Get_Playlist_Entry
//
////////////////////////////////////////////////////////////////////////////////////////////////
AudibleSoundClass *
WWAudioClass::Get_Playlist_Entry (int index) const
{
	AudibleSoundClass *sound_obj = NULL;

	// Params OK?
	WWASSERT (index >= 0 && index < m_Playlist[m_CurrPage].Count ());
	if ((index >= 0) && (index < m_Playlist[m_CurrPage].Count ())) {
		m_Playlist[m_CurrPage][index]->Add_Ref ();
		m_Playlist[m_CurrPage][index];
	}

	// Return a pointer to the sound object
	return sound_obj;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Add_To_Playlist
//
////////////////////////////////////////////////////////////////////////////////////////////////
bool
WWAudioClass::Add_To_Playlist (AudibleSoundClass *sound)
{
	bool retval = false;

	WWASSERT (sound != NULL);
	if (sound != NULL) {

		//
		// Loop through all the entries in the playlist
		//
		bool already_added = false;
		for (int index = 0; (index < m_Playlist[m_CurrPage].Count ()) && (!already_added ); index ++) {
			already_added = (sound == m_Playlist[m_CurrPage][index]);
		}

		//
		// Add this sound to our playlist
		//
		if (!already_added ) {
			sound->Add_Ref ();
			m_Playlist[m_CurrPage].Add (sound);
		}
	}

	return retval;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Remove_From_Playlist
//
////////////////////////////////////////////////////////////////////////////////////////////////
bool
WWAudioClass::Remove_From_Playlist (AudibleSoundClass *sound_obj)
{
	bool retval = false;

	WWASSERT (sound_obj != NULL);
	if (sound_obj != NULL) {

		//
		// Loop through all the entries in the playlist
		//
		for (int page = 0; page < PAGE_COUNT && !retval; page ++) {
			for (int index = 0; (index < m_Playlist[page].Count ()) && !retval; index ++) {

				//
				// Is this the entry we are looking for?
				//
				if (sound_obj == m_Playlist[page][index]) {

					//
					// Add this sound to the 'completed' list
					//
					m_CompletedSounds.Add (sound_obj);
					retval = true;
				}
			}
		}

		//
		// Notify any callbacks that this sound is ending...
		//
		if (sound_obj->Get_Loop_Count () != INFINITE_LOOPS) {
			for (int index = 0; index < m_EOSCallbackList.Count (); index ++) {
				uint32 user_data				= 0;
				LPFNEOSCALLBACK callback	= m_EOSCallbackList.Get_Callback (index, &user_data);
				if (callback != NULL) {
					(*callback) (sound_obj, user_data);
				}
			}
		}
	}

	return retval;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Is_Sound_In_Playlist
//
////////////////////////////////////////////////////////////////////////////////////////////////
bool
WWAudioClass::Is_Sound_In_Playlist (AudibleSoundClass *sound_obj)
{
	// Assume failure
	bool retval = false;

	// Loop through all the entries in the playlist
	for (int index = 0; (index < m_Playlist[m_CurrPage].Count ()) && (!retval); index ++) {
		if (sound_obj == m_Playlist[m_CurrPage][index]) {
			retval = true;
		}
	}

	// Return the true/false result code
	return retval;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Acquire_Virtual_Channel
//
////////////////////////////////////////////////////////////////////////////////////////////
bool
WWAudioClass::Acquire_Virtual_Channel (AudibleSoundClass *sound_obj, int channel_index)
{
	//
	//	Verify parameters
	//
	channel_index --;
	if (sound_obj == NULL || channel_index < 0 || channel_index >= MAX_VIRTUAL_CHANNELS) {
		return false;
	}

	//
	//	Is there already a sound playing on this channel?
	//
	bool retval = true;
	if (m_VirtualChannels[channel_index] != NULL) {
		AudibleSoundClass *curr_sound = m_VirtualChannels[channel_index];

		//
		//	If the new sound has overriding priority, then stop the sound
		// that's currently playing on this channel
		//
		if (sound_obj->Get_Priority () >= curr_sound->Get_Priority ()) {
			m_VirtualChannels[channel_index] = NULL;
			curr_sound->Stop ();
			REF_PTR_RELEASE (curr_sound);
		} else {
			retval = false;
		}
	}

	//
	//	Store this sound in the virtual channel
	//
	if (retval) {
		m_VirtualChannels[channel_index] = sound_obj;
		sound_obj->Add_Ref ();
	}

	return retval;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Release_Virtual_Channel
//
////////////////////////////////////////////////////////////////////////////////////////////
void
WWAudioClass::Release_Virtual_Channel (AudibleSoundClass *sound_obj, int channel_index)
{
	//
	//	Verify parameters
	//
	channel_index --;
	if (sound_obj == NULL || channel_index < 0 || channel_index >= MAX_VIRTUAL_CHANNELS) {
		return ;
	}

	//
	//	Check to ensure this sound has control of the virtual channel
	//
	if (m_VirtualChannels[channel_index] == sound_obj) {

		//
		//	Free the channel
		//
		m_VirtualChannels[channel_index] = NULL;
		REF_PTR_RELEASE (sound_obj);
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Find_Sound_Object
//
////////////////////////////////////////////////////////////////////////////////////////////
SoundSceneObjClass *
WWAudioClass::Find_Sound_Object (uint32 sound_obj_id)
{
	SoundSceneObjClass *sound_obj = NULL;

	//
	//	Lookup the sound object and return it to the caller
	//
	int index = 0;
	if (SoundSceneObjClass::Find_Sound_Object (sound_obj_id, &index)) {
		sound_obj = SoundSceneObjClass::m_GlobalSoundList[index];
	}

	return sound_obj;
}

////////////////////////////////////////////////////////////////////////////////////////////
//
//	Load_From_Registry
//
////////////////////////////////////////////////////////////////////////////////////////////
bool
WWAudioClass::Load_From_Registry
(
	const char *	subkey_name,
	StringClass &	device_name,
	bool &			is_stereo,
	int &				bits,
	int &				hertz,
	bool &			sound_enabled,
	bool &			music_enabled,
	bool &			dialog_enabled,
	bool &			cinematic_enabled,
	float &			sound_volume,
	float &			music_volume,
	float &			dialog_volume,
	float &			cinematic_volume,
	int &				speaker_type
)
{
	bool retval = false;
	int defaultmusicvolume, defaultsoundvolume, defaultdialogvolume, defaultcinematicvolume;
	Load_Default_Volume (defaultmusicvolume, defaultsoundvolume, defaultdialogvolume, defaultcinematicvolume);

	//
	//	Attempt to open the config file
	//
	INIClass ini(W3D_CONF_FILE);

	if(ini.Is_Present(W3D_SECTION_SOUND)) {
		
		//
		//	Read the device name into a string object
		//
		char temp_buffer[256] = { 0 };
		ini.Get_String (W3D_SECTION_SOUND, VALUE_INI_DEVICE_NAME, "", temp_buffer, sizeof (temp_buffer));
		device_name = temp_buffer;

		//
		//	Read the 2D settings
		//
		is_stereo	= ini.Get_Bool (W3D_SECTION_SOUND, VALUE_INI_IS_STEREO, true);
		bits			= ini.Get_Int (W3D_SECTION_SOUND, VALUE_INI_BITS, 16);
		hertz			= ini.Get_Int (W3D_SECTION_SOUND, VALUE_INI_HERTZ, 44100);

		//
		//	Read the sound/music enabled settings
		//
		music_enabled		= ini.Get_Bool (W3D_SECTION_SOUND, VALUE_INI_MUSIC_ENABLED, true);
		sound_enabled		= ini.Get_Bool (W3D_SECTION_SOUND, VALUE_INI_SOUND_ENABLED, true);
		dialog_enabled		= ini.Get_Bool (W3D_SECTION_SOUND, VALUE_INI_DIALOG_ENABLED, true);
		cinematic_enabled = ini.Get_Bool (W3D_SECTION_SOUND, VALUE_INI_CINEMATIC_ENABLED, true);

		
		//
		//	Read the volume information
		//
		music_volume		= ini.Get_Float (W3D_SECTION_SOUND, VALUE_INI_MUSIC_VOL, defaultmusicvolume / 100.0F);
		sound_volume		= ini.Get_Float (W3D_SECTION_SOUND, VALUE_INI_SOUND_VOL, defaultsoundvolume / 100.0F);
		dialog_volume		= ini.Get_Float (W3D_SECTION_SOUND, VALUE_INI_DIALOG_VOL, defaultdialogvolume / 100.0F);
		cinematic_volume	= ini.Get_Float (W3D_SECTION_SOUND, VALUE_INI_CINEMATIC_VOL, defaultcinematicvolume / 100.0F);

		//
		//	Misc
		//
		speaker_type		= ini.Get_Int (W3D_SECTION_SOUND, VALUE_INI_SPEAKER_TYPE, 0);

		retval		= true;
	}

	music_volume		= std::clamp (music_volume, 0.0F, 1.0F);
	sound_volume		= std::clamp (sound_volume, 0.0F, 1.0F);
	dialog_volume		= std::clamp (dialog_volume, 0.0F, 1.0F);
	cinematic_volume	= std::clamp (cinematic_volume, 0.0F, 1.0F);

	return retval;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Save_To_Registry
//
////////////////////////////////////////////////////////////////////////////////////////////
bool
WWAudioClass::Save_To_Registry
(
	const char *			subkey_name,
	const StringClass &	device_name,
	bool						is_stereo,
	int						bits,
	int						hertz,
	bool						sound_enabled,
	bool						music_enabled,
	bool						dialog_enabled,
	bool						cinematic_enabled,
	float						sound_volume,
	float						music_volume,
	float						dialog_volume,
	float						cinematic_volume,
	int						speaker_type
)
{
	bool retval = false;

	//
	//	Attempt to open the config file
	//
	INIClass ini(W3D_CONF_FILE);

	ini.Put_String (W3D_SECTION_SOUND, VALUE_INI_DEVICE_NAME, device_name);
	ini.Put_Bool (W3D_SECTION_SOUND, VALUE_INI_IS_STEREO, is_stereo);
	ini.Put_Int (W3D_SECTION_SOUND, VALUE_INI_BITS, bits);
	ini.Put_Int (W3D_SECTION_SOUND, VALUE_INI_HERTZ, hertz);
	ini.Put_Bool (W3D_SECTION_SOUND, VALUE_INI_MUSIC_ENABLED,		music_enabled);
	ini.Put_Bool (W3D_SECTION_SOUND, VALUE_INI_SOUND_ENABLED,		sound_enabled);
	ini.Put_Bool (W3D_SECTION_SOUND, VALUE_INI_DIALOG_ENABLED,		dialog_enabled);
	ini.Put_Bool (W3D_SECTION_SOUND, VALUE_INI_CINEMATIC_ENABLED,	cinematic_enabled);
	ini.Put_Float (W3D_SECTION_SOUND, VALUE_INI_MUSIC_VOL,			music_volume);
	ini.Put_Float (W3D_SECTION_SOUND, VALUE_INI_SOUND_VOL,			sound_volume);
	ini.Put_Float (W3D_SECTION_SOUND, VALUE_INI_DIALOG_VOL,			dialog_volume);
	ini.Put_Float (W3D_SECTION_SOUND, VALUE_INI_CINEMATIC_VOL,		cinematic_volume);
	ini.Put_Int (W3D_SECTION_SOUND, VALUE_INI_SPEAKER_TYPE,		speaker_type);
	retval = ini.Save(W3D_CONF_FILE) != 0;

	return retval;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Load_Default_Volume
//
////////////////////////////////////////////////////////////////////////////////////////////
void WWAudioClass::Load_Default_Volume (int &defaultmusicvolume, int &defaultsoundvolume, int &defaultdialogvolume, int &defaultcinematicvolume)
{
	const int minsetting =   0;
	const int maxsetting = 100;

	// IML: If the audio INI has not yet been loaded then do it now.
	if (AudioIni == NULL) {
		AudioIni = new INIClass;
		if (!AudioIni->Load (WWAUDIO_INI_FILENAME)) {
			AudioIni->Load (WWAUDIO_INI_RELATIVE_PATHNAME);
		}
	}

	defaultmusicvolume	  = MIN (maxsetting, MAX (minsetting, AudioIni->Get_Int (INI_DEFAULT_VOLUME_SECTION, INI_MUSIC_VOLUME_ENTRY, 31)));
	defaultsoundvolume	  = MIN (maxsetting, MAX (minsetting, AudioIni->Get_Int (INI_DEFAULT_VOLUME_SECTION, INI_SOUND_VOLUME_ENTRY, 43)));
	defaultdialogvolume	  = MIN (maxsetting, MAX (minsetting, AudioIni->Get_Int (INI_DEFAULT_VOLUME_SECTION, INI_DIALOG_VOLUME_ENTRY, 50)));
	defaultcinematicvolume = MIN (maxsetting, MAX (minsetting, AudioIni->Get_Int (INI_DEFAULT_VOLUME_SECTION, INI_CINEMATIC_VOLUME_ENTRY, 100)));
}

////////////////////////////////////////////////////////////////////////////////////////////
//
//	Is_Disabled
//
////////////////////////////////////////////////////////////////////////////////////////////////
bool
WWAudioClass::Is_Disabled (void) const
{
	static bool _firsttime = true;
	static bool _disabled = false;

	if (_firsttime) {
		_firsttime = false;

		//
		//	Read the disabled key from the registry
		//
		RegistryClass registry ("SOFTWARE\\Westwood\\WWAudio");
		if (registry.Is_Valid ()) {
			if (registry.Get_Int ("Disabled", 0) == 1) {
				_disabled = true;
				WWDEBUG_SAY (("WWAudio: Audio system disabled in registry.\r\n"));
			}
		}
	}

	return (_disabled | m_ForceDisable);
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Set_Active_Sound_Page
//
////////////////////////////////////////////////////////////////////////////////////////////
void
WWAudioClass::Set_Active_Sound_Page (SOUND_PAGE page)
{
	if (page == m_CurrPage) {
		return ;
	}

	//
	//	Pause any sounds that are playing in the old page
	//
	int index;
	for (index = 0; index < m_Playlist[m_CurrPage].Count ();index ++) {
		m_Playlist[m_CurrPage][index]->Pause ();
	}

	//
	//	Resume any sounds that are playing in the new page
	//
	for (index = 0; index < m_Playlist[page].Count ();index ++) {
		m_Playlist[page][index]->Resume ();
	}

	m_CurrPage = page;
	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Push_Active_Sound_Page
//
////////////////////////////////////////////////////////////////////////////////////////////
void
WWAudioClass::Push_Active_Sound_Page (SOUND_PAGE page)
{
	m_PageStack.Add (m_CurrPage);
	Set_Active_Sound_Page (page);
	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Pop_Active_Sound_Page
//
////////////////////////////////////////////////////////////////////////////////////////////
void
WWAudioClass::Pop_Active_Sound_Page (void)
{
	if (m_PageStack.Count () > 0) {
		SOUND_PAGE new_page = m_PageStack[m_PageStack.Count () - 1];
		m_PageStack.Delete (m_PageStack.Count () - 1);
		Set_Active_Sound_Page (new_page);
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Free_Completed_Sounds
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
WWAudioClass::Free_Completed_Sounds (void)
{
	if (m_CompletedSounds.Count () > 0) {

		//
		// Loop through all the entries in the completed sounds list
		//
		for (int index = 0; index < m_CompletedSounds.Count (); index ++) {
			AudibleSoundClass *sound_obj = m_CompletedSounds[index];
			WWASSERT(sound_obj != NULL); //TSS 05/24/99

			//
			//	Be careful not to remove the sound from the playlist unless
			// its really done playing
			//
			if (sound_obj->Get_State () == AudibleSoundClass::STATE_STOPPED) {

				//
				// Remove this sound from the playlist
				//
				bool found = false;
				for (int page = 0; page < PAGE_COUNT && !found; page ++) {
					for (int play_index = 0; (play_index < m_Playlist[page].Count ()) && !found; play_index ++) {
						if (m_Playlist[page][play_index] == sound_obj) {

							//
							// Free our hold on this sound object
							//
							m_Playlist[page].Delete (play_index);
							REF_PTR_RELEASE (sound_obj);
							found = true;
						}
					}
				}
			}
		}

		//
		// Free the list structure
		//
		m_CompletedSounds.Delete_All ();

		//
		// Try to give a play-handle back to a sound that was priority-bumped.
		//
		Reprioritize_Playlist ();
	}

	return;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Reprioritize_Playlist
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
WWAudioClass::Reprioritize_Playlist (void)
{
	AudibleSoundClass *sound_to_get_handle = NULL;
	float hightest_priority = 0;

	//
	// Loop through all the entries in the playlist
	//
	for (int index = 0; index < m_Playlist[m_CurrPage].Count (); index ++) {

		//
		// Is this the highest priority without a miles handle?
		//
		AudibleSoundClass *sound_obj = m_Playlist[m_CurrPage][index];
		if ((sound_obj->Get_Miles_Handle () == NULL) &&
			 (!sound_obj->Is_Sound_Culled()) &&
			 (sound_obj->Get_Priority () > hightest_priority))
		{
			//
			// This is now the highest priority sound effect without
			// a play-handle.
			//
			sound_to_get_handle = sound_obj;
			hightest_priority = sound_obj->Get_Priority ();
		}
	}

	//
	// Get a new handle for this sound if necessary
	//
	if (sound_to_get_handle != NULL) {
		sound_to_get_handle->Allocate_Miles_Handle ();
	}

	return;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Update_Fade
//
////////////////////////////////////////////////////////////////////////////////////////////
void
WWAudioClass::Update_Fade (void)
{
	if (m_FadeType == FADE_NONE || m_FadeType == FADED_OUT) {
		return ;
	}

	m_FadeTimer -= (WW3D::Get_Frame_Time () / 1000.0F);
	m_FadeTimer = std::max (m_FadeTimer, 0.0F);

	//
	//	Determine what percent we should ramp up or down to...
	//
	float percent	= (m_FadeTimer / m_NonDialogFadeTime);
	percent			= std::clamp (percent, 0.0F, 1.0F);

	//
	//	Invert the percent if we're fading out
	//
	if (m_FadeType == FADE_IN) {
		percent = 1.0F - percent;
	}

	//
	//	Determine what the current percent is
	//
	const float FADE_MAX = 0.6F;
	percent = (1.0F - FADE_MAX) + (percent * FADE_MAX);

	//
	//	Re-adjust the music and sound effect volumes
	//
	Internal_Set_Music_Volume (m_RealMusicVolume * percent);
	Internal_Set_Sound_Effects_Volume (m_RealSoundVolume * percent);

	//
	//	If we've done the fade, then return to the "no fade" stage
	//
	if (m_FadeTimer == 0) {
		if (m_FadeType == FADE_OUT) {
			m_FadeType = FADED_OUT;
		} else {
			m_FadeType = FADE_NONE;
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Temp_Disable_Audio
//
////////////////////////////////////////////////////////////////////////////////////////////
void
WWAudioClass::Temp_Disable_Audio (bool onoff)
{
	if (onoff) {
		m_CachedIsMusicEnabled				= m_IsMusicEnabled;
		m_CachedIsDialogEnabled				= m_IsDialogEnabled;
		m_CachedIsCinematicSoundEnabled	= m_IsCinematicSoundEnabled;
		m_CachedAreSoundEffectsEnabled	= m_AreSoundEffectsEnabled;
		Allow_Sound_Effects (false);
		Allow_Music (false);
		Allow_Dialog (false);
		Allow_Cinematic_Sound (false);
	} else {
		Allow_Sound_Effects (m_CachedAreSoundEffectsEnabled);
		Allow_Music (m_CachedIsMusicEnabled);
		Allow_Dialog (m_CachedIsDialogEnabled);
		Allow_Cinematic_Sound (m_CachedIsCinematicSoundEnabled);
	}

	return ;
}
