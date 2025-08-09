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
 *                     $Archive:: /Commando/Code/Commando/consolefunction.cpp                 $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 10/13/02 10:13p                                             $*
 *                                                                                             *
 *                    $Revision:: 339                                                         $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "consolefunction.h"
#include "console.h"
#include "textdisplay.h"
#include "combatgmode.h"
#include "combat.h"
#include "datasafe.h"
#include "pscene.h"
#include "playermanager.h"
#include "ccamera.h"
#include "debug.h"
#include "level.h"
#include "weaponmanager.h"
#include "cnetwork.h"
#include "ww3d.h"
#include "miscutil.h"
#include "smartgameobj.h"
#include "weapons.h"
#include "WWAudio.H"
#include "wwprofile.h"
//#include "gamesettings.h"
#include "waypoint.h"
#include "action.h"
#include "gameobjmanager.h"
#include "surfaceeffects.h"
#include "gamedata.h"
#include "damage.h"
#include "radar.h"
#include "assets.h"
#include "animobj.h"
#include "matpass.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "objlibrary.h"
#include "useroptions.h"
#include "devoptions.h"
#include "god.h"
#include "playertype.h"
#include "systemsettings.h"
#include "spawn.h"
#include "input.h"
#include "pathfind.h"
#include "waypath.h"
#include "definitionclassids.h"
#include "netinterface.h"
#include "bandwidth.h"
#include "statistics.h"
#include "physaabtreecull.h"
#include "gametype.h"
#include "part_emt.h"
#include "translatedb.h"
#include "string_ids.h"
#include "dx8renderer.h"
#include "backgroundmgr.h"
#include "dynamicphys.h"
#include "humanphys.h"
#include "vehiclephys.h"
#include "objectives.h"
#include "umbrasupport.h"
#include "vehicle.h"
#include "damageablestaticphys.h"
#include "persistfactory.h"
#include "wwphysids.h"
#include "hud.h"
#include "conversationmgr.h"
#include "basecontroller.h"
#include "building.h"
#include "vendor.h"
#include "texture.h"
#include "rddesc.h"
#include "combatchunkid.h"
#include "dialogtests.h"
#include "resource.h"
#include "dx8wrapper.h"
#include "sortingrenderer.h"
#include "weathermgr.h"
#include "mapmgr.h"
#include "path.h"
#include "sctextobj.h"
#include "consolecommandevent.h"
#include "hudinfo.h"
#include "physresourcemgr.h"
#include "cstextobj.h"
#include "suicideevent.h"
#include "godmodeevent.h"
#include "serverfps.h"
#include "warpevent.h"
//#include "helptext.h"
#include "ffactory.h"
#include "dazzle.h"
#include "renegadedialogmgr.h"
#include "moneyevent.h"
#include "scoreevent.h"
#include "dlgmpingamechat.h"
#include "playerterminal.h"
#include "encyclopediamgr.h"
#include "clientbboevent.h"
#include "wheelvehicle.h"
#include "trackedvehicle.h"
#include "woldiags.h"
#include "packetmgr.h"
#include "requestkillevent.h"
#include "csconsolecommandevent.h"
#include "apppacketstats.h"
#include "bandwidthgraph.h"
#include "scexplosionevent.h"
#include "bwbalance.h"
#include "sbbomanager.h"
#include "_globals.h"
#include "registry.h"
#include "vipmodeevent.h"
#include "dx8rendererdebugger.h"
#include "changeteamevent.h"
#include "gamesideservercontrol.h"
#include "autostart.h"
#include "except.h"
#include "slavemaster.h"
#include "donateevent.h"
#include "consolemode.h"
#include "dlgcncpurchasemainmenu.h"
#include "realcrc.h"
#include "gamespyadmin.h"
#include "gamespybanlist.h"
#include "specialbuilds.h"
#include "lightsolve.h"
#include "lightsolvecontext.h"



void	ConsoleFunctionClass::Print( const char *format, ... )
{
	va_list arg_list;
	va_start (arg_list, format);
	StringClass string;
	string.Format_Args( format, arg_list );
	if (Get_Text_Display()) {
		WWASSERT( Get_Text_Display() );
		Get_Text_Display()->Print_System( string );
	}
	ConsoleBox.Print(string.Peek_Buffer());
	va_end (arg_list);
}

//----------------------------------------------------------------------------
//
// DEVELOPMENT CONSOLE FUNCTIONS ONLY
// Only present in Debug and Profile builds.
//
//----------------------------------------------------------------------------

#ifdef WWDEBUG

/*
** Console Functions
*/
class BuildingStateConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "building_state"; }
	virtual	const char * Get_Alias( void ) { return "bs"; }
	virtual	const char * Get_Help( void )	{ return "BUILDING_STATE <health:0,100> <power:0,1> - force all buildings to the given state"; }
	virtual	void Activate( const char * input ) {

		int health_percentage,power;
		int cnt=sscanf(input, "%d %d", &health_percentage, &power);
		if (cnt == 2) {
			GameObjManager::Debug_Set_All_Building_States((float)health_percentage,(power==1));
			Print("Building States set to Health = %d%%, Power = %d\n",health_percentage,power);
		} else {
			GameObjManager::Debug_Set_All_Building_States(100,1);
			GameObjManager::Debug_Set_All_Building_States(0,1);
			Print("Blew up all buildings!\n");
		}
	}
};

class DSAPOResetConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "dsapo_reset"; }
	virtual	const char * Get_Help( void )	{ return "DSAPO_RESET - force all DSAPO's back to their initial state."; }
	virtual	void Activate( const char * input ) {

		if (COMBAT_SCENE) {
			RefPhysListIterator	it = COMBAT_SCENE->Get_Static_Anim_Object_Iterator();
			for (it.First();!it.Is_Done();it.Next()) {

				if (it.Peek_Obj()->Get_Factory().Chunk_ID() == PHYSICS_CHUNKID_DAMAGEABLESTATICPHYS) {
					DamageableStaticPhysClass * dobj = (DamageableStaticPhysClass *)it.Peek_Obj();
					dobj->Reset_Health();
				}
			}
		}

		int health_percentage,power;
		int cnt=sscanf(input, "%d %d", &health_percentage, &power);
		if (cnt == 2) {
			GameObjManager::Debug_Set_All_Building_States((float)health_percentage,(power==1));
			Print("Building States set to Health = %d%%, Power = %d\n",health_percentage,power);
		} else {
			GameObjManager::Debug_Set_All_Building_States(100,1);
			GameObjManager::Debug_Set_All_Building_States(0,1);
			Print("Blew up all buildings!\n");
		}
	}
};

class HUDConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "hud"; }
	virtual	const char * Get_Help( void )	{ return "HUD - toggles the HUD."; }
	virtual	void Activate( const char * input ) {
		HUDClass::Enable( !HUDClass::Is_Enabled() );
      Print( "HUD toggled\n" );
	}
};


class PatsDebugConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "pat"; }
	virtual	const char * Get_Help( void )	{ return "PAT - Pat's generic debug console function."; }
	virtual	void Activate( const char * input )
	{
		RenegadeDialogMgrClass::Goto_Location (RenegadeDialogMgrClass::LOC_INTERNET_MAIN);
		return ;
	}
};


class TextureMemoryCounterConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "texture_memory_counter"; }
	virtual	const char * Get_Help( void )	{ return "TEXTURE_MEMORY_COUNTER - 0=off 1=on 2=details"; }
	virtual	void Activate( const char * input ) {
		Debug_Statistics::RecordTextureMode b;
		if (sscanf(input, "%d", &b) == 1) {
			Debug_Statistics::Record_Texture_Mode(b);
		}
	}
};


class MeshDebuggerEnableConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "mesh_debugger_enable"; }
	virtual	const char * Get_Help( void )	{ return "MESH_DEBUGGER_ENABLE - 0=off 1=on"; }
	virtual	void Activate( const char * input ) {
		unsigned onoff;
		if (sscanf(input, "%d", &onoff) == 1) {
			DX8RendererDebugger::Enable(!!onoff);
		}
	}
};

class MeshDebuggerEnableMeshConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "mesh_debugger_enable_mesh"; }
	virtual	const char * Get_Help( void )	{ return "MESH_DEBUGGER_ENABLE_MESH - id0 ... id9"; }
	virtual	void Activate( const char * input ) {
		// Parameter '*' means ALL meshes
		char string[200];
		sscanf(input, "%s", string);
		if (!stricmp("*",string)) {
			DX8RendererDebugger::Enable_All();
		}
		else {
			unsigned id[10];
			unsigned count=sscanf(input, "%d %d %d %d %d %d %d %d %d %d",
				&id[0], &id[1], &id[2], &id[3], &id[4], &id[5], &id[6], &id[7], &id[8], &id[9]);
			for (unsigned i=0;i<count;++i) {
				DX8RendererDebugger::Enable_Mesh(id[i]);
			}
		}
	}
};

class MeshDebuggerDisableMeshConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "mesh_debugger_disable_mesh"; }
	virtual	const char * Get_Help( void )	{ return "MESH_DEBUGGER_DISABLE_MESH - id0 ... id9"; }
	virtual	void Activate( const char * input ) {
		// Parameter '*' means ALL meshes
		char string[200];
		sscanf(input, "%s", string);
		if (!stricmp("*",string)) {
			DX8RendererDebugger::Disable_All();
		}
		else {
			unsigned id[10];
			unsigned count=sscanf(input, "%d %d %d %d %d %d %d %d %d %d",
				&id[0], &id[1], &id[2], &id[3], &id[4], &id[5], &id[6], &id[7], &id[8], &id[9]);
			for (unsigned i=0;i<count;++i) {
				DX8RendererDebugger::Disable_Mesh(id[i]);
			}
		}
	}
};


class FlashTextureConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "flash_texture"; }
	virtual	const char * Get_Help( void )	{ return "FLASH_TEXTURE <ID> <0/1> - sets texture flashing"; }
	virtual	void Activate( const char * input ) {
#ifdef WW3D_DX8
		int id,state;
		int cnt=sscanf(input, "%d %d", &id, &state);
		TextureFileClass* tfc=NULL;
		if (cnt>=1) tfc=TextureFileClass::Get_Texture(id);
		if (tfc) {
			if (cnt==1) {
				state=!tfc->Get_Texture_Flash();
				tfc->Set_Texture_Flash(!!state);
			}
			else if (cnt==2) {
				tfc->Set_Texture_Flash(!!state);
			}
			StringClass tmp;
			if (state) {
				tmp.Format("Texture %s (id=%d) set FLASHING\n",tfc->Get_File_Name(),tfc->ID());
			}
			else {
				tmp.Format("Texture %s (id=%d) set not flashing\n",tfc->Get_File_Name(),tfc->ID());
			}
			Print(tmp);
		}
#endif //WW3D_DX8
	}
};


class FogRangeConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "fog_range"; }
	virtual	const char * Get_Help( void )	{ return "FOG_RANGE <start [0+]> <end [0+]> [ramptime [0+]] - Sets fog range.";}
	virtual	void Activate( const char * input ) {

		const char *feedbackstring = "Fog range changed\n";

		int	argcount;
		float	arg0, arg1, arg2;
		bool	success = false;

		argcount = sscanf (input, "%f %f %f", &arg0, &arg1, &arg2);
		switch (argcount) {

			case 2:
				success = WeatherMgrClass::Set_Fog_Range (arg0, arg1);
				break;

			case 3:
				success = WeatherMgrClass::Set_Fog_Range (arg0, arg1, arg2);
				break;

			default:
				// Do nothing.
				break;
		}
		if (success) Print (feedbackstring);
	}
};


class FogToggleConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name (void)	{ return "fog_toggle"; }
	virtual	const char * Get_Help (void)	{ return "FOG_TOGGLE - toggles fog on/off."; }
	virtual	void Activate (const char *input) {

		const char *_statusstring [2] = {"Fog disabled\n", "Fog enabled\n"};

		WeatherMgrClass::Set_Fog_Enable (!WeatherMgrClass::Get_Fog_Enable());
		Print (_statusstring [WeatherMgrClass::Get_Fog_Enable()]);
	}
};


class LightModeConsoleFunctionClass : public ConsoleFunctionClass {
	virtual	const char * Get_Name (void)	{ return "light_mode"; }
	virtual	const char * Get_Help (void)	{ return "LIGHT_MODE <0,1,2> - 0=none, 1=surrender, 2=cheap."; }
	virtual	void Activate (const char *input) {
		int mode;
		if (sscanf(input, "%d", &mode) == 1) {
			COMBAT_SCENE->Set_Lighting_Mode(mode);
		}

		switch(COMBAT_SCENE->Get_Lighting_Mode()) {
		case PhysicsSceneClass::LIGHTING_MODE_SURRENDER:
			Print ("Lighting Mode: SURRENDER");
			break;

		case PhysicsSceneClass::LIGHTING_MODE_CHEAP:
			Print ("Lighting Mode: CHEAP");
			break;

		default:
			Print ("Lighting Mode: NONE");
			break;
		}
	}
};

class LightDebugToggleConsoleFunctionClass : public ConsoleFunctionClass {
	virtual	const char * Get_Name (void)	{ return "light_debug"; }
	virtual	const char * Get_Help (void)	{ return "LIGHT_DEBUG - toggles lighting debugging on/off."; }
	virtual	void Activate (const char *input) {
		COMBAT_SCENE->Enable_Lighting_Debug_Display(!COMBAT_SCENE->Is_Lighting_Debug_Display_Enabled());
		if (COMBAT_SCENE->Is_Lighting_Debug_Display_Enabled()) {
			Print ("Lighting debug display ENABLED");
		} else {
			Print ("Lighting debug display DISABLED");
		}
	}
};

class LightLODConsoleFunctionClass : public ConsoleFunctionClass {
	virtual	const char * Get_Name (void)	{ return "light_lod"; }
	virtual	const char * Get_Help (void)	{ return "LIGHT_LOD <intensity> - sets the LOD cutoff for lighting."; }
	virtual	void Activate (const char *input) {
		float inten;
		if (sscanf(input, "%f", &inten) == 1) {
			COMBAT_SCENE->Set_Lighting_LOD_Cutoff(inten);
		}
		Print("Lighting LOD: %d\r\n",COMBAT_SCENE->Get_Lighting_LOD_Cutoff());
	}
};

class LightCalcConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "light_calc"; }
	virtual	const char * Get_Help( void )	{ return "LIGHT_CALC - computes a static vertex lighting solve"; }
	virtual	void Activate( const char * input ) {
		if (COMBAT_SCENE) {
			LightSolveContextClass context;
			LightSolveClass::Generate_Static_Light_Solve(context);
		}
	}
};



class CameraConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "camera"; }
	virtual	const char * Get_Help( void )	{ return "CAMERA <NAME> - switches to camera profile <name>."; }
	virtual	void Activate( const char * input ) {
		if ( COMBAT_CAMERA ) {
			COMBAT_CAMERA->Use_Profile( input );
		   Print( "Profile Changed\n" );
		}
	}
};

class VisConsoleFunctionClass : public ConsoleFunctionClass
{
public:
	virtual	const char * Get_Name( void )	{ return "vis_toggle"; }
	virtual	const char * Get_Help( void )	{ return "VIS_TOGGLE - toggles vis culling."; }
	virtual	void Activate( const char * input ) {
		COMBAT_SCENE->Enable_Vis(!COMBAT_SCENE->Is_Vis_Enabled());
		if (COMBAT_SCENE->Is_Vis_Enabled()) {
			Print("vis enabled\n");
		} else {
			Print("vis disabled\n");
		}
	}
};

class VisInvertConsoleFunctionClass : public ConsoleFunctionClass
{
public:
	virtual	const char * Get_Name( void )	{ return "vis_invert"; }
	virtual	const char * Get_Help( void )	{ return "VIS_INVERT - toggles vis invert setting."; }
	virtual	void Activate( const char * input ) {

		COMBAT_SCENE->Invert_Vis(!COMBAT_SCENE->Is_Vis_Inverted());
		if (COMBAT_SCENE->Is_Vis_Inverted()) {
			Print("vis_invert enabled\n");
		} else {
			Print("vis_invert disabled\n");
		}
	}
};

class VisGridDisplayConsoleFunctionClass : public ConsoleFunctionClass
{
public:
	virtual	const char * Get_Name( void )	{ return "vis_grid_display"; }
	virtual	const char * Get_Help( void )	{ return "VIS_GRID_DISPLAY <0,1,2,3> - 0=off, 1=boxes, 2=centers, 3=occupied."; }
	virtual	void Activate( const char * input ) {

		int mode = 0;
		if (sscanf(input, "%d", &mode) == 1) {
			COMBAT_SCENE->Set_Vis_Grid_Display_Mode(mode);
			if (mode == 0) {
				Print("vis_grid_display off\n");
			} else if (mode == 1) {
				Print("vis_grid_display boxes\n");
			} else if (mode == 2) {
				Print("vis_grid_display centers\n");
			} else if (mode == 3) {
				Print("vis_grid_display occupied\n");
			}
		}
	}
};

class VisGridDebugConsoleFunctionClass : public ConsoleFunctionClass
{
public:
	virtual	const char * Get_Name( void )	{ return "vis_grid_debug"; }
	virtual	const char * Get_Help( void )	{ return "VIS_GRID_DEBUG <p,s,f,b,r> - p=parent,s=sibling,f=front,b=back,r=reset"; }
	virtual	void Activate( const char * input ) {

		char mode = 0;
		if (sscanf(input, "%c", &mode) == 1) {
			mode = toupper(mode);

			if (mode == 'P') {
				if (COMBAT_SCENE->Vis_Grid_Debug_Enter_Parent()) {
					Print("Entered parent node\n");
				} else {
					Print("No parent node\n");
				}
			} else if (mode == 'S') {
				if (COMBAT_SCENE->Vis_Grid_Debug_Enter_Sibling()) {
					Print("Entered sibling\n");
				} else {
					Print("No sibling\n");
				}
			} else if (mode == 'F') {
				if (COMBAT_SCENE->Vis_Grid_Debug_Enter_Front_Child()) {
					Print("Entered front child\n");
				} else {
					Print("No front child\n");
				}
			} else if (mode == 'B') {
				if (COMBAT_SCENE->Vis_Grid_Debug_Enter_Back_Child()) {
					Print("Entered back child\n");
				} else {
					Print("No back child\n");
				}
			} else if (mode == 'R') {
				COMBAT_SCENE->Vis_Grid_Debug_Reset_Node();
				Print("Reset to root\n");
			}
		}
	}
};

class VisHierarchicalConsoleFunctionClass : public ConsoleFunctionClass
{
public:
	virtual	const char * Get_Name( void )	{ return "vis_hierarchical"; }
	virtual	const char * Get_Help( void )	{ return "VIS_HIERARCHICAL - toggles hierarchical vis culling."; }
	virtual	void Activate( const char * input ) {

		PhysAABTreeCullClass::Enable_Hierarchical_Vis_Culling(!PhysAABTreeCullClass::Is_Hierarchical_Vis_Culling_Enabled());
		if (PhysAABTreeCullClass::Is_Hierarchical_Vis_Culling_Enabled()) {
			Print("hierarchical vis culling enabled\n");
		} else {
			Print("hierarchical vis culling disabled\n");
		}
	}
};

class VisLockConsoleFunctionClass : public ConsoleFunctionClass
{
public:
	virtual	const char * Get_Name( void )	{ return "vis_lock_sample_point"; }
	virtual	const char * Get_Help( void )	{ return "VIS_LOCK_SAMPLE_POINT - locks/unlocks the current pvs."; }
	virtual	void Activate( const char * input ) {

		COMBAT_SCENE->Lock_Vis_Sample_Point(!COMBAT_SCENE->Is_Vis_Sample_Point_Locked());
		if (COMBAT_SCENE->Is_Vis_Sample_Point_Locked()) {
			Print("Vis sample point locked.\n");
		} else {
			Print("Vis sample point unlocked.\n");
		}
	}
};

class VisSectorDisplayConsoleFunctionClass : public ConsoleFunctionClass
{
public:
	virtual	const char * Get_Name( void )	{ return "vis_sector_display"; }
	virtual	const char * Get_Help( void )	{ return "VIS_SECTOR_DISPLAY - toggles diplay of the current vis sector."; }
	virtual	void Activate( const char * input ) {

		COMBAT_SCENE->Enable_Vis_Sector_Display(!COMBAT_SCENE->Is_Vis_Sector_Display_Enabled());
		if (COMBAT_SCENE->Is_Vis_Sector_Display_Enabled()) {
			Print("vis_sector_display enabled\n");
		} else {
			Print("vis_sector_display disabled\n");
		}
	}
};

class VisSectorHistoryConsoleFunctionClass : public ConsoleFunctionClass
{
public:
	virtual	const char * Get_Name( void )	{ return "vis_sector_history"; }
	virtual	const char * Get_Help( void )	{ return "VIS_SECTOR_HISTORY - toggles diplay of the previous 3 vis sectors."; }
	virtual	void Activate( const char * input ) {

		COMBAT_SCENE->Enable_Vis_Sector_History_Display(!COMBAT_SCENE->Is_Vis_Sector_History_Display_Enabled());
		if (COMBAT_SCENE->Is_Vis_Sector_History_Display_Enabled()) {
			Print("vis_sector_history enabled\n");
		} else {
			Print("vis_sector_history disabled\n");
		}
	}
};


class SuicideConsoleFunctionClass : public ConsoleFunctionClass
{
public:
	virtual	const char * Get_Name( void )	{ return "suicide"; }
	virtual	const char * Get_Help( void )	{ return "SUICIDE - kill your commando."; }
	virtual	void Activate( const char * input ) {
		if (cNetwork::I_Am_Client()) {
         Print("Committing suicide.\n");

			cSuicideEvent * p_suicide = new cSuicideEvent;
			p_suicide->Init();

		} else {
         Print("Only clients can commit suicide.\n");
		}
	}
};


class ShadowAttenuationConsoleFunctionClass : public ConsoleFunctionClass
{
	virtual	const char * Get_Name( void )	{ return "shadow_attenuation"; }
	virtual	const char * Get_Help( void )	{ return "SHADOW_ATTENUATION <near> <far> - set the attenuation ranges for shadows."; }
	virtual	void Activate( const char * input ) {
		PhysicsSceneClass * scene = PhysicsSceneClass::Get_Instance();
		float znear,zfar;
		if (sscanf(input, "%f %f",&znear,&zfar) == 2) {
			scene->Set_Shadow_Attenuation(znear,zfar);
			scene->Get_Shadow_Attenuation(&znear,&zfar);
			Print("shadow attenuation set to %f %f\n",znear,zfar);
		}
	}
};

class ShadowBlobTextureConsoleFunctionClass : public ConsoleFunctionClass
{
	virtual	const char * Get_Name( void )	{ return "shadow_blob_texture"; }
	virtual	const char * Get_Help( void )	{ return "SHADOW_BLOB_TEXTURE <texture filename> - sets the texture to use for blob shadows."; }
	virtual	void Activate( const char * input ) {
		if (strlen(input) > 0) {
			if (PhysResourceMgrClass::Set_Shadow_Blob_Texture(input)) {
				Print("shadow blob texture set to: %s\n",input);
			}
		}
	}
};

class ShadowElevationConsoleFunctionClass : public ConsoleFunctionClass
{
	virtual	const char * Get_Name( void )	{ return "shadow_elevation"; }
	virtual	const char * Get_Help( void )	{ return "SHADOW_ELEVATION <elevation degrees> - sets the angle of the shadow projection."; }
	virtual	void Activate( const char * input )
	{
		/*
		** NOTE: THIS IS JUST TEST CODE TO SEE HOW VARIOUS SHADOW ALGORITHMS
		** WORK WITH VARIOUS SUN-LIGHT ANGLES.  THE SUN IS NOT TO BE MOVED IN-GAME!!!
		*/
		PhysicsSceneClass * scene = PhysicsSceneClass::Get_Instance();

		float elevation,rotation;
		scene->Get_Sun_Light_Orientation(&rotation,&elevation);
		if (sscanf(input, "%f",&elevation) == 1) {
			scene->Set_Sun_Light_Orientation(rotation,DEG_TO_RADF(elevation));
		}
	}
};


class ShadowIntensityConsoleFunctionClass : public ConsoleFunctionClass
{
	virtual	const char * Get_Name( void )	{ return "shadow_intensity"; }
	virtual	const char * Get_Help( void )	{ return "SHADOW_INTENSITY <intensity> - sets the darkness of dynamic shadows (0 - 1)."; }
	virtual	void Activate( const char * input )
	{
		PhysicsSceneClass * scene = PhysicsSceneClass::Get_Instance();
		scene->Set_Shadow_Normal_Intensity(atof(input));
		Print("shadow intensity set to: %f\n",scene->Get_Shadow_Normal_Intensity());
	}
};

class ShadowPerPolyCullingConsoleFunctionClass : public ConsoleFunctionClass
{
	virtual	const char * Get_Name( void )	{ return "shadow_per_poly_culling"; }
	virtual	const char * Get_Help( void )	{ return "SHADOW_PER_POLY_CULLING - toggles per poly shadow culling."; }
	virtual	void Activate( const char * input )
	{
		MaterialPassClass::Enable_Per_Polygon_Culling(!MaterialPassClass::Is_Per_Polygon_Culling_Enabled());
		if (MaterialPassClass::Is_Per_Polygon_Culling_Enabled()) {
			Print("per-polygon shadow culling enabled!\n");
		} else {
			Print("per-polygon shadow culling disabled\n");
		}
	}
};

class ShadowResolutionConsoleFunctionClass : public ConsoleFunctionClass
{
	virtual	const char * Get_Name( void )	{ return "shadow_resolution"; }
	virtual	const char * Get_Help( void )	{ return "SHADOW_RESOLUTION <res> - sets the resolution of dynamic shadows (use a power of 2!)"; }
	virtual	void Activate( const char * input )
	{
		PhysicsSceneClass * scene = PhysicsSceneClass::Get_Instance();
		scene->Set_Shadow_Resolution(atof(input));
		Print("shadow resolution set to: %f\n",scene->Get_Shadow_Resolution());
	}
};

