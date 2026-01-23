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
 *                 Project Name : LevelEdit                                                    *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/MainFrm.cpp                  $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 3/12/02 4:01p                                               $*
 *                                                                                             *
 *                    $Revision:: 140                                                         $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "StdAfx.h"
#include "LevelEdit.h"
#include "MainFrm.h"
#include "LevelEditView.h"
#include "deviceselectiondialog.h"
#include "Utils.h"
#include "outputform.h"
#include "cameramgr.h"
#include "MouseMgr.h"
#include "lightambientform.h"
#include "camerasettingsform.h"
#include "ww3d.h"
#include "filemgr.h"
#include "sceneeditor.h"
#include "editorassetmgr.h"
#include "_assetmgr.h"
#include "welcomedialog.h"
#include "regkeys.h"
#include "icons.h"
#include "part_ldr.h"
#include "includefilesdialog.h"
#include "_wwaudio.h"
#include "SoundScene.h"
#include "sunlightdialog.h"
#include "generatevisdialog.h"
#include "visualoptionsdialog.h"
#include "filelocations.h"
#include "viserrorreportdialog.h"
#include "mesh.h"
#include "PathDebugPlotter.h"
#include "Pathfind.h"
#include "Path.h"
#include "chunkio.h"
#include "saveload.h"
#include "definitionfactory.h"
#include "definitionfactorymgr.h"
#include "definitionmgr.h"
#include "definition.h"
#include "definitionclassids.h"
#include "presetslibform.h"
#include "presetmgr.h"
#include "groupmgr.h"
#include "mover.h"
#include "node.h"
#include "combatsaveload.h"
#include "combat.h"
#include "assets.h"
#include "export.h"
#include "timemgr.h"
#include "scriptmgr.h"
#include "scriptman.h"
#include "gameobjmanager.h"
#include "nodemgr.h"
#include "editorsaveload.h"
#include "surfaceeffects.h"
#include "progressui.h"
#include "DefinitionUtils.h"
#include "physstaticsavesystem.h"
#include "PathfindPortal.h"
#include "objectnode.h"
#include "vehicle.h"
#include "staticphys.h"
#include "GotoLocationDialog.h"
#include "VisStatsDialog.h"
#include "NodeIDStartDialog.h"
#include "PathfindSectorBuilder.h"
#include "vismgr.h"
#include "optimizingvisdialog.h"
#include "stringlibrarydialog.h"
#include "stringsmgr.h"
#include "heightdb.h"
#include "editconversationlistdialog.h"
#include "pathmgr.h"
#include "pathsolve.h"
#include "generictextdialog.h"
#include "musicproppage.h"
#include "skyproppage.h"
#include "WeatherPropPage.h"
#include "memlogdialog.h"
#include "translatedb.h"
#include "gotoobjectbyiddialog.h"
#include "exportpresetsdialog.h"
#include "presetexport.h"
#include "dazzle.h"
#include "sphereobj.h"
#include "ringobj.h"
#include "soundrobj.h"
#include "levelsettingsdialog.h"
#include "ffactorylist.h"
#include "importtranslationdialog.h"
#include "reportmgr.h"
#include "missingtranslationreportdialog.h"
#include "translationexportdialog.h"
#include "ccamera.h"
#include "rcstringextractor.h"
#include "dialogmgr.h"
#include "mixfiledatabase.h"
#include "assetpackagemgr.h"
#include "lightsolveoptionsdialog.h"
#include <algorithm>


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
//	Constants
/////////////////////////////////////////////////////////////////////////////
const int CAMERA_MENU_INDEX				= 8;


/////////////////////////////////////////////////////////////////////////////
//	Globals
/////////////////////////////////////////////////////////////////////////////
EditorFileFactoryClass	_TheEditorFileFactory;


/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_COMMAND(IDM_OBJECT_MANIPULATE, OnObjectManipulate)
	ON_UPDATE_COMMAND_UI(IDM_OBJECT_MANIPULATE, OnUpdateObjectManipulate)
	ON_COMMAND(IDM_MODE_CAMERA, OnModeCamera)
	ON_UPDATE_COMMAND_UI(IDM_MODE_CAMERA, OnUpdateModeCamera)
	ON_COMMAND(IDM_DELETE_SELECTION, OnDeleteSelection)
	ON_COMMAND(IDM_MODE_ORBIT, OnModeOrbit)
	ON_UPDATE_COMMAND_UI(IDM_MODE_ORBIT, OnUpdateModeOrbit)
	ON_COMMAND(IDM_COORD_WORLD, OnCoordWorld)
	ON_UPDATE_COMMAND_UI(IDM_COORD_WORLD, OnUpdateCoordWorld)
	ON_COMMAND(IDM_COORD_VIEW, OnCoordView)
	ON_UPDATE_COMMAND_UI(IDM_COORD_VIEW, OnUpdateCoordView)
	ON_COMMAND(IDM_COORD_OBJECT, OnCoordObject)
	ON_UPDATE_COMMAND_UI(IDM_COORD_OBJECT, OnUpdateCoordObject)
	ON_COMMAND(IDM_RESTRICT_X, OnRestrictX)
	ON_COMMAND(IDM_RESTRICT_Y, OnRestrictY)
	ON_COMMAND(IDM_RESTRICT_Z, OnRestrictZ)
	ON_UPDATE_COMMAND_UI(IDM_RESTRICT_Z, OnUpdateRestrictZ)
	ON_UPDATE_COMMAND_UI(IDM_RESTRICT_Y, OnUpdateRestrictY)
	ON_UPDATE_COMMAND_UI(IDM_RESTRICT_X, OnUpdateRestrictX)
	ON_COMMAND(IDM_CAMERA_FRONT, OnCameraFront)
	ON_COMMAND(IDM_CAMERA_BACK, OnCameraBack)
	ON_COMMAND(IDM_CAMERA_LEFT, OnCameraLeft)
	ON_COMMAND(IDM_CAMERA_RIGHT, OnCameraRight)
	ON_COMMAND(IDM_CAMERA_TOP, OnCameraTop)
	ON_COMMAND(IDM_CAMERA_BOTTOM, OnCameraBottom)
	ON_COMMAND(IDM_DROP_TO_GROUND, OnDropToGround)
	ON_UPDATE_COMMAND_UI(IDM_DROP_TO_GROUND, OnUpdateDropToGround)
	ON_COMMAND(IDM_MODE_WALKTHROUGH, OnModeWalkthrough)
	ON_UPDATE_COMMAND_UI(IDM_MODE_WALKTHROUGH, OnUpdateModeWalkthrough)
	ON_COMMAND(IDM_VIEW_CHANGE_DEVICE, OnViewChangeDevice)
	ON_COMMAND(IDM_LEVEL_EXPORT, OnLevelExport)
	ON_COMMAND(IDM_VIEW_FULLSCREEN, OnViewFullscreen)
	ON_UPDATE_COMMAND_UI(IDM_VIEW_FULLSCREEN, OnUpdateViewFullscreen)
	ON_COMMAND(IDM_VIEW_AMBIENT_LIGHT_DLG, OnViewAmbientLightDlg)
	ON_UPDATE_COMMAND_UI(IDM_VIEW_AMBIENT_LIGHT_DLG, OnUpdateViewAmbientLightDlg)
	ON_UPDATE_COMMAND_UI(IDM_VIEW_TOOLBAR_CAMERA_SETTINGS, OnUpdateViewToolbarCameraSettings)
	ON_COMMAND(IDM_VIEW_TOOLBAR_CAMERA_SETTINGS, OnViewToolbarCameraSettings)
	ON_COMMAND(IDM_CAMERA_DEPTH_LESS, OnCameraDepthLess)
	ON_COMMAND(IDM_CAMERA_DEPTH_MORE, OnCameraDepthMore)
	ON_COMMAND(IDM_SAVE_PRESETS, OnSavePresets)
	ON_UPDATE_COMMAND_UI(IDM_SAVE_PRESETS, OnUpdateSavePresets)
	ON_COMMAND(IDM_TOGGLE_SNAPPOINTS, OnToggleSnappoints)
	ON_UPDATE_COMMAND_UI(IDM_TOGGLE_SNAPPOINTS, OnUpdateToggleSnappoints)
	ON_COMMAND(IDM_ALWAYS_EXPORT, OnAlwaysExport)
	ON_WM_DESTROY()
	ON_COMMAND(IDM_CHANGE_BASE, OnChangeBase)
	ON_COMMAND(IDM_CAM_SPEED_INCREASE, OnCamSpeedIncrease)
	ON_COMMAND(IDM_CAM_SPEED_DECREASE, OnCamSpeedDecrease)
	ON_COMMAND(IDM_CAM_RESET, OnCamReset)
	ON_WM_KEYDOWN()
	ON_COMMAND(IDM_GEN_VIS, OnGenVis)
	ON_COMMAND(IDM_TOGGLE_VIS_WINDOW, OnToggleVisWindow)
	ON_COMMAND(IDM_ANI_BACK_FRAME, OnAniBackFrame)
	ON_COMMAND(IDM_ANI_FIRST_FRAME, OnAniFirstFrame)
	ON_COMMAND(IDM_ANI_FWD_FRAME, OnAniFwdFrame)
	ON_COMMAND(IDM_ANI_LAST_FRAME, OnAniLastFrame)
	ON_COMMAND(IDM_ANI_PAUSE, OnAniPause)
	ON_COMMAND(IDM_ANI_PLAY, OnAniPlay)
	ON_COMMAND(IDM_ANI_STOP, OnAniStop)
	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateEditUndo)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, OnUpdateEditCut)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditPaste)
	ON_COMMAND(IDM_CREATE_GROUP, OnCreateGroup)
	ON_UPDATE_COMMAND_UI(IDM_CREATE_GROUP, OnUpdateCreateGroup)
	ON_COMMAND(IDM_TOGGLE_LIGHTS, OnToggleLights)
	ON_UPDATE_COMMAND_UI(IDM_TOGGLE_LIGHTS, OnUpdateToggleLights)
	ON_COMMAND(IDM_REPARTITION, OnRepartition)
	ON_COMMAND(IDM_ALLOW_AUTO_UPDATES, OnAllowAutoUpdates)
	ON_UPDATE_COMMAND_UI(IDM_ALLOW_AUTO_UPDATES, OnUpdateAllowAutoUpdates)
	ON_COMMAND(IDM_ROTATE_LEFT, OnRotateLeft)
	ON_COMMAND(IDM_ROTATE_RIGHT, OnRotateRight)
	ON_COMMAND(IDM_CALC_ALL_LIGHTS, OnCalcAllLights)
	ON_COMMAND(IDM_CALC_SEL_LIGHTS, OnCalcSelLights)
	ON_UPDATE_COMMAND_UI(IDM_CALC_SEL_LIGHTS, OnUpdateCalcSelLights)
	ON_COMMAND(IDM_LOCK_OBJECTS, OnLockObjects)
	ON_COMMAND(IDM_UNLOCK_OBJECTS, OnUnlockObjects)
	ON_UPDATE_COMMAND_UI(IDM_LOCK_OBJECTS, OnUpdateLockObjects)
	ON_UPDATE_COMMAND_UI(IDM_UNLOCK_OBJECTS, OnUpdateUnlockObjects)
	ON_COMMAND(IDM_EDIT_INCLUDES, OnEditIncludes)
	ON_COMMAND(IDM_VOL_DEC, OnVolDec)
	ON_COMMAND(IDM_VOL_INC, OnVolInc)
	ON_COMMAND(IDM_BACKGROUND_SETTINGS, OnBackgroundSettings)
	ON_COMMAND(IDM_TOGGLE_MUSIC, OnToggleMusic)
	ON_UPDATE_COMMAND_UI(IDM_TOGGLE_MUSIC, OnUpdateToggleMusic)
	ON_COMMAND(IDM_TOGGLE_SOUNDS, OnToggleSounds)
	ON_UPDATE_COMMAND_UI(IDM_TOGGLE_SOUNDS, OnUpdateToggleSounds)
	ON_COMMAND(IDM_VIEW_AGGREGATE_CHILDREN, OnViewAggregateChildren)
	ON_UPDATE_COMMAND_UI(IDM_VIEW_AGGREGATE_CHILDREN, OnUpdateViewAggregateChildren)
	ON_COMMAND(IDM_TOGGLE_SUNLIGHT, OnToggleSunlight)
	ON_UPDATE_COMMAND_UI(IDM_TOGGLE_SUNLIGHT, OnUpdateToggleSunlight)
	ON_COMMAND(IDM_EDIT_SUNLIGHT, OnEditSunlight)
	ON_COMMAND(IDM_AUTO_GEN_VIS, OnAutoGenVis)
	ON_COMMAND(IDM_VISUAL_OPTIONS, OnVisualOptions)
	ON_COMMAND(IDM_VIS_INVERT, OnVisInvert)
	ON_COMMAND(IDM_VIEW_VIS_POINTS, OnViewVisPoints)
	ON_UPDATE_COMMAND_UI(IDM_VIEW_VIS_POINTS, OnUpdateViewVisPoints)
	ON_COMMAND(IDM_SPECIFY_ASSET_DATABASE, OnSpecifyAssetDatabase)
	ON_UPDATE_COMMAND_UI(IDM_VIS_INVERT, OnUpdateVisInvert)
	ON_COMMAND(IDM_TOGGLE_BACKFACES, OnToggleBackfaces)
	ON_UPDATE_COMMAND_UI(IDM_TOGGLE_BACKFACES, OnUpdateToggleBackfaces)
	ON_COMMAND(IDM_TOGGLE_WIREFRAME_MODE, OnToggleWireframeMode)
	ON_UPDATE_COMMAND_UI(IDM_TOGGLE_WIREFRAME_MODE, OnUpdateToggleWireframeMode)
	ON_COMMAND(IDM_VIEW_VIS_ERRORS, OnViewVisErrors)
	ON_COMMAND(IDM_USE_VIS_CAMERA, OnUseVisCamera)
	ON_UPDATE_COMMAND_UI(IDM_USE_VIS_CAMERA, OnUpdateUseVisCamera)
	ON_COMMAND(IDM_GENERATE_OBSTACLE_VOLUMES, OnGenerateObstacleVolumes)
	ON_COMMAND(IDM_VIS_DISABLE, OnVisDisable)
	ON_UPDATE_COMMAND_UI(IDM_VIS_DISABLE, OnUpdateVisDisable)
	ON_COMMAND(IDM_DISPLAY_PATHS, OnDisplayPaths)
	ON_UPDATE_COMMAND_UI(IDM_DISPLAY_PATHS, OnUpdateDisplayPaths)
	ON_COMMAND(IDM_TEST_PATHFIND, OnTestPathfind)
	ON_COMMAND(IDM_DISPLAY_PATHFIND_SECTORS, OnDisplayPathfindSectors)
	ON_UPDATE_COMMAND_UI(IDM_DISPLAY_PATHFIND_SECTORS, OnUpdateDisplayPathfindSectors)
	ON_COMMAND(IDM_DISPLAY_PATHFIND_PORTALS, OnDisplayPathfindPortals)
	ON_UPDATE_COMMAND_UI(IDM_DISPLAY_PATHFIND_PORTALS, OnUpdateDisplayPathfindPortals)
	ON_COMMAND(IDM_DISPLAY_PATHFIND_RAW_DATA, OnDisplayPathfindRawData)
	ON_UPDATE_COMMAND_UI(IDM_DISPLAY_PATHFIND_RAW_DATA, OnUpdateDisplayPathfindRawData)
	ON_COMMAND(IDM_EXPORT_PATHFIND, OnExportPathfind)
	ON_COMMAND(IDM_IMPORT_PATHFIND, OnImportPathfind)
	ON_COMMAND(IDM_IMPORT_VIS, OnImportVis)
	ON_COMMAND(IDM_EXPORT_VIS, OnExportVis)
	ON_COMMAND(IDM_VIEW_VIS_WINDOW, OnViewVisWindow)
	ON_UPDATE_COMMAND_UI(IDM_VIEW_VIS_WINDOW, OnUpdateViewVisWindow)
	ON_COMMAND(IDM_TOGGLE_MAN_VIS_POINTS, OnToggleManVisPoints)
	ON_UPDATE_COMMAND_UI(IDM_TOGGLE_MAN_VIS_POINTS, OnUpdateToggleManVisPoints)
	ON_COMMAND(IDM_DISCARD_PATHFIND, OnDiscardPathfind)
	ON_COMMAND(IDM_EXPORT_SOUNDS, OnDoDebug1)
	ON_COMMAND(IDM_IMPORT_SOUNDS, OnDoDebug2)
	ON_COMMAND(IDM_IMPORT_DYN_OBJS, OnImportDynObjs)
	ON_COMMAND(IDM_EXPORT_DYN_OBJS, OnExportDynObjs)
	ON_COMMAND(IDM_TEST_GOTO, OnTestGoto)
	ON_COMMAND(IDM_IMPORT_STATIC, OnImportStatic)
	ON_COMMAND(IDM_EXPORT_STATIC, OnExportStatic)
	ON_COMMAND(IDM_DISPLAY_WEB, OnDisplayWeb)
	ON_UPDATE_COMMAND_UI(IDM_DISPLAY_WEB, OnUpdateDisplayWeb)
	ON_UPDATE_COMMAND_UI(IDM_DISPLAY_FULL_PATHS, OnUpdateDisplayFullPaths)
	ON_COMMAND(IDM_DISPLAY_FULL_PATHS, OnDisplayFullPaths)
	ON_COMMAND(IDM_ESCAPE, OnEscape)
	ON_COMMAND(IDM_DISPLAY_VIS_GRID, OnDisplayVisGrid)
	ON_UPDATE_COMMAND_UI(IDM_DISPLAY_VIS_GRID, OnUpdateDisplayVisGrid)
	ON_COMMAND(IDM_DISPLAY_VIS_SECTORS, OnDisplayVisSectors)
	ON_UPDATE_COMMAND_UI(IDM_DISPLAY_VIS_SECTORS, OnUpdateDisplayVisSectors)
	ON_COMMAND(IDM_PRELIT_MULTIPASS, OnPrelitMultipass)
	ON_UPDATE_COMMAND_UI(IDM_PRELIT_MULTIPASS, OnUpdatePrelitMultipass)
	ON_COMMAND(IDM_PRELIT_MULTITEX, OnPrelitMultitex)
	ON_UPDATE_COMMAND_UI(IDM_PRELIT_MULTITEX, OnUpdatePrelitMultitex)
	ON_COMMAND(IDM_PRELIT_VERTEX, OnPrelitVertex)
	ON_UPDATE_COMMAND_UI(IDM_PRELIT_VERTEX, OnUpdatePrelitVertex)
	ON_COMMAND(IDM_DEBUG_SCRIPTS_MODE, OnDebugScriptsMode)
	ON_UPDATE_COMMAND_UI(IDM_DEBUG_SCRIPTS_MODE, OnUpdateDebugScriptsMode)
	ON_COMMAND(IDM_IMPORT_LIGHTS, OnImportLights)
	ON_COMMAND(IDM_VIEW_SOUND_SPHERES, OnViewSoundSpheres)
	ON_UPDATE_COMMAND_UI(IDM_VIEW_SOUND_SPHERES, OnUpdateViewSoundSpheres)
	ON_COMMAND(IDM_VIEW_LIGHT_SPHERES, OnViewLightSpheres)
	ON_UPDATE_COMMAND_UI(IDM_VIEW_LIGHT_SPHERES, OnUpdateViewLightSpheres)
	ON_COMMAND(IDM_IMPORT_SUNLIGHT, OnImportSunlight)
	ON_COMMAND(IDM_DISCARD_VIS, OnDiscardVis)
	ON_COMMAND(IDM_INCREASE_SPHERE, OnIncreaseSphere)
	ON_UPDATE_COMMAND_UI(IDM_INCREASE_SPHERE, OnUpdateIncreaseSphere)
	ON_COMMAND(IDM_DECREASE_SPHERE, OnDecreaseSphere)
	ON_UPDATE_COMMAND_UI(IDM_DECREASE_SPHERE, OnUpdateDecreaseSphere)
	ON_COMMAND(IDM_RUN_JOB, OnRunJob)
	ON_COMMAND(IDM_GOTO_LOCATION, OnGotoLocation)
	ON_COMMAND(IDM_EXPORT_THUMBNAILS, OnExportThumbnails)
	ON_COMMAND(IDM_VIS_STATS, OnVisStats)
	ON_COMMAND(IDM_SET_NODE_ID_START, OnSetNodeIdStart)
	ON_COMMAND(IDM_BUILD_DYNA_CULLSYS, OnBuildDynaCullsys)
	ON_COMMAND(IDM_IMPORT_VIS_REMAP_DATA, OnImportVisRemapData)
	ON_COMMAND(IDM_EXPORT_VIS_REMAP_DATA, OnExportVisRemapData)
	ON_UPDATE_COMMAND_UI(IDM_BULK_REPLACE, OnUpdateBulkReplace)
	ON_COMMAND(IDM_BULK_REPLACE, OnBulkReplace)
	ON_COMMAND(IDM_CHECK_IDS, OnCheckIds)
	ON_COMMAND(IDM_EXPORT_LIGHTS, OnExportLights)
	ON_COMMAND(IDM_OPTIMIZE_VIS_DATA, OnOptimizeVisData)
	ON_COMMAND(IDM_DISPLAY_VIS_GRID_LITTLE_BOXES, OnDisplayVisGridLittleBoxes)
	ON_UPDATE_COMMAND_UI(IDM_DISPLAY_VIS_GRID_LITTLE_BOXES, OnUpdateDisplayVisGridLittleBoxes)
	ON_COMMAND(IDM_EDIT_STRINGS_TABLE, OnEditStringsTable)
	ON_COMMAND(IDM_EXPORT_STRING_IDS, OnExportStringIds)
	ON_COMMAND(IDM_IMPORT_STRING_IDS, OnImportStringIds)
	ON_COMMAND(IDM_BUILD_FLIGHT_INFO, OnBuildFlightInfo)
	ON_COMMAND(IDM_TOGGLE_ATTENUATION_SPHERES, OnToggleAttenuationSpheres)
	ON_UPDATE_COMMAND_UI(IDM_TOGGLE_ATTENUATION_SPHERES, OnUpdateToggleAttenuationSpheres)
	ON_COMMAND(IDM_FIX_ID_COLLISIONS, OnFixIdCollisions)
	ON_COMMAND(IDM_RESET_DYNA_CULL_SYSTEM, OnResetDynaCullSystem)
	ON_COMMAND(IDM_DISPLAY_LIGHT_VECTORS, OnDisplayLightVectors)
	ON_UPDATE_COMMAND_UI(IDM_DISPLAY_LIGHT_VECTORS, OnUpdateDisplayLightVectors)
	ON_COMMAND(IDM_DISPLAY_STATIC_ANIM_OBJECTS, OnDisplayStaticAnimObjects)
	ON_UPDATE_COMMAND_UI(IDM_DISPLAY_STATIC_ANIM_OBJECTS, OnUpdateDisplayStaticAnimObjects)
	ON_COMMAND(IDM_EDIT_CONVERSATIONS, OnEditConversations)
	ON_COMMAND(IDM_BUILDING_POWER_ON, OnBuildingPowerOn)
	ON_UPDATE_COMMAND_UI(IDM_BUILDING_POWER_ON, OnUpdateBuildingPowerOn)
	ON_COMMAND(IDM_CREATE_PROXIES, OnCreateProxies)
	ON_UPDATE_COMMAND_UI(IDM_CREATE_PROXIES, OnUpdateCreateProxies)
	ON_COMMAND(IDM_CHECK_IN_PRESET_CHANGES, OnCheckInPresetChanges)
	ON_UPDATE_COMMAND_UI(IDM_CHECK_IN_PRESET_CHANGES, OnUpdateCheckInPresetChanges)
	ON_COMMAND(IDM_IMMEDIATE_PRESET_CHECKIN, OnImmediatePresetCheckin)
	ON_UPDATE_COMMAND_UI(IDM_IMMEDIATE_PRESET_CHECKIN, OnUpdateImmediatePresetCheckin)
	ON_COMMAND(IDM_REMAP_IDS, OnRemapIds)
	ON_UPDATE_COMMAND_UI(IDM_REMAP_IDS, OnUpdateRemapIds)
	ON_COMMAND(IDM_VERIFY_CULLING, OnVerifyCulling)
	ON_COMMAND(IDM_ENABLE_VIS_SECTOR_FALLBACK, OnEnableVisSectorFallback)
	ON_UPDATE_COMMAND_UI(IDM_ENABLE_VIS_SECTOR_FALLBACK, OnUpdateEnableVisSectorFallback)
	ON_COMMAND(IDM_RUN_MANUAL_VIS_POINTS, OnRunManualVisPoints)
	ON_COMMAND(IDM_SHOW_EDITOR_OBJECTS, OnShowEditorObjects)
	ON_UPDATE_COMMAND_UI(IDM_SHOW_EDITOR_OBJECTS, OnUpdateShowEditorObjects)
	ON_COMMAND(IDM_ADD_CHILD_NODE, OnAddChildNode)
	ON_UPDATE_COMMAND_UI(IDM_ADD_CHILD_NODE, OnUpdateAddChildNode)
	ON_COMMAND(IDM_CHECK_MEMLOG, OnCheckMemlog)
	ON_COMMAND(IDM_MAKE_VIS_POINT, OnMakeVisPoint)
	ON_COMMAND(IDM_EXPORT_STRING_TABLE, OnExportStringTable)
	ON_COMMAND(IDM_GOTO_OBJECT, OnGotoObject)
	ON_COMMAND(IDM_EXPORT_PRESETS, OnExportPresets)
	ON_COMMAND(IDM_IMPORT_PRESETS, OnImportPresets)
	ON_COMMAND(IDM_BATCH_EXPORT, OnBatchExport)
	ON_COMMAND(IDM_LOCK_VIS_SAMPLE_POINT, OnLockVisSamplePoint)
	ON_UPDATE_COMMAND_UI(IDM_LOCK_VIS_SAMPLE_POINT, OnUpdateLockVisSamplePoint)
	ON_COMMAND(IDM_TOGGLE_TERRAIN_SELECTION, OnToggleTerrainSelection)
	ON_UPDATE_COMMAND_UI(IDM_TOGGLE_TERRAIN_SELECTION, OnUpdateToggleTerrainSelection)
	ON_COMMAND(IDM_EXPORT_PRESET_FILE_DEPENDENCIES, OnExportPresetFileDependencies)
	ON_COMMAND(IDM_IMPORT_STRINGS, OnImportStrings)
	ON_COMMAND(IDM_EXPORT_TILE_LIST, OnExportTileList)
	ON_COMMAND(IDM_REMAP_UNIMPORTANT_IDS, OnRemapUnimportantIds)
	ON_COMMAND(IDM_EDIT_LEVEL_SETTINGS, OnEditLevelSettings)
	ON_COMMAND(IDM_EXPORT_TRANSLATION_DATA, OnExportTranslationData)
	ON_COMMAND(IDM_IMPORT_TRANSLATION_DATA, OnImportTranslationData)
	ON_COMMAND(IDM_COMPRESS_TEXTURES, OnCompressTextures)
	ON_UPDATE_COMMAND_UI(IDM_COMPRESS_TEXTURES, OnUpdateCompressTextures)
	ON_COMMAND(IDM_EXPORT_FILE_USAGE_REPORT, OnExportFileUsageReport)
	ON_COMMAND(IDM_EXPORT_MISSING_TRANSLATION_REPORT, OnExportMissingTranslationReport)
	ON_COMMAND(IDM_EXPORT_LANGUAGE, OnExportLanguage)
	ON_WM_INITMENUPOPUP()
	ON_COMMAND(IDM_ALWAYS_LOCAL_EXPORT, OnAlwaysLocalExport)
	ON_COMMAND(IDM_EXPORT_INSTALLER_LANGUAGE_VERSION, OnExportInstallerLanguageVersion)
	ON_COMMAND(IDM_EXTRACT_INSTALLER_RC_STRINGS, OnExtractInstallerRcStrings)
	ON_COMMAND(IDM_EXTRACT_RC_STRINGS, OnExtractRcStrings)
	ON_COMMAND(IDM_CAMERA_PERSPECTIVE, OnCameraPerspective)
	ON_COMMAND(IDM_CAMERA_ORTHOGRAPHIC, OnCameraOrthographic)
	ON_UPDATE_COMMAND_UI(IDM_CAMERA_ORTHOGRAPHIC, OnUpdateCameraOrthographic)
	ON_UPDATE_COMMAND_UI(IDM_CAMERA_PERSPECTIVE, OnUpdateCameraPerspective)
	ON_COMMAND(IDM_MOD_EXPORT, OnModExport)
	ON_COMMAND(IDM_SAVE_LEVEL, OnSaveLevel)
	ON_COMMAND(IDM_COMPUTE_VERTEX_SOLVE, OnComputeVertexSolve)
	ON_COMMAND(IDM_MODE_HEIGHT_EDIT, OnModeHeightEdit)
	ON_UPDATE_COMMAND_UI(IDM_MODE_HEIGHT_EDIT, OnUpdateModeHeightEdit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	IDS_CAM_LOCATION,
	IDS_ANIM_FRAMES,
	IDS_TEXTURE_MEM,
	IDS_POLY_COUNT
};

