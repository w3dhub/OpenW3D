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
 *                 Project Name : ww3d2																		  *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/animatedsoundmgr.cpp                   $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 12/13/01 6:05p                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "animatedsoundmgr.h"
#include "ini.h"
#include "inisup.h"
#include "ffactory.h"
#include "wwfile.h"
#include <stdio.h>
#include "definition.h"
#include "definitionmgr.h"
#include "definitionclassids.h"
#include "wwaudio.h"
#include "audiblesound.h"
#include "htree.h"
#include "hanim.h"


//////////////////////////////////////////////////////////////////////
//	Static member initialization
//////////////////////////////////////////////////////////////////////
HashTemplateClass<StringClass, AnimatedSoundMgrClass::ANIM_SOUND_LIST *>	AnimatedSoundMgrClass::AnimationNameHash;
DynamicVectorClass<AnimatedSoundMgrClass::ANIM_SOUND_LIST *>					AnimatedSoundMgrClass::AnimSoundLists;


//////////////////////////////////////////////////////////////////////
//	Local inlines
//////////////////////////////////////////////////////////////////////
static WWINLINE INIClass *
Get_INI (const char *filename)
{
	INIClass *ini = NULL;

	//
	//	Get the file from our filefactory
	//
	FileClass *file = _TheFileFactory->Get_File (filename);
	if (file) {
		
		//
		//	Create the INI object
		//
		if (file->Is_Available ()) {
			ini = new INIClass (*file);
		}

		//
		//	Close the file
		//
		_TheFileFactory->Return_File (file);
	}

	return ini;
}

static int
Build_List_From_String
(
	const char *	buffer,
	const char *	delimiter,
	StringClass **	string_list
)
{
	int count = 0;

	WWASSERT (buffer != NULL);
	WWASSERT (delimiter != NULL);
	WWASSERT (string_list != NULL);
	if ((buffer != NULL) &&
		 (delimiter != NULL) &&
		 (string_list != NULL))
	{
		int delim_len = ::strlen (delimiter);

		//
		// Determine how many entries there will be in the list
		//
		const char *entry;
		for (entry = buffer;
			  (entry != NULL) && (entry[1] != 0);
			  entry = ::strstr (entry, delimiter))
		{
			
			//
			// Move past the current delimiter (if necessary)
			//
			if ((::strnicmp (entry, delimiter, delim_len) == 0) && (count > 0)) {
				entry += delim_len;
			}

			// Increment the count of entries
			count ++;
		}
	
		if (count > 0) {

			//
			// Allocate enough StringClass objects to hold all the strings in the list
			//
			(*string_list) = new StringClass[count];
		
			//
			// Parse the string and pull out its entries.
			//
			count = 0;
			for (entry = buffer;
				  (entry != NULL) && (entry[1] != 0);
				  entry = ::strstr (entry, delimiter))
			{
				
				//
				// Move past the current delimiter (if necessary)
				//
				if ((::strnicmp (entry, delimiter, delim_len) == 0) && (count > 0)) {
					entry += delim_len;
				}

				//
				// Copy this entry into its own string
				//
				StringClass entry_string = entry;
				char *delim_start = (char *)::strstr (entry_string, delimiter);				
				if (delim_start != NULL) {
					delim_start[0] = 0;
				}

				//
				// Add this entry to our list
				//
				if ((entry_string.Get_Length () > 0) || (count == 0)) {
					(*string_list)[count++] = entry_string;
				}
			}

		} else if (delim_len > 0) {
			count = 1;
			(*string_list) = new StringClass[count];
			(*string_list)[0] = buffer;
		}
				
	}

	//
	// Return the number of entries in our list
	//
	return count;
}


