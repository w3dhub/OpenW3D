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
 *                 Project Name : Combat																		  *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Commando/dialogresource.h      $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 11/09/01 4:45p                                              $*
 *                                                                                             *
 *                    $Revision:: 18                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __DIALOG_RESOURCE_H
#define __DIALOG_RESOURCE_H

#include "dialogspec.h"

enum class RenegadeDialogID {
    IDD_CDVERIFY =                    1,
    IDD_CNC_PURCHASE_MAIN_SCREEN_OLD = 124,
    IDD_CNC_PURCHASE_VEHICLE_SCREEN = 125,
    IDD_CNC_PURCHASE_POWERUP_SCREEN = 126,
    IDD_CNC_PURCHASE_CHARACTER_SCREEN = 127,
    IDD_MENU_MAIN =                   128,
    IDD_QUIT_TO_DESKTOP =             129,
    IDD_MENU_START_SP =               130,
    IDD_MENU_DIFFICULTY =             131,
    IDD_MENU_HIGH_SCORES =            132,
    IDD_SCORE_CAMPAIGN_TAB =          133,
    IDD_SCORE_MISSIONS_TAB =          134,
    IDD_MENU_OPTIONS =                135,
    IDD_MENU_CONTROLS =               136,
    IDD_CONTROLS_BASIC_MOVMENT_TAB =  137,
    IDD_CONTROLS_ADV_MOVMENT_TAB =    138,
    IDD_CONTROLS_ATTACK_TAB =         139,
    IDD_CONTROLS_WEAPONS_TAB =        140,
    IDD_CONTROLS_LOOK_TAB =           141,
    IDD_CONTROLS_MISC_TAB =           142,
    IDD_CONTROLS_MULTIPLAYER_TAB =    143,
    IDD_CONTROLS_ADVANCED_TAB =       144,
    IDD_MENU_LOAD_SP_GAME =           145,
    IDD_ENCYCLOPEDIA_OBJECTIVES_TAB = 146,
    IDD_ENCYCLOPEDIA_MAP_TAB =        147,
    IDD_ENCYCLOPEDIA_DATA_TAB =       148,
    IDD_ENCYCLOPEDIA_CHARACTERS_TAB = 149,
    IDD_ENCYCLOPEDIA_WEAPONS_TAB =    150,
    IDD_ENCYCLOPEDIA_VEHICLES_TAB =   151,
    IDD_ENCYCLOPEDIA_BUILDINGS_TAB =  152,
    IDD_MENU_EVA_ENCYCLOPEDIA =       153,
    IDD_MENU_MAIN_MULTIPLAY =         154,
    IDD_MULTIPLAY_CS_SEL =            155,
    IDD_MULTIPLAY_GAME_TYPE =         156,
    IDD_MULTIPLAY_SERVER_CONFIG =     157,
    IDD_MULTIPLAY_START_GAME =        158,
    IDD_MULTIPLAY_JOIN_GAME =         159,
    IDD_MULTIPLAY_CONNECTING =        160,
    IDD_MULTIPLAY_WWOL_CS_SEL =       163,
    IDD_MP_WOL_CONNECTING =           164,
    IDD_MP_WOL_CONNECT_ERROR =        165,
    IDD_MULTIPLAY_OPTIONS =           166,
    IDD_OPTIONS_CHARACTER =           167,
    IDD_OPTIONS_CHEATS =              168,
    IDD_OPTIONS_TECH =                169,
    IDD_OPTIONS_MOVIES =              170,
    IDD_OPTIONS_PREVIEW =             171,
    IDD_OPTIONS_CREDITS =             172,
    IDD_MP_CONN_TYPE =                173,
    IDD_MP_LAN_GAME_LIST =            174,
    IDD_MP_LAN_HOST_GAME =            175,
    IDD_MP_LAN_HOST_OPTIONS =         176,
    IDD_MP_LAN_HOST_OPTIONS_BASIC_TAB = 177,
    IDD_MP_LAN_HOST_OPTIONS_ADVANCED_TAB = 178,
    IDD_MP_LAN_HOST_OPTIONS_VICTORY_TAB = 179,
    IDD_MP_LAN_HOST_OPTIONS_MAP_TAB = 180,
    IDD_MP_JOIN_PASSWORD =            181,
    IDD_MP_INVALID_PASSWORD =         182,
    IDD_MP_INVALID_SERVER_CONFIG =    183,
    IDD_MP_LAN_HOST_OPTIONS_CTF_SETTINGS = 184,
    IDD_MP_LAN_HOST_OPTIONS_CNC_SETTINGS = 188,
    IDD_MP_WOL_LOGON =                189,
    IDD_MP_DEBUG_ADD_PERSONA =        190,
    IDD_MP_WOL_MAIN =                 191,
    IDD_MP_WOL_QUICKMATCH =           192,
    IDD_MP_WOL_BUDDIES =              194,
    IDD_MP_WOL_QUICKMATCH_OPTIONS =   195,
    IDD_DEATH_OPTIONS =               196,
    IDD_FAILED_OPTIONS =              197,
    IDD_MP_WOL_GAME_LIST =            198,
    IDD_MP_WOL_PAGE_BUDDY =           199,
    IDD_MP_WOL_BUDDY_LIST_POPUP =     200,
    IDD_MP_WOL_PAGE_REPLY =           201,
    IDD_MP_WOL_ADD_BUDDY =            202,
    IDD_MP_WOL_DELETE_BUDDY =         203,
    IDD_MP_WOL_IGNORE_LIST =          204,
    IDD_MP_WOL_ADD_IGNORE_ENTRY =     205,
    IDD_MP_WOL_CHAT =                 206,
    IDD_WEBPAGE =                     207,
    IDD_MP_WOL_HOST_CHAT_LOBBY =      208,
    IDD_MESSAGEBOX_YESNO =            209,
    IDD_MESSAGEBOX_OK =               210,
    IDD_MENU_SAVE_GAME =              211,
    IDD_MP_WOL_DELETE_IGNORE_ENTRY =  212,
    IDD_MP_WOL_INVITATION_REPLY =     213,
    IDD_MP_WOL_QUICKMATCH_CONNECT =   214,
    IDD_WOL_WAIT =                    215,
    IDD_MENU_CONTROL_SAVELOAD =       216,
    IDD_WOL_LOGON =                   218,
    IDD_WOL_SETTINGS =                219,
    IDD_CNC_PURCHASE_SCREEN =         220,
    IDD_CNC_PURCHASE_SCREEN1 =        221,
    IDD_CNC_PURCHASE_MAIN_SCREEN =    221,
    IDD_MP_CHOOSE_TEAM =              223,
    IDD_SIDEBAR_HELP_POPUP =          224,
    IDD_MP_WOL_AUTO_LOGIN_PROMPT =    225,
    IDD_MP_WOL_MOTD =                 226,
    IDD_OPTIONS_INTERNET =            227,
    IDD_MP_WOL_LOCALE_NAG =           228,
    IDD_MULTIPLAY_INGAME_CHAT =       229,
    IDD_CNC_PURCHASE_ACCESS_DENIED =  230,
    IDD_CONFIG_AUDIO =                231,
    IDD_CONFIG_PERFORMANCE =          232,
    IDD_CONFIG_VIDEO =                233,
    IDD_EDIT_WHEELED_VEHICLE =        234,
    IDD_EDIT_TRACKED_VEHICLE =        235,
    IDD_CHAT_MODULE =                 236,
    IDD_MULTIPLAY_INGAME_TEAM_CHAT =  237,
    IDD_WOL_DOWNLOAD =                238,
    IDD_SCORE_SCREEN =                239,
    IDD_CNC_TEAM_INFO =               240,
    IDD_CNC_TEAM_INFO1 =              241,
    IDD_CNC_BATTLE_INFO =             242,
    IDD_HELP_SCREEN =                 243,
    IDD_MP_AUTO_RESTART_PROGRESS =    244,
    IDD_MENU_CNC_REFERENCE =          245,
    IDD_MP_TEAM_SELECT =              246,
    IDD_MP_WOL_SLAVESERVER =          247,
    IDD_CNC_WINSCREEN =               248,
    IDD_CNC_SERVER_INFO =             249,
    IDD_MENU_SERVER_SETTINGS_SAVELOAD = 250,
    IDD_MENU_OPTION_MAIN_MULTIPLAY =  251,
    IDD_MP_CHANGE_LAN_NICKNAME =      252,
    IDD_MENU_GAMESPY_MAIN =           253,
    IDD_MENU_GAMESPY_OPTIONS =        254,
    IDD_MENU_SPLASH1 =                255,
    IDD_MENU_SPLASH2 =                256,
    IDD_MULTIPLAY_CONNECTION_REFUSED = 257,
};
#define IDC_INVITE_BUDDY_BUTTON         3
#define IDC_PAGE_BUTTON                 4
#define IDC_BUDDY_LIST_BUTTON           7
#define IDC_QUIT                        1000
#define IDC_DRIVER_LIST                 1001
#define IDC_STEREO_CHECK                1002
#define IDC_EXPERT_CHECK                1007
#define IDC_PERFORMANCE_SLIDER          1008
#define IDC_TERRAIN_SHADOW_CHECK        1009
#define IDC_CHAR_SHADOWS_SLIDER         1010
#define IDC_MENU_MP_KOTH_BUTTON         1011
#define IDC_NPATCH_CHECK                1011
#define IDC_MENU_MP_COOP_BUTTON         1012
#define IDC_TEXTURE_DETAIL_SLIDER       1012
#define IDC_MENU_MP_CNC_BUTTON          1013
#define IDC_PARTICLE_DETAIL_SLIDER      1013
#define IDC_MENU_MP_CHALLENGE_BUTTON    1014
#define IDC_MIPMAPPING_CHECK            1014
#define IDC_SURFACE_DETAIL_SLIDER       1015
#define IDC_GEOMETRY_DETAIL_SLIDER      1017
#define IDC_RATE_COMBO                  1018
#define IDC_SPEAKER_SETUP_COMBO         1019
#define IDC_QUALITY_COMBO               1020
#define IDC_SOUND_EFFECTS_SLIDER        1021
#define IDC_MUSIC_SLIDER                1022
#define IDC_SOUND_EFFECTS_CHECK         1023
#define IDC_MUSIC_CHECK                 1024
#define IDC_DIALOG_CHECK                1025
#define IDC_DIALOG_SLIDER               1026
#define IDC_CINEMATIC_CHECK             1027
#define IDC_CINEMATIC_SLIDER            1028
#define IDC_MENU_PLAY_MOVIE_BUTTON      1032
#define IDC_REFRESH_GAME_LIST           1034
#define IDC_PAGE_BUDDY_BUTTON           1035
#define IDC_REFRESH_BUTTON              1036
#define IDC_SORT_DEDICATED_BUTTON       1037
#define IDC_SORT_PASSWORD_BUTTON        1038
#define IDC_SORT_CLAN_BUTTON            1039
#define IDC_SORT_LADDERED_BUTTON        1040
#define IDC_EDIT1                       1170
#define IDC_COMBO1                      1171
#define IDC_EDIT2                       1171
#define IDC_STEERING_MODE_COMBO         1171
#define IDC_MOD_COMBO                   1171
#define IDC_PACKAGE_COMBO               1171
#define IDC_MOD_PACKAGE_COMBO           1171
#define IDC_CHECK1                      1172
#define IDC_EDIT3                       1172
#define IDC_SERVER_COMBO                1172
#define IDC_AUTO_AIM_CHECK              1172
#define IDC_CHARACTER_COMBO             1172
#define IDC_LOOP_MAPS_CHECK             1172
#define IDC_TEAM_GDI_CHECK              1172
#define IDC_CHECK2                      1173
#define IDC_EDIT4                       1173
#define IDC_CONNECTION_SPEED_COMBO      1173
#define IDC_UNLIM_AMMO_CHECK            1173
#define IDC_AUTO_SWITCH_WEAPONS_CHECK   1173
#define IDC_TEAM_AUTO_CHECK             1173
#define IDC_CHECK3                      1174
#define IDC_GIVE_WEAPONS_CHECK          1174
#define IDC_TEAM_NOD_CHECK              1174
#define IDC_BEACON_CHECK                1175
#define IDC_DM_SLIDER                   1175
#define IDC_FALLING_DAMAGE_CHECK        1176
#define IDC_VEHICLE_TYPE_COMBO          1177
#define IDC_INTERMISSION_TIME_EDIT      1178
#define IDC_EMOT_MESSAGE_BUTTON         1178
#define IDC_CHARACTERS_BUTTON           1178
#define IDC_BUTTON2                     1179
#define IDC_BACK                        1179
#define IDC_STARTING_CREDITS_EDIT       1179
#define IDC_TDM_SLIDER                  1179
#define IDC_DAMAGE_INDICATOR_CHECK      1179
#define IDC_VEHICLES_BUTTON             1180
#define IDC_BUTTON3                     1180
#define IDC_LIST1                       1180
#define IDC_MAX_PLAYERS_EDIT            1180
#define IDC_DEATH_OPTION_LOAD           1180
#define IDC_MUTATION_SLIDER             1180
#define IDC_INVERT_MOUSE_CHECK          1180
#define IDC_BASIC_EQUIP_BUTTON          1180
#define IDC_EQUIPMENT_BUTTON            1180
#define IDC_BUTTON4                     1181
#define IDC_EDIT11                      1181
#define IDC_TARGET_CREDITS_EDIT         1181
#define IDC_LIST2                       1181
#define IDC_CTF_SLIDER                  1181
#define IDC_INVERT_CURSOR_TARGETTING_CHECK 1181
#define IDC_ADV_EQUIP_BUTTON            1181
#define IDC_WIN2_LIST_CTRL              1181
#define IDC_DEATH_OPTION_QUIT           1181
#define IDC_BUTTON5                     1182
#define IDC_CHECK15                     1182
#define IDC_CTM_SLIDER                  1182
#define IDC_LOSE1_LIST_CTRL             1182
#define IDC_LOCK_CAMERA_TO_TURRET_CHECK 1182
#define IDC_CNC_SLIDER                  1183
#define IDC_LOSE2_LIST_CTRL             1183
#define IDC_FREE_WEAPONS_CHECK          1184
#define IDC_CHALLENGE_SLIDER            1184
#define IDC_MENU_MP_DEATHMATCH_BUTTON   1185
#define IDC_IS_LADDERED_CHECK           1185
#define IDC_LMS_SLIDER                  1185
#define IDC_ALLOW_QUICKMATCH            1185
#define IDC_MENU_MP_TEAM_DEATHMATCH_BUTTON 1186
#define IDC_AUTO_TEAM_CHECK             1186
#define IDC_BUDDIES_SLIDER              1186
#define IDC_CLAN_GAME_CHECK             1186
#define IDC_MENU_MP_MUTATION_BUTTON     1187
#define IDC_FRIENDLY_FIRE_CHECK         1187
#define IDC_LADDERED_CHECK              1187
#define IDC_MENU_MP_CTF_BUTTON          1188
#define IDC_DEDICATED_SERVER_CHECK      1188
#define IDC_MENU_MP_CTM_BUTTON          1189
#define IDC_ALLOW_VEHICLES_CHECK        1189
#define IDC_MENU_MP_LASTMAN_BUTTON      1190
#define IDC_NICKNAME_EDIT               1191
#define IDC_TIME_LIMIT_EDIT             1192
#define IDC_SCORE_LIMIT_EDIT            1193
#define IDC_MAP_NAME_COMBO              1194
#define IDC_STARTING_CREDITS_STATIC     1195
#define IDC_TARGET_CREDITS_STATIC       1196
#define IDC_MAX_PLAYERS_STATIC          1197
#define IDC_MAP_CTRL                    1200
#define IDC_OBJECT_DESC_LIST            1201
#define IDC_GAME_LIST                   1202
#define IDC_AFFILIATION_STATIC          1203
#define IDC_RANK_STATIC                 1204
#define IDC_DESCRIPTION_STATIC          1205
#define IDC_LIST_CTRL                   1206
#define IDC_VIEWER_CTRL                 1207
#define IDC_NOD_LIST_CTRL               1207
#define IDC_SKIN_LIST_CTRL              1207
#define IDC_PASSWORD_EDIT               1208
#define IDC_MP_NICKNAME_STATIC          1209
#define IDC_ERROR_STATIC                1210
#define IDC_DISPLAY_TEAM_LIST_CHECK     1212
#define IDC_DISPLAY_PLAYER_LIST_CHECK   1213
#define IDC_DISPLAY_PLAYER_NAMES_CHECK  1214
#define IDC_CUSTOM1                     1215
#define IDC_DISPLAY_TEAM_LIST_CHECK2    1215
#define IDC_DISPLAY_PLAYER_NAMES_CHECK5 1215
#define IDC_ALLOW_PAGES_CHECK           1215
#define IDC_MODEL_VIEWER                1215
#define IDC_PLAYERNAME_EDIT             1216
#define IDC_CUSTOM2                     1216
#define IDC_ITEM_ROW0_COL1              1216
#define IDC_DISPLAY_PLAYER_LIST_CHECK2  1216
#define IDC_DISPLAY_PLAYER_NAMES_CHECK6 1216
#define IDC_LANGUAGE_FILTER_CHECK       1216
#define IDC_ITEM_2                      1216
#define IDC_MENU_PLAY_PREVIEW_BUTTON    1217
#define IDC_CUSTOM3                     1217
#define IDC_ITEM_ROW0_COL2              1217
#define IDC_DISPLAY_PLAYER_NAMES_CHECK2 1217
#define IDC_DISPLAY_PLAYER_NAMES_CHECK7 1217
#define IDC_ALLOW_FIND_CHECK            1217
#define IDC_ITEM_3                      1217
#define IDC_OBJECTIVES_LIST_CTRL        1218
#define IDC_CUSTOM4                     1218
#define IDC_ITEM_ROW0_COL3              1218
#define IDC_DISPLAY_PLAYER_NAMES_CHECK3 1218
#define IDC_DISPLAY_PLAYER_NAMES_CHECK8 1218
#define IDC_DISPLAY_ASIAN_CHECK         1218
#define IDC_DESCRIPTION_LIST_CTRL       1219
#define IDC_CUSTOM5                     1219
#define IDC_ITEM_ROW0_COL4              1219
#define IDC_DISPLAY_TEAM_LIST_CHECK3    1219
#define IDC_DISPLAY_PLAYER_NAMES_CHECK9 1219
#define IDC_DISPLAY_NONASIAN_CHECK      1219
#define IDC_HOTKEY1                     1220
#define IDC_CUSTOM6                     1220
#define IDC_ITEM_ROW1_COL0              1220
#define IDC_DISPLAY_PLAYER_LIST_CHECK3  1220
#define IDC_SEND_DELAY_CHECK            1220
#define IDC_ITEM_4                      1220
#define IDC_SLIDER3                     1221
#define IDC_CUSTOM7                     1221
#define IDC_ITEM_ROW1_COL1              1221
#define IDC_DISPLAY_PLAYER_NAMES_CHECK4 1221
#define IDC_SHOW_BUDDIES_CHAT_CHECK     1221
#define IDC_ITEM_5                      1221
#define IDC_HOTKEY12                    1222
#define IDC_SLIDER4                     1222
#define IDC_GAME_LIST_CTRL              1222
#define IDC_CUSTOM8                     1222
#define IDC_ITEM_ROW1_COL2              1222
#define IDC_MOUSE_SENSITIVITY_SLIDER    1222
#define IDC_SHOW_CLAN_CHAT_CHECK        1222
#define IDC_ITEM_6                      1222
#define IDC_HOTKEY2                     1223
#define IDC_SLIDER5                     1223
#define IDC_GAME_VIEW_LIST_CTRL         1223
#define IDC_TABCTRL                     1223
#define IDC_CUSTOM9                     1223
#define IDC_ITEM_ROW1_COL3              1223
#define IDC_DETAILS_LIST                1223
#define IDC_HOTKEY3                     1224
#define IDC_SLIDER6                     1224
#define IDC_JOIN_GAME_BUTTON            1224
#define IDC_DEATH_OPTION_RESTART        1224
#define IDC_CUSTOM10                    1224
#define IDC_ITEM_ROW1_COL4              1224
#define IDC_HOTKEY4                     1225
#define IDC_IP_ADDRESS_STATIC           1225
#define IDC_DEATH_OPTION_RESPAWN        1225
#define IDC_CUSTOM11                    1225
#define IDC_ITEM_ROW2_COL0              1225
#define IDC_ITEM_7                      1225
#define IDC_HOTKEY5                     1226
#define IDC_NUM_PLAYERS_EDIT            1226
#define IDC_FAILED_OPTION_RESTART       1226
#define IDC_CUSTOM12                    1226
#define IDC_ITEM_ROW2_COL1              1226
#define IDC_ITEM_8                      1226
#define IDC_HOTKEY6                     1227
#define IDC_GAME_NAME_EDIT              1227
#define IDC_FAILED_OPTION_LOAD          1227
#define IDC_CUSTOM13                    1227
#define IDC_ITEM_ROW2_COL2              1227
#define IDC_ITEM_9                      1227
#define IDC_HOTKEY7                     1228
#define IDC_MAP_TIME_LIMIT_EDIT         1228
#define IDC_CARRIER_MAX_SPEED_EDIT      1228
#define IDC_CUSTOM14                    1228
#define IDC_ITEM_ROW2_COL3              1228
#define IDC_ITEM_10                     1228
#define IDC_HOTKEY8                     1229
#define IDC_ROUND_TIME_LIMIT_EDIT       1229
#define IDC_CUSTOM15                    1229
#define IDC_ITEM_ROW2_COL4              1229
#define IDC_HOTKEY9                     1230
#define IDC_ALLIED_FIRE_CHECK           1230
#define IDC_CUSTOM16                    1230
#define IDC_ITEM_ROW3_COL0              1230
#define IDC_HOTKEY10                    1231
#define IDC_ALLIED_SQUISH_CHECK         1231
#define IDC_CUSTOM17                    1231
#define IDC_ITEM_ROW3_COL1              1231
#define IDC_HOTKEY11                    1232
#define IDC_PERSISTENT_WEAPONS_CHECK    1232
#define IDC_CUSTOM18                    1232
#define IDC_ITEM_ROW3_COL2              1232
#define IDC_HOTKEY13                    1233
#define IDC_BONUS_CRATES_CHECK          1233
#define IDC_CUSTOM19                    1233
#define IDC_ITEM_ROW3_COL3              1233
#define IDC_HOTKEY14                    1234
#define IDC_ALLOW_SPECTATORS_CHECK      1234
#define IDC_CUSTOM20                    1234
#define IDC_ITEM_ROW3_COL4              1234
#define IDC_HOTKEY15                    1235
#define IDC_GAME_MODE_LIST              1235
#define IDC_TEAM_CHANGE_CHECK           1235
#define IDC_HOTKEY16                    1236
#define IDC_KILL_LIMIT_EDIT             1236
#define IDC_HOTKEY17                    1237
#define IDC_SCORE_LIMIT_CHECK           1237
#define IDC_HOTKEY18                    1238
#define IDC_KILL_LIMIT_CHECK            1238
#define IDC_HOTKEY19                    1239
#define IDC_DESTROY_ALL_BUILDINGS_CHECK 1239
#define IDC_HOTKEY20                    1240
#define IDC_HOTKEY21                    1241
#define IDC_HOTKEY22                    1242
#define IDC_HOTKEY23                    1243
#define IDC_ADD_MAP_BUTTON              1245
#define IDC_AVAILABLE_MAP_LIST_CTRL     1246
#define IDC_MAP_CYCLE_LIST_CTRL         1247
#define IDC_MESSAGE_OF_THE_DAY_EDIT     1248
#define IDC_REMOVE_MAP_BUTTON           1249
#define IDC_CAPTURE_POINTS_EDIT         1250
#define IDC_CARRIER_POINTS_EDIT         1252
#define IDC_CAN_MOVE_HOME_FLAG_CHECK    1253
#define IDC_CARRIER_CAN_USE_WEAPONS_CHECK 1254
#define IDC_VEHICLE_MODE_COMBO          1262
#define IDC_PERSONA_COMBO               1263
#define IDC_RADAR_MODE_COMBO            1263
#define IDC_REMEMBER_LOGIN_CHECK        1264
#define IDC_PERSONA_EDIT                1265
#define IDC_ADD_PERSONA_BUTTON          1266
#define IDC_MANAGE_ACCOUNT_BUTTON       1267
#define IDC_WOL_LOG_ON_BUTTON           1268
#define IDC_DELETE_ACCOUNT_BUTTON       1269
#define IDC_BUDDY_LIST_CTRL             1276
#define IDC_MP_WOL_QUICK_MATCH_BUTTON   1278
#define IDC_MP_WOL_QUICK_MATCH_COOP_BUTTON 1279
#define IDC_MP_WOL_QUICK_MATCH_OPTIONS_BUTTON 1280
#define IDC_MP_WOL_ADVANCED_LISTINGS_BUTTON 1281
#define IDC_DM_STATIC                   1283
#define IDC_TDM_STATIC                  1284
#define IDC_MUTATION_STATIC             1285
#define IDC_CTF_STATIC                  1286
#define IDC_CTM_STATIC                  1287
#define IDC_CNC_STATIC                  1288
#define IDC_CHALLENGE_STATIC            1289
#define IDC_LMS_STATIC                  1290
#define IDC_BUDDIES_STATIC              1291
#define IDC_VIEW_BUDDY_LIST_BUTTON      1292
#define IDC_SELECT_BUTTON               1293
#define IDC_MESSAGE_LIST                1294
#define IDC_ADD_BUTTON                  1294
#define IDC_ADD_BUDDY_BUTTON            1295
#define IDC_IGNORE_PLAYER_BUTTON        1296
#define IDC_REPLY_BUTTON                1297
#define IDC_DELETE_BUTTON               1298
#define IDC_REMOVE_BUTTON               1299
#define IDC_SAVE_BUTTON                 1299
#define IDC_MESSAGE_LIST_CTRL           1300
#define IDC_LOAD_BUTTON                 1300
#define IDC_CURRENT_LOBBY_STATIC        1301
#define IDC_LOBBY_LIST_CTRL             1302
#define IDC_PLAYERS_LIST_CTRL           1303
#define IDC_CURRENT_LOBBY_STATIC2       1304
#define IDC_CURRENT_LOBBY_STATIC3       1305
#define IDC_SQUELCH_BUTTON              1306
#define IDC_JOIN_LOBBY_BUTTON           1307
#define IDC_USERINFO                    1307
#define IDC_CHAT_EDIT                   1308
#define IDC_CREATE_BUTTON               1309
#define IDC_NAME_EDIT                   1310
#define IDC_SHORTCUT_BAR                1311
#define IDC_MESSAGE                     1313
#define IDC_OUTPUT                      1315
#define IDC_SAVE_GAME_BUTTON            1316
#define IDC_FILENAME_EDIT               1317
#define IDC_BUDDY_NAME_EDIT             1318
#define IDC_REPLY_EDIT                  1319
#define IDC_IGNORE_USER_EDIT            1320
#define IDC_MESSAGE_EDIT                1321
#define IDC_BUDDY_NAME_STATIC           1322
#define IDC_HOST_GAME_BUTTON            1324
#define IDC_MP_WOL_BUDDIES_PAGE_BUTTON  1325
#define IDC_MP_WOL_BUDDIES_ADD_BUTTON   1326
#define IDC_LOAD_GAME_LIST_CTRL2        1327
#define IDC_MP_WOL_BUDDIES_REFRESH_BUTTON 1327
#define IDC_MP_WOL_BUDDIES_DELETE_BUTTON 1328
#define IDC_LOAD_GAME_LIST_CTRL3        1329
#define IDC_MP_WOL_BUDDIES_JOIN_BUTTON  1330
#define IDC_MP_WOL_BUDDIES_IGNORE_BUTTON 1331
#define IDC_USER_NAME_STATIC            1332
#define IDC_INVITATION_TEXT             1333
#define IDC_JOIN_BUTTON                 1334
#define IDC_DECLINE_BUTTON              1335
#define IDC_WAITTEXT                    1336
#define IDC_CONTROL_TABCTRL             1337
#define IDC_SAVELOAD_BUTTON             1338
#define IDC_DEFAULTS_BUTTON             1339
#define IDC_CHATSERVER_COMBO            1343
#define IDC_GAMESERVER_COMBO            1344
#define IDC_AUTOLOGIN_CHECK             1345
#define IDC_LOCALE_COMBO                1346
#define IDC_LOGIN_EDIT                  1350
#define IDC_HOSTING_IP_COMBO            1353
#define IDC_ITEM_ROW0_COL0              1354
#define IDC_ITEM_1                      1354
#define IDC_TOTAL_COST                  1355
#define IDC_ENLISTED_PURCHASE_02        1355
#define IDC_CREDITS                     1356
#define IDC_ENLISTED_PURCHASE_03        1356
#define IDC_PURCHASE_BUTTON             1357
#define IDC_ENLISTED_PURCHASE_04        1357
#define IDC_CLEAR_BUTTON                1358
#define IDC_BEACON_PURCHASE             1358
#define IDC_MENU_MP_DEFECT_BUTTON       1359
#define IDC_FULL_HEALTH_PURCHASE        1359
#define IDC_SUPPLY_PURCHASE             1359
#define IDC_CHOOSE_NOD_BUTTON           1360
#define IDC_CREDITS_TEXT                1360
#define IDC_CHOOSE_GDI_BUTTON           1361
#define IDC_REMEMBER_CHECK              1362
#define IDC_BUDDY_NAME_COMBO            1363
#define IDC_DONT_SHOW_AGAIN_CHECK       1364
#define IDC_DONT_ASK_AGAIN_CHECK        1365
#define IDC_NEWPERSONA                  1366
#define IDC_MULTIPLAY_INGAME_CHAT_ALL   1367
#define IDC_CHECK_BANDWIDTH             1367
#define IDC_MULTIPLAY_INGAME_CHAT_TEAM  1368
#define IDC_MULTIPLAY_INGAME_CHAT_EDIT  1369
#define IDC_VEHICLES_STATIC             1370
#define IDC_STATIC_JOIN_COUNTDOWN       1372
#define IDC_LISTCTRL                    1373
#define IDC_IMAGE                       1374
#define IDC_DISPLAY_DRIVER              1375
#define IDC_IMAGE2                      1375
#define IDC_WEAPONS_IMAGE               1375
#define IDC_RESOLUTION                  1376
#define IDC_IMAGE3                      1376
#define IDC_POWERUPS_IMAGE              1376
#define IDC_BIT_DEPTH                   1377
#define IDC_IMAGE4                      1377
#define IDC_FRIENDLIES_IMAGE            1377
#define IDC_LIGHTMAP_MODE               1378
#define IDC_IMAGE5                      1378
#define IDC_SHOTS_FIRED_IMAGE           1378
#define IDC_INVULN_CHECK                1379
#define IDC_IMAGE6                      1379
#define IDC_ACCURACY_IMAGE              1379
#define IDC_RANKTYPE                    1380
#define IDC_ENEMIES_KILLED_IMAGE        1380
#define IDC_USERPROFILE                 1381
#define IDC_VEHICLE_TIME_IMAGE          1381
#define IDC_MAP_TITLE                   1382
#define IDC_VEHICLE_KILLS_IMAGE         1382
#define IDC_LOGINNAME                   1383
#define IDC_VEHICLE_SQUISHES_IMAGE      1383
#define IDC_SERVERNAME                  1384
#define IDC_VEHICLES_DESTROYED_IMAGE    1384
#define IDC_GAMMA_SLIDER                1385
#define IDC_CONNECTIONSPEED             1385
#define IDC_BUILDINGS_DESTROYED_IMAGE   1385
#define IDC_GAMMA_SLIDER2               1386
#define IDC_HIT_MAP_IMAGE               1386
#define IDC_BRIGHTNESS_SLIDER           1386
#define IDC_PROFILENAME                 1387
#define IDC_CONTRAST_SLIDER             1387
#define IDC_DESCRIPTION_EDIT            1388
#define IDC_PORT_EDIT                   1389
#define IDC_CREDITS_EDIT                1390
#define IDC_PROFILERANK                 1391
#define IDC_SERIAL_EDIT1                1391
#define IDC_GAME_TYPE_TITLE             1392
#define IDC_GRAVSCALE_EDIT              1393
#define IDC_SERIAL_EDIT2                1393
#define IDC_AERODRAG_EDIT               1394
#define IDC_SERIAL_EDIT3                1395
#define IDC_MAX_ENGINE_TORQUE_EDIT      1397
#define IDC_SERIAL_EDIT4                1397
#define IDC_SPRING_CONSTANT_EDIT        1398
#define IDC_DAMPING_CONSTANT_EDIT       1399
#define IDC_SERIAL_EDIT5                1399
#define IDC_SPRING_LENGTH_EDIT          1400
#define IDC_TRACK_U_SCALE_FACTOR_EDIT   1401
#define IDC_SERIAL_EDIT6                1401
#define IDC_GEAR_COUNT_EDIT             1402
#define IDC_TRACK_V_SCALE_FACTOR_EDIT   1402
#define IDC_GEAR_RATIO1_EDIT            1403
#define IDC_TURN_TORQUE_SCALE_FACTOR_EDIT 1403
#define IDC_SERIAL_EDIT7                1403
#define IDC_GEAR_RATIO2_EDIT            1404
#define IDC_GEAR_RATIO3_EDIT            1405
#define IDC_GEAR_RATIO4_EDIT            1406
#define IDC_GEAR_RATIO5_EDIT            1407
#define IDC_MAX_STEERING_ANGLE_EDIT     1408
#define IDC_LATERAL_MOMENT_ARM_EDIT     1409
#define IDC_TRACTIVE_MOMENT_ARM_EDIT    1410
#define IDC_SHIFT_UP_RPM_EDIT           1411
#define IDC_SHIFT_DOWN_RPM_EDIT         1412
#define IDC_MASS_EDIT                   1413
#define IDC_WOLAPI_VER                  1414
#define IDC_VERSION_STATIC              1415
#define IDC_SCORE_SCREEN_RANK           1418
#define IDC_SCORE_SCREEN_PLAYTIME       1419
#define IDC_SCORE_SCREEN_DIFFICULTY     1420
#define IDC_MISSION_EVAL_STAR_1         1421
#define IDC_MISSION_EVAL_STAR_2         1422
#define IDC_MISSION_EVAL_STAR_3         1423
#define IDC_MISSION_EVAL_STAR_4         1424
#define IDC_MISSION_EVAL_STAR_5         1425
#define IDC_TIME_TO_FINISH_STAR_1       1426
#define IDC_LEVEL_OF_PLAY_STAR_1        1427
#define IDC_SAVES_LOADED_STAR_1         1428
#define IDC_SECONDARY_MISSIONS_STAR_1   1429
#define IDC_TIME_TO_FINISH_STAR_2       1430
#define IDC_TIME_TO_FINISH_STAR_3       1431
#define IDC_TIME_TO_FINISH_STAR_4       1432
#define IDC_TIME_TO_FINISH_STAR_5       1433
#define IDC_LEVEL_OF_PLAY_STAR_2        1434
#define IDC_LEVEL_OF_PLAY_STAR_3        1435
#define IDC_LEVEL_OF_PLAY_STAR_4        1436
#define IDC_LEVEL_OF_PLAY_STAR_5        1437
#define IDC_SAVES_LOADED_STAR_2         1438
#define IDC_SAVES_LOADED_STAR_3         1439
#define IDC_SAVES_LOADED_STAR_4         1440
#define IDC_SAVES_LOADED_STAR_5         1441
#define IDC_SECONDARY_MISSIONS_STAR_2   1442
#define IDC_SECONDARY_MISSIONS_STAR_3   1443
#define IDC_SECONDARY_MISSIONS_STAR_4   1444
#define IDC_SECONDARY_MISSIONS_STAR_5   1445
#define IDC_WHEEL_RADIUS                1446
#define IDC_SCORE_SCREEN_LOADS          1446
#define IDC_SCORE_SCREEN_SECONDARY      1447
#define IDC_RANK_IMAGE                  1448
#define IDC_SS_MISSION_NAME             1449
#define IDC_SS_BACKGROUND               1450
#define IDC_SCORE_SCREEN_BUILDING_COUNT 1451
#define IDC_SCORE_SCREEN_VEHICLE_COUNT  1452
#define IDC_SCORE_SCREEN_ENEMY_COUNT    1453
#define IDC_SCORE_SCREEN_HEAD_COUNT     1454
#define IDC_SCORE_SCREEN_TORSO_COUNT    1455
#define IDC_SCORE_SCREEN_ARM_COUNT      1456
#define IDC_SCORE_SCREEN_CROTCH_COUNT   1457
#define IDC_SCORE_SCREEN_LEG_COUNT      1458
#define IDC_SCORE_SCREEN_BONUS          1459
#define IDC_TYPE_STATIC                 1460
#define IDC_CHAT_AREA                   1461
#define IDC_SEND                        1462
#define IDC_BUY                         1463
#define IDC_ENLISTED_PURCHASE_01        1464
#define IDC_PROGRESS                    1465
#define IDC_STATUSTEXT                  1466
#define IDC_PROGRESSTEXT                1467
#define IDC_TRANSFERTEXT                1469
#define IDC_SOLDIERS_STATIC             1470
#define IDC_TRUST_CLIENTS_CHECK         1471
#define IDC_TEAM_ICON                   1472
#define IDC_SERVER_RESTART_CHECK        1472
#define IDC_DEFENSES_ICON               1473
#define IDC_BUILDING01_ICON             1473
#define IDC_NOD_TEAM_ICON               1473
#define IDC_REMIX_TEAMS_CHECK           1473
#define IDC_REFINERY_ICON               1474
#define IDC_NOD_BUILDING01_ICON         1474
#define IDC_POWERPLANT_ICON             1475
#define IDC_SOLDIER_FACTORY_ICON        1476
#define IDC_VEHICLE_FACTORY_ICON        1477
#define IDC_BUILDING01_HEALTHBAR        1478
#define IDC_BUILDING02_ICON             1479
#define IDC_BUILDING02_HEALTHBAR        1480
#define IDC_BUILDING03_ICON             1481
#define IDC_BUILDING03_HEALTHBAR        1482
#define IDC_BUILDING04_ICON             1483
#define IDC_BUILDING04_HEALTHBAR        1484
#define IDC_BUILDING05_ICON             1485
#define IDC_BUILDING05_HEALTHBAR        1486
#define IDC_GDI_BUILDING06_ICON         1487
#define IDC_GDI_BUILDING06_HEALTHBAR    1488
#define IDC_NOD_BUILDING01_HEALTHBAR    1489
#define IDC_NOD_BUILDING02_ICON         1490
#define IDC_NOD_BUILDING02_HEALTHBAR    1491
#define IDC_NOD_BUILDING03_ICON         1492
#define IDC_NOD_BUILDING03_HEALTHBAR    1493
#define IDC_NOD_BUILDING04_ICON         1494
#define IDC_NOD_BUILDING04_HEALTHBAR    1495
#define IDC_NOD_BUILDING05_ICON         1496
#define IDC_NOD_BUILDING05_HEALTHBAR    1497
#define IDC_NOD_BUILDING06_ICON         1498
#define IDC_NOD_BUILDING06_HEALTHBAR    1499
#define IDC_GDI_BUILDING01_ICON         1500
#define IDC_GDI_BUILDING01_HEALTHBAR    1501
#define IDC_GDI_BUILDING02_ICON         1502
#define IDC_GDI_BUILDING02_HEALTHBAR    1503
#define IDC_GDI_BUILDING03_ICON         1504
#define IDC_GDI_BUILDING03_HEALTHBAR    1505
#define IDC_GDI_BUILDING04_ICON         1506
#define IDC_GDI_BUILDING04_HEALTHBAR    1507
#define IDC_GDI_BUILDING05_ICON         1508
#define IDC_GDI_BUILDING05_HEALTHBAR    1509
#define IDC_GDI_LIST_CTRL               1510
#define IDC_GDI_TEAM_ICON               1511
#define IDC_CAN_REPAIR_BUILDINGS_CHECK  1512
#define IDC_HOTKEY_TEXT_01              1513
#define IDC_HOTKEY_TEXT_02              1514
#define IDC_HOTKEY_TEXT_03              1515
#define IDC_HOTKEY_TEXT_04              1516
#define IDC_HOTKEY_TEXT_05              1517
#define IDC_HOTKEY_TEXT_06              1518
#define IDC_HOTKEY_TEXT_07              1519
#define IDC_HOTKEY_TEXT_08              1520
#define IDC_HOTKEY_TEXT_09              1521
#define IDC_HOTKEY_TEXT_10              1522
#define IDC_DRIVER_IS_ALWAYS_GUNNER_CHECK 1523
#define IDC_OPTIONS_MULTIPLAY_SUICIDE   1524
#define IDC_SPAWN_WEAPONS_CHECK         1524
#define IDC_OPTIONS_MULTIPLAY_CHANGE_TEAMS 1525
#define IDC_HELP_KEY_RETURN             1526
#define IDC_HELP_KEY_FORWARD            1527
#define IDC_HELP_KEY_WEAPON1            1528
#define IDC_HELP_KEY_WEAPON2            1529
#define IDC_HELP_KEY_WEAPON3            1530
#define IDC_HELP_KEY_LEFT               1531
#define IDC_HELP_KEY_RIGHT              1532
#define IDC_HELP_KEY_BACKWARD           1533
#define IDC_HELP_KEY_ACTION             1534
#define IDC_HELP_KEY_NEXT_WEAPON        1535
#define IDC_HELP_KEY_CYCLE_POGS         1536
#define IDC_HELP_KEY_JUMP               1537
#define IDC_HELP_KEY_CROUCH             1538
#define IDC_HELP_KEY_WALK               1539
#define IDC_HELP_KEY_OBJECTIVES         1540
#define IDC_PROGRESS_INFO               1541
#define IDC_HELP_KEY_WEAPON4            1541
#define IDC_HELP_BUTTON                 1542
#define IDC_HELP_KEY_WEAPON5            1542
#define IDC_GAME_TIME_IMAGE             1543
#define IDC_WEAPONS_TEXT                1544
#define IDC_POWERUPS_TEXT               1545
#define IDC_FRIENDLIES_TEXT             1546
#define IDC_SHOTS_FIRED_TEXT            1547
#define IDC_ACCURACY_TEXT               1548
#define IDC_ENEMIES_KILLED_TEXT         1549
#define IDC_VEHICLES_TIME_TEXT          1550
#define IDC_VEHICLE_KILLS_TEXT          1551
#define IDC_VEHICLE_SQUISHES_TEXT       1552
#define IDC_VEHICLES_KILLED_TEXT        1553
#define IDC_BUILDINGS_KILLED_TEXT       1554
#define IDC_HEAD_SHOT_TEXT              1555
#define IDC_TORSO_SHOT_TEXT             1556
#define IDC_ARM_SHOT_TEXT               1557
#define IDC_CROTCH_SHOT_TEXT            1558
#define IDC_LEG_SHOT_TEXT               1559
#define IDC_GAME_TIME_HOURS_TEXT        1560
#define IDC_GAME_TIME_MINS_TEXT         1561
#define IDC_GAME_TIME_SECS_TEXT         1562
#define IDC_MENU_START_PRACTICE_GAME_BUTTON 1563
#define IDC_CHEAT_WARNING_TEXT          1564
#define IDC_CHEAT_BUTTON                1565
#define IDC_COST_X2_TEXT                1566
#define IDC_AUTO_AIM_TEXT               1566
#define IDC_STATIC_TEXT_01              1567
#define IDC_AUTO_AIM_TEXT2              1567
#define IDC_STATIC_TEXT_02              1568
#define IDC_STATIC_TEXT_03              1569
#define IDC_STATIC_TEXT_04              1570
#define IDC_STATIC_TEXT_05              1571
#define IDC_STATIC_TEXT_06              1572
#define IDC_STATIC_TEXT_07              1573
#define IDC_STATIC_TEXT_08              1574
#define IDC_STATIC_TEXT_09              1575
#define IDC_STATIC_TEXT_10              1576
#define IDC_STATIC_TEXT_11              1577
#define IDC_STATIC_TEXT_12              1578
#define IDC_STATIC_TEXT_13              1579
#define IDC_STATIC_TEXT_14              1580
#define IDC_STATIC_TEXT_15              1581
#define IDC_STATIC_TEXT_16              1582
#define IDC_KEY_NAME_01                 1583
#define IDC_TITLE_TEXT_2                1583
#define IDC_KEY_NAME_02                 1584
#define IDC_MVP_TEXT                    1584
#define IDC_KEY_NAME_03                 1585
#define IDC_TIME_TEXT                   1585
#define IDC_KEY_NAME_04                 1586
#define IDC_WINNER_TEAM_SCORE_TEXT      1586
#define IDC_KEY_NAME_05                 1587
#define IDC_LOSER_TEAM_SCORE_TEXT       1587
#define IDC_KEY_NAME_06                 1588
#define IDC_MENU_TEXT_NEXT_MAP          1588
#define IDC_STATIC_TEXT_17              1589
#define IDC_STATIC_TEXT_18              1590
#define IDC_STATIC_TEXT_19              1591
#define IDC_STATIC_TEXT_20              1592
#define IDC_TIME_REMAINING_TEXT         1600
#define IDC_MAPNAME_TEXT                1601
#define IDC_GDI_SCORE                   1602
#define IDC_NOD_SCORE                   1603
#define IDC_STARTGAME                   1604
#define IDC_GDI_CLAN                    1605
#define IDC_NOD_CLAN                    1606
#define IDC_CHOOSESIDE_COMBO            1607
#define IDC_PLAYING_FOR_NOD_TEXT        1608
#define IDC_NICK_EDIT1                  1609
#define IDC_NICK_EDIT2                  1610
#define IDC_NICK_EDIT3                  1611
#define IDC_NICK_EDIT4                  1612
#define IDC_NICK_EDIT5                  1613
#define IDC_NICK_EDIT6                  1614
#define IDC_NICK_EDIT7                  1615
#define IDC_SLAVE_ENABLE1               1616
#define IDC_MENU_MP_LAN_SLAVE_SERVER_BUTTON 1617
#define IDC_PASS_EDIT1                  1617
#define IDC_SLAVE_ENABLE3               1618
#define IDC_SLAVE_ENABLE2               1619
#define IDC_SLAVE_ENABLE4               1620
#define IDC_SLAVE_ENABLE5               1621
#define IDC_SLAVE_ENABLE6               1622
#define IDC_SLAVE_ENABLE7               1623
#define IDC_PORT_EDIT1                  1624
#define IDC_PORT_EDIT2                  1625
#define IDC_PORT_EDIT3                  1626
#define IDC_PORT_EDIT4                  1627
#define IDC_PORT_EDIT5                  1628
#define IDC_PORT_EDIT6                  1629
#define IDC_PORT_EDIT7                  1630
#define IDC_WIN1_LIST_CTRL              1631
#define IDC_PASS_EDIT2                  1631
#define IDC_TITLE_TEXT_1                1632
#define IDC_PASS_EDIT3                  1632
#define IDC_BETA_TEST_TEXT              1633
#define IDC_PASS_EDIT4                  1633
#define IDC_AIM_TEXT                    1634
#define IDC_PASS_EDIT5                  1634
#define IDC_PLAYER_HEALTH_TEXT          1635
#define IDC_PASS_EDIT6                  1635
#define IDC_SUPPLIES_TEXT               1636
#define IDC_PASS_EDIT7                  1636
#define IDC_REINFORCEMENTS_TEXT         1637
#define IDC_BODY_ARMOR_TEXT             1638
#define IDC_SLAVE1_SETTINGS             1640
#define IDS_STATIC                      1642
#define IDC_SLAVE2_SETTINGS             1643
#define IDC_GAMMA_SETTING               1643
#define IDC_SLAVE3_SETTINGS             1644
#define IDC_BRIGHTNESS_SETTING          1644
#define IDC_SLAVE4_SETTINGS             1645
#define IDC_CONTRAST_SETTING            1645
#define IDC_SLAVE5_SETTINGS             1646
#define IDC_SLAVE6_SETTINGS             1647
#define IDC_SKIN_SELECTION_BUTTON       1647
#define IDC_SLAVE7_SETTINGS             1648
#define IDC_MP_OPTIONS_BUTTON           1648
#define IDC_LIGHTING_MODE               1650
#define IDC_TEXTURE_FILTER              1651
#define IDC_MENU_GAMESPY_JOIN           1652
#define IDC_MENU_GAMESPY_HOST           1653
#define IDC_MENU_MP_INTERNET_WOL        1654
#define IDC_MENU_GAMESPY_OPTIONS        1654
#define IDC_MENU_MP_INTERNET_GAMESPY    1655
#define IDC_MENU_MP_INTERNET_WOL2       1655
#define IDC_GAMESPY_ICON                1656
#define IDC_IMAGE_CTRL                  1657
#define IDC_REFUSAL_TEXT                1658
#define IDC_BUY_RENEGADE                1659
#define IDC_VISIT_WEB_BUTTON            11037
#define IDC_EXIT_BUTTON                 11038

