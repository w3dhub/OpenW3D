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
 *                     $Archive:: /Commando/Code/Commando/cnetwork.cpp                         $*
 *                                                                                             *
 *                      $Author:: Byon_g                                                     $*
 *                                                                                             *
 *                     $Modtime:: 3/29/02 5:02p                                               $*
 *                                                                                             *
 *                    $Revision:: 152                                                        $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "cnetwork.h"

#include <shellapi.h>
#include <stdio.h>

#include "specialbuilds.h"

#include "langmode.h"
#include "wolgmode.h"
#include "playermanager.h"
#include "textdisplay.h"
#include "gameobjmanager.h"
#include "multihud.h"
#include "WWAudio.H"
#include "useroptions.h"
#include "devoptions.h"
#include "translatedb.h"
#include "string_ids.h"
#include "msgstatlistgroup.h"
#include "wwprofile.h"
#include "gametype.h"
#include "crc.h"
#include "render2d.h"
#include "wwmemlog.h"
#include "svrgoodbyeevent.h"
#include "gameoptionsevent.h"
#include "evictionevent.h"
#include "changeteamevent.h"
#include "clientcontrol.h"
#include "serverfps.h"
#include "sbbomanager.h"
#include "clientgoodbyeevent.h"
//#include "helptext.h"
#include	"natter.h"
#include	"vistable.h"
#include "gameinitmgr.h"
#include "dlgmessagebox.h"
#include "apppacketstats.h"
#include "clientfps.h"
#include "gamechanlist.h"
#include "packetmgr.h"
#include "clientpingmanager.h"
#include "bandwidthgraph.h"
#include "buildnum.h"
#include "messagewindow.h"
#include "wwmemlog.h"
#include "consolemode.h"
#include "slavemaster.h"
#include "gamedataupdateevent.h"
#include "gamespyadmin.h"
#include "demosupport.h"
#include "serversettings.h"
#include "dlgmpconnectionrefused.h"

#include "Resource.h"
#include <WWUI\DialogMgr.h>
#include "ffactory.h"
#include "realcrc.h"

extern bool g_is_loading;

//
// Networking cNetwork statics
//
char												cNetwork::MessageToSend[];
char												cNetwork::Command[];
int												cNetwork::ExeKey								= 0;
int												cNetwork::ExeCRC								= 0;
int												cNetwork::StringsCRC							= 0;
char												cNetwork::ClientString[];
char												cNetwork::ClientEnumerationString[];
CombatNetworkReceiverInstanceClass *	cNetwork::NetworkReceiver					= NULL;
GameCombatNetworkHandlerClass				cNetwork::NetHandler;
int												cNetwork::Fps									= 0;
int												cNetwork::ThinkCount							= 0;
cMsgStatList *									cNetwork::PClientStatList					= NULL;
cMsgStatListGroup *							cNetwork::PServerStatListGroup			= NULL;
cConnection *									cNetwork::PClientConnection				= NULL;
cConnection *									cNetwork::PServerConnection				= NULL;
CombatNetworkReceiver *						cNetwork::Receiver							= NULL;
float												cNetwork::GraphingY;
float												cNetwork::BandwidthBarLength				= 0;
//int												cNetwork::BandwidthScaler					= 28800;
int												cNetwork::BandwidthScaler					= 50000;

bool												cNetwork::HaveDoneTeamChangeDialog		= false;

bool												cNetwork::HaveDoneMotdDialog				= false;
VisTableClass *								cNetwork::VisTable							= NULL;
bool												cNetwork::LastServerConnectionStateBad = false;
bool												cNetwork::SensibleUpdates					= true;

//-----------------------------------------------------------------------------
void cNetwork::Init_Client(unsigned short my_port)
{
	WWMEMLOG(MEM_NETWORK);

#ifndef FREEDEDICATEDSERVER

   WWDEBUG_SAY(("cNetwork::Init_Client\n"));

	if (PClientConnection != NULL) {
		Cleanup_Client();
	}

	PClientStatList = new cMsgStatList;
	WWASSERT(PClientStatList != NULL);
	//PClientStatList->Init(MESSAGE_COUNT);
	PClientStatList->Init(1);
	for (int i = 0; i < PClientStatList->Get_Num_Stats(); i++) {
		//PClientStatList->Set_Name(i, Message_Type_Translation(i));

		/*
		char message_trans[200];
		strcpy(message_trans, Message_Type_Translation(i));
		PClientStatList->Set_Name(i, &message_trans[8]);
		*/
		PClientStatList->Set_Name(i, "message");
	}

   WWASSERT(PClientConnection == NULL);
   PClientConnection = new cConnection;
   WWASSERT(PClientConnection != NULL);

	CombatManager::Set_I_Am_Client(true);

	PClientConnection->Install_Accept_Handler(Accept_Handler);
	PClientConnection->Install_Refusal_Handler(Refusal_Handler);
	PClientConnection->Install_Client_Broken_Connection_Handler(Client_Broken_Connection_Handler);
	PClientConnection->Install_Client_Packet_Handler(Client_Packet_Handler);

	if (cGameSpyAdmin::Is_Gamespy_Game()) {
		WWASSERT(PTheGameData != NULL);
		The_Game()->Set_Password(cGameSpyAdmin::Get_Password_Attempt());
	}

	ULONG bbo = 0;
	//if (IS_SOLOPLAY || GameModeManager::Find("LAN")->Is_Active()) {
	if (IS_SOLOPLAY || 
		 (GameModeManager::Find("LAN")->Is_Active() && !cGameSpyAdmin::Is_Gamespy_Game())) {

		bbo = cBandwidth::Get_Bandwidth_Bps_From_Type(BANDWIDTH_LANT1);

		WWASSERT(bbo > 0);
		cBandwidthGraph::Set_Scale(200000);

		HaveDoneTeamChangeDialog = false;
	} else {
		//WWASSERT(GameModeManager::Find("WOL")->Is_Active());
		bbo = cBandwidth::Get_Bandwidth_Bps_From_Type((BANDWIDTH_TYPE_ENUM)cUserOptions::Get_Bandwidth_Type());
		//bbo = cUserOptions::BandwidthBps.Get();
		WWASSERT(bbo > 0);

		int bw_scale = (bbo * 2) / 10;
		bw_scale = (bw_scale / 1000) * 1000;
		cBandwidthGraph::Set_Scale(bw_scale);

		if (GameModeManager::Find("WOL")->Is_Active()) {
			HaveDoneTeamChangeDialog = true;
		}
	}

	PClientConnection->Set_Bandwidth_Budget_Out(bbo);

   BOOL is_flow_control_enabled = !IS_SOLOPLAY;
   PClientConnection->Enable_Flow_Control(is_flow_control_enabled);

	WWASSERT(PTheGameData != NULL);
   PClientConnection->Init_As_Client(
		The_Game()->Get_Ip_Address(), The_Game()->Get_Port(), my_port);

	//
	// This packet resurfaces on the server in Application_Acceptance_Handler
	//

	cPacket packet;
   packet.Add_Wide_Terminated_String(cNetInterface::Get_Nickname());
   packet.Add_Wide_Terminated_String(The_Game()->Get_Password(), true);
	packet.Add(ExeKey);
	packet.Add(bbo); // note, this field is consumed by wwnet

   PClientConnection->Connect_Cs(packet);
	packet.Flush();

	/*
	if (I_Am_Only_Client()) {
		ServerFps.Init();
	}
	*/

	HaveDoneMotdDialog = false;

	if (I_Am_Only_Client())
	{
		cAppPacketStats::Reset();
	}
	LastServerConnectionStateBad = false;

	cClientPingManager::Init();

#endif // !FREEDEDICATEDSERVER

}