//////////////////////////////////////////////////////////////////////
//
//	Initialize
//
//////////////////////////////////////////////////////////////////////
void
AnimatedSoundMgrClass::Initialize (const char *ini_filename)
{
	//
	//	Don't re-initialize...
	//
	if (AnimSoundLists.Count () > 0) {
		return ;
	}

	const char *DEFAULT_INI_FILENAME	= "w3danimsound.ini";
	
	//
	//	Determine which filename to use
	//
	const char *filename_to_use = ini_filename;
	if (filename_to_use == NULL) {
		filename_to_use = DEFAULT_INI_FILENAME;
	}

	//
	//	Get the INI file which contains the data for this viewer
	//
	INIClass *ini_file = ::Get_INI (filename_to_use);
	if (ini_file != NULL) {

		//
		//	Loop over all the sections in the INI
		//
		List<INISection *> &section_list = ini_file->Get_Section_List ();
		for (	INISection *section = section_list.First ();
				section != NULL && section->Is_Valid ();
				section = section->Next_Valid ())
		{
			//
			//	Get the animation name from the section name
			//
			StringClass animation_name = section->Section;
			::strupr (animation_name.Peek_Buffer ());

			//
			//	Allocate a sound list
			//
			ANIM_SOUND_LIST *sound_list = new ANIM_SOUND_LIST;

			//
			//	Loop over all the entries in this section
			//
			int entry_count = ini_file->Entry_Count (section->Section);
			for (int entry_index = 0; entry_index < entry_count; entry_index ++) {
				StringClass value;

				//
				//	Get the data associated with this entry
				//
				const char *entry_name = ini_file->Get_Entry (section->Section, entry_index);
				ini_file->Get_String (value, section->Section, entry_name);

				//
				//	Extract the parameters from the section
				//
				int len = value.Get_Length ();					
				StringClass definition_name (len + 1, true);
				int frame_start = 0;

				//
				//	Separate the parameters into an easy-to-handle data structure
				//
				StringClass *param_list = NULL;
				int param_count = ::Build_List_From_String (value, ",", &param_list);
				if (param_count == 2) {
					frame_start			= ::atoi (param_list[0]);
					definition_name	= param_list[1];
					definition_name.Trim ();
					delete [] param_list;

					//
					//	Find this sound definition
					//
					DefinitionClass *definition = DefinitionMgrClass::Find_Typed_Definition (definition_name, CLASSID_SOUND);
					if (definition != NULL) {

						//
						//	Tie the relevant information together and store it
						// in the list of sounds for this animation
						//
						ANIM_SOUND_INFO sound_info;
						sound_info.Frame					= frame_start;
						sound_info.SoundDefinitionID	= definition->Get_ID ();
						sound_list->Add (sound_info);
					}
				}
			}

			if (sound_list->Count () != 0) {
				
				//
				//	Add this sound list to our hash-table and vector-array
				//
				AnimationNameHash.Insert (animation_name, sound_list);
				AnimSoundLists.Add (sound_list);

			} else {
				WWDEBUG_SAY (("AnimatedSoundMgrClass::Initialize -- No sounds added for %d!\n", animation_name.Peek_Buffer ()));
				delete sound_list;
			}
		}

		delete ini_file;
	}

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Shutdown
//
//////////////////////////////////////////////////////////////////////
void
AnimatedSoundMgrClass::Shutdown (void)
{
	//
	//	Reset the animation name hash
	//
	AnimationNameHash.Remove_All ();

	//
	//	Free each of the sound objects
	//
	for (int index = 0; index < AnimSoundLists.Count (); index ++) {
		delete AnimSoundLists[index];
	}

	AnimSoundLists.Delete_All ();
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Does_Animation_Have_Embedded_Sounds
//
//////////////////////////////////////////////////////////////////////
bool
AnimatedSoundMgrClass::Does_Animation_Have_Embedded_Sounds (HAnimClass *anim)
{
	return (Find_Sound_List (anim) != NULL);
}


//////////////////////////////////////////////////////////////////////
//
//	Find_Sound_List
//
//////////////////////////////////////////////////////////////////////
AnimatedSoundMgrClass::ANIM_SOUND_LIST *
AnimatedSoundMgrClass::Find_Sound_List (HAnimClass *anim)
{
	//
	//	Build the full name of the animation
	//
	StringClass full_name (0, true);
	full_name = anim->Get_Name ();

	//
	//	Make the name uppercase
	//
	::strupr (full_name.Peek_Buffer ());

	//
	//	Lookup the sound list for this animation
	//
	ANIM_SOUND_LIST *retval = AnimationNameHash.Get (full_name);
	return retval;
}


//////////////////////////////////////////////////////////////////////
//
//	Trigger_Sound
//
//////////////////////////////////////////////////////////////////////
float
AnimatedSoundMgrClass::Trigger_Sound
(
	HAnimClass *		anim,
	float					old_frame,
	float					new_frame,
	const Matrix3D &	tm
)
{
	if (anim == NULL) {
		return old_frame;
	}

	float retval = old_frame;

	//
	//	Lookup the sound list for this animation
	//
	ANIM_SOUND_LIST *sound_list = Find_Sound_List (anim);
	if (sound_list != NULL) {
		
		for (int index = 0; index < sound_list->Count (); index ++) {			
			int frame = (*sound_list)[index].Frame;

			//
			//	Is the animation passing the frame we need?
			//
			if (old_frame < frame && new_frame >= frame) {

				//
				//	Don't trigger the sound if its skipped to far past...
				//
				if (WWMath::Fabs (new_frame - old_frame) < 3.0F) {
					
					//
					//	Play the sound
					//
					int def_id = (*sound_list)[index].SoundDefinitionID;
					WWAudioClass::Get_Instance ()->Create_Instant_Sound (def_id, tm);
					WWDEBUG_SAY (("Triggering Sound %d\n", GetTickCount ()));
					retval = frame;
				}
			}
		}

		//retval = true;
	}

	return retval;
}

