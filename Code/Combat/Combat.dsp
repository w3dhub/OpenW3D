# Microsoft Developer Studio Project File - Name="Combat" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=Combat - Win32 DebugE
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Combat.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Combat.mak" CFG="Combat - Win32 DebugE"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Combat - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Combat - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "Combat - Win32 Profile" (based on "Win32 (x86) Static Library")
!MESSAGE "Combat - Win32 DebugE" (based on "Win32 (x86) Static Library")
!MESSAGE "Combat - Win32 ProfileE" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Commando/Code/Combat", IVHCAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Combat - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MT /W4 /O2 /Ob2 /I "..\wwui" /I "..\wwutil" /I "..\wwbitpack" /I "..\wwaudio" /I "..\miles6\include" /I "..\wwnet" /I "..\wwmath" /I "..\ww3d2" /I "..\wwphys" /I "..\wwlib" /I "..\wwdebug" /I "..\wwsaveload" /I "..\DirectX\include" /I "..\wwtranslatedb" /D "NDEBUG" /D _WIN32_WINNT=0x400 /D "WIN32" /D "_MBCS" /D "_LIB" /D "_WINDOWS" /YX /FD /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\libs\Release\Combat.lib"

!ELSEIF  "$(CFG)" == "Combat - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W4 /Gm /ZI /Od /I "..\wwui" /I "..\wwutil" /I "..\wwbitpack" /I "..\wwaudio" /I "..\miles6\include" /I "..\wwnet" /I "..\wwmath" /I "..\ww3d2" /I "..\wwphys" /I "..\wwlib" /I "..\wwdebug" /I "..\wwsaveload" /I "..\DirectX\include" /I "..\wwtranslatedb" /D "_DEBUG" /D "WWDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "_WINDOWS" /YX /FD /GZ /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\libs\Debug\Combat.lib"

!ELSEIF  "$(CFG)" == "Combat - Win32 Profile"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Profile"
# PROP BASE Intermediate_Dir "Profile"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Profile"
# PROP Intermediate_Dir "Profile"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /I "..\wwaudio" /I "..\miles" /I "..\wolapi" /I "..\wwutil" /I "..\wwlzhl" /I "..\wwnet" /I "..\wwmath" /I "..\ww3d" /I "..\wwphys" /I "..\wwlib" /I "..\wwdebug" /I "..\wwsaveload" /I "..\directx\inc" /I "..\srsdk1x\include" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MT /W4 /Zi /O2 /Op /Ob2 /I "..\wwui" /I "..\wwutil" /I "..\wwbitpack" /I "..\wwaudio" /I "..\miles6\include" /I "..\wwnet" /I "..\wwmath" /I "..\ww3d2" /I "..\wwphys" /I "..\wwlib" /I "..\wwdebug" /I "..\wwsaveload" /I "..\DirectX\include" /I "..\wwtranslatedb" /D "NDEBUG" /D "WWDEBUG" /D _WIN32_WINNT=0x400 /D "WIN32" /D "_MBCS" /D "_LIB" /D "_WINDOWS" /FD /c
# SUBTRACT CPP /Fr /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\libs\Profile\Combat.lib"

!ELSEIF  "$(CFG)" == "Combat - Win32 DebugE"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "DebugE"
# PROP BASE Intermediate_Dir "DebugE"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DebugE"
# PROP Intermediate_Dir "DebugE"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W4 /Gm /GX /ZI /Od /I "..\wwutil" /I "..\wwbitpack" /I "..\wwaudio" /I "..\miles6\include" /I "..\wwnet" /I "..\wwmath" /I "..\ww3d" /I "..\wwphys" /I "..\wwlib" /I "..\wwdebug" /I "..\wwsaveload" /I "..\directx\inc" /I "..\srsdk1x\include" /I "..\wwtranslatedb" /D "_DEBUG" /D "WWDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "_WINDOWS" /FR /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W4 /Gm /ZI /Od /I "..\wwui" /I "..\wwutil" /I "..\wwbitpack" /I "..\wwaudio" /I "..\miles6\include" /I "..\wwnet" /I "..\wwmath" /I "..\ww3d2" /I "..\wwphys" /I "..\wwlib" /I "..\wwdebug" /I "..\wwsaveload" /I "..\DirectX\include" /I "..\wwtranslatedb" /D "_DEBUG" /D "WWDEBUG" /D "PARAM_EDITING_ON" /D "WIN32" /D "_MBCS" /D "_LIB" /D "_WINDOWS" /YX /FD /GZ /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\libs\Debug\Combat.lib"
# ADD LIB32 /nologo /out:"..\libs\Debug\Combate.lib"

