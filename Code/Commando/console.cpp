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
 *                     $Archive:: /Commando/Code/Commando/console.cpp       $*
 *                                                                                             *
 *                      $Author:: Jani_p                                                      $*
 *                                                                                             *
 *                     $Modtime:: 3/14/02 4:07p                                               $*
 *                                                                                             *
 *                    $Revision:: 138                                                         $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "console.h"
#include "consolefunction.h"
#include "textdisplay.h"
#include "assets.h"
#include "font3d.h"
#include "debug.h"
#include "timemgr.h"
#include "input.h"
#include "miscutil.h"
#include "cnetwork.h"
#include "teammanager.h"
#include "scene.h"
#include "ww3d.h"
#include <stdio.h>
#include "WWAudio.h"
#include "AudibleSound.h"
//#include "gamesettings.h"
#include "gamedata.h"
#include "overlay.h"
#include "combat.h"
#include "camera.h"
#include "ccamera.h"
#include "gameobjmanager.h"
#include "smartgameobj.h"
#include "playermanager.h"
#include "_globals.h"
#include "registry.h"
#include "phys3.h"
#include "wolgmode.h"
#include "devoptions.h"
#include "playertype.h"
#include "pscene.h"
#include "translatedb.h"
#include "string_ids.h"
#include "vehicle.h"
#include "wheelvehicle.h"
#include "wwprofile.h"
#include "wwmemlog.h"
#include "wheel.h"
#include "statistics.h"
#include "meshmdl.h"
#include "w3d_file.h"		// for SURFACE_TYPE_STRINGS
#include "colors.h"
#include "chatshre.h"
#include "dx8renderer.h"
#include "dx8wrapper.h"
#include "umbrasupport.h"
#include "render2d.h"
#include "sortingrenderer.h"
#include "sctextobj.h"
#include "textdisplay.h"
#include "trackedvehicle.h"
#include "dx8rendererdebugger.h"
#include "FastAllocator.h"
#include <WWOnline/WOLSession.h>
#include "ConsoleMode.h"

//#include "dlgmpingamechat.h"

#include <sstream>

static bool profile_log_active;
static StringClass* profile_log_names;

//
// ConsoleGameModeClass statics
//
ConsoleGameModeClass * ConsoleGameModeClass::Instance = NULL;
const float ConsoleGameModeClass::LeftMargin = 0.02f;

/*
**	called each time through the main loop
*/
void 	ConsoleGameModeClass::Init()
{
	ConsoleFunctionManager::Init();

	WWASSERT( ConsoleGameModeClass::Instance == NULL );

	ConsoleGameModeClass::Instance = this;
	InputActive = false;
	InputLine[0] = 0;
	Clear_Suggestion();

//	FPSActive = false;
	FPSFrames = 0;
	FPSTime = 0.0f;
	FPSLastTime = 0.0f;
	FPS = 0.0f;

//	ShowPlayerPosition = false;

	PerformanceSamplingActive = false;

	Load_Registry_Keys();

	ProfileIterator = NULL;
}

/*
**	called each time through the main loop
*/
void 	ConsoleGameModeClass::Shutdown()
{
	Save_Registry_Keys();

	WWASSERT( ConsoleGameModeClass::Instance == this );

	ConsoleGameModeClass::Instance = NULL;

	ConsoleFunctionManager::Shutdown();
}

void ConsoleGameModeClass::Load_Registry_Keys(void)
{
//	Debug_Say(( "CombatGameModeClass::Load_Registry_Keys...\n" ));
	RegistryClass * registry = new RegistryClass( APPLICATION_SUB_KEY_NAME_OPTIONS );
	WWASSERT( registry );
	if ( registry->Is_Valid() ) {

      WW3D::Set_Screen_UV_Bias( registry->Get_Int( "ScreenUVBias", 1 ) != 0 );

      Get_Console()->Set_FPS_Active( registry->Get_Int( "FPS", 1 ) != 0 );
	}
	delete registry;
}

void ConsoleGameModeClass::Save_Registry_Keys(void)
{
//	Debug_Say(( "CombatGameModeClass::Save_Registry_Keys...\n"));
	RegistryClass * registry = new RegistryClass( APPLICATION_SUB_KEY_NAME_OPTIONS );
	WWASSERT( registry );
	if ( registry->Is_Valid() ) {
		registry->Set_Int( "ScreenUVBias", WW3D::Is_Screen_UV_Biased() );

//      registry->Set_Int( "TextureReduction", WW3D::Get_Texture_Reduction() );
//      registry->Set_Int( "TextureThumbnail", WW3D::Get_Texture_Thumbnail_Mode() );
//      registry->Set_Int( "TextureCompression", WW3D::Get_Texture_Compression_Mode() );
//      registry->Set_Int( "NPatchesLevel", WW3D::Get_NPatches_Level() );

      registry->Set_Int( "FPS", Get_Console()->Is_FPS_Active() );
	}
	delete registry;
}



#define	BACKSPACE_KEY		8
#define	ESC_KEY				27
#define	ENTER_KEY			13
#define	TAB_KEY				0x09
#define	SPACE_KEY			0x20