class ShadowCountConsoleFunctionClass : public ConsoleFunctionClass
{
	virtual	const char * Get_Name( void )	{ return "shadow_count"; }
	virtual	const char * Get_Help( void )	{ return "SHADOW_COUNT <count> - sets the maximum simultaneous shadows"; }
	virtual	void Activate( const char * input )
	{
		PhysicsSceneClass * scene = PhysicsSceneClass::Get_Instance();
		scene->Set_Max_Simultaneous_Shadows(atof(input));
		Print("simultaneous shadow count set to: %f\n",scene->Get_Max_Simultaneous_Shadows());
	}
};


class PhysicsDebugConsoleFunctionClass : public ConsoleFunctionClass
{
public:
	virtual	const char * Get_Name( void )	{ return "physics_debug"; }
	virtual	const char * Get_Help( void )	{ return "PHYSICS_DEBUG - toggles physics debugging display."; }
	virtual	void Activate( const char * input ) {

		COMBAT_SCENE->Enable_Debug_Display(!COMBAT_SCENE->Is_Debug_Display_Enabled());
		if (COMBAT_SCENE->Is_Debug_Display_Enabled()) {
			Print("physics debug display enabled!\n");
		} else {
			Print("physics debug display disabled :-(\n");
		}
	}
};

class Phys3NetConsoleFunctionClass : public ConsoleFunctionClass
{
public:
	virtual	const char * Get_Name( void )	{ return "phys3_net"; }
	virtual	const char * Get_Help( void )	{ return "PHYS3_NET <allowable_error> <pop_error> <corr_time> - sets the phys3 networking constants."; }
	virtual	void Activate( const char * input ) {
		float corr_time,allow_error,pop_error;
		int params = sscanf(input, "%f %f %f",&allow_error,&pop_error,&corr_time);
		if (params == 3) {
			Phys3Class::Set_Allowable_Error(allow_error);
			Phys3Class::Set_Pop_Error(pop_error);
			Phys3Class::Set_Correction_Time(corr_time);
		}
		Print("Phys3 Network Params: allowable_error = %f\r\n",Phys3Class::Get_Allowable_Error());
		Print("Phys3 Network Params: pop_error = %f\r\n",Phys3Class::Get_Pop_Error());
		Print("Phys3 Network Params: correction_time = %f\r\n",Phys3Class::Get_Correction_Time());
	}
};

class RBodyNetConsoleFunctionClass : public ConsoleFunctionClass
{
public:
	virtual	const char * Get_Name( void )	{ return "rbody_net"; }
	virtual	const char * Get_Help( void )	{ return "rbody_net <corr_time> - sets the rigid body correction time."; }
	virtual	void Activate( const char * input ) {
		float corr_time;
		int params = sscanf(input, "%f",&corr_time );
		if (params == 3) {
			RigidBodyClass::Set_Correction_Time(corr_time);
		}
		Print("Rigid Body Network Params: correction_time = %f\r\n",RigidBodyClass::Get_Correction_Time());
	}
};

class ProjectorDebugConsoleFunctionClass : public ConsoleFunctionClass
{
public:
	virtual	const char * Get_Name( void )	{ return "projector_debug"; }
	virtual	const char * Get_Help( void )	{ return "PROJECTOR_DEBUG - toggles projector debugging display."; }
	virtual	void Activate( const char * input ) {

		COMBAT_SCENE->Enable_Projector_Debug_Display(!COMBAT_SCENE->Is_Projector_Debug_Display_Enabled());
		if (COMBAT_SCENE->Is_Projector_Debug_Display_Enabled()) {
			Print("projector debug display enabled!\n");
		} else {
			Print("projector debug display disabled :-(\n");
		}
	}
};

class DirtyCullDebugConsoleFunctionClass : public ConsoleFunctionClass
{
public:
	virtual	const char * Get_Name( void )	{ return "dirty_cull_debug"; }
	virtual	const char * Get_Help( void )	{ return "DIRTY_CULL_DEBUG - toggles debugging of the dirty-cull-list."; }
	virtual	void Activate( const char * input ) {

		COMBAT_SCENE->Enable_Dirty_Cull_Debug_Display(!COMBAT_SCENE->Is_Dirty_Cull_Debug_Display_Enabled());
		if (COMBAT_SCENE->Is_Dirty_Cull_Debug_Display_Enabled()) {
			Print("dirty cull debug display enabled!\n");
		} else {
			Print("dirty cull debug display disabled.\n");
		}
	}
};

class NextRenderConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "next_render"; }
	virtual	const char * Get_Help( void )	{ return "NEXT_RENDER - switches to the next render device."; }
	virtual	void Activate( const char * input ) {
		WW3D::Set_Next_Render_Device();
	}
};

class OneShotKillsConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "one_shot_kills"; }
	virtual	const char * Get_Alias( void )	{ return "osk"; }
	virtual	const char * Get_Help( void )	{ return "ONE_SHOT_KILLS - kill or destroy with one shot. Server only."; }
	virtual	void Activate( const char * input ) {

		if (cNetwork::I_Am_Server()) {
         bool osk = false;

#ifdef WWDEBUG
			osk = DefenseObjectClass::Toggle_One_Shot_Kills();
#endif // WWDEBUG

         Print( "One Shot Kills: %s\n", osk ? "ON" : "OFF");
		} else {
         Print(Get_Help());
		}
	}
};

class OpenConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "open"; }
	virtual	const char * Get_Help( void )	{ return "OPEN [regedit|logfile|ini ininame] - open specified object."; }
	virtual	void Activate( const char * input ) {

		if (!stricmp(input, "regedit")) {
			cNetwork::Shell_Command("regedit");
		} else if (!stricmp(input, "logfile")) {
			cNetwork::Shell_Command(DebugManager::Logfile_Name());
		} else if (strstr(input, "ini")) {
			char filename[200];
			sscanf(input, "ini %s", filename);
			char open_command[200];
			sprintf(open_command, "data\\%s.ini", filename);
			cNetwork::Shell_Command(open_command);
		} else {
		   Print(Get_Help());
		}
	}
};

class InvalidateMeshCacheConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "invalidate_mesh_cache"; }
	virtual	const char * Get_Help( void )	{ return "INVALIDATE_MESH_CACHE - invalidates all cached mesh data."; }
	virtual	void Activate( const char * input ) {
		WW3D::_Invalidate_Mesh_Cache();
      Print( "Mesh cache invalidated." );
	}
};

class InvalidateTexturesConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "invalidate_textures"; }
	virtual	const char * Get_Help( void )	{ return "INVALIDATE_TEXTURES - invalidates all textures."; }
	virtual	void Activate( const char * input ) {
		WW3D::_Invalidate_Textures();
      Print( "Textures invalidated." );
	}
};

class TexturingEnableConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "texturing_enable"; }
	virtual	const char * Get_Help( void )	{ return "TEXTURING_ENABLE toggle texturing."; }
	virtual	void Activate( const char * input ) {
		WW3D::Enable_Texturing(!WW3D::Is_Texturing_Enabled());
		Print( "Textures switched." );
	}
};

class ExposePrelitConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "expose_prelit"; }
	virtual	const char * Get_Help( void )	{ return "EXPOSE_PRELIT - expose precalculated lighting on meshes."; }
	virtual	void Activate( const char * input ) {

		static const char *_comment [2] = {
			"Unexposed prelit meshes\n",
			"Exposed	prelit meshes\n"
		};

		WW3D::Expose_Prelit (!WW3D::Expose_Prelit());
		Print (_comment [WW3D::Expose_Prelit() ? 1 : 0]);
	}
};

class PlayerPositionConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "player_position"; }
	//virtual	const char * Get_Alias( void )	{ return "pp"; }
	virtual	const char * Get_Help( void )	{ return "PLAYER_POSITION - toggles the players world position display."; }
	virtual	void Activate( const char * input ) {
		Get_Console()->Toggle_Player_Position();
	}
};

class ToggleAssetPreloadingConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "toggle_preloading"; }
	virtual	const char * Get_Help( void )	{ return "TOGGLE_PRELOADING - toggles asset preloading."; }
	virtual	void Activate( const char * input ) {
		bool is_preloading = cDevOptions::PreloadAssets.Toggle();
      Print(is_preloading ? "Assets WILL preload.\n" : "Assets will NOT preload.\n" );
	}
};

class ToggleRenderingConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "toggle_rendering"; }
	virtual	const char * Get_Help( void )	{ return "TOGGLE_RENDERING <static,dynamic,human,vehicles> - toggles rendering of various types."; }
	virtual	void Activate( const char * input ) {

		/*
		** Note: I'm only comparing the first two characters so the user doesn't have to type the whole thing
		*/
		if (_strnicmp(input,"st",2) == 0) {

			StaticPhysClass::Disable_All_Rendering(!StaticPhysClass::Is_All_Rendering_Disabled());
			Print(StaticPhysClass::Is_All_Rendering_Disabled() ? "Static object rendering DISABLED" : "Static object rendering ENABLED");

		} else if (_strnicmp(input,"dy",2) == 0) {

			DynamicPhysClass::Disable_All_Rendering(!DynamicPhysClass::Is_All_Rendering_Disabled());
			Print(DynamicPhysClass::Is_All_Rendering_Disabled() ? "Dynamic object rendering DISABLED" : "Dynamic object rendering ENABLED");

		} else if (_strnicmp(input,"hu",2) == 0) {

			HumanPhysClass::Disable_All_Rendering(!HumanPhysClass::Is_All_Rendering_Disabled());
			Print(HumanPhysClass::Is_All_Rendering_Disabled() ? "Human object rendering DISABLED" : "Human object rendering ENABLED");

		} else if (_strnicmp(input,"ve",2) == 0) {

			VehiclePhysClass::Disable_All_Rendering(!VehiclePhysClass::Is_All_Rendering_Disabled());
			Print(VehiclePhysClass::Is_All_Rendering_Disabled() ? "Vehicle object rendering DISABLED" : "Vehicle object rendering ENABLED");

		}
	}
};


class ToggleSimulationConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "toggle_simulation"; }
	virtual	const char * Get_Help( void )	{ return "TOGGLE_SIMULATION <static,dynamic,human,vehicles> - toggles rendering of various types."; }
	virtual	void Activate( const char * input ) {

		/*
		** Note: I'm only comparing the first two characters so the user doesn't have to type the whole thing
		*/
		if (_strnicmp(input,"st",2) == 0) {

			StaticPhysClass::Disable_All_Simulation(!StaticPhysClass::Is_All_Simulation_Disabled());
			Print(StaticPhysClass::Is_All_Simulation_Disabled() ? "Static object Simulation DISABLED" : "Static object Simulation ENABLED");

		} else if (_strnicmp(input,"dy",2) == 0) {

			DynamicPhysClass::Disable_All_Simulation(!DynamicPhysClass::Is_All_Simulation_Disabled());
			Print(DynamicPhysClass::Is_All_Simulation_Disabled() ? "Dynamic object Simulation DISABLED" : "Dynamic object Simulation ENABLED");

		} else if (_strnicmp(input,"hu",2) == 0) {

			HumanPhysClass::Disable_All_Simulation(!HumanPhysClass::Is_All_Simulation_Disabled());
			Print(HumanPhysClass::Is_All_Simulation_Disabled() ? "Human object Simulation DISABLED" : "Human object Simulation ENABLED");

		} else if (_strnicmp(input,"ve",2) == 0) {

			VehiclePhysClass::Disable_All_Simulation(!VehiclePhysClass::Is_All_Simulation_Disabled());
			Print(VehiclePhysClass::Is_All_Simulation_Disabled() ? "Vehicle object Simulation DISABLED" : "Vehicle object Simulation ENABLED");

		}
	}
};


class TomConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "tom"; }
	virtual	const char * Get_Help( void )	{ return "TOM - Annoyance-reduction registry tweaks customized by Tom."; }
	virtual	void Activate( const char * input ) {

		//
		// Enable a good level of diagnostics - all devices and types except
		// screen and netprolific
		//
      ConsoleFunctionManager::Parse_Input("debug_device on");
      ConsoleFunctionManager::Parse_Input("debug_device screen");
      ConsoleFunctionManager::Parse_Input("debug_type on");
      ConsoleFunctionManager::Parse_Input("debug_type netprolific");

		//
		// Set max scroll lines
		//
		if (Get_Text_Display()) {
			WWASSERT(Get_Text_Display() != NULL);
			Get_Text_Display()->Set_Max_Scroll_Lines(10);
		}

		//
		// Debug flags
		//
		//cDevOptions::ShowDiagnostics.Set(true);
		cDevOptions::PreloadAssets.Set(false);
		cDevOptions::ShowFps.Set(true);
		cDevOptions::ShowGodStatus.Set(true);
		cDevOptions::CompareExeVersionOnNetwork.Set(false);
		cDevOptions::CrtDbgEnabled.Set(false);
		cDevOptions::LogDataSafe.Set(false);
		cDevOptions::FilterLevelFiles.Set(false);
		cDevOptions::ShowThumbnailPreInitDialog.Set(false);

		//
		// Release flags
		//
		cUserOptions::ShowNamesOnSoldier.Set(true);
		cUserOptions::CameraLockedToTurret.Set(true);
		cUserOptions::SkipQuitConfirmDialog.Set(true);
		cUserOptions::SkipIngameQuitConfirmDialog.Set(true);
		cUserOptions::PermitDiagLogging.Set(false);

		//
		// Set 800 x 600, windowed mode
		//
		{
		RegistryClass registry(APPLICATION_SUB_KEY_NAME_RENDER);
		WWASSERT(registry.Is_Valid());
		registry.Set_Int(VALUE_NAME_RENDER_DEVICE_WIDTH, 800);
		registry.Set_Int(VALUE_NAME_RENDER_DEVICE_HEIGHT, 600);
		registry.Set_Int(VALUE_NAME_RENDER_DEVICE_WINDOWED, TRUE);
		}

		/*
		//
		// Stop Jani from spying on us (avoid delay on exit)
		//
		{
		RegistryClass registry(APPLICATION_SUB_KEY_NAME_DEBUG);
		WWASSERT(registry.Is_Valid());
		registry.Set_Int("SystemInfoLogDisable", TRUE);
		registry.Set_Int("DisableLogCopying", TRUE);
		}
		*/

		//
		// Default to third person
		//
		{
		RegistryClass registry(APPLICATION_SUB_KEY_NAME_OPTIONS);
		WWASSERT(registry.Is_Valid());
		registry.Set_Int("DefaultToFirstPerson", FALSE);
		}

		//
		// Skip intro movies
		//
		{
		RegistryClass registry(APPLICATION_SUB_KEY_NAME_OPTIONS);
		WWASSERT(registry.Is_Valid());
		registry.Set_Int("SkipAllIntroMovies", TRUE);
		}

		//
		// Do not require CD for movies
		//
		{
		RegistryClass registry(APPLICATION_SUB_KEY_NAME_DEBUG);
		WWASSERT(registry.Is_Valid());
		registry.Set_Int("DisableCDCheck", TRUE);
		}

		//
		// Set gamma etc so that they don't screw up debugging
		//
		{
		RegistryClass registry(APPLICATION_SUB_KEY_NAME_SYSTEM_SETTINGS);
		WWASSERT(registry.Is_Valid());
		registry.Set_Int("Gamma",			1);
		registry.Set_Int("Brightness",	0);
		registry.Set_Int("Contrast",		1);
		}
	}
};

class RenderBoxesConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "render_boxes"; }
	virtual	const char * Get_Help( void )	{ return "RENDER_BOXES <mask> - enables rendering of boxes.  0=none 1=all 2=physical 4=projectile."; }
	virtual	void Activate( const char * input ) {
		int mask = atoi(input);
		WW3D::Set_Collision_Box_Display_Mask(mask);
	}
};

class AmbientLightConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "ambient_light"; }
	virtual	const char * Get_Help( void )	{ return "AMBIENT_LIGHT <R> <G> <B> - sets the ambeint light color to <r> <g> <b>."; }
	virtual	void Activate( const char * input ) {
		if ( COMBAT_SCENE ) {
			float r, g, b;
			if (sscanf(input, "%f %f %f", &r, &g, &b) == 3) {
		      COMBAT_SCENE->Set_Ambient_Light(Vector3(r, g, b));
		      Print( "Ambient Light changed\n" );
			}
		} else {
		   Print( "Error : no COMBAT_SCENE\n" );
		}
	}
};

class VblankSyncConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "vblank_sync"; }
	virtual	const char * Get_Help( void )	{ return "VBLANK_SYNC - toggles vblank syncing"; }
	virtual	void Activate( const char * input ) {

		if (WW3D::Get_Ext_Swap_Interval() == 0) {
			WW3D::Set_Ext_Swap_Interval(1);
			Print("Vblank syncing ENABLED!\n");
		} else {
			WW3D::Set_Ext_Swap_Interval(0);
			Print("Vblank syncing DISABLED!\n");
		}
	}
};

class FrameTexturesConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "frame_textures"; }
	virtual	const char * Get_Help( void )	{ return "FRAME_TEXTURES - measures the amoutn of textures needed for the next frame."; }
	virtual	void Activate( const char * input ) {
		DebugManager::Measure_Frame_Textures();
	}
};

class DebugObjectivesConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "objectives_debug"; }
	virtual	const char * Get_Help( void )	{ return "OBJECTIVES_DEBUG - toggles Mission Objectives debug mode."; }
	virtual	void Activate( const char * input ) {
		bool enable_mode = !ObjectiveManager::Is_Objective_Debug_Mode_Enabled();
		ObjectiveManager::Enable_Objective_Debug_Mode( enable_mode );
		if (enable_mode) {
			Print( "Activating Mission Objectives Debug Mode.\n" );
		} else {
			Print( "Deactivating Mission Objectives Debug Mode.\n" );
		}
	}
};


class FreeWeaponsConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "free_weapons"; }
	virtual	const char * Get_Help( void )	{ return "FREE_WEAPONS - toggle free_weapons flag (server only)."; }
	virtual	void Activate( const char * input ) {
      if (!cNetwork::I_Am_Server()) {
			Print("Only the server can toggle free-weapons.\n");
      } else {
			WWASSERT(PTheGameData != NULL);
			bool on = The_Game()->IsFreeWeapons.Toggle();
         Print("Free weapons is %s.\n", on ? "ON" : "OFF");

			if (on) {
				//
				// Give everyone free weapons right away
				//
				SLNode<SmartGameObj> * objnode;
				for (objnode = GameObjManager::Get_Smart_Game_Obj_List()->Head(); objnode; objnode = objnode->Next()) {
					SmartGameObj * p_smart_obj = objnode->Data();
					WWASSERT(p_smart_obj != NULL);
					if (p_smart_obj->As_SoldierGameObj() != NULL &&
						p_smart_obj->Get_Player_Type() != PLAYERTYPE_NEUTRAL) {
						p_smart_obj->As_SoldierGameObj()->Give_All_Weapons();
					}
				}
			}
      }
	}
};


class ToggleEmotIconsConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "toggle_emoticons"; }
	virtual	const char * Get_Help( void )	{ return "TOGGLE_EMOTICONS - Toggle the display of emot icons during conversations."; }
	virtual	void Activate( const char * /*input*/ ) {

		bool display = !ConversationMgrClass::Are_Emot_Icons_Displayed ();
		ConversationMgrClass::Display_Emot_Icons (display);

		if (display) {
			Print( "EmotIcons are now enabled.\n" );
		} else {
			Print( "EmotIcons are now disabled.\n" );
		}
	}
};


class SoldierGhostDebugDisplayConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "toggle_ghost_debug_display"; }
	virtual	const char * Get_Help( void )	{ return "toggle_ghost_debug_display - Displays a box around soldiers that are in 'ghost' collision mode."; }
	virtual	void Activate( const char * input ) {

		bool enable = !SoldierGameObj::Is_Ghost_Collision_Debug_Display_Enabled ();
		SoldierGameObj::Enable_Ghost_Collision_Debug_Display (enable);

		if (enable) {
			Print( "Ghost collision debug display enabled.\n" );
		} else {
			Print( "Ghost collision debug display disabled.\n" );
		}

		return ;
	}
};


class HUDHelpTextTestConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "hud_help_text"; }
	//virtual	const char * Get_Help( void )	{ return "CnC [vehicle<def_name> | powerup<def_name> |char<class>[minigunner|rocket_soldier|grenadier|engineer|flame_thrower|mutant] <rank>[enlisted|officer|special_forces|boss] ] - Purchases a team vehicle, a player powerup or upgrades the player."; }
	virtual	const char * Get_Help( void )	{ return "hud_help_text - Sets the current HUD help text."; }
	virtual	void Activate( const char * input ) {

		WideStringClass wide_string;
		wide_string.Convert_From (input);
		HUDInfo::Set_HUD_Help_Text (wide_string);
		return ;
	}
};


class CnCModeConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "cnc"; }
	//virtual	const char * Get_Help( void )	{ return "CnC [vehicle<def_name> | powerup<def_name> |char<class>[minigunner|rocket_soldier|grenadier|engineer|flame_thrower|mutant] <rank>[enlisted|officer|special_forces|boss] ] - Purchases a team vehicle, a player powerup or upgrades the player."; }
	virtual	const char * Get_Help( void )	{ return "CnC - Displays CnC player console (for gods only)."; }
	virtual	void Activate( const char * input ) {

		if (cNetwork::I_Am_God()) {
			PlayerTerminalClass::Get_Instance ()->Display_Default_Terminal_For_Player (COMBAT_STAR);
		} else {
			Print(Get_Help());
		}
	}
};


class DebugEncyRevealConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "reveal_encyclopedia"; }
	virtual	const char * Get_Help( void )	{ return "reveal_encyclopedia - Shows all entries in the encyclopedia."; }
	virtual	void Activate( const char * input ) {

		EncyclopediaMgrClass::Reveal_All_Objects ();
		return ;
	}
};


class DebugShowMoveVectorConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "move_vectors"; }
	virtual	const char * Get_Help( void )	{ return "move_vectors - Toggles the debug move vectors."; }
	virtual	void Activate( const char * input ) {
		bool displayed = PathClass::Are_Move_Vectors_Displayed ();
		displayed = !displayed;
		PathClass::Display_Move_Vectors (displayed);
		if ( displayed ) {
			Print( "Move vectors ON.\n" );
		} else {
			Print( "Move vectors OFF.\n" );
		}

		return ;
	}
};


class MapRevealConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "map_reveal"; }
	virtual	const char * Get_Help( void )	{ return "map_reveal - Removes the cloud from the map."; }
	virtual	void Activate( const char * input ) {
		MapMgrClass::Clear_All_Cloud_Cells ();
		Print( "Map is now visible.\n" );
		return ;
	}
};


class ChangeCharacterClassConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "change_character_class"; }
	virtual	const char * Get_Help( void )	{ return "CHANGE_CHARACTER_CLASS <preset_name> - changes your character to the specified class."; }
	virtual	void Activate( const char * input ) {
		bool success = false;

		DefinitionClass *definition = DefinitionMgrClass::Find_Typed_Definition( input, CLASSID_GAME_OBJECT_DEF_SOLDIER );
		if (definition != NULL) {
			SoldierGameObjDef *soldier_def = reinterpret_cast<SoldierGameObjDef *> (definition);
			COMBAT_STAR->Re_Init( *soldier_def );
			COMBAT_STAR->Post_Re_Init();
			success = true;
		}

		if (success) {
			Print( "Successfully changed character class.\n" );
		} else {
			Print( "Unable to change character class.\n" );
		}
	}
};

class RenderModeConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "render_mode"; }
   virtual	const char * Get_Alias( void ) { return "rm"; }
	virtual	const char * Get_Help( void )	{ return "RENDER_MODE [0 fill | 1 lines | 2 points] - set the current render mode."; }
	virtual	void Activate( const char * input ) {
		if (COMBAT_SCENE != NULL) {
			if ((stricmp(input,"points") == 0) || (stricmp(input,"2") == 0)) {
				COMBAT_SCENE->Set_Polygon_Mode(SceneClass::POINT);
				Print("Polygon mode set to POINT\n");
			} else if ((stricmp(input,"lines") == 0) || (stricmp(input,"1") == 0)) {
				COMBAT_SCENE->Set_Polygon_Mode(SceneClass::LINE);
				Print("Polygon mode set to LINE\n");
			} else if ((stricmp(input,"fill") == 0) || (stricmp(input,"0") == 0)) {
				COMBAT_SCENE->Set_Polygon_Mode(SceneClass::FILL);
				Print("Polygon mode set to FILL\n");
			}
		}
	}
};

class RenderModeExtraPassConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "render_mode_extra_pass"; }
   virtual	const char * Get_Alias( void ) { return "rm"; }
	virtual	const char * Get_Help( void )	{ return "RENDER_MODE_EXTRA_PASS [0 disable | 1 lines | 2 zbuffered]"; }
	virtual	void Activate( const char * input ) {
		if (COMBAT_SCENE != NULL) {
			if ((stricmp(input,"disable") == 0) || (stricmp(input,"0") == 0)) {
				COMBAT_SCENE->Set_Extra_Pass_Polygon_Mode(SceneClass::EXTRA_PASS_DISABLE);
				Print("Extra pass polygon mode set to DISABLE\n");
			} else if ((stricmp(input,"lines") == 0) || (stricmp(input,"1") == 0)) {
				COMBAT_SCENE->Set_Extra_Pass_Polygon_Mode(SceneClass::EXTRA_PASS_LINE);
				Print("Extra pass polygon mode set to LINE\n");
			} else if ((stricmp(input,"zbuffered") == 0) || (stricmp(input,"2") == 0)) {
				COMBAT_SCENE->Set_Extra_Pass_Polygon_Mode(SceneClass::EXTRA_PASS_CLEAR_LINE);
				Print("Extra pass polygon mode set to ZBUFFERED\n");
			}
		}
	}
};

class TextureFilterModeConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "texture_filter_mode"; }
	virtual	const char * Get_Help( void )	{ return "TEXTURE_FILTER_MODE [none|fast|best] - set texture filter and mipmap modes."; }
	virtual	void Activate( const char * input ) {
		if (stricmp(input,"none") == 0) {
			TextureClass::_Set_Default_Min_Filter(TextureClass::FILTER_TYPE_FAST);
			TextureClass::_Set_Default_Mag_Filter(TextureClass::FILTER_TYPE_FAST);
			TextureClass::_Set_Default_Mip_Filter(TextureClass::FILTER_TYPE_NONE);
			Print("Texture filter mode changed to: none\n");
		}
		else if (stricmp(input,"fast") == 0) {
			TextureClass::_Set_Default_Min_Filter(TextureClass::FILTER_TYPE_FAST);
			TextureClass::_Set_Default_Mag_Filter(TextureClass::FILTER_TYPE_FAST);
			TextureClass::_Set_Default_Mip_Filter(TextureClass::FILTER_TYPE_FAST);
			Print("Texture filter mode changed to: fast\n");
		}
		if (stricmp(input,"best") == 0) {
			TextureClass::_Set_Default_Min_Filter(TextureClass::FILTER_TYPE_BEST);
			TextureClass::_Set_Default_Mag_Filter(TextureClass::FILTER_TYPE_BEST);
			TextureClass::_Set_Default_Mip_Filter(TextureClass::FILTER_TYPE_BEST);
			Print("Texture filter mode changed to: best\n");
		}
		else {
			Print("Valid filter modes are: none|fast|best\n");
		}
	}
};

class EnableTriangleRenderConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "enable_triangle_render"; }
	virtual	const char * Get_Help( void )	{ return "ENABLE_TRIANGLE_RENDER [none|opaque|sorting|all] - enable/disable triangle rendering."; }
	virtual	void Activate( const char * input ) {
		if (stricmp(input,"none") == 0) {
			DX8Wrapper::_Enable_Triangle_Draw(false);
			SortingRendererClass::_Enable_Triangle_Draw(false);
			Print("All polygon rendering disabled\n");
		}
		else if (stricmp(input,"opaque") == 0) {
			DX8Wrapper::_Enable_Triangle_Draw(true);
			SortingRendererClass::_Enable_Triangle_Draw(false);
			Print("Only opaque polygons enabled\n");
		}
		else if (stricmp(input,"sorting") == 0) {
			DX8Wrapper::_Enable_Triangle_Draw(false);
			SortingRendererClass::_Enable_Triangle_Draw(true);
			Print("Only sorting polygons enabled\n");
		}
		else {
			DX8Wrapper::_Enable_Triangle_Draw(true);
			SortingRendererClass::_Enable_Triangle_Draw(true);
			Print("All polygons enabled\n");
		}
	}
};

class RunFastConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "run_fast"; }
	virtual	const char * Get_Help( void )	{ return "RUN_FAST - set options for fast framerate."; }
	virtual	void Activate( const char * input ) {

		DebugManager::Option_Enable(DebugManager::DEBUG_DISABLE_LIGHTS, true);
		WWASSERT(DebugManager::Option_Is_Enabled(DebugManager::DEBUG_DISABLE_LIGHTS));
		Print("Lights disabled.\n");

      ConsoleFunctionManager::Parse_Input("Dynamic_LOD_Budget 100");

      ConsoleFunctionManager::Parse_Input("Static_LOD_Budget 100");

      ConsoleFunctionManager::Parse_Input("Shadow_Mode 0");

      ConsoleFunctionManager::Parse_Input("Prelit_Mode 0");

      //ConsoleFunctionManager::Parse_Input("Mesh_Draw_Mode 0");

		if (COMBAT_SCENE != NULL) {
			COMBAT_SCENE->Enable_Dynamic_Projectors(false);
			Print("Dynamic Projectors disabled.\n");
			COMBAT_SCENE->Enable_Static_Projectors(false);
			Print("Static Projectors disabled.\n");
		}

		WW3D::Set_Texture_Reduction(1);
		Print("Texture Resolution set to 1.\n");

		ConsoleGameModeClass::Save_Registry_Keys();

		if (Get_Text_Display()) {
			WWASSERT(Get_Text_Display() != NULL);
			Get_Text_Display()->Set_Max_Scroll_Lines(3);
			Print("Set_Max_Scroll_Lines(3)\n");
		}
	}
};

class RenderBackfacesConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "render_backfaces"; }
	virtual	const char * Get_Help( void )	{ return "RENDER_BACKFACES - toggles debug rendering of backfaced polys."; }
	virtual	void Activate( const char * input ) {
		/*
		** This turns on WW3D backface rendering and also causes the Physics-Scene to
		** stop rendering non-occluders so that we can get a better idea of what VIS
		** is seeing when it renders the occluders and checks for backfaces
		*/
		PhysicsSceneClass * scene = PhysicsSceneClass::Get_Instance();
		if (scene) {
			scene->Enable_Backface_Occluder_Debug(!scene->Is_Backface_Occluder_Debug_Enabled());
			if (scene->Is_Backface_Occluder_Debug_Enabled()) {
				Print("Backface Debug Display ENABLED\n");
			} else {
				Print("Backface Debug Display DISABLED\n");
			}
		}
	}
};

class AmmoConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "ammo"; }
	virtual	const char * Get_Help( void )	{ return "AMMO <count> - sets the players weapon's ammo count."; }
	virtual	void Activate( const char * input ) {
		if ( COMBAT_STAR != NULL ) {
			WeaponClass * weapon = COMBAT_STAR->Get_Weapon();
			if ( weapon ) {
				int amount = atoi( input );
				weapon->Set_Total_Rounds( amount );
				Print( "Ammo set to %d rounds\n", amount );
			}
		}
	}
};

extern	bool		ScriptTrace;
class ScriptTraceConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "script_trace"; }
	virtual	const char * Get_Help( void )	{ return "SCRIPT_TRACE - toggles debug trace of all scipt events and commands."; }
	virtual	void Activate( const char * input ) {
      ScriptTrace = !ScriptTrace;
		Print( "ScriptTrace is %s\n", ScriptTrace ? "ON" : "OFF" );
	}
};


class DebugTypeConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "debug_type"; }
   const char * Get_Alias( void ) { return "dt"; }
	virtual	const char * Get_Help( void )	{ return
		"DEBUG_TYPE [type|on|off] - toggles the debug display of type (info, warning, error, script)"; }
	virtual	void Activate( const char * input ) {
		char str[128];
		sprintf(str,"ERROR (%s)\n", input );
		if (stricmp(input,"info") == 0) {
			DebugManager::Toggle_Type_Enabled( DebugManager::DEBUG_TYPE_INFORMATION );
			sprintf(str, "Information Debug %s\n", DebugManager::Is_Type_Enabled( DebugManager::DEBUG_TYPE_INFORMATION ) ? "Enabled" : "Disabled" );
		} else if (stricmp(input,"warning") == 0) {
			DebugManager::Toggle_Type_Enabled( DebugManager::DEBUG_TYPE_WARNING );
			sprintf(str, "Warning Debug %s\n", DebugManager::Is_Type_Enabled( DebugManager::DEBUG_TYPE_WARNING ) ? "Enabled" : "Disabled" );
		} else if (stricmp(input,"error") == 0) {
			DebugManager::Toggle_Type_Enabled( DebugManager::DEBUG_TYPE_ERROR );
			sprintf(str, "Error Debug %s\n", DebugManager::Is_Type_Enabled( DebugManager::DEBUG_TYPE_ERROR ) ? "Enabled" : "Disabled" );
		} else if (stricmp(input,"script") == 0) {
			DebugManager::Toggle_Type_Enabled( DebugManager::DEBUG_TYPE_SCRIPT );
			sprintf(str, "Script Debug %s\n", DebugManager::Is_Type_Enabled( DebugManager::DEBUG_TYPE_SCRIPT ) ? "Enabled" : "Disabled" );
		} else if (stricmp(input,"netadmin") == 0) {
			DebugManager::Toggle_Type_Enabled( DebugManager::DEBUG_TYPE_NETWORK_ADMIN );
			sprintf(str, "Network Admin Debug %s\n", DebugManager::Is_Type_Enabled( DebugManager::DEBUG_TYPE_NETWORK_ADMIN ) ? "Enabled" : "Disabled" );
		} else if (stricmp(input,"netbasic") == 0) {
			DebugManager::Toggle_Type_Enabled( DebugManager::DEBUG_TYPE_NETWORK_BASIC );
			sprintf(str, "Network Basic Debug %s\n", DebugManager::Is_Type_Enabled( DebugManager::DEBUG_TYPE_NETWORK_BASIC ) ? "Enabled" : "Disabled" );
		} else if (stricmp(input,"netprolific") == 0) {
			DebugManager::Toggle_Type_Enabled( DebugManager::DEBUG_TYPE_NETWORK_PROLIFIC );
			sprintf(str, "Network Prolific Debug %s\n", DebugManager::Is_Type_Enabled( DebugManager::DEBUG_TYPE_NETWORK_PROLIFIC ) ? "Enabled" : "Disabled" );
		} else if (stricmp(input,"net") == 0) {
         DebugManager::Disable_Type( DebugManager::DEBUG_TYPE_NETWORK_ADMIN );
         DebugManager::Disable_Type( DebugManager::DEBUG_TYPE_NETWORK_BASIC );
         DebugManager::Disable_Type( DebugManager::DEBUG_TYPE_NETWORK_PROLIFIC );
			sprintf(str, "All network debug types DISABLED\n");
		} else if (stricmp(input,"on") == 0) {
			DebugManager::Enable_Type( (DebugManager::DebugType)-1 );
			sprintf(str, "All Debug Types Enabled\n" );
		} else if (stricmp(input,"off") == 0) {
			DebugManager::Disable_Type( (DebugManager::DebugType)-1 );
			sprintf(str, "All Debug Types Disabled\n" );
		}
		Print(str);
		Debug_Say((str));
	}
};

/*
class RadarZoomConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "radar_zoom"; }
	virtual	const char * Get_Help( void )	{ return "RADAR_ZOOM <factor> - Set radar zoom factor."; }
	virtual	void Activate( const char * input ) {
		float zoom_factor = atof(input);
		if (zoom_factor > WWMATH_EPSILON) {
//			RadarManager::Set_Zoom_Factor(zoom_factor);
			Print( "Radar zoomed\n" );
		} else {
			Print( "Error - zoom_factor must be small and positive.\n" );
		}
	}
};


class RadarMaxConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "radar_max"; }
	virtual	const char * Get_Help( void )	{ return "RADAR_MAX - zoom out the radar to world extents."; }
	virtual	void Activate( const char * input ) {
		float world_size = The_Game()->Get_Maximum_World_Distance();
		if (world_size > WWMATH_EPSILON) {
//			RadarManager::Set_Zoom_Factor(1 / world_size);
			Print( "Radar zoomed\n" );
		} else {
			Print( "Error - world size not set.\n" );
		}
	}
};
*/

class DefectConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "defect"; }
	virtual	const char * Get_Help( void )	{ return "DEFECT - switch to the opposite team (gods only)."; }
	virtual	void Activate( const char * input ) {

		cPlayer * p_player = cNetwork::Get_My_Player_Object();

		if (p_player != NULL && p_player->Invulnerable.Is_True()) {
			cChangeTeamEvent * p_event = new cChangeTeamEvent;
			p_event->Init();
		} else {
			Print(Get_Help());
		}
	}
};


class RadarToggleConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "radar_toggle"; }
	virtual	const char * Get_Help( void )	{ return "RADAR_TOGGLE - toggles the Radar."; }
	virtual	void Activate( const char * input ) {
		RadarManager::Set_Hidden( !RadarManager::Is_Hidden() );
      Print( "Radar toggled\n" );
	}
};


class ShellCommandConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "shell"; }
	virtual	const char * Get_Help( void )	{ return "SHELL - SHell command (server only)."; }
	virtual	void Activate( const char * input ) {
      if (IS_SOLOPLAY || !cNetwork::I_Am_Server()) {
         Print("SH is for server only.\n");
      } else {
         cNetwork::Shell_Command(input);
      }
	}
};


/*
class RshCommandConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "rsh"; }
	virtual	const char * Get_Help( void )	{ return "RSH - Remote SHell command (server only)."; }
	virtual	void Activate( const char * input ) {
      if (IS_SOLOPLAY || !cNetwork::I_Am_Server()) {
         Print("rsh is for server only.\n");
      } else {
         Print("Sending command to clients...\n");
         cNetwork::Send_Server_Rsh_Command(input);
      }
	}
};
*/


class LrshCommandConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "lrsh"; }
	virtual	const char * Get_Help( void )	{ return "LRSH - Local and Remote SHell command (server only)."; }

	virtual	void Activate( const char * input ) {
      /*
		ConsoleFunctionManager::Parse_Input(input);
      char rsh_command[500];
      WWASSERT(strlen(input) < sizeof(rsh_command) - 10);
      sprintf(rsh_command, "rsh %s", input);
      ConsoleFunctionManager::Parse_Input(rsh_command);
		*/

      if (IS_SOLOPLAY || !cNetwork::I_Am_Server() || ::strlen(input) == 0) {
		   Print(Get_Help());
		} else {
			cConsoleCommandEvent * p_event = new cConsoleCommandEvent;
			p_event->Init(input);
		}
	}
};


class SrshCommandConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "srsh"; }
	virtual	const char * Get_Help( void )	{ return "SRSH - C->S Remote SHell command (client only)."; }

	virtual	void Activate( const char * input ) {

		if (cNetwork::I_Am_Only_Client() && ::strlen(input) > 0) {
			cCsConsoleCommandEvent * p_event = new cCsConsoleCommandEvent;
			p_event->Init(input);
		} else {
		   Print(Get_Help());
		}
	}
};


class MainMenuConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "main_menu"; }
	virtual	const char * Get_Alias( void )	{ return "mm"; }
	virtual	const char * Get_Help( void )	{ return "MAIN_MENU - go back to the main menu."; }
	virtual	void Activate( const char * input ) {
		cDevOptions::GoToMainMenu.Set(true);
		cDevOptions::QuickFullExit.Set(true);
	}
};


class MaxFacingPenaltyConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "max_facing_penalty"; }
	virtual	const char * Get_Alias( void ){ return "mfp"; }
	virtual	const char * Get_Help( void )	{ return "MAX_FACING_PENALTY - set the max facing penalty. Server only. Float. Use zero to disable."; }
	virtual	void Activate( const char * input ) {
      WWASSERT(input != NULL);
		if (cNetwork::I_Am_Server()) {
			float f = ::atof(input);
			cUserOptions::MaxFacingPenalty.Set(f);
         Print( "MaxFacingPenalty set to %5.2f.", f);
		} else {
		   Print(Get_Help());
		}
	}
};


class IrrelevancePenaltyConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "irrelevance_penalty"; }
	//virtual	const char * Get_Alias( void ){ return ""; }
	virtual	const char * Get_Help( void )	{ return "IRRELEVANCE_PENALTY - set the irrelevance penalty. Server only. Float. Use zero to disable."; }
	virtual	void Activate( const char * input ) {
      WWASSERT(input != NULL);
		if (cNetwork::I_Am_Server()) {
			float f = ::atof(input);
			cUserOptions::IrrelevancePenalty.Set(f);
         Print( "IrrelevancePenalty set to %5.2f.", f);
		} else {
		   Print(Get_Help());
		}
	}
};


class MissionConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual const char * Get_Name(void) {return "mission";}
	virtual const char * Get_Help(void) {return "MISSION <success/failure> end mission with given status.";}
	virtual void Activate(const char * input) {
		if (!stricmp(input, "success")) {
			CombatManager::Mission_Complete(true);
		} else if (!stricmp(input, "failure")) {
			CombatManager::Mission_Complete(false);
		} else {
		   Print(Get_Help());
		}
	}
};


class MoneyConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual const char * Get_Name(void) {return "money";}
	virtual const char * Get_Help(void) {return "MONEY <amount> - money cheat, for gods only.";}

	virtual void Activate(const char * input) {

		int amount = ::atoi(input);

		if (cNetwork::I_Am_God()) {
			cMoneyEvent * p_money = new cMoneyEvent;
			p_money->Init(amount);
		} else {
		   Print(Get_Help());
		}
	}
};


class ScoreConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual const char * Get_Name(void) {return "score";}
	virtual const char * Get_Help(void) {return "SCORE <increment> - score cheat, for gods only.";}

	virtual void Activate(const char * input) {

		int amount = ::atoi(input);

		if (cNetwork::I_Am_God()) {
			cScoreEvent * p_score = new cScoreEvent;
			p_score->Init(amount);
		} else {
		   Print(Get_Help());
		}
	}
};


class MultiHudConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "multihud"; }
	virtual	const char * Get_Help( void )	{ return "MULTIHUD - toggle the multiplayer HUD."; }
	virtual	void Activate( const char * input ) {
      CombatGameModeClass::Toggle_Multi_Hud();
      Print("MultiHud toggled\n");
	}
};


class SetPacketLossConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "set_packet_loss"; }
	virtual	const char * Get_Alias( void )	{ return "spl"; }
	virtual	const char * Get_Help( void )	{ return "SET_PACKET_LOSS <percentage> - simulate packetloss at given percentage"; }
	virtual	void Activate( const char * input ) {
      WWASSERT(input != NULL);
      double percentage;
      percentage = atof(input);
      if (percentage > -WWMATH_EPSILON && percentage < 100 + WWMATH_EPSILON) {
         cNetwork::Set_Simulated_Packet_Loss_Pc((int)percentage);
			ConsoleFunctionManager::Parse_Input("clear ns"); // reset stats
      } else {
		   Print(Get_Help());
      }
   }
};


class SetBandwidthBudgetOutToggleConsoleFunctionClass : public ConsoleFunctionClass {
public:
   virtual	const char * Get_Name( void )		{ return "toggle_sbbo"; }
	virtual	const char * Get_Help( void )		{ return "TOGGLE_SBBO - toggle cSbboManager (server only)."; }

	virtual	void Activate( const char * input ) {

		if (cNetwork::I_Am_Server()) {
			bool is_enabled = cSbboManager::Toggle_Is_Enabled();
			Print("cSbboManager %s enabled.\n", is_enabled ? "IS" : "is NOT");
		} else {
			Print(Get_Help());
		}
	}
};



class SetSendFrequencyConsoleFunctionClass : public ConsoleFunctionClass {
public:
   virtual	const char * Get_Name( void )		{ return "send_frequency"; }
	virtual	const char * Get_Help( void )		{ return "send_freqency <delay> - set delay between packet sends."; }

	virtual	void Activate( const char * input ) {
      WWASSERT(input != NULL);
      int delay = atoi(input);
		PacketManager.Set_Flush_Frequency((unsigned long) delay);
		if (delay) {
			Print("Packets will be sent no more than %d times per second.\n", 1000 / delay);
		} else {
			Print("Packets will be sent immediately.\n");
		}
	}
};


class TogglePacketDeltasConsoleFunctionClass : public ConsoleFunctionClass {
public:
   virtual	const char * Get_Name( void )		{ return "packet_delta"; }
	virtual	const char * Get_Help( void )		{ return "packet_delta - toggle packet delta enable flag."; }

	virtual	void Activate( const char * ) {
		bool delta = PacketManager.Toggle_Allow_Deltas();

		if (delta) {
			Print("Packets can be sent as deltas.\n");
		} else {
			Print("Packets are not allowed to be sent as deltas.\n");
		}
	}
};



class TogglePacketComboConsoleFunctionClass : public ConsoleFunctionClass {
public:
   virtual	const char * Get_Name( void )		{ return "packet_combo"; }
	virtual	const char * Get_Help( void )		{ return "packet_combo - toggle packet packing."; }

	virtual	void Activate( const char * ) {
		bool combos = PacketManager.Toggle_Allow_Combos();

		if (combos) {
			Print("Packets of different sizes can be combined into one packet.\n");
		} else {
			Print("Packets of different sizes cannot be combined into one packet.\n");
		}
	}
};



class TogglePacketOptConsoleFunctionClass : public ConsoleFunctionClass {
public:
   virtual	const char * Get_Name( void )		{ return "packet_opt"; }
	virtual	const char * Get_Help( void )		{ return "packet_opt <state> - enable low level packet optimizations."; }

	virtual	void Activate( const char * input ) {
      WWASSERT(input != NULL);
      int on = atoi(input);

		if (on) {
			PacketManager.Set_Flush_Frequency(1000 / 15);
			bool ok = PacketManager.Toggle_Allow_Deltas();
			if (!ok) {
				PacketManager.Toggle_Allow_Deltas();
			}
			ok = PacketManager.Toggle_Allow_Combos();
			if (!ok) {
				PacketManager.Toggle_Allow_Combos();
			}
			Print("Low level packet optioizations enabled.\n");
		} else {
			PacketManager.Set_Flush_Frequency(0);
			bool ok = PacketManager.Toggle_Allow_Deltas();
			if (ok) {
				PacketManager.Toggle_Allow_Deltas();
			}
			ok = PacketManager.Toggle_Allow_Combos();
			if (ok) {
				PacketManager.Toggle_Allow_Combos();
			}
			Print("Low level packet optioizations disabled.\n");
		}
	}
};



class SetLatencyConsoleFunctionClass : public ConsoleFunctionClass {
public:
   virtual	const char * Get_Name( void )		{ return "latency"; }
	virtual	const char * Get_Help( void )		{ return "latency <low,high> - set simulated latency on packets."; }

	virtual	void Activate( const char * input ) {
      WWASSERT(input != NULL);
		int low,high,current;
		int fields = sscanf(input, "%d,%d", &low, &high);
		if (fields == 1) {
			high = low;
			fields = 2;
		}
		if (fields == 2) {
			cConnection::Set_Latency(low, high);
			Print("Latency set\n");
		}

		cConnection::Get_Latency(low, high, current);
		Print("Latency set to %d,%d (current latency is %d ms)\n", low, high, current);
	}
};



class ToggleBandwidthBalancerConsoleFunctionClass : public ConsoleFunctionClass {
public:
   virtual	const char * Get_Name( void )		{ return "bw_balance"; }
	virtual	const char * Get_Help( void )		{ return "bw_balance - toggle bandwidth balancer code enable."; }

	virtual	void Activate( const char * ) {
		if (BandwidthBalancer.IsEnabled == false) {
			BandwidthBalancer.IsEnabled = true;
			Print("Using bandwidth balancer.\n");
		} else {
			BandwidthBalancer.IsEnabled = false;
			Print("Not using bandwidth balancer.\n");
		}
	}
};


class ToggleNewClientUpdateMethodConsoleFunctionClass : public ConsoleFunctionClass {
public:
   virtual	const char * Get_Name( void )		{ return "new_update"; }
	virtual	const char * Get_Help( void )		{ return "new_update - toggle new client update method."; }

	virtual	void Activate( const char * ) {
		if (cNetwork::SensibleUpdates == false) {
			cNetwork::SensibleUpdates = true;
			Print("Using new per object per client update scheme.\n");
		} else {
			cNetwork::SensibleUpdates = false;
			Print("Using old funky distance based update scheme.\n");
		}
	}
};



class SetDesiredSleepConsoleFunctionClass : public ConsoleFunctionClass {
public:
   virtual	const char * Get_Name( void )	{ return "set_desired_sleep"; }
	virtual	const char * Get_Alias( void )	{ return "sds"; }
	virtual	const char * Get_Help( void )	{ return "SET_DESIRED_SLEEP <sleep_ms> - sleep this many ms per frame."; }
	virtual	void Activate( const char * input ) {
      WWASSERT(input != NULL);
      int desired_sleep = atoi(input);
		if (desired_sleep >= 0) {
         cNetwork::Set_Desired_Frame_Sleep_Ms(desired_sleep);
         Print( "Frame sleep set to %d ms.", desired_sleep );
		} else {
		   Print(Get_Help());
		}
	}
};

class SetPacketDuplicationConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "set_packet_duplication"; }
	//virtual	const char * Get_Alias( void )	{ return "spd"; }
	virtual	const char * Get_Help( void )	{ return "SET_PACKET_DUPLICATION <percentage> - simulate packet duplication at given percentage"; }
	virtual	void Activate( const char * input ) {
      WWASSERT(input != NULL);
      double percentage = atof(input);
      if (percentage > -WWMATH_EPSILON && percentage < 100 + WWMATH_EPSILON) {
         cNetwork::Set_Simulated_Packet_Duplication_Pc((int)percentage);
			ConsoleFunctionManager::Parse_Input("clear ns"); // reset stats
      } else {
		   Print(Get_Help());
      }
   }
};


class SetPacketLatencyRangeConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "set_packet_latency_range"; }
	virtual	const char * Get_Alias( void )	{ return "latency"; }
	virtual	const char * Get_Help( void )	{ return "SET_PACKET_LATENCY_RANGE <min> <max> - sets a range for simulated latency (in ms)"; }
	virtual	void Activate( const char * input ) {
      WWASSERT(input != NULL);
      int min_latency_ms;
      int max_latency_ms;
      int num_args = sscanf(input, "%d %d", &min_latency_ms, &max_latency_ms);

      if (num_args == 2 && min_latency_ms >= 0 && max_latency_ms >= 0 &&
         min_latency_ms <= max_latency_ms) {
         cNetwork::Set_Simulated_Latency_Range_Ms(min_latency_ms, max_latency_ms);
      } else {
		   Print(Get_Help());
      }
	}
};


class SetSpamCountConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "set_spam_count"; }
	virtual	const char * Get_Alias( void )	{ return "ssc"; }
	virtual	const char * Get_Help( void )	{ return "SET_SPAM_COUNT <count> - S->C state packet is sent this many extra times."; }
	virtual	void Activate( const char * input ) {
      WWASSERT(input != NULL);
      int spam_count = atoi(input);
		if (spam_count >= 0) {
         cNetwork::Set_Spam_Count(spam_count);
         //Print( "Spam Count set to %d ms.", desired_sleep );
		} else {
		   Print(Get_Help());
		}
	}
};


class ClearConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "clear"; }
	virtual	const char * Get_Help( void )	{ return "CLEAR <object> - clear specified object."; }
	virtual	void Activate( const char * input ) {

		if (cMiscUtil::Is_String_Same(input, "isc")) {
		   NetworkObjectMgrClass::Reset_Import_State_Counts();
		} else if (cMiscUtil::Is_String_Same(input, "netstats") ||
			cMiscUtil::Is_String_Same(input, "ns")) {
		   if (cNetwork::PClientConnection != NULL) {
			   cNetwork::PClientConnection->Init_Stats();
			   Print("Network stats for client connection reset.\n");
		   }
		   if (cNetwork::PServerConnection != NULL) {
			   cNetwork::PServerConnection->Init_Stats();
			   Print("Network stats for server connection reset.\n");
		   }
      } else if (cMiscUtil::Is_String_Same(input, "packet_simulation") ||
			cMiscUtil::Is_String_Same(input, "ps")) {
         ConsoleFunctionManager::Parse_Input("spl 0");
         ConsoleFunctionManager::Parse_Input("spd 0");
         ConsoleFunctionManager::Parse_Input("latency 0 0");
		} else if (cMiscUtil::Is_String_Same(input, "0")) {
			ConsoleFunctionManager::Parse_Input("clear ht");
			ConsoleFunctionManager::Parse_Input("clear isc");
			ConsoleFunctionManager::Parse_Input("clear ns");
			ConsoleFunctionManager::Parse_Input("clear ps");
		} else if (cMiscUtil::Is_String_Same(input, "")) {

         Print("Valid clear objects:\n");
         Print("     help_text - clears any help text\n");
         Print("     isc - reset the import state count of all smart game objects\n");
         Print("     netstats - reinitialize net stats\n");
         Print("     packet_simulation - turns off packet simulation (loss, duplication, latency)\n");

      } else {
			Print("Error: no such clear object.\n");
		}
	}
};


class ClientHintFactorConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "client_hint_factor"; }
	virtual	const char * Get_Alias( void ){ return "chf"; }
	virtual	const char * Get_Help( void )	{ return "CLIENT_HINT_FACTOR - set the client hint factor (worst:average). Client only. Float. Use zero to disable."; }
	virtual	void Activate( const char * input ) {
      WWASSERT(input != NULL);
		if (cNetwork::I_Am_Only_Client()) {
			float chf = ::atof(input);
			cUserOptions::ClientHintFactor.Set(chf);
         Print( "ClientHintFactor set to %5.2f.", chf);
		} else {
		   Print(Get_Help());
		}
	}
};


class CreateBotConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "create_bot"; }
	virtual	const char * Get_Alias( void )	{ return "cb"; }
	virtual	const char * Get_Help( void )	{ return "CREATE_BOT <number> - create specified number of bots (max 20, server only)."; }
	virtual	void Activate( const char * input ) {
      if (cNetwork::I_Am_Server()) {
			int num_bots = 1;
			if (strcmp(input, "")) {
				num_bots = atoi(input);
			}
			if (num_bots < 0) {
				num_bots = 0;
			}
			if (num_bots > 20) {
				num_bots = 20;
			}
			for (int i = 0; i < num_bots; i++) {
				WWASSERT(PTheGameData != NULL);
				if (cPlayerManager::Count() < The_Game()->Get_Max_Players()) {
					cGod::Create_Ai_Player();
				}
			}
		} else {
		   Print(Get_Help());
		}
	}
};


class CreateGruntConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "create_grunt"; }
	virtual	const char * Get_Alias( void )	{ return "cg"; }
	virtual	const char * Get_Help( void )	{ return "CREATE_GRUNT <number> - create specified number of grunts (server only, max 20)."; }
	virtual	void Activate( const char * input ) {
      if ( cNetwork::I_Am_Server()) {
			int num_grunts = 1;
			if (strcmp(input, "")) {
				num_grunts = atoi(input);
			}
			if (num_grunts < 0) {
				num_grunts = 0;
			}
			if (num_grunts > 20) {
				num_grunts = 20;
			}

			if (COMBAT_STAR != NULL) {
				Vector3 star_pos;
				COMBAT_STAR->Get_Position(&star_pos);
				Vector3 star_target_pos = COMBAT_STAR->Get_Targeting_Pos();
				//star_pos.Z = 0;
				//star_target_pos.Z = 0;
				star_target_pos.Z = star_pos.Z;
				Vector3 look_vector = star_target_pos - star_pos;
				look_vector.Normalize();
				Vector3 pos = star_pos + 20 * look_vector;
				pos.Z += 0.5;

				for (int i = 0; i < num_grunts; i++) {
					cGod::Create_Grunt(pos);
				}
			}

		} else {
		   Print(Get_Help());
		}
	}
};


class CreateObjectConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "create_object"; }
	virtual	const char * Get_Alias( void )	{ return "co"; }
	virtual	const char * Get_Help( void )	{ return "CREATE_OBJECT <name> - create the named object (server only). Use with caution."; }
	virtual	void Activate( const char * input ) {
      if (COMBAT_STAR && cNetwork::I_Am_Server()) {
         PhysicalGameObj * obj = ObjectLibraryManager::Create_Object(input);
			//if (obj != NULL && obj->As_SmartGameObj() != NULL) {
	      if (obj != NULL) {
            Vector3 position;
            COMBAT_STAR->Get_Position(&position);
	         position.X += 4;
	         //position.Z += 4;
		      Matrix3D tm(1);
		      tm.Set_Translation(position);
		      obj->Set_Transform(tm);
				obj->Start_Observers();
			} else {
		      Print("Failed to create %s.", input);
         }
		} else {
		   Print(Get_Help());
		}
	}
};


class ListConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "list"; }
	//virtual	const char * Get_Alias( void )	{ return "l"; }
	virtual	const char * Get_Help( void )	{ return "LIST <object> - show list of specified object."; }
	virtual	void Activate( const char * input ) {

		if (cMiscUtil::Is_String_Same(input, "active_game_modes") ||
			cMiscUtil::Is_String_Same(input, "agm")) {
		   GameModeManager::List_Active_Game_Modes();
		} else if (cMiscUtil::Is_String_Same(input, "definitions") ||
			cMiscUtil::Is_String_Same(input, "d")) {
		   DefinitionMgrClass::List_Available_Definitions(CLASSID_GAME_OBJECTS);
      } else if (cMiscUtil::Is_String_Same(input, "")) {

         Print("Valid list objects:\n");
         Print("     active_game_modes - list all active game modes\n");
         Print("     definitions - list all definition names\n");

      } else {
			Print("Error: no such list object.\n");
		}
	}
};


class BreakExecutionConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "break"; }
	virtual	const char * Get_Help( void )	{ return "BREAK - break execution. Do not use this just for fun."; }
	virtual	void Activate( const char * input ) {
		Print("Breaking execution on demand.\n");
		_asm int 0x03;
	}
};


class CrashExecutionConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "crash"; }
	virtual	const char * Get_Help( void )	{ return "CRASH - crash execution. Do not use this just for fun."; }
	virtual	void Activate( const char * input ) {
		Print("Bang....\n");
		char *zero = NULL;
		*zero = 0;
	}
};


class GiveWeaponsConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "give_weapons"; }
	virtual	const char * Get_Help( void )	{ return "GIVE_WEAPONS - give the commando all the weapons (single-player only)"; }
	virtual	void Activate( const char * input ) {

      if (cNetwork::I_Am_Server()) {
			if (IS_SOLOPLAY && COMBAT_STAR ) {
				COMBAT_STAR->Give_All_Weapons();
			} else {
				Print("Use FREE_WEAPONS instead in a multiplayer game.\n");
			}
		} else {
		   Print(Get_Help());
		}
	}
};


class GodConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "god"; }
	virtual	const char * Get_Help( void )	{ return "GOD - if you are a true believer, you may toggle your godlike status.\n"; }

	virtual	void Activate( const char * input ) {

		StringClass password;
		password.Format("%s", input);

      if (cNetwork::I_Am_Client() &&
			(!password.Is_Empty() || cDevOptions::IBelieveInGod.Is_True())) {

			cGodModeEvent * p_event = new cGodModeEvent;
			p_event->Init(password);

		} else {

		   Print(Get_Help());
		}
	}
};


class IBelieveInGodConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "ibelieveingod"; }
	virtual	const char * Get_Help( void )	{ return "IBELIEVEINGOD - toggle philosophical standpoint on this issue.\n"; }

	virtual	void Activate( const char * input ) {

		bool believe = cDevOptions::IBelieveInGod.Toggle();

		if (believe) {
			Print("You believe in god !\n");
		} else {
			Print("You do NOT believe in god !\n");

			if (cNetwork::I_Am_Server()) {
				//
				// Reduce any gods to mere mortals
				//
				for (
					SLNode<cPlayer> * player_node = cPlayerManager::Get_Player_Object_List()->Head();
					player_node;
					player_node = player_node->Next()) {

					cPlayer * p_player = player_node->Data();
					WWASSERT(p_player != NULL);

					if (p_player->Invulnerable.Is_True()) {
						p_player->Invulnerable.Set(false);
						p_player->Mark_As_Modified();
					}
				}
			}
		}
	}
};


class HideStarConsoleFunctionClass : public ConsoleFunctionClass
{
public:
	virtual	const char * Get_Name( void )	{ return "hide_star"; }
	virtual	const char * Get_Help( void )	{ return "HIDE_STAR - toggles display of the commando."; }
	virtual	void Activate( const char * input ) {
		if ( COMBAT_STAR ) {
			RenderObjClass	 *ro = COMBAT_STAR->Peek_Model();
			ro->Set_Hidden( !ro->Is_Hidden() );
		}
	}
};

class FlyStarConsoleFunctionClass : public ConsoleFunctionClass
{
public:
	virtual	const char * Get_Name( void )	{ return "fly_star"; }
	virtual	const char * Get_Help( void )	{ return "FLY_STAR - toggles fly mode for the commando."; }
	virtual	void Activate( const char * input ) {
		if ( COMBAT_STAR != NULL && (IS_SOLOPLAY || cNetwork::I_Am_God() )) {
			COMBAT_STAR->Toggle_Fly_Mode();
		}
	}
};

class WatchConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "watch"; }
	//virtual	const char * Get_Alias( void )	{ return "w"; }
	virtual	const char * Get_Help( void )	{ return "WATCH <object> - show debug output of specified type."; }
   static   void         Get_Verbose_Help(char * buffer, int bufsize) {
      WWASSERT(buffer != NULL);
      WWASSERT(bufsize >= 1000);

      buffer[0] = 0;
      strcat(buffer, "\n");
      strcat(buffer, "Valid watch objects:\n");
      strcat(buffer, "     bandwidth               (bw)\n");
      strcat(buffer, "     bandwidth_budget_out    (bbo)\n");
//strcat(buffer, "     bounty                  \n");
      strcat(buffer, "     building_data           (bd)\n");
      strcat(buffer, "     client_fps					(client_fps)\n");
      strcat(buffer, "     client_rhost_data       (crd)\n");
      strcat(buffer, "     diagnostics             (diag)\n");
      strcat(buffer, "     door_data               (dd)\n");
      strcat(buffer, "     DSAPO_data              (DSAPO_data)\n");
      strcat(buffer, "     elevator_data           (ed)\n");
      strcat(buffer, "     fps                     (fps)\n");
      strcat(buffer, "     god                     (god)\n");
      strcat(buffer, "     import_states_count     (isc)\n");
      strcat(buffer, "     ids                     (ids)\n");
      strcat(buffer, "     ip_addresses            (ip)\n");
      strcat(buffer, "     last_contact            (lc)\n");
      strcat(buffer, "     latency                 (lat)\n");
      strcat(buffer, "     list_size               (ls)\n");
      strcat(buffer, "     list_packet_size        (lps)\n");
      strcat(buffer, "     list_time               (lt)\n");
//strcat(buffer, "     menu_stack              (ms)\n");
      strcat(buffer, "     money							(money)\n");
      strcat(buffer, "     names                   (names)\n");
      strcat(buffer, "     object_tally            (ot)\n");
      strcat(buffer, "     packets                 (p)\n");
      strcat(buffer, "     ping							(ping)\n");
      strcat(buffer, "     powerup_data				(pd)\n");
      strcat(buffer, "     priorities              (pri)\n");
      strcat(buffer, "     server_rhost_data       (srd)\n");
      strcat(buffer, "     soldier_data            (sd)\n");
      strcat(buffer, "     spawner_data            (spawn)\n");
      strcat(buffer, "     vehicle_data            (vd)\n");
      strcat(buffer, "     wol							(wol)\n");
      strcat(buffer, "     wol_location            (wl)\n");

      strcat(buffer, "     PacketsSentServer						(pss)\n");
      strcat(buffer, "     PacketsSentClient						(psc)\n");
      strcat(buffer, "     PacketsRecdServer						(prs)\n");
      strcat(buffer, "     PacketsRecdClient						(prc)\n");
      strcat(buffer, "     AvgSizePacketsSentServer			(aspss)\n");
      strcat(buffer, "     AvgSizePacketsSentClient			(aspsc)\n");
      strcat(buffer, "     AvgSizePacketsRecdServer			(asprs)\n");
      strcat(buffer, "     AvgSizePacketsRecdClient			(asprc)\n");
      strcat(buffer, "     BytesSentServer						(bss)\n");
      strcat(buffer, "     BytesSentClient						(bsc)\n");
      strcat(buffer, "     BytesRecdServer						(brs)\n");
      strcat(buffer, "     BytesRecdClient						(brc)\n");

      strcat(buffer, "     WwnetPacketsSentServer				(wpss)\n");
      strcat(buffer, "     WwnetPacketsSentClient				(wpsc)\n");
      strcat(buffer, "     WwnetPacketsRecdServer				(wprs)\n");
      strcat(buffer, "     WwnetPacketsRecdClient				(wprc)\n");
      strcat(buffer, "     WwnetAvgSizePacketsSentServer		(waspss)\n");
      strcat(buffer, "     WwnetAvgSizePacketsSentClient		(waspsc)\n");
      strcat(buffer, "     WwnetAvgSizePacketsRecdServer		(wasprs)\n");
      strcat(buffer, "     WwnetAvgSizePacketsRecdClient		(wasprc)\n");
      strcat(buffer, "     WwnetBytesSentServer					(wbss)\n");
      strcat(buffer, "     WwnetBytesSentClient					(wbsc)\n");
      strcat(buffer, "     WwnetBytesRecdServer					(wbrs)\n");
      strcat(buffer, "     WwnetBytesRecdClient					(wbrc)\n");

   }
	virtual	void Activate( const char * input ) {

		if (cMiscUtil::Is_String_Same(input, "bandwidth") ||
			cMiscUtil::Is_String_Same(input, "bw")) {
			cDevOptions::ShowBandwidth.Toggle();
      //} else if (cMiscUtil::Is_String_Same(input, "bandwidth_budget_out") ||
		//	cMiscUtil::Is_String_Same(input, "bbo")) {
		//	cDevOptions::ShowBandwidthBudgetOut.Toggle();
		} else if (cMiscUtil::Is_String_Same(input, "client_fps")) {
			cDevOptions::ShowClientFps.Toggle();
		} else if (cMiscUtil::Is_String_Same(input, "client_rhost_data") ||
			cMiscUtil::Is_String_Same(input, "crd")) {
			cDevOptions::ShowClientRhostData.Toggle();
		} else if (cMiscUtil::Is_String_Same(input, "diagnostics") ||
			cMiscUtil::Is_String_Same(input, "diag")) {
			cDevOptions::ShowDiagnostics.Toggle();
		} else if (cMiscUtil::Is_String_Same(input, "gs_auth")) {
			cDevOptions::ShowGameSpyAuthState.Toggle();
		} else if (cMiscUtil::Is_String_Same(input, "ids")) {
			cDevOptions::ShowId.Toggle();
		} else if (cMiscUtil::Is_String_Same(input, "money")) {
			cDevOptions::ShowMoney.Toggle();
		} else if (cMiscUtil::Is_String_Same(input, "ping")) {
			cDevOptions::ShowPing.Toggle();
		} else if (cMiscUtil::Is_String_Same(input, "ip_addresses") ||
			cMiscUtil::Is_String_Same(input, "ip")) {
			cDevOptions::ShowIpAddresses.Toggle();
		} else if (cMiscUtil::Is_String_Same(input, "fps")) {
			cDevOptions::ShowFps.Toggle();
		} else if (cMiscUtil::Is_String_Same(input, "god")) {
			cDevOptions::ShowGodStatus.Toggle();
      } else if (cMiscUtil::Is_String_Same(input, "import_states_count") ||
			cMiscUtil::Is_String_Same(input, "isc")) {
			bool is_shown = cDevOptions::ShowImportStates.Toggle();
			if (is_shown) {
				Print("You may use RESET_ISC to reset the counts.\n");
			}
      } else if (cMiscUtil::Is_String_Same(input, "import_states_count_soldiers_vehicles") ||
			cMiscUtil::Is_String_Same(input, "iscsv")) {
			bool is_shown = cDevOptions::ShowImportStatesSV.Toggle();
			if (is_shown) {
				Print("You may use RESET_ISC to reset the counts.\n");
			}
		} else if (cMiscUtil::Is_String_Same(input, "latency") ||
			cMiscUtil::Is_String_Same(input, "lat")) {
			ConsoleFunctionManager::Parse_Input("clear ns"); // reset stats
			cDevOptions::ShowLatency.Toggle();
		} else if (cMiscUtil::Is_String_Same(input, "last_contact") ||
			cMiscUtil::Is_String_Same(input, "lc")) {
			//ConsoleFunctionManager::Parse_Input("clear ns"); // reset stats
			cDevOptions::ShowLastContact.Toggle();
		} else if (cMiscUtil::Is_String_Same(input, "list_size") ||
			cMiscUtil::Is_String_Same(input, "ls")) {
			cDevOptions::ShowListSizes.Toggle();
		} else if (cMiscUtil::Is_String_Same(input, "list_packet_size") ||
			cMiscUtil::Is_String_Same(input, "lps")) {
			cDevOptions::ShowListPacketSizes.Toggle();
		} else if (cMiscUtil::Is_String_Same(input, "list_time") ||
			cMiscUtil::Is_String_Same(input, "lt")) {
			cDevOptions::ShowListTimes.Toggle();
		/*
		} else if (cMiscUtil::Is_String_Same(input, "menu_stack") ||
			cMiscUtil::Is_String_Same(input, "ms")) {
			cDevOptions::ShowMenuStack.Toggle();
		*/
		} else if (cMiscUtil::Is_String_Same(input, "names")) {
         if (IS_MISSION) {
            Print("Name showing is for multiplayer only.\n");
         } else {
            bool on = cUserOptions::ShowNamesOnSoldier.Toggle();
            if (on) {
               Print("Name showing is now ON\n");
            } else {
               Print("Name showing is now OFF\n");
            }
         }
		} else if (cMiscUtil::Is_String_Same(input, "object_tally") ||
			cMiscUtil::Is_String_Same(input, "ot")) {
			cDevOptions::ShowObjectTally.Toggle();
		} else if (cMiscUtil::Is_String_Same(input, "inactive")) {
			cDevOptions::ShowInactivePlayers.Toggle();
      } else if (cMiscUtil::Is_String_Same(input, "PacketsSentServer")			|| cMiscUtil::Is_String_Same(input, "pss")) {
			cDevOptions::PacketsSentServer.Toggle();
      } else if (cMiscUtil::Is_String_Same(input, "PacketsSentClient")			|| cMiscUtil::Is_String_Same(input, "psc")) {
			cDevOptions::PacketsSentClient.Toggle();
      } else if (cMiscUtil::Is_String_Same(input, "PacketsRecdServer")			|| cMiscUtil::Is_String_Same(input, "prs")) {
			cDevOptions::PacketsRecdServer.Toggle();
      } else if (cMiscUtil::Is_String_Same(input, "PacketsRecdClient")			|| cMiscUtil::Is_String_Same(input, "prc")) {
			cDevOptions::PacketsRecdClient.Toggle();
      } else if (cMiscUtil::Is_String_Same(input, "AvgSizePacketsSentServer")	|| cMiscUtil::Is_String_Same(input, "aspss")) {
			cDevOptions::AvgSizePacketsSentServer.Toggle();
      } else if (cMiscUtil::Is_String_Same(input, "AvgSizePacketsSentClient") || cMiscUtil::Is_String_Same(input, "aspsc")) {
			cDevOptions::AvgSizePacketsSentClient.Toggle();
      } else if (cMiscUtil::Is_String_Same(input, "AvgSizePacketsRecdServer") || cMiscUtil::Is_String_Same(input, "asprs")) {
			cDevOptions::AvgSizePacketsRecdServer.Toggle();
      } else if (cMiscUtil::Is_String_Same(input, "AvgSizePacketsRecdClient") || cMiscUtil::Is_String_Same(input, "asprc")) {
			cDevOptions::AvgSizePacketsRecdClient.Toggle();
      } else if (cMiscUtil::Is_String_Same(input, "BytesSentServer")				|| cMiscUtil::Is_String_Same(input, "bss")) {
			cDevOptions::BytesSentServer.Toggle();
      } else if (cMiscUtil::Is_String_Same(input, "BytesSentClient")				|| cMiscUtil::Is_String_Same(input, "bsc")) {
			cDevOptions::BytesSentClient.Toggle();
      } else if (cMiscUtil::Is_String_Same(input, "BytesRecdServer")				|| cMiscUtil::Is_String_Same(input, "brs")) {
			cDevOptions::BytesRecdServer.Toggle();
      } else if (cMiscUtil::Is_String_Same(input, "BytesRecdClient")				|| cMiscUtil::Is_String_Same(input, "brc")) {
			cDevOptions::BytesRecdClient.Toggle();

      } else if (cMiscUtil::Is_String_Same(input, "WwnetPacketsSentServer")			|| cMiscUtil::Is_String_Same(input, "wpss")) {
			cDevOptions::WwnetPacketsSentServer.Toggle();
      } else if (cMiscUtil::Is_String_Same(input, "WwnetPacketsSentClient")			|| cMiscUtil::Is_String_Same(input, "wpsc")) {
			cDevOptions::WwnetPacketsSentClient.Toggle();
      } else if (cMiscUtil::Is_String_Same(input, "WwnetPacketsRecdServer")			|| cMiscUtil::Is_String_Same(input, "wprs")) {
			cDevOptions::WwnetPacketsRecdServer.Toggle();
      } else if (cMiscUtil::Is_String_Same(input, "WwnetPacketsRecdClient")			|| cMiscUtil::Is_String_Same(input, "wprc")) {
			cDevOptions::WwnetPacketsRecdClient.Toggle();
      } else if (cMiscUtil::Is_String_Same(input, "WwnetAvgSizePacketsSentServer")	|| cMiscUtil::Is_String_Same(input, "waspss")) {
			cDevOptions::WwnetAvgSizePacketsSentServer.Toggle();
      } else if (cMiscUtil::Is_String_Same(input, "WwnetAvgSizePacketsSentClient")  || cMiscUtil::Is_String_Same(input, "waspsc")) {
			cDevOptions::WwnetAvgSizePacketsSentClient.Toggle();
      } else if (cMiscUtil::Is_String_Same(input, "WwnetAvgSizePacketsRecdServer")  || cMiscUtil::Is_String_Same(input, "wasprs")) {
			cDevOptions::WwnetAvgSizePacketsRecdServer.Toggle();
      } else if (cMiscUtil::Is_String_Same(input, "WwnetAvgSizePacketsRecdClient")  || cMiscUtil::Is_String_Same(input, "wasprc")) {
			cDevOptions::WwnetAvgSizePacketsRecdClient.Toggle();
      } else if (cMiscUtil::Is_String_Same(input, "WwnetBytesSentServer")				|| cMiscUtil::Is_String_Same(input, "wbss")) {
			cDevOptions::WwnetBytesSentServer.Toggle();
      } else if (cMiscUtil::Is_String_Same(input, "WwnetBytesSentClient")				|| cMiscUtil::Is_String_Same(input, "wbsc")) {
			cDevOptions::WwnetBytesSentClient.Toggle();
      } else if (cMiscUtil::Is_String_Same(input, "WwnetBytesRecdServer")				|| cMiscUtil::Is_String_Same(input, "wbrs")) {
			cDevOptions::WwnetBytesRecdServer.Toggle();
      } else if (cMiscUtil::Is_String_Same(input, "WwnetBytesRecdClient")				|| cMiscUtil::Is_String_Same(input, "wbrc")) {
			cDevOptions::WwnetBytesRecdClient.Toggle();

      } else if (cMiscUtil::Is_String_Same(input, "priorities") ||
			cMiscUtil::Is_String_Same(input, "pri")) {
			cDevOptions::ShowPriorities.Toggle();
		} else if (cMiscUtil::Is_String_Same(input, "server_rhost_data") ||
			cMiscUtil::Is_String_Same(input, "srd")) {
			cDevOptions::ShowServerRhostData.Toggle();
		} else if (cMiscUtil::Is_String_Same(input, "soldier_data") ||
			cMiscUtil::Is_String_Same(input, "sd")) {
			cDevOptions::ShowSoldierData.Toggle();
		} else if (cMiscUtil::Is_String_Same(input, "spawner_data") ||
			cMiscUtil::Is_String_Same(input, "spawn")) {
			cDevOptions::ShowSpawnerData.Toggle();
		} else if (cMiscUtil::Is_String_Same(input, "vehicle_data") ||
			cMiscUtil::Is_String_Same(input, "vd")) {
			cDevOptions::ShowVehicleData.Toggle();
		} else if (cMiscUtil::Is_String_Same(input, "door_data") ||
			cMiscUtil::Is_String_Same(input, "dd")) {
			cDevOptions::ShowDoorData.Toggle();
		} else if (cMiscUtil::Is_String_Same(input, "elevator_data") ||
			cMiscUtil::Is_String_Same(input, "ed")) {
			cDevOptions::ShowElevatorData.Toggle();
		} else if (cMiscUtil::Is_String_Same(input, "DSAPO_data") ||
			cMiscUtil::Is_String_Same(input, "DSAPO_data")) {
			cDevOptions::ShowDSAPOData.Toggle();
		} else if (cMiscUtil::Is_String_Same(input, "building_data") ||
			cMiscUtil::Is_String_Same(input, "bd")) {
			cDevOptions::ShowBuildingData.Toggle();
		} else if (cMiscUtil::Is_String_Same(input, "powerup_data") ||
			cMiscUtil::Is_String_Same(input, "pd")) {
			cDevOptions::ShowPowerupData.Toggle();
		} else if (cMiscUtil::Is_String_Same(input, "wol_location") ||
			cMiscUtil::Is_String_Same(input, "wl")) {
			cDevOptions::ShowWolLocation.Toggle();
		} else if (cMiscUtil::Is_String_Same(input, "0")) {
			//
			// Turn off any watch objects
			//
			cDevOptions::ShowBandwidth.Set(false);
			//cDevOptions::ShowBandwidthBudgetOut.Set(false);
			cDevOptions::ShowDiagnostics.Set(false);
         cDevOptions::ShowFps.Set(false);
         cDevOptions::ShowGodStatus.Set(false);
         //cDevOptions::ShowPacketGraphs.Set(false);
         cDevOptions::ShowPriorities.Set(false);
			cDevOptions::ShowLatency.Set(false);
			cDevOptions::ShowLastContact.Set(false);
			cDevOptions::ShowListSizes.Set(false);
			cDevOptions::ShowListTimes.Set(false);
			cDevOptions::ShowListPacketSizes.Set(false);
			cDevOptions::ShowMenuStack.Set(false);
			cDevOptions::ShowObjectTally.Set(false);
			cDevOptions::ShowInactivePlayers.Set(false);
         cDevOptions::ShowSoldierData.Set(false);
         cDevOptions::ShowSpawnerData.Set(false);
         cDevOptions::ShowVehicleData.Set(false);
         cDevOptions::ShowDoorData.Set(false);
         cDevOptions::ShowElevatorData.Set(false);
         cDevOptions::ShowDSAPOData.Set(false);
         cDevOptions::ShowBuildingData.Set(false);
         cDevOptions::ShowPowerupData.Set(false);
			cDevOptions::ShowImportStates.Set(false);
			cDevOptions::ShowImportStatesSV.Set(false);
			cDevOptions::ShowServerRhostData.Set(false);
			cDevOptions::ShowClientRhostData.Set(false);
			cDevOptions::ShowClientFps.Set(false);
			cDevOptions::ShowWolLocation.Set(false);
			cDevOptions::ShowPing.Set(false);
			cDevOptions::ShowMoney.Set(false);
			cDevOptions::ShowGameSpyAuthState.Set(false);

      } else if (cMiscUtil::Is_String_Same(input, "")) {

			cDevOptions::ShowWolLocation.Toggle();

			if (Get_Text_Display()) {
				if (cDevOptions::ShowWolLocation.Is_True())
				{
					char buffer[10000];
					Get_Verbose_Help(buffer, sizeof(buffer));
					Get_Text_Display()->Set_Verbose_Help_Text(buffer);
				}
				else
				{
					Get_Text_Display()->Set_Verbose_Help_Text("");
				}
			}

      } else {
			Print("Error: no such watch object.\n");
		}
	}
};


class SurfaceEffectsDebugConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "surface_effects_debug"; }
	virtual	const char * Get_Help( void )	{ return "SURFACE_EFFECTS_DEBUG - toggles display of surface effects debug info."; }
	virtual	void Activate( const char * input ) {
		SurfaceEffectsManager::Toggle_Debug();
		Print( "Surface Effects Debug Toggled\n" );
	}
};

class SurfaceTypeOverrideConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "surface_type_override"; }
	virtual	const char * Get_Help( void )	{ return "SURFACE_TYPE_OVERRIDE <int> - all surfaces forced to be given type (-1 disables)."; }
	virtual	void Activate( const char * input ) {
		int type = atoi( input );
		SurfaceEffectsManager::Set_Override_Surface_Type(type);;
		Print( "Override Surface Type Set: %d\n",type );
	}
};

class SurfacePhysicsOverrideConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "surface_physics_override"; }
	virtual	const char * Get_Help( void )	{ return "SURFACE_PHYSICS_OVERRIDE <friction> <drag> - force friction and drag for all surfaces (0.0 disables)."; }
	virtual	void Activate( const char * input ) {
		float friction,drag;
		if (sscanf(input, "%f %f", &friction, &drag) == 2) {
			PhysicsConstants::Set_Override_Surface_Friction(friction);
			PhysicsConstants::Set_Override_Surface_Drag(drag);
			Print( "Friction set to: %f  Drag set to: %f\n",friction,drag );
		} 	else if (sscanf(input, "%f", &friction) == 1) {
			PhysicsConstants::Set_Override_Surface_Friction(friction);
			Print( "Friction set to: %f \n",friction);
		}
	}
};


class SwapTeamsConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "swap_teams"; }
	virtual	const char * Get_Help( void )	{ return "SWAP_TEAMS - switch everyone to the opposite team (server only)."; }
	virtual	void Activate( const char * input ) {
		if (cNetwork::I_Am_Server()) {
			//if (The_Game() != NULL && The_Game()->Is_Team_Game()) {
			if (The_Game() != NULL) {
				for (
					SLNode<cPlayer> * player_node = cPlayerManager::Get_Player_Object_List()->Head();
					player_node;
					player_node = player_node->Next()) {

					cPlayer * p_player = player_node->Data();
					WWASSERT(p_player != NULL);

					int team = p_player->Get_Player_Type();

					WWASSERT(team == PLAYERTYPE_NOD || team == PLAYERTYPE_GDI);

					int new_team = PLAYERTYPE_NOD;
					if (team == PLAYERTYPE_NOD)
					{
						new_team = PLAYERTYPE_GDI;
					}
					else
					{
						new_team = PLAYERTYPE_NOD;
					}

					p_player->Set_Player_Type(new_team);

					SoldierGameObj * p_soldier = GameObjManager::Find_Soldier_Of_Client_ID(p_player->Get_Id());
					if (p_soldier != NULL)
					{
						p_soldier->Set_Delete_Pending();
					}
				}
			}
		} else {
			Print(Get_Help());
		}
	}
};


/*
**
*/
class SetStarHealthConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "set_star_health"; }
	virtual	const char * Get_Alias( void ){ return "ssh"; }
	virtual	const char * Get_Help( void )	{ return "SET_STAR_HEALTH <strength> - sets the strength of the star."; }
	virtual	void Activate( const char * input ) {
		if ( COMBAT_STAR ) {
			float health = atof( input );
			DefenseObjectClass * defense = COMBAT_STAR->Get_Defense_Object();
			WWASSERT(defense != NULL);
			if (health > defense->Get_Health_Max()) {
				defense->Set_Health_Max( health );
			}
			defense->Set_Health( health );
			Print( "Star Health set to %1.1f\n", defense->Get_Health() );
		}
	}
};

class SetStarSkinConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "set_star_skin"; }
	virtual	const char * Get_Help( void )	{ return "SET_STAR_SKIN <type> - sets the type of the star's skin."; }
	virtual	void Activate( const char * input ) {
		if ( COMBAT_STAR ) {
			DefenseObjectClass * defense = COMBAT_STAR->Get_Defense_Object();
			defense->Set_Skin( ArmorWarheadManager::Get_Armor_Type( input ) );
			Print( "Star Skin set to %s\n", ArmorWarheadManager::Get_Armor_Name( defense->Get_Skin() ) );
		}
	}
};

class SetStarShieldStrengthConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "set_star_shield_strength"; }
	virtual	const char * Get_Help( void )	{ return "SET_STAR_SHIELD_STRENGTH <strength> - sets the strength of the star's shield."; }
	virtual	void Activate( const char * input ) {
		if ( COMBAT_STAR ) {
			DefenseObjectClass * defense = COMBAT_STAR->Get_Defense_Object();
			defense->Set_Shield_Strength( atof( input ) );
			Print( "Star Shield Strength set to %1.1f\n", defense->Get_Shield_Strength() );
		}
	}
};

class SetStarShieldTypeConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "set_star_shield_type"; }
	virtual	const char * Get_Help( void )	{ return "SET_STAR_SHIELD_TYPE <type> - sets the type of the star's shield."; }
	virtual	void Activate( const char * input ) {
		if ( COMBAT_STAR ) {
			DefenseObjectClass * defense = COMBAT_STAR->Get_Defense_Object();
			defense->Set_Shield_Type( ArmorWarheadManager::Get_Armor_Type( input ) );
			Print( "Star Sheild_Type set to %s\n", ArmorWarheadManager::Get_Armor_Name( defense->Get_Shield_Type() ) );
		}
	}
};


class BuByeConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "bubye"; }
	virtual	const char * Get_Help( void )	{ return "BUBYE <mindist> - kills everything but the star if they are at least mindist away."; }
	virtual	void Activate( const char * input ) {
		Vector3	star_pos(0,0,0);
		if ( COMBAT_STAR ) {
			COMBAT_STAR->Get_Position( &star_pos );
		}
		float min_dist = atof( input );

		int count = 0;
		SLNode<SmartGameObj> *objnode;
		for (	objnode = GameObjManager::Get_Smart_Game_Obj_List()->Head(); objnode; objnode = objnode->Next() ) {
			if ( objnode->Data() != (SmartGameObj*)COMBAT_STAR ) {
				Vector3 obj_pos;
				objnode->Data()->Get_Position( &obj_pos );
				obj_pos -= star_pos;
				if ( obj_pos.Length() > min_dist ) {
					objnode->Data()->Set_Delete_Pending();
					count++;
				}
			}
		}
		Print( "%d others say \"Bye Now!\"\n", count );

	}
};


class KillConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual LPCSTR Get_Name(void)	{return "kill";}
	virtual LPCSTR Get_Help(void)	{return "KILL <object types> <percent=n> <id=xxxxx> - kills % of types (vehicles,soldiers,powerups,c4s,armeds,smarts,all). Server only";}
	virtual void Activate(LPCSTR input) {

		/*
		if (!cNetwork::I_Am_Server()) {
			Print(Get_Help());
			return;
		}
		*/

		if (strcmp(input, "")) {

			//
			// C & S can both use id=xxxxxx format
			//
			const char * p_id = ::strstr(input, "id=");
			if (p_id != NULL) {
				int id = -1;
				::sscanf(p_id, "id=%d", &id);
				if (cNetwork::I_Am_Client()) {
					cRequestKillEvent * p_event = new cRequestKillEvent;
					p_event->Init(id);
				} else {
					NetworkObjectClass *	p_object = NetworkObjectMgrClass::Find_Object(id);
					if (p_object != NULL) {
						p_object->Set_Delete_Pending();
					}
				}
				return;
			} else if (!cNetwork::I_Am_Server()) {
				Print(Get_Help());
				return;
			}

			//
			// Remaining functionality only for server.
			//

			const char * p_pc = strstr(input, "percent=");
			int percent = 100;
			if (p_pc != NULL) {
				sscanf(p_pc, "percent=%d", &percent);
				if (percent < 0) {
					percent = 0;
				} else if (percent > 100) {
					percent = 100;
				}
			}

			int count = 0;
			PhysicalGameObj * p_object = NULL;
			for (SLNode<BaseGameObj> * objnode = GameObjManager::Get_Game_Obj_List()->Head(); objnode != NULL; objnode = objnode->Next()) {

				p_object = objnode->Data()->As_PhysicalGameObj();
				if (p_object == NULL) {
					continue;
				}

				if (p_object->As_SmartGameObj() != NULL && p_object == COMBAT_STAR) {
					continue;
				}

				if ((strstr(input, "vehicles")	&& p_object->As_VehicleGameObj()		!= NULL) ||
					 (strstr(input, "soldiers")	&& p_object->As_SoldierGameObj()		!= NULL) ||
//					 (strstr(input, "commandos")	&& p_object->As_CommandoGameObj()	!= NULL) ||
					 (strstr(input, "powerups")	&& p_object->As_PowerUpGameObj()		!= NULL) ||
					 (strstr(input, "c4s")			&& p_object->As_C4GameObj()			!= NULL) ||
					 (strstr(input, "armeds")		&& p_object->As_ArmedGameObj()		!= NULL) ||
					 (strstr(input, "smarts")		&& p_object->As_SmartGameObj()		!= NULL) ||
					 (strstr(input, "all")) ||
					 (strstr(input, "-9"))) {

					if (rand() % 101 <= percent) {
						objnode->Data()->Set_Delete_Pending();
						count++;
					}
				}
			}

			Print("Killed a total of %d objects. Hope you're happy now.\n", count);

		} else {
			Print(Get_Help());
			Print("\n  Object types: vehicles, soldiers, commandos, powerups, c4s, armeds, smarts, all\n");
		}
	}
};


class SkeletonSliderDemoConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "skeleton_slider_demo"; }
	virtual	const char * Get_Help( void )	{ return "SKELETON_SLIDER_DEMO - toggles the Skeleton Slider Demo."; }
	virtual	void Activate( const char * input ) {
		CombatManager::Toggle_Skeleton_Slider_Demo();
		Print( "Skeleton Slider Demo Toggles\n" );
	}
};

class SkeletonStandersConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "skeleton_standers"; }
	virtual	const char * Get_Help( void )	{ return "SKELETON_STANDERS - creates 4 skeleton demos."; }
	virtual	void Activate( const char * input ) {

		const char * skel_names[ 4 ] = {
			"human_bt",
			"human_st",
			"human_bw",
			"human_sw",
		};

		for ( int i = 0; i < 4; i++ ) {
			PhysicalGameObjDef * def = (PhysicalGameObjDef *)DefinitionMgrClass::Find_Typed_Definition( "Commando", CLASSID_GAME_OBJECT_DEF_SOLDIER );
			WWASSERT( def );
			PhysicalGameObj * obj = (PhysicalGameObj *)def->Create();
			WWASSERT( obj );
			Matrix3D tm;
			tm.Set_Translation( Vector3( 0, i*1.2f, 0 ) );
			obj->Set_Transform( tm );
			HTreeClass *tree = WW3DAssetManager::Get_Instance()->Get_HTree( skel_names[i] );
			if ( tree ) {
				Animatable3DObjClass * robj = (Animatable3DObjClass *)obj->Peek_Model();
				robj->Set_HTree( tree );
			}
		}

		Print( "Skeleton Standers Created\n" );
	}
};

class GroupControllerTestConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "group_controller_test"; }
	virtual	const char * Get_Help( void )	{ return "GROUP_CONTROLLER_TEST - tests group controller code."; }
	virtual	void Activate( const char * input ) {

#if 0
		GroupControllerClass * group = new TestGroupControllerClass();

		for ( int i = 0; i < 3; i++) {
			SoldierGameObjDef * def = (SoldierGameObjDef *)DefinitionMgrClass::Find_Definition( "Nod Minigunner" );
			WWASSERT( def );
			SoldierGameObj * obj = (SoldierGameObj *)def->Create();
			WWASSERT( obj );
			obj->Set_Control_Owner( SmartGameObj::SERVER_CONTROL_OWNER );
			obj->Set_Player_Type( PLAYERTYPE_NOD );
			Vector3 pos( 3*i, 3*i, 1 );
			obj->Set_Position( pos );

			obj->Add_Observer( group );
//			group->Add_Unit( obj );
		}

		GroupControllerManager::Add_Controller( group );

		Print( "Group Controller Test Started\n" );
#else
		Print( "Group Controller Test BROKEN\n" );
#endif
	}
};


class StarVisibilityConsoleFunctionClass : public ConsoleFunctionClass
{
public:
	virtual	const char * Get_Name( void )	{ return "star_visibility"; }
	virtual	const char * Get_Help( void )	{ return "STAR_VISIBILITY - toggles the logical visibility of the star."; }
	virtual	void Activate( const char * input ) {
		if ( COMBAT_STAR ) {
			COMBAT_STAR->Set_Is_Visible( !COMBAT_STAR->Is_Visible() );
			Print( "Star is %svisible.\n", COMBAT_STAR->Is_Visible() ? "" : "in" );
		}
	}
};


/*
class StealthConsoleFunctionClass : public ConsoleFunctionClass
{
public:
	virtual	const char * Get_Name( void )	{ return "stealth"; }
	virtual	const char * Get_Help( void )	{ return "STEALTH - toggle stealth ability on star (client only)."; }
	virtual	void Activate( const char * input ) {
		//if (COMBAT_STAR != NULL) {
		//	COMBAT_STAR->Toggle_Stealth();
		//	Print("Star %s stealth ability\n", COMBAT_STAR->Is_Stealth_Enabled() ? "has" : "does NOT have");
		//}
      if (cNetwork::I_Am_Client()) {

			cPlayer * p_player = cNetwork::Get_My_Player_Object();

			if (p_player == NULL) {
				Print("Error: you don't have a player object.");
			} else {

				bool i_can_stealth = p_player->StealthEnabled.Toggle();

				Print("You %s stealth ability\n", i_can_stealth ? "have" : "do NOT have");

				if (cNetwork::I_Am_Only_Client())
				{
					cStealthEvent * p_event = new cStealthEvent;
					p_event->Init(p_player->StealthEnabled.Get());
				}
				else
				{
					p_player->Mark_As_Modified();
				}
			}
		} else {
		   Print("Error: you are not a client.");
		}
	}
};
*/


class ProfileConsoleFunctionClass : public ConsoleFunctionClass
{
public:
	virtual	const char * Get_Name( void )	{ return "profile"; }
	virtual	const char * Get_Help( void )	{ return "PROFILE - send a profile command (on,off,up,<#>)."; }
	virtual	void Activate( const char * input ) {
		Get_Console()->Profile_Command( input );
	}
};


class QuickStatsConsoleFunctionClass : public ConsoleFunctionClass
{
public:
	virtual	const char * Get_Name( void )	{ return "quick_stats"; }
	virtual	const char * Get_Alias( void ){ return "qs"; }
	virtual	const char * Get_Help( void )	{ return "QUICK_STATS - stats profile + profile on."; }
	virtual	void Activate( const char * input ) {
      ConsoleFunctionManager::Parse_Input("stats profile");
		Get_Console()->Profile_Command( "on" );
		Get_Console()->Profile_Command( "1" );
		Get_Console()->Profile_Command( "reset" );
		if (ConsoleBox.Is_Exclusive()) {
			ConsoleBox.Set_Profile_Mode(true);
		}
	}
};


class ProfileCollectBeginConsoleFunctionClass : public ConsoleFunctionClass
{
public:
	virtual	const char * Get_Name( void )	{ return "profile_collect_begin"; }
	virtual	const char * Get_Help( void )	{ return "PROFILE_COLLECT_BEGIN"; }
	virtual	void Activate( const char * input ) {
		WWProfileManager::Begin_Collecting();
	}
};

class ProfileCollectEndConsoleFunctionClass : public ConsoleFunctionClass
{
public:
	virtual	const char * Get_Name( void )	{ return "profile_collect_end"; }
	virtual	const char * Get_Help( void )	{ return "PROFILE_COLLECT_END"; }
	virtual	void Activate( const char * input ) {
		WWProfileManager::End_Collecting("profile_log.txt");
	}
};


class SetTheStarConsoleFunctionClass : public ConsoleFunctionClass
{
public:
	virtual	const char * Get_Name( void )	{ return "set_the_star"; }
	virtual	const char * Get_Help( void )	{ return "SET_THE_STAR <id> - set the star to a different object."; }
	virtual	void Activate( const char * input ) {
		SmartGameObj * star = GameObjManager::Find_SmartGameObj( atoi( input ) );
		if ( star != NULL && star->As_SoldierGameObj() != NULL ) {
			CombatManager::Set_The_Star( star->As_SoldierGameObj() );
		}
	}
};


class TeleportStarConsoleFunctionClass : public ConsoleFunctionClass
{
public:
	virtual	const char * Get_Name( void )	{ return "teleport_star"; }
	virtual	const char * Get_Help( void )	{ return "TELEPORT_STAR < x, y, z > - teleport the star."; }
	virtual	void Activate( const char * input ) {
		float x,y,z;
		::sscanf( input, "%f, %f, %f", &x, &y, &z );
		if ( COMBAT_STAR ) {
			COMBAT_STAR->Set_Position( Vector3( x,y,z ) );
			Print( "Teleported Star to %f %f %f\n", x,y,z );
		}
	}
};


class ScaleTimeConsoleFunctionClass : public ConsoleFunctionClass
{
public:
	virtual	const char * Get_Name( void )	{ return "scale_time"; }
	virtual	const char * Get_Help( void )	{ return "SCALE_TIME < factor > - scale time (SP only)."; }
	virtual	void Activate( const char * input ) {
		if (IS_SOLOPLAY) {
			float scale = atof( input );
			if ( scale <= 0 ) {
				scale = 1;
			}
			TimeManager::Set_Time_Scale( scale );
			Print( "TimeScaling %f\n", scale );
		} else {
			Print(Get_Help());
		}
	}
};

class ParticleToggleConsoleFunctionClass : public ConsoleFunctionClass
{
public:
	virtual	const char * Get_Name( void )	{ return "particle_toggle"; }
	virtual	const char * Get_Help( void )	{ return "PARTICLE_TOGGLE - toggles particle emission on and off."; }
	virtual	void Activate( const char * input ) {
		ParticleEmitterClass::Disable_All_Emitters(!ParticleEmitterClass::Are_Emitters_Disabled());
		if (ParticleEmitterClass::Are_Emitters_Disabled()) {
			Print("Emitters disabled\n");
		} else {
			Print("Emitters enabled\n");
		}
	}
};

class TargetSteeringToggleConsoleFunctionClass : public ConsoleFunctionClass
{
public:
	virtual	const char * Get_Name( void )	{ return "target_steering_toggle"; }
	virtual	const char * Get_Help( void )	{ return "TARGET_STEERING_TOGGLE - toggles vehicle targetsteering."; }
	virtual	void Activate( const char * input ) {
		Print( "Target Steering %s\n", VehicleGameObj::Toggle_Target_Steering() ? "ON" : "OFF" );
	}
};

class DazzleReloadConsoleFunctionClass : public ConsoleFunctionClass
{
public:
	virtual	const char * Get_Name( void )	{ return "dazzle_reload"; }
	virtual	const char * Get_Help( void )	{ return "DAZZLE_RELOAD - reloads the dazzle ini file (DANGEROUS!)."; }
	virtual	void Activate( const char * input ) {

		FileClass * dazzle_ini_file = _TheFileFactory->Get_File("DAZZLE.INI");
		if (dazzle_ini_file) {
			INIClass dazzle_ini(*dazzle_ini_file);
			DazzleRenderObjClass::Init_From_INI(&dazzle_ini);
			_TheFileFactory->Return_File(dazzle_ini_file);
		}
	}
};


/*
**
** Class to print out data safe stats
**
** ST - 7/18/2001 2:06PM
*/
class DataSafeStatsConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "safe_stats"; }
	virtual	const char * Get_Help( void )	{ return "SAFE_STATS - Show data safe statistics"; }
	virtual	void Activate( const char * input ) {
		static char dump_buffer[1024];
		GenericDataSafeClass::Dump_Safe_Stats(dump_buffer, 1024);
		Print(dump_buffer);
	}
};





#endif // WWDEBUG, development only commands

class ScreenUVBiasConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "screen_uv_bias"; }
	//virtual	const char * Get_Alias( void )	{ return "sub"; }
	virtual	const char * Get_Help( void )	{ return "SCREEN_UV_BIAS - toggles the half pixel bias in screen text."; }
	virtual	void Activate( const char * input ) {
		WW3D::Set_Screen_UV_Bias( !WW3D::Is_Screen_UV_Biased() );
      Print( "Screen UV Bias Switched\n" );
	}
};


class SetBandwidthBudgetOutConsoleFunctionClass : public ConsoleFunctionClass {
public:
   virtual	const char * Get_Name( void )		{ return "set_bw_budget_out"; }
	virtual	const char * Get_Alias( void )	{ return "sbbo"; }
	virtual	const char * Get_Help( void )		{ return "SET_BW_BUDGET_OUT <bps> - set total bps budget out."; }

	virtual	void Activate( const char * input ) {
      WWASSERT(input != NULL);
      ULONG bbo = (ULONG) ::atol(input);
		if (bbo > 0) {

      	if (cNetwork::I_Am_Server()) {
				cNetwork::PServerConnection->Set_Bandwidth_Budget_Out(bbo);
			}

      	if (cNetwork::I_Am_Client()) {
				cNetwork::PClientConnection->Set_Bandwidth_Budget_Out(bbo);

				//
				// Notify the server
				//
				cClientBboEvent * p_event = new cClientBboEvent;
				p_event->Init(bbo);
			}

			int bw_scale = (bbo * 2) / 10;
			bw_scale = (bw_scale / 1000) * 1000;
			cBandwidthGraph::Set_Scale(bw_scale);

		   if (cNetwork::PClientConnection != NULL) {
			   cNetwork::PClientConnection->Init_Stats();
			   Print("Network stats for client connection reset.\n");
		   }
		   if (cNetwork::PServerConnection != NULL) {
			   cNetwork::PServerConnection->Init_Stats();
			   Print("Network stats for server connection reset.\n");
		   }
			//ConsoleFunctionManager::Parse_Input("clear ns"); // reset stats
		}
	}
};



class GameOverConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "gameover"; }
	virtual	const char * Get_Help( void )	{ return "GAMEOVER - end current game (server only)."; }
	virtual	void Activate( const char * input ) {

		bool print_help = false;
      if (IS_MISSION) {
			//
			// Fail the mission
			//
#ifdef WWDEBUG
	      ConsoleFunctionManager::Parse_Input("mission failure");
#else //WWDEBUG
			print_help = true;
#endif //WWDEBUG
		} else if (!cNetwork::I_Am_Server()) {
			print_help = true;
      } else {
			if (cNetwork::I_Am_Only_Server()) {
         	Print("Terminating game on demand...\n");
         	cGameData::Set_Manual_Restart(true);
			} else {
#ifdef WWDEBUG
         	cGameData::Set_Manual_Restart(true);
#else //WWDEBUG
				print_help = true;
#endif //WWDEBUG
			}
      }
		if (print_help) {
			Print("GAMEOVER is for dedicated server only\n");
		}
	}
};


class ToggleSortingConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "toggle_sorting"; }
	virtual	const char * Get_Help( void )	{ return "TOGGLE_SORTING - toggles WW3D sorting."; }
	virtual	void Activate( const char * input ) {
		WW3D::Enable_Sorting(!WW3D::Is_Sorting_Enabled());
      Print( WW3D::Is_Sorting_Enabled() ? "Sorting Enabled\n" : "Sorting Disabled\n" );
	}
};

class ExtrasConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "extras"; }
	virtual	const char * Get_Help( void )	{ return "EXTRAS <key>"; }
	virtual	void Activate( const char * input ) {

#if 0
		//int crc = CRC_String("quantifigon");//not any more, pw changed...

		//WWDEBUG_SAY(("*E* %d\n", CRC_String(input)));

		//bool onoff = (CRC_String(input) == 0xa8452331);

		CNCPurchaseMainMenuClass::Enable_Secrets(false);  // Just to scramble code
		CNCPurchaseMainMenuClass::Enable_Secrets(false);  // Just to scramble code
		bool onoff = (CRC_String(input) == 65729409);
		CNCPurchaseMainMenuClass::Enable_Secrets(onoff);  // Just to scramble code

		if (The_Game()->IsLaddered.Is_True()) {
			onoff = false;
			CNCPurchaseMainMenuClass::Enable_Secrets(onoff);  // Just to scramble code
			Print("Not allowed in laddered games\r\n");
		}

		CNCPurchaseMainMenuClass::Enable_Secrets(onoff);
		if (CNCPurchaseMainMenuClass::Are_Secrets_Enabled()) {
			Print("extras ENABLED!\r\n");
		}
		CNCPurchaseMainMenuClass::Enable_Secrets(onoff);  // Just to scramble code
#else
		// (gth) secrets don't need a password any more...
		if (The_Game()->IsLaddered.Is_True()) {

			CNCPurchaseMainMenuClass::Enable_Secrets(false);
			Print("Not allowed in laddered games\r\n");

		} else {

			CNCPurchaseMainMenuClass::Enable_Secrets(true);
			if (CNCPurchaseMainMenuClass::Are_Secrets_Enabled()) {
				Print("extras ENABLED!\r\n");
			}
		}
#endif
	}
};

class DebugDeviceConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "debug_device"; }
   const char * Get_Alias( void ) { return "dd"; }
	virtual	const char * Get_Help( void )	{ return
		"DEBUG_DEVICE [device|on|off] - toggles the debug device (screen, mono, dbwin32, log, windows)"; }
	virtual	void Activate( const char * input ) {
		char str[128];
		sprintf(str,"ERROR (%s)\n", input );
		if (stricmp(input,"screen") == 0) {
			DebugManager::Toggle_Device_Enabled( DebugManager::DEBUG_DEVICE_SCREEN );
			sprintf(str, "Screen Debug %s\n", DebugManager::Is_Device_Enabled( DebugManager::DEBUG_DEVICE_SCREEN ) ? "Enabled" : "Disabled" );
		} else if (stricmp(input,"mono") == 0) {
			DebugManager::Toggle_Device_Enabled( DebugManager::DEBUG_DEVICE_MONO );
			sprintf(str, "Mono Debug %s\n", DebugManager::Is_Device_Enabled( DebugManager::DEBUG_DEVICE_MONO ) ? "Enabled" : "Disabled" );
		} else if (stricmp(input,"dbwin32") == 0) {
			DebugManager::Toggle_Device_Enabled( DebugManager::DEBUG_DEVICE_DBWIN32 );
			sprintf(str, "DBWin32 Debug %s\n", DebugManager::Is_Device_Enabled( DebugManager::DEBUG_DEVICE_DBWIN32 ) ? "Enabled" : "Disabled" );
		} else if (stricmp(input,"log") == 0) {
			DebugManager::Toggle_Device_Enabled( DebugManager::DEBUG_DEVICE_LOG );
			sprintf(str, "Log File Debug %s\n", DebugManager::Is_Device_Enabled( DebugManager::DEBUG_DEVICE_LOG ) ? "Enabled" : "Disabled" );
		} else if (stricmp(input,"windows") == 0) {
			DebugManager::Toggle_Device_Enabled( DebugManager::DEBUG_DEVICE_WINDOWS );
			sprintf(str, "Windows Debug %s\n", DebugManager::Is_Device_Enabled( DebugManager::DEBUG_DEVICE_WINDOWS ) ? "Enabled" : "Disabled" );
		} else if (stricmp(input,"on") == 0) {
			DebugManager::Enable_Device( (DebugManager::DebugDevice)-1 );
			sprintf(str, "All Debug Devices Enabled\n" );
		} else if (stricmp(input,"off") == 0) {
			DebugManager::Disable_Device( (DebugManager::DebugDevice)-1 );
			sprintf(str, "All Debug Devices Disabled\n" );
		}
		Print(str);
		Debug_Say((str));
	}
};

class AdminMessageConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "admin_message"; }
   const char * Get_Alias( void ) { return "amsg"; }
	virtual	const char * Get_Help( void )	{ return
		"ADMIN_MESSAGE <message> - sends an admin message to all clients. Host only."; }
	virtual	void Activate( const char * input ) {
		WWASSERT(input != NULL);
		if (cNetwork::I_Am_Server() && ::strlen(input) > 0) {
			WideStringClass widestring;
			widestring.Convert_From(input);
			cScTextObj * p_message = new cScTextObj;
			p_message->Init(widestring, TEXT_MESSAGE_PUBLIC, true, HOST_TEXT_SENDER, -1);
		} else {
			Print(Get_Help());
		}
	}
};




class MessageConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "message"; }
   const char * Get_Alias( void ) { return "msg"; }
	virtual	const char * Get_Help( void )	{ return "MESSAGE <message> - sends a chat message to all clients. Host only."; }
	virtual	void Activate( const char * input ) {
		WWASSERT(input != NULL);
		if (cNetwork::I_Am_Server() && ::strlen(input) > 0) {
			WideStringClass widestring(input, true);
			cScTextObj *event_obj = new cScTextObj;
			event_obj->Init(widestring, TEXT_MESSAGE_PUBLIC, false, HOST_TEXT_SENDER, -1);
		} else {
			Print(Get_Help());
		}
	}
};


#ifndef FREEDEDICATEDSERVER
class FPSConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "fps"; }
   const char * Get_Alias( void ) { return "fps"; }
	virtual	const char * Get_Help( void )	{ return "FPS - toggle FPS display."; }
	virtual	void Activate( const char * ) {
//		if (!cNetwork::I_Am_Server()) {
         cDevOptions::ShowFps.Toggle();
//		}
	}
};
#endif //FREEDEDICATEDSERVER

#if 1
class ClientPhysicsOptimizationConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "client_physics_optimization"; }
	const char * Get_Alias( void ) { return "cpo"; }
	virtual	const char * Get_Help( void )	{ return "CLIENT_PHYSICS_OPTIMIZATION [0|1] - Update only visible physic objects on client."; }
	virtual	void Activate( const char * input) {
		if (!cNetwork::I_Am_Server()) {
			int state = 0;
			int argcount=::sscanf(input, "%d", &state);
			if (argcount==1) {
				state=!!state;
			}
			else {
				state=!COMBAT_SCENE->Get_Update_Only_Visible_Objects();
			}
         COMBAT_SCENE->Set_Update_Only_Visible_Objects(state==1);

			Print( "Client physics optimization %s\n", state ? "ENABLED" : "DISABLED");
		}
	}
};
#endif

class StatsConsoleFunctionClass : public ConsoleFunctionClass
{
public:
	virtual	const char * Get_Name( void )	{ return "stats"; }
	virtual	const char * Get_Help( void )	{ return "STATS <off|fps|dx8|star|sr_history|sr|gerd|collision|culling|physics|vehicle|ai|wol|profile|memory> - changes stats display."; }
	virtual	void Activate( const char * input ) {
		StatisticsDisplayManager::Set_Display( input );
	}
};

class ResolutionConsoleFunctionClass : public ConsoleFunctionClass
{
public:
	virtual	const char * Get_Name( void )	{ return "resolution"; }
	virtual	const char * Get_Help( void )	{ return "RESOLUTION [+|-]"; }
	virtual	void Activate( const char * input ) {
		int w,h,bits;
		bool windowed;
		WW3D::Get_Device_Resolution(w,h,bits,windowed);
		const RenderDeviceDescClass& desc=WW3D::Get_Render_Device_Desc();
		const DynamicVectorClass<ResolutionDescClass> & resos=desc.Enumerate_Resolutions();
		int i;
		for (i=0;i<resos.Count();++i) {
			if (resos[i].Width==w && resos[i].Height==h && resos[i].BitDepth==bits) {
				break;
			}
		}

		unsigned char sign;
		if (sscanf (input, "%c", &sign) == 1 && (sign=='+' || sign=='-')) {

			if (sign=='+') i++;
			else i--;

			if (i>=resos.Count()) i=0;
			else if (i<0) i=resos.Count()-1;

			WW3DErrorType error=WW3D::Set_Device_Resolution(
				resos[i].Width,
				resos[i].Height,
				resos[i].BitDepth,
				-1,
				true );
			if (error==WW3D_ERROR_OK) {
				RectClass rect;
				rect.Left = 0;
				rect.Top = 0;
				rect.Right = resos[i].Width;
				rect.Bottom = resos[i].Height;

				Render2DClass::Set_Screen_Resolution(rect);
				Print( "Resolution changed to %d * %d, %d bits\n", resos[i].Width,resos[i].Height,resos[i].BitDepth);
			}
			else {
				Print( "Failed to set resolution to %d * %d, %d bits\n", resos[i].Width,resos[i].Height,resos[i].BitDepth);
				error=WW3D::Set_Device_Resolution(
					w,
					h,
					bits,
					-1,
					true );
			}
		}
		else {
			Print("Resolution is %d * %d, %d bits\n",w,h,bits);
		}
	}
};


class ToggleSurfaceEffectsConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "toggle_surface_effects"; }
	virtual	const char * Get_Help( void )	{ return "TOGGLE_SURFACE_EFFECTS - toggles surface effects."; }
	virtual	void Activate( const char * input ) {
		SurfaceEffectsManager::Enable_Effects(!SurfaceEffectsManager::Are_Effects_Enabled());
      Print( SurfaceEffectsManager::Are_Effects_Enabled() ? "Surface Effects Enabled\n" : "Surface Effects Disabled\n" );
	}
};


//----------------------------------------------------------------------------
//
// SHIPPING CONSOLE FUNCTIONS ONLY
// THESE MUST BE DEBUGGED!  THEY ARE PRESENT IN RELEASE BUILDS!!!
//
//----------------------------------------------------------------------------

class DisplayFindpathConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "display_findpath"; }
	virtual	const char * Get_Help( void )	{ return "DISPLAY_FINDPATH - Toggles the display of findpath lines."; }
	virtual	void Activate( const char * input ) {
		Toggle_Display_Findpaths();
	}
};


class DonateConsoleFunctionClass : public ConsoleFunctionClass {
	StringClass HelpText;
public:
	virtual const char * Get_Name(void) {return "donate";}
	virtual const char * Get_Help(void) {
		HelpText.Format("DONATE <credits> <teammate> - donate money (minimum %d credits) to teammate.",
			cDonateEvent::Get_Minimum_Acceptable_Donation());
		return HelpText.Peek_Buffer();;
	}

	virtual void Activate(const char * input) {

		int amount = 0;
		char recipient[1000];
		::sscanf(input, "%d %s", &amount, recipient);

		cPlayer * p_me = cNetwork::Get_My_Player_Object();
		cPlayer * p_recipient = cPlayerManager::Find_Player(recipient);

		if (p_me != NULL &&
			 p_recipient != NULL &&
			 amount >= cDonateEvent::Get_Minimum_Acceptable_Donation() &&
			(p_me->Get_Player_Type() == p_recipient->Get_Player_Type()) &&
			 p_me->Get_Money() >= amount) {

			cDonateEvent * p_event = new cDonateEvent;
			p_event->Init(amount, p_recipient->Get_Id());

		} else {
		   Print(Get_Help());
		}
	}
};


class DoStuffConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "do_stuff"; }
	virtual	const char * Get_Help( void )	{ return "DO_STUFF - Execute some task..."; }
	virtual	void Activate( const char * input ) {

		//
		// This command is for executing some test... put whatever code
		// you want here...
		//

		/*
		int recipient_id = atoi(input);

		WideStringClass msg;
		msg.Format(L"Hola amigo!");

		if (cNetwork::I_Am_Client()) {
			cCsTextObj * p_message = new cCsTextObj;
			p_message->Init(msg, TEXT_MESSAGE_PRIVATE, cNetwork::Get_My_Id(), recipient_id);
		} else {
			cScTextObj * p_message = new cScTextObj;
			p_message->Init(msg, TEXT_MESSAGE_PRIVATE, HOST_TEXT_SENDER, recipient_id);
		}
		*/

		//::exit(1);

		//::Sleep(::atoi(input));
/*
#if 0
		if (cNetwork::I_Am_Server()) {
			float x, y, z;
			::sscanf(input, "%f, %f, %f", &x, &y, &z);
			Vector3 position;
			position.Set(Vector3(x, y, z));

			cScExplosionEvent * p_event = new cScExplosionEvent;
			p_event->Init(1, position);
		}
#endif
*/

		cPlayer * p_player = cPlayerManager::Find_Player(input);
		if (p_player != NULL) {
			int id = p_player->Get_Id();
			cNetwork::Server_Kill_Connection(id);
			cNetwork::Cleanup_After_Client(id);
		}
	}
};


//
// REQUIRED for shipping dedicated server.
//
class QuitConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "quit"; }
	virtual	const char * Get_Help( void )	{
		if (!SlaveMaster.Am_I_Slave()) {
			return ("QUIT - End game and quit to desktop (dedicated server only).\n");
		} else {
			return ("QUIT - End game and restart slave server.\nTo exit without restarting use 'QUIT_SLAVE' on master server\n");
		}
	}
	virtual	void Activate( const char * input ) {

		if (!IS_MISSION) {
			if (AutoRestart.Is_Active()) {
				AutoRestart.Set_Restart_Flag(false);
				Set_Exit_On_Exception(true);
         	cGameData::Set_Manual_Exit(true);
			} else {
				if (cNetwork::I_Am_Server()) {
					if (The_Game() && The_Game()->IsDedicated.Is_True()) {

						if (!SlaveMaster.Am_I_Slave()) {
         				Print("Terminating slaves on demand...\n");
							SlaveMaster.Shutdown_Slaves();
						}

         			Print("Terminating game on demand...\n");
						AutoRestart.Set_Restart_Flag(false);
						Set_Exit_On_Exception(true);
         			cGameData::Set_Manual_Exit(true);
					} else {
         			Print("QUIT is for dedicated server only.\n");
					}
				} else {
					/*
					** I_Am_Server may not report true if we just started up and we aren't a server yet.
					*/
					if (ConsoleBox.Is_Exclusive()) {
						AutoRestart.Set_Restart_Flag(false);
						Set_Exit_On_Exception(true);
         			cGameData::Set_Manual_Exit(true);
					} else {
         			Print("QUIT is for dedicated server only.\n");
					}
				}
			}
      }
	}
};




//
// REQUIRED for shipping dedicated server.
//
class QuitSlaveConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "quit_slave"; }
	virtual	const char * Get_Help( void )	{ return "QUIT_SLAVE slavename - Shutdown a slave server (dedicated master server only)."; }
	virtual	void Activate( const char * input ) {

		if (!IS_MISSION) {
			if (cNetwork::I_Am_Server()) {
				if (The_Game() && The_Game()->IsDedicated.Is_True()) {

					if (!SlaveMaster.Am_I_Slave()) {
         			Print("Terminating slave %s on demand...\n", input);
						if (SlaveMaster.Shutdown_Slave((char*)input)) {
         				Print("Slave %s terminated\n", input);
						} else {
         				Print("Slave %s not found\n", input);
						}
						return;
					}
				}
			}
		}
      Print("QUIT_SLAVE is for dedicated master server only.\n");
	}
};





class NetUpdateRateConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "net_update_rate"; }
	virtual	const char * Get_Alias( void ){ return "nur"; }
	virtual	const char * Get_Help( void )	{ return "NET_UPDATE_RATE - set the max. net update think rate (times per second)."; }
	virtual	void Activate( const char * input ) {
      WWASSERT(input != NULL);
		int rate = ::atoi(input);
#ifdef WWDEBUG
		if (rate > 0) {
#else //WWDEBUG
		if (rate >= 5 && rate <= 30) {
#endif //WWDEBUG
			cUserOptions::NetUpdateRate.Set(rate);
         Print( "NetUpdateRate set to %d.", rate);
		} else {
		   Print(Get_Help());
		}
	}
};




class GameInfoConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "game_info"; }
	virtual	const char * Get_Help( void )	{ return "GAME_INFO - Print info about a game in progress to console box"; }
	virtual	void Activate(const char *) {

		bool active = false;

		if (cNetwork::I_Am_Server()) {

			if (The_Game() && The_Game()->IsDedicated.Is_True()) {

				char upstring[256] = "?";
				char timestr[256] = "?";
				FILETIME creation;
				FILETIME exit;
				FILETIME kernel;
				FILETIME user;
				int ok = GetProcessTimes(GetCurrentProcess(), &creation, &exit, &kernel, &user);
				if (ok) {
					FILETIME local;
					if (FileTimeToLocalFileTime(&creation, &local)) {
						SYSTEMTIME time;
						if (FileTimeToSystemTime(&local, &time)) {
							GetDateFormat(LOCALE_SYSTEM_DEFAULT, 0, &time, NULL, upstring, 256);
							GetTimeFormat(LOCALE_SYSTEM_DEFAULT, TIME_FORCE24HOURFORMAT, &time, NULL, timestr, 256);
							strcat(upstring, " - ");
							strcat(upstring, timestr);
						}
					}
				}

				GameModeClass* game = GameModeManager::Find("Combat");
				if (game && game->Is_Active()) {
					game = GameModeManager::Find("WOL");
					if (game && game->Is_Active()) {
						ConsoleBox.Print("Westwood Online mode active since %s\n", upstring);
						active = true;
					} else {
						game = GameModeManager::Find("LAN");
						if (game && game->Is_Active()) {
							if (cGameSpyAdmin::Get_Is_Server_Gamespy_Listed()) {
								ConsoleBox.Print("GameSpy mode active since %s\n", upstring);
							} else {
								ConsoleBox.Print("LAN mode active since %s\n", upstring);
							}
							active = true;
						}
					}
				}

				if (active && The_Game()) {

					unsigned long time = The_Game()->Get_Time_Remaining_Seconds();
					unsigned long seconds = time % 60;
					unsigned long minutes = (time / 60) % 60;
					unsigned long hours = (time / (60*60));


					if (The_Game()->Is_Gameplay_Permitted()) {
						ConsoleBox.Print("Gameplay in progress\n");
					} else {
						ConsoleBox.Print("Gameplay Pending\n");
					}

					char buf[256];
					sprintf(buf, "     Map : %s\n", The_Game()->Get_Map_Name().Peek_Buffer());
					ConsoleBox.Print(buf);

					sprintf(buf, "    Time : %d.%02d.%02d\n", hours, minutes, seconds);
					ConsoleBox.Print(buf);

					sprintf(buf, "     Fps : %d\n", cNetwork::Get_Fps());
					ConsoleBox.Print(buf);

					sprintf(buf, "     GDI : %d/%d players      %d points\n",
						cPlayerManager::Tally_Team_Size(PLAYERTYPE_GDI),
						The_Game()->Get_Max_Players(),
						(int)cTeamManager::Find_Team(PLAYERTYPE_GDI)->Get_Score());
					ConsoleBox.Print(buf);

					sprintf(buf, "     NOD : %d/%d players      %d points\n",
						cPlayerManager::Tally_Team_Size(PLAYERTYPE_NOD),
						The_Game()->Get_Max_Players(),
						(int)cTeamManager::Find_Team(PLAYERTYPE_NOD)->Get_Score());
					ConsoleBox.Print(buf);

					ConsoleBox.Print("\n");
				}
			}
		}
	}
};








class PlayerInfoConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "player_info"; }
	virtual	const char * Get_Help( void )	{ return "PLAYER_INFO - Print info about players in the game to the console box"; }
	virtual	void Activate(const char *) {

		if (cNetwork::I_Am_Server() && The_Game() && The_Game()->IsDedicated.Is_True() && cNetwork::PServerConnection != NULL) {

			unsigned long time = TIMEGETTIME();
			unsigned long bw = 0;

			if (cNetwork::PServerConnection->Get_Num_RHosts() == 0) {
				ConsoleBox.Print("No players\n");
			} else {

				ConsoleBox.Print("\rId  Name           Score Side\tPing\tAddress               Kbits/s Time\n\n");

				for (int i=cNetwork::PServerConnection->Get_Min_RHost() ; i <= cNetwork::PServerConnection->Get_Max_RHost() ; i++) {

					cRemoteHost *client = cNetwork::PServerConnection->Get_Remote_Host(i);

					if (client) {
      				cPlayer * playerptr = cPlayerManager::Find_Player(i);

						if (playerptr) {
							WideStringClass name(playerptr->Get_Name(), true);
							StringClass short_name;
							name.Convert_To(short_name);
							int len = name.Get_Length();
							char temp_name[256];
							strcpy(temp_name, short_name.Peek_Buffer());
							strncat(temp_name, "                    ", 14-len);

							unsigned long dur = (time - client->Get_Creation_Time()) / 1000;

							unsigned long seconds = dur % 60;
							unsigned long minutes = (dur / 60) % 60;
							unsigned long hours = (dur / (60*60));

							bw += PacketManager.Get_Compressed_Bandwidth_Out(&client->Get_Address());

							char addr_string[128];
							sockaddr_in *addr = &client->Get_Address();
							sprintf(addr_string, "%d.%d.%d.%d;%d", 	(int)(addr->sin_addr.S_un.S_un_b.s_b1),
																					(int)(addr->sin_addr.S_un.S_un_b.s_b2),
																					(int)(addr->sin_addr.S_un.S_un_b.s_b3),
																					(int)(addr->sin_addr.S_un.S_un_b.s_b4),
																					unsigned int(ntohs(addr->sin_port)));
							int addr_string_len = strlen(addr_string);
							char local_addr_string[128];
							strcpy(local_addr_string, addr_string);
							if (addr_string_len < 21) {
								strncat(local_addr_string, "                         ", 21-addr_string_len);
							}

							Vector3 color = playerptr->Get_Color();
							char buffer[256];
							sprintf(buffer, "%3d %s%- 6d %s\t%d\t%s%- 7d  %03d.%02d.%02d\n", i, temp_name,
																								(int)playerptr->Get_Score(),
																								(playerptr->Get_Player_Type() == PLAYERTYPE_NOD) ? "NOD" : "GDI",
																								(int)client->Get_Average_Internal_Pingtime_Ms(),
																								local_addr_string,
																								(int)(PacketManager.Get_Compressed_Bandwidth_Out(&client->Get_Address()) / 1024),
																								hours, minutes, seconds);

							WideStringClass ws(buffer, true);
							ConsoleBox.Add_Message(&ws, &color, true);
						}
					}
				}
				ConsoleBox.Print("Total current bandwidth usage for players is %d kilobits per second\n", bw / 1024);
			}
   	} else {
			Print("player_info is for dedicated server only\n");
		}
	}
};

class KickConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "kick"; }
	virtual	const char * Get_Help( void )	{ return "KICK [<Nickname>|<Id>] - Kick a user from the game."; }
	bool KickWOLUser(const WideStringClass & user_name) {

		GameModeClass* game = GameModeManager::Find("WOL");

		if (game && game->Is_Active()) {

			if (user_name.Get_Length() > 0) {
				WolGameModeClass* wolgame = reinterpret_cast<WolGameModeClass*>(game);

				if (wolgame->Kick_Player(user_name)) {
					StringClass smallstring(true);
					user_name.Convert_To(smallstring);
					ConsoleBox.Print("%s was kicked\n", smallstring.Peek_Buffer());
					return true;
				}
			}
		}
		return false;
	}
	bool KickWOLUser(int id) {
		cPlayer *player = cPlayerManager::Find_Player(id);
		if (player) {
			return KickWOLUser(player->Get_Name());
		}
		return false;
	}
	bool KickGameSpyUser(int id) {
		if (cGameSpyAdmin::Is_Gamespy_Game()) {
			return GameSpyBanList.Kick_Player(id);
		}
		return false;
	}
	bool KickGameSpyUser(const WideStringClass & user_name) {
		if (cGameSpyAdmin::Is_Gamespy_Game()) {
			cPlayer *player = NULL;

			for (SLNode<cPlayer> *player_node = cPlayerManager::Get_Player_Object_List ()->Head ()
				; player_node != NULL; player_node = player_node->Next ()) {

				player = player_node->Data ();
				WWASSERT (player != NULL);

				if (player->Get_Is_Active().Is_False() || !player->Is_Human()) {
					continue;
				}
				if (stricmp(StringClass(user_name), StringClass(player->Get_Name())) == 0) {
					KickGameSpyUser(player->Get_Id());
					return true;
				}
			}
		}
		return false;
	}
	virtual	void Activate( const char * input ) {

		if (cNetwork::I_Am_Server()) {
			if (!input || !(*input)) return;
			bool kicked = false;
			WideStringClass user_name(input, true);

			kicked = KickWOLUser(user_name);
			if (!kicked) kicked = KickGameSpyUser(user_name);

			if (!kicked) {
				// try to kick by Id number
				// make sure this is a decimal number since atoi returns 0 on error
				// and we don't want to kick player 0 unless they really mean to
				const char *t = input;
				if (!(*t)) t = NULL;
				while (t && *t) {
					if (!isdigit(*t)) t = NULL;
					else t++;
				}
				if (t) {
					kicked = KickWOLUser(atoi(input));
					if (!kicked) kicked = KickGameSpyUser(atoi(input));
				}
			}

			if (!kicked) {
				ConsoleBox.Print("%s not found\n", input);
			}
		}
	}
};

class BanConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "ban"; }
	virtual	const char * Get_Help( void )	{ return "BAN [<Nickname>|<Id>] - Permanently ban a user from this server(GameSpy & WOL mode only)."; }
	virtual	void Activate( const char * input ) {

		GameModeClass* wolgame = GameModeManager::Find("WOL");
		bool is_wol = false;
		if (wolgame && wolgame->Is_Active()) {
			is_wol = true;
		}
		//if (cGameSpyAdmin::Is_Gamespy_Game() && cNetwork::I_Am_Server()) {
		if ((cGameSpyAdmin::Is_Gamespy_Game() || is_wol) && cNetwork::I_Am_Server()) {

			if (!input || !(*input)) return;
			cPlayer *player = NULL;

			SLNode<cPlayer> *player_node;
			for (player_node = cPlayerManager::Get_Player_Object_List ()->Head ()
				; player_node != NULL; player_node = player_node->Next ()) {

				player = player_node->Data ();
				WWASSERT (player != NULL);

				if (player->Get_Is_Active().Is_False() || !player->Is_Human()) {
					continue;
				}
				if (stricmp(StringClass(input), StringClass(player->Get_Name())) == 0) {
					if (is_wol) {
						((WolGameModeClass*)wolgame)->Ban_Player(player->Get_Name(), player->Get_Ip_Address());
					} else {
						if (player->Get_GameSpy_Hash_Id().Is_Empty()) {
							GameSpyBanList.Ban_User(input);
						} else {
							GameSpyBanList.Ban_User(input, player->Get_GameSpy_Hash_Id());
						}
					}
					break;
				}
			}
			if (player_node == NULL) {
				// make sure this is a decimal number since atoi returns 0 on error
				// and we don't want to kick player 0 unless they really mean to
				const char *t = input;
				if (!(*t)) t = NULL;
				while (t && *t) {
					if (!isdigit(*t)) t = NULL;
					else t++;
				}
				if (t) {
					player = cPlayerManager::Find_Player(atoi(input));
					if (player) {
						if (is_wol) {
							((WolGameModeClass*)wolgame)->Ban_Player(player->Get_Name(), player->Get_Ip_Address());
						} else {
							if (player->Get_GameSpy_Hash_Id().Is_Empty() && !player->Get_Name().Is_Empty()) {
								GameSpyBanList.Ban_User(StringClass(player->Get_Name()));
							} else if (!player->Get_GameSpy_Hash_Id().Is_Empty()) {
								GameSpyBanList.Ban_User(StringClass(player->Get_Name()), player->Get_GameSpy_Hash_Id());
							}
						}
					}
				} else {
					ConsoleBox.Print("%s not found\n", input);
				}
			}
		}
	}
};


class AllowConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "allow"; }
	virtual	const char * Get_Help( void )	{ return "ALLOW [<Nickname>|<Id>] - Remove user channel ban from this server (WOL mode only)."; }
	virtual	void Activate( const char * input ) {

		GameModeClass* game = GameModeManager::Find("WOL");

		if (!input || !(*input)) return;

		if (game && game->Is_Active()) {

			WideStringClass wname(input, true);
			
			if (wname.Get_Length() > 0 && wname.Get_Length() <15) {
				RefPtr<WWOnline::Session> wol_session = WWOnline::Session::GetInstance(false);
				if (wol_session.IsValid()) {
					wol_session->BanUser(wname, false);
				}
			}
		}
	}
};


class PageConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "page"; }
	virtual	const char * Get_Help( void )	{ return "PAGE Nickname Message - Page a Westwood Online user."; }
	virtual	void Activate( const char * input ) {

		static unsigned long last_page = 0;

		GameModeClass* wolgame = GameModeManager::Find("WOL");
		bool is_wol = false;
		if (wolgame && wolgame->Is_Active()) {
			is_wol = true;
		}
		
		if (is_wol) {
			RefPtr<WWOnline::Session> wol_session = WWOnline::Session::GetInstance(false);
			if (wol_session.IsValid()) {
				
				unsigned long time = TIMEGETTIME();
				if (time < last_page || time - last_page > 1000) {
					last_page = time;

					char temp[1024];
					strncpy(temp, input, 1023);
					char *name = temp;
					char *message = strchr(temp, ' ');
					if (message) {
						*message++ = 0;

						if (strlen(name) && strlen(message)) {
							WideStringClass wname(name, true);
							WideStringClass wmessage(message, true);
							wol_session->PageUser(wname, wmessage);
						}
					}
				}
			}
		}
	}
};





//
// REQUIRED for shipping dedicated server.
//
class RestartConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "restart"; }
	virtual	const char * Get_Help( void )	{ return "RESTART - Quit to desktop and restart process (dedicated server only)."; }
	virtual	void Activate( const char * input ) {

		if (!IS_MISSION) {
			if (AutoRestart.Is_Active()) {
				AutoRestart.Set_Restart_Flag(true);
				Set_Exit_On_Exception(true);
         	cGameData::Set_Manual_Exit(true);
			} else {
				if (cNetwork::I_Am_Server()) {
					if (The_Game() && The_Game()->IsDedicated.Is_True()) {

						if (SlaveMaster.Am_I_Slave() == false) {
         				Print("Terminating slaves on demand...\n");
							SlaveMaster.Shutdown_Slaves();
						}

         			Print("Restarting game on demand...\n");
						AutoRestart.Set_Restart_Flag(true);
						Set_Exit_On_Exception(true);
         			cGameData::Set_Manual_Exit(true);
					} else {
         			Print("RESTART is for dedicated server only.\n");
					}
				} else {
         		Print("RESTART is for dedicated server only.\n");
				}
			}
      }
	}
};




class LogMeshStatsConsoleFunctionClass : public ConsoleFunctionClass
{
public:
	virtual	const char * Get_Name( void )	{ return "log_mesh_stats"; }
	virtual	const char * Get_Help( void )	{ return "LOG_MESH_STATS - log stats to debug window or file."; }
	virtual	void Activate( const char * input ) {
		TheDX8MeshRenderer.Request_Log_Statistics();
	}
};

class LogTexturesConsoleFunctionClass : public ConsoleFunctionClass
{
public:
	virtual	const char * Get_Name( void )	{ return "log_textures"; }
	virtual	const char * Get_Help( void )	{ return "LOG_TEXTURES - log texture statistics to debug window or file."; }
	virtual	void Activate( const char * input ) {
		WW3DAssetManager::Get_Instance()->Log_All_Textures();
	}
};

class DeviceInfoConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "device_info"; }
	virtual	const char * Get_Help( void )	{ return "DEVICE_INFO - get device driver info."; }
	virtual	void Activate( const char * input ) {
		const RenderDeviceDescClass &	rddesc = WW3D::Get_Render_Device_Desc();
		Print( "Render Device: %s\n", rddesc.Get_Device_Name() );
		Print( "Graphics Hardware: %s\n",rddesc.Get_Hardware_Name() );
	}
};


//
// TEMP DEBUG CODE
//
class NewTCADOConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "newtcado"; }
	virtual	const char * Get_Help( void )	{ return "newtcado - Toggle new TCADO code"; }
	virtual	void Activate( const char * input ) {
		bool is_new_tcado = cDevOptions::UseNewTCADO.Toggle();
      Print(is_new_tcado ? "Using new TCADO.\n" : "Using old TCADO.\n" );
	}
};


class TimeOfDayConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name (void)	{return ("time_of_day");}
	virtual	const char * Get_Help (void)	{return ("TIME_OF_DAY <hours [0..23]> <minutes [0..59]> - sets the time of day for the background.");}
	virtual	void Activate (const char *input) {

		if (COMBAT_SCENE != NULL) {

			unsigned hours, minutes;

			if (sscanf (input, "%d %d", &hours, &minutes) == 2) {
				if (BackgroundMgrClass::Set_Time_Of_Day (hours, minutes)) {

					const float theta = (((hours * 60.0f) + minutes) / 1440.0f) * 2.0f * WWMATH_PI;

					BackgroundMgrClass::Set_Light_Source_Type (BackgroundMgrClass::LIGHT_SOURCE_TYPE_SUN);
					COMBAT_SCENE->Set_Sun_Light_Orientation (0.0f, theta + (WWMATH_PI * 1.5f));
					Print ("Time of day changed\n");
				}
			}
		}
	}
};


class CloudsConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name (void)	{return ("clouds");}
	virtual	const char * Get_Help (void)	{return ("CLOUDS <cloudcover [0..1]> <gloominess [0..1]> [ramptime [0+]]");}
	virtual	void Activate (const char *input) {

		const char *feedbackstring = "Clouds changed\n";

		int	argcount;
		float cloudcover, gloominess, ramptime;
		bool	success = false;

		argcount = sscanf (input, "%f %f %f", &cloudcover, &gloominess, &ramptime);
		switch (argcount) {

			case 2:
				success = BackgroundMgrClass::Set_Clouds (cloudcover, gloominess);
				break;

			case 3:
				success = BackgroundMgrClass::Set_Clouds (cloudcover, gloominess, ramptime);
				break;

			default:
				// Do nothing.
				break;
		}
		if (success) Print (feedbackstring);
	}
};


class CopyLogsConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name (void)	{return ("copy_logs");}
	virtual	const char * Get_Help (void)	{return ("COPY_LOGS <on/off> - enable/disable error logging to network.");}
	virtual	void Activate (const char *input) {

		if (!::stricmp(input, "on")) {

			RegistryClass registry(APPLICATION_SUB_KEY_NAME_DEBUG);
			WWASSERT(registry.Is_Valid());
			registry.Set_Int("SystemInfoLogDisable", FALSE);
			registry.Set_Int("DisableLogCopying", FALSE);

			Print("Log copying is ON.");

		} else if (!::stricmp(input, "off")) {

			RegistryClass registry(APPLICATION_SUB_KEY_NAME_DEBUG);
			WWASSERT(registry.Is_Valid());
			registry.Set_Int("SystemInfoLogDisable", TRUE);
			registry.Set_Int("DisableLogCopying", TRUE);

			Print("Log copying is OFF.");

		} else {

			Print(Get_Help());
		}
	}
};


class SkyTintConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name (void)	{return ("sky_tint");}
	virtual	const char * Get_Help (void)	{return ("SKY_TINT <tintfactor [0..1]> [ramptime [0+]]");}
	virtual	void Activate (const char *input) {

		const char *feedbackstring = "Sky tint changed\n";

		int	argcount;
		float skytintfactor, ramptime;
		bool	success = false;

		argcount = sscanf (input, "%f %f", &skytintfactor, &ramptime);
		switch (argcount) {

			case 1:
				success = BackgroundMgrClass::Set_Sky_Tint (skytintfactor);
				break;

			case 2:
				success = BackgroundMgrClass::Set_Sky_Tint (skytintfactor, ramptime);
				break;

			default:
				// Do nothing.
				break;
		}
		if (success) Print (feedbackstring);
	}
};


class InfoDebugToggle : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "info_debug_toggle"; }
	virtual	const char * Get_Help( void )	{ return "INFO_DEBUG_TOGGLE - Toggles display of debug info on object info."; }
	virtual	void Activate( const char * input ) {
		extern bool	InfoDebug;
		InfoDebug = !InfoDebug;
		Print( "Toggled Info Debug\n" );
	}
};

class MeshRenderSnapshotFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "mesh_render_snapshot"; }
	virtual	const char * Get_Help( void )	{ return "MESH_RENDER_SNAPSHOT - Logs one frame of mesh render system."; }
	virtual	void Activate( const char * input ) {
		WW3D::Activate_Snapshot(true);
	}
};

class UmbraToggleFunctionClass : public ConsoleFunctionClass {
public:
	virtual  const char * Get_Name(void) { return "umbra_toggle"; }
	virtual	const char * Get_Help(void) { return "UMBRA_TOGGLE - toggles umbra culling (if compiled into this build)."; }
	virtual  void Activate(const char * input) {
#if (UMBRASUPPORT)
		UmbraSupport::Enable_Umbra(!UmbraSupport::Is_Umbra_Enabled());
		Print("Umbra %s\n",(UmbraSupport::Is_Umbra_Enabled() ? "enabled." : "disabled."));
#else
		Print("Umbra support not compiled into this build.\n");
#endif

	}
};

class UnusedSpawnerFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "unused_spawners"; }
	virtual	const char * Get_Help( void )	{ return "UNUSED_SPAWNERS - Displays the ID of spawns which have spawned nothing."; }
	virtual	void Activate( const char * input ) {
		SpawnManager::Display_Unused_Spawners();
	}
};

class VipConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "vip"; }
	virtual	const char * Get_Help( void )	{ return "VIP <password> - Toggle VIP mode. Client only.\n"; }

	virtual	void Activate( const char * input ) {

		StringClass password;
		password.Format("%s", input);

      if (cNetwork::I_Am_Client()) {// && !password.Is_Empty()) {

			cVipModeEvent * p_event = new cVipModeEvent;
			p_event->Init(password);

		} else {

		   Print(Get_Help());
		}
	}
};

class AllowKillingHibernatingSpawnFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "allow_killing_hibernating_spawn"; }
	virtual	const char * Get_Help( void )	{ return "ALLOW_KILLING_HIBERNATING_SPAWN - Toggles the global ability for spawners to kill hibernating spawn."; }
	virtual	void Activate( const char * input ) {
		if (	SpawnManager::Toggle_Allow_Killing_Hibernating_Spawn() ) {
			Print("Killing Hibernating Spawn Allowed\n");
		} else {
			Print("Killing Hibernating Spawn Disallowed\n");
		}
	}
};

class DisplayHibernatingFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "display_hibernating"; }
	virtual	const char * Get_Help( void )	{ return "DISPLAY_HIBERNATING - Toggles displaying of objects begining and ending hibernation."; }
	virtual	void Activate( const char * input ) {
extern bool	_DisplayHibernating;
		_DisplayHibernating = !_DisplayHibernating;
		Print("Display Hibernating now %s\n", _DisplayHibernating ? "ON" : "OFF" );
	}
};

class HideHUDPointsFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "hide_hud_points"; }
	virtual	const char * Get_Help( void )	{ return "HIDE_HUD_POINTS - Toggles displaying of points on the HUD."; }
	virtual	void Activate( const char * input ) {
		Print("HUD points toggled\n" );
		HUDClass::Toggle_Hide_Points();
	}
};

class SnapShotModeToggleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "snap_shot_mode_toggle"; }
	virtual	const char * Get_Alias( void ){ return "ssm"; }
	virtual	const char * Get_Help( void )	{ return "SNAP_SHOT_MODE_TOGGLE - Toggles Snap Shot Mode."; }
	virtual	void Activate( const char * input ) {
		if ( COMBAT_CAMERA ) {
			COMBAT_CAMERA->Set_Snap_Shot_Mode( !COMBAT_CAMERA->Is_Snap_Shot_Mode() );
			Print( "Snap Shot Mode %s\n", COMBAT_CAMERA->Is_Snap_Shot_Mode() ? "ON" : "OFF" );
		}
	}
};


class LightningConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name (void)	{return ("lightning");}
	virtual	const char * Get_Help (void)	{return ("LIGHTNING <intensity [0..1]> [<start [0..1]> <end [0..1]> <heading [0..360]> <distribution [0..1]>] [ramptime [0+]] - Sets lightning.");}
	virtual	void Activate (const char *input) {

		const char *feedbackstring = "Lightning changed\n";

		int	argcount;
		float arg0, arg1, arg2, arg3, arg4, arg5;
		bool  success = false;

		argcount = sscanf (input, "%f %f %f %f %f %f", &arg0, &arg1, &arg2, &arg3, &arg4, &arg5);
		switch (argcount) {

			case 1:
				success = BackgroundMgrClass::Set_Lightning_Intensity (arg0);
				break;

			case 5:
				success = BackgroundMgrClass::Set_Lightning (arg0, arg1, arg2, arg3, arg4);
				break;

			case 6:
				success = BackgroundMgrClass::Set_Lightning (arg0, arg1, arg2, arg3, arg4, arg5);
				break;

			default:
				// Do nothing.
				break;
		}
		if (success) Print (feedbackstring);
	}
};


class WarBlitzConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name (void)	{return ("warblitz");}
	virtual	const char * Get_Help (void)	{return ("WARBLITZ <intensity [0..1]> [<start [0..1]> <end [0..1]> <heading [0..360]> <distribution [0..1]>] [ramptime [0+]] - Sets war blitz.");}
	virtual	void Activate (const char *input) {

		const char *feedbackstring = "War blitz changed\n";

		int	argcount;
		float arg0, arg1, arg2, arg3, arg4, arg5;
		bool  success = false;

		argcount = sscanf (input, "%f %f %f %f %f %f", &arg0, &arg1, &arg2, &arg3, &arg4, &arg5);
		switch (argcount) {

			case 1:
				success = BackgroundMgrClass::Set_War_Blitz (arg0);
				break;

			case 5:
				success = BackgroundMgrClass::Set_War_Blitz (arg0, arg1, arg2, arg3, arg4);
				break;

			case 6:
				success = BackgroundMgrClass::Set_War_Blitz (arg0, arg1, arg2, arg3, arg4, arg5);
				break;

			default:
				// Do nothing.
				break;
		}
		if (success) Print (feedbackstring);
	}
};


class WarpConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name (void)	{return ("warp");}
	virtual	const char * Get_Help (void)	{return ("WARP <player name> - warp to first other player or named player (client only).");}
	virtual	void Activate (const char *input) {
      if (cNetwork::I_Am_Client()) {

			WideStringClass player_name;
			player_name.Convert_From(input);

			cWarpEvent * p_warp = new cWarpEvent;
			p_warp->Init(player_name);

		} else {
		   Print(Get_Help());
		}
	}
};


class WhoIsConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name (void)	{return ("whois");}
	virtual	const char * Get_Help (void)	{return ("WHOIS <player name> - who the heck is this annoying bugger? (server only)\n");}
	virtual	void Activate (const char *input) {
		cPlayer * p_player = cPlayerManager::Find_Player(input);

		if (!cNetwork::I_Am_Server() || p_player == NULL) {
		   Print(Get_Help());
		} else {

			ULONG ip = p_player->Get_Ip_Address();
			HOSTENT * p_host = ::gethostbyaddr((char *) &ip, sizeof(ip), AF_INET);
			char resolved_ip[100] = "";
			if (p_host != NULL) {
				::sprintf(resolved_ip, "(%s)", p_host->h_name);
			}

			Print("%s is at ip %s %s\n",
				input, cNetUtil::Address_To_String(ip), resolved_ip);
		}
	}
};


class WindConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name (void)	{return ("wind");}
	virtual	const char * Get_Help (void)	{return ("WIND <heading [0..360]> <speed [0+]> <variability [0..1]> [ramptime [0+]] - Sets wind.");}
	virtual	void Activate (const char *input) {

		const char *feedbackstring = "Wind changed\n";

		int	argcount;
		float heading, speed, variability, ramptime;
		bool  success = false;

		argcount = sscanf (input, "%f %f %f %f", &heading, &speed, &variability, &ramptime);
		switch (argcount) {

			case 3:
				success = WeatherMgrClass::Set_Wind (heading, speed, variability);
				break;

			case 4:
				success = WeatherMgrClass::Set_Wind (heading, speed, variability, ramptime);
				break;

			default:
				// Do nothing.
				break;
		}
		if (success) Print (feedbackstring);
	}
};


class RainConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name (void)	{return ("rain");}
	virtual	const char * Get_Help (void)	{return ("RAIN <density [0+]> [ramptime [0+]] - Sets rain.");}
	virtual	void Activate (const char *input) {

		const char *feedbackstring = "Rain changed\n";

		int	argcount;
		float	arg0, arg1;
		bool	success = false;

		argcount = sscanf (input, "%f %f", &arg0, &arg1);
		switch (argcount) {

			case 1:
				success = WeatherMgrClass::Set_Precipitation (WeatherMgrClass::PRECIPITATION_RAIN, arg0);
				break;

			case 2:
				success = WeatherMgrClass::Set_Precipitation (WeatherMgrClass::PRECIPITATION_RAIN, arg0, arg1);
				break;

			default:
				// Do nothing.
				break;
		}
		if (success) Print (feedbackstring);
	}
};


class SnowConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name (void)	{return ("snow");}
	virtual	const char * Get_Help (void)	{return ("SNOW <density [0+]> [ramptime [0+]] - Sets snow.");}
	virtual	void Activate (const char *input) {

		const char *feedbackstring = "Snow changed\n";

		int	argcount;
		float	arg0, arg1;
		bool	success = false;

		argcount = sscanf (input, "%f %f", &arg0, &arg1);
		switch (argcount) {

			case 1:
				success = WeatherMgrClass::Set_Precipitation (WeatherMgrClass::PRECIPITATION_SNOW, arg0);
				break;

			case 2:
				success = WeatherMgrClass::Set_Precipitation (WeatherMgrClass::PRECIPITATION_SNOW, arg0, arg1);
				break;

 			default:
				// Do nothing.
				break;
		}
		if (success) Print (feedbackstring);
	}
};


class AppPacketTypesResetConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name( void )	{ return "app_packet_types_reset"; }
	virtual	const char * Get_Alias( void ){ return "aptr"; }
	virtual	const char * Get_Help( void )	{ return "APP_PACKET_TYPES_RESET - reset app packet stats."; }
	virtual	void Activate( const char * input ) {
		cAppPacketStats::Reset();
	}
};


class AshConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name (void)	{return ("ash");}
	virtual	const char * Get_Help (void)	{return ("ASH <density [0+]> [ramptime [0+]] - Sets ash.");}
	virtual	void Activate (const char *input) {

		const char *feedbackstring = "Ash changed\n";

		int	argcount;
		float	arg0, arg1;
	  	bool	success = false;

		argcount = sscanf (input, "%f %f", &arg0, &arg1);
		switch (argcount) {

			case 1:
				success = WeatherMgrClass::Set_Precipitation (WeatherMgrClass::PRECIPITATION_ASH, arg0);
				break;

			case 2:
				success = WeatherMgrClass::Set_Precipitation (WeatherMgrClass::PRECIPITATION_ASH, arg0, arg1);
				break;

			default:
				// Do nothing.
				break;
		}
		if (success) Print (feedbackstring);
	}
};

/*
**
*/
class CinematicFreezeConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name (void)	{return ("cinematic_freeze");}
	virtual	const char * Get_Help (void)	{return ("CINEMATIC_FREEZE - Toggles Cinematic Freeze.");}
	virtual	void Activate (const char *input) {
		GameObjManager::Toggle_Cinematic_Freeze();
		Print( "Cinematic Freeze %s\n",
			GameObjManager::Is_Cinematic_Freeze_Active() ? "ON" : "OFF" );
	}
};



class EditVehicleConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name (void)	{return ("edit_vehicle");}
	virtual	const char * Get_Help (void)	{return ("EDIT_VEHICLE - Edit the parameters of the currently driven vehicle.");}
	virtual	void Activate (const char *input)
	{
		// Only do this in single player or 1-player multiplay games
      if (IS_SOLOPLAY || ((PTheGameData != NULL) && (PTheGameData->Get_Max_Players() == 1)) )
		{
			VehicleGameObj * vehicle = COMBAT_STAR->Get_Vehicle();
			if ((vehicle != NULL) && (vehicle->Peek_Vehicle_Phys() != NULL)) {
				VehiclePhysClass * pobj = vehicle->Peek_Vehicle_Phys();

				TrackedVehicleClass * tv = pobj->As_TrackedVehicleClass();
				WheeledVehicleClass * wv = pobj->As_WheeledVehicleClass();

				if (tv != NULL) {

					float wheel_radius = 0.0f;
					for (int i=0; i<tv->Get_Wheel_Count(); i++) {
						WheelClass * wheel = (WheelClass*)tv->Peek_Wheel(i);
						if (wheel->Get_Flag(SuspensionElementClass::ENGINE)) {
							wheel_radius = wheel->Get_Radius();
						}
					}

					TrackedVehicleDefClass * def = (TrackedVehicleDefClass *)tv->Get_TrackedVehicleDef();

					EditTrackedVehicleDialogClass * popup = new EditTrackedVehicleDialogClass(def,wheel_radius);
					popup->Start_Dialog();
					popup->Release_Ref();

				} else if (wv != NULL) {

					float wheel_radius = 0.0f;
					for (int i=0; i<wv->Get_Wheel_Count(); i++) {
						WheelClass * wheel = (WheelClass*)wv->Peek_Wheel(i);
						if (wheel->Get_Flag(SuspensionElementClass::ENGINE)) {
							wheel_radius = wheel->Get_Radius();
						}
					}

					WheeledVehicleDefClass * def = (WheeledVehicleDefClass *)wv->Get_WheeledVehicleDef();
					EditWheeledVehicleDialogClass * popup = new EditWheeledVehicleDialogClass(def,wheel_radius);
					popup->Start_Dialog();
					popup->Release_Ref();

				}
			}
		}
	}
};