//-----------------------------------------------------------------------------
void cNetwork::Cleanup_Client(void)
{
	WWMEMLOG(MEM_NETWORK);
#ifndef FREEDEDICATEDSERVER

   WWDEBUG_SAY(("cNetwork::Cleanup_Client\n"));

   if (I_Am_Client()) {

      if (PClientConnection->Is_Established()) { // i.e we did have a connection

			cClientGoodbyeEvent * p_event = new cClientGoodbyeEvent;
			p_event->Init();
			Flush();
      }

      delete PClientConnection;
      PClientConnection = NULL;
   }

	CombatManager::Set_I_Am_Client(false);

	delete PClientStatList;
	PClientStatList = NULL;

	delete PClientControl;
	PClientControl = NULL;

	delete PClientFps;
	PClientFps = NULL;

#endif // !FREEDEDICATEDSERVER
}

//-----------------------------------------------------------------------------
void cNetwork::Accept_Handler(void)
{
	WWMEMLOG(MEM_NETWORK);
#ifndef FREEDEDICATEDSERVER

   WWDEBUG_SAY(("cNetwork::Accept_Handler\n"));

   WWASSERT(I_Am_Client());

	CombatManager::Set_My_Id(Get_My_Id());

	NetworkObjectMgrClass::Init_New_Client_ID(Get_My_Id());

	if (I_Am_Only_Client()) {

		//
		// Create C->S mirrored client control object
		//
		WWASSERT(PClientControl == NULL);
		PClientControl = new CClientControl;
		PClientControl->Init();

		//
		// Create C->S mirrored client fps object
		//
		WWASSERT(PClientFps == NULL);
		PClientFps = new CClientFps;
		PClientFps->Init();
	}


   if (!I_Am_Server()) {
      if (GameModeManager::Find("LAN")->Is_Active()) {
         PLC->Accept_Actions();
      } else {
			 GameModeClass* gameMode = GameModeManager::Find("WOL");

			 if (gameMode && gameMode->Is_Active()) {
				 WolGameModeClass* wolGame = static_cast<WolGameModeClass*>(gameMode);
				 WWASSERT(wolGame);
				 wolGame->Accept_Actions();
			 }
      }
   }

#endif // !FREEDEDICATEDSERVER
}

//-----------------------------------------------------------------------------
void cNetwork::Refusal_Handler(REFUSAL_CODE refusal_code)
{
#ifndef FREEDEDICATEDSERVER

   WWDEBUG_SAY(("cNetwork::Refusal_Handler\n"));

	// Close connecting dialog as neccessary.
	DialogBaseClass* dialog = DialogMgrClass::Find_Dialog(IDD_MULTIPLAY_CONNECTING);

	if (dialog != NULL) {
		// Sending 1 as the parameter tells the dialog that it is being closed
		// as a result of a refusal from the server.
 		dialog->On_Command(IDCANCEL, 0, 1);
	}

	 // Verify refusal code is within valid range
	 WWASSERT(REFUSAL_CLIENT_ACCEPTED <= refusal_code && REFUSAL_BY_APPLICATION >= refusal_code);

   WWASSERT(I_Am_Client());

   if (GameModeManager::Find("LAN")->Is_Active()) {
      PLC->Refusal_Actions();
   } else {
		 GameModeClass* gameMode = GameModeManager::Find("WOL");

		 if (gameMode && gameMode->Is_Active()) {
			 WolGameModeClass* wolGame = static_cast<WolGameModeClass*>(gameMode);
			 WWASSERT(wolGame);
			 wolGame->Refusal_Actions();
		 }
   }

   //
   // The server refused our connection request! At this stage this
   // is fatal. Later on we would have to re-init the connection etc.
   //
	static const unsigned long _refusalStrings[] = {
		IDS_MP_CONNECTION_REFUSED_GAME_FULL,      // REFUSAL_GAME_FULL
		IDS_MP_PASSWORD_WRONG, // REFUSAL_BAD_PASSWORD
		IDS_MENU_VERSION_MISMATCH, // REFUSAL_VERSION_MISMATCH
		IDS_MP_CONNECTION_REFUSED_BY_APPLICATION, // REFUSAL_PLAYER_EXISTS
		IDS_MP_CONNECTION_REFUSED_BY_APPLICATION  // REFUSAL_BY_APPLICATION
		};

	const unsigned long refusalMsg = _refusalStrings[refusal_code - 1];

	if (cGameSpyAdmin::Is_Gamespy_Game()) {
		if (refusal_code == REFUSAL_VERSION_MISMATCH) {
			WideStringClass tval;
			tval.Format(L"%s...%s", TRANSLATE(IDS_MP_CONNECTION_REFUSED_BY_APPLICATION), 
				TRANSLATE(IDS_MENU_VERSION_MISMATCH));
			DlgMPConnectionRefused::DoDialog(tval, false);
		} else {
			DlgMPConnectionRefused::DoDialog(TRANSLATE(refusalMsg), false);
		}
	} else {
		DlgMsgBox::DoDialog(TRANSLATE (IDS_MENU_SERVER_MESSAGE_TITLE), TRANSLATE(refusalMsg));
	}
	
	
	//
	// N.B. We cannot destroy the connection from inside this callback.
	//

#endif // !FREEDEDICATEDSERVER
}