//
//	These button IDs are used for "simple" buttons
// that's only purpose is to start a new dialog.
//
//	Note:  These IDs are coded using defines instead of an enumeration
// because the resource compiler ignores enumerations.
//
#define DIALOG_LINK_FIRST						11000

#define IDC_MENU_START_SP_GAME_BUTTON		DIALOG_LINK_FIRST
#define IDC_MENU_START_MP_GAME_BUTTON		IDC_MENU_START_SP_GAME_BUTTON + 1
#define IDC_MENU_VIEW_SCORES_BUTTON			IDC_MENU_START_MP_GAME_BUTTON + 1
#define IDC_MENU_OPTIONS_BUTTON				IDC_MENU_VIEW_SCORES_BUTTON + 1
#define IDC_MENU_START_CAMPAIGN_BUTTON		IDC_MENU_OPTIONS_BUTTON + 1
#define IDC_MENU_START_TUTORIAL_BUTTON		IDC_MENU_START_CAMPAIGN_BUTTON + 1
#define IDC_MENU_LOAD_SP_GAME_BUTTON		IDC_MENU_START_TUTORIAL_BUTTON + 1
#define IDC_MENU_DIFFCULTY01_BUTTON			(IDC_MENU_LOAD_SP_GAME_BUTTON + 1)
#define IDC_MENU_DIFFCULTY02_BUTTON			IDC_MENU_DIFFCULTY01_BUTTON + 1
#define IDC_MENU_DIFFCULTY03_BUTTON			IDC_MENU_DIFFCULTY02_BUTTON + 1
#define IDC_MENU_DIFFCULTY04_BUTTON			IDC_MENU_DIFFCULTY03_BUTTON + 1
#define IDC_MENU_CONTROLS_BUTTON				IDC_MENU_DIFFCULTY04_BUTTON + 1
#define IDC_MENU_CHARACTER_BUTTON			IDC_MENU_CONTROLS_BUTTON + 1
#define IDC_MENU_CHEATS_BUTTON				IDC_MENU_CHARACTER_BUTTON + 1
#define IDC_MENU_TECH_BUTTON					IDC_MENU_CHEATS_BUTTON + 1
#define IDC_MENU_MOVIES_BUTTON				IDC_MENU_TECH_BUTTON + 1
#define IDC_MENU_PREVIEWS_BUTTON				IDC_MENU_MOVIES_BUTTON + 1
#define IDC_MENU_CREDITS_BUTTON				IDC_MENU_PREVIEWS_BUTTON + 1
#define IDC_MENU_QUIT_BUTTON					IDC_MENU_CREDITS_BUTTON + 1
#define IDC_MENU_INGAME_MENU_BUTTON			IDC_MENU_QUIT_BUTTON + 1
#define IDC_MENU_MAIN_MENU_BUTTON			IDC_MENU_INGAME_MENU_BUTTON + 1
#define IDC_MENU_SAVE_SP_GAME_BUTTON		IDC_MENU_MAIN_MENU_BUTTON + 1
#define IDC_MENU_MP_LAN_BUTTON				IDC_MENU_SAVE_SP_GAME_BUTTON + 1
#define IDC_MENU_MP_INTERNET_BUTTON			IDC_MENU_MP_LAN_BUTTON + 1
#define IDC_MENU_MP_START_SERVER_BUTTON	IDC_MENU_MP_INTERNET_BUTTON + 1
#define IDC_MENU_MP_JOIN_GAME_BUTTON		IDC_MENU_MP_START_SERVER_BUTTON + 1
#define IDC_MENU_MP_START_GAME_BUTTON		IDC_MENU_MP_JOIN_GAME_BUTTON + 1