/*
**	called each time through the main loop
*/
void 	ConsoleGameModeClass::Think()
{
	WWPROFILE( "Console Think" );

   /*
	//
	// TSS092501 - disabling this because it is fatal.
	// If you want it, you'll have to fix it!
	//
	if (Input::Get_State(INPUT_FUNCTION_VERBOSE_HELP)) {
		ConsoleFunctionManager::Next_Verbose_Help_Screen();
	}
	*/

	//cNetwork::Watch_Wol_Location(Drawer, Font);

   if ( !InputActive ) {
      bool enable_console = false;

// FIXME TODO: relocate and provide UI for player communication.



// HACK: Disable console in ATI demo
//#ifndef ATI_DEMO_HACK
		if (Input::Get_State(INPUT_FUNCTION_BEGIN_CONSOLE)) {
         enable_console = true;
         ConsoleInputType = INPUT_FUNCTION_BEGIN_CONSOLE;
         strcpy(InputLine, "Command >");
      }
//#endif

      if (enable_console) {
		   InputActive = true;
         PromptLength = ::strlen(InputLine);
		   Input::Console_Enable();
			Clear_Suggestion();
      }
	}



	/*
	** Handle Console Input
	*/
	if ( InputActive ) {
WWPROFILE( "Input Active" );

		int key = Input::Console_Get_Key();

		while ( key ) {

			size_t len = ::strlen( InputLine );

			switch( key ) {

				case ENTER_KEY:
					if (ConsoleInputType == INPUT_FUNCTION_BEGIN_CONSOLE) {
						Accept_Suggestion(InputLine + PromptLength);
						len = ::strlen(InputLine);
						Clear_Suggestion();
					}
					//Parse_Input( InputLine );
					Parse_Input( InputLine + PromptLength );

				case ESC_KEY:
					InputActive = false;
					Input::Console_Disable();
					break;

				case BACKSPACE_KEY:
					//if ( len > 0 ) {
					if ( len > PromptLength ) {
						InputLine[ --len ] = 0;
						if (ConsoleInputType == INPUT_FUNCTION_BEGIN_CONSOLE) {
							Update_Suggestion(InputLine + PromptLength,true);
						}

					} else {
                  //DebugManager::Display();
               }
					break;

				case TAB_KEY:
					if (ConsoleInputType == INPUT_FUNCTION_BEGIN_CONSOLE) {
						Update_Suggestion(InputLine + PromptLength,true);
					}
					break;

				case SPACE_KEY:
					// Accept any suggested command line completion and fall through to default
					if (ConsoleInputType == INPUT_FUNCTION_BEGIN_CONSOLE) {
						Accept_Suggestion(InputLine + PromptLength);
						len = ::strlen(InputLine);
					}

				default:
					if ( len + 1 < static_cast<size_t>(MAX_INPUT_LINE_LENGTH)) {
						InputLine[ len++ ] = key;
						InputLine[ len ] = 0;

						if (ConsoleInputType == INPUT_FUNCTION_BEGIN_CONSOLE) {
							Update_Suggestion(InputLine + PromptLength,false);
						}
               } else {
                  //DebugManager::Display();
               }
					break;
			}

			key = Input::Console_Get_Key();

		}

		/*
		** Console Input Output
		*/
		char mess[MAX_INPUT_LINE_LENGTH+2];
		strcpy( mess, InputLine );

		static int flash = 0;
		if ( (int)(TimeManager::Get_Seconds()*4) & 1 ) {
			strcat( mess, "|" );
		}
		strcat( mess, "\n" );

		Vector3	color(1.0f,1.0f,1.0f);
		switch (ConsoleInputType) {
			/*
			case INPUT_FUNCTION_BEGIN_PUBLIC_MESSAGE :
				color = COLOR_PUBLIC_TEXT;
				break;
			case INPUT_FUNCTION_BEGIN_TEAM_MESSAGE :
				color = cNetwork::Get_My_Color();
				break;
			case INPUT_FUNCTION_BEGIN_PRIVATE_MESSAGE :
				color = COLOR_PRIVATE_TEXT;
				break;
			*/
			case INPUT_FUNCTION_BEGIN_CONSOLE :
				color = COLOR_CONSOLE_TEXT;
				break;
			default :
				DIE;
				break;
		}

		if (Get_Text_Display()) {
			WWASSERT( Get_Text_Display() );
			Get_Text_Display()->Set_Input_Text( mess );
			Get_Text_Display()->Set_Help_Text( HelpLine );
		}

	} else {

		if (Get_Text_Display()) {
			WWASSERT( Get_Text_Display() );
			Vector3 color(1,1,1);
			Get_Text_Display()->Set_Input_Text( "" );
			Get_Text_Display()->Set_Help_Text( "" );
		}

	}

	/****************************************************************************************
	**
	** Vis Warning, In debug and profile mode, always display a warning if vis is missing
	**
	****************************************************************************************/

#ifdef WWDEBUG
{	WWPROFILE( "Vis Check" );
	if (Get_Text_Display()) {
		PhysicsSceneClass * scene = PhysicsSceneClass::Get_Instance();
		Get_Text_Display()->Display_Vis_Warning( scene && scene->Is_Vis_Sector_Missing() );
	}
}
#endif

	// Note:  As you add more stats, also add to the stats help command.

	/****************************************************************************************
	**
	** Frame-Rate Stats.  Activate with 'stats fps'
	**
	****************************************************************************************/

	if (Get_Text_Display()) {

		// Update text if it's visible
		if (StatisticsDisplayManager::Is_Current_Display("histogram")) {
			StringClass working_string(true);
			StringClass message(true);
			float	cur_time = TimeManager::Get_Seconds();

			FPSTime += cur_time - FPSLastTime;
			FPSLastTime = cur_time;
			FPSFrames++;
			if ( FPSTime >= 1.0f ) {
				FPS = (float)FPSFrames/FPSTime;
				FPSTime = 0.0f;
				FPSFrames = 0;
			}

			working_string.Format("%2.0f fps\n\n",FPS);
			message += working_string;

			unsigned slot_count=TimeManager::Peek_Frame_Time_Histogram().Get_Slot_Count();
			if (slot_count) {
				unsigned char* slots=new unsigned char[slot_count];
				TimeManager::Peek_Frame_Time_Histogram().Get_Packed_Report(slots);
				unsigned i;
				for (i=0;i<slot_count;++i) {
					working_string.Format("%d: %d\n",
						unsigned(TimeManager::Peek_Frame_Time_Histogram().Get_Step()*float(i)),
						slots[i]);
					message+=working_string;
				}
				delete[] slots;
			}
			StatisticsDisplayManager::Set_Stat( "histogram", message );
		}

		// Update text if it's visible
		if (StatisticsDisplayManager::Is_Current_Display("fps")) {
			StringClass working_string(true);
			StringClass message(true);
			float	cur_time = TimeManager::Get_Seconds();

			FPSTime += cur_time - FPSLastTime;
			FPSLastTime = cur_time;
			FPSFrames++;
			if ( FPSTime >= 1.0f ) {
				FPS = (float)FPSFrames/FPSTime;
				FPSTime = 0.0f;
				FPSFrames = 0;
			}

			working_string.Format("%2.0f fps\n",FPS);
			message += working_string;

#ifdef ATI_DEMO_HACK
			if (WW3D::Get_NPatches_Level()>1) {
				working_string.Format(
					"\nNPATCH level: %d\n",
					WW3D::Get_NPatches_Level());
			}
			else {
				working_string.Format("\nNPATCH OFF\n");
			}
			message += working_string;
#endif
			working_string.Format(
				"\npolys/frame: %7d\npolys/second %4dk\n",
				WW3D::Get_Last_Frame_Poly_Count(),
				int(WW3D::Get_Last_Frame_Poly_Count()*FPS/1000));
			message += working_string;

			unsigned ffheap=FastAllocatorGeneral::Get_Allocator()->Get_Total_Heap_Size();
			unsigned ffuse=FastAllocatorGeneral::Get_Allocator()->Get_Total_Allocated_Size();
			unsigned actualuse=FastAllocatorGeneral::Get_Allocator()->Get_Total_Actual_Memory_Usage();
			unsigned count=FastAllocatorGeneral::Get_Allocator()->Get_Total_Allocation_Count();
			working_string.Format(
				"\nMalloc count: %d\n"
				"Free count: %d\n"
				"FF Heap: %d.%3.3d.%3.3d (%d Mb)\n"
				"FF Use: %d.%3.3d.%3.3d (%d Mb)\n"
				"Actual Use: %d.%3.3d.%3.3d (%d Mb)\n"
				"FF Count: %d\n"
				,
				WW3D::Get_Last_Frame_Memory_Allocation_Count(),
				WW3D::Get_Last_Frame_Memory_Free_Count(),
				ffheap/(1000*1000),(ffheap/1000)%1000,ffheap%1000, ffheap/(1024*1024),
				ffuse/(1000*1000),(ffuse/10000)%1000,ffuse%1000, ffuse/(1024*1024),
				actualuse/(1000*1000),(actualuse/1000)%1000,actualuse%1000, actualuse/(1024*1024),
				count);
			message += working_string;

#ifndef ATI_DEMO_HACK
			working_string.Format("verts/frame: %7d v/p ratio: %2.2f\n",
					WW3D::Get_Last_Frame_Vertex_Count(),
					float(WW3D::Get_Last_Frame_Vertex_Count())/float(WW3D::Get_Last_Frame_Poly_Count()));
			message += working_string;
#endif

			int texture_reduction = WW3D::Get_Texture_Reduction();
			if (texture_reduction > 0) {
				working_string.Format("Tex. Red. %d\n\n", texture_reduction);
				message += working_string;
			}

			// Only display texture and vertex processor statistics if texture statistics recording is enabled

			if (Debug_Statistics::Get_Record_Texture_Mode()!=Debug_Statistics::RECORD_TEXTURE_NONE) {
				// Texture usage
				int red_size=Debug_Statistics::Get_Record_Texture_Size();
				int lightmap_size=Debug_Statistics::Get_Record_Lightmap_Texture_Size();
				int procedural_size=Debug_Statistics::Get_Record_Procedural_Texture_Size();
				working_string.Format(
					"\n"
					"textures/frame: %5d\n"
					"tex memory used: %d.%dMb\n"
					"texture changes: %d\n"
					,
					Debug_Statistics::Get_Record_Texture_Count(),
					red_size>>20,
					(10*(red_size>>10)>>10)%10,
					Debug_Statistics::Get_Record_Texture_Change_Count());
				message += working_string;

				// Lightmap info
				working_string.Format(
					"\n"
					"lightmaps/frame: %5d\n"
					"lightmap memory used: %d.%dMb\n"
					,
					Debug_Statistics::Get_Record_Lightmap_Texture_Count(),
					lightmap_size>>20,
					(10*(lightmap_size>>10)>>10)%10);
				message += working_string;

				// Procedural texture info
				working_string.Format(
					"\n"
					"procedural textures/frame: %5d\n"
					"procedural memory used: %d.%dMb\n\n"
					,
					Debug_Statistics::Get_Record_Procedural_Texture_Count(),
					procedural_size>>20,
					(10*(procedural_size>>10)>>10)%10);
				message += working_string;

				// Texture info
				red_size=TextureClass::_Get_Total_Texture_Size();
				lightmap_size=TextureClass::_Get_Total_Lightmap_Texture_Size();
				procedural_size=TextureClass::_Get_Total_Procedural_Texture_Size();
				working_string.Format(
					"\n"
					"total tex loaded: %5d\n"
					"total size of textures: %d.%dMb\n"
					"\n"
					"total lightmaps: %5d\n"
					"total size of lightmaps: %dMb\n"
					"\n"
					"total procedural textures: %5d\n"
					"total size of procedural textures: %dMb\n"
					,
					TextureClass::_Get_Total_Texture_Count(),
					red_size>>20,
					(10*(red_size>>10)>>10)%10,
					TextureClass::_Get_Total_Lightmap_Texture_Count(),
					lightmap_size>>20,
					(10*(lightmap_size>>10)>>10)%10,
					TextureClass::_Get_Total_Procedural_Texture_Count(),
					procedural_size>>20,
					(10*(procedural_size>>10)>>10)%10);
				message += working_string;

				// Thumbnail info
				red_size=TextureClass::_Get_Total_Locked_Surface_Size();
				working_string.Format(
					"total thumbnails: %5d\n"
					"total size of thumbnails: %d.%dMb\n"
					,
					TextureClass::_Get_Total_Locked_Surface_Count(),
					red_size>>20,
					(10*(red_size>>10)>>10)%10);
				message += working_string;

				if (Debug_Statistics::Get_Record_Texture_Mode()==Debug_Statistics::RECORD_TEXTURE_DETAILS) {
					message+="\n"
								"<F9 + F5> Scroll up\n"
								"<F9 + F6> Scroll down\n"
								"\n";

					message+=Debug_Statistics::Get_Record_Texture_String();
				}

			}

			if (Debug_Statistics::Get_Record_Texture_Mode()==Debug_Statistics::RECORD_TEXTURE_DETAILS) {
				StatisticsDisplayManager::Set_Stat( "fps", message, 0xffffffcf );
			}
			else {
				StatisticsDisplayManager::Set_Stat( "fps", message );
			}
		}

		/****************************************************************************************
		**
		** Meshmodel Stats.  Activate with 'stats dx8'
		**
		****************************************************************************************/

		// Update text if it's visible
		if (StatisticsDisplayManager::Is_Current_Display("dx8")) {
			StringClass working_string(true);
			StringClass message(true);
			float	cur_time = TimeManager::Get_Seconds();

			FPSTime += cur_time - FPSLastTime;
			FPSLastTime = cur_time;
			FPSFrames++;
			if ( FPSTime >= 1.0f ) {
				FPS = (float)FPSFrames/FPSTime;
				FPSTime = 0.0f;
				FPSFrames = 0;
			}

			message.Format("%2.0f fps\n",FPS);

			working_string.Format(
				"Total DX8 calls: %d\n"
				"Total Polys: %d\n"
				"Total Verts: %d\n"
				"Skins rendered: %d\n"
				"(Total Polys in skins: %d)\n"
				"(Total Verts in skins: %d)\n"
				"Matrix changes: %d\n"
				"Texture changes: %d\n"
				"Material changes: %d\n"
				"VB changes: %d\n"
				"IB changes: %d\n"
				"Light changes: %d\n\n"
				"Sorted polys: %d\n"
				"Sorted verts: %d\n",
				DX8Wrapper::Get_Last_Frame_DX8_Calls(),
				Debug_Statistics::Get_DX8_Polygons(),
				Debug_Statistics::Get_DX8_Vertices(),
				Debug_Statistics::Get_DX8_Skin_Renders(),
				Debug_Statistics::Get_DX8_Skin_Polygons(),
				Debug_Statistics::Get_DX8_Skin_Vertices(),
				DX8Wrapper::Get_Last_Frame_Matrix_Changes(),
				Debug_Statistics::Get_Record_Texture_Count(),
				DX8Wrapper::Get_Last_Frame_Material_Changes(),
				DX8Wrapper::Get_Last_Frame_Vertex_Buffer_Changes(),
				DX8Wrapper::Get_Last_Frame_Index_Buffer_Changes(),
				DX8Wrapper::Get_Last_Frame_Light_Changes(),
				Debug_Statistics::Get_Sorting_Polygons(),
				Debug_Statistics::Get_Sorting_Vertices());
			message+=working_string;

	//		DX8MeshRendererContainerClass* n=DX8MeshRendererContainerClass::Head();
	//		int cont=0;
	//		while (n) {
				// First check if container it empty
	/*			bool empty=true;
				for (int a=0;a<DX8MeshRendererContainerClass::RENDER_CLASS_MAX;++a) {
					DX8MeshRendererClass* renderer=n->render_class_types[a].Get_Renderer_Head();
					if (renderer) {
						empty=false;
						break;
					}
				}

				// Log stats only if container is not empty
				if  (empty) {
					working_string.Format("Container: %d EMPTY\n",cont);
					message+=working_string;
				}
				else {
					working_string.Format("Container: %d\n",cont);
					message+=working_string;
					for (int a=0;a<DX8MeshRendererContainerClass::RENDER_CLASS_MAX;++a) {
						working_string.Format("Class: %s",
							DX8MeshRendererContainerClass::Get_Render_Class_Name(a));
						message+=working_string;

						DX8MeshRendererClass* renderer=n->render_class_types[a].Get_Renderer_Head();
						if (!renderer) {
							message+=" No registered renderers!\n";
						}
						else {
							message+="\n";
						}

						while (renderer) {
							if (renderer->stats_last_frame_add_calls || renderer->stats_last_frame_count) {
								working_string.Format("%24s Add: %3d Render: %3d Polys: %3d\n",
									renderer->name,
									renderer->stats_last_frame_add_calls,
									renderer->stats_last_frame_count,
									renderer->stats_last_frame_polys);
								message+=working_string;
							}

							renderer=renderer->Succ();
						}
					}
				}
	*/
	//		n=n->Succ();
	//			cont++;
	//		}

	/*		working_string.Format("Total add renders: %d\n",DX8MeshRendererClass::Get_Last_Frame_Render_Stats());
			message+=working_string;
			for (int ridx=0;ridx<DX8MeshRendererContainerClass::RENDER_CLASS_MAX;++ridx) {
				working_string.Format("Class: %s\nObjects: %d\nPolys: %d\n\n",
					DX8MeshRendererContainerClass::Get_Render_Class_Name(ridx),
					DX8MeshRendererClass::Get_Last_Frame_Render_Stats(ridx).count,
					DX8MeshRendererClass::Get_Last_Frame_Render_Stats(ridx).polys);
				message+=working_string;
			}
	*/		StatisticsDisplayManager::Set_Stat( "dx8", message, 0xffffffff );
		}








		/****************************************************************************************
		**
		** Audio Stats.  Activate with 'stats audio'
		**
		****************************************************************************************/

		// Update text if it's visible
		if (StatisticsDisplayManager::Is_Current_Display("audio")) {

			StringClass message (true);
			StringClass temp_string (true);
			int count_2d = WWAudioClass::Get_Instance ()->Get_2D_Sample_Count ();
			int count_3d = WWAudioClass::Get_Instance ()->Get_3D_Sample_Count ();

			message = "2D or Pseudo-3D Sounds:\n";

			//
			//	Add all the 2D sounds to the message
			//
			int sample_index;
			for (sample_index = 0; sample_index < count_2d; sample_index ++) {
				temp_string.Format (" %d.", sample_index + 1);
				message += temp_string;

				AudibleSoundClass *sound_obj = WWAudioClass::Get_Instance ()->Peek_2D_Sample (sample_index);
				if (sound_obj != NULL) {

					if (sound_obj->Get_Type () == AudibleSoundClass::TYPE_MUSIC) {
						message += "(M)";
					} else if (sound_obj->Get_Type () == AudibleSoundClass::TYPE_SOUND_EFFECT) {
						message += "(S)";
					} else if (sound_obj->Get_Type () == AudibleSoundClass::TYPE_DIALOG) {
						message += "(D)";
					}

					StringClass filename = sound_obj->Get_Filename ();
					message += filename;
					message += "\n";

					Vector3 curr_pos = sound_obj->Get_Position ();
					PhysicsSceneClass::Get_Instance ()->Add_Debug_AABox (AABoxClass (curr_pos, Vector3 (0.25F,0.25F,0.25F)), Vector3 (1, 0, 0));

				} else {
					message += "  --\n";
				}
			}

			message += "\n3D Sounds:\n";

			//
			//	Add all the 2D sounds to the message
			//
			for (sample_index = 0; sample_index < count_3d; sample_index ++) {
				temp_string.Format (" %d.", sample_index + 1);
				message += temp_string;

				AudibleSoundClass *sound_obj = WWAudioClass::Get_Instance ()->Peek_3D_Sample (sample_index);
				if (sound_obj != NULL) {

					if (sound_obj->Get_Type () == AudibleSoundClass::TYPE_MUSIC) {
						message += "(M)";
					} else if (sound_obj->Get_Type () == AudibleSoundClass::TYPE_SOUND_EFFECT) {
						message += "(S)";
					} else if (sound_obj->Get_Type () == AudibleSoundClass::TYPE_DIALOG) {
						message += "(D)";
					}

					StringClass filename = sound_obj->Get_Filename ();
					message += filename;
					message += "\n";

					Vector3 curr_pos = sound_obj->Get_Position ();
					PhysicsSceneClass::Get_Instance ()->Add_Debug_AABox (AABoxClass (curr_pos, Vector3 (0.25F,0.25F,0.25F)), Vector3 (0, 0, 1));

				} else {
					message += "  --\n";
				}
			}

			StatisticsDisplayManager::Set_Stat( "audio", message, 0xffffffff );
		}

		/****************************************************************************************
		**
		** Mesh rendering debug display page.  Activate with 'stats mesh'
		**
		****************************************************************************************/
		if (StatisticsDisplayManager::Is_Current_Display("mesh")) {
			StringClass message;
			DX8RendererDebugger::Enable(true);	// The first frame will not have any information...
			DX8RendererDebugger::Get_String(message);

			StatisticsDisplayManager::Set_Stat( "star", message );
			Vector2 pos = Render2DClass::Get_Screen_Resolution().Upper_Left();
			StatisticsDisplayManager::Set_Stat( "mesh", message, 0xffffffff, pos );
		}

		/****************************************************************************************
		**
		** Star Stats Page.  Activate with 'stats star'
		**
		****************************************************************************************/
		if (StatisticsDisplayManager::Is_Current_Display("star")) {
			StringClass working_string(true);
			StringClass message(true);
			working_string.Format("Star (%1.1f, %1.1f, %1.1f) %1.1f\n",
				PlayerPosition.X, PlayerPosition.Y, PlayerPosition.Z, RAD_TO_DEGF(PlayerFacing) );
			message = working_string;

			if (COMBAT_STAR) {
				Phys3Class * pobj = COMBAT_STAR->Peek_Physical_Object()->As_Phys3Class();
				if (pobj) {
					Vector3 vel;
					pobj->Get_Velocity(&vel);
					working_string.Format("Vel (%1.1f, %1.1f, %1.1f)\n",vel.X,vel.Y,vel.Z);
					message += working_string;
				}
			}

			if ( COMBAT_CAMERA ) {
				working_string.Format("Range %1.1f\n", COMBAT_CAMERA->Get_Sniper_Distance() );
				message += working_string;
			}

			StatisticsDisplayManager::Set_Stat( "star", message );
		}


		/****************************************************************************************
		**
		** Collision Detection Stats Page.  Activate with 'stats collision'
		**
		****************************************************************************************/
		if (StatisticsDisplayManager::Is_Current_Display("collision")) {
			StringClass working_string(true);
			StringClass message(true);
			const CollisionMath::ColmathStatsStruct & stats = CollisionMath::Get_Current_Stats();

			working_string.Format("Collision Stats\n");
			message = working_string;

			working_string.Format("Tests: %d\n",stats.TotalCollisionCount);
			message += working_string;
			working_string.Format("Hits: %d\n",stats.TotalCollisionHitCount);
			message += working_string;

			working_string.Format("Ray-Tri Tests: %d\n",stats.CollisionRayTriCount);
			message += working_string;
			working_string.Format("Ray-Tri Hits: %d\n",stats.CollisionRayTriHitCount);
			message += working_string;

			working_string.Format("AAB-Tri Tests: %d\n",stats.CollisionAABoxTriCount);
			message += working_string;
			working_string.Format("AAB-Tri Hits: %d\n",stats.CollisionAABoxTriHitCount);
			message += working_string;

			working_string.Format("OBB-Tri Tests: %d\n",stats.CollisionOBBoxTriCount);
			message += working_string;
			working_string.Format("OBB-Tri Hits: %d\n",stats.CollisionOBBoxTriHitCount);
			message += working_string;

			StatisticsDisplayManager::Set_Stat( "collision", message );
		}
		CollisionMath::Reset_Stats();


		/****************************************************************************************
		**
		** Culling Stats Page.  Activate with 'stats culling'
		**
		****************************************************************************************/
		if (StatisticsDisplayManager::Is_Current_Display("culling")) {
			StringClass working_string(true);
			StringClass message(true);

			PhysicsSceneClass * the_scene = PhysicsSceneClass::Get_Instance();

			working_string.Format("Culling Stats\n");
			message = working_string;

			if (the_scene != NULL) {
				const PhysicsSceneClass::StatsStruct & stats = the_scene->Get_Statistics();
				if (stats.FrameCount > 0) {
					working_string.Format("Frames: %d\n",stats.FrameCount);
					message += working_string;
					working_string.Format("Node Count: %d\n",stats.CullNodeCount);
					message += working_string;
					working_string.Format("Accepted: %d  %10.3f pf\n",stats.CullNodesAccepted,(float)stats.CullNodesAccepted / (float)stats.FrameCount);
					message += working_string;
					working_string.Format("Triv Accepted: %d  %10.3f pf\n",stats.CullNodesTriviallyAccepted,(float)stats.CullNodesTriviallyAccepted / (float)stats.FrameCount);
					message += working_string;
					working_string.Format("Rejected: %d  %10.3f pf\n",stats.CullNodesRejected,(float)stats.CullNodesRejected / (float)stats.FrameCount);
					message += working_string;
				}
			}
			StatisticsDisplayManager::Set_Stat( "culling", message );
		}

		/****************************************************************************************
		**
		** Physics Stats Page.  Activate with 'stats physics'
		**
		****************************************************************************************/
		if (StatisticsDisplayManager::Is_Current_Display("physics")) {
			StringClass working_string(true);
			StringClass message(true);
			PhysicsSceneClass * the_scene = PhysicsSceneClass::Get_Instance();

			working_string.Format("Physics Stats\n");
			message = working_string;

			if (the_scene != NULL) {
				int phys3_count = 0;
				int phys3_active_count = 0;
				int human_count = 0;

				int rbody_count = 0;
				int rbody_active_count = 0;
				int wheeled_count = 0;
				int tracked_count = 0;
				int vtol_count = 0;

				int projectile_count = 0;
				int decoration_count = 0;
				int light_count = 0;
				int rendobj_count = 0;

				int other_count = 0;

				RefPhysListIterator iterator = the_scene->Get_Dynamic_Object_Iterator();
				for (iterator.First(); !iterator.Is_Done(); iterator.Next()) {
					PhysClass * obj = iterator.Peek_Obj();
					WWASSERT(obj != NULL);

					if (obj->As_Phys3Class()) {
						phys3_count++;
						if (!obj->Is_Asleep()) {
							phys3_active_count++;
						}
						if (obj->As_HumanPhysClass()) {
							human_count++;
						}
					} else if (obj->As_RigidBodyClass()) {
						rbody_count++;

						bool is_active = true;
						if (obj->Is_Asleep()) {
							is_active = false;
						}
						if ((obj->As_VehiclePhysClass()) && (obj->As_VehiclePhysClass()->Get_VehiclePhysDef()->Is_Fake() == true)) {
							is_active = false;
						}

						if (is_active) {
							rbody_active_count++;
						}

						if (obj->As_WheeledVehicleClass()) {
							wheeled_count++;
						} else if (obj->As_TrackedVehicleClass()) {
							tracked_count++;
						} else if (obj->As_VTOLVehicleClass()) {
							vtol_count++;
						}
					} else if (obj->As_LightPhysClass()) {
						light_count++;
					} else if (obj->As_DecorationPhysClass()) {
						decoration_count++;
					} else if (obj->As_ProjectileClass()) {
						projectile_count++;
					} else if (obj->As_RenderObjPhysClass()) {
						rendobj_count++;
					} else {
						other_count++;
					}
				}

				working_string.Format("Phys3 Objects: %d (active: %d)\n",phys3_count,phys3_active_count);
				message += working_string;
				working_string.Format("  Human Objects: %d\n\n",human_count);
				message += working_string;

				working_string.Format("RBody Objects: %d (active: %d)\n",rbody_count,rbody_active_count);
				message += working_string;
				working_string.Format("  WheeledVehicle Objects: %d\n",wheeled_count);
				message += working_string;
				working_string.Format("  TrackedVehicle Objects: %d\n",tracked_count);
				message += working_string;
				working_string.Format("  VTOLVehicle Objects: %d\n\n",vtol_count);
				message += working_string;

				working_string.Format("Static Lights: %d\n",light_count);
				message += working_string;
				working_string.Format("Decoration Objects: %d\n",decoration_count);
				message += working_string;
				working_string.Format("Projectile Objects: %d\n",projectile_count);
				message += working_string;
				working_string.Format("Render Object Wrappers: %d\n",rendobj_count);
				message += working_string;
				working_string.Format("Other Objects: %d\n",other_count);
				message += working_string;

				int static_obj_count = 0;
				int static_anim_count = 0;

				iterator = the_scene->Get_Static_Object_Iterator();
				for (iterator.First(); !iterator.Is_Done(); iterator.Next()) {
					PhysClass * obj = iterator.Peek_Obj();
					WWASSERT(obj != NULL);
					if (obj->As_StaticPhysClass()) static_obj_count++;
					if (obj->As_StaticAnimPhysClass()) static_anim_count++;
				}
				working_string.Format("Static objects: %d\n",static_obj_count);
				message += working_string;
				working_string.Format("  Static Anim Objects: %d\n",static_anim_count);
				message += working_string;

				StatisticsDisplayManager::Set_Stat("physics", message);

			}
		}

		/****************************************************************************************
		**
		** Vehicle Debug Stats Page.  Activate with 'stats vehicle'
		** This will dump stats about any vehicle the player is currently controlling.
		**
		****************************************************************************************/
		if (StatisticsDisplayManager::Is_Current_Display("vehicle")) {
			StringClass working_string(true);
			StringClass message(true);
			working_string.Format("Vehicle Debug Stats\n");
			message = working_string;

			if ((COMBAT_STAR != NULL) && (((PhysicalGameObj *) COMBAT_STAR)->As_SoldierGameObj() != NULL)) {
				VehicleGameObj	* vehicle_game_obj = ((PhysicalGameObj *)COMBAT_STAR)->As_SoldierGameObj()->Get_Vehicle();

				if ((vehicle_game_obj != NULL) && (vehicle_game_obj->Peek_Physical_Object() != NULL)) {

					VehiclePhysClass * vehicle = vehicle_game_obj->Peek_Physical_Object()->As_VehiclePhysClass();
					if (vehicle != NULL) {
						Vector3 vel;
						vehicle->Get_Velocity(&vel);
						float meters_per_sec = vel.Length();
						float miles_per_hour = meters_per_sec * 60.0f * 60.0f * (1/1609.0f);

						working_string.Format("Current Velocity: %10.2f m/s (%10.2f mph)\r\n",meters_per_sec,miles_per_hour);
						message += working_string;

						WheeledVehicleClass * wv = vehicle->As_WheeledVehicleClass();
						if (wv != NULL) {

							working_string.Format("Current Gear: %d\n", wv->Get_Current_Gear());
							message += working_string;
							working_string.Format("Engine avel: %10.3f\n", wv->Get_Engine_Angular_Velocity());
							message += working_string;
							working_string.Format("Engine rpm: %10.3f\n", RADS_TO_RPM(wv->Get_Engine_Angular_Velocity()));
							message += working_string;
							working_string.Format("Axle rpm: %10.3f\n", RADS_TO_RPM(wv->Get_Axle_Angular_Velocity()));
							message += working_string;
							working_string.Format("Engine torque: %10.3f\n", wv->Get_Engine_Torque());
							message += working_string;
							working_string.Format("Axle torque: %10.3f\n", wv->Get_Axle_Torque());
							message += working_string;
							working_string.Format("Max Engine torque: %10.3f\n", wv->Get_Max_Engine_Torque());
							message += working_string;

						}

						TrackedVehicleClass * tv = vehicle->As_TrackedVehicleClass();
						if (tv != NULL) {
							const TrackedVehicleDefClass * def = tv->Get_TrackedVehicleDef();
							if (def != NULL) {
								working_string.Format("Max Engine Torque: %10.3f\n", def->Get_Max_Engine_Torque());
								message += working_string;
								working_string.Format("Turn Torque Scale Factor: %10.3f\n", def->Get_Turn_Torque_Scale_Factor());
								message += working_string;
							}
						}
					}
				}
			}
			StatisticsDisplayManager::Set_Stat("vehicle", message, 0xffffffff );
		}

		/****************************************************************************************
		**
		** Collision Detection Stats Page.  Activate with 'stats collision'
		**
		****************************************************************************************/
		if (StatisticsDisplayManager::Is_Current_Display("ai")) {
			StringClass working_string(true);
			StringClass message(true);

			working_string.Format("AI Stats\n");
			message = working_string;

			extern int _ActionActCalls;
			working_string.Format("%d Action Act Calls\n",_ActionActCalls);
			message += working_string;
			_ActionActCalls = 0;

			extern int _ActionCodeChanges;
			working_string.Format("%d Action Code Changes\n", _ActionCodeChanges);
			message += working_string;
			_ActionCodeChanges = 0;

			extern int _AwakeSoldiers;
			working_string.Format("%d Awake Soldiers\n", _AwakeSoldiers);
			message += working_string;
			_AwakeSoldiers = 0;

			extern int _HibernatingSoldiers;
			working_string.Format("%d Hibernating Soldiers\n", _HibernatingSoldiers);
			message += working_string;
			_HibernatingSoldiers = 0;

			SLNode<BaseGameObj> *objnode;
			for (	objnode = GameObjManager::Get_Game_Obj_List()->Head(); objnode; objnode = objnode->Next()) {
				if ( !objnode->Data()->Is_Hibernating() ) {
					if ( objnode->Data()->As_SmartGameObj() ) {
						if ( objnode->Data()->As_SmartGameObj()->As_SoldierGameObj() ) {
							Vector3 pos;
							objnode->Data()->As_SmartGameObj()->Get_Position( &pos );
							pos -= COMBAT_CAMERA->Get_Transform().Get_Translation();
							float distance = pos.Length();
							working_string.Format("ID %d awake (%1.1fm)\n", objnode->Data()->Get_ID(), distance );
							message += working_string;
						}
					}

				}
			}

			StatisticsDisplayManager::Set_Stat( "ai", message );
		}

#ifdef WWDEBUG
   	/*
		** WOL location diagnostic
		*/
#if(0) // obsolete
		if (StatisticsDisplayManager::Is_Current_Display("wol")) {
			char string[200];
      	if (GameModeManager::Find("WOL")->Is_Active()) {
         	WWASSERT(PWC != NULL);
	      	//sprintf(string, "WOL location: %s", PWC->Translate_Location());
	      	sprintf(string, "WOL location: %s", Translate_Location(PWC->Get_Current_Location()));
      	} else {
	      	sprintf(string, "WOL is not active.");
      	}

			StatisticsDisplayManager::Set_Stat( "wol", string );
		}
#endif // obsolete
#endif // WWDEBUG
	}

   if ( Input::Get_State( INPUT_FUNCTION_CNC )) {
		ConsoleFunctionManager::Parse_Input( "CNC" );
	}

	// Note:  As you add more stats, also add to the stats help command.

	Update_Profile();
	Update_Memory_Log();
}