//-----------------------------------------------------------------------------
int cNetwork::Get_Data_Files_CRC(void)
{
#define	UNINITIALLIZED_CRC	0x4592abf1
	static int crc = UNINITIALLIZED_CRC;
	if ( crc == UNINITIALLIZED_CRC ) {
		const char * filelist[] = {
		"jgo`fqv+aag",					//"objects.ddb",           
		"dwhjw+lkl",					//"armor.ini",             
		"gjk`v+lkl",					//"bones.ini",             
		"vpwcdf``cc`fqv+lkl",		//"surfaceeffects.ini",    
		"fdh`wdv+lkl",					//"cameras.ini",           
		"fZkjaZhbZi5+r6a",			//"c_nod_mg_l0.w3d",       
		"fZkjaZwnZi5+r6a",			//"c_nod_rk_l0.w3d",       
		"fZkjaZciZi5+r6a",			//"c_nod_fl_l0.w3d",       
		"fZkjaZ`kZi5+r6a",			//"c_nod_en_l0.w3d",       
		"fZkjaZhbjZi5+r6a",			//"c_nod_mgo_l0.w3d",      
		"fZkjaZwnjZi5+r6a",			//"c_nod_rko_l0.w3d",      
		"fZkjaZfm`hqZi5+r6a",		//"c_nod_chemt_l0.w3d",    
		"fZkjaZvklu`wZi5+r6a",		//"c_nod_sniper_l0.w3d",   
		"fZkjaZwvjiaZi5+r6a",		//"c_nod_rsold_l0.w3d",    
		"fZkjaZvqiqmZi5+r6a",		//"c_nod_stlth_l0.w3d",    
		"fZkjaZvdnpZi5+r6a",			//"c_nod_saku_l0.w3d",     
		"fZkjaZvdnp7Zi5+r6a",		//"c_nod_saku2_l0.w3d",    
		"fZkjaZwdsZi5+r6a",			//"c_nod_rav_l0.w3d",      
		"fZkjaZhwdsZi5+r6a",			//"c_nod_mrav_l0.w3d",     
		"fZkjaZhaZi5+r6a",			//"c_nod_mdz_l0.w3d",      
		"fZkjaZha7Zi5+r6a",			//"c_nod_mdz2_l0.w3d",     
		"fZkjaZqfZi5+r6a",			//"c_nod_tc_l0.w3d",       
		"fZkjaZhpqdkqZi5+r6a",		//"c_nod_mutant_l0.w3d",   
		"fZkjaZhviaZi5+r6a",			//"c_nod_msld_l0.w3d",     
		"fZkjaZvvjiaZi5+r6a",		//"c_nod_ssold_l0.w3d",    
		"fZkjaZu`qhZi5+r6a",			//"c_nod_petm_l0.w3d",     
		"fZkjaZndk`Zi5+r6a",			//"c_nod_kane_l0.w3d",     
		"fZbalZhbZi5+r6a",			//"c_gdi_mg_l0.w3d",       
		"fZbalZwnZi5+r6a",			//"c_gdi_rk_l0.w3d",       
		"fZbalZbwZi5+r6a",			//"c_gdi_gr_l0.w3d",       
		"fZbalZ`kZi5+r6a",			//"c_gdi_en_l0.w3d",       
		"fZbalZhbjZi5+r6a",			//"c_gdi_mgo_l0.w3d",      
		"fZbalZwnjZi5+r6a",			//"c_gdi_rko_l0.w3d",      
		"fZbalZv|aZi5+r6a",			//"c_gdi_syd_l0.w3d",      
		"fZbalZa`daZi5+r6a",			//"c_gdi_dead_l0.w3d",     
		"fZbalZbpkZi5+r6a",			//"c_gdi_gun_l0.w3d",      
		"fZbalZuqfmZi5+r6a",			//"c_gdi_ptch_l0.w3d",     
		"fZmdsjfZi5+r6a",				//"c_havoc_l0.w3d",        
		"fZmdsjfkZi5+r6a",			//"c_havocn_l0.w3d",       
		"fZmdsjfrZi5+r6a",			//"c_havocw_l0.w3d",       
		"fZmdsjfaZi5+r6a",			//"c_havocd_l0.w3d",       
		"fZbalZv|aZi5+r6a",			//"c_gdi_syd_l0.w3d",      
		"fZbalZv|a7Zi5+r6a",			//"c_gdi_syd2_l0.w3d",     
		"fZbalZhjglZi5+r6a",			//"c_gdi_mobi_l0.w3d",     
		"fZbalZmjqrZi5+r6a",			//"c_gdi_hotw_l0.w3d",     
		"fZbalZiqZi5+r6a",			//"c_gdi_lt_l0.w3d",       
		"fZkjaZu`qwZi5+r6a",			//"c_nod_petr_l0.w3d",     
		"fZijbdkZi5+r6a",				//"c_logan_l0.w3d",        
		"fZbalZijfn`Zi5+r6a",		//"c_gdi_locke_l0.w3d",    
		"sZkjaZgpbb|+r6a",			//"v_nod_buggy.w3d",       
		"sZkjaZdufZh+r6a",			//"v_nod_apc_m.w3d",       
		"sZkjaZdwqiw|+r6a",			//"v_nod_artlry.w3d",      
		"sZkjaZcidh`+r6a",			//"v_nod_flame.w3d",       
		"sZkjaZiqdkn+r6a",			//"v_nod_ltank.w3d",       
		"sZkjaZvqiqm+r6a",			//"v_nod_stlth.w3d",       
		"sZkjaZqwkvuqZh+r6a",		//"v_nod_trnspt_m.w3d",    
		"sZkjaZdudfm`Zh+r6a",		//"v_nod_apache_m.w3d",    
		"sZfmdh`i`jk+r6a",			//"v_chameleon.w3d",       
		"sZbalZmphs``+r6a",			//"v_gdi_humvee.w3d",      
		"sZbalZdufZh+r6a",			//"v_gdi_apc_m.w3d",       
		"sZbalZhwiv+r6a",				//"v_gdi_mrls.w3d",        
		"sZbalZh`aqkn+r6a",			//"v_gdi_medtnk.w3d",      
		"sZbalZhdhhqm+r6a",			//"v_gdi_mammth.w3d",      
		"sZulfnpu54+r6a",				//"v_pickup01.w3d",        
		"sZv`adk54+r6a",				//"v_sedan01.w3d",         
		"sZbalZjwfdZh+r6a",			//"v_gdi_orca_m.w3d",      
		"sZbalZqwkvuqZh+r6a",		//"v_gdi_trnspt_m.w3d",    
		};
#define	NUM_CRC_FILES	(sizeof(	filelist ) / sizeof( filelist[0] ) )
		crc = 0;
		for ( int i = 0; i < NUM_CRC_FILES; i++ ) {
			StringClass name = filelist[i];
			// Obfuscate name
			char * n = &name[0];
			while ( *n ) *n++ ^= 0x5;
//			Debug_Say(( "		\"%s\",\n", name ));
			FileClass * file = _TheFileFactory->Get_File( name );
			if ( file && file->Is_Available() ) {
				int size = file->Size();
				file->Open();
				while ( size > 0 ) {
					unsigned char buffer[ 4096 ];
					int amount = min( (int)size, (int)sizeof(buffer) );
					amount = file->Read( buffer, amount );
					crc = CRC_Memory( buffer, amount, crc );
					size -= amount;
				}
				file->Close();
			} else {
//				Debug_Say(( "******%s not found\n", name ));
			}
		}
	}
	return crc;
}
//-----------------------------------------------------------------------------

void cNetwork::Compute_Exe_Key(void)
{
   WWDEBUG_SAY(("cNetwork::Compute_Exe_Key\n"));

	char exe_filename[500];
   int succeeded = 0;
	succeeded = ::GetModuleFileName(NULL, exe_filename, sizeof(exe_filename));
	WWASSERT(succeeded);

	StringClass key_string;
	StringClass string;
	StringClass build_string;

	//
	// 11/07/01
	// We now match only on build number.
	//
	string.Format("RENEGADE %u", BuildInfoClass::Get_Build_Number());

	WWDEBUG_SAY(("File id string: %s\n", string));
	key_string += string;
	key_string += " ";
	ExeCRC = CRCEngine()(string, strlen(string));

	//
	// TSS 09/07/01
	// N.B. Do not require scripts to be in sync any more, since they do not run on the client.
	// Furthermore we would have to match the correct scripts target (d/p/r) and
	// also handle the registry flag that forces scriptsd.dll.
	//

	//
	// TSS102401
	// We cannot match on exact strings.tdb because foreign language versions
	// are different files. Instead, match on filename and internal version number.
	//
	//cMiscUtil::Get_File_Id_String("Data\\strings.tdb", string);
	string.Format("strings.tdb %u", TranslateDBClass::Get_Version_Number());
	WWDEBUG_SAY(("File id string: %s\n", string));
	key_string += string;
	key_string += " ";
	StringsCRC = CRCEngine()(string, strlen(string));

	//
	// TSS102401 - we can't match always.dbs either.
	// always.dbs will not match among foreign language version for numerous reasons.
	//

	//
	// Use the crc of the keystring as the key
	//
	ExeKey = CRCEngine()(key_string, strlen(key_string));

	//
	// Include data file crc
	//
	int data_file_crc = Get_Data_Files_CRC();
	ExeKey ^= data_file_crc;
}