!ELSEIF  "$(CFG)" == "Combat - Win32 ProfileE"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ProfileE"
# PROP BASE Intermediate_Dir "ProfileE"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ProfileE"
# PROP Intermediate_Dir "ProfileE"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W4 /GX /Zi /O2 /I "..\wwutil" /I "..\wwbitpack" /I "..\wwaudio" /I "..\miles6\include" /I "..\wwnet" /I "..\wwmath" /I "..\ww3d" /I "..\wwphys" /I "..\wwlib" /I "..\wwdebug" /I "..\wwsaveload" /I "..\directx\inc" /I "..\srsdk1x\include" /I "..\wwtranslatedb" /D "NDEBUG" /D "WWDEBUG" /D _WIN32_WINNT=0x400 /D "WIN32" /D "_MBCS" /D "_LIB" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W4 /Zi /O2 /I "..\wwui" /I "..\wwutil" /I "..\wwbitpack" /I "..\wwaudio" /I "..\miles6\include" /I "..\wwnet" /I "..\wwmath" /I "..\ww3d2" /I "..\wwphys" /I "..\wwlib" /I "..\wwdebug" /I "..\wwsaveload" /I "..\DirectX\include" /I "..\wwtranslatedb" /D "NDEBUG" /D "WWDEBUG" /D _WIN32_WINNT=0x400 /D "PARAM_EDITING_ON" /D "WIN32" /D "_MBCS" /D "_LIB" /D "_WINDOWS" /YX /FD /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\libs\Profile\Combat.lib"
# ADD LIB32 /nologo /out:"..\libs\Profile\Combate.lib"

!ENDIF 

# Begin Target

# Name "Combat - Win32 Release"
# Name "Combat - Win32 Debug"
# Name "Combat - Win32 Profile"
# Name "Combat - Win32 DebugE"
# Name "Combat - Win32 ProfileE"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\action.cpp
# End Source File
# Begin Source File

SOURCE=.\activeconversation.cpp
# End Source File
# Begin Source File

SOURCE=.\airstripgameobj.cpp
# End Source File
# Begin Source File

SOURCE=.\animcontrol.cpp
# End Source File
# Begin Source File

SOURCE=.\armedgameobj.cpp
# End Source File
# Begin Source File

SOURCE=.\assetdep.cpp
# End Source File
# Begin Source File

SOURCE=.\backgroundmgr.cpp
# End Source File
# Begin Source File

SOURCE=.\basecontroller.cpp
# End Source File
# Begin Source File

SOURCE=.\basegameobj.cpp
# End Source File
# Begin Source File

SOURCE=.\beacongameobj.cpp
# End Source File
# Begin Source File

SOURCE=.\bones.cpp
# End Source File
# Begin Source File

SOURCE=.\building.cpp
# End Source File
# Begin Source File

SOURCE=.\buildingaggregate.cpp
# End Source File
# Begin Source File

SOURCE=.\buildingmonitor.cpp
# End Source File
# Begin Source File

SOURCE=.\buildingstate.cpp
# End Source File
# Begin Source File

SOURCE=.\bullet.cpp
# End Source File
# Begin Source File

SOURCE=.\c4.cpp
# End Source File
# Begin Source File

SOURCE=.\ccamera.cpp
# End Source File
# Begin Source File