#define IDC_MENU_MULTIPLAY_OPTS_BUTTON		IDC_MENU_MP_START_GAME_BUTTON + 1

#define IDC_MENU_MP_MENU_BUTTON				IDC_MENU_MULTIPLAY_OPTS_BUTTON + 1
#define IDC_MENU_MP_INTERNET_GAME_BUTTON	IDC_MENU_MP_MENU_BUTTON + 1
//efine IDC_MENU_START_SKIRMISH_GAME_BUTTON	IDC_MENU_MP_MENU_BUTTON + 1
//efine IDC_MENU_MP_INTERNET_GAME_BUTTON	IDC_MENU_START_SKIRMISH_GAME_BUTTON + 1
#define IDC_MENU_MP_LAN_GAME_BUTTON			IDC_MENU_MP_INTERNET_GAME_BUTTON + 1
#define IDC_MENU_MP_LAN_JOIN_BUTTON			IDC_MENU_MP_LAN_GAME_BUTTON + 1
#define IDC_MENU_MP_LAN_START_BUTTON		IDC_MENU_MP_LAN_JOIN_BUTTON + 1

#define DIALOG_LINK_LAST						IDC_MENU_MP_LAN_START_BUTTON + 1

//
//	These IDs are for non-link controls that reside
// inside a dialog.
//
#define IDC_MENU_BACK_BUTTON					DIALOG_LINK_LAST + 1
#define IDC_LOAD_GAME_LIST_CTRL				IDC_MENU_BACK_BUTTON + 1
#define IDC_DELETE_GAME_BUTTON				IDC_LOAD_GAME_LIST_CTRL + 1
#define IDC_LOAD_GAME_BUTTON					IDC_DELETE_GAME_BUTTON + 1
#define IDC_MENU_MP_START_BUTTON				IDC_LOAD_GAME_BUTTON + 1
#define IDC_MENU_MP_JOING_BUTTON				IDC_MENU_MP_START_BUTTON + 1
#define IDC_MP_NICKNAME_EDIT					IDC_LOAD_GAME_BUTTON + 1
#define IDC_MP_CONNECTING_OK_BUTTON			IDC_MP_NICKNAME_EDIT + 1
#define IDC_MP_PASSWORD_VALID_COMMAND		IDC_MP_CONNECTING_OK_BUTTON + 1