extern CLevelEditApp theApp;

//////////////////////////////////////////////////////////////////////////
//
//  CMainFrame
//
CMainFrame::CMainFrame (void)
	:	m_UpdateTimer (0),
		m_GrowthRate (0.25F),
		m_pCurrentDoc (NULL),
		m_MaxCamProfiles (0)
{
	return ;	
}

//////////////////////////////////////////////////////////////////////////
//
//  ~CMainFrame
//
CMainFrame::~CMainFrame (void)
{
	return ;	
}


//////////////////////////////////////////////////////////////////////////
//
//  OnCreate
//
//////////////////////////////////////////////////////////////////////////
int
CMainFrame::OnCreate (LPCREATESTRUCT lpCreateStruct)
{
	ProgressUIMgrClass::Set_Status_Text ("Initializing User Interface...");

	if (CFrameWnd::OnCreate (lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	// Reset the status bar text
	m_wndStatusBar.SetPaneText (1, "");
	m_wndStatusBar.SetPaneText (2, "");
	m_wndStatusBar.SetPaneText (3, "");

	// Allow docking on any station
	EnableDocking (CBRS_ALIGN_ANY);

	// Create our global icon imagelist
	m_ImageList.Create (16, 16, ILC_COLOR | ILC_MASK, 2, 2);
	for (int index = 0; index < ICON_COUNT; index ++) {
		
		// Load this icon and add it to our imagelist
		HICON hicon = (HICON)::LoadImage (::AfxGetResourceHandle (),
													 MAKEINTRESOURCE (ICON_RESOURCE_IDS[index]),
													 IMAGE_ICON,
													 16,
													 16,
													 LR_SHARED);
		m_ImageList.Add (hicon);
	}

	ImageList_SetOverlayImage (m_ImageList, PLUS_OVERLAY_ICON, 1);

	//
	// Create the application's toolbars
	//
	Create_Toolbars ();
	Create_Camera_Menu ();

	// Put the focus on the main view
	GetDlgItem (AFX_IDW_PANE_FIRST)->SetFocus ();

	Restore_Window_State ();
	SetForegroundWindow ();

	return 0;
}


////////////////////////////////////////////////////////////////////////////
//
//  Create_Camera_Menu
//
////////////////////////////////////////////////////////////////////////////
void
CMainFrame::Create_Camera_Menu (void)
{
	m_MaxCamProfiles = 0;

	//
	//	Get the path to the camera ini file
	//
	CString full_path = ::Get_File_Mgr ()->Make_Full_Path (ALWAYS_INI_DIR);
	full_path = Make_Path (full_path, "cameras.ini");

	CMenu *main_menu = GetMenu ();
	CMenu *camera_menu = main_menu->GetSubMenu (CAMERA_MENU_INDEX);

	//
	//	Get the INI file which contains the data for this viewer
	//
	INIClass *ini_file = ::Get_INI (full_path);
	if (ini_file != NULL) {

		//
		//	Enumerate the entries...
		//
		int count = ini_file->Entry_Count ("Profile_List");
		for (int index = 0; index < count; index ++) {
			StringClass	profile_name (0, true);
			ini_file->Get_String (profile_name, "Profile_List", ini_file->Get_Entry ("Profile_List", index));

			//
			//	Add this entry to the menu
			//
			camera_menu->AppendMenu (MF_STRING | MF_ENABLED, IDM_FIRST_CAM_PROFILE + index, profile_name);			
			m_CameraProfiles.Add (profile_name);
			m_MaxCamProfiles ++;
		}

		//
		//	Set the check next to the first entry in the list
		//
		camera_menu->CheckMenuRadioItem (IDM_FIRST_CAM_PROFILE,
							IDM_FIRST_CAM_PROFILE + m_MaxCamProfiles, IDM_FIRST_CAM_PROFILE, MF_BYCOMMAND);
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Restore_Window_State
//
////////////////////////////////////////////////////////////////////////////
void
CMainFrame::Restore_Window_State (void)
{
	//
	// Read the cached window information from the registry
	//
	/*CRect rect;
	rect.left	= theApp.GetProfileInt ("Window", "Left", -1);
	rect.right	= theApp.GetProfileInt ("Window", "Right", -1);
	rect.top		= theApp.GetProfileInt ("Window", "Top", -1);
	rect.bottom	= theApp.GetProfileInt ("Window", "Bottom", -1);
	bool is_max	= (theApp.GetProfileInt ("Window", "Maximized", -1) == 1);

	if (is_max) {
		::ShowWindow (m_hWnd, SW_MAXIMIZE);
	} else if (rect.left > 0 && rect.right > 0 && rect.top > 0 && rect.bottom > 0) {		
		::SetWindowPos (m_hWnd, NULL, rect.left, rect.top, rect.Width (), rect.Height (), SWP_NOZORDER);
	}*/

	//
	//	Always show the editor maximized
	//
	::ShowWindow (m_hWnd, SW_MAXIMIZE);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  Create_Toolbars
//
void
CMainFrame::Create_Toolbars (void)
{
	//m_DialogBar.Create (this);
	
	//DockableFormClass *new_form = new OutputFormClass;
	//new_form->Create (CWnd::FromHandlePermanent (m_DialogBar.GetReBarCtrl ()), 101);

	//m_DialogBar.AddBar (new_form);

	m_TestBar.Create (this, CBRS_RIGHT, 101);
	m_TestBar.SetBarStyle (m_TestBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC | CBRS_GRIPPER);
	m_TestBar.EnableDocking (CBRS_ALIGN_LEFT | CBRS_ALIGN_RIGHT);

	// Create all the application's toolbars
	//m_wndToolBar.Create (this);
	//m_wndToolBar.LoadToolBar (IDR_MAINFRAME);
	m_AniToolbar.Create (this);
	m_AniToolbar.LoadToolBar (IDR_ANI_TOOLBAR);
	m_AniToolbar.SetWindowText ("Animation");
	m_wndBottomToolBar.Create (this);
	m_wndBottomToolBar.LoadToolBar (IDR_BOTTOM_TOOLBAR);
	m_OutputToolbar.Create (new OutputFormClass, "Output", this, 102);
	m_AmbientLightToolbar.Create (new LightAmbientFormClass, "Ambient Light", this, 103);
	m_CameraSettingsToolbar.Create (new CameraSettingsFormClass, "Camera Settings", this, 104);

	// Set the styles on the main toolbar
	/*m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);*/
	m_AniToolbar.SetBarStyle(m_AniToolbar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);
	m_wndBottomToolBar.SetBarStyle(m_wndBottomToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);

	 m_wndBottomToolBar.ModifyStyle(0, TBSTYLE_FLAT); 

	// Set the allowed docking stations for each toolbar
	//m_wndToolBar.EnableDocking(CBRS_ALIGN_TOP);
	m_AniToolbar.EnableDocking(CBRS_ALIGN_ANY);
	m_wndBottomToolBar.EnableDocking(CBRS_ALIGN_TOP);
	m_OutputToolbar.EnableDocking (CBRS_ALIGN_BOTTOM | CBRS_ALIGN_TOP);
	m_AmbientLightToolbar.EnableDocking (0L);
	m_CameraSettingsToolbar.EnableDocking (0L);
	
	// Dock our dialogs
	//DockControlBar (&m_wndToolBar);	
	DockControlBar (&m_wndBottomToolBar);
	DockControlBar (&m_OutputToolbar, AFX_IDW_DOCKBAR_BOTTOM);		
	
	// Hide our floating dialog toolbars
	ShowControlBar (&m_AmbientLightToolbar, false, false);
	ShowControlBar (&m_CameraSettingsToolbar, false, false);
	ShowControlBar (&m_AniToolbar, false, false);
			

	CRect window_rect;
	GetWindowRect (&window_rect);

	//
	// Float our hidden dialog toolbars
	//
	FloatControlBar (&m_AmbientLightToolbar,
						  CPoint (window_rect.left + ((window_rect.Width () << 1) / 3), window_rect.top + (window_rect.Height () >> 1)),
						  CBRS_ALIGN_TOP);

	FloatControlBar (&m_CameraSettingsToolbar,
						  CPoint (window_rect.left + ((window_rect.Width () << 1) / 3), window_rect.top + (window_rect.Height () >> 1)),
						  CBRS_ALIGN_TOP);

	FloatControlBar (&m_AniToolbar,
						  CPoint (window_rect.left + 10, (window_rect.top + window_rect.Height ()) - 10),
						  CBRS_ALIGN_BOTTOM);

	// Allow timer updates while the mouse is hovering over the animation toolbar
	::SetProp (m_AniToolbar, "ALLOW_UPDATE", (HANDLE)1);
	
	RecalcLayout ();
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  PreCreateWindow
//
BOOL
CMainFrame::PreCreateWindow (CREATESTRUCT& cs)
{
	// Allow the base class to process this message
	return CFrameWnd::PreCreateWindow (cs);
}


/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG


//////////////////////////////////////////////////////////////////////////
//
//  OnFinalRelease
//
void
CMainFrame::OnFinalRelease (void)
{
	// Allow the base class to process this message
	CFrameWnd::OnFinalRelease ();
	return ;
}


void
PatsTextCallback (AudibleSoundClass * /* sound_obj */, const StringClass &text, uint32 user_param)
{
	CString message;
	message.Format ("Got text callback: %s\tUser = %d\r\n", (LPCTSTR)text, user_param);
	::Output_Message (message);
	return ;
}



//////////////////////////////////////////////////////////////////////////
//
//  OnCreateClient
//
BOOL
CMainFrame::OnCreateClient
(
    LPCREATESTRUCT lpcs,
    CCreateContext* pContext
)
{
    // Let MFC know that we want to be the main window
    theApp.m_pMainWnd = this;

	// Allow the base class to process this message
	BOOL retval = CFrameWnd::OnCreateClient (lpcs, pContext);

	// Get the path where this exe was run from
	TCHAR filename[MAX_PATH];
	::GetModuleFileName (NULL, filename, sizeof (filename));

	// Does this filename have a directory?
	LPTSTR pathname = ::strrchr (filename, '\\');
	if (pathname) {

		//
		// Set this directory as the current directory
		//
		pathname[0] = 0;
		::SetCurrentDirectory (filename);		
	}
	
	//
	// Get a pointer to the current doc
	//
	m_pCurrentDoc = (CLevelEditDoc *)GetActiveDocument ();

	//
	//	Configure the INI file path for the game engine to use
	//
	Configure_File_Factories ();


	// Get a pointer to the main view
	CLevelEditView *pview = (CLevelEditView *)GetActiveView ();
	if (pview)
	{
		ProgressUIMgrClass::Set_Status_Text ("Initializing Render Device...");

		// Initialize the WW3D engine using the window handle from
		// the main view
		BOOL retval = (WW3D::Init ((HWND)*pview) == WW3D_ERROR_OK);
		ASSERT (retval);

		// Show a dialog to the user asking them which
		// device they would like to use.
		CDeviceSelectionDialog devicedialog;
		if (devicedialog.DoModal () == IDOK) {
			// Pass the user's selection from the onto the view
			pview->Set_Render_Device_Info (devicedialog.GetDeviceIndex (), devicedialog.GetBitsPerPixel ());
		} else {
			// Stop the application from running
			PostMessage (WM_CLOSE);
			retval = false;
		}
	}

	//
	// Register a loader for particle emitters
	//
	WW3DAssetManager::Get_Instance()->Register_Prototype_Loader (&_ParticleEmitterLoader);
	WW3DAssetManager::Get_Instance()->Register_Prototype_Loader (&_SphereLoader);
	WW3DAssetManager::Get_Instance()->Register_Prototype_Loader (&_RingLoader);
	WW3DAssetManager::Get_Instance()->Register_Prototype_Loader (&_SoundRenderObjLoader);
	WW3DAssetManager::Get_Instance()->Set_WW3D_Load_On_Demand (true);

	// Initialize the render device
	::Get_Main_View ()->Initialize_Render_Device ();

	if (m_pCurrentDoc != NULL) {
		m_pCurrentDoc->Init_Scene ();
	}

	//
	// Start up the audio system
	//
	ProgressUIMgrClass::Set_Status_Text ("Initializing Audio Device...");
	_pTheAudioManager = WWAudioClass::Create_Instance();
	_pTheAudioManager->Initialize ();	
	_pTheAudioManager->Register_Text_Callback (PatsTextCallback, 1115);

	//
	//	Initialize the game
	//
	ProgressUIMgrClass::Set_Status_Text ("Initializing Combat Engine...");
	DialogMgrClass::Initialize ("stylemgr.ini");
	CombatManager::Set_Observers_Active (false);
	CombatManager::Init ();	
	CombatManager::Pre_Load_Level ();
	CombatManager::Set_I_Am_Client (true);
	CombatManager::Set_I_Am_Server (true);
	SurfaceEffectsManager::Enable_Effects (false);
	SurfaceEffectsManager::Set_Mode (SurfaceEffectsManager::MODE_OFF);
	PathMgrClass::Initialize ();

	CString scripts_path = ::Get_File_Mgr ()->Make_Full_Path (SCRIPTS_PATH);
	::SetCurrentDirectory (scripts_path);
	ScriptMgrClass::Initialize ();	
	ScriptManager::Init ();
	WW3D::Enable_Static_Sort_Lists(true);

	//
	//	Load the presets
	//
	ProgressUIMgrClass::Set_Status_Text ("Loading preset library...");
	PresetsFormClass::Load_Databases ();

	//
	//	Attach the sound system to the camera
	//
	SoundSceneClass *sound_scene = _pTheAudioManager->Get_Sound_Scene ();
	if (sound_scene != NULL) {
		sound_scene->Attach_Listener_To_Obj (::Get_Camera_Mgr ()->Get_Camera ());
	}

	// Return the true/false result code
	return retval;
}


//////////////////////////////////////////////////////////////////////////
//
//  Configure_File_Factories
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::Configure_File_Factories (void)
{
	_TheFileFactory = &_TheEditorFileFactory;

	//
	//	Prime the file factory with the path to INI files
	//
	CString ini_path = ::Get_File_Mgr ()->Make_Full_Path (ALWAYS_INI_DIR);
	Delimit_Path (ini_path);
	_TheEditorFileFactory.Add_Search_Path (ini_path);


	//
	//	Configure a file factory that knows where to find INI files while in the editor
	//
	/*CString ini_path = ::Get_File_Mgr ()->Make_Full_Path (ALWAYS_INI_DIR);
	Delimit_Path (ini_path);

	_EditorINIFileFactory.Set_Sub_Directory (ini_path);
	_EditorINIFileFactory.Set_Strip_Path (true);

	//
	//	Add these file factories to the list of factories
	//
	_TheEditorFileFactory.Add_FileFactory (&_EditorINIFileFactory);
	_TheEditorFileFactory.Add_FileFactory (&_EditorBaseFileFactory);

	_TheFileFactory = &_TheEditorFileFactory;*/
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnClose
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnClose (void)
{
	if (PresetMgrClass::Are_Presets_Dirty ()) {

		//
		//	Does the user want to save the preset changes?
		//
		if (::Message_Box (m_hWnd, IDS_SAVE_PRESETS_MSG, IDS_SAVE_PRESETS_TITLE, MB_ICONQUESTION | MB_YESNO) == IDYES) {
			PresetMgrClass::Check_In_Presets ();
		} else {
			PresetMgrClass::Discard_Preset_Changes ();
		}
	}

	// Allow the base class to process this message
	CFrameWnd::OnClose ();
	return; 
}


//////////////////////////////////////////////////////////////////////////
//
//  OnObjectManipulate
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnObjectManipulate (void) 
{
	// Get a pointer to the mouse manager for the editor
	MouseMgrClass *pmousemgr = ::Get_Mouse_Mgr ();

	// Were we succesful in getting the mouse manager?
	ASSERT (pmousemgr != NULL);
	if (pmousemgr != NULL) {
		pmousemgr->Set_Mouse_Mode (MouseMgrClass::MODE_OBJECT_MANIPULATE);
	}

	return ;	
}


//////////////////////////////////////////////////////////////////////////
//
//  OnUpdateObjectManipulate
//
void
CMainFrame::OnUpdateObjectManipulate (CCmdUI* pCmdUI) 
{
	// Get a pointer to the mouse manager for the editor
	MouseMgrClass *pmousemgr = ::Get_Mouse_Mgr ();

	// Were we succesful in getting the mouse manager?
	ASSERT (pmousemgr != NULL);
	if (pmousemgr != NULL) {
		pCmdUI->SetCheck ((BOOL)pmousemgr->Get_Mouse_Mode () == MouseMgrClass::MODE_OBJECT_MANIPULATE);
	}

	return ;	
}


//////////////////////////////////////////////////////////////////////////
//
//  OnModeCamera
//
void
CMainFrame::OnModeCamera (void) 
{
	// Get a pointer to the mouse manager for the editor
	MouseMgrClass *pmousemgr = ::Get_Mouse_Mgr ();

	// Were we succesful in getting the mouse manager?
	ASSERT (pmousemgr != NULL);
	if (pmousemgr != NULL) {
		pmousemgr->Set_Mouse_Mode (MouseMgrClass::MODE_CAMERA_DEFAULT);
	}

	return ;	
}


//////////////////////////////////////////////////////////////////////////
//
//  OnUpdateModeCamera
//
void
CMainFrame::OnUpdateModeCamera (CCmdUI* pCmdUI) 
{
	// Get a pointer to the mouse manager for the editor
	MouseMgrClass *pmousemgr = ::Get_Mouse_Mgr ();

	// Were we succesful in getting the mouse manager?
	ASSERT (pmousemgr != NULL);
	if (pmousemgr != NULL) {
		pCmdUI->SetCheck ((BOOL)pmousemgr->Get_Mouse_Mode () == MouseMgrClass::MODE_CAMERA_DEFAULT);
	}

	return ;	
}

//////////////////////////////////////////////////////////////////////////
//
//  OnDeleteSelection
//
void
CMainFrame::OnDeleteSelection (void) 
{
	::Get_Scene_Editor ()->Delete_Nodes ();
	return ;
}

//////////////////////////////////////////////////////////////////////////
//
//  OnModeOrbit
//
void
CMainFrame::OnModeOrbit (void) 
{
	// Get a pointer to the mouse manager for the editor
	MouseMgrClass *pmousemgr = ::Get_Mouse_Mgr ();

	// Were we succesful in getting the mouse manager?
	ASSERT (pmousemgr != NULL);
	if (pmousemgr != NULL) {
		pmousemgr->Set_Mouse_Mode (MouseMgrClass::MODE_CAMERA_ORBIT);
	}

	return ;	
}


//////////////////////////////////////////////////////////////////////////
//
//  OnUpdateModeOrbit
//
void
CMainFrame::OnUpdateModeOrbit (CCmdUI* pCmdUI) 
{
	// Get a pointer to the mouse manager for the editor
	MouseMgrClass *pmousemgr = ::Get_Mouse_Mgr ();

	// Were we succesful in getting the mouse manager?
	ASSERT (pmousemgr != NULL);
	if (pmousemgr != NULL) {
		pCmdUI->SetCheck ((BOOL)pmousemgr->Get_Mouse_Mode () == MouseMgrClass::MODE_CAMERA_ORBIT);
	}

	return ;	
}


//////////////////////////////////////////////////////////////////////////
//
//  OnCoordWorld
//
void
CMainFrame::OnCoordWorld (void)
{
	::Get_Current_Document ()->Set_Coord_System (CLevelEditDoc::COORD_WORLD);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnUpdateCoordWorld
//
void
CMainFrame::OnUpdateCoordWorld (CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (::Get_Current_Document ()->Get_Coord_System () == CLevelEditDoc::COORD_WORLD);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnCoordView
//
void
CMainFrame::OnCoordView (void)
{
	::Get_Current_Document ()->Set_Coord_System (CLevelEditDoc::COORD_VIEW);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnUpdateCoordView
//
void
CMainFrame::OnUpdateCoordView (CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (::Get_Current_Document ()->Get_Coord_System () == CLevelEditDoc::COORD_VIEW);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnCoordObject
//
void
CMainFrame::OnCoordObject (void)
{
	::Get_Current_Document ()->Set_Coord_System (CLevelEditDoc::COORD_OBJECT);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnUpdateCoordObject
//
void
CMainFrame::OnUpdateCoordObject (CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (::Get_Current_Document ()->Get_Coord_System () == CLevelEditDoc::COORD_OBJECT);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnRestrictX
//
void
CMainFrame::OnRestrictX (void)
{
	CLevelEditDoc::AXIS_RESTRICTION restriction = CLevelEditDoc::RESTRICT_X;

	// Is we are already resticting to this axis, then turn off restriction
	if (::Get_Current_Document ()->Get_Axis_Restriction () == CLevelEditDoc::RESTRICT_X) {
		restriction = CLevelEditDoc::RESTRICT_NONE;
	}

	// Set the new axis restriction
	::Get_Current_Document ()->Set_Axis_Restriction (restriction);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnRestrictY
//
void
CMainFrame::OnRestrictY (void)
{
	CLevelEditDoc::AXIS_RESTRICTION restriction = CLevelEditDoc::RESTRICT_Y;

	// Is we are already resticting to this axis, then turn off restriction
	if (::Get_Current_Document ()->Get_Axis_Restriction () == CLevelEditDoc::RESTRICT_Y) {
		restriction = CLevelEditDoc::RESTRICT_NONE;
	}

	// Set the new axis restriction
	::Get_Current_Document ()->Set_Axis_Restriction (restriction);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnRestrictZ
//
void
CMainFrame::OnRestrictZ (void)
{
	CLevelEditDoc::AXIS_RESTRICTION restriction = CLevelEditDoc::RESTRICT_Z;

	// Is we are already resticting to this axis, then turn off restriction
	if (::Get_Current_Document ()->Get_Axis_Restriction () == CLevelEditDoc::RESTRICT_Z) {
		restriction = CLevelEditDoc::RESTRICT_NONE;
	}

	// Set the new axis restriction
	::Get_Current_Document ()->Set_Axis_Restriction (restriction);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnUpdateRestrictZ
//
void
CMainFrame::OnUpdateRestrictZ (CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (::Get_Current_Document ()->Get_Axis_Restriction () == CLevelEditDoc::RESTRICT_Z);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnUpdateRestrictY
//
void
CMainFrame::OnUpdateRestrictY (CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (::Get_Current_Document ()->Get_Axis_Restriction () == CLevelEditDoc::RESTRICT_Y);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnUpdateRestrictX
//
void
CMainFrame::OnUpdateRestrictX (CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (::Get_Current_Document ()->Get_Axis_Restriction () == CLevelEditDoc::RESTRICT_X);
	return ;
}

//////////////////////////////////////////////////////////////////////////
//
//  OnCameraFront
//
void
CMainFrame::OnCameraFront (void) 
{
	// Move the camera to this new position
	::Get_Camera_Mgr ()->Set_Camera_Pos (CameraMgr::CAMERA_FRONT);
	return ;
}

//////////////////////////////////////////////////////////////////////////
//
//  OnCameraBack
//
void
CMainFrame::OnCameraBack (void) 
{
	// Move the camera to this new position
	::Get_Camera_Mgr ()->Set_Camera_Pos (CameraMgr::CAMERA_BACK);
	return ;
}

//////////////////////////////////////////////////////////////////////////
//
//  OnCameraLeft
//
void
CMainFrame::OnCameraLeft (void) 
{
	// Move the camera to this new position
	::Get_Camera_Mgr ()->Set_Camera_Pos (CameraMgr::CAMERA_LEFT);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnCameraRight
//
void
CMainFrame::OnCameraRight (void) 
{
	// Move the camera to this new position
	::Get_Camera_Mgr ()->Set_Camera_Pos (CameraMgr::CAMERA_RIGHT);
	return ;
}

//////////////////////////////////////////////////////////////////////////
//
//  OnCameraTop
//
void
CMainFrame::OnCameraTop (void) 
{
	// Move the camera to this new position
	::Get_Camera_Mgr ()->Set_Camera_Pos (CameraMgr::CAMERA_TOP);
	return ;
}

//////////////////////////////////////////////////////////////////////////
//
//  OnCameraBottom
//
void
CMainFrame::OnCameraBottom (void) 
{
	// Move the camera to this new position
	::Get_Camera_Mgr ()->Set_Camera_Pos (CameraMgr::CAMERA_BOTTOM);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnDropToGround
//
void
CMainFrame::OnDropToGround (void)
{
	int mods = ::Get_Current_Document ()->Get_Mode_Modifiers ();
	::Get_Current_Document ()->Set_Mode_Modifiers (mods ^ MODE_MOD_DROP_TO_GROUND);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnUpdateDropToGround
//
void
CMainFrame::OnUpdateDropToGround (CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (::Get_Current_Document ()->Get_Mode_Modifiers () & MODE_MOD_DROP_TO_GROUND);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnModeWalkthrough
//
void
CMainFrame::OnModeWalkthrough (void)
{
	// Get a pointer to the mouse manager for the editor
	MouseMgrClass *pmousemgr = ::Get_Mouse_Mgr ();

	// Were we succesful in getting the mouse manager?
	ASSERT (pmousemgr != NULL);
	if (pmousemgr != NULL) {
		pmousemgr->Set_Mouse_Mode (MouseMgrClass::MODE_CAMERA_WALK);
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnUpdateModeWalkthrough
//
void
CMainFrame::OnUpdateModeWalkthrough (CCmdUI* pCmdUI) 
{
	// Get a pointer to the mouse manager for the editor
	MouseMgrClass *pmousemgr = ::Get_Mouse_Mgr ();

	// Were we succesful in getting the mouse manager?
	ASSERT (pmousemgr != NULL);
	if (pmousemgr != NULL) {
		pCmdUI->SetCheck ((BOOL)pmousemgr->Get_Mouse_Mode () == MouseMgrClass::MODE_CAMERA_WALK);
	}

	return ;	
}


//////////////////////////////////////////////////////////////////////////
//
//  OnUpdateModeWalkthrough
//
void
CMainFrame::OnViewChangeDevice (void) 
{
	CDeviceSelectionDialog devicedialog (false);
	if (devicedialog.DoModal () == IDOK) {
		// Pass the user's selection from the onto the view
		::Get_Main_View ()->Set_Render_Device_Info (devicedialog.GetDeviceIndex (), devicedialog.GetBitsPerPixel ());
		::Get_Main_View ()->Initialize_Render_Device ();
	}

	return ;	
}


//////////////////////////////////////////////////////////////////////////
//
//  OnLevelExport
//
void
CMainFrame::OnLevelExport (void)
{		
	// Determine which directory we should use as a default for the export
	CString full_path = theApp.GetProfileString (CONFIG_KEY, LAST_EXPORT_DIR_VALUE);
	if (full_path.GetLength () == 0) {
		full_path = ::Get_File_Mgr ()->Make_Full_Path (LEVELS_ASSET_DIR);		
	}
	_pThe3DAssetManager->Set_Current_Directory (full_path);


	// Use the level's filename to generate a default INI filename
	CString default_name = ::Get_Current_Document ()->GetPathName ();
	if (default_name.GetLength () > 0) {
		default_name = ::Get_Filename_From_Path (default_name);
		default_name = ::Asset_Name_From_Filename (default_name);
		default_name += ".mix";
	}

	CFileDialog saveFileDialog (false,
										 ".mix",
										 default_name,
										 OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER,
										 "Renegade Mix (*.mix)|*.mix||",
										 this);

	// Set the path, so it opens in the correct directory
	saveFileDialog.m_ofn.lpstrInitialDir = full_path;

	// Ask the user what commando level file they wish to save to
	if (saveFileDialog.DoModal () == IDOK) {		

		ExporterClass exporter;
		exporter.Export_Level (saveFileDialog.GetPathName ());
	}
	
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnViewFullscreen
//
void
CMainFrame::OnViewFullscreen (void) 
{
	// Toggle the fullscreen view
	::Get_Main_View ()->Set_Windowed (!::Get_Main_View ()->Is_Windowed ());
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnUpdateViewFullscreen
//
void
CMainFrame::OnUpdateViewFullscreen (CCmdUI* pCmdUI) 
{
	// Check or uncheck the menu item based on the current windowed mode
	pCmdUI->SetCheck (!::Get_Main_View ()->Is_Windowed ());
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnViewAmbientLightDlg
//
void
CMainFrame::OnViewAmbientLightDlg (void) 
{
	// Toggle the visiblity state of the toolbar
	BOOL show_dlg = !m_AmbientLightToolbar.IsWindowVisible ();	
	ShowControlBar (&m_AmbientLightToolbar, show_dlg, false);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnUpdateViewAmbientLightDlg
//
void
CMainFrame::OnUpdateViewAmbientLightDlg (CCmdUI* pCmdUI) 
{
	// Check the menu option if the toolbar is currently displayed
	pCmdUI->SetCheck (m_AmbientLightToolbar.IsWindowVisible ());
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnViewToolbarCameraSettings
//
void
CMainFrame::OnViewToolbarCameraSettings (void) 
{
	// Toggle the visiblity state of the toolbar
	BOOL show_dlg = !m_CameraSettingsToolbar.IsWindowVisible ();	
	ShowControlBar (&m_CameraSettingsToolbar, show_dlg, false);

	// Get a pointer to the camera settings dialog
	CameraSettingsFormClass *pform = (CameraSettingsFormClass *)m_CameraSettingsToolbar.Get_Form ();
	if (pform != NULL) {
		pform->Update_Controls ();
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnUpdateViewToolbarCameraSettings
//
void
CMainFrame::OnUpdateViewToolbarCameraSettings (CCmdUI* pCmdUI) 
{
	// Check the menu option if the toolbar is currently displayed
	pCmdUI->SetCheck (m_CameraSettingsToolbar.IsWindowVisible ());
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnCameraDepthLess
//
void
CMainFrame::OnCameraDepthLess (void) 
{
	CameraMgr *pcamera_mgr = ::Get_Camera_Mgr ();
	if (pcamera_mgr != NULL) {
		
		// Get the current 'near' clip plane
		float near_plane = 0;
		float far_plane = 0;
		pcamera_mgr->Get_Camera ()->Get_Clip_Planes (near_plane, far_plane);

		if (far_plane - 50 > 20) {
			// Set the new 'far' clip plane
			pcamera_mgr->Get_Camera ()->Set_Clip_Planes (near_plane, far_plane - 50);
		}

		// Make sure the scene is painted
		::Refresh_Main_View ();
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnCameraDepthMore
//
void
CMainFrame::OnCameraDepthMore (void)
{
	CameraMgr *pcamera_mgr = ::Get_Camera_Mgr ();
	if (pcamera_mgr != NULL) {
		
		// Get the current 'near' clip plane
		float near_plane = 0;
		float far_plane = 0;
		pcamera_mgr->Get_Camera ()->Get_Clip_Planes (near_plane, far_plane);

		if (far_plane + 50 < 4000) {
			// Set the new 'far' clip plane
			pcamera_mgr->Get_Camera ()->Set_Clip_Planes (near_plane, far_plane + 50);
		}

		// Make sure the scene is painted
		::Refresh_Main_View ();
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnSavePresets
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnSavePresets (void)
{
	//
	//	Reload the presets into the form
	//
	/*PresetsFormClass *presets_form = ::Get_Presets_Form ();
	if (presets_form != NULL) {
		presets_form->Save_Presets ();
	}*/

	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnUpdateSavePresets
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnUpdateSavePresets (CCmdUI *pCmdUI)
{
	pCmdUI->Enable (false);
	return ;	
}


//////////////////////////////////////////////////////////////////////////
//
//  OnCommand
//
BOOL
CMainFrame::OnCommand
(
	WPARAM wParam,
	LPARAM lParam
)
{
	if ((LOWORD (wParam) == ID_FILE_SAVE_AS) ||
	    (LOWORD (wParam) == ID_FILE_SAVE) ||
	    (LOWORD (wParam) == ID_FILE_OPEN)) {
		
		// Set the current working directory to point to the last directory where
		// we saved a file...
		CString full_path = theApp.GetProfileString (CONFIG_KEY, LAST_SAVE_DIR_VALUE);
		if (full_path.GetLength () != 0) {
			::SetCurrentDirectory (full_path);
			_pThe3DAssetManager->Set_Current_Directory (full_path);
			

			if (LOWORD (wParam) != ID_FILE_OPEN) {
				//
				//	Compose a default save name
				//
				Delimit_Path (full_path);
				full_path += ::Get_Current_Document ()->GetTitle ();
				CString temp_path = full_path;
				temp_path.MakeLower ();
				if (temp_path.Find (".lvl") == -1) {
					full_path += ".lvl";
				}
				::Get_Current_Document ()->Set_Current_Path (full_path);
			}
		}
	} else if (LOWORD (wParam) >= IDM_FIRST_CAM_PROFILE && LOWORD (wParam) <= IDM_LAST_CAM_PROFILE) {

		CMenu *main_menu = GetMenu ();
		CMenu *camera_menu = main_menu->GetSubMenu (CAMERA_MENU_INDEX);
		
		//
		//	Switch to the new camera profile
		//
		int index = LOWORD (wParam) - IDM_FIRST_CAM_PROFILE;
		CombatManager::Get_Camera()->Use_Profile (m_CameraProfiles[index]);

		//
		//	Set the check next to the first entry in the list
		//
		camera_menu->CheckMenuRadioItem (IDM_FIRST_CAM_PROFILE,
							IDM_FIRST_CAM_PROFILE + m_MaxCamProfiles, LOWORD (wParam), MF_BYCOMMAND);
	}
	
	// Allow the base class to process this message
	return CFrameWnd::OnCommand(wParam, lParam);
}


//////////////////////////////////////////////////////////////////////////
//
//  OnUpdateToggleSnappoints
//
void
CMainFrame::OnToggleSnappoints() 
{
	// Toggle the snap points setting
	CLevelEditDoc *pdoc = ::Get_Current_Document ();
	pdoc->Activate_Snap_Points (!pdoc->Are_Snap_Points_Active ());
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnUpdateToggleSnappoints
//
void
CMainFrame::OnUpdateToggleSnappoints (CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (::Get_Current_Document ()->Are_Snap_Points_Active ());
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnAlwaysExport
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnAlwaysExport (void)
{
	//
	// Ask the user to pick a path
	//
	CString path;
	if (::Browse_For_Folder (path,
									 NULL,
									 NULL,
									 "Select a directory where the core file set will be copied.")) {
		
		//
		// Copy the always files to the specified directory
		//
		CWaitCursor wait_cursor;		
		
		ExporterClass exporter;
		exporter.Export_Always_Files (path);

		//
		//	Cache this path in the registry
		//
		theApp.WriteProfileString (CONFIG_KEY, LAST_EXPORT_DIR_VALUE, path);
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnDestroy
//
void
CMainFrame::OnDestroy (void)
{
	// Don't allow any more repaints
	CLevelEditView::Allow_Repaint (false);

	CRect rect;	
	WINDOWPLACEMENT wnd_info = { sizeof (WINDOWPLACEMENT), 0 };
	::GetWindowPlacement (m_hWnd, &wnd_info);
	::GetWindowRect (m_hWnd, &rect);

	//
	// Cache this information in the registry
	//
	theApp.WriteProfileInt ("Window", "Left", rect.left);
	theApp.WriteProfileInt ("Window", "Right", rect.right);
	theApp.WriteProfileInt ("Window", "Top", rect.top);
	theApp.WriteProfileInt ("Window", "Bottom", rect.bottom);
	theApp.WriteProfileInt ("Window", "Maximized", int(wnd_info.showCmd == SW_SHOWMAXIMIZED));

	// We're closing the application so cleanup resources
	CLevelEditDoc *pCDoc = (CLevelEditDoc *)GetActiveDocument ();
	if (pCDoc) {
		
		// Ask the Doc to free its resources
		pCDoc->CleanupResources ();
	}        

	//
	// Shutdown the different systems
	//	
	CombatManager::Unload_Level ();
	CombatManager::Shutdown ();
	ScriptMgrClass::Shutdown ();
	PresetMgrClass::Free_Presets ();
	DazzleRenderObjClass::Deinit ();
	DefinitionMgrClass::Free_Definitions ();
	DialogMgrClass::Shutdown ();
	WWAudioClass::Get_Instance ()->Shutdown ();
	PathMgrClass::Shutdown ();

	//
	// Free any resources the WW3D engine allocated
	//
	WW3DAssetManager::Get_Instance()->Free_Assets ();
	//WW3D::Flush_Texture_Cache ();
	WW3D::Shutdown ();	
	SAFE_DELETE (_pThe3DAssetManager);

	::RemoveProp (m_AniToolbar, "ALLOW_UPDATE");

	CFrameWnd::OnDestroy ();
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnChangeBase
//
void
CMainFrame::OnChangeBase (void)
{
	::Get_Current_Document ()->Change_Base_Path ();
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  WindowProc
//
LRESULT
CMainFrame::WindowProc
(
	UINT message,
	WPARAM wParam,
	LPARAM lParam
)
{
	if (message == WM_USER+109) {
		
		// Show the welcome dialog
		WelcomeDialogClass welcome_dialog (this);
		welcome_dialog.DoModal ();
	}
	
	return CFrameWnd::WindowProc(message, wParam, lParam);
}


//////////////////////////////////////////////////////////////////////////
//
//  OnCamSpeedIncrease
//
void CMainFrame::OnCamSpeedIncrease (void)
{
	::Get_Camera_Mgr ()->Increase_Speed ();
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnCamSpeedDecrease
//
void CMainFrame::OnCamSpeedDecrease (void)
{
	::Get_Camera_Mgr ()->Decrease_Speed ();
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnCamReset
//
void CMainFrame::OnCamReset (void)
{
	::Get_Camera_Mgr ()->Level_Camera ();
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnKeyDown
//
void
CMainFrame::OnKeyDown
(
	UINT nChar,
	UINT nRepCnt,
	UINT nFlags
)
{


	// Allow the base class to process this message
	CFrameWnd::OnKeyDown(nChar, nRepCnt, nFlags);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnGenVis
//
void
CMainFrame::OnGenVis (void)
{

	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnToggleVisWindow
//
void
CMainFrame::OnToggleVisWindow (void) 
{
	HWND hchild = ::GetDlgItem (::GetDesktopWindow (), 777);
	if (::IsWindow (hchild)) {
		UINT vis = ::IsWindowVisible (hchild) ? SW_HIDE : SW_SHOW;
		::ShowWindow (hchild, vis);
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  Show_Ani_Toolbar
//
void
CMainFrame::Show_Ani_Toolbar (bool bshow) 
{
	ShowControlBar (&m_AniToolbar, (BOOL)bshow, false);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnAniBackFrame
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnAniBackFrame (void) 
{
	/*NodeInstanceClass *pnode = Get_Selection_Mgr ().Get_First ();
	if ((pnode != NULL) &&
	    (pnode->Get_Type () == NODE_TYPE_TRANSITION_CHARACTER)) {
		TransitionCharacterClass *pcharacter = (TransitionCharacterClass *)pnode;
		pcharacter->Send_Animation_Message (TransitionCharacterClass::ANIM_BACK_FRAME);
		
	}*/

	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnAniFirstFrame
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnAniFirstFrame (void)
{
	/*NodeInstanceClass *pnode = Get_Selection_Mgr ().Get_First ();
	if ((pnode != NULL) &&
	    (pnode->Get_Type () == NODE_TYPE_TRANSITION_CHARACTER)) {
		TransitionCharacterClass *pcharacter = (TransitionCharacterClass *)pnode;
		pcharacter->Send_Animation_Message (TransitionCharacterClass::ANIM_FIRST_FRAME);
		
	}*/

	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnAniFwdFrame
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnAniFwdFrame (void)
{
	/*NodeInstanceClass *pnode = Get_Selection_Mgr ().Get_First ();
	if ((pnode != NULL) &&
	    (pnode->Get_Type () == NODE_TYPE_TRANSITION_CHARACTER)) {
		TransitionCharacterClass *pcharacter = (TransitionCharacterClass *)pnode;
		pcharacter->Send_Animation_Message (TransitionCharacterClass::ANIM_ADVANCE_FRAME);
		
	}*/

	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnAniLastFrame
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnAniLastFrame (void)
{
	/*NodeInstanceClass *pnode = Get_Selection_Mgr ().Get_First ();
	if ((pnode != NULL) &&
	    (pnode->Get_Type () == NODE_TYPE_TRANSITION_CHARACTER)) {
		TransitionCharacterClass *pcharacter = (TransitionCharacterClass *)pnode;
		pcharacter->Send_Animation_Message (TransitionCharacterClass::ANIM_LAST_FRAME);
		
	}*/

	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnAniPause
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnAniPause (void)
{
	/*NodeInstanceClass *pnode = Get_Selection_Mgr ().Get_First ();
	if ((pnode != NULL) &&
	    (pnode->Get_Type () == NODE_TYPE_TRANSITION_CHARACTER)) {
		TransitionCharacterClass *pcharacter = (TransitionCharacterClass *)pnode;
		pcharacter->Send_Animation_Message (TransitionCharacterClass::ANIM_PAUSE);
		
	}*/

	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnAniPlay
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnAniPlay (void)
{
	/*NodeInstanceClass *pnode = Get_Selection_Mgr ().Get_First ();
	if ((pnode != NULL) &&
	    (pnode->Get_Type () == NODE_TYPE_TRANSITION_CHARACTER)) {
		TransitionCharacterClass *pcharacter = (TransitionCharacterClass *)pnode;
		pcharacter->Send_Animation_Message (TransitionCharacterClass::ANIM_PLAY);
		
	}*/

	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnAniStop
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnAniStop (void)
{
	/*NodeInstanceClass *pnode = Get_Selection_Mgr ().Get_First ();
	if ((pnode != NULL) &&
	    (pnode->Get_Type () == NODE_TYPE_TRANSITION_CHARACTER)) {
		TransitionCharacterClass *pcharacter = (TransitionCharacterClass *)pnode;
		pcharacter->Send_Animation_Message (TransitionCharacterClass::ANIM_STOP);
		
	}*/

	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  Update_Poly_Count
//
void
CMainFrame::Update_Poly_Count (void)
{
	static DWORD last_update = ::GetTickCount ();
	DWORD current_ticks = ::GetTickCount ();

	if ((current_ticks - last_update) > 500) {
		CString message;
		message.Format ("Polys %d", WW3D::Get_Last_Frame_Poly_Count());
		m_wndStatusBar.SetPaneText (4, message);
		last_update = current_ticks;
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  Update_Texture_Mem
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::Update_Texture_Mem (void)
{
	static DWORD last_update = ::GetTickCount ();
	DWORD current_ticks = ::GetTickCount ();

	if ((current_ticks - last_update) > 500) {

		/*srGERD::Statistics stats = { 0 };
		WW3D::Get_GERD ()->getStatistics (stats);

		CString message;
		message.Format ("Tex Upload: %dKB", int(float(stats.texDataTransfer / stats.frames) / 1024.0F));
		m_wndStatusBar.SetPaneText (3, message);
		last_update = current_ticks;

		WW3D::Get_GERD ()->resetStatistics ();
		*/
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  Update_Ani_Frame
//
void
CMainFrame::Update_Ani_Frame
(
	int frame,
	int max_frames
)
{
	CString message;
	message.Format ("Frame %d / %d", frame + 1, max_frames);

	m_wndStatusBar.SetPaneText (2, message);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  Update_Camera_Location
//
void
CMainFrame::Update_Camera_Location (const Vector3 &position)
{
	if (::IsWindow (m_wndStatusBar)) {
		CString message;
		message.Format ("Camera (%.2f,%.2f,%.2f)", position.X, position.Y, position.Z);
		m_wndStatusBar.SetPaneText (1, message);
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnUpdateEditUndo
//
void
CMainFrame::OnEditUndo (void)
{
	::Get_Scene_Editor ()->Undo ();
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnUpdateEditUndo
//
void
CMainFrame::OnUpdateEditUndo (CCmdUI *pCmdUI) 
{
	// Enable/disable the menu option
	pCmdUI->Enable (BOOL(::Get_Scene_Editor ()->Get_Undo_Mgr ().Get_Possible_Undo_Count () > 0));

	// Determine what text to display for the undo operation
	CString text;	
	if (::Get_Scene_Editor ()->Get_Undo_Mgr ().Get_Possible_Undo_Count () > 0) {
		text.Format ("Undo (%s)\tAlt+Backspace", ::Get_Scene_Editor ()->Get_Undo_Mgr ().Get_Next_Undo_Name ());
	} else {
		text.Format ("Undo\tAlt+Backspace");
	}

	// Change the text of the menu option
	pCmdUI->SetText (text);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnEditCopy
//
void
CMainFrame::OnEditCopy (void)
{
	::Get_Scene_Editor ()->Copy_Objects ();
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnUpdateEditCopy
//
void
CMainFrame::OnUpdateEditCopy (CCmdUI* pCmdUI) 
{
	pCmdUI->Enable (::Get_Selection_Mgr ().Get_Count () > 0);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnEditCut
//
void
CMainFrame::OnEditCut (void)
{
	::Get_Scene_Editor ()->Cut_Objects ();
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnUpdateEditCut
//
void
CMainFrame::OnUpdateEditCut (CCmdUI* pCmdUI) 
{
	pCmdUI->Enable (::Get_Scene_Editor ()->Get_Selection_Mgr ().Get_Count () > 0);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnEditPaste
//
void
CMainFrame::OnEditPaste (void)
{
	::Get_Scene_Editor ()->Paste_Objects ();
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnUpdateEditPaste
//
void
CMainFrame::OnUpdateEditPaste (CCmdUI* pCmdUI) 
{
	pCmdUI->Enable (::Get_Scene_Editor ()->Can_Paste ());
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnCreateGroup
//
void
CMainFrame::OnCreateGroup (void) 
{
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnUpdateCreateGroup
//
void
CMainFrame::OnUpdateCreateGroup (CCmdUI *pCmdUI) 
{
	pCmdUI->Enable (Get_Selection_Mgr ().Get_Count () > 0);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnToggleLights
//
void CMainFrame::OnToggleLights (void)
{
	// Toggle the lights on/off
	bool blights_on = ::Get_Scene_Editor ()->Are_Lights_On ();
	::Get_Scene_Editor ()->Turn_Lights_On (!blights_on);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnUpdateToggleLights
//
void
CMainFrame::OnUpdateToggleLights (CCmdUI *pCmdUI) 
{
	pCmdUI->SetCheck (::Get_Scene_Editor ()->Are_Lights_On ());
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnRepartition
//
void
CMainFrame::OnRepartition (void)
{
	CWaitCursor wait_cursor;

	//
	// Force a repartition for the scene...
	//
	::Get_Scene_Editor ()->Re_Partition_Static_Objects ();
	::Get_Scene_Editor ()->Re_Partition_Static_Lights ();	
	::Get_Scene_Editor ()->Re_Partition_Audio_System ();
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnUpdateToggleLights
//
void
CMainFrame::OnAllowAutoUpdates (void)
{
	// Toggle the state
	FileMgrClass::_bAutoUpdateOn = !FileMgrClass::_bAutoUpdateOn;
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnUpdateToggleLights
//
void
CMainFrame::OnUpdateAllowAutoUpdates (CCmdUI *pCmdUI) 
{
	pCmdUI->SetCheck (FileMgrClass::_bAutoUpdateOn);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnRotateLeft
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnRotateLeft (void)
{
	MoverClass::Rotate_Nodes_Z (-1.0F);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnRotateRight
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnRotateRight (void) 
{
	MoverClass::Rotate_Nodes_Z (1.0F);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnCalcAllLights
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnCalcAllLights (void) 
{
	//::Get_Scene_Editor ()->Update_Lighting ();
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnCalcSelLights
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnCalcSelLights (void) 
{
	/*PhysListClass list;	

	for (int index = 0; index < Get_Selection_Mgr ().Get_Count (); index ++) {
		NodeClass *node = Get_Selection_Mgr ().Get_At (index);
		if (node->Is_Static ()) {
			list.Add (node->Peek_Physics_Obj ());
		}
	}

	::Get_Scene_Editor ()->Update_Lighting (&list);*/
	return; 
}


//////////////////////////////////////////////////////////////////////////
//
//  OnUpdateCalcSelLights
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnUpdateCalcSelLights (CCmdUI *pCmdUI)
{
	pCmdUI->Enable (Get_Selection_Mgr ().Get_Count () > 0);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnLockObjects
//
void
CMainFrame::OnLockObjects (void)
{
	::Get_Scene_Editor ()->Lock_Nodes (true);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnUnlockObjects
//
void
CMainFrame::OnUnlockObjects (void)
{
	::Get_Scene_Editor ()->Lock_Nodes (false);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnUpdateLockObjects
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnUpdateLockObjects (CCmdUI *pCmdUI)
{
	pCmdUI->Enable (Get_Selection_Mgr ().Get_Count () > 0);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnUpdateUnlockObjects
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnUpdateUnlockObjects (CCmdUI *pCmdUI)
{
	pCmdUI->Enable (Get_Selection_Mgr ().Get_Count () > 0);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnEditIncludes
//
void
CMainFrame::OnEditIncludes (void)
{
	IncludeFilesDialogClass dialog (this);
	dialog.DoModal ();
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnVolDec
//
void
CMainFrame::OnVolDec (void)
{
	float vol = WWAudioClass::Get_Instance ()->Get_Sound_Effects_Volume ();
	vol -= 0.05F;
	WWAudioClass::Get_Instance ()->Set_Sound_Effects_Volume (vol);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnVolInc
//
void
CMainFrame::OnVolInc (void)
{
	float vol = WWAudioClass::Get_Instance ()->Get_Sound_Effects_Volume ();
	vol += 0.05F;
	WWAudioClass::Get_Instance ()->Set_Sound_Effects_Volume (vol);	
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnBackgroundSettings
//
void
CMainFrame::OnBackgroundSettings (void)
{
	const char *title = "Background Settings";
	
	CPropertySheet			 backgroundsettingspropsheet (title);
	MusicPropPageClass	*musicproppageptr;
	SkyPropPageClass		*skyproppageptr;
	WeatherPropPageClass *weatherproppageptr;

	musicproppageptr	 = new MusicPropPageClass;
	skyproppageptr		 = new SkyPropPageClass;	
	weatherproppageptr = new WeatherPropPageClass;

	backgroundsettingspropsheet.AddPage (musicproppageptr);
	backgroundsettingspropsheet.AddPage (skyproppageptr);
	backgroundsettingspropsheet.AddPage (weatherproppageptr);

	backgroundsettingspropsheet.DoModal();

	// Clean-up.
	delete musicproppageptr;
	delete skyproppageptr;
	delete weatherproppageptr;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnToggleMusic
//
void
CMainFrame::OnToggleMusic (void)
{
	bool is_on = WWAudioClass::Get_Instance ()->Is_Music_On ();
	WWAudioClass::Get_Instance ()->Allow_Music (!is_on);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnUpdateToggleMusic
//
void
CMainFrame::OnUpdateToggleMusic (CCmdUI *pCmdUI) 
{
	pCmdUI->SetCheck (WWAudioClass::Get_Instance ()->Is_Music_On ());
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnToggleSounds
//
void
CMainFrame::OnToggleSounds (void) 
{
	bool is_on = WWAudioClass::Get_Instance ()->Are_Sound_Effects_On ();
	WWAudioClass::Get_Instance ()->Allow_Sound_Effects (!is_on);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnUpdateToggleSounds
//
void
CMainFrame::OnUpdateToggleSounds (CCmdUI *pCmdUI) 
{
	pCmdUI->SetCheck (WWAudioClass::Get_Instance ()->Are_Sound_Effects_On ());
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnViewAggregateChildren
//
void CMainFrame::OnViewAggregateChildren (void)
{
	CWaitCursor wait_cursor;

	// Toggle the view state of aggregate children
	bool view = ::Get_Scene_Editor ()->Are_Aggregate_Children_Visible ();
	::Get_Scene_Editor ()->View_Aggregate_Children (!view);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnUpdateViewAggregateChildren
//
void
CMainFrame::OnUpdateViewAggregateChildren (CCmdUI *pCmdUI) 
{	
	pCmdUI->SetCheck (::Get_Scene_Editor ()->Are_Aggregate_Children_Visible ());
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnToggleSunlight
//
void
CMainFrame::OnToggleSunlight (void)
{
	bool enabled = ::Get_Scene_Editor ()->Is_Sun_Light_Enabled ();
	::Get_Scene_Editor ()->Enable_Sun_Light (!enabled);
	::Get_Scene_Editor ()->Update_Lighting ();
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnUpdateToggleSunlight
//
void
CMainFrame::OnUpdateToggleSunlight (CCmdUI *pCmdUI) 
{
	pCmdUI->SetCheck (::Get_Scene_Editor ()->Is_Sun_Light_Enabled ());
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnEditSunlight
//
void
CMainFrame::OnEditSunlight (void) 
{
	SunlightDialogClass dialog (this);
	dialog.DoModal ();
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnAutoGenVis
//
void
CMainFrame::OnAutoGenVis (void)
{
	GenerateVisDialogClass dialog (this);
	dialog.DoModal ();
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnVisualOptions
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnVisualOptions (void) 
{
	VisualOptionsDialogClass dialog (this);
	dialog.DoModal ();
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnVisInvert
//
//////////////////////////////////////////////////////////////////////////
void CMainFrame::OnVisInvert() 
{
	::Get_Scene_Editor()->Invert_Vis(!::Get_Scene_Editor()->Is_Vis_Inverted());
	::Refresh_Main_View ();
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnViewVisPoints
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnViewVisPoints (void) 
{
	bool enabled = ::Get_Scene_Editor ()->Are_Vis_Points_Displayed ();
	::Get_Scene_Editor ()->Display_Vis_Points (!enabled);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnUpdateViewVisPoints
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnUpdateViewVisPoints (CCmdUI *pCmdUI) 
{
	pCmdUI->SetCheck (::Get_Scene_Editor ()->Are_Vis_Points_Displayed ());
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnSpecifyAssetDatabase
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnSpecifyAssetDatabase (void) 
{
	CFileDialog dialog (true,
							  NULL,//"srcsafe.ini",
							  "srcsafe.ini",
							  OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_SHAREAWARE,
							  "Sourcesafe Databases|srcsafe.ini||",
							  this);

	//
	// Set the default path so the dialog opens to the correct location
	//
	CString default_path = theApp.GetProfileString (CONFIG_KEY, VSSDB_VALUE, DEF_VSS_DB);
	default_path = ::Strip_Filename_From_Path (default_path);
	dialog.m_ofn.lpstrInitialDir = default_path;

	//
	// Prompt the user for a new asset database location
	//
	if (dialog.DoModal () == IDOK) {
		CString new_path = dialog.GetPathName ();
		theApp.WriteProfileString (CONFIG_KEY, VSSDB_VALUE, (LPCTSTR)new_path);

		//
		//	Let the user know they must exit and restart
		//
		MessageBox ("You must exit and restart the level editor for your changes to take effect.", "Database Change", MB_OK | MB_ICONINFORMATION);
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnUpdateVisInvert
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnUpdateVisInvert (CCmdUI *pCmdUI) 
{
	pCmdUI->SetCheck (::Get_Scene_Editor()->Is_Vis_Inverted());
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnToggleBackfaces
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnToggleBackfaces (void)
{
	SceneEditorClass *scene = ::Get_Scene_Editor ();
	if (scene != NULL) {
		
		//
		//	Toggle the backface debug setting
		//
		bool enable = scene->Is_Backface_Occluder_Debug_Enabled ();
		scene->Enable_Backface_Occluder_Debug (!enable);
		::Refresh_Main_View ();
	}
	
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnUpdateToggleBackfaces
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnUpdateToggleBackfaces (CCmdUI *pCmdUI) 
{
	SceneEditorClass *scene = ::Get_Scene_Editor ();
	if (scene != NULL) {
		
		//
		//	Update the menu's checkmark
		//
		pCmdUI->SetCheck (scene->Is_Backface_Occluder_Debug_Enabled ());
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnToggleWireframeMode
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnToggleWireframeMode (void) 
{
	bool enable = (::Get_Scene_Editor()->Get_Polygon_Mode () != SceneClass::LINE);
	::Get_Scene_Editor()->Set_Polygon_Mode (enable ? SceneClass::LINE : SceneClass::FILL);
	::Refresh_Main_View ();
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnUpdateToggleWireframeMode
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnUpdateToggleWireframeMode (CCmdUI *pCmdUI) 
{
	pCmdUI->SetCheck (::Get_Scene_Editor()->Get_Polygon_Mode () == SceneClass::LINE);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnUpdateToggleWireframeMode
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnViewVisErrors (void)
{
	VisErrorReportDialogClass dialog (this);
	dialog.DoModal ();
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnUseVisCamera
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnUseVisCamera (void)
{
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnUpdateUseVisCamera
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnUpdateUseVisCamera (CCmdUI * /* pCmdUI */) 
{
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnVisDisable
//
//////////////////////////////////////////////////////////////////////////
void CMainFrame::OnVisDisable() 
{
	SceneEditorClass *scene = ::Get_Scene_Editor ();
	bool enabled = scene->Is_Vis_Enabled();
	scene->Enable_Vis(!enabled);
	::Refresh_Main_View ();
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnGenerateObstacleVolumes
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnGenerateObstacleVolumes (void)
{
	::Get_Scene_Editor ()->Generate_Pathfind_Portals ();
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnUpdateVisDisable
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnUpdateVisDisable (CCmdUI *pCmdUI) 
{
	SceneEditorClass *scene = ::Get_Scene_Editor ();
	pCmdUI->SetCheck (!scene->Is_Vis_Enabled());	
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnDisplayPaths
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnDisplayPaths (void)
{
	bool enabled = PathDebugPlotterClass::Get_Instance ()->Is_Displayed ();
	PathDebugPlotterClass::Get_Instance ()->Display (!enabled);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnUpdateDisplayPaths
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnUpdateDisplayPaths (CCmdUI *pCmdUI) 
{
	pCmdUI->SetCheck (PathDebugPlotterClass::Get_Instance ()->Is_Displayed ());
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnTestPathfind
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnTestPathfind (void) 
{
	CWaitCursor wait_cursor;

	PathDebugPlotterClass::Get_Instance ()->Reset ();

	NodeClass *node1 = NodeMgrClass::Find_Node ("PathStart");
	NodeClass *node2 = NodeMgrClass::Find_Node ("PathEnd");

	if ((node1 == NULL) || (node2 == NULL)) {
		node1 = NodeMgrClass::Find_Node ("GotoStart");
		node2 = NodeMgrClass::Find_Node ("GotoEnd");
	}

	if ((node1 != NULL) && (node2 != NULL)) {

		//
		//	Register all the waypaths with the pathfinding system
		//
		NodeClass *node;
		for (	node = NodeMgrClass::Get_First (NODE_TYPE_WAYPATH);
				node != NULL;
				node = NodeMgrClass::Get_Next (node, NODE_TYPE_WAYPATH))
		{
			node->Pre_Export ();
		}

		//
		//	Force the pathfind system to use any available waypath data
		// in its evaluation
		//
		PathfindClass::Get_Instance ()->Generate_Waypath_Sectors_And_Portals ();
		
		//
		//	Find the path
		//
		Vector3 start_pos	= node1->Get_Transform ().Get_Translation ();
		Vector3 end_pos	= node2->Get_Transform ().Get_Translation ();

		PathObjectClass path_obj;
		path_obj.Initialize (*(node1->Peek_Physics_Obj ()));
		path_obj.Set_Max_Speed (10);				

		//
		//	Determine the turn radius for the starting object...
		//
		float turn_radius = 0;
		if (node1->Get_Type () == NODE_TYPE_OBJECT) {
			PhysicalGameObj *game_obj = ((ObjectNodeClass *)node1)->Peek_Game_Obj ();
			if (game_obj != NULL) {
				VehicleGameObj *vehicle_obj = game_obj->As_VehicleGameObj ();
				if (vehicle_obj != NULL) {
					turn_radius = vehicle_obj->Get_Turn_Radius ();
				}

				path_obj.Set_Flag (PathObjectClass::IS_VEHICLE, bool(vehicle_obj != NULL));
			}
		}
		path_obj.Set_Turn_Radius (turn_radius);		

		PathSolveClass *path = PathMgrClass::Request_Path_Object ();
		path->Set_Path_Object (path_obj);
		path->Reset (start_pos, end_pos, 3);
		Vector3 curr_pos = start_pos;
		Vector3 new_pos (0, 0, 0);

		path->Process_Initial_Sector ();
		
		//
		//	Solve the path
		//
		while (path->Timestep () == PathSolveClass::THINKING) ;

		path->Unlink_Pathfind_Hooks ();

		//
		//	Did we find a path?
		//	
		if (path->Get_State () == PathSolveClass::SOLVED_PATH) {
			
			//
			//	Display the path
			//
			PathClass *solved_path = new PathClass;
			solved_path->Set_Path_Object (path_obj);
			solved_path->Initialize (*path);
			solved_path->Display_Path (true);
			REF_PTR_RELEASE (solved_path);
		}		
		
		PathMgrClass::Return_Path_Object (path);

		//
		//	Unregister all the waypaths with the pathfinding system
		//
		for (	node = NodeMgrClass::Get_First (NODE_TYPE_WAYPATH);
				node != NULL;
				node = NodeMgrClass::Get_Next (node, NODE_TYPE_WAYPATH))
		{
			node->Post_Export ();
		}
	}
	
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnDisplayPathfindSectors
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnDisplayPathfindSectors (void) 
{
	bool displayed = PathfindClass::Get_Instance ()->Are_Sectors_Displayed ();
	PathfindClass::Get_Instance ()->Display_Sectors (!displayed);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnUpdateDisplayPathfindSectors
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnUpdateDisplayPathfindSectors (CCmdUI *pCmdUI) 
{
	bool displayed = PathfindClass::Get_Instance ()->Are_Sectors_Displayed ();
	pCmdUI->SetCheck (displayed);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnDisplayPathfindPortals
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnDisplayPathfindPortals (void)
{
	bool displayed = PathfindClass::Get_Instance ()->Are_Portals_Displayed ();
	PathfindClass::Get_Instance ()->Display_Portals (!displayed);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnDisplayPathfindPortals
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnUpdateDisplayPathfindPortals (CCmdUI *pCmdUI)
{
	bool displayed = PathfindClass::Get_Instance ()->Are_Portals_Displayed ();
	pCmdUI->SetCheck (displayed);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnDisplayPathfindRawData
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnDisplayPathfindRawData (void) 
{
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnUpdateDisplayPathfindRawData
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnUpdateDisplayPathfindRawData (CCmdUI * /* pCmdUI */)
{
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnExportPathfind
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnExportPathfind (void) 
{
	// Use the level's filename to generate a default filename
	CString default_name = ::Get_Current_Document ()->GetPathName ();
	if (default_name.GetLength () > 0) {
		default_name = ::Get_Filename_From_Path (default_name);
		default_name = ::Asset_Name_From_Filename (default_name);
		default_name += ".pth";
	}

	CFileDialog dialog (	false,
								".pth",
								default_name,
								OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER,
								"Pathfind database (*.pth)|*.pth||",
								this);

	//
	// Ask the user what commando level file they wish to save to
	//
	if (dialog.DoModal () == IDOK) {
		
		//
		//	Export the pathfind data
		//
		CString filename = dialog.GetPathName ();
		PathfindImportExportSaveLoadClass::Export_Pathfind (filename);
	}
	
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnImportPathfind
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnImportPathfind (void)
{
	//
	// Use the level's filename to generate a default filename
	//
	CString default_name = ::Get_Current_Document ()->GetPathName ();
	if (default_name.GetLength () > 0) {
		default_name = ::Get_Filename_From_Path (default_name);
		default_name = ::Asset_Name_From_Filename (default_name);
		default_name += ".pth";
	}

	CFileDialog dialog (	true,
								".pth",
								default_name,
								OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER,
								"Pathfind database (*.pth)|*.pth||",
								this);

	//
	// Ask the user what file they wish to load from
	//
	if (dialog.DoModal () == IDOK) {
		
		//
		//	Import the pathfind data
		//
		CString filename = dialog.GetPathName ();
		PathfindImportExportSaveLoadClass::Import_Pathfind (filename);
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnImportVis
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnImportVis (void)
{
	// Use the level's filename to generate a default filename
	CString default_name = ::Get_Current_Document ()->GetPathName ();
	if (default_name.GetLength () > 0) {
		default_name = ::Get_Filename_From_Path (default_name);
		default_name = ::Asset_Name_From_Filename (default_name);
		default_name += ".vis";
	}

	CFileDialog dialog (	true,
								".vis",
								default_name,
								OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER | OFN_ALLOWMULTISELECT,
								"Vis Data Files (*.vis)|*.vis||",
								this);

    TCHAR szFileNameList[MAX_PATH*10] = { 0 };
    dialog.m_ofn.lpstrFile = szFileNameList;
    dialog.m_ofn.nMaxFile = sizeof (szFileNameList);

	// Ask the user what commando level file they wish to save to
	if (dialog.DoModal () == IDOK) {
      
		// Loop through all the selected files
      POSITION pos = dialog.GetStartPosition (); 
      while (pos != NULL) {
          
			// Is the filename OK?
			CString filename = dialog.GetNextPathName (pos);
			HANDLE hfile = ::CreateFile (filename,
												  GENERIC_READ,
												  FILE_SHARE_READ,
												  NULL,
												  OPEN_EXISTING,
												  0L,
												  NULL);

			ASSERT (hfile != INVALID_HANDLE_VALUE);
			if (hfile != INVALID_HANDLE_VALUE) {

				//
				//	Create a chunk-loader for use with the vis data file
				//
				RawFileClass file_obj;		
				file_obj.Attach (hfile);
				ChunkLoadClass chunk_load (&file_obj);

				//
				//	Load the vis data
				//
				::Get_Scene_Editor ()->Import_Vis_Data (chunk_load);
			}
      }
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnExportVis
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnExportVis (void)
{
	// Use the level's filename to generate a default filename
	CString default_name = ::Get_Current_Document ()->GetPathName ();
	if (default_name.GetLength () > 0) {
		default_name = ::Get_Filename_From_Path (default_name);
		default_name = ::Asset_Name_From_Filename (default_name);
		default_name += ".vis";
	}

	CFileDialog dialog (	false,
								".vis",
								default_name,
								OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER,
								"Vis Data File (*.vis)|*.vis||",
								this);

	// Ask the user what file they want to save to
	if (dialog.DoModal () == IDOK) {
		CString filename = dialog.GetPathName ();

		// Is the filename OK?
		HANDLE hfile = ::CreateFile (filename,
											  GENERIC_WRITE,
											  0,
											  NULL,
											  CREATE_ALWAYS,
											  0L,
											  NULL);

		ASSERT (hfile != INVALID_HANDLE_VALUE);
		if (hfile != INVALID_HANDLE_VALUE) {

			//
			//	Create a chunk-saver
			//
			RawFileClass file_obj;		
			file_obj.Attach (hfile);
			ChunkSaveClass chunk_save (&file_obj);

			//
			//	Save the vis data to file
			//
			::Get_Scene_Editor ()->Export_Vis_Data (chunk_save);
		}
	}
	
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnViewVisWindow
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnViewVisWindow (void)
{
	SceneEditorClass *scene = ::Get_Scene_Editor ();
	if (scene != NULL) {
		bool show = !scene->Is_Vis_Window_Visible ();
		scene->Show_Vis_Window (show);
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnViewVisWindow
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnUpdateViewVisWindow (CCmdUI *pCmdUI) 
{
	SceneEditorClass *scene = ::Get_Scene_Editor ();
	if (scene != NULL) {
		pCmdUI->SetCheck (scene->Is_Vis_Window_Visible ());
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnToggleManVisPoints
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnToggleManVisPoints (void)
{
	bool points_visible = ::Get_Scene_Editor ()->Are_Manual_Vis_Points_Visible ();
	::Get_Scene_Editor ()->Show_Manual_Vis_Points (!points_visible);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnUpdateToggleManVisPoints
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnUpdateToggleManVisPoints (CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck (::Get_Scene_Editor ()->Are_Manual_Vis_Points_Visible ());
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnDiscardPathfind
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnDiscardPathfind (void)
{
	PathfindClass::Get_Instance ()->Reset_Sectors ();
	PathfindClass::Get_Instance ()->Reset_Portals ();
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnDoDebug1
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnDoDebug1 (void)
{
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnDoDebug2
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnDoDebug2 (void)
{
	/*const char *filename = "c:\\projects\\renegade\\code\\commando\\chat.rc";
	const char *output_filename = "c:\\projects\\renegade\\code\\commando\\chat2.rc";

	if (StringsMgrClass::Check_Out ()) {

		//
		//	Try to open the .RC file
		//
		TextFileClass file (filename);
		TextFileClass output_file (output_filename);
		if (file.Open (RawFileClass::READ) && output_file.Open (RawFileClass::WRITE)) {
			
			int current_index = 1;

			//
			//	Read each line from the file
			//
			StringClass curr_line;
			while (file.Read_Line (curr_line)) {
				
				Process_Line (curr_line, current_index);

				curr_line += "\r\n";
				output_file.Write (curr_line, ::lstrlen (curr_line));
				//output_file.Write_Line (curr_line);
			}

			//
			//	Close the files
			//
			file.Close ();
			output_file.Close ();
		}

		StringsMgrClass::Save_Translation_Database ();
		StringsMgrClass::Check_In ();
	}*/


	//
	//	Loop over all the definitions
	//
	/*for (	PresetClass *preset = PresetMgrClass::Get_First ();
			preset != NULL;
			preset = PresetMgrClass::Get_Next (preset))
	{
		if (preset->Get_IsTemporary () == false) {						

			DEFINITION_LIST definition_list;
			::Build_Embedded_Definition_List (definition_list, preset->Get_Definition ());

			for (int index = 0; index < definition_list.Count (); index ++) {
				DefinitionClass *definition = definition_list[index];

				//
				//	Loop over all the parameters contained in the definitions
				//
				int count = definition->Get_Parameter_Count ();
				for (int index = 0; index < count; index ++) {
					ParameterClass *param = definition->Lock_Parameter (index);
		
					if (param->Get_Type () == ParameterClass::TYPE_ZONE) {
						ZoneParameterClass *zone_param = (ZoneParameterClass *)param;
						const OBBoxClass &old_zone = zone_param->Get_Zone ();
						OBBoxClass new_zone = old_zone;
						new_zone.Extent = (old_zone.Extent * 0.5F);
						zone_param->Set_Zone (new_zone);
					}

					definition->Unlock_Parameter (index);
				}
			}
		}
	}	

	//
	//	Get the list of all static objects from the physics scene
	//
	for (int index = 0; index < door_def_list.Count (); index ++) {

			
		DoorPhysDefClass *definition = door_def_list[index];

		const OBBoxClass &box1 = definition->Get_Trigger_Zone1 ();
		const OBBoxClass &box2 = definition->Get_Trigger_Zone2 ();
		
		definition->Set_Trigger_Zone2 (box1);
	}

	PresetsFormClass *form = ::Get_Presets_Form ();
	form->Save_Preset_Libraries ();*/

	

	/*PresetsFormClass *presets_form = ::Get_Presets_Form ();

	CString message = "Presets with invalid IDs:\n\n";

	//
	//	Loop over all the definitions
	//
	for (	PresetClass *preset = PresetMgrClass::Get_First ();
			preset != NULL;
			preset = PresetMgrClass::Get_Next (preset))
	{
		if (preset->Get_IsTemporary () == false) {
					
			DEFINITION_LIST definition_list;
			::Build_Embedded_Definition_List (definition_list, preset->Get_Definition ());

			for (int index = 0; index < definition_list.Count (); index ++) {
				DefinitionClass *definition = definition_list[index];
				
				if (definition != NULL && definition->Get_ID () >= TEMP_DEF_ID_START) {
					
					message += "Preset Name: ";
					message += preset->Get_Name ();
					message += "\tDefinition Name: ";
					message += definition->Get_Name ();
					message += "\n";
				}
			}
		}
	}

	MessageBox (message, "Validate Preset IDs", MB_ICONINFORMATION | MB_OK);*/
	return ;	
}


//////////////////////////////////////////////////////////////////////////
//
//  OnImportDynObjs
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnImportDynObjs (void)
{
	// Use the level's filename to generate a default filename
	CString default_name = ::Get_Current_Document ()->GetPathName ();
	if (default_name.GetLength () > 0) {
		default_name = ::Get_Filename_From_Path (default_name);
		default_name = ::Asset_Name_From_Filename (default_name);
		default_name += ".dyn";
	}

	CFileDialog dialog (	true,
								".dyn",
								default_name,
								OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER,
								"Dynamic Objects (*.dyn)|*.dyn||",
								this);

	if (dialog.DoModal () == IDOK) {		
		EditorSaveLoadClass::Import_Dynamic_Objects (dialog.GetPathName ());
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnExportDynObjs
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnExportDynObjs (void)
{

	// Use the level's filename to generate a default filename
	CString default_name = ::Get_Current_Document ()->GetPathName ();
	if (default_name.GetLength () > 0) {
		default_name = ::Get_Filename_From_Path (default_name);
		default_name = ::Asset_Name_From_Filename (default_name);
		default_name += ".dyn";
	}

	CFileDialog dialog (	false,
								".dyn",
								default_name,
								OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER,
								"Dynamic Objects (*.dyn)|*.dyn||",
								this);


	if (dialog.DoModal () == IDOK) {		
		EditorSaveLoadClass::Export_Dynamic_Objects (dialog.GetPathName ());
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnTestGoto
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnTestGoto (void)
{
	CWaitCursor wait_cursor;

	PathDebugPlotterClass::Get_Instance ()->Reset ();

	NodeClass *node1 = NodeMgrClass::Find_Node ("GotoStart");
	NodeClass *node2 = NodeMgrClass::Find_Node ("GotoEnd");
	if ((node1 != NULL) && (node2 != NULL)) {
		
		::Get_Scene_Editor ()->DoObjectGoto (node1, node2);
	}
	
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnImportStatic
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnImportStatic (void)
{

	// Use the level's filename to generate a default filename
	CString default_name = ::Get_Current_Document ()->GetPathName ();
	if (default_name.GetLength () > 0) {
		default_name = ::Get_Filename_From_Path (default_name);
		default_name = ::Asset_Name_From_Filename (default_name);
		default_name += ".sta";
	}

	CFileDialog dialog (	true,
								".sta",
								default_name,
								OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER,
								"Static Data (*.sta)|*.sta||",
								this);

	if (dialog.DoModal () == IDOK) {
		CString filename = dialog.GetPathName ();

		// Is the filename OK?
		HANDLE hfile = ::CreateFile (filename,
											  GENERIC_READ,
											  0,
											  NULL,
											  OPEN_EXISTING,
											  0L,
											  NULL);

		ASSERT (hfile != INVALID_HANDLE_VALUE);
		if (hfile != INVALID_HANDLE_VALUE) {

			//
			//	Create a chunk-loader for use with the pathfind database
			//
			RawFileClass file_obj;		
			file_obj.Attach (hfile);
			ChunkLoadClass cload (&file_obj);
		
			//
			//	Remove all static nodes from the level
			//
			NODE_LIST obj_list;
			NodeMgrClass::Remove_Static_Objects (obj_list);
			for (int index = 0; index < obj_list.Count (); index ++) {
				NodeClass *node = obj_list[index];
				MEMBER_RELEASE (node);
			}

			//
			//	Load the scene data, then import the node's list...
			//
			EditorSaveLoadClass::Set_Loaded_Vis_Valid (true);
			SaveLoadSystemClass::Load (cload);

			//
			//	Repartition the static culling systems
			//
//			::Get_Scene_Editor ()->Re_Partition_Static_Objects ();
			::Get_Scene_Editor ()->Re_Partition_Static_Lights ();
			::Get_Scene_Editor ()->Re_Partition_Audio_System ();
			
			if (EditorSaveLoadClass::Is_Loaded_Vis_Valid ()) {
				::Get_Scene_Editor ()->Validate_Vis ();
			}

			//
			//	Reset the IDs of all the static nodes
			//
			for (	NodeClass *node = NodeMgrClass::Get_First ();
					node != NULL;
					node = NodeMgrClass::Get_Next (node))
			{
				if (node->Is_Static ()) {
					node->Set_ID (NodeMgrClass::Get_Node_ID (node->Get_Type ()));
					NodeMgrClass::Setup_Node_Identity (*node);
				}
			}
		}
	}
	
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnExportStatic
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnExportStatic (void)
{
	// Use the level's filename to generate a default filename
	CString default_name = ::Get_Current_Document ()->GetPathName ();
	if (default_name.GetLength () > 0) {
		default_name = ::Get_Filename_From_Path (default_name);
		default_name = ::Asset_Name_From_Filename (default_name);
		default_name += ".sta";
	}

	CFileDialog dialog (	false,
								 ".sta",
								 default_name,
								 OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER,
								 "Static Data (*.sta)|*.sta||",
								 this);

	// Ask the user what commando level file they wish to save to
	if (dialog.DoModal () == IDOK) {
		CString filename = dialog.GetPathName ();

		// Is the filename OK?
		HANDLE hfile = ::CreateFile (filename,
											  GENERIC_WRITE,
											  0,
											  NULL,
											  CREATE_ALWAYS,
											  0L,
											  NULL);

		ASSERT (hfile != INVALID_HANDLE_VALUE);
		if (hfile != INVALID_HANDLE_VALUE) {

			//
			//	Create a chunk-saver for use with the pathfind database
			//
			RawFileClass file_obj;		
			file_obj.Attach (hfile);
			ChunkSaveClass csave (&file_obj);

			//
			//	Repartition the static culling systems
			// (gth) cannot re-partition the object culling systems because they
			// now contain hierarchical visibility data...
			//
			::Get_Scene_Editor ()->Re_Partition_Static_Lights ();	
			::Get_Scene_Editor ()->Re_Partition_Audio_System ();		

			//
			//	Export the culling system, vis, etc
			//
			SaveLoadSystemClass::Save (csave, _PhysStaticDataSaveSystem);

			//
			//	Export the static node list
			//
			NODE_LIST obj_list;
			NodeMgrClass::Remove_Dynamic_Objects (obj_list);
			SaveLoadSystemClass::Save (csave, _TheNodeMgr);
			NodeMgrClass::Put_Objects_Back (obj_list);
		}		
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnDisplayWeb
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnDisplayWeb (void) 
{
	bool enable = !::Get_Current_Document ()->Is_Path_Web_Displayed ();
	::Get_Current_Document ()->Display_Path_Web (enable);


	PathDebugPlotterClass::Get_Instance ()->Reset ();
	PathDebugPlotterClass::Get_Instance ()->Display (enable);

	if (enable) {
		PathfindSectorClass *sector = NULL;
		int sector_index = 0;
		while ((sector = PathfindClass::Get_Instance ()->Peek_Sector (sector_index++)) != NULL) {

			Vector3 center = sector->Get_Bounding_Box ().Center;

			int portal_index = sector->Get_Portal_Count ();
			while (portal_index --) {

				PathfindPortalClass *portal = sector->Peek_Portal (portal_index);
				AABoxClass portal_box;
				portal->Get_Bounding_Box (portal_box);
				
				PathDebugPlotterClass::Get_Instance ()->Add (center, portal_box.Center, Vector3 (1, 0, 0));
			}
		}
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnUpdateDisplayWeb
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnUpdateDisplayWeb (CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (::Get_Current_Document ()->Is_Path_Web_Displayed ());
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnUpdateDisplayFullPaths
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnUpdateDisplayFullPaths (CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (::Get_Current_Document ()->Are_Full_Paths_Displayed ());
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnDisplayFullPaths
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnDisplayFullPaths (void)
{
	bool enable = !::Get_Current_Document ()->Are_Full_Paths_Displayed ();
	::Get_Current_Document ()->Display_Full_Paths (enable);
	return ;	
}


//////////////////////////////////////////////////////////////////////////
//
//  OnEscape
//
//////////////////////////////////////////////////////////////////////////
void CMainFrame::OnEscape (void) 
{
	//
	//	Force the mode back to the default...
	//
	::Get_Mouse_Mgr ()->Set_Mouse_Mode (MouseMgrClass::MODE_OBJECT_MANIPULATE);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnDisplayVisSectors
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnDisplayVisSectors (void) 
{
	bool enabled = ::Get_Scene_Editor ()->Is_Vis_Sector_Display_Enabled ();
	::Get_Scene_Editor ()->Enable_Vis_Sector_Display (!enabled);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnUpdateDisplayVisSectors
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnUpdateDisplayVisSectors (CCmdUI *pCmdUI)
{
	bool enabled = ::Get_Scene_Editor ()->Is_Vis_Sector_Display_Enabled ();
	pCmdUI->SetCheck (enabled);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnPrelitMultipass
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnPrelitMultipass (void)
{
	if (WW3D::Get_Prelit_Mode () != WW3D::PRELIT_MODE_LIGHTMAP_MULTI_PASS) {
		WW3D::Set_Prelit_Mode (WW3D::PRELIT_MODE_LIGHTMAP_MULTI_PASS);
		::Get_Scene_Editor ()->Reload_Lightmap_Models ();
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnUpdatePrelitMultipass
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnUpdatePrelitMultipass (CCmdUI *pCmdUI) 
{
	bool enable = (WW3D::Get_Prelit_Mode () == WW3D::PRELIT_MODE_LIGHTMAP_MULTI_PASS);
	pCmdUI->SetRadio (enable);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnPrelitMultitex
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnPrelitMultitex (void) 
{
	if (WW3D::Get_Prelit_Mode () != WW3D::PRELIT_MODE_LIGHTMAP_MULTI_TEXTURE) {
		WW3D::Set_Prelit_Mode (WW3D::PRELIT_MODE_LIGHTMAP_MULTI_TEXTURE);
		::Get_Scene_Editor ()->Reload_Lightmap_Models ();
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnUpdatePrelitMultitex
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnUpdatePrelitMultitex (CCmdUI *pCmdUI) 
{
	bool enable = (WW3D::Get_Prelit_Mode () == WW3D::PRELIT_MODE_LIGHTMAP_MULTI_TEXTURE);
	pCmdUI->SetRadio (enable);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnPrelitVertex
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnPrelitVertex (void)
{
	if (WW3D::Get_Prelit_Mode () != WW3D::PRELIT_MODE_VERTEX) {
		WW3D::Set_Prelit_Mode (WW3D::PRELIT_MODE_VERTEX);
		::Get_Scene_Editor ()->Reload_Lightmap_Models ();
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnUpdatePrelitVertex
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnUpdatePrelitVertex (CCmdUI *pCmdUI) 
{
	bool enable = (WW3D::Get_Prelit_Mode () == WW3D::PRELIT_MODE_VERTEX);
	pCmdUI->SetRadio (enable);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnDebugScriptsMode
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnDebugScriptsMode (void)
{
	bool enabled = ::Get_Current_Document ()->Get_Debug_Script_Mode ();
	::Get_Current_Document ()->Set_Debug_Script_Mode (!enabled);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnUpdateDebugScriptsMode
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnUpdateDebugScriptsMode (CCmdUI *pCmdUI) 
{
	pCmdUI->SetCheck (::Get_Current_Document ()->Get_Debug_Script_Mode ());
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnImportLights
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnImportLights (void)
{
	CFileDialog dialog (	true,
								".wlt",
								NULL,
								OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_ALLOWMULTISELECT | OFN_EXPLORER,
								"Westwood Light Database (*.wlt)|*.wlt||",
								this);

	TCHAR filename_list[MAX_PATH*20] = { 0 };
	dialog.m_ofn.lpstrFile = filename_list;
	dialog.m_ofn.nMaxFile = sizeof (filename_list);

	//
	// Ask the user what files they wish to import
	//
	if (dialog.DoModal () == IDOK) {

      //
		// Loop through all the selected files
		//
		DynamicVectorClass<StringClass> filename_list;
		POSITION pos = dialog.GetStartPosition (); 
		while (pos != NULL) {

			//
			//	Add this filename to the list
			//
			filename_list.Add ((LPCTSTR)dialog.GetNextPathName (pos));
      }

		::Get_Scene_Editor ()->Import_Lights (filename_list);
	}
		
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnViewSoundSpheres
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnViewSoundSpheres (void)
{
	bool enabled = ::Get_Scene_Editor ()->Are_Sound_Spheres_Displayed ();
	::Get_Scene_Editor ()->Display_Sound_Spheres (!enabled);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnUpdateViewSoundSpheres
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnUpdateViewSoundSpheres (CCmdUI *pCmdUI) 
{
	bool enabled = ::Get_Scene_Editor ()->Are_Sound_Spheres_Displayed ();
	pCmdUI->SetCheck (enabled);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnViewLightSpheres
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnViewLightSpheres (void) 
{
	bool enabled = ::Get_Scene_Editor ()->Are_Light_Spheres_Displayed ();
	::Get_Scene_Editor ()->Display_Light_Spheres (!enabled);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnUpdateViewLightSpheres
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnUpdateViewLightSpheres (CCmdUI *pCmdUI) 
{
	bool enabled = ::Get_Scene_Editor ()->Are_Light_Spheres_Displayed ();
	pCmdUI->SetCheck (enabled);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnImportSunlight
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnImportSunlight (void)
{
	CFileDialog dialog (	true,
								".wlt",
								NULL,
								OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER,
								"Westwood Light Database (*.wlt)|*.wlt||",
								this);

	//
	// Ask the user what file they wish to import
	//
	if (dialog.DoModal () == IDOK) {

      //
		// Load the sunlight's settings from this file
		//
		::Get_Scene_Editor ()->Import_Sunlight (dialog.GetPathName ());
	}
		
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnDiscardVis
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnDiscardVis (void)
{
	::Get_Scene_Editor ()->Reset_Vis (true);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  Expand_Attenuation_Spheres
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::Expand_Attenuation_Spheres (float amount)
{
	SelectionMgrClass &sel_mgr = ::Get_Scene_Editor ()->Get_Selection_Mgr ();

	//
	//	Loop over the currently selected nodes
	//
	for (int index = 0; index < sel_mgr.Get_Count (); index ++) {
		NodeClass *node = sel_mgr.Get_At (index);
		if (node != NULL) {
			
			//
			//	Increment this node's radius and pass it back
			// to the node...
			//
			float radius	= node->Get_Attenuation_Radius ();
			radius			+= amount;
			if (radius > 0) {
				node->Set_Attenuation_Radius (radius);
			}
		}
	}
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnIncreaseSphere
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnIncreaseSphere (void)
{
	Adjust_Growth_Rate ();
	Expand_Attenuation_Spheres (m_GrowthRate);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnUpdateIncreaseSphere
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnUpdateIncreaseSphere (CCmdUI *pCmdUI) 
{	
	pCmdUI->Enable (::Get_Selection_Mgr ().Get_Count () > 0);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnDecreaseSphere
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnDecreaseSphere (void) 
{
	Adjust_Growth_Rate ();
	Expand_Attenuation_Spheres (-m_GrowthRate);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnDecreaseSphere
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnUpdateDecreaseSphere (CCmdUI *pCmdUI) 
{
	pCmdUI->Enable (::Get_Selection_Mgr ().Get_Count () > 0);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  Adjust_Growth_Rate
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::Adjust_Growth_Rate (void)
{
	DWORD curr_time = ::GetTickCount ();
	
	//
	//	Reset the growth rate if too much time has elapsed
	//
	if ((curr_time - m_UpdateTimer) > 800) {
		m_GrowthRate = 0.25F;
	}
	
	m_UpdateTimer = curr_time;

	//
	//	Clamp the growth rate to ensure things don't get out of hand
	//
	m_GrowthRate = WWMath::Clamp (m_GrowthRate + 0.005F, 0.25F, 5.0F);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnRunJob
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnRunJob (void) 
{
	CFileDialog dialog (	true,
								".job",
								"farm01.job",
								OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER,
								"Vis Farm Job (*.job)|*.job||",
								this);

	//
	// Ask the user which job they want to load
	//
	if (dialog.DoModal () == IDOK) {
		::Perform_Job (dialog.GetPathName (), false);
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnGotoLocation
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnGotoLocation (void)
{
	GotoLocationDialogClass dialog (this);
	dialog.DoModal ();
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnExportThumbnails
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnExportThumbnails (void)
{
	//
	// Determine which directory we should use as a default for the export
	//
	CString full_path = theApp.GetProfileString (CONFIG_KEY, LAST_EXPORT_DIR_VALUE);
	if (full_path.GetLength () == 0) {
		full_path = ::Get_File_Mgr ()->Make_Full_Path (LEVELS_ASSET_DIR);		
	}
	_pThe3DAssetManager->Set_Current_Directory (full_path);


	
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnVisStats
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnVisStats (void)
{
	VisStatsDialogClass dialog (this);
	dialog.DoModal ();
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnSetNodeIdStart
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnSetNodeIdStart (void)
{
	NodeIDStartDialogClass dialog (this);
	dialog.DoModal ();
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnBuildDynaCullsys
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnBuildDynaCullsys (void)
{
	CWaitCursor wait_cursor;

	//
	//	Create a pathfind floodfiller object
	//
	PathfindSectorBuilderClass builder;
	//builder.Allow_Water_Floodfill (true);
	builder.Initialize ();	

	//
	//	Find all the pathfind start-points in the level and add
	// them to the floodfiller
	//
	NodeClass *node = NULL;
	for (	node = NodeMgrClass::Get_First ();
			node != NULL;
			node = NodeMgrClass::Get_Next (node))
	{
		if (node->Get_Type () == NODE_TYPE_PATHFIND_START) {
			builder.Add_Start_Point (node->Get_Transform ().Get_Translation ());
		}
	}
	
	//
	//	Perform the floodfill from each of the start locations
	//
	builder.Generate_Sectors ();
	builder.Compress_Sectors_For_Dyna_Culling ();

	//
	//	Cleanup
	//
	builder.Shutdown ();
	::Get_Scene_Editor ()->Reset_Dynamic_Object_Visibility_Status ();
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnImportVisRemapData
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnImportVisRemapData (void)
{
	//
	// Use the level's filename to generate a default filename
	//
	CString default_name = ::Get_Current_Document ()->GetPathName ();
	if (default_name.GetLength () > 0) {
		default_name = ::Get_Filename_From_Path (default_name);
		default_name = ::Asset_Name_From_Filename (default_name);
		default_name += ".vrt";
	}

	CFileDialog dialog (	true,
								".vrt",
								default_name,
								OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER,
								"Vis remap table (*.vrt)|*.vrt||",
								this);

	//
	// Ask the user what file they wish to load from
	//
	if (dialog.DoModal () == IDOK) {
		CString filename = dialog.GetPathName ();

		//
		// Is the filename OK?
		//
		HANDLE hfile = ::CreateFile (filename,
											  GENERIC_READ,
											  0,
											  NULL,
											  OPEN_EXISTING,
											  0L,
											  NULL);

		ASSERT (hfile != INVALID_HANDLE_VALUE);
		if (hfile != INVALID_HANDLE_VALUE) {

			//
			//	Create a chunk-loader for use with the vis-importer
			//
			RawFileClass file_obj;		
			file_obj.Attach (hfile);
			ChunkLoadClass chunk_load (&file_obj);

			//
			//	Import the data
			//
			VisMgrClass::Import_Remap_Data (chunk_load);
		}
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnExportVisRemapData
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnExportVisRemapData (void)
{
	//
	// Use the level's filename to generate a default filename
	//
	CString default_name = ::Get_Current_Document ()->GetPathName ();
	if (default_name.GetLength () > 0) {
		default_name = ::Get_Filename_From_Path (default_name);
		default_name = ::Asset_Name_From_Filename (default_name);
		default_name += ".vrt";
	}

	CFileDialog dialog (	false,
								".vrt",
								default_name,
								OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER,
								"Vis remap table (*.vrt)|*.vrt||",
								this);

	//
	// Ask the user what commando level file they wish to save to
	//
	if (dialog.DoModal () == IDOK) {
		CString filename = dialog.GetPathName ();

		//
		// Is the filename OK?
		//
		HANDLE hfile = ::CreateFile (filename,
											  GENERIC_WRITE,
											  0,
											  NULL,
											  CREATE_ALWAYS,
											  0L,
											  NULL);

		ASSERT (hfile != INVALID_HANDLE_VALUE);
		if (hfile != INVALID_HANDLE_VALUE) {

			//
			//	Create a chunk-saver for use with the pathfind database
			//
			RawFileClass file_obj;		
			file_obj.Attach (hfile);
			ChunkSaveClass chunk_save (&file_obj);

			//
			//	Export the data
			//
			VisMgrClass::Export_Remap_Data (chunk_save);
		}
	}
	
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnUpdateBulkReplace
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnUpdateBulkReplace (CCmdUI *pCmdUI)
{
	pCmdUI->Enable (::Get_Selection_Mgr ().Get_Count () > 0);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnBulkReplace
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnBulkReplace (void)
{
	::Get_Scene_Editor ()->Replace_Selection ();
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnCheckIds
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnCheckIds (void)
{
	NODE_LIST node_list;
	NodeMgrClass::Build_ID_Collision_List (node_list);

	if (node_list.Count () > 0) {

		CString message = "ID collision detected in the following objects:\r\n\r\n";
		
		//
		//	Build an error log of all the offending nodes
		//
		for (int index = 0; index < node_list.Count (); index ++) {
			NodeClass *node = node_list[index];
			
			//
			//	Add information about this entry to the log
			//
			CString entry;
			entry.Format ("%s, ID = %d\r\n", node->Get_Name (), node->Get_ID ());			
			message += entry;
		}

		//
		//	Display the log to the user
		//

		GenericTextDialogClass dialog (this);
		dialog.Set_Title ("ID Validation");
		dialog.Set_Description ("ID Collisions listed below.");
		dialog.Set_Text (message);
		dialog.DoModal ();

	} else {

		//
		//	Let the user know there were no problems
		//
		MessageBox ("No ID collision detected in this level.", "ID Validation", MB_ICONINFORMATION | MB_OK);
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnExportLights
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnExportLights (void)
{
	//
	// Use the level's filename to generate a default WLT filename
	//
	CString default_name = ::Get_Current_Document ()->GetPathName ();
	if (default_name.GetLength () > 0) {
		default_name = ::Get_Filename_From_Path (default_name);
		default_name = ::Asset_Name_From_Filename (default_name);
		default_name += ".wlt";
	}

	CFileDialog dialog (	false,
								".wlt",
								NULL,
								OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER,
								"Westwood Light Database (*.wlt)|*.wlt||",
								this);

	//
	// Ask the user what file they wish to export to
	//
	if (dialog.DoModal () == IDOK) {		
		::Get_Scene_Editor ()->Export_Lights (dialog.GetPathName ());
	}
	
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnOptimizeVisData
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnOptimizeVisData (void)
{
	OptimizingVisDialogClass::Optimize ();
/*
	StringClass message;
	message.Format("Starting Bit Count: %d\nEnding Bit Count: %d\nDynamic Object Cells Removed: %d\nObjects Merged: %d\nSectors Merged: %d\nTotal Bits Removed: %d\nFinal Vis Sector Count: %d\nFinal Vis Object Count: %d\n",
							stats.InitialBitCount,
							stats.FinalBitCount,
							stats.DynCellsRemoved,
							stats.ObjectsMerged,
							stats.SectorsMerged,
							stats.TotalBitsRemoved,
							vis_sector_count,
							vis_object_count);

	MessageBox(message,"Results");
*/
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnDisplayVisGridLittleBoxes
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnDisplayVisGridLittleBoxes (void) 
{
	bool enabled = ::Get_Scene_Editor ()->Get_Vis_Grid_Display_Mode () == PhysicsSceneClass::VIS_GRID_DISPLAY_CENTERS;
	if (!enabled) {
		::Get_Scene_Editor ()->Set_Vis_Grid_Display_Mode (PhysicsSceneClass::VIS_GRID_DISPLAY_CENTERS);
	} else {
		::Get_Scene_Editor ()->Set_Vis_Grid_Display_Mode (PhysicsSceneClass::VIS_GRID_DISPLAY_NONE);
	}
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnUpdateDisplayVisGridLittleBoxes
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnUpdateDisplayVisGridLittleBoxes (CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck (::Get_Scene_Editor ()->Get_Vis_Grid_Display_Mode () == PhysicsSceneClass::VIS_GRID_DISPLAY_CENTERS);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnDisplayVisGrid
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnDisplayVisGrid (void)
{
	bool enabled = ::Get_Scene_Editor ()->Get_Vis_Grid_Display_Mode () == PhysicsSceneClass::VIS_GRID_DISPLAY_ACTUAL_BOXES;
	if (!enabled) {
		::Get_Scene_Editor ()->Set_Vis_Grid_Display_Mode (PhysicsSceneClass::VIS_GRID_DISPLAY_ACTUAL_BOXES);
	} else {
		::Get_Scene_Editor ()->Set_Vis_Grid_Display_Mode (PhysicsSceneClass::VIS_GRID_DISPLAY_NONE);
	}
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnUpdateDisplayVisGrid
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnUpdateDisplayVisGrid (CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck (::Get_Scene_Editor ()->Get_Vis_Grid_Display_Mode () == PhysicsSceneClass::VIS_GRID_DISPLAY_ACTUAL_BOXES);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnEditStringsTable
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnEditStringsTable (void)
{
	StringsMgrClass::Edit_Database (m_hWnd);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnExportStringIds
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnExportStringIds (void)
{
	CWaitCursor wait_cursor;
	StringsMgrClass::Export_IDs ();
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnImportStringIds
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnImportStringIds (void)
{
	CWaitCursor wait_cursor;
	StringsMgrClass::Import_IDs ();
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnBuildFlightInfo
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnBuildFlightInfo (void)
{
	CWaitCursor wait_cursor;

	HeightDBClass::Generate ();	
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnToggleAttenuationSpheres
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnToggleAttenuationSpheres (void)
{
	SelectionMgrClass &sel_mgr = ::Get_Scene_Editor ()->Get_Selection_Mgr ();

	//
	//	Loop over the currently selected nodes
	//
	for (int index = 0; index < sel_mgr.Get_Count (); index ++) {
		NodeClass *node = sel_mgr.Get_At (index);
		if (node != NULL) {
			
			//
			//	Toggle the sphere display...
			//
			bool show_sphere = node->Is_Attenuation_Sphere_Shown ();
			node->Show_Attenuation_Spheres (!show_sphere);
		}
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnUpdateToggleAttenuationSpheres
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnUpdateToggleAttenuationSpheres (CCmdUI *pCmdUI)
{
	pCmdUI->Enable (::Get_Selection_Mgr ().Get_Count () > 0);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnFixIdCollisions
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnFixIdCollisions (void)
{
	NODE_LIST node_list;
	NodeMgrClass::Build_ID_Collision_List (node_list);

	if (node_list.Count () > 0) {

		//
		//	Remap all the IDs
		//
		int count = 0;
		for (int index = 0; index < node_list.Count (); index ++) {
			NodeClass *node = node_list[index];
			if (node != NULL) {
				node->Set_ID (NodeMgrClass::Get_Node_ID (node->Get_Type ()));
				count ++;
			}
		}

		CString message;
		message.Format ("Remapped %d object IDs.", count);
		MessageBox (message, "ID Repair", MB_ICONINFORMATION | MB_OK);

	} else {

		//
		//	Let the user know there were no problems
		//
		MessageBox ("No ID collision detected in this level.", "ID Validation", MB_ICONINFORMATION | MB_OK);
	}
	
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnResetDynaCullSystem
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnResetDynaCullSystem (void)
{
	::Get_Scene_Editor ()->Re_Partition_Dynamic_Culling_System ();
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnDisplayLightVectors
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnDisplayLightVectors (void)
{
	bool enabled = ::Get_Scene_Editor ()->Is_Lighting_Debug_Display_Enabled ();
	::Get_Scene_Editor ()->Enable_Lighting_Debug_Display (!enabled);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnUpdateDisplayLightVectors
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnUpdateDisplayLightVectors (CCmdUI *pCmdUI)
{
	bool enabled = ::Get_Scene_Editor ()->Is_Lighting_Debug_Display_Enabled ();
	pCmdUI->SetCheck (enabled);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnDisplayStaticAnimObjects
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnDisplayStaticAnimObjects (void)
{
	bool enabled = ::Get_Scene_Editor ()->Are_Static_Anim_Phys_Displayed ();
	::Get_Scene_Editor ()->Display_Static_Anim_Phys (!enabled);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnUpdateDisplayStaticAnimObjects
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnUpdateDisplayStaticAnimObjects (CCmdUI *pCmdUI)
{
	bool enabled = ::Get_Scene_Editor ()->Are_Static_Anim_Phys_Displayed ();
	pCmdUI->SetCheck (enabled);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnEditConversations
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnEditConversations (void)
{
	EditConversationListDialogClass dialog (this);
	dialog.DoModal ();
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnBuildingPowerOn
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnBuildingPowerOn (void) 
{
	bool enabled = ::Get_Scene_Editor ()->Is_Building_Power_Enabled ();
	::Get_Scene_Editor ()->Enable_Building_Power (!enabled);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnUpdateBuildingPowerOn
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnUpdateBuildingPowerOn (CCmdUI *pCmdUI)
{
	bool enabled = ::Get_Scene_Editor ()->Is_Building_Power_Enabled ();
	pCmdUI->SetCheck (enabled);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnCreateProxies
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnCreateProxies (void)
{
	bool enabled = ::Get_Scene_Editor ()->Is_Proxy_Creation_Enabled ();
	::Get_Scene_Editor ()->Enable_Proxy_Creation (!enabled);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnUpdateCreateProxies
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnUpdateCreateProxies (CCmdUI *pCmdUI)
{
	bool enabled = ::Get_Scene_Editor ()->Is_Proxy_Creation_Enabled ();	
	pCmdUI->SetCheck (enabled);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnCheckInPresetChanges
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnCheckInPresetChanges (void)
{
	PresetMgrClass::Check_In_Presets ();
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnUpdateCheckInPresetChanges
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnUpdateCheckInPresetChanges (CCmdUI *pCmdUI)
{
	pCmdUI->Enable (PresetMgrClass::Are_Presets_Dirty ());
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnImmediatePresetCheckin
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnImmediatePresetCheckin (void)
{
	bool enabled = PresetMgrClass::Get_Immediate_Check_In_Mode ();
	PresetMgrClass::Set_Immediate_Check_In_Mode (!enabled);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnUpdateImmediatePresetCheckin
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnUpdateImmediatePresetCheckin (CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck (PresetMgrClass::Get_Immediate_Check_In_Mode ());
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnRemapIds
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnRemapIds (void)
{
	if (::MessageBox (NULL, "Are you sure you want to remap the IDs of the currently selected objects into the current ID range?", "ID Remap", MB_ICONQUESTION | MB_YESNO) == IDYES) {
		SelectionMgrClass &sel_mgr = ::Get_Scene_Editor ()->Get_Selection_Mgr ();

		//
		//	Loop over the currently selected nodes
		//
		for (int index = 0; index < sel_mgr.Get_Count (); index ++) {
			NodeClass *node = sel_mgr.Get_At (index);
			if (node != NULL) {
				
				//
				//	Remap this node's ID
				//
				node->Set_ID (NodeMgrClass::Get_Node_ID (node->Get_Type ()));
			}
		}
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnUpdateRemapIds
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnUpdateRemapIds (CCmdUI *pCmdUI)
{
	pCmdUI->Enable (::Get_Selection_Mgr ().Get_Count () > 0);
	return ;
}

//////////////////////////////////////////////////////////////////////////
//
//  OnVerifyCulling
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnVerifyCulling (void) 
{
	StringClass errors;
	errors.Format("Errors Detected:\r\n");

	bool result = ::Get_Scene_Editor()->Verify_Culling_Systems(errors);
	if (result == true) {
		::MessageBox (NULL, "Culling Systems Verified!\n", "Results", MB_OK);
	} else {
		GenericTextDialogClass dialog (this);
		dialog.Set_Title ("Results");
		dialog.Set_Description ("Culling system errors listed below.");
		dialog.Set_Text (errors);
		dialog.DoModal ();
	}

	return ;
}

//////////////////////////////////////////////////////////////////////////
//
//  OnEnableVisSectorFallback
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnEnableVisSectorFallback (void)
{
	::Get_Scene_Editor()->Enable_Vis_Sector_Fallback(!::Get_Scene_Editor()->Is_Vis_Sector_Fallback_Enabled());
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnUpdateEnableVisSectorFallback
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnUpdateEnableVisSectorFallback(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(::Get_Scene_Editor()->Is_Vis_Sector_Fallback_Enabled());
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnRunManualVisPoints
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnRunManualVisPoints (void)
{
	CWaitCursor wait_cursor;

	VisMgrClass::Render_Manual_Vis_Points ();
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnShowEditorObjects
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnShowEditorObjects (void)
{
	bool enabled = ::Get_Scene_Editor ()->Are_Editor_Objects_Displayed ();
	::Get_Scene_Editor ()->Display_Editor_Objects (!enabled);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnUpdateShowEditorObjects
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnUpdateShowEditorObjects (CCmdUI *pCmdUI)
{
	bool enabled = ::Get_Scene_Editor ()->Are_Editor_Objects_Displayed ();
	pCmdUI->SetCheck (enabled);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnAddChildNode
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnAddChildNode (void)
{
	SelectionMgrClass &sel_mgr = ::Get_Scene_Editor ()->Get_Selection_Mgr ();

	//
	//	Ensure we only have one selected node
	//
	if (sel_mgr.Get_Count () == 1) {
		NodeClass *node = sel_mgr.Get_At (0);
		if (node != NULL) {

			//
			//	If this node is a child node already, then
			// create a new child from its parent
			//
			NodeClass *parent_node = node->Get_Parent_Node ();
			if (parent_node != NULL) {
				node = parent_node;
			}

			//
			//	Add a child node and start moving it around
			//
			NodeClass *child_node = node->Add_Child_Node (Matrix3D(1));
			if (child_node != NULL) {
				::Get_Mouse_Mgr ()->Move_Node (child_node);
			}
		}
	}
	
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnUpdateAddChildNode
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnUpdateAddChildNode (CCmdUI *pCmdUI) 
{
	bool enable = false;

	SelectionMgrClass &sel_mgr = ::Get_Scene_Editor ()->Get_Selection_Mgr ();

	//
	//	Ensure we only have one selected node and its the right type
	//
	if (sel_mgr.Get_Count () == 1) {
		NodeClass *node = sel_mgr.Get_At (0);
		if (node != NULL) {

			//
			//	If we are only a child node, then use our parent
			//
			if (node->Get_Parent_Node () != NULL) {
				node = node->Get_Parent_Node ();
			}

			if (node->Can_Add_Child_Nodes ()) {
				enable = true;
			}
		}
	}

	pCmdUI->Enable (enable);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnCheckMemlog
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnCheckMemlog (void)
{
	MemLogDialogClass dialog (this);
	dialog.DoModal ();
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnMakeVisPoint
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnMakeVisPoint (void)
{
	const Matrix3D &tm = ::Get_Camera_Mgr ()->Get_Camera ()->Get_Transform ();
	::Get_Scene_Editor ()->Record_Vis_Info (tm, tm.Get_Translation ());

	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnExportStringTable
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnExportStringTable (void) 
{
	CFileDialog dialog (false, ".txt", "strings_table.txt",
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER,
		"Text Files (*.txt)|*.txt||", ::AfxGetMainWnd ());

	//
	// Ask the user what file they want to create
	//
	if (dialog.DoModal () == IDOK) {

		//
		//	Check to make sure the destination filename is not read-only
		//
		CString path				= dialog.GetPathName ();
		DWORD file_attributes	= ::GetFileAttributes (path);
		if (file_attributes != 0xFFFFFFFF && file_attributes & FILE_ATTRIBUTE_READONLY) {
			::MessageBox (::AfxGetMainWnd ()->m_hWnd, "File is read-only, export operation can not complete.", "File Error", MB_ICONERROR | MB_ICONEXCLAMATION);
		} else {
			TranslateDBClass::Export_Table (path);
		}
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnGotoObject
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnGotoObject (void)
{
	GotoObjectByIDDialogClass dialog (this);
	dialog.DoModal ();
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnExportPresets
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnExportPresets (void) 
{
	ExportPresetsDialogClass dialog (this);
	dialog.DoModal ();
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnImportPresets
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnImportPresets (void) 
{
	CFileDialog dialog (true, ".txt", "presets.txt",
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER,
		"Text Files (*.txt)|*.txt||", ::AfxGetMainWnd ());

	//
	// Ask the user what file they want to import
	//
	if (dialog.DoModal () == IDOK) {

		//
		//	Import the data
		//
		CString path = dialog.GetPathName ();
		PresetExportClass::Import (path);
	}
	
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnBatchExport
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnBatchExport (void)
{

	CFileDialog dialog (	true,
								".lvl",
								NULL,
								OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER | OFN_ALLOWMULTISELECT,
								"Level Editor Files (*.lvl)|*.lvl||",
								this);

    TCHAR filename_list[MAX_PATH * 10] = { 0 };
    dialog.m_ofn.lpstrFile = filename_list;
    dialog.m_ofn.nMaxFile = sizeof (filename_list);

	//
	// Ask the user what commando level file they wish to save to
	//
	if (dialog.DoModal () == IDOK) {

		//
		//	Get the destination for the export
		//
		CString destination_dir;
		if (::Browse_For_Folder (destination_dir, m_hWnd, "", "Select the destination directory for the export process.")) {

			//
			//	Ask the user if they want to create a subdir for each level
			// or not
			//
			bool create_subdirs = false;
			if (::MessageBox (m_hWnd, "Would you like to export each level to its own sub-directory?", "Batch Export", MB_ICONQUESTION | MB_YESNO) == IDYES) {
				create_subdirs = true;
			}
      
			//
			//	Turn off rendering
			//
			::Get_Main_View ()->Allow_Repaint (false);
			CLevelEditDoc::Set_Is_Batch_Export_Mode (true);

			//
			// Loop through all the selected files
			//
			POSITION pos = dialog.GetStartPosition (); 
			while (pos != NULL) {

				//
				//	Start fresh
				//
				::Get_Current_Document ()->OnNewDocument ();

				//
				// Load the level
				//
				CString lvl_filename = dialog.GetNextPathName (pos);
				EditorSaveLoadClass::Load_Level (lvl_filename);
				::Get_Current_Document ()->SetModifiedFlag (false);

				//
				//	Build the full path of the destination file
				//
				CString filename_base;
				filename_base = ::Get_Filename_From_Path (lvl_filename);
				filename_base = ::Asset_Name_From_Filename (filename_base);

				CString export_path = destination_dir;
				if (create_subdirs) {
					export_path = Make_Path (destination_dir, filename_base);
				}

				//
				//	Make sure the directory exists
				//
				::Create_Dir_If_Necessary (export_path);

				//
				//	Add the filename onto the path
				//
				CString full_path = Make_Path (export_path, filename_base);
				full_path += ".mix";

				//
				//	Export the level
				//
				ExporterClass exporter;
				exporter.Export_Level (full_path);

			}

			//
			//	Turn on rendering
			//
			::Get_Main_View ()->Allow_Repaint (true);
			CLevelEditDoc::Set_Is_Batch_Export_Mode (false);
		}
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnLockVisSamplePoint
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnLockVisSamplePoint (void)
{
	::Get_Scene_Editor()->Lock_Vis_Sample_Point(!::Get_Scene_Editor()->Is_Vis_Sample_Point_Locked());
	return ;
	
}


//////////////////////////////////////////////////////////////////////////
//
//  OnUpdateLockVisSamplePoint
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnUpdateLockVisSamplePoint (CCmdUI *pCmdUI) 
{
	pCmdUI->SetCheck (::Get_Scene_Editor ()->Is_Vis_Sample_Point_Locked());
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnToggleTerrainSelection
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnToggleTerrainSelection (void)
{
	if (::Get_Scene_Editor ()->Do_Groups_Collide (MOUSE_CLICK_COLLISION_GROUP, 15)) {
		::Get_Scene_Editor ()->Disable_Collision_Detection (MOUSE_CLICK_COLLISION_GROUP, 15);
	} else {
		::Get_Scene_Editor ()->Enable_Collision_Detection (MOUSE_CLICK_COLLISION_GROUP, 15);
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnUpdateToggleTerrainSelection
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnUpdateToggleTerrainSelection (CCmdUI *pCmdUI) 
{
	
	pCmdUI->SetCheck (::Get_Scene_Editor ()->Do_Groups_Collide (MOUSE_CLICK_COLLISION_GROUP, 15));
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnExportPresetFileDependencies
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnExportPresetFileDependencies (void) 
{
	
	PresetsFormClass *presets_form = ::Get_Presets_Form ();
	if (presets_form != NULL) {

		CFileDialog dialog (	false,
									".txt",
									"presets.txt",
									OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER,
									"Text file (*.txt)|*.txt||",
									this);

		//
		// Ask the user where they want to export this data
		//
		if (dialog.DoModal () == IDOK) {
			CString filename = dialog.GetPathName ();

			presets_form->Export_File_Dependencies (filename);
		}
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnImportStrings
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnImportStrings (void)
{
	CWaitCursor wait_cursor;
	StringsMgrClass::Import_Strings ();
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnExportTileList
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnExportTileList (void) 
{
	//
	// Use the level's filename to generate a default filename
	//
	CString default_name = ::Get_Current_Document ()->GetPathName ();
	if (default_name.GetLength () > 0) {
		default_name = ::Get_Filename_From_Path (default_name);
		default_name = ::Asset_Name_From_Filename (default_name);
		default_name += ".txt";
	}

	CFileDialog dialog (	false,
								".txt",
								default_name,
								OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER,
								"Text Files (*.txt)|*.txt||",
								this);

	if (dialog.DoModal () == IDOK) {		
		
		//
		//	Open the file
		//
		StringClass filename = static_cast<const char *>(dialog.GetPathName ());
		TextFileClass text_file (filename);
		if (text_file.Open (FileClass::WRITE)) {

			DynamicVectorClass<NodeClass *> node_list;
			
			//
			//	Ask the user if they want to export all tiles or just the selection set.
			//
			bool do_sel_only = (MessageBox ("Would you like to export tiles that are only in the current selection?", "Export Tile List", MB_ICONQUESTION | MB_YESNO) == IDYES);
			if (do_sel_only) {

				SelectionMgrClass &sel_mgr = ::Get_Scene_Editor ()->Get_Selection_Mgr ();

				//
				//	Loop over the currently selected nodes
				//
				for (int index = 0; index < sel_mgr.Get_Count (); index ++) {
					NodeClass *node = sel_mgr.Get_At (index);
					if (	node != NULL &&
							node->Is_Proxied () == false &&
							node->Get_Type () == NODE_TYPE_TILE)
					{
						node_list.Add (node);
					}
				}
				
			} else {
				
				//
				//	Loop over all the nodes in the scene
				//
				for (	NodeClass *node = NodeMgrClass::Get_First ();
						node != NULL;
						node = NodeMgrClass::Get_Next (node))
				{
					if (	node != NULL &&
							node->Is_Proxied () == false &&
							node->Get_Type () == NODE_TYPE_TILE)
					{
						node_list.Add (node);
					}
				}
			}

			//
			//	Write an entry for each node in the list
			//
			for (int index = 0; index < node_list.Count (); index ++) {
				NodeClass *node	= node_list[index];
				
				const char *name		= node->Get_Name ();
				const Matrix3D &tm	= node->Get_Transform ();
				const Vector3 &pos	= tm.Get_Translation ();

				//
				//	Build a string containing the tile information
				//
				StringClass text;
				text.Format ("%s:\t(%.3f, %.3f, %.3f), %.2f degrees", name, pos.X, pos.Y, pos.Z, RAD_TO_DEGF (tm.Get_Z_Rotation ()));
				
				//
				//	Write this line to the file
				//
				text_file.Write_Line (text);
			}
			
			text_file.Close ();
		}
	}
	
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnRemapUnimportantIds
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnRemapUnimportantIds (void)
{
	NODE_LIST node_list;
	NodeMgrClass::Build_Unimportant_ID_Collision_List (node_list);

	if (node_list.Count () > 0) {

		//
		//	Remap all the IDs
		//
		int count = 0;
		for (int index = 0; index < node_list.Count (); index ++) {
			NodeClass *node = node_list[index];
			if (node != NULL) {
				node->Set_ID (NodeMgrClass::Get_Node_ID (node->Get_Type ()));
				count ++;
			}
		}

		CString message;
		message.Format ("Remapped %d object IDs.", count);
		MessageBox (message, "ID Repair", MB_ICONINFORMATION | MB_OK);

	} else {

		//
		//	Let the user know there were no problems
		//
		MessageBox ("No ID collision detected in this level.", "ID Validation", MB_ICONINFORMATION | MB_OK);
	}
	
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnEditLevelSettings
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnEditLevelSettings (void)
{
	LevelSettingsDialogClass dialog;
	dialog.DoModal ();
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnExportTranslationData
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnExportTranslationData (void)
{
	CFileDialog dialog (	false,
								".xls",
								"strings.xls",
								OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER,
								"Excel Spreadsheet (*.xls)|*.xls||",
								this);

	//
	// Ask the user what file they want to save to
	//
	if (dialog.DoModal () == IDOK) {
		ImportTranslationDialogClass import_dialog (this);
		import_dialog.Set_Filename (dialog.GetPathName ());
		import_dialog.Set_Is_For_Export (true);
		import_dialog.DoModal ();		
	}
		
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnExportTranslationData
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnImportTranslationData (void)
{
	CFileDialog dialog (	true,
								".xls",
								"strings.xls",
								OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_SHAREAWARE,
								"Excel Spreadsheet (*.xls)|*.xls||",
								this);

	//
	// Ask the user what file they want to import
	//
	if (dialog.DoModal () == IDOK) {
		ImportTranslationDialogClass import_dialog (this);
		import_dialog.Set_Filename (dialog.GetPathName ());
		import_dialog.DoModal ();
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////
//
// OnCompressTextures
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnCompressTextures (void)
{
	bool enabled = EditorMixFileCreator::Are_Textures_Compressed();
	EditorMixFileCreator::Set_Texture_Compression (!enabled);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
// OnUpdateCompressTextures
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnUpdateCompressTextures (CCmdUI *pCmdUI) 
{
	bool enabled = EditorMixFileCreator::Are_Textures_Compressed();
	pCmdUI->SetCheck (enabled);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
// OnExportFileUsageReport
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnExportFileUsageReport (void)
{
	CFileDialog dialog (	false,
								".txt",
								"files.txt",
								OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER,
								"Text Files (*.txt)|*.txt||",
								this);

	//
	// Ask the user what file they want to save to
	//
	if (dialog.DoModal () == IDOK) {
		ReportMgrClass::Export_File_Usage_Report (dialog.GetPathName ());
	}
		
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
// OnExportMissingTranslationReport
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnExportMissingTranslationReport (void)
{
	CFileDialog dialog (	false,
								".txt",
								"files.txt",
								OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER,
								"Text Files (*.txt)|*.txt||",
								this);

	//
	// Ask the user what file they want to save to
	//
	if (dialog.DoModal () == IDOK) {
		MissingTranslationReportDialogClass report_dialog (this);
		report_dialog.Set_Filename (dialog.GetPathName ());
		report_dialog.DoModal ();
	}
		
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
// OnExportLanguage
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnExportLanguage (void)
{
	//
	// Determine which directory we should use as a default for the export
	//
	CString full_path = theApp.GetProfileString (CONFIG_KEY, LAST_EXPORT_DIR_VALUE);
	if (full_path.GetLength () == 0) {
		full_path = ::Get_File_Mgr ()->Make_Full_Path (LEVELS_ASSET_DIR);		
	}
	_pThe3DAssetManager->Set_Current_Directory (full_path);


	CFileDialog dialog (	false,
								".dbs",
								"always.dbs",
								OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER,
								"Renegade Database (*.dbs)|*.dbs||",
								this);

	//
	// Set the path, so it opens in the correct directory
	//
	dialog.m_ofn.lpstrInitialDir = full_path;

	//
	// Ask the user what file they want to save to
	//
	if (dialog.DoModal () == IDOK) {
		TranslationExportDialogClass export_dialog (this);
		export_dialog.Set_Filename (dialog.GetPathName ());
		export_dialog.DoModal ();
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////
//
// OnInitMenuPopup
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnInitMenuPopup (CMenu *popup_menu, UINT nIndex, BOOL bSysMenu) 
{
	CFrameWnd::OnInitMenuPopup (popup_menu, nIndex, bSysMenu);

	if (nIndex == CAMERA_MENU_INDEX) {

		for (int index = 0; index < m_MaxCamProfiles; index ++) {
			popup_menu->EnableMenuItem (IDM_FIRST_CAM_PROFILE + index, MF_BYCOMMAND | MF_ENABLED);
		}
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////
//
// OnAlwaysLocalExport
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnAlwaysLocalExport (void)
{
	//
	// Ask the user to pick a path
	//
	CString path;
	if (::Browse_For_Folder (path,
									 NULL,
									 NULL,
									 "Select a directory where the core file set will be copied.")) {
		
		//
		// Copy the always files to the specified directory
		//
		CWaitCursor wait_cursor;		
		
		ExporterClass exporter;
		exporter.Dont_Get_Latest_Versions (true);
		exporter.Export_Always_Files (path);

		//
		//	Cache this path in the registry
		//
		theApp.WriteProfileString (CONFIG_KEY, LAST_EXPORT_DIR_VALUE, path);
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////
//
// OnExportInstallerLanguageVersion
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnExportInstallerLanguageVersion (void)
{
	//
	// Determine which directory we should use as a default for the export
	//
	CString full_path = theApp.GetProfileString (CONFIG_KEY, LAST_EXPORT_DIR_VALUE);
	if (full_path.GetLength () == 0) {
		full_path = ::Get_File_Mgr ()->Make_Full_Path (LEVELS_ASSET_DIR);		
	}
	_pThe3DAssetManager->Set_Current_Directory (full_path);


	CFileDialog dialog (	false,
								".tdb",
								"istrings.tdb",
								OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER,
								"Translation Database(*.tdb)|*.tdb||",
								this);

	//
	// Set the path, so it opens in the correct directory
	//
	dialog.m_ofn.lpstrInitialDir = full_path;

	//
	// Ask the user what file they want to save to
	//
	if (dialog.DoModal () == IDOK) {
		TranslationExportDialogClass export_dialog (this);
		export_dialog.Set_Filename (dialog.GetPathName ());
		export_dialog.Set_Is_Installer (true);
		export_dialog.DoModal ();
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////
//
// OnExtractInstallerRcStrings
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnExtractInstallerRcStrings (void)
{
	CFileDialog dialog (	true,
								".rc",
								NULL,
								OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_SHAREAWARE,
								"Resource Files (*.rc)|*.rc||",
								this);

	//
	// Ask the user what file they want to extract strings from
	//
	if (dialog.DoModal () == IDOK) {
		RCStringExtractorClass extractor;
		extractor.Set_Src_RC_Filename (dialog.GetPathName ());
		extractor.Set_TranslationDB_Prefix ("IDS_INSTALLER_TEXT");
		extractor.Set_TranslationDB_Category ("Installer");
		extractor.Extract_Strings ();
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////
//
// OnExtractRcStrings
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnExtractRcStrings (void)
{
	CFileDialog dialog (	true,
								".rc",
								NULL,
								OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_SHAREAWARE,
								"Resource Files (*.rc)|*.rc||",
								this);

	//
	// Ask the user what file they want to extract strings from
	//
	if (dialog.DoModal () == IDOK) {
		RCStringExtractorClass extractor;
		extractor.Set_Src_RC_Filename (dialog.GetPathName ());
		extractor.Set_TranslationDB_Prefix ("IDS_MENU_TEXT");
		extractor.Set_TranslationDB_Category ("Menu");
		extractor.Extract_Strings ();
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////
//
// OnCameraPerspective
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnCameraPerspective (void) 
{
	::Get_Camera_Mgr ()->Get_Camera ()->Set_Projection_Type (CameraClass::PERSPECTIVE);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
// OnCameraOrthographic
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnCameraOrthographic (void)
{
	CameraClass *camera = ::Get_Camera_Mgr ()->Get_Camera ();
	camera->Set_Projection_Type (CameraClass::ORTHO);;

	Vector3 world_min;
	Vector3 world_max;
	PhysicsSceneClass::Get_Instance ()->Get_Level_Extents (world_min, world_max);

	Vector3 delta = world_max - world_min;

	Vector3 world_center = world_min + ((world_max - world_min) * 0.5F);
	world_center.Z = world_max.Z + 100.0F;

	Vector2 view_min;
	Vector2 view_max;

	view_min.X = world_min.X - world_center.X;
	view_min.Y = world_min.Y - world_center.Y;

	view_max.X = world_max.X - world_center.X;
	view_max.Y = world_max.Y - world_center.Y;

	camera->Set_View_Plane (view_min, view_max);
	camera->Set_Clip_Planes (50.0F, 5000.0F);

	Matrix3D tm (1);
	tm.Translate(world_center);

	camera->Set_Transform (tm);

	return ;
}


//////////////////////////////////////////////////////////////////////////
//
// OnUpdateCameraOrthographic
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnUpdateCameraOrthographic (CCmdUI *pCmdUI) 
{
	bool is_checked = (::Get_Camera_Mgr ()->Get_Camera ()->Get_Projection_Type () == CameraClass::ORTHO);
	pCmdUI->SetCheck (is_checked);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
// OnUpdateCameraPerspective
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnUpdateCameraPerspective (CCmdUI *pCmdUI) 
{
	bool is_checked = (::Get_Camera_Mgr ()->Get_Camera ()->Get_Projection_Type () == CameraClass::PERSPECTIVE);
	pCmdUI->SetCheck (is_checked);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
// OnModExport
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnModExport (void) 
{
	//
	// Determine which directory we should use as a default for the export
	//
	CString full_path = MixFileDatabaseClass::Get_Instance ()->Get_Mix_File_Path ();
	_pThe3DAssetManager->Set_Current_Directory (full_path);

	//
	// Use the level's filename to generate a default INI filename
	//
	CString default_name = AssetPackageMgrClass::Get_Current_Package ();
	if (default_name.GetLength () > 0) {
		default_name += ".pkg";
	}

	CFileDialog dialog (	false,
								".pkg",
								default_name,
								OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER,
								"Renegade Mod Package (*.pkg)|*.pkg||",
								this);

	//
	// Set the path, so it opens in the correct directory
	//
	dialog.m_ofn.lpstrInitialDir = full_path;

	//
	// Ask the user where they want to save this package
	//
	if (dialog.DoModal () == IDOK) {
		ExporterClass exporter;
		exporter.Export_Package (dialog.GetPathName ());
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////
//
// OnSaveLevel
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnSaveLevel (void)
{
	//
	// Determine which directory we should save to
	//
	CString full_path = ::Get_File_Mgr ()->Make_Full_Path (LEVELS_ASSET_DIR);		
	_pThe3DAssetManager->Set_Current_Directory (full_path);

	//
	// Make a default filename from the name of the document
	//
	CString default_name = GetActiveDocument ()->GetTitle ();
	if (default_name.GetLength () > 0) {
		default_name += ".lvl";
	}

	CFileDialog dialog (	true,
								".lvl",
								default_name,
								OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER,
								"Level Editor Files (*.lvl)|*.lvl||",
								this);

	//
	// Set the path, so it opens in the correct directory
	//
	dialog.m_ofn.lpstrInitialDir = full_path;

	//
	// Ask the user where they want to save this level
	//
	if (dialog.DoModal () == IDOK) {		
		
		//
		//	Save in level editor format
		//
		GetActiveDocument ()->OnSaveDocument (dialog.GetPathName ());

		//
		//	Save in game format
		//
		ExporterClass exporter;
		exporter.Export_Level_Only (dialog.GetPathName ());
	}

	return ;
}


void CMainFrame::OnComputeVertexSolve() 
{
	LightSolveOptionsDialogClass dialog(this);
	
	if (dialog.DoModal() == IDOK) {
		SceneEditorClass *scene = ::Get_Scene_Editor ();
		if (scene != NULL) {
			scene->Update_Lighting();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// OnModeHeightEdit
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnModeHeightEdit (void)
{
	::Get_Mouse_Mgr ()->Set_Mouse_Mode (MouseMgrClass::MODE_HEIGHTFIELD_EDIT);
	return ;
}


//////////////////////////////////////////////////////////////////////////
//
//  OnUpdateModeHeightEdit
//
//////////////////////////////////////////////////////////////////////////
void
CMainFrame::OnUpdateModeHeightEdit (CCmdUI *pCmdUI) 
{
	MouseMgrClass *mouse_mgr = ::Get_Mouse_Mgr ();

	//
	//	Update the toolbar
	//
	if (mouse_mgr != NULL) {
		pCmdUI->SetCheck ((BOOL)::Get_Mouse_Mgr ()->Get_Mouse_Mode () == MouseMgrClass::MODE_HEIGHTFIELD_EDIT);
	}

	return ;	
}
