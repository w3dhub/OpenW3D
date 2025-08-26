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
 *                     $Archive:: /Commando/Code/Combat/objectives.cpp                        $* 
 *                                                                                             * 
 *                      $Author:: Patrick                                                     $* 
 *                                                                                             * 
 *                     $Modtime:: 1/24/02 5:10p                                               $* 
 *                                                                                             * 
 *                    $Revision:: 38                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "objectives.h"
#include "gameobjref.h"
#include "debug.h"
#include "radar.h"
#include "scriptablegameobj.h"
#include "translatedb.h"
#include "combat.h"
#include "messagewindow.h"
#include "globalsettings.h"
#include "physicalgameobj.h"
#include "diaglog.h"
#include "WWAudio.h"
#include "hud.h"
#include "string_ids.h"


/*
**	Static member initialization
*/
SimpleDynVecClass<Objective*>	ObjectiveManager::ObjectiveList;
ObjectivesViewerClass			ObjectiveManager::Viewer;
bool									ObjectiveManager::DebugMode = false;
bool									ObjectiveManager::HUDUpdate = true;
int									ObjectiveManager::NumSpecifiedTertiaryObjectives;


/*
**
*/
const Vector3 & Objective::Type_To_Base_Color( void )
{
	static Vector3 color (1.0F, 1.0F, 1.0F);

	if ( HUDGlobalSettingsDef::Get_Instance() != NULL ) {
		switch ( Type ) {
			case ObjectiveManager::TYPE_PRIMARY:
				color = HUDGlobalSettingsDef::Get_Instance()->Get_Primary_Objective_Color(); 
				break;

			case ObjectiveManager::TYPE_SECONDARY:	
				color = HUDGlobalSettingsDef::Get_Instance()->Get_Secondary_Objective_Color();
				break;

			case ObjectiveManager::TYPE_TERTIARY:
				color = HUDGlobalSettingsDef::Get_Instance()->Get_Tertiary_Objective_Color();
				break;


		}
	}

	return color;
}


/*
**
*/
const Vector3 & Objective::Type_To_Color( void ) 
{
	static Vector3 color (1.0F, 1.0F, 1.0F);
	color = Type_To_Base_Color ();

	//
	//	Dim the colors if this objective has been accomplished
	//
	if( Status == ObjectiveManager::STATUS_ACCOMPLISHED ) {
		color *= 0.6F;
	}

	return color;
}

const WCHAR * Objective::Type_To_Name( void ) 
{
	switch ( Type ) {
		case ObjectiveManager::TYPE_PRIMARY:		return TRANSLATE (IDS_MENU_TEXT145);
		case ObjectiveManager::TYPE_SECONDARY:		return TRANSLATE (IDS_MENU_TEXT113);
		case ObjectiveManager::TYPE_TERTIARY:		return TRANSLATE (IDS_MENU_TERTIARY);
		default:												return TRANSLATE (IDS_LOCALE_UNKNOWN);
	}
}

const WCHAR * Objective::Status_To_Name( void ) 
{
	if ( Status == ObjectiveManager::STATUS_ACCOMPLISHED )	return TRANSLATE (IDS_MENU_OBJ_ACCOMPLISHED);
	if ( Status == ObjectiveManager::STATUS_FAILED )			return TRANSLATE (IDS_MENU_OBJ_FAILED);
	if ( Status == ObjectiveManager::STATUS_HIDDEN )			return TRANSLATE (IDS_MENU_OBJ_HIDDEN);
	return TRANSLATE (IDS_MENU_OBJ_PENDING);
}

const Vector3 & Objective::Status_To_Color( void ) 
{
	static	const	Vector3	RED(1,0,0);
	static	const	Vector3	GREEN(0,1,0);
	static	const	Vector3	YELLOW(1,1,0);
	static	const	Vector3	GREY(0.8F,0.8F,0.8F);

	switch ( Status ) {
		case ObjectiveManager::STATUS_IS_PENDING:		return GREEN;
		case ObjectiveManager::STATUS_ACCOMPLISHED:		return YELLOW;
		case ObjectiveManager::STATUS_FAILED:			return RED;
		case ObjectiveManager::STATUS_HIDDEN:			return GREY;
		default:										return GREEN;
	}
}

Objective::Objective( void ) :
	ID( 0 ),
	Type( 0 ),
	Status( 0 ),
	DrawBlip( false ),
	Position( 0,0,0 ),
	BlipIntensity( 0 ), 
	HUDMessageStringID( 0 ),
	LongDescriptionID( 0 ),
	ShortDescriptionID( 0 ),
	HUDPriority( 0 ),
	Age( 0 )
{
}