/*
**
*/
void 	ConsoleGameModeClass::Parse_Input( char * string )
{
   WWASSERT(Get_Console() != NULL);
	if ( ConsoleInputType == INPUT_FUNCTION_BEGIN_PUBLIC_MESSAGE ||
        ConsoleInputType == INPUT_FUNCTION_BEGIN_TEAM_MESSAGE   ||
        ConsoleInputType == INPUT_FUNCTION_BEGIN_PRIVATE_MESSAGE ) {

      if (GameModeManager::Find("Combat")->Is_Active()) {
			/*
			if (cNetwork::I_Am_Client()) {
				cNetwork::Send_Client_Text_Message(string, ConsoleInputType);
			} else {

            WWASSERT(ConsoleInputType == INPUT_FUNCTION_BEGIN_PUBLIC_MESSAGE);

				WideStringClass text;
				text.Convert_From(string);
				if (!text.Is_Empty()) {
					cScTextObj * p_test_obj = new cScTextObj;
					p_test_obj->Init(text, TEXT_MESSAGE_PUBLIC, HOST_TEXT_SENDER);
				}
			}
			*/
      } else if (GameModeManager::Find("WOL")->Is_Active()) {
				RefPtr<WWOnline::Session> wolSession = WWOnline::Session::GetInstance(false);

				if (wolSession.IsValid()) {
					wolSession->SendPublicMessage(string);
				}
      }
	} else {
		ConsoleFunctionManager::Parse_Input( string );
	}
}

