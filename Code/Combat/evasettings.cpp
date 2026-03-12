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
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando                                                     *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Combat/evasettings.cpp         $*
 *                                                                                             *
 *                      $Author:: Patrick                                                     $*
 *                                                                                             *
 *                     $Modtime:: 1/07/02 10:46a                                              $*
 *                                                                                             *
 *                    $Revision:: 10                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "evasettings.h"
#include "combatchunkid.h"
#include "wwhack.h"
#include "persistfactory.h"
#include "definitionfactory.h"
#include "simpledefinitionfactory.h"
#include "debug.h"
#include "objectives.h"
#include "combat.h"
#include "messagewindow.h"


DECLARE_FORCE_LINK (EvaSettings)

///////////////////////////////////////////////////////////////////////////////////////////
//	Static member initialization
///////////////////////////////////////////////////////////////////////////////////////////
EvaSettingsDefClass *		EvaSettingsDefClass::EvaSettings = NULL;


///////////////////////////////////////////////////////////////////////////////////////////
//	Factories
///////////////////////////////////////////////////////////////////////////////////////////
SimplePersistFactoryClass<EvaSettingsDefClass, CHUNKID_GLOBAL_SETTINGS_DEF_EVA>	_EvaSettingsDefPersistFactory;
DECLARE_DEFINITION_FACTORY(EvaSettingsDefClass, CLASSID_GLOBAL_SETTINGS_DEF_EVA, "Eva Settings") _EvaSettingsDefDefFactory;

///////////////////////////////////////////////////////////////////////////////////////////
//	Save/load constants
///////////////////////////////////////////////////////////////////////////////////////////
enum
{
	CHUNKID_PARENT							=	803001812,
	CHUNKID_VARIABLES,

	VARID_OBJECTIVESSCREENRECT			=	1,
	VARID_OBJECTIVESTEXTRECT,
	VARID_OBJECTIVESENDCAPUVRECT,
	VARID_OBJECTIVESFADEOUTUVRECT,
	VARID_OBJECTIVESBACKGROUNDUVRECT,
	VARID_OBJECTIVESTEXTURESIZE,

	VARID_MESSAGESSCREENRECT,
	VARID_MESSAGESTEXTRECT,
	VARID_MESSAGESENDCAPUVRECT,
	VARID_MESSAGESFADEOUTUVRECT,
	VARID_MESSAGESBACKGROUNDUVRECT,
	VARID_MESSAGESTEXTURESIZE,
	VARID_MESSAGESICONPOS
};


///////////////////////////////////////////////////////////////////////////////////////////
//	Default values
///////////////////////////////////////////////////////////////////////////////////////////
const RectClass		DEF_OBJECTIVESSCREENRECT (0.063F, 0.25F, 0.938F, 0.75);
const RectClass		DEF_OBJECTIVESTEXTRECT (0.1F, 0.260F, 0.906F, 0.555F);
const RectClass		DEF_OBJECTIVESENDCAPUVRECT (0, 67, 8, 127);
const RectClass		DEF_OBJECTIVESFADEOUTUVRECT (41, 34, 127, 65);
const RectClass		DEF_OBJECTIVESBACKGROUNDUVRECT (2, 1, 126, 32);
const Vector2			DEF_OBJECTIVESTEXTURESIZE (128, 128);

const RectClass		DEF_MESSAGESSCREENRECT (0.116F, 0.021F, 0.938F, 0.208F);
const RectClass		DEF_MESSAGESTEXTRECT (0.147F, 0.031F, 0.906F, 0.198F);
const RectClass		DEF_MESSAGESENDCAPUVRECT (0, 67, 8, 127);
const RectClass		DEF_MESSAGESFADEOUTUVRECT (41, 34, 127, 65);
const RectClass		DEF_MESSAGESBACKGROUNDUVRECT (2, 1, 126, 32);
const Vector2			DEF_MESSAGESTEXTURESIZE (128, 128);
const Vector2			DEF_MESSAGESICONPOS (0.016F, 0.021F);


EvaSettingsDefClass	_DefaultSettings;