Objective::~Objective( void )
{
	Set_Object( NULL );
}

int	Objective::Radar_Blip_Color_Type( void )
{
	return Type - ObjectiveManager::TYPE_PRIMARY + RadarManager::BLIP_COLOR_TYPE_PRIMARY_OBJECTIVE;
}

void	Objective::Set_Object( PhysicalGameObj * object )
{
	PhysicalGameObj * obj = (PhysicalGameObj*)Object.Get_Ptr();
	if ( obj != NULL ) {
		obj->Reset_Radar_Blip_Shape_Type();
		obj->Reset_Radar_Blip_Color_Type();
	}
	Object = object;
	Update_Object_Blip();
}

void	Objective::Update_Object_Blip( void )
{
	PhysicalGameObj * obj = (PhysicalGameObj*)Object.Get_Ptr();
	if ( obj != NULL ) {
		if ( Status == ObjectiveManager::STATUS_IS_PENDING ) {
			obj->Set_Radar_Blip_Shape_Type( RadarManager::BLIP_SHAPE_TYPE_OBJECTIVE );
			obj->Set_Radar_Blip_Color_Type( Radar_Blip_Color_Type() );
		} else {
			obj->Reset_Radar_Blip_Shape_Type();
			obj->Reset_Radar_Blip_Color_Type();
		}
	}
}

Vector3	Objective::Get_Position( void )
{
	if ( Object.Get_Ptr() != NULL ) {
		Vector3 pos;
		Object.Get_Ptr()->Get_Position( &pos );
		return pos;
	}
	return Position;
}

/*
**
*/
enum	{
	CHUNKID_VARIABLES						=	629001440,
	CHUNKID_OBJECT,

	MICROCHUNKID_ID						=	1,
	MICROCHUNKID_TYPE,
	MICROCHUNKID_STATUS,
	MICROCHUNKID_DESCRIPTION,
	MICROCHUNKID_DESCRIPTION_SOUND,
	XXXMICROCHUNKID_RADAR_MARKER_ID,
	MICROCHUNKID_DESCRIPTION_ID,
	MICROCHUNKID_DRAW_BLIP,
	MICROCHUNKID_POSITION,
	MICROCHUNKID_LONG_DESCRIPTION_ID,
	MICROCHUNKID_AGE,
	MICROCHUNKID_HUD_POG_TEXTURE_NAME,
	MICROCHUNKID_HUD_MESSAGE_STRING_ID,
	MICROCHUNKID_HUD_PRIORITY,
	MICROCHUNKID_HUD_AGE,
};

bool	Objective::Save( ChunkSaveClass & csave )
{
	csave.Begin_Chunk( CHUNKID_VARIABLES );
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_ID, ID );
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_TYPE, Type );                    
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_STATUS, Status );                  
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_DESCRIPTION_ID, ShortDescriptionID );             
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_LONG_DESCRIPTION_ID, LongDescriptionID );
		WRITE_MICRO_CHUNK_WWSTRING( csave, 	MICROCHUNKID_DESCRIPTION_SOUND, DescriptionSoundFilename );
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_DRAW_BLIP, DrawBlip );
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_POSITION, Position );
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_AGE, Age );
		WRITE_MICRO_CHUNK_WWSTRING( csave, 	MICROCHUNKID_HUD_POG_TEXTURE_NAME, HUDPogTextureName );
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_MESSAGE_STRING_ID, HUDMessageStringID );
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_PRIORITY, HUDPriority );
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_HUD_AGE, Age );
	csave.End_Chunk();


	if ( Object.Get_Ptr() != NULL ) {
		csave.Begin_Chunk( CHUNKID_OBJECT );
		Object.Save( csave );
		csave.End_Chunk();
	}

	return true;
}

bool	Objective::Load( ChunkLoadClass &cload )
{
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_ID, ID );
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_TYPE, Type );                    
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_STATUS, Status );                  
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_DESCRIPTION_ID, ShortDescriptionID );
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_LONG_DESCRIPTION_ID, LongDescriptionID );
						READ_MICRO_CHUNK_WWSTRING( cload, 	MICROCHUNKID_DESCRIPTION_SOUND, DescriptionSoundFilename );
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_DRAW_BLIP, DrawBlip );
   						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_POSITION, Position );
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_AGE, Age );
						READ_MICRO_CHUNK_WWSTRING( cload, 	MICROCHUNKID_HUD_POG_TEXTURE_NAME, HUDPogTextureName );
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_MESSAGE_STRING_ID, HUDMessageStringID );
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_PRIORITY, HUDPriority );
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_HUD_AGE, Age );

						default:
							Debug_Say(("Unhandled Chunk:%d File:%s Line:%d\r\n",cload.Cur_Chunk_ID(),__FILE__,__LINE__));
							break;
					}
					cload.Close_Micro_Chunk();
				}
				break;

			case CHUNKID_OBJECT:
				Object.Load( cload );
				break;

			default:
				Debug_Say(("Unhandled Chunk:%d File:%s Line:%d\r\n",cload.Cur_Chunk_ID(),__FILE__,__LINE__));
				break;

		}
		cload.Close_Chunk();
	}
	return true;
}