//-----------------------------------------------------------------------------
void cNetwork::Onetime_Init(void)
{
	WWMEMLOG(MEM_NETWORK);
   WWDEBUG_SAY(("cNetwork::Onetime_Init\n"));

	Compute_Exe_Key();

   NetworkReceiver = new CombatNetworkReceiverInstanceClass;
	Set_Receiver(NetworkReceiver);

	CombatManager::Set_Combat_Network_Handler(&NetHandler);

   //Clear_Help_Text();
}

//-----------------------------------------------------------------------------
void cNetwork::Onetime_Shutdown(void)
{
   WWDEBUG_SAY(("cNetwork::Onetime_Shutdown\n"));

   Set_Receiver(NULL);
	delete NetworkReceiver;

#if 0
	UINT comp_bytes	= cConnection::Get_Total_Compressed_Bytes_Sent();
	UINT uncomp_bytes = cConnection::Get_Total_Uncompressed_Bytes_Sent();
	Debug_Say(("\n"));
	Debug_Say(("TotalCompressedBytesSent   = %d\n", comp_bytes));
	Debug_Say(("Without compression        = %d\n", uncomp_bytes));
	if (uncomp_bytes > 0) {
		Debug_Say(("Compressed/Uncompressed    = %-5.2f\n\n",
			comp_bytes / (float) uncomp_bytes));
	}
#endif // 0

#pragma message("(TSS) This packet ref count assert very occasionally fails.")
   //WWASSERT(cPacket::Get_Ref_Count() == 0);

	REF_PTR_RELEASE(VisTable);

	cGameChannelList::Remove_All();//TSS092201 added

	//
	// These are only printed once. So only run one game on a test run.
	//
	cAppPacketStats::Dump_Diagnostics();
}

//-----------------------------------------------------------------------------
void cNetwork::Init_Server(void)
{
	WWMEMLOG(MEM_NETWORK);
   WWDEBUG_SAY(("cNetwork::Init_Server\n"));

#ifndef BETACLIENT

	NetworkObjectClass::Set_Is_Server(true);

	PServerStatListGroup = new cMsgStatListGroup;
	WWASSERT(PServerStatListGroup != NULL);
	//PServerStatListGroup->Init(The_Game()->Get_Max_Players(), MESSAGE_COUNT);
	WWASSERT(PTheGameData != NULL);
	PServerStatListGroup->Init(The_Game()->Get_Max_Players(), 1);

	/*
	for (int message_type = 0; message_type < MESSAGE_COUNT; message_type++) {

		char message_trans[200];
		strcpy(message_trans, Message_Type_Translation(message_type));
		PServerStatListGroup->Set_Name(message_type, &message_trans[8]);
	}
	*/

   WWASSERT(PServerConnection == NULL);
   PServerConnection = new cConnection;
   WWASSERT(PServerConnection != NULL);

   CombatManager::Set_I_Am_Server(true);

	PServerConnection->Install_Server_Broken_Connection_Handler(Server_Broken_Connection_Handler);
	PServerConnection->Install_Eviction_Handler(Eviction_Handler);
	PServerConnection->Install_Conn_Handler(Connection_Handler);
	PServerConnection->Install_Application_Acceptance_Handler(Application_Acceptance_Handler);
	PServerConnection->Install_Server_Packet_Handler(Server_Packet_Handler);


	//if (IS_SOLOPLAY || GameModeManager::Find("LAN")->Is_Active()) {
	if (IS_SOLOPLAY || 
		 (GameModeManager::Find("LAN")->Is_Active() && !cGameSpyAdmin::Is_Gamespy_Game())) {

		ULONG bbo = cBandwidth::Get_Bandwidth_Bps_From_Type(BANDWIDTH_LANT1);
		WWASSERT(bbo > 0);
		PServerConnection->Set_Bandwidth_Budget_Out(bbo);
		cBandwidthGraph::Set_Scale(200000);
	} else {
		//WWASSERT(GameModeManager::Find("WOL")->Is_Active());
		WWASSERT(cUserOptions::BandwidthBps.Get() > 0);
		unsigned long bw = cBandwidth::Get_Bandwidth_Bps_From_Type((BANDWIDTH_TYPE_ENUM)cUserOptions::Get_Bandwidth_Type());

		/*
		** Only use a portion of the bandwidth based on how many slave servers there are.
		*/
		if (The_Game()->IsDedicated.Get() && !SlaveMaster.Am_I_Slave()) {
			if (cUserOptions::Get_Bandwidth_Type() == BANDWIDTH_AUTO) {
				if (ServerSettingsClass::Get_Master_Bandwidth() == 0 || ServerSettingsClass::Get_Master_Bandwidth() == 0xffffffff) {
					int num_slaves = SlaveMaster.Get_Num_Enabled_Slaves();
					if (num_slaves) {
						bw = bw / (num_slaves+1);
					}
				}
			}
		}

		PServerConnection->Set_Bandwidth_Budget_Out(bw);
		//PServerConnection->Set_Bandwidth_Budget_Out(cUserOptions::BandwidthBps.Get());
		int bw_scale = (cUserOptions::BandwidthBps.Get() * 2) / 10;
		bw_scale = (bw_scale / 1000) * 1000;
		cBandwidthGraph::Set_Scale(bw_scale);
	}

   double max_acceptable_packetloss_pc = 10;
   PServerConnection->Set_Max_Acceptable_Packetloss_Pc(max_acceptable_packetloss_pc);

   BOOL is_flow_control_enabled = !IS_SOLOPLAY;
   PServerConnection->Enable_Flow_Control(is_flow_control_enabled);

	WWASSERT(PTheGameData != NULL);
	PServerConnection->Init_As_Server(
		The_Game()->Get_Port(),
		The_Game()->Get_Max_Players(),
		The_Game()->IsDedicated.Get(),
		ntohl(The_Game()->Get_Ip_Address()));

   //
   // Create teams
   //
	//if (The_Game()->Is_Team_Game()) {
	for (int team_num = 0; team_num < MAX_TEAMS; team_num++) {
		cTeam * p_team = new cTeam;
		p_team->Init(team_num);
	}
	//}

	cSbboManager::Reset();

	cAppPacketStats::Reset();

#endif // not BETACLIENT
}

//-----------------------------------------------------------------------------
void cNetwork::Cleanup_Server(void)
{
   WWDEBUG_SAY(("cNetwork::Cleanup_Server\n"));

   if (I_Am_Server()) {

      delete PServerConnection;
      PServerConnection = NULL;
   }

   CombatManager::Set_I_Am_Server(false);

	delete PServerStatListGroup;
	PServerStatListGroup = NULL;

	NetworkObjectClass::Set_Is_Server(false);
}

//-----------------------------------------------------------------------------
enum {
	CHUNKID_PLAYERMANAGER = 9876543,
};