void ConsoleGameModeClass::Clear_Suggestion(void)
{
	memset(Suggestion,0,sizeof(Suggestion));
	memset(HelpLine,0,sizeof(HelpLine));
}

void ConsoleGameModeClass::Accept_Suggestion(char * cmd)
{
	if (ConsoleInputType == INPUT_FUNCTION_BEGIN_CONSOLE) {
		// If a space has already been entered or there is no suggestion, do nothing
		if ((strchr(cmd,' ') == NULL) && (strlen(Suggestion) > 0)) {
			strcpy(cmd,Suggestion);
		}
	}
}

void ConsoleGameModeClass::Update_Suggestion(char * cmd,bool go_to_next)
{
	if (ConsoleInputType == INPUT_FUNCTION_BEGIN_CONSOLE) {

		// If a space has already been entered don't update so that the help stays up
		if ((strlen(cmd) > 0) && (strchr(cmd,' ') == NULL)) {
			char * cur_suggestion = NULL;
			if ((go_to_next) && (strlen(Suggestion) > 0)) {
				cur_suggestion = &(Suggestion[0]);
			}

			bool gotone = ConsoleFunctionManager::Get_Command_Suggestion(cmd,cur_suggestion,Suggestion,HelpLine,sizeof(Suggestion));
			if (!gotone) {
				Clear_Suggestion();
			}
		}

	}
}