/*
**
*/
class ToggleUseLetencyInterpretConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name (void)	{return ("toggle_latency_interpret");}
	virtual	const char * Get_Help (void)	{return ("TOGGLE_LATENCY_INTERPRET - Toggles Latency Interpret logic.");}
	virtual	void Activate (const char *input) {
		extern	bool	_UseLatencyInterpret;
		_UseLatencyInterpret	= !_UseLatencyInterpret;
		Print( "UseLetencyInterpret %s\n",
			_UseLatencyInterpret ? "ON" : "OFF" );
	}
};

/*
**
*/
class ToggleHitReticleConsoleFunctionClass : public ConsoleFunctionClass {
public:
	virtual	const char * Get_Name (void)	{return ("toggle_hit_reticle");}
	virtual	const char * Get_Help (void)	{return ("TOGGLE_HIT_RETICLE - Toggles the center reticle dot.");}
	virtual	void Activate (const char *input) {
		CombatManager::Toggle_Hit_Reticle_Enabled();
		Print( "Hit Reticle Toggled\n" );
	}
};


//------------------------------------------------------------------------------------
/*
** Console Function Manager
*/
DynamicVectorClass<ConsoleFunctionClass *>	ConsoleFunctionManager::FunctionList;

void	ConsoleFunctionManager::Init( void )
{

#ifdef WWDEBUG
   //
   // DEVELOPMENT CONSOLE FUNCTIONS ONLY
	// ONLY PRESENT IN DEBUG AND PROFILE BUILDS
   //

	FunctionList.Add( new AllowKillingHibernatingSpawnFunctionClass() );
	FunctionList.Add( new AmbientLightConsoleFunctionClass() );
	FunctionList.Add( new AmmoConsoleFunctionClass() );
	FunctionList.Add( new AppPacketTypesResetConsoleFunctionClass() );
	FunctionList.Add( new AshConsoleFunctionClass() );
	FunctionList.Add( new BreakExecutionConsoleFunctionClass() );
	FunctionList.Add( new CrashExecutionConsoleFunctionClass() );
	FunctionList.Add( new BuByeConsoleFunctionClass() );
	FunctionList.Add( new BuildingStateConsoleFunctionClass() );
	FunctionList.Add( new CameraConsoleFunctionClass() );
	FunctionList.Add( new CinematicFreezeConsoleFunctionClass() );
	FunctionList.Add( new ClearConsoleFunctionClass() );
	FunctionList.Add( new ClientHintFactorConsoleFunctionClass() );
	FunctionList.Add( new CloudsConsoleFunctionClass() );
	FunctionList.Add( new CopyLogsConsoleFunctionClass() );
	FunctionList.Add( new SkyTintConsoleFunctionClass() );
	FunctionList.Add( new CreateBotConsoleFunctionClass() );
	FunctionList.Add( new CreateGruntConsoleFunctionClass() );
	FunctionList.Add( new CreateObjectConsoleFunctionClass() );
	FunctionList.Add( new DazzleReloadConsoleFunctionClass() );
	FunctionList.Add( new DebugTypeConsoleFunctionClass() );
	FunctionList.Add( new DefectConsoleFunctionClass() );
	FunctionList.Add( new DisplayFindpathConsoleFunctionClass() );
	FunctionList.Add( new DisplayHibernatingFunctionClass() );
	FunctionList.Add( new DirtyCullDebugConsoleFunctionClass() );
   FunctionList.Add( new DonateConsoleFunctionClass() );
	FunctionList.Add( new DoStuffConsoleFunctionClass() );
	FunctionList.Add( new DSAPOResetConsoleFunctionClass() );
	FunctionList.Add( new EnableTriangleRenderConsoleFunctionClass() );
	FunctionList.Add( new ExposePrelitConsoleFunctionClass() );
	FunctionList.Add( new FlashTextureConsoleFunctionClass() );
	FunctionList.Add( new FlyStarConsoleFunctionClass() );
	FunctionList.Add( new FogRangeConsoleFunctionClass() );
	FunctionList.Add( new FogToggleConsoleFunctionClass() );
	FunctionList.Add( new FrameTexturesConsoleFunctionClass() );
	FunctionList.Add( new FreeWeaponsConsoleFunctionClass() );
	FunctionList.Add( new GiveWeaponsConsoleFunctionClass() );
	FunctionList.Add( new GodConsoleFunctionClass() );
	FunctionList.Add( new IBelieveInGodConsoleFunctionClass() );
	FunctionList.Add( new IrrelevancePenaltyConsoleFunctionClass() );
	FunctionList.Add( new GroupControllerTestConsoleFunctionClass() );
	FunctionList.Add( new HideStarConsoleFunctionClass() );
	FunctionList.Add( new HideHUDPointsFunctionClass() );
	FunctionList.Add( new InvalidateMeshCacheConsoleFunctionClass() );
	FunctionList.Add( new InvalidateTexturesConsoleFunctionClass() );
	FunctionList.Add( new InfoDebugToggle() );
	FunctionList.Add( new HUDConsoleFunctionClass() );
	FunctionList.Add( new KillConsoleFunctionClass() );
	FunctionList.Add( new ListConsoleFunctionClass() );
	FunctionList.Add( new LightModeConsoleFunctionClass() );
	FunctionList.Add( new LightDebugToggleConsoleFunctionClass() );
	FunctionList.Add( new LightCalcConsoleFunctionClass() );
	FunctionList.Add( new LightLODConsoleFunctionClass() );
	FunctionList.Add( new LightningConsoleFunctionClass() );
	FunctionList.Add( new LrshCommandConsoleFunctionClass() );
	FunctionList.Add( new LogMeshStatsConsoleFunctionClass() );
	FunctionList.Add( new LogTexturesConsoleFunctionClass() );
	FunctionList.Add( new MainMenuConsoleFunctionClass() );
	FunctionList.Add( new MaxFacingPenaltyConsoleFunctionClass() );
	FunctionList.Add( new MeshDebuggerDisableMeshConsoleFunctionClass() );
	FunctionList.Add( new MeshDebuggerEnableConsoleFunctionClass() );
	FunctionList.Add( new MeshDebuggerEnableMeshConsoleFunctionClass() );
	FunctionList.Add( new MeshRenderSnapshotFunctionClass() );
	FunctionList.Add( new MissionConsoleFunctionClass() );
	FunctionList.Add( new MoneyConsoleFunctionClass() );
	FunctionList.Add( new MultiHudConsoleFunctionClass() );
	FunctionList.Add( new NextRenderConsoleFunctionClass() );
	FunctionList.Add( new OneShotKillsConsoleFunctionClass() );
	FunctionList.Add( new OpenConsoleFunctionClass() );
	FunctionList.Add( new Phys3NetConsoleFunctionClass() );
	FunctionList.Add( new PhysicsDebugConsoleFunctionClass() );
	FunctionList.Add( new PlayerPositionConsoleFunctionClass() );
	FunctionList.Add( new ProfileCollectBeginConsoleFunctionClass() );
	FunctionList.Add( new ProfileCollectEndConsoleFunctionClass() );
	FunctionList.Add( new ProjectorDebugConsoleFunctionClass() );
	//FunctionList.Add( new RadarMaxConsoleFunctionClass() );
	FunctionList.Add( new RadarToggleConsoleFunctionClass() );
	//FunctionList.Add( new RadarZoomConsoleFunctionClass() );
	FunctionList.Add( new RBodyNetConsoleFunctionClass() );
	FunctionList.Add( new RenderBackfacesConsoleFunctionClass() );
	FunctionList.Add( new RenderBoxesConsoleFunctionClass() );
	FunctionList.Add( new RenderModeConsoleFunctionClass() );
	FunctionList.Add( new RenderModeExtraPassConsoleFunctionClass() );
	FunctionList.Add( new ResolutionConsoleFunctionClass() );
	FunctionList.Add( new RunFastConsoleFunctionClass() );
	FunctionList.Add( new ParticleToggleConsoleFunctionClass() );
	FunctionList.Add( new ProfileConsoleFunctionClass() );
	FunctionList.Add( new QuickStatsConsoleFunctionClass() );
	FunctionList.Add( new RainConsoleFunctionClass() );
	//FunctionList.Add( new RshCommandConsoleFunctionClass() );
	FunctionList.Add( new ScaleTimeConsoleFunctionClass() );
	FunctionList.Add( new ScoreConsoleFunctionClass() );
	FunctionList.Add( new SnowConsoleFunctionClass() );
	FunctionList.Add( new SrshCommandConsoleFunctionClass() );
	FunctionList.Add( new DebugObjectivesConsoleFunctionClass() );
	FunctionList.Add( new CnCModeConsoleFunctionClass() );
	FunctionList.Add( new HUDHelpTextTestConsoleFunctionClass() );
	FunctionList.Add( new DebugEncyRevealConsoleFunctionClass() );
	FunctionList.Add( new DebugShowMoveVectorConsoleFunctionClass() );
	FunctionList.Add( new ChangeCharacterClassConsoleFunctionClass() );
	FunctionList.Add( new MapRevealConsoleFunctionClass() );
	FunctionList.Add( new ScriptTraceConsoleFunctionClass() );
	FunctionList.Add( new SetBandwidthBudgetOutToggleConsoleFunctionClass() );
	FunctionList.Add( new SetDesiredSleepConsoleFunctionClass() );
	FunctionList.Add( new SetPacketDuplicationConsoleFunctionClass() );
	FunctionList.Add( new SetPacketLatencyRangeConsoleFunctionClass() );
	FunctionList.Add( new SetPacketLossConsoleFunctionClass() );
	FunctionList.Add( new SetSpamCountConsoleFunctionClass() );
	FunctionList.Add( new SetStarHealthConsoleFunctionClass() );
	FunctionList.Add( new SetStarSkinConsoleFunctionClass() );
	FunctionList.Add( new SetStarShieldStrengthConsoleFunctionClass() );
	FunctionList.Add( new SetStarShieldTypeConsoleFunctionClass() );
	FunctionList.Add( new SetTheStarConsoleFunctionClass() );
	FunctionList.Add( new ShadowAttenuationConsoleFunctionClass() );
	FunctionList.Add( new ShadowBlobTextureConsoleFunctionClass() );
	FunctionList.Add( new ShadowCountConsoleFunctionClass() );
	FunctionList.Add( new ShadowElevationConsoleFunctionClass() );
	FunctionList.Add( new ShadowIntensityConsoleFunctionClass() );
	FunctionList.Add( new ShadowPerPolyCullingConsoleFunctionClass() );
	FunctionList.Add( new ShadowResolutionConsoleFunctionClass() );
	FunctionList.Add( new ShellCommandConsoleFunctionClass() );
	FunctionList.Add( new SkeletonSliderDemoConsoleFunctionClass() );
	FunctionList.Add( new SkeletonStandersConsoleFunctionClass() );
	FunctionList.Add( new SnapShotModeToggleFunctionClass() );
	FunctionList.Add( new SoldierGhostDebugDisplayConsoleFunctionClass() );
	FunctionList.Add( new StarVisibilityConsoleFunctionClass() );
	FunctionList.Add( new StatsConsoleFunctionClass() );
	//FunctionList.Add( new StealthConsoleFunctionClass() );
	FunctionList.Add( new SuicideConsoleFunctionClass() );
	FunctionList.Add( new SurfaceEffectsDebugConsoleFunctionClass() );
	FunctionList.Add( new SurfaceTypeOverrideConsoleFunctionClass() );
	FunctionList.Add( new SurfacePhysicsOverrideConsoleFunctionClass() );
	FunctionList.Add( new SwapTeamsConsoleFunctionClass() );
	FunctionList.Add( new TargetSteeringToggleConsoleFunctionClass() );
	FunctionList.Add( new TeleportStarConsoleFunctionClass() );
	FunctionList.Add( new TextureFilterModeConsoleFunctionClass() );
	FunctionList.Add( new TextureMemoryCounterConsoleFunctionClass() );
	FunctionList.Add( new TexturingEnableConsoleFunctionClass() );
	FunctionList.Add( new TimeOfDayConsoleFunctionClass() );
	FunctionList.Add( new ToggleAssetPreloadingConsoleFunctionClass() );
	FunctionList.Add( new ToggleEmotIconsConsoleFunctionClass() );
	FunctionList.Add( new ToggleHitReticleConsoleFunctionClass() );
	FunctionList.Add( new ToggleUseLetencyInterpretConsoleFunctionClass() );
	FunctionList.Add( new ToggleSimulationConsoleFunctionClass() );
	FunctionList.Add( new ToggleRenderingConsoleFunctionClass() );
	FunctionList.Add( new TomConsoleFunctionClass() );
	FunctionList.Add( new UmbraToggleFunctionClass() );
	FunctionList.Add( new UnusedSpawnerFunctionClass() );
	FunctionList.Add( new VipConsoleFunctionClass() );
	FunctionList.Add( new VisConsoleFunctionClass() );
	FunctionList.Add( new VisInvertConsoleFunctionClass() );
	FunctionList.Add( new VisGridDisplayConsoleFunctionClass() );
	FunctionList.Add( new VisGridDebugConsoleFunctionClass() );
	FunctionList.Add( new VisHierarchicalConsoleFunctionClass() );
	FunctionList.Add( new VisLockConsoleFunctionClass() );
	FunctionList.Add( new VisSectorDisplayConsoleFunctionClass() );
	FunctionList.Add( new VisSectorHistoryConsoleFunctionClass() );
	FunctionList.Add( new VblankSyncConsoleFunctionClass() );
	FunctionList.Add( new WatchConsoleFunctionClass() );
	FunctionList.Add( new WarBlitzConsoleFunctionClass() );
	FunctionList.Add( new WarpConsoleFunctionClass() );
	FunctionList.Add( new WhoIsConsoleFunctionClass() );
	FunctionList.Add( new WindConsoleFunctionClass() );
	FunctionList.Add( new WOLConsoleFunctionClass() );
	FunctionList.Add( new DataSafeStatsConsoleFunctionClass() );
	FunctionList.Add( new PatsDebugConsoleFunctionClass() );
	FunctionList.Add( new SetSendFrequencyConsoleFunctionClass() );
	FunctionList.Add( new TogglePacketDeltasConsoleFunctionClass() );
	FunctionList.Add( new TogglePacketComboConsoleFunctionClass() );
	FunctionList.Add( new TogglePacketOptConsoleFunctionClass() );
	FunctionList.Add( new SetLatencyConsoleFunctionClass() );
	FunctionList.Add( new ToggleNewClientUpdateMethodConsoleFunctionClass() );
	FunctionList.Add( new ToggleBandwidthBalancerConsoleFunctionClass() );
	FunctionList.Add( new NewTCADOConsoleFunctionClass() );

   FunctionList.Add( new DebugDeviceConsoleFunctionClass() );
	FunctionList.Add( new StatsConsoleFunctionClass() );
	FunctionList.Add( new ToggleSurfaceEffectsConsoleFunctionClass() );

	FunctionList.Add( new DeviceInfoConsoleFunctionClass() );

#endif // WWDEBUG (development commands)

   //
   // SHIPPING CONSOLE FUNCTIONS ONLY
   //

	FunctionList.Add( new GameOverConsoleFunctionClass() );
   FunctionList.Add( new AdminMessageConsoleFunctionClass() );
	FunctionList.Add( new GameInfoConsoleFunctionClass() );
	FunctionList.Add( new KickConsoleFunctionClass() );
	FunctionList.Add( new AllowConsoleFunctionClass() );

	FunctionList.Add( new BanConsoleFunctionClass() );
   FunctionList.Add( new MessageConsoleFunctionClass() );
	FunctionList.Add( new PlayerInfoConsoleFunctionClass() );
	FunctionList.Add( new QuitConsoleFunctionClass() );
	FunctionList.Add( new QuitSlaveConsoleFunctionClass() );
	FunctionList.Add( new RestartConsoleFunctionClass() );
	FunctionList.Add( new ScreenUVBiasConsoleFunctionClass() );
	FunctionList.Add( new SetBandwidthBudgetOutConsoleFunctionClass() );
	FunctionList.Add( new ToggleSortingConsoleFunctionClass() );
	FunctionList.Add( new ExtrasConsoleFunctionClass() );  /// CHEATS? MAY NEED TO BE DEV ONLY!!!!
	FunctionList.Add( new EditVehicleConsoleFunctionClass() );
	FunctionList.Add( new NetUpdateRateConsoleFunctionClass() );
	FunctionList.Add( new ClientPhysicsOptimizationConsoleFunctionClass() );
#ifndef FREEDEDICATEDSERVER
	FunctionList.Add( new FPSConsoleFunctionClass() );		// Steve W wanted this.
#endif //FREEDEDICATEDSERVER
#ifdef FREEDEDICATEDSERVER
	FunctionList.Add( new PageConsoleFunctionClass() );
#endif //FREEDEDICATEDSERVER


	SystemSettings::Add_Console_Functions( FunctionList );

	Sort_Function_List();

	//
	// Dump list to a file
	//
	Verbose_Help_File();
}

void	ConsoleFunctionManager::Shutdown( void )
{
	while ( FunctionList.Count() ) {
		delete FunctionList[ FunctionList.Count()-1 ];
		FunctionList.Delete( FunctionList.Count()-1 );
	}
}

//------------------------------------------------------------------------------------

void	ConsoleFunctionManager::Sort_Function_List( void )
{
	// ARGH!! Bubblesort!!!
	bool done = false;
	int count = FunctionList.Count();
	while ( !done ) {
		done = true;
		count--;
		for (	int index = 0; index < count; index++) {
			if ( stricmp(	FunctionList[index]->Get_Name(),
								FunctionList[index+1]->Get_Name() ) > 0 ) {
				ConsoleFunctionClass * temp = FunctionList[index];
				FunctionList[index] = FunctionList[index+1];
				FunctionList[index+1] = temp;
				done = false;
			}
		}
	}
}

//------------------------------------------------------------------------------------

void	ConsoleFunctionManager::Help(	const char * function_name )
{
	char string[120];

	// Eat leading spaces
	while ( function_name && *function_name == ' ' ) {
		function_name++;
	}

	if ( ( function_name == NULL ) || ( function_name[0] == 0 ) ) {
		// Show name of each function...
      Print( "------ Available commands -------\n" );
		*string = 0;
		for (	int index = 0; index < FunctionList.Count(); index++) {
			ConsoleFunctionClass * function = FunctionList[index];
			if ( *string ) {
				strcat( string, ", " );
			}
			strcat( string, function->Get_Name() );
			if ( ( strlen( string ) > (sizeof( string ) / 2) ) || (index == FunctionList.Count()-1) ) {
				strcat( string, "\n" );
				Print( string );
				*string = 0;
			}
		}
	} else {
		// Show help for a certian function
		for (	int index = 0; index < FunctionList.Count(); index++) {
			ConsoleFunctionClass * function = FunctionList[index];
			if ( !stricmp( function->Get_Name(), function_name ) ) {
		      Print( function->Get_Help() );
				return;
			}
		}
		Print( "Help not found for \"%s\"\n", function_name );
	}
}

//------------------------------------------------------------------------------------
int ConsoleFunctionManager::Get_Function_Count(void)
{
	return FunctionList.Count();
}

//------------------------------------------------------------------------------------
void ConsoleFunctionManager::Verbose_Help_File(void)
{
	//
	// Log function list to a file
	//
   FILE * file = fopen("commands.txt", "w");
	if ( file ) {
		char buffer[500];
		sprintf(buffer, "RENEGADE COMMANDS as at %s\n\n", cMiscUtil::Get_Text_Time());
		fwrite(buffer, 1, strlen(buffer), file);
		for (	int index = 0; index < FunctionList.Count(); index++) {
			ConsoleFunctionClass * function = FunctionList[index];
			WWASSERT(function != NULL);
			sprintf(buffer, "%s", function->Get_Help());
			if (function->Get_Alias() != NULL) {
				char alias_string[100];
				sprintf(alias_string, " (%s)", function->Get_Alias());
				strcat(buffer, alias_string);
			}
			strcat(buffer, "\n");
			fwrite(buffer, 1, strlen(buffer), file);
		}

#ifdef WWDEBUG
		char watch_help_buffer[5000];
      WatchConsoleFunctionClass::Get_Verbose_Help(watch_help_buffer, sizeof(watch_help_buffer));
		fwrite(watch_help_buffer, 1, strlen(watch_help_buffer), file);
#endif

		fclose(file);
	}
}

//------------------------------------------------------------------------------------
void ConsoleFunctionManager::Next_Verbose_Help_Screen(void)
{
	//
	// Show name of each function on a separate line...
	//
	char buffer[5000];
	buffer[0] = 0;

	static int page_number = -1;
	page_number++;

	if ( page_number >= 0 ) {
		int command_number = 0;
		for (	int index = 0; index < FunctionList.Count(); index++) {
			command_number++;
			int per_page = 57;
			if (floor(command_number / per_page) == page_number) {
				ConsoleFunctionClass * function = FunctionList[index];
				WWASSERT(function != NULL);
				strcat( buffer, function->Get_Help() );
				strcat( buffer, "\n" );

				if ( index == FunctionList.Count() - 1 ) {	// if this is the last line,
					page_number = -2;				// turn it off for next time

#ifdef WWDEBUG
					//
               // Also, show help for watch objects
               //
               strcat( buffer, "\n" );
               char watch_help_buffer[10000];
               WatchConsoleFunctionClass::Get_Verbose_Help(watch_help_buffer, sizeof(watch_help_buffer));
   				strcat( buffer, watch_help_buffer );
#endif // WWDEBUG
				}
			}
		}
	}
	if (Get_Text_Display()) {
		Get_Text_Display()->Set_Verbose_Help_Text( buffer );
	}
}

//------------------------------------------------------------------------------------
void	ConsoleFunctionManager::Parse_Input( const char * string )
{
	if (!strnicmp(string, "help", 4)) {
		Help( string + 4 );
	} else {
		for (	int index = 0; index < FunctionList.Count(); index++) {
			ConsoleFunctionClass * function = FunctionList[index];

         if ( !strnicmp( string, function->Get_Name(), strlen( function->Get_Name() ) ) ) {

				const char *s = string + strlen (function->Get_Name());

				// The next input character must be either a string terminator or space character
				// in order for the input string to match the function list string.
				if ((*s == '\0') || (*s == ' ')) {

					// Remove any trailing white space.
					while (*s == ' ') s++;

					function->Activate (s);
					return;
				}
         }

         //
         // Check short-form alias too
         // There has to be a space (or nothing) following the alias for
         // it to match
         //
         if (function->Get_Alias() != NULL &&
            strlen(function->Get_Alias()) > 0) {
            char alias[100];
            WWASSERT(strlen(function->Get_Alias()) < sizeof(alias));
            strcpy(alias, function->Get_Alias());
            int alias_len = strlen(alias);
            int string_len = strlen(string);
            if (!strnicmp(string, alias, alias_len) &&
               (string_len == alias_len || string[alias_len] == ' ')) {
				   string += alias_len;
				   while ( string && *string == ' ' ) string++;
				   function->Activate( string );
				   return;
            }
         }
		}

		if (ConsoleBox.Is_Exclusive()) {
      	Print( "\"%s\" is not a command. To send a message, use \"msg\" and \"amsg\"\n", string );
		} else {
      	Print( "\"%s\" is not a command. To send a message, Use F2 and F3\n", string );
		}
      Print( "For a list of available commands, use the \"help\" command !\n" );
	}
}

bool ConsoleFunctionManager::Get_Command_Suggestion
(
	const char *	input,					// what user has typed,
	const char *	cur_suggestion,		// last suggestion we gave (move to next if not null)
	char *			set_suggestion,		// copy suggestion into this string
	char *			set_help,				// copy help into this string
	int				len						// don't copy more than this many characters.
)
{
	ConsoleFunctionClass * function = NULL;
	int	node_index = 0;

	/*
	** If user is passing in a 'cur_suggestion', then we need to first find that
	** function in the list and search past it.  If we don't find a suggestion, we
	** will drop through and search from the head of the list.
	*/
	if (cur_suggestion != NULL) {

		node_index = Find_Function_Node(cur_suggestion);

		/*
		** Try to find a suggestion after this node.
		*/
		if (node_index != -1) {
			function = Find_Command_Suggestion(input,node_index+1);
		}
	}

	/*
	** Find a suggestion starting at the head of the list
	*/
	if (function == NULL) {
		function = Find_Command_Suggestion(input,0);
	}

	/*
	** If we found a suggestion, copy its command into 'set_suggestion' and its help
	** into 'set_help'
	*/
	if (function != NULL) {
		if (set_suggestion != NULL) {
			strncpy(set_suggestion,function->Get_Name(),len);
		}
		if (set_help != NULL) {
			strncpy(set_help,function->Get_Help(),len);
		}
		return true;
	}
	return false;
}

int ConsoleFunctionManager::Find_Function_Node(const char * name)
{
	for (	int index = 0; index < FunctionList.Count(); index++) {
		ConsoleFunctionClass * function = FunctionList[index];
		if (stricmp(name,function->Get_Name()) == 0) {
			return index;
		}
	}
	return -1;
}

ConsoleFunctionClass * ConsoleFunctionManager::Find_Command_Suggestion
(
	const char *	input,		// incomplete function name
	int				start_index	// node to start searching from
)
{
	for (	int index = start_index; index < FunctionList.Count(); index++) {
		ConsoleFunctionClass * function = FunctionList[index];
		if (_strnicmp(input,function->Get_Name(),strlen(input)) == 0) {
			return function;
		}
	}
	return NULL;
}

void	ConsoleFunctionManager::Print( const char *format, ... )
{
	va_list arg_list;
	va_start (arg_list, format);
	StringClass string;
	string.Format_Args( format, arg_list );
	if (Get_Text_Display()) {
		WWASSERT( Get_Text_Display() );
		Get_Text_Display()->Print_System( string );
	}
	ConsoleBox.Print(string.Peek_Buffer());
	va_end (arg_list);
}