//-----------------------------------------------------------------------------
bool cNetwork::Save(ChunkSaveClass & csave)
{
	csave.Begin_Chunk(CHUNKID_PLAYERMANAGER);
	cPlayerManager::Save(csave);
	csave.End_Chunk();

	return true;
}

//-----------------------------------------------------------------------------
bool cNetwork::Load(ChunkLoadClass &cload)
{
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {

			case CHUNKID_PLAYERMANAGER:
				//cPlayerManager::Remove_All();
				cPlayerManager::Load(cload);
				break;

			default:
				Debug_Say(( "Unrecognized cNetwork chunkID\n" ));
				break;

		}
		cload.Close_Chunk();
	}

	return true;
}

//-----------------------------------------------------------------------------
void cNetwork::Update_Fps(void)
{
	static DWORD last_time_ms = 0;
	static int frame_count = 0;
   DWORD time_now_ms = TIMEGETTIME();

	// Handle timer resetting.
	if (time_now_ms < last_time_ms) {
		last_time_ms = time_now_ms;
	}
	frame_count++;
	DWORD time_interval = time_now_ms - last_time_ms;

	if (time_interval > 1000) {

		Fps = (int) (frame_count * 1000 / (float) time_interval + 0.5f);
		last_time_ms = time_now_ms;
		frame_count = 0;

// Needed for release mode too. ST - 1/23/2002 11:02PM
//#ifdef WWDEBUG
		if (I_Am_Server()) {
			WWASSERT(cServerFps::Get_Instance() != NULL);
			cServerFps::Get_Instance()->Set_Fps(Fps);
		}
//#endif // WWDEBUG

		if (I_Am_Client() && PClientFps != NULL) {
			PClientFps->Set_Fps(Fps);
		}
	}
}




void cNetwork::Connection_Status_Change_Feedback(void)
{
	static unsigned long _last_print = TIMEGETTIME();
	static bool _last_print_bad = false;
	static unsigned long _print_good_soon = 0;

	unsigned long time = TIMEGETTIME();
	const WCHAR *string = NULL;
	if (LastServerConnectionStateBad) {
		if (_last_print_bad && time - _last_print < 4000) {
			return;
		}
		string = TRANSLATE (IDS_MENU_CONNECTION_INTERRUPTED);
		_last_print_bad = true;
	} else {
		/*
		** Wait 2 secs after connection restore before printing message.
		*/
		if (_last_print_bad && _print_good_soon == 0) {
			_print_good_soon = time;
			return;
		}
		if (!_last_print_bad) {
			return;
		}
		if (time - _print_good_soon < 2000) {
			return;
		}
		string = TRANSLATE (IDS_MENU_CONNECTION_RESTORED);
		_last_print_bad = false;
	}
	_last_print = time;
	WideStringClass widestring(string, true);
	if (CombatManager::Get_Message_Window() != NULL) {
		//
		//	Display the message...
		//
		CombatManager::Get_Message_Window ()->Add_Message (widestring);
	}

	//
	// Write it to the logfile
	//
	StringClass temp_string;
	widestring.Convert_To(temp_string);
	WWDEBUG_SAY(("\n***%s\n", temp_string.Peek_Buffer()));
}

//-----------------------------------------------------------------------------
void cNetwork::Update(void)
{
	WWPROFILE( "cNetwork::Update" );
	WWMEMLOG(MEM_GAMEDATA);

	bool flush_packets = false;

	ThinkCount++;

	//
	// Coding bugs may result in this function being called recursively.
	// Detect and assert!
	//
	static int recursion_level = 0;
	recursion_level++;
	WWASSERT(recursion_level == 1);

	Update_Fps();

#ifdef WWDEBUG
	//
	// Watch out for unexpected slow frames. They may interrupt networking.
	//
	static DWORD last_time_ms = TIMEGETTIME();
	DWORD time_now_ms = TIMEGETTIME();
	if (time_now_ms - last_time_ms > 2000) {
		Debug_Say(("\n***cNetwork::Update: warning, think # %d was slow (%u ms)\n\n",
			ThinkCount,
			time_now_ms - last_time_ms));
	}
	last_time_ms = time_now_ms;
#endif // WWDEBUG

	if (I_Am_Server()) {
		if (I_Am_Client()) {
			WWPROFILE( "Client Send" );
			PClientConnection->Service_Send();

			if (PClientConnection->Is_Bad_Connection() != LastServerConnectionStateBad) {
				LastServerConnectionStateBad = PClientConnection->Is_Bad_Connection();
				//Connection_Status_Change_Feedback();
			}
			Connection_Status_Change_Feedback();
		}

//		{	WWPROFILE("GameSpy_QnR");
//			GameSpyQnR.Think();
//		}

		WWPROFILE( "Server Read" );
		PServerConnection->Service_Read();

		if (!g_is_loading) {
			WWPROFILE( "Shared CS Think" );
			Shared_Client_And_Server_Think();
		}

		if (I_Am_Client() && !g_is_loading) {
			WWPROFILE( "Client_Think" );
			if (Client_Think()) {
				flush_packets = true;
			}
		}

		if (!g_is_loading) {
			WWPROFILE( "Server_Think" );
			if (Server_Think()) {
				flush_packets = true;
			} else {
				// Server only sends packets in response to server think, not client think.
				flush_packets = false;
			}
		}

		{
		WWPROFILE( "Server Send" );
		PServerConnection->Service_Send();
		}

		if (I_Am_Client()) {
			WWPROFILE( "Client Read" );
			PClientConnection->Service_Read();
		}

	} else if (I_Am_Client()) {

		{
		WWPROFILE( "Client Read" );
		PClientConnection->Service_Read();
		}

		if (PClientConnection->Is_Bad_Connection() != LastServerConnectionStateBad) {
			LastServerConnectionStateBad = PClientConnection->Is_Bad_Connection();
			Connection_Status_Change_Feedback();
		}

		if (!g_is_loading) {
			WWPROFILE( "Shared CS Think" );
			Shared_Client_And_Server_Think();
		}

		if (!g_is_loading) {
			WWPROFILE( "Client_Think" );
			if (Client_Think()) {
				flush_packets = true;
			}
		}

		{
		WWPROFILE( "Client Send" );
		if (PClientConnection != NULL) {
			PClientConnection->Service_Send();
		}
		}
	}

	DEMO_SECURITY_CHECK;

	NetworkObjectMgrClass::Delete_Pending();

	if (flush_packets) {
		PacketManager.Flush(true);
	}

	recursion_level--;
	WWASSERT(recursion_level == 0);
}

//-----------------------------------------------------------------------------
void cNetwork::Client_Send_Packet(cPacket & packet, int mode)
{
#ifndef FREEDEDICATEDSERVER

   WWASSERT(I_Am_Client());

	if (cNetwork::PClientConnection->Is_Established()) {

		PClientConnection->Send_Packet_To_Individual(packet, 0, mode);

		/*
		BYTE message_type = packet.Peek_Message_Type();
		PClientStatList->Increment_Num_Msg_Sent(message_type);
		PClientStatList->Increment_Num_Byte_Sent(message_type, packet.Get_Compressed_Size_Bytes());
		*/

	} else {
		WWDEBUG_SAY(("cNetwork::Client_Send_Packet: warning: Connection not yet established.\n"));
		//TSS2001 XXX DIE;
	}

#endif // !FREEDEDICATEDSERVER
}