/*
**
*/
Objective * ObjectiveManager::Add_Loadable_Objective( void )
{
	Objective * objective = new Objective();
	ObjectiveList.Add( objective );
	return objective;
}

Objective * ObjectiveManager::Find_Objective( int id ) 
{
	for ( int i = 0; i < ObjectiveList.Count(); i++ ) {
		if ( ObjectiveList[i]->ID == id ) {
			return ObjectiveList[i];
		}
	}
	return NULL;
}

/*
**
*/
void	ObjectiveManager::Init( void )
{
	Viewer.Initialize();
	HUDUpdate = true;
	NumSpecifiedTertiaryObjectives = 0;
}

void	ObjectiveManager::Shutdown( void )
{
	Viewer.Shutdown();

	Reset();
}

void	ObjectiveManager::Reset( void )
{
	HUDUpdate = true;
	while ( ObjectiveList.Count() != 0 ) {
		Objective * objective = ObjectiveList[0];
		ObjectiveList.Delete( 0 );
		delete objective;
	}	
}

/*
**
*/
enum	{
	CHUNKID_OBJECTIVE_ENTRY		=	629001432,
	CHUNKID_MANAGER_VARIABLES,

	MICROCHUNKID_NUM_SPECIFIED_TERTIARY_OBJECTIVES		=	1,
};

/*
**
*/
bool	ObjectiveManager::Save( ChunkSaveClass &csave )
{
	for ( int i = 0; i < ObjectiveList.Count(); i++ ) {
		csave.Begin_Chunk( CHUNKID_OBJECTIVE_ENTRY );
			ObjectiveList[i]->Save( csave );
		csave.End_Chunk();
	}

	csave.Begin_Chunk( CHUNKID_MANAGER_VARIABLES );
		WRITE_MICRO_CHUNK( csave, 	MICROCHUNKID_NUM_SPECIFIED_TERTIARY_OBJECTIVES,	NumSpecifiedTertiaryObjectives );
	csave.End_Chunk();

	return true;
}

bool	ObjectiveManager::Load( ChunkLoadClass &cload )
{
	WWASSERT( ObjectiveList.Count() == 0 );

	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_MANAGER_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK( cload, 	MICROCHUNKID_NUM_SPECIFIED_TERTIARY_OBJECTIVES,	NumSpecifiedTertiaryObjectives );
						default:
							Debug_Say(("Unhandled Chunk:%d File:%s Line:%d\r\n",cload.Cur_Chunk_ID(),__FILE__,__LINE__));
							break;
					}
					cload.Close_Micro_Chunk();
				}
				break;

			case CHUNKID_OBJECTIVE_ENTRY:
			{
				Objective * objective = Add_Loadable_Objective();
				objective->Load( cload );
				break;
			}

			default:
				Debug_Say(("Unhandled Chunk:%d File:%s Line:%d\r\n",cload.Cur_Chunk_ID(),__FILE__,__LINE__));
				break;

		}
		cload.Close_Chunk();
	}

	Viewer.Update ();

	HUDUpdate = true;

	return true;
}

/*
**
*/
void	ObjectiveManager::Add_Objective( int id, int type, int status, int short_description_id, int long_description_id, char * description_sound_filename )
{
	if ( Find_Objective( id ) != NULL ) {
		Debug_Say(( "Adding a duplicate Objective ID\n" ));
		return;
	}

	Objective * objective = Add_Loadable_Objective();
	objective->ID = id;
	objective->Type = type;
	objective->Status = status;
	objective->ShortDescriptionID = short_description_id;
	objective->LongDescriptionID = long_description_id;
	objective->DescriptionSoundFilename = description_sound_filename;


#if 01
	//
	//	Update our EVA message window
	//
	if ( status != ObjectiveManager::STATUS_HIDDEN ) {
		WideStringClass message;
		WideStringClass description = TranslateDBClass::Get_String( short_description_id );
		message.Format( TRANSLATE (IDS_OBJ_NEW_OBJ), objective->Type_To_Name(), description );
		CombatManager::Get_Message_Window ()->Add_Message (message, objective->Type_To_Base_Color());

		HUDClass::Add_Objective( type );
	}

	const char * preset_name = NULL;
	switch ( type ) {
		case ObjectiveManager::TYPE_PRIMARY:	preset_name = "EVA_Primary_Add"; break;
		case ObjectiveManager::TYPE_SECONDARY:	preset_name = "EVA_Secondary_Add"; break;
	}
	if ( preset_name ) {
//		WWAudioClass::Get_Instance()->Create_Instant_Sound( preset_name, Matrix3D(1) );
	}

#endif

	Viewer.Update ();
	HUDUpdate = true;
	return ;
}