void	ConsoleGameModeClass::Profile_Command( const char * command )
{
	if ( ProfileIterator != NULL ) {
		if ( stricmp( command, "log" ) == 0 ) {
			if (profile_log_active) End_Profile_Log();
			else Begin_Profile_Log();
		} else if ( stricmp( command, "off" ) == 0 ) {
			WWProfileManager::Release_Iterator( ProfileIterator );
			ProfileIterator = NULL;
		} else	if ( stricmp( command, "reset" ) == 0 ) {
			WWProfileManager::Reset();
		} else	if ( stricmp( command, "up" ) == 0 ) {
			ProfileIterator->Enter_Parent();
		} else {
			int index = atoi( command );
			if ( index > -1 ) {
				ProfileIterator->Enter_Child( index );
			}
		}
	}

	if ( ProfileIterator == NULL ) {
		if ( stricmp( command, "on" ) == 0 ) {
			ProfileIterator = WWProfileManager::Get_Iterator();
		}
	}
}

StringClass	profile_string;
StringClass working_string;

void	ConsoleGameModeClass::Update_Profile( void )
{
	WWPROFILE( "Update Profile" );

#ifdef ATI_DEMO_HACK
// HACK
	if (Input::Get_State(INPUT_FUNCTION_PROFILE_ENTER_CHILD0)) WW3D::Set_NPatches_Level(0);
	if (Input::Get_State(INPUT_FUNCTION_PROFILE_ENTER_CHILD1)) WW3D::Set_NPatches_Level(1);
	if (Input::Get_State(INPUT_FUNCTION_PROFILE_ENTER_CHILD2)) WW3D::Set_NPatches_Level(2);
	if (Input::Get_State(INPUT_FUNCTION_PROFILE_ENTER_CHILD3)) WW3D::Set_NPatches_Level(3);
	if (Input::Get_State(INPUT_FUNCTION_PROFILE_ENTER_CHILD4)) WW3D::Set_NPatches_Level(4);
	if (Input::Get_State(INPUT_FUNCTION_PROFILE_ENTER_CHILD5)) WW3D::Set_NPatches_Level(5);
	if (Input::Get_State(INPUT_FUNCTION_PROFILE_ENTER_CHILD6)) WW3D::Set_NPatches_Level(6);
	if (Input::Get_State(INPUT_FUNCTION_PROFILE_ENTER_CHILD7)) WW3D::Set_NPatches_Level(7);
	if (Input::Get_State(INPUT_FUNCTION_PROFILE_ENTER_CHILD8)) WW3D::Set_NPatches_Level(8);
	if (Input::Get_State(INPUT_FUNCTION_PROFILE_ENTER_PARENT)) {
		if (COMBAT_SCENE->Get_Polygon_Mode()==SceneClass::LINE) {
			COMBAT_SCENE->Set_Polygon_Mode(SceneClass::FILL);
		}
		else {
			COMBAT_SCENE->Set_Polygon_Mode(SceneClass::LINE);
		}
	}
	if (Input::Get_State(INPUT_FUNCTION_PROFILE_ENTER_CHILD9)) {
		unsigned i=(unsigned)WW3D::Get_NPatches_Gap_Filling_Mode();
		i&=1;
		i^=1;
		WW3D::Set_NPatches_Gap_Filling_Mode( (WW3D::NPatchesGapFillingModeEnum) i);
	}
#endif

	if (!StatisticsDisplayManager::Is_Current_Display("profile")) return;

	if (profile_log_active) {
		Process_Profile_Log();
		Vector2 pos = (Render2DClass::Get_Screen_Resolution().Upper_Left() + Render2DClass::Get_Screen_Resolution().Lower_Left()) * 0.5f;
		StatisticsDisplayManager::Set_Stat( "profile", "LOG IN PROGRESS", 0xffffffff, pos );
		return;
	}

	// It costs to allocate these, lets just skip strings that grow.
//	StringClass	profile_string( 2000, true );
//	StringClass working_string( 200, true );

	if ( ProfileIterator ) {


		/*
		** Handle user input.  When the profiler is active, the numeric keypad can be used to
		** traverse the profile tree.
		** '1'-'9'    enter profile node 1-9
		** '.'        go to parent node
		** '*'        reset the profile data
		*/
#ifndef ATI_DEMO_HACK
		if (Input::Get_State(INPUT_FUNCTION_PROFILE_ENTER_CHILD0)) { Profile_Command("0"); }
		if (Input::Get_State(INPUT_FUNCTION_PROFILE_ENTER_CHILD1)) { Profile_Command("1"); }
		if (Input::Get_State(INPUT_FUNCTION_PROFILE_ENTER_CHILD2)) { Profile_Command("2"); }
		if (Input::Get_State(INPUT_FUNCTION_PROFILE_ENTER_CHILD3)) { Profile_Command("3"); }
		if (Input::Get_State(INPUT_FUNCTION_PROFILE_ENTER_CHILD4)) { Profile_Command("4"); }
		if (Input::Get_State(INPUT_FUNCTION_PROFILE_ENTER_CHILD5)) { Profile_Command("5"); }
		if (Input::Get_State(INPUT_FUNCTION_PROFILE_ENTER_CHILD6)) { Profile_Command("6"); }
		if (Input::Get_State(INPUT_FUNCTION_PROFILE_ENTER_CHILD7)) { Profile_Command("7"); }
		if (Input::Get_State(INPUT_FUNCTION_PROFILE_ENTER_CHILD8)) { Profile_Command("8"); }
		if (Input::Get_State(INPUT_FUNCTION_PROFILE_ENTER_CHILD9)) { Profile_Command("9"); }
		if (Input::Get_State(INPUT_FUNCTION_PROFILE_ENTER_PARENT)) { Profile_Command("up"); }
		if (Input::Get_State(INPUT_FUNCTION_PROFILE_RESET)) { Profile_Command("reset"); }
#endif

		// Only update every 1/4 second
		static float timer = 0;
		timer += TimeManager::Get_Frame_Seconds();
		if ( timer < 0.25f ) {
			return;
		}
		timer = 0;

		profile_string = "";
		working_string = "";

		/*
		** Update the profile display
		*/
		const char * parent_name = ProfileIterator->Get_Current_Parent_Name();
		float parent_time = ProfileIterator->Get_Current_Parent_Total_Time();

		profile_string.Format( "PROFILE DATA for %s\n\n", parent_name );
		working_string.Format(
			"   Name                      %%parent  %%total  ms/f   ms/call   calls/f\n\n");
		profile_string += working_string;

		int index = 0;
		float total_time = WWProfileManager::Get_Time_Since_Reset();
		int total_frames = WWProfileManager::Get_Frame_Count_Since_Reset();

		float	missing_parent_time = parent_time;

		if ((total_frames > 0) && (total_time > 0.0f)) {

			float percent_of_parent;
			float percent_of_total;
			float ms_per_frame;
			float ms_per_call;
			int calls_per_frame;

			/*
			** Display stats for each child node
			*/
			for ( ProfileIterator->First(); !ProfileIterator->Is_Done(); ProfileIterator->Next() ) {

				int calls = ProfileIterator->Get_Current_Total_Calls();
				float time = ProfileIterator->Get_Current_Total_Time();
				const char * name = ProfileIterator->Get_Current_Name();

				missing_parent_time -= time;

				/*
				** First print the index and name of the profile entry
				*/
				working_string.Format("%-2d %-25s",index,name);
				profile_string += working_string;

				/*
				** Print the percent of the parent and percent of total time.
				** If there is no parent time, we are at the wrapper of everything and
				** we simply print that it is 100% of the time and
				*/
				percent_of_total = 100.0f * time / total_time;
				if (parent_time != 0.0f) {
					percent_of_parent = 100.0f * time / parent_time;
					working_string.Format(" %-6.2f   %-6.2f",percent_of_parent,percent_of_total);
				} else {
					percent_of_parent = 100.0f;
					working_string.Format("   --     %-6.2f",percent_of_total);
				}
				profile_string += working_string;

				/*
				** Print the ms_per_frame
				*/
				ms_per_frame = 1000.0f * time / total_frames;
				working_string.Format("  %-6.2f",ms_per_frame);
				profile_string += working_string;

				/*
				** Print the ms_per_call and number of calls or '--' if not available
				*/
				if (calls > 0) {
					ms_per_call = 1000.0f * time / (float)calls;
					calls_per_frame = calls / total_frames;
					working_string.Format(" %-6.2f    %-4d (%4d)\n",ms_per_call, calls_per_frame, calls);
				} else {
					working_string.Format("   --       -- \n");
				}
				profile_string += working_string;

				/*
				** Increment the index
				*/
				index++;
			}

			/*
			** Display stats for the un-accounted time.  Note that there will only be
			** "missing_parent_time" if there also was "parent_time"...
			*/
			if (parent_time > 0.0f) {
				percent_of_parent = 100.0f * missing_parent_time / parent_time;
				percent_of_total = 100.0f * missing_parent_time / total_time;
				ms_per_frame = 1000.0f * missing_parent_time / total_frames;

				working_string.Format(
						"   %-25s %-6.2f   %-6.2f  %-6.2f\n",
						"MISSING TIME",
						percent_of_parent,
						percent_of_total,
						ms_per_frame );
				profile_string += working_string;
			}
		}

		if (ConsoleBox.Is_Exclusive()) {
			ConsoleBox.Update_Profile(profile_string);
		} else {
			Vector2 pos = (Render2DClass::Get_Screen_Resolution().Upper_Left() + Render2DClass::Get_Screen_Resolution().Lower_Left()) * 0.5f;
			StatisticsDisplayManager::Set_Stat( "profile", profile_string, 0xffffffff, pos );
		}
	}
}