//-----------------------------------------------------------------------------
void cNetwork::Server_Send_Packet(cPacket & packet, int mode, int recipient)
{
#ifndef BETACLIENT

   WWASSERT(I_Am_Server());
   WWASSERT(PServerConnection->Is_Established());

	if (recipient == ALL) {
		//
		// We cannot just send to all rhosts because that includes anyone
		// browsing the server settings, whereas here we wish to send to the
		// ingame players.
		//
		//PServerConnection->Send_Packet_To_All(*p_packet, mode);

      SLNode<cPlayer> * objnode;
      for (objnode = cPlayerManager::Get_Player_Object_List()->Head();
         objnode; objnode = objnode->Next()) {

		   cPlayer * p_player = objnode->Data();
         WWASSERT(p_player != NULL);

			//if (p_player->Is_Human()) {
			if (p_player->Get_Is_Active().Is_True() &&
				 p_player->Is_Human() &&
				 p_player->Get_Is_In_Game().Is_True()) {

				int client_id = p_player->Get_Id();

				PServerConnection->Send_Packet_To_Individual(
					packet, client_id, mode);

				/*
				BYTE message_type = packet.Peek_Message_Type();
				PServerStatListGroup->Increment_Num_Msg_Sent(client_id - 1, message_type);
				PServerStatListGroup->Increment_Num_Byte_Sent(client_id - 1, message_type, packet.Get_Compressed_Size_Bytes());
				*/
			}
		}

	} else {
		PServerConnection->Send_Packet_To_Individual(packet, recipient, mode);

		/*
		BYTE message_type = packet.Peek_Message_Type();
		PServerStatListGroup->Increment_Num_Msg_Sent(recipient - 1, message_type);
		PServerStatListGroup->Increment_Num_Byte_Sent(recipient - 1, message_type, packet.Get_Compressed_Size_Bytes());
		*/
	}

#endif // not BETACLIENT
}

//-----------------------------------------------------------------------------
void cNetwork::Server_Send_Packet_To_All_Connected(cPacket & packet, int mode)
{
#ifndef BETACLIENT

	//
	// Traverse the rhost list here in the application level, so that
	// we can record message stats.
	//

	WWASSERT(I_Am_Server());
   WWASSERT(PServerConnection->Is_Established());

	//BYTE message_type = packet.Peek_Message_Type();

   for (int rhost_id = PServerConnection->Get_Min_RHost(); rhost_id <= PServerConnection->Get_Max_RHost(); rhost_id++) {
		if (Get_Server_Rhost(rhost_id) != NULL) {
         PServerConnection->Send_Packet_To_Individual(packet, rhost_id, mode);

			/*
			PServerStatListGroup->Increment_Num_Msg_Sent(rhost_id - 1, message_type);
			PServerStatListGroup->Increment_Num_Byte_Sent(rhost_id - 1, message_type, packet.Get_Compressed_Size_Bytes());
			*/
		}
	}

#endif // not BETACLIENT
}

//-----------------------------------------------------------------------------
LPCSTR cNetwork::Get_Client_Enumeration_String(void)
{
   WWASSERT(I_Am_Server());
   WWASSERT(PServerConnection->Is_Established());

   char temp_str[10];
   strcpy(ClientEnumerationString, "");
   for (int rhost_id = PServerConnection->Get_Min_RHost(); rhost_id <= PServerConnection->Get_Max_RHost(); rhost_id++) {
		if (Get_Server_Rhost(rhost_id) != NULL) {
         strcat(ClientEnumerationString, itoa(rhost_id, temp_str, 10));
         strcat(ClientEnumerationString, " ");
      }

      if (strlen(ClientEnumerationString) > 20) {
         strcpy(ClientEnumerationString, "many");
         break;
      }
	}

   return ClientEnumerationString;
}

//-----------------------------------------------------------------------------
cRemoteHost * cNetwork::Get_Server_Rhost(int client_id)
{
   WWASSERT(I_Am_Server());
   return PServerConnection->Get_Remote_Host(client_id);
}

//-----------------------------------------------------------------------------
cRemoteHost * cNetwork::Get_Client_Rhost(void)
{
   WWASSERT(I_Am_Client());
   return PClientConnection->Get_Remote_Host(SERVER_RHOST_ID);
}

//-----------------------------------------------------------------------------
float cNetwork::Get_Server_Rhost_Threshold_Priority(int client_id)
{
   WWASSERT(I_Am_Server());
   WWASSERT(Get_Server_Rhost(client_id) != NULL);
   return Get_Server_Rhost(client_id)->Get_Threshold_Priority();
}

//-----------------------------------------------------------------------------
float cNetwork::Get_Client_Rhost_Threshold_Priority(void)
{
   WWASSERT(I_Am_Client());
   WWASSERT(Get_Client_Rhost() != NULL);
   return Get_Client_Rhost()->Get_Threshold_Priority();
}

//-----------------------------------------------------------------------------
int cNetwork::Get_My_Id(void)
{
   WWASSERT(I_Am_Client());
   return PClientConnection->Get_Local_Id();
}

//-----------------------------------------------------------------------------
LPCSTR cNetwork::Get_Client_String(int recipient)
{
   //WWASSERT(I_Am_Server());
   if (recipient > 0) {
		WWASSERT(PServerConnection->Is_Established());

		if (recipient == ALL) {
			sprintf(ClientString, "all %d clients (%s)",
				PServerConnection->Get_Num_RHosts(),
				Get_Client_Enumeration_String());
		} else {
			sprintf(ClientString, "client %d", recipient);
		}
	} else {
		sprintf(ClientString, "server");
	}

   //
   // Note: This WWASSERT would catch the overwrite AFTER it has happened...
   //
   WWASSERT(strlen(ClientString) < sizeof(ClientString));

   return(ClientString);
}

//-----------------------------------------------------------------------------
void cNetwork::Server_Broken_Connection_Handler(int broken_rhost_id)
{
	WWASSERT(broken_rhost_id >= 0);

   //
   // The net lib calls this when a reliable packet fails after
   // many attempts. The connection data has already been destroyed.
   //
   WWDEBUG_SAY(("\n***Connection to client %d broken   \n\n", broken_rhost_id));

	WideStringClass widestring;
	widestring.Format(
		L"%s %d\n",
		TRANSLATION(IDS_MP_CONNECTION_TO_CLIENT_BROKEN),
		broken_rhost_id);
   WWASSERT(CombatManager::Get_Message_Window () != NULL);
	//Get_Text_Display()->Print_System(widestring);

	//
	//	Display the message...
	//
	CombatManager::Get_Message_Window ()->Add_Message (widestring);
	Vector3 color(1.0f, 1.0f, 0.0f);
	ConsoleBox.Add_Message(&widestring, &color);
	WWAudioClass::Get_Instance()->Create_Instant_Sound("Broken_Connection", Matrix3D(1));

   cNetwork::Cleanup_After_Client(broken_rhost_id);
}