#define IDC_MP_SHORTCUT_NEWS					IDC_MP_PASSWORD_VALID_COMMAND + 1
#define IDC_MP_SHORTCUT_CLANS					IDC_MP_SHORTCUT_NEWS + 1
#define IDC_MP_SHORTCUT_RANKINGS				IDC_MP_SHORTCUT_CLANS + 1
#define IDC_MP_SHORTCUT_BUDDIES				IDC_MP_SHORTCUT_RANKINGS + 1
#define IDC_MP_SHORTCUT_ACCOUNT				IDC_MP_SHORTCUT_BUDDIES + 1
#define IDC_MP_SHORTCUT_NET_STATUS			IDC_MP_SHORTCUT_ACCOUNT + 1
#define IDC_MP_SHORTCUT_PAGE_BUDDY			IDC_MP_SHORTCUT_NET_STATUS + 1
#define IDC_MP_SHORTCUT_CHAT					IDC_MP_SHORTCUT_PAGE_BUDDY + 1
#define IDC_MP_SHORTCUT_GAMELIST					IDC_MP_SHORTCUT_CHAT + 1
#define IDC_MP_SHORTCUT_QUICKMATCH_OPTIONS				IDC_MP_SHORTCUT_GAMELIST + 1
#define IDC_MP_SHORTCUT_INTERNET_OPTIONS	IDC_MP_SHORTCUT_QUICKMATCH_OPTIONS + 1
#define IDC_MENU_MP_LAN_HOST_BUTTON			IDC_MP_SHORTCUT_INTERNET_OPTIONS + 1