SOURCE=.\characterclasssettings.cpp
# End Source File
# Begin Source File

SOURCE=.\cheatmgr.cpp
# End Source File
# Begin Source File

SOURCE=.\cinematicgameobj.cpp
# End Source File
# Begin Source File

SOURCE=.\clientcontrol.cpp
# End Source File
# Begin Source File

SOURCE=.\CNCModeSettings.cpp
# End Source File
# Begin Source File

SOURCE=.\colors.cpp
# End Source File
# Begin Source File

SOURCE=.\combat.cpp
# End Source File
# Begin Source File

SOURCE=.\combatdazzle.cpp
# End Source File
# Begin Source File

SOURCE=.\combatmaterialeffectmanager.cpp
# End Source File
# Begin Source File

SOURCE=.\combatsaveload.cpp
# End Source File
# Begin Source File

SOURCE=.\combatsound.cpp
# End Source File
# Begin Source File

SOURCE=.\comcentergameobj.cpp
# End Source File
# Begin Source File

SOURCE=.\control.cpp
# End Source File
# Begin Source File

SOURCE=.\conversation.cpp
# End Source File
# Begin Source File

SOURCE=.\conversationmgr.cpp
# End Source File
# Begin Source File

SOURCE=.\conversationremark.cpp
# End Source File
# Begin Source File

SOURCE=.\cover.cpp
# End Source File
# Begin Source File

SOURCE=.\csdamageevent.cpp
# End Source File
# Begin Source File

SOURCE=.\damage.cpp
# End Source File
# Begin Source File

SOURCE=.\damageablegameobj.cpp
# End Source File
# Begin Source File

SOURCE=.\damageablestaticphys.cpp
# End Source File
# Begin Source File

SOURCE=.\damagezone.cpp
# End Source File
# Begin Source File

SOURCE=.\diaglog.cpp
# End Source File
# Begin Source File

SOURCE=.\dialogue.cpp
# End Source File
# Begin Source File

SOURCE=.\doors.cpp
# End Source File
# Begin Source File

SOURCE=.\dynamicspeechanim.cpp
# End Source File
# Begin Source File

SOURCE=.\effectrecycler.cpp
# End Source File
# Begin Source File

SOURCE=.\elevator.cpp
# End Source File
# Begin Source File

SOURCE=.\encyclopediamgr.cpp
# End Source File
# Begin Source File

SOURCE=.\evasettings.cpp
# End Source File
# Begin Source File

SOURCE=.\explosion.cpp
# End Source File
# Begin Source File

SOURCE=.\ffactorylist.cpp
# End Source File
# Begin Source File

SOURCE=.\gameobjmanager.cpp
# End Source File
# Begin Source File

SOURCE=.\gameobjobserver.cpp
# End Source File
# Begin Source File

SOURCE=.\gametype.cpp
# End Source File
# Begin Source File

SOURCE=.\globalsettings.cpp
# End Source File
# Begin Source File

SOURCE=.\harvester.cpp
# End Source File
# Begin Source File

SOURCE=.\hud.cpp
# End Source File
# Begin Source File

SOURCE=.\hudinfo.cpp
# End Source File
# Begin Source File

SOURCE=.\humanrecoil.cpp
# End Source File
# Begin Source File

SOURCE=.\humanstate.cpp
# End Source File
# Begin Source File

SOURCE=.\input.cpp
# End Source File
# Begin Source File

SOURCE=.\inventory.cpp
# End Source File
# Begin Source File

SOURCE=.\mapmgr.cpp
# End Source File
# Begin Source File

SOURCE=.\mendozabossgameobj.cpp
# End Source File
# Begin Source File

SOURCE=.\messagewindow.cpp
# End Source File
# Begin Source File

SOURCE=.\muzzlerecoil.cpp
# End Source File
# Begin Source File

SOURCE=.\objectives.cpp
# End Source File
# Begin Source File

SOURCE=.\objectivesviewer.cpp
# End Source File
# Begin Source File