///////////////////////////////////////////////////////////////////////////////////////////
//
//	EvaSettingsDefClass
//
///////////////////////////////////////////////////////////////////////////////////////////
EvaSettingsDefClass::EvaSettingsDefClass (void)	:
	ObjectivesScreenRect (DEF_OBJECTIVESSCREENRECT),
	ObjectivesTextRect (DEF_OBJECTIVESTEXTRECT),
	ObjectivesEndcapUVRect (DEF_OBJECTIVESENDCAPUVRECT),
	ObjectivesFadeoutUVRect (DEF_OBJECTIVESFADEOUTUVRECT),
	ObjectivesBackgroundUVRect (DEF_OBJECTIVESBACKGROUNDUVRECT),
	ObjectivesTextureSize (DEF_OBJECTIVESTEXTURESIZE),
	MessagesScreenRect (DEF_MESSAGESSCREENRECT),
	MessagesTextRect (DEF_MESSAGESTEXTRECT),
	MessagesEndcapUVRect (DEF_MESSAGESENDCAPUVRECT),
	MessagesFadeoutUVRect (DEF_MESSAGESFADEOUTUVRECT),
	MessagesBackgroundUVRect (DEF_MESSAGESBACKGROUNDUVRECT),
	MessagesTextureSize (DEF_MESSAGESTEXTURESIZE),
	MessagesIconPos (DEF_MESSAGESICONPOS)
{
	PARAM_SEPARATOR (EvaSettingsDefClass, "Objectives Viewer");
	NAMED_EDITABLE_PARAM (EvaSettingsDefClass, ParameterClass::TYPE_RECT, ObjectivesScreenRect, "Screen Rect (normalized pixels)");
	NAMED_EDITABLE_PARAM (EvaSettingsDefClass, ParameterClass::TYPE_RECT, ObjectivesTextRect, "Text Rect (normalized pixels)");
	NAMED_EDITABLE_PARAM (EvaSettingsDefClass, ParameterClass::TYPE_VECTOR2, ObjectivesTextureSize, "Texture Size (pixels)");
	NAMED_EDITABLE_PARAM (EvaSettingsDefClass, ParameterClass::TYPE_RECT, ObjectivesEndcapUVRect, "Endcap UV Rect (pixels)");
	NAMED_EDITABLE_PARAM (EvaSettingsDefClass, ParameterClass::TYPE_RECT, ObjectivesFadeoutUVRect, "Fadeout UV Rect (pixels)");
	NAMED_EDITABLE_PARAM (EvaSettingsDefClass, ParameterClass::TYPE_RECT, ObjectivesBackgroundUVRect, "Background UV Rect (pixels)");

	PARAM_SEPARATOR (EvaSettingsDefClass, "Messages Window");
	NAMED_EDITABLE_PARAM (EvaSettingsDefClass, ParameterClass::TYPE_RECT, MessagesScreenRect, "Screen Rect (normalized pixels)");
	NAMED_EDITABLE_PARAM (EvaSettingsDefClass, ParameterClass::TYPE_RECT, MessagesTextRect, "Text Rect (normalized pixels)");
	NAMED_EDITABLE_PARAM (EvaSettingsDefClass, ParameterClass::TYPE_VECTOR2, MessagesIconPos, "Icon Position (normalized pixels)");
	NAMED_EDITABLE_PARAM (EvaSettingsDefClass, ParameterClass::TYPE_VECTOR2, MessagesTextureSize, "Texture Size (pixels)");
	NAMED_EDITABLE_PARAM (EvaSettingsDefClass, ParameterClass::TYPE_RECT, MessagesEndcapUVRect, "Endcap UV Rect (pixels)");
	NAMED_EDITABLE_PARAM (EvaSettingsDefClass, ParameterClass::TYPE_RECT, MessagesFadeoutUVRect, "Fadeout UV Rect (pixels)");
	NAMED_EDITABLE_PARAM (EvaSettingsDefClass, ParameterClass::TYPE_RECT, MessagesBackgroundUVRect, "Background UV Rect (pixels)");

	EvaSettings = this;
	return ;
}


///////////////////////////////////////////////////////////////////////////////////////////
//
//	~EvaSettingsDefClass
//
///////////////////////////////////////////////////////////////////////////////////////////
EvaSettingsDefClass::~EvaSettingsDefClass (void)
{
	EvaSettings = &_DefaultSettings;
	return ;
}


///////////////////////////////////////////////////////////////////////////////////////////
//
//	Get_Class_ID
//
///////////////////////////////////////////////////////////////////////////////////////////
uint32
EvaSettingsDefClass::Get_Class_ID (void) const
{
	return CLASSID_GLOBAL_SETTINGS_DEF_EVA;
}


///////////////////////////////////////////////////////////////////////////////////////////
//
//	Get_Factory
//
///////////////////////////////////////////////////////////////////////////////////////////
const PersistFactoryClass &
EvaSettingsDefClass::Get_Factory (void) const
{
	return _EvaSettingsDefPersistFactory;
}


///////////////////////////////////////////////////////////////////////////////////////////
//
//	Create
//
///////////////////////////////////////////////////////////////////////////////////////////
PersistClass *
EvaSettingsDefClass::Create (void) const
{
	WWASSERT (0);
	return NULL;
}