#define IDC_CNC_PURCHASE_VEHICLE_BUTTON	12000
#define IDC_CNC_PURCHASE_POWERUP_BUTTON	12001
#define IDC_CNC_PURCHASE_CHARACTER_BUTTON	12002

#define IDC_BUY_GDI_VEHICLE01_BUTTON		2000
#define IDC_BUY_GDI_VEHICLE02_BUTTON		2001
#define IDC_BUY_GDI_VEHICLE03_BUTTON		2002
#define IDC_BUY_GDI_VEHICLE04_BUTTON		2003
#define IDC_BUY_NOD_VEHICLE01_BUTTON		2004
#define IDC_BUY_NOD_VEHICLE02_BUTTON		2005
#define IDC_BUY_NOD_VEHICLE03_BUTTON		2006
#define IDC_BUY_NOD_VEHICLE04_BUTTON		2007
#define IDC_BUY_NOD_VEHICLE05_BUTTON		2008
#define IDC_BUY_NOD_VEHICLE06_BUTTON		2009
#define IDC_BUY_NOD_VEHICLE07_BUTTON		2010

#define IDC_BUY_POWERUP01_BUTTON				3000
#define IDC_BUY_POWERUP02_BUTTON				3001
#define IDC_BUY_POWERUP03_BUTTON				3002
#define IDC_BUY_POWERUP04_BUTTON				3003
#define IDC_BUY_POWERUP05_BUTTON				3004
#define IDC_BUY_POWERUP06_BUTTON				3005
#define IDC_BUY_POWERUP07_BUTTON				3006
#define IDC_BUY_POWERUP08_BUTTON				3007
#define IDC_BUY_POWERUP09_BUTTON				3008
#define IDC_BUY_POWERUP10_BUTTON				3009
#define IDC_BUY_POWERUP11_BUTTON				3010
#define IDC_BUY_POWERUP12_BUTTON				3011
#define IDC_BUY_POWERUP13_BUTTON				3012
#define IDC_BUY_POWERUP14_BUTTON				3013

#define IDC_BUY_CHARACTER01_BUTTON			4000
#define IDC_BUY_CHARACTER02_BUTTON			4001
#define IDC_BUY_CHARACTER03_BUTTON			4002
#define IDC_BUY_CHARACTER04_BUTTON			4003
#define IDC_BUY_CHARACTER05_BUTTON			4004
#define IDC_BUY_CHARACTER06_BUTTON			4005
#define IDC_BUY_CHARACTER07_BUTTON			4006
#define IDC_BUY_CHARACTER08_BUTTON			4007
#define IDC_BUY_CHARACTER09_BUTTON			4008
#define IDC_BUY_CHARACTER10_BUTTON			4009
#define IDC_BUY_CHARACTER11_BUTTON			4010
#define IDC_BUY_CHARACTER12_BUTTON			4011
#define IDC_BUY_CHARACTER13_BUTTON			4012


#define IDC_SCORE_TABCTRL						40001
#define IDC_GENERIC_TABCTRL					40002

const DialogResource * GetRenegadeDialog(RenegadeDialogID id);

#endif //__DIALOG_RESOURCE_H