SOURCE=.\objlibrary.cpp
# End Source File
# Begin Source File

SOURCE=.\orator.cpp
# End Source File
# Begin Source File

SOURCE=.\oratortypes.cpp
# End Source File
# Begin Source File

SOURCE=.\pathaction.cpp
# End Source File
# Begin Source File

SOURCE=.\persistentgameobjobserver.cpp
# End Source File
# Begin Source File

SOURCE=.\physicalgameobj.cpp
# End Source File
# Begin Source File

SOURCE=.\pilot.cpp
# End Source File
# Begin Source File

SOURCE=.\playerdata.cpp
# End Source File
# Begin Source File

SOURCE=.\playerterminal.cpp
# End Source File
# Begin Source File

SOURCE=.\playertype.cpp
# End Source File
# Begin Source File

SOURCE=.\powerplantgameobj.cpp
# End Source File
# Begin Source File

SOURCE=.\powerup.cpp
# End Source File
# Begin Source File

SOURCE=.\purchasesettings.cpp
# End Source File
# Begin Source File

SOURCE=.\radar.cpp
# End Source File
# Begin Source File

SOURCE=.\raveshawbossgameobj.cpp
# End Source File
# Begin Source File

SOURCE=.\refinerygameobj.cpp
# End Source File
# Begin Source File

SOURCE=.\reflist.cpp
# End Source File
# Begin Source File

SOURCE=.\repairbaygameobj.cpp
# End Source File
# Begin Source File

SOURCE=.\sakurabossgameobj.cpp
# End Source File
# Begin Source File

SOURCE=.\samsite.cpp
# End Source File
# Begin Source File

SOURCE=.\savegame.cpp
# End Source File
# Begin Source File

SOURCE=.\scexplosionevent.cpp
# End Source File
# Begin Source File

SOURCE=.\scobeliskevent.cpp
# End Source File
# Begin Source File

SOURCE=.\screenfademanager.cpp
# End Source File
# Begin Source File

SOURCE=.\scriptablegameobj.cpp
# End Source File
# Begin Source File

SOURCE=.\scriptcommands.cpp
# End Source File
# Begin Source File

SOURCE=.\scripts.cpp
# End Source File
# Begin Source File

SOURCE=.\scriptzone.cpp
# End Source File
# Begin Source File

SOURCE=.\simplegameobj.cpp
# End Source File
# Begin Source File

SOURCE=.\smartgameobj.cpp
# End Source File
# Begin Source File

SOURCE=.\sniper.cpp
# End Source File
# Begin Source File

SOURCE=.\soldier.cpp

!IF  "$(CFG)" == "Combat - Win32 Release"

!ELSEIF  "$(CFG)" == "Combat - Win32 Debug"

!ELSEIF  "$(CFG)" == "Combat - Win32 Profile"

# ADD CPP /Ob2

!ELSEIF  "$(CFG)" == "Combat - Win32 DebugE"

!ELSEIF  "$(CFG)" == "Combat - Win32 ProfileE"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\soldierfactorygameobj.cpp
# End Source File
# Begin Source File

SOURCE=.\soldierobserver.cpp
# End Source File
# Begin Source File

SOURCE=.\SoundEnvironment.cpp
# End Source File
# Begin Source File

SOURCE=.\spawn.cpp
# End Source File
# Begin Source File

SOURCE=.\specialeffectsgameobj.cpp
# End Source File
# Begin Source File

SOURCE=.\statemachine.cpp
# End Source File
# Begin Source File

SOURCE=.\staticnetworkobject.cpp
# End Source File
# Begin Source File

SOURCE=.\surfaceeffects.cpp
# End Source File
# Begin Source File

SOURCE=.\systeminfolog.cpp
# End Source File
# Begin Source File

SOURCE=.\teampurchasesettings.cpp
# End Source File
# Begin Source File

SOURCE=.\textwindow.cpp
# End Source File
# Begin Source File

SOURCE=.\transition.cpp
# End Source File
# Begin Source File