//-----------------------------------------------------------------------------
void cNetwork::Client_Broken_Connection_Handler(void)
{
   //
   // The net lib calls this when a reliable packet fails after
   // many attempts. The connection data has already been destroyed.
   //

   WWASSERT(Get_Text_Display() != NULL);
   WWASSERT(PClientConnection != NULL);

   /**/
	if (PClientConnection->Have_Id()) {
		//cHelpText::Set(TRANSLATION(IDS_MP_CONNECTION_TO_SERVER_BROKEN));
		DlgMsgBox::DoDialog(L"", TRANSLATION(IDS_MP_CONNECTION_TO_SERVER_BROKEN));
	} else {
		//cHelpText::Set(TRANSLATION(IDS_MP_UNABLE_CONNECT_TO_SERVER));
	}


	//TSS090401
	//
	// The client needs to quit back to the game list
	//
	extern bool g_client_quit;
	g_client_quit = true;

	//WWAudioClass::Get_Instance()->Create_Instant_Sound("Broken_Connection", Matrix3D(1));
	/**/

	/*
	WWDEBUG_SAY(("Connection to server broken.\n"));

	if (PClientConnection->Have_Id()) {
		DlgMPConnectionRefused::DoDialog(TRANSLATION(IDS_MP_CONNECTION_TO_SERVER_BROKEN), true);
	} else {
		extern bool g_client_quit;
		g_client_quit = true;
	}
	*/
}

//-----------------------------------------------------------------------------
void cNetwork::Process_Eviction_Sc(cPacket & packet)
{
   WWDEBUG_SAY(("cNetwork::Process_Eviction_Sc\n"));

	WWAudioClass::Get_Instance()->Create_Instant_Sound("Evicted_By_Server", Matrix3D(1));

   WWASSERT(I_Am_Client());


	UINT min_bps;
	packet.Get(min_bps);//naughty... type conversion
   float max_packetloss;
	packet.Get(max_packetloss);

   //UINT sustainable_bps  = (UINT) packet.Get();

	WWDEBUG_SAY(("\n* You were evicted from the server for inadequate bandwidth performance.\n"
         "* This server requires packetloss of less than %5.2f %%, and a minimum\n"
         "* sustainable bandwidth of %d bps.\n", max_packetloss, min_bps));

   //
   // At this stage, just signal to close down. This is
   // a convenience during development
   //
   DIE;
}

//-----------------------------------------------------------------------------
void cNetwork::Eviction_Handler(int evicted_rhost_id)
{
	WWMEMLOG(MEM_NETWORK);
   WWDEBUG_SAY(("cNetwork::Eviction_Handler\n"));

	WWASSERT(evicted_rhost_id >= 0);

   WWASSERT(I_Am_Server());
   //Send_Eviction_Sc(evicted_rhost_id);

	cEvictionEvent * p_event = new cEvictionEvent;
	p_event->Init(evicted_rhost_id, EVICTION_POOR_BANDWIDTH);

	cNetwork::Flush();

	//WWAudioClass::Get_Instance()->Create_Instant_Sound("Broken_Connection", Matrix3D(1));
}

//-----------------------------------------------------------------------------
bool cNetwork::I_Am_God(void)
{
	bool i_am_god = false;

	if (I_Am_Client()) {
		cPlayer * p_player = cPlayerManager::Find_Player(Get_My_Id());
		if (p_player != NULL && p_player->Invulnerable.Is_True()) {
			i_am_god = true;
		}
	}

	return i_am_god;
}

//-----------------------------------------------------------------------------
cPlayer * cNetwork::Get_My_Player_Object(void)
{
   cPlayer * p_me = NULL;

	if (I_Am_Client()) {
		p_me = cPlayerManager::Find_Player(Get_My_Id());
	}

   return p_me;
}

//-----------------------------------------------------------------------------
int cNetwork::Get_My_Team_Number(void)
{
   cPlayer * p_me = Get_My_Player_Object();
   WWASSERT(p_me != NULL);
   return p_me->Get_Player_Type();
}

//-----------------------------------------------------------------------------
Vector3 cNetwork::Get_My_Color(void)
{
   cPlayer * p_me = cPlayerManager::Find_Player(Get_My_Id());
   WWASSERT(p_me != NULL);
   return p_me->Get_Color();
}

//-----------------------------------------------------------------------------
int cNetwork::Show_Welcome_Message(WideStringClass & name)
{
   int show = false;

   if (!IS_MISSION && (name == cNetInterface::Get_Nickname())) {
      show = true;
   }

   return show;
}

//---------------------------------------------------------------------------
float cNetwork::Get_Distance_Priority(Vector3 & pos1, Vector3 & pos2)
{
   Vector3 gap = pos2 - pos1;
   float d = gap.Length();

	WWASSERT(PTheGameData != NULL);
	float max_distance = The_Game()->Get_Maximum_World_Distance();
   WWASSERT(max_distance > 0);

	float range1 = max_distance / 25.0f;
   float range2 = max_distance / 5.0f;
   float range3 = max_distance + 1;

   float priority;

   if (d < range1) {
      priority = (float) ((range1 - d) / range1 * 0.499 + 0.50);
   } else if (d < range2) {
      priority = (float) ((range2 - d) / (range2 - range1) * 0.40 + 0.10);
   } else if (d < range3) {
      priority = (float) ((range3 - d) / (range3 - range2) * 0.10 + 0.00);
   } else {
		//WWASSERT(0);
		static bool already_warned = false;
		if (!already_warned) {
			Debug_Say(("Someone went outside the world box! (possibly numerous times)\n"));
			already_warned = true;
		}
      priority = 0;
   }

   WWASSERT(priority > -WWMATH_EPSILON && priority < 1 + WWMATH_EPSILON);

   return(priority);
}

//-----------------------------------------------------------------------------
void cNetwork::Shell_Command(LPCSTR command)
{
	WWASSERT(command != NULL);

	HINSTANCE hinst = ShellExecute(NULL, NULL, command, NULL, "", SW_SHOW);
	if ((int) hinst <= 32) {
      WWDEBUG_SAY(("Error: ShellExecute failed.\n"));
	}
}

//-----------------------------------------------------------------------------
REFUSAL_CODE cNetwork::Application_Acceptance_Handler(cPacket & packet)
{
#ifndef BETACLIENT

   WWDEBUG_SAY(("cNetwork::Application_Acceptance_Handler\n"));

	WWASSERT(I_Am_Server());

	//
	// Get player name
	// This is not supposed to be empty, but if for whatever reason it it, we should
	// just refuse, rather than crash.
	//
	WideStringClass player_name(0, true);
	//packet.Get_Wide_Terminated_String(player_name.Get_Buffer(256), 256);
	packet.Get_Wide_Terminated_String(player_name.Get_Buffer(256), 256, true);
	if (player_name.Get_Length() == 0) {
      return REFUSAL_VERSION_MISMATCH;
	}

	// Get the clients password
	WideStringClass password(0, true);
	packet.Get_Wide_Terminated_String(password.Get_Buffer(256), 256, true);

	// Get clients exe version
	int client_exe_key = packet.Get(client_exe_key);

	// Make sure the clients password matches the games password.
	WWASSERT(PTheGameData != NULL);
	if (The_Game()->IsPassworded.Is_True() && password.Compare(The_Game()->Get_Password()) != 0) {
		return REFUSAL_BAD_PASSWORD;
	}

	// Make sure the exe versions match
	if (client_exe_key != cNetwork::Get_Exe_Key()) {
      return REFUSAL_VERSION_MISMATCH;
   }

	// Make sure we haven't exceeded our max player limit
	if (cPlayerManager::Count() >= The_Game()->Get_Max_Players()) {
      return REFUSAL_GAME_FULL;
	}

	//
	// TSS100501
	//
	// Make sure the player is not already in the game
   //GAMESPY
	//if (cPlayerManager::Find_Player(player_name)) {
	if (!cGameSpyAdmin::Is_Gamespy_Game() && 
	    cPlayerManager::Find_Player(player_name)) {
		return REFUSAL_PLAYER_EXISTS;
	}

#endif // not BETACLIENT

	return REFUSAL_CLIENT_ACCEPTED;
}