void	ObjectiveManager::Remove_Objective( int id )
{
	Objective * objective = Find_Objective( id );
	if ( objective != NULL ) {

#if 01
		WideStringClass message;
		message.Format( TRANSLATE (IDS_OBJ_CANCELLED), objective->Type_To_Name () );
		CombatManager::Get_Message_Window ()->Add_Message( message, objective->Type_To_Base_Color() );

//		WWAudioClass::Get_Instance()->Create_Instant_Sound( "EVA_Complete", Matrix3D(1) );
#endif

		ObjectiveList.Delete( objective );
		delete objective;
	} else {
		Debug_Say(( "Objective not found to delete\n" ));
	}

	Viewer.Update ();
	HUDUpdate = true;
	return ;
}

void	ObjectiveManager::Set_Objective_Status( int id, int status )
{
	Objective * objective = Find_Objective( id );
	if ( objective != NULL ) {

		bool is_unhiding =	(objective->Status == ObjectiveManager::STATUS_HIDDEN) && 
									(status != ObjectiveManager::STATUS_HIDDEN);

		objective->Status = status;

		if ( status == STATUS_FAILED ) {
			DIAG_LOG(( "OBFA", "%d; %s", id, objective->Status_To_Name() ));
		}

		if ( status == STATUS_ACCOMPLISHED ) {
			DIAG_LOG(( "OBCO", "%d; %s", id, objective->Status_To_Name() ));
		}

		objective->Update_Object_Blip();
	
		if ( is_unhiding ) {
			objective->Age = 0;		// Reset age
		}

#if 01
		//
		// Special case changing an objective from hidden to pending... (Note:  for
		// a completed objective, we display the normal message even if it was hidden).
		//
		WideStringClass message;
		if (is_unhiding && (status != ObjectiveManager::STATUS_ACCOMPLISHED)) {
			WideStringClass description = TranslateDBClass::Get_String( objective->ShortDescriptionID );
			message.Format( TRANSLATE (IDS_OBJ_NEW_OBJ), objective->Type_To_Name(), description );

			HUDClass::Add_Objective( objective->Type );
		} else {
			if ( objective->Status != ObjectiveManager::STATUS_HIDDEN ) {
				message.Format( TRANSLATE (IDS_OBJ_STATUS_CHANGED), objective->Type_To_Name(), objective->Status_To_Name() );
			}
		}
		CombatManager::Get_Message_Window ()->Add_Message( message, objective->Type_To_Base_Color() );
#endif

	} else {
		Debug_Say(( "Objective not found to set status\n" ));
	}

	Sort_Objectives();
	Viewer.Update ();
	HUDUpdate = true;
	return ;
}

void	ObjectiveManager::Change_Objective_Type( int id, int type )
{
	Objective * objective = Find_Objective( id );
	if ( objective != NULL ) {
		objective->Type = type;
		objective->Update_Object_Blip();
		//DebugManager::Display_Text( "Mission objective priority changed\n", objective->Type_To_Color () );		
	} else {
		Debug_Say(( "Objective not found to change type\n" ));
	}

	Viewer.Update ();
	HUDUpdate = true;
	return ;
}

void	ObjectiveManager::Set_Objective_Radar_Blip( int id, Vector3 position )
{
	Objective * objective = Find_Objective( id );
	if ( objective != NULL ) {
		objective->Set_Object( NULL );
		objective->Position = position;
		objective->DrawBlip = true;
	} else {
		Debug_Say(( "Objective not found to set_radar_blip\n" ));
	}
}

void	ObjectiveManager::Set_Objective_Radar_Blip( int id, PhysicalGameObj * object )
{
	Objective * objective = Find_Objective( id );
	if ( objective != NULL ) {
		objective->Set_Object( object );
	} else {
		Debug_Say(( "Objective not found to set_radar_blip\n" ));
	}
}