class ProfileLogNodeClass
{
	StringClass string;
	ProfileLogNodeClass* succ;
	float* percentages;
	unsigned count;
public:
	ProfileLogNodeClass(unsigned count);
	~ProfileLogNodeClass();

	void Set(unsigned index,float value) { WWASSERT(index<count); percentages[index]=value; }
	float Get(unsigned index) const { WWASSERT(index<count); return percentages[index]; }
	void Set_String(const StringClass& string_) { string=string_; }
	const StringClass& Get_String() const { return string; }
	unsigned Get_Count() const { return count; }

	ProfileLogNodeClass* Succ() { return succ; }
};

static ProfileLogNodeClass* profile_log_head;
static ProfileLogNodeClass* profile_log_tail;

ProfileLogNodeClass::ProfileLogNodeClass(unsigned count_)
	:
	count(count_),
	succ(NULL)
{
	if (!profile_log_head) {
		profile_log_head=this;
		profile_log_tail=this;
	}
	else {
		profile_log_tail->succ=this;
		profile_log_tail=this;
	}

	percentages=new float[count];
}

ProfileLogNodeClass::~ProfileLogNodeClass()
{
	delete[] percentages;
	if (profile_log_head==this) profile_log_head=succ;
	if (profile_log_tail==this) profile_log_tail=NULL;
}