SOURCE=.\transitiongameobj.cpp
# End Source File
# Begin Source File

SOURCE=.\unitcoordinationzonemgr.cpp
# End Source File
# Begin Source File

SOURCE=.\vehicle.cpp
# End Source File
# Begin Source File

SOURCE=.\vehicledriver.cpp
# End Source File
# Begin Source File

SOURCE=.\vehiclefactorygameobj.cpp
# End Source File
# Begin Source File

SOURCE=.\viseme.cpp
# End Source File
# Begin Source File

SOURCE=.\warfactorygameobj.cpp
# End Source File
# Begin Source File

SOURCE=.\weaponbag.cpp
# End Source File
# Begin Source File

SOURCE=.\weaponmanager.cpp
# End Source File
# Begin Source File

SOURCE=.\weapons.cpp
# End Source File
# Begin Source File

SOURCE=.\weaponview.cpp
# End Source File
# Begin Source File

SOURCE=.\WeatherMgr.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\action.h
# End Source File
# Begin Source File

SOURCE=.\actionparams.h
# End Source File
# Begin Source File

SOURCE=.\activeconversation.h
# End Source File
# Begin Source File

SOURCE=.\airstripgameobj.h
# End Source File
# Begin Source File

SOURCE=.\animcontrol.h
# End Source File
# Begin Source File

SOURCE=.\apppackettypes.h
# End Source File
# Begin Source File

SOURCE=.\armedgameobj.h
# End Source File
# Begin Source File

SOURCE=.\assetdep.h
# End Source File
# Begin Source File

SOURCE=.\backgroundmgr.h
# End Source File
# Begin Source File

SOURCE=.\basecontroller.h
# End Source File
# Begin Source File

SOURCE=.\basegameobj.h
# End Source File
# Begin Source File

SOURCE=.\beacongameobj.h
# End Source File
# Begin Source File

SOURCE=.\bones.h
# End Source File
# Begin Source File

SOURCE=.\building.h
# End Source File
# Begin Source File

SOURCE=.\buildingaggregate.h
# End Source File
# Begin Source File

SOURCE=.\buildingmonitor.h
# End Source File
# Begin Source File

SOURCE=.\buildingstate.h
# End Source File
# Begin Source File

SOURCE=.\bullet.h
# End Source File
# Begin Source File

SOURCE=.\c4.h
# End Source File
# Begin Source File

SOURCE=.\ccamera.h
# End Source File
# Begin Source File

SOURCE=.\characterclasssettings.h
# End Source File
# Begin Source File

SOURCE=.\cheatmgr.h
# End Source File
# Begin Source File

SOURCE=.\cinematicgameobj.h
# End Source File
# Begin Source File

SOURCE=.\clientcontrol.h
# End Source File
# Begin Source File

SOURCE=.\CNCModeSettings.h
# End Source File
# Begin Source File

SOURCE=.\colors.h
# End Source File
# Begin Source File

SOURCE=.\combat.h
# End Source File
# Begin Source File

SOURCE=.\combatchunkid.h
# End Source File
# Begin Source File

SOURCE=.\combatdazzle.h
# End Source File
# Begin Source File

SOURCE=.\combatmaterialeffectmanager.h
# End Source File
# Begin Source File

SOURCE=.\combatphysobserver.h
# End Source File
# Begin Source File

SOURCE=.\combatsaveload.h
# End Source File
# Begin Source File

SOURCE=.\combatsound.h
# End Source File
# Begin Source File

SOURCE=.\comcentergameobj.h
# End Source File
# Begin Source File

SOURCE=.\control.h
# End Source File
# Begin Source File

SOURCE=.\conversation.h
# End Source File
# Begin Source File

SOURCE=.\conversationmgr.h
# End Source File
# Begin Source File

SOURCE=.\conversationremark.h
# End Source File
# Begin Source File

SOURCE=.\cover.h
# End Source File
# Begin Source File

SOURCE=.\csdamageevent.h
# End Source File
# Begin Source File