////////////////////////////////////////////////////////////////
//
//	fnCompareObjectivesCallback
//
////////////////////////////////////////////////////////////////
int __cdecl ObjectiveManager::ObjectiveSortCallback( const void *elem1, const void *elem2 )
{
   WWASSERT (elem1 != NULL);
   WWASSERT (elem2 != NULL);
   Objective *objective1 = *((Objective **)elem1);
   Objective *objective2 = *((Objective **)elem2);	

	//	Sort first on status, low first
	if (objective1->Status < objective2->Status) {
		return -1;
	}
	if (objective1->Status > objective2->Status) {
		return 1;
	}

	//	Sort Next on Priority, high first
	if (objective1->HUDPriority > objective2->HUDPriority) {
		return -1;
	}
	if (objective1->HUDPriority < objective2->HUDPriority) {
		return 1;
	}

	return 0;
}

void	ObjectiveManager::Sort_Objectives( void )
{
	// Assume the rest of the list is sorted
	// And bubble him to where he belongs
	if ( ObjectiveList.Count() != 0 ) {
		::qsort (&ObjectiveList[0], ObjectiveList.Count(), sizeof(ObjectiveList[0]), ObjectiveSortCallback );
	}
}

void	ObjectiveManager::Set_Objective_HUD_Info( int id, float priority, const char * texture_name, int message_id )
{
	Objective * objective = Find_Objective( id );
	if ( objective != NULL ) {
		objective->HUDPriority = priority;
		objective->HUDPogTextureName = texture_name;
		objective->HUDMessageStringID = message_id;

		Sort_Objectives();
		HUDUpdate = true;

	} else {
		Debug_Say(( "Objective not found to Set_Objective_HUD_Info\n" ));
	}

}

void	ObjectiveManager::Set_Objective_HUD_Info( int id, float priority, const char * texture_name, int message_id, const Vector3 & position )
{
	Objective * objective = Find_Objective( id );
	if ( objective != NULL ) {
		objective->HUDPriority = priority;
		objective->HUDPogTextureName = texture_name;
		objective->HUDMessageStringID = message_id;
		objective->Position = position;
		Sort_Objectives();
		HUDUpdate = true;

	} else {
		Debug_Say(( "Objective not found to Set_Objective_HUD_Info\n" ));
	}

}

int	ObjectiveManager::Get_Num_HUD_Objectives( void )
{
	// Assume the pendings are first, in priority order
	int count = 0;
	for ( int i = 0; i < ObjectiveList.Count(); i++ ) {
		if ( ( ObjectiveList[i]->Status == STATUS_IS_PENDING ) && 
			  ( ObjectiveList[i]->HUDPriority > 0 ) ) {
			count++;
		}
	}
	return count; 
}

const char *	ObjectiveManager::Get_HUD_Objectives_Pog_Texture_Name( int index )
{
	return ObjectiveList[index]->HUDPogTextureName;
}

const WCHAR *	ObjectiveManager::Get_HUD_Objectives_Message( int index )
{
	return TranslateDBClass::Get_String( ObjectiveList[index]->HUDMessageStringID );
}

Vector3			ObjectiveManager::Get_HUD_Objectives_Location( int index )
{
	return ObjectiveList[index]->Get_Position();
}

float	ObjectiveManager::Get_HUD_Objectives_Age( int index )
{
	return ObjectiveList[index]->Age;
}

int		ObjectiveManager::Get_Num_Objectives( int type )
{
	int count = 0;
    for ( int i = 0; i < ObjectiveList.Count(); i++ ) {
		if ( ( ObjectiveList[i]->Type == type ) && 
			 ( ObjectiveList[i]->Status != STATUS_HIDDEN ) ) {
			count++;
		}
	}

	if ( type == TYPE_TERTIARY ) {
		if ( count < NumSpecifiedTertiaryObjectives ) {
			count = NumSpecifiedTertiaryObjectives;
		}
	}

	return count; 
}

int		ObjectiveManager::Get_Num_Completed_Objectives( int type )
{
	int count = 0;
    for ( int i = 0; i < ObjectiveList.Count(); i++ ) {
		if ( ( ObjectiveList[i]->Type == type ) && 
			 ( ObjectiveList[i]->Status == STATUS_ACCOMPLISHED ) ) {
			count++;
		}
	}
	return count; 
}

void	ObjectiveManager::Update( float dt )
{
	for ( int i = 0; i < ObjectiveList.Count(); i++ ) {
		if ( ObjectiveList[i]->Status != STATUS_HIDDEN ) {
			ObjectiveList[i]->Age += dt;
		}
	}
}