void	ConsoleGameModeClass::Begin_Profile_Log()
{
	if (profile_log_active) return;
	profile_log_active=true;
}

void	ConsoleGameModeClass::End_Profile_Log()
{
	if (!profile_log_active) return;
	profile_log_active=false;

	ProfileLogNodeClass* node=profile_log_head;
	if (node && profile_log_names) {
		for (unsigned index=0;index<node->Get_Count();++index) {
			char tmp[8];
			strncpy(tmp,profile_log_names[index],sizeof(tmp));
			tmp[7]='\0';
			WWDEBUG_SAY(("%7s ",tmp));
		}
		WWDEBUG_SAY(("\n"));
	}

	while (node) {
		for (unsigned index=0;index<node->Get_Count();++index) {
			WWDEBUG_SAY(("%2.2f	",node->Get(index)));
		}
		WWDEBUG_SAY(("\n"));
		node=node->Succ();
	}

	WWDEBUG_SAY(("\n\n"));
	node=profile_log_head;
	while (node) {
		WWDEBUG_SAY(("%s\n",node->Get_String()));
		node=node->Succ();
	}

	while (profile_log_head) {
		delete profile_log_head;
	}

	delete[] profile_log_names;
	profile_log_names=NULL;
}

void	ConsoleGameModeClass::Process_Profile_Log()
{
	if ( ProfileIterator ) {

		// Only update every 1/4 second
		static float timer = 0;
		timer += TimeManager::Get_Frame_Seconds();
		if ( timer < 0.25f ) {
			return;
		}
		timer = 0;

		profile_string = "";
		working_string = "";

		/*
		** Update the profile display
		*/
//		const char * parent_name = ProfileIterator->Get_Current_Parent_Name();
//		float parent_time = ProfileIterator->Get_Current_Parent_Total_Time();
//
		float total_time = WWProfileManager::Get_Time_Since_Reset();
		int total_frames = WWProfileManager::Get_Frame_Count_Since_Reset();
//
//		float	missing_parent_time = parent_time;

		if ((total_frames > 0) && (total_time > 0.0f)) {

//			float percent_of_parent;
			float percent_of_total;
//			float ms_per_frame;
//			float ms_per_call;
//			int calls_per_frame;

			unsigned index=0;
			for ( ProfileIterator->First(); !ProfileIterator->Is_Done(); ProfileIterator->Next(),index++ ) {
			}

			delete[] profile_log_names;
			profile_log_names=NULL;
			if (index) {
				profile_log_names=new StringClass[index];
			}

			ProfileLogNodeClass* node=new ProfileLogNodeClass(index);

			/*
			** Display stats for each child node
			*/
			index=0;
			for ( ProfileIterator->First(); !ProfileIterator->Is_Done(); ProfileIterator->Next() ) {

//				int calls = ProfileIterator->Get_Current_Total_Calls();
				float time = ProfileIterator->Get_Current_Total_Time();
				const char * name = ProfileIterator->Get_Current_Name();
				profile_log_names[index]=name;

//				missing_parent_time -= time;

				/*
				** First print the index and name of the profile entry
				*/
//				working_string.Format("%-2d %-25s",index,name);
//				profile_string += working_string;

				/*
				** Print the percent of the parent and percent of total time.
				** If there is no parent time, we are at the wrapper of everything and
				** we simply print that it is 100% of the time and
				*/
				percent_of_total = 100.0f * time / total_time;
//				if (parent_time != 0.0f) {
//					percent_of_parent = 100.0f * time / parent_time;
//					working_string.Format(" %-6.2f   %-6.2f",percent_of_parent,percent_of_total);
//				} else {
//					percent_of_parent = 100.0f;
//					working_string.Format("   --     %-6.2f",percent_of_total);
//				}
//				profile_string += working_string;

				node->Set(index,percent_of_total);

				/*
				** Print the ms_per_frame
				*/
//				ms_per_frame = 1000.0f * time / total_frames;
//				working_string.Format("  %-6.2f",ms_per_frame);
//				profile_string += working_string;

				/*
				** Print the ms_per_call and number of calls or '--' if not available
				*/
//				if (calls > 0) {
//					ms_per_call = 1000.0f * time / (float)calls;
//					calls_per_frame = calls / total_frames;
//					working_string.Format(" %-6.2f    %-4d (%4d)\n",ms_per_call, calls_per_frame, calls);
//				} else {
//					working_string.Format("   --       -- \n");
//				}
//				profile_string += working_string;

				/*
				** Increment the index
				*/
				index++;
			}

			/*
			** Display stats for the un-accounted time.  Note that there will only be
			** "missing_parent_time" if there also was "parent_time"...
			*/
/*			if (parent_time > 0.0f) {
				percent_of_parent = 100.0f * missing_parent_time / parent_time;
				percent_of_total = 100.0f * missing_parent_time / total_time;
				ms_per_frame = 1000.0f * missing_parent_time / total_frames;

				working_string.Format(
						"   %-25s %-6.2f   %-6.2f  %-6.2f\n",
						"MISSING TIME",
						percent_of_parent,
						percent_of_total,
						ms_per_frame );
				profile_string += working_string;
			}
*/

			node->Set_String(profile_string);

//		Vector2 pos = (Render2DClass::Get_Screen_Resolution().Upper_Left() + Render2DClass::Get_Screen_Resolution().Lower_Left()) * 0.5f;
//		StatisticsDisplayManager::Set_Stat( "profile", profile_string, 0xffffffff, pos );

		}
	}
}