SOURCE=.\damage.h
# End Source File
# Begin Source File

SOURCE=.\damageablegameobj.h
# End Source File
# Begin Source File

SOURCE=.\damageablestaticphys.h
# End Source File
# Begin Source File

SOURCE=.\damagezone.h
# End Source File
# Begin Source File

SOURCE=.\diaglog.h
# End Source File
# Begin Source File

SOURCE=.\dialogue.h
# End Source File
# Begin Source File

SOURCE=.\doors.h
# End Source File
# Begin Source File

SOURCE=.\dynamicspeechanim.h
# End Source File
# Begin Source File

SOURCE=.\effectrecycler.h
# End Source File
# Begin Source File

SOURCE=.\elevator.h
# End Source File
# Begin Source File

SOURCE=.\encyclopediamgr.h
# End Source File
# Begin Source File

SOURCE=.\evasettings.h
# End Source File
# Begin Source File

SOURCE=.\explosion.h
# End Source File
# Begin Source File

SOURCE=.\ffactorylist.h
# End Source File
# Begin Source File

SOURCE=.\gameobjmanager.h
# End Source File
# Begin Source File

SOURCE=.\gameobjobserver.h
# End Source File
# Begin Source File

SOURCE=.\gameobjref.h
# End Source File
# Begin Source File

SOURCE=.\gametype.h
# End Source File
# Begin Source File

SOURCE=.\globalsettings.h
# End Source File
# Begin Source File

SOURCE=.\harvester.h
# End Source File
# Begin Source File

SOURCE=.\hud.h
# End Source File
# Begin Source File

SOURCE=.\hudinfo.h
# End Source File
# Begin Source File

SOURCE=.\humanrecoil.h
# End Source File
# Begin Source File

SOURCE=.\humanstate.h
# End Source File
# Begin Source File

SOURCE=.\input.h
# End Source File
# Begin Source File

SOURCE=.\inventory.h
# End Source File
# Begin Source File

SOURCE=.\mapmgr.h
# End Source File
# Begin Source File

SOURCE=.\mendozabossgameobj.h
# End Source File
# Begin Source File

SOURCE=.\messagewindow.h
# End Source File
# Begin Source File

SOURCE=.\muzzlerecoil.h
# End Source File
# Begin Source File

SOURCE=.\netclassids.h
# End Source File
# Begin Source File

SOURCE=.\objectives.h
# End Source File
# Begin Source File

SOURCE=.\objectivesviewer.h
# End Source File
# Begin Source File

SOURCE=.\objlibrary.h
# End Source File
# Begin Source File

SOURCE=.\orator.h
# End Source File
# Begin Source File

SOURCE=.\oratortypes.h
# End Source File
# Begin Source File

SOURCE=.\pathaction.h
# End Source File
# Begin Source File

SOURCE=.\persistentgameobjobserver.h
# End Source File
# Begin Source File

SOURCE=.\physicalgameobj.h
# End Source File
# Begin Source File

SOURCE=.\pilot.h
# End Source File
# Begin Source File

SOURCE=.\playerdata.h
# End Source File
# Begin Source File

SOURCE=.\playerterminal.h
# End Source File
# Begin Source File

SOURCE=.\playertype.h
# End Source File
# Begin Source File

SOURCE=.\powerplantgameobj.h
# End Source File
# Begin Source File

SOURCE=.\powerup.h
# End Source File
# Begin Source File

SOURCE=.\purchasesettings.h
# End Source File
# Begin Source File

SOURCE=.\radar.h
# End Source File
# Begin Source File

SOURCE=.\raveshawbossgameobj.h
# End Source File
# Begin Source File

SOURCE=.\refinerygameobj.h
# End Source File
# Begin Source File

SOURCE=.\repairbaygameobj.h
# End Source File
# Begin Source File

SOURCE=.\sakurabossgameobj.h
# End Source File
# Begin Source File

SOURCE=.\samsite.h
# End Source File
# Begin Source File