//-----------------------------------------------------------------------------
void cNetwork::Connection_Handler(int new_rhost_id)
{
	WWMEMLOG(MEM_NETWORK);
#ifndef BETACLIENT

   WWDEBUG_SAY(("cNetwork::Connection_Handler\n"));

	WWASSERT(new_rhost_id >= 0);
   WWASSERT(I_Am_Server());
   WWASSERT(Receiver != NULL);

   /*
	//
   // Tell the new guy about all the teams. He needs this early in case he
   // needs to choose a team
   //
	SLNode<cTeam> * team_node;
   cTeam * p_team;
   for (team_node = cTeamManager::Get_Team_Object_List()->Head(); team_node; team_node = team_node->Next()) {
		p_team = team_node->Data();
      WWASSERT(p_team != NULL);
		Send_Server_New_Team(p_team, new_rhost_id);
   }
	*/

	//TSS2001 - normal update mechanism won't catch this in time.
	//
   // Tell the new guy about all the teams. He needs this early in case he
   // needs to choose a team
   //
	if (!cNetwork::I_Am_Client() || new_rhost_id != cNetwork::Get_My_Id()) {
		for (
			SLNode<cTeam> * team_node = cTeamManager::Get_Team_Object_List()->Head();
			team_node;
			team_node = team_node->Next()) {

			cTeam * p_team = team_node->Data();
			WWASSERT(p_team != NULL);

			Send_Object_Update(p_team, new_rhost_id);
		}
	}

   //
   // Next, send info about any remaining game options. This is only those
   // options that he doesn't pick up from the channel listing.
   //
   //Send_Server_Game_Options(new_rhost_id);
	cGameOptionsEvent * p_event = new cGameOptionsEvent;
	p_event->Init(new_rhost_id);
	Send_Object_Update(p_event, new_rhost_id);

#endif // not BETACLIENT
}


//-----------------------------------------------------------------------------
void cNetwork::Set_Desired_Frame_Sleep_Ms(int b)
{
   WWASSERT(b >= 0);

#ifdef WWDEBUG
	cDevOptions::DesiredFrameSleepMs.Set(b);
#endif //WWDEBUG
}

//-----------------------------------------------------------------------------
void cNetwork::Set_Simulated_Packet_Loss_Pc(int b)
{
   WWASSERT(b >= 0);

   if (PClientConnection != NULL) {
      PClientConnection->Set_Packet_Loss(b);
   }
   if (PServerConnection != NULL) {
      PServerConnection->Set_Packet_Loss(b);
   }

   //SimulatedPacketLossPc = b;
#ifdef WWDEBUG
	cDevOptions::SimulatedPacketLossPc.Set(b);
#endif //WWDEBUG
}

//-----------------------------------------------------------------------------
void cNetwork::Set_Simulated_Packet_Duplication_Pc(int b)
{
   WWASSERT(b >= 0);

   if (PClientConnection != NULL) {
      PClientConnection->Set_Packet_Duplication(b);
   }
   if (PServerConnection != NULL) {
      PServerConnection->Set_Packet_Duplication(b);
   }

   //SimulatedPacketDuplicationPc = b;
#ifdef WWDEBUG
	cDevOptions::SimulatedPacketDuplicationPc.Set(b);
#endif
}

//-----------------------------------------------------------------------------
void cNetwork::Set_Simulated_Latency_Range_Ms(int lower, int upper)
{
   WWASSERT(lower >= 0);
   WWASSERT(upper >= 0);
   WWASSERT(lower <= upper);

   if (PClientConnection != NULL) {
      PClientConnection->Set_Packet_Latency_Range(lower, upper);
   }
   if (PServerConnection != NULL) {
      PServerConnection->Set_Packet_Latency_Range(lower, upper);
   }

   //SimulatedLatencyRangeMsLower = lower;
   //SimulatedLatencyRangeMsUpper = upper;

#ifdef WWDEBUG
	cDevOptions::SimulatedLatencyRangeMsLower.Set(lower);
	cDevOptions::SimulatedLatencyRangeMsUpper.Set(upper);
#endif
}

//-----------------------------------------------------------------------------
void cNetwork::Set_Spam_Count(int spam_count)
{
   WWASSERT(spam_count >= 0);

	//SpamCount = spam_count;
#ifdef WWDEBUG
	cDevOptions::SpamCount.Set(spam_count);
#endif
}

//-----------------------------------------------------------------------------
void cNetwork::Get_Simulated_Latency_Range_Ms(int & lower, int & upper)
{
#ifdef WWDEBUG
	lower = cDevOptions::SimulatedLatencyRangeMsLower.Get();
	upper = cDevOptions::SimulatedLatencyRangeMsUpper.Get();
#endif
}

//-----------------------------------------------------------------------------
void
cNetwork::Flush(void)
{
   WWDEBUG_SAY(("cNetwork::Flush\n"));

	const bool is_urgent = true;

	//if (GameModeManager::Find ("Combat")->Is_Active() && Receiver != NULL) {
	if (Receiver != NULL) {

		if (I_Am_Server()) {
			Receiver->Server_Update_Dynamic_Objects(is_urgent);
		}

		if (I_Am_Client()) {
			Receiver->Client_Update_Dynamic_Objects(is_urgent);
		}
	}

	if (I_Am_Server()) {
		PServerConnection->Service_Send(is_urgent);
	}

	if (I_Am_Client()) {
		PClientConnection->Service_Send(is_urgent);
	}

	PacketManager.Flush(true);
}


void cNetwork::SwitchTeam(int newTeam)
{
	cPlayer* player = cPlayerManager::Find_Player(Get_My_Id());

	if (player) {
		int team = player->Get_Player_Type();

		if (newTeam != team) {
			cChangeTeamEvent* event = new cChangeTeamEvent;
			event->Init();
		}
	}
}


//-----------------------------------------------------------------------------
void cNetwork::Enable_Waiting_Players(void)
{
	SLNode<cPlayer> * objnode;
   for (objnode = cPlayerManager::Get_Player_Object_List()->Head() ; objnode != NULL ; objnode = objnode->Next()) {

		cPlayer * p_player = objnode->Data();
      WWASSERT(p_player != NULL);

		if (p_player->Is_Human() && p_player->Get_Is_Waiting_For_Intermission().Is_True()) {

			p_player->Set_Is_In_Game(true);
			p_player->Set_Is_Waiting_For_Intermission(false);
			if (cNetwork::PServerConnection) {
				cNetwork::PServerConnection->Set_Rhost_Is_In_Game(p_player->Get_Id(), true);
			}
			cGameDataUpdateEvent * p_event = new cGameDataUpdateEvent();
			p_event->Init(p_player->Get_Id());
		}
   }
}