void	ConsoleGameModeClass::Update_Memory_Log( void )
{
	if (!StatisticsDisplayManager::Is_Current_Display("memory")) return;

	const float MEGABYTE = 1048576.0f;
	const float OOMEGABYTE = 1.0f / MEGABYTE;

	StringClass	memory_string(2048);
	StringClass temp_string(true);

	memory_string.Format("Memory Category     Current(Mb)    Peak(Mb)\n");
	int total = 0;
	for (int i=0; i<WWMemoryLogClass::Get_Category_Count(); i++) {

		// (gth) to compute Mb should I divide by the nearest power of two to a million?
		temp_string.Format("%-18s  %-10.2f     %-10.2f\r\n",
										WWMemoryLogClass::Get_Category_Name(i),
										(float)WWMemoryLogClass::Get_Current_Allocated_Memory(i) * OOMEGABYTE,
										(float)WWMemoryLogClass::Get_Peak_Allocated_Memory(i) * OOMEGABYTE);
		memory_string += temp_string;
		total += WWMemoryLogClass::Get_Current_Allocated_Memory(i);
	}

#if (UMBRASUPPORT)
	float umbra_mem = UmbraSupport::Get_Umbra_Memory_Consumption();
	total += umbra_mem;
	working_string.Format("Umbra:              %-10.2f\r\n",umbra_mem * OOMEGABYTE);
	memory_string += working_string;
#endif

	temp_string.Format("SUB TOTAL:          %-10.2f\r\n\r\n",(float)total * OOMEGABYTE);
	memory_string += temp_string;

	// display the estimated space used by textures residing in system ram
	int tex_size=TextureClass::_Get_Total_Texture_Size();
	temp_string.Format("%-18s  %-10.2f\r\n","Textures(est)",(float)tex_size * OOMEGABYTE);
	memory_string += temp_string;
	total+=tex_size;

	// display estimated vertex buffer space
	unsigned vb_size=VertexBufferClass::Get_Total_Allocated_Memory();
	temp_string.Format("%-18s  %-10.2f\r\n","Vertex Buffers(est)",(float)vb_size * OOMEGABYTE);
	memory_string += temp_string;
	total+=vb_size;

	// display estimated index buffer space
	unsigned ib_size=IndexBufferClass::Get_Total_Allocated_Memory();
	temp_string.Format("%-18s  %-10.2f\r\n","Index Buffers(est)",(float)ib_size * OOMEGABYTE);
	memory_string += temp_string;
	total+=ib_size;

	// total!
	temp_string.Format("TOTAL:              %-10.2f\r\n\r\n",(float)total * OOMEGABYTE);
	memory_string += temp_string;

	StatisticsDisplayManager::Set_Stat( "memory", memory_string, 0xffffffff );
}




