///////////////////////////////////////////////////////////////////////////////////////////
//
//	Save
//
///////////////////////////////////////////////////////////////////////////////////////////
bool
EvaSettingsDefClass::Save (ChunkSaveClass &csave)
{
	csave.Begin_Chunk (CHUNKID_PARENT);
		DefinitionClass::Save (csave);
	csave.End_Chunk ();

	csave.Begin_Chunk (CHUNKID_VARIABLES);

		WRITE_MICRO_CHUNK (csave, 	VARID_OBJECTIVESSCREENRECT,			ObjectivesScreenRect);
		WRITE_MICRO_CHUNK (csave, 	VARID_OBJECTIVESTEXTRECT,				ObjectivesTextRect);
		WRITE_MICRO_CHUNK (csave, 	VARID_OBJECTIVESENDCAPUVRECT,			ObjectivesEndcapUVRect);
		WRITE_MICRO_CHUNK (csave, 	VARID_OBJECTIVESFADEOUTUVRECT,		ObjectivesFadeoutUVRect);
		WRITE_MICRO_CHUNK (csave, 	VARID_OBJECTIVESBACKGROUNDUVRECT,	ObjectivesBackgroundUVRect);
		WRITE_MICRO_CHUNK (csave, 	VARID_OBJECTIVESTEXTURESIZE,			ObjectivesTextureSize);

		WRITE_MICRO_CHUNK (csave, 	VARID_MESSAGESSCREENRECT,				MessagesScreenRect);
		WRITE_MICRO_CHUNK (csave, 	VARID_MESSAGESTEXTRECT,					MessagesTextRect);
		WRITE_MICRO_CHUNK (csave, 	VARID_MESSAGESENDCAPUVRECT,			MessagesEndcapUVRect);
		WRITE_MICRO_CHUNK (csave, 	VARID_MESSAGESFADEOUTUVRECT,			MessagesFadeoutUVRect);
		WRITE_MICRO_CHUNK (csave, 	VARID_MESSAGESBACKGROUNDUVRECT,		MessagesBackgroundUVRect);
		WRITE_MICRO_CHUNK (csave, 	VARID_MESSAGESTEXTURESIZE,				MessagesTextureSize);
		WRITE_MICRO_CHUNK (csave, 	VARID_MESSAGESICONPOS,					MessagesIconPos);


	csave.End_Chunk();

	return true;
}


///////////////////////////////////////////////////////////////////////////////////////////
//
//	Load
//
///////////////////////////////////////////////////////////////////////////////////////////
bool
EvaSettingsDefClass::Load (ChunkLoadClass &cload)
{
	while (cload.Open_Chunk ()) {
		switch(cload.Cur_Chunk_ID ()) {

			case CHUNKID_PARENT:
				DefinitionClass::Load (cload);
				break;

			case CHUNKID_VARIABLES:
				while (cload.Open_Micro_Chunk ()) {
					switch(cload.Cur_Micro_Chunk_ID ()) {

						READ_MICRO_CHUNK (cload, 	VARID_OBJECTIVESSCREENRECT,			ObjectivesScreenRect);
						READ_MICRO_CHUNK (cload, 	VARID_OBJECTIVESTEXTRECT,				ObjectivesTextRect);
						READ_MICRO_CHUNK (cload, 	VARID_OBJECTIVESENDCAPUVRECT,			ObjectivesEndcapUVRect);
						READ_MICRO_CHUNK (cload, 	VARID_OBJECTIVESFADEOUTUVRECT,		ObjectivesFadeoutUVRect);
						READ_MICRO_CHUNK (cload, 	VARID_OBJECTIVESBACKGROUNDUVRECT,	ObjectivesBackgroundUVRect);
						READ_MICRO_CHUNK (cload, 	VARID_OBJECTIVESTEXTURESIZE,			ObjectivesTextureSize);

						READ_MICRO_CHUNK (cload, 	VARID_MESSAGESSCREENRECT,				MessagesScreenRect);
						READ_MICRO_CHUNK (cload, 	VARID_MESSAGESTEXTRECT,					MessagesTextRect);
						READ_MICRO_CHUNK (cload, 	VARID_MESSAGESENDCAPUVRECT,			MessagesEndcapUVRect);
						READ_MICRO_CHUNK (cload, 	VARID_MESSAGESFADEOUTUVRECT,			MessagesFadeoutUVRect);
						READ_MICRO_CHUNK (cload, 	VARID_MESSAGESBACKGROUNDUVRECT,		MessagesBackgroundUVRect);
						READ_MICRO_CHUNK (cload, 	VARID_MESSAGESTEXTURESIZE,				MessagesTextureSize);
						READ_MICRO_CHUNK (cload, 	VARID_MESSAGESICONPOS,					MessagesIconPos);

						default:
							Debug_Say(("Unhandled Micro Chunk:%d File:%s Line:%d\r\n",cload.Cur_Micro_Chunk_ID (),__FILE__,__LINE__));
							break;
					}
					cload.Close_Micro_Chunk();
				}
				break;

			default:
				Debug_Say(("Unhandled Chunk:%d File:%s Line:%d\r\n",cload.Cur_Chunk_ID(),__FILE__,__LINE__));
				break;

		}

		cload.Close_Chunk();
	}

	ObjectiveManager::Reload_Viewer ();

	//
	//	Ask the message window to recalculate its size...
	//
	MessageWindowClass *message_window = CombatManager::Get_Message_Window ();
	if (message_window != NULL) {
		message_window->Clear_Log ();
		message_window->Reset_Current_Rect ();
		message_window->Set_Window_Dirty (true);
	}

	return true;
}