SOURCE=.\savegame.h
# End Source File
# Begin Source File

SOURCE=.\scexplosionevent.h
# End Source File
# Begin Source File

SOURCE=.\scobeliskevent.h
# End Source File
# Begin Source File

SOURCE=.\screenfademanager.h
# End Source File
# Begin Source File

SOURCE=.\scriptablegameobj.h
# End Source File
# Begin Source File

SOURCE=.\scriptcommands.h
# End Source File
# Begin Source File

SOURCE=.\scriptevents.h
# End Source File
# Begin Source File

SOURCE=.\scriptman.h
# End Source File
# Begin Source File

SOURCE=.\scriptzone.h
# End Source File
# Begin Source File

SOURCE=.\simplegameobj.h
# End Source File
# Begin Source File

SOURCE=.\smartgameobj.h
# End Source File
# Begin Source File

SOURCE=.\sniper.h
# End Source File
# Begin Source File

SOURCE=.\soldier.h
# End Source File
# Begin Source File

SOURCE=.\soldierfactorygameobj.h
# End Source File
# Begin Source File

SOURCE=.\soldierobserver.h
# End Source File
# Begin Source File

SOURCE=.\SoundEnvironment.h
# End Source File
# Begin Source File

SOURCE=.\spawn.h
# End Source File
# Begin Source File

SOURCE=.\specialbuilds.h
# End Source File
# Begin Source File

SOURCE=.\specialeffectsgameobj.h
# End Source File
# Begin Source File

SOURCE=.\statemachine.h
# End Source File
# Begin Source File

SOURCE=.\staticnetworkobject.h
# End Source File
# Begin Source File

SOURCE=.\surfaceeffects.h
# End Source File
# Begin Source File

SOURCE=.\systeminfolog.h
# End Source File
# Begin Source File

SOURCE=.\teampurchasesettings.h
# End Source File
# Begin Source File

SOURCE=.\textwindow.h
# End Source File
# Begin Source File

SOURCE=.\transition.h
# End Source File
# Begin Source File

SOURCE=.\transitiongameobj.h
# End Source File
# Begin Source File

SOURCE=.\unitcoordinationzonemgr.h
# End Source File
# Begin Source File

SOURCE=.\vehicle.h
# End Source File
# Begin Source File

SOURCE=.\vehicledriver.h
# End Source File
# Begin Source File

SOURCE=.\vehiclefactorygameobj.h
# End Source File
# Begin Source File

SOURCE=.\viseme.h
# End Source File
# Begin Source File

SOURCE=.\warfactorygameobj.h
# End Source File
# Begin Source File

SOURCE=.\weaponbag.h
# End Source File
# Begin Source File

SOURCE=.\weaponmanager.h
# End Source File
# Begin Source File

SOURCE=.\weapons.h
# End Source File
# Begin Source File

SOURCE=.\weaponview.h
# End Source File
# Begin Source File

SOURCE=.\WeatherMgr.h
# End Source File
# End Group
# Begin Group "Base Code"

# PROP Default_Filter ""
# Begin Group "Source  Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\assets.cpp
# End Source File
# Begin Source File

SOURCE=.\crandom.cpp
# End Source File
# Begin Source File

SOURCE=.\debug.cpp
# End Source File
# Begin Source File

SOURCE=.\directinput.cpp
# End Source File
# Begin Source File

SOURCE=.\timemgr.cpp
# End Source File
# End Group
# Begin Group "Header  Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\assets.h
# End Source File
# Begin Source File

SOURCE=.\crandom.h
# End Source File
# Begin Source File

SOURCE=.\debug.h
# End Source File
# Begin Source File

SOURCE=.\directinput.h
# End Source File
# Begin Source File

SOURCE=.\reflist.h
# End Source File
# Begin Source File

SOURCE=.\string_ids.h
# End Source File
# Begin Source File

SOURCE=.\timemgr.h
# End Source File
# End Group
# End Group
# End Target
# End Project
