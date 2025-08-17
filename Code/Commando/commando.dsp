# Microsoft Developer Studio Project File - Name="commando" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=COMMANDO - WIN32 RELEASE
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "commando.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "commando.mak" CFG="COMMANDO - WIN32 RELEASE"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "commando - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "commando - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "commando - Win32 Profile" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Commando/Code/Commando", LOGAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "commando - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "commando"
# PROP BASE Intermediate_Dir "commando"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /I "..\directx" /I "..\sr\h" /I "..\sr\devs.h" /I "..\sr\sr.h" /I "..\wwmath" /I "..\ww3d" /I "..\library" /I "..\glide" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "OBSIDIAN" /YX /FD /c
# ADD CPP /nologo /MTd /W4 /Gi /ZI /Od /I "..\DirectX\include" /I "..\\" /I "..\wwui" /I "..\wwbitpack" /I "..\combat" /I "..\wwaudio" /I "..\miles6\include" /I "..\wolapi" /I "..\wwutil" /I "..\wwlzhl" /I "..\wwnet" /I "..\wwmath" /I "..\ww3d2" /I "..\wwphys" /I "..\wwlib" /I "..\wwdebug" /I "..\wwsaveload" /I "..\wwtranslatedb" /I "..\BinkMovie" /I "..\scontrol" /I "..\GameSpy" /D "_DEBUG" /D "WWDEBUG" /D WINVER=0x400 /D "_WINDOWS" /D "DIRECTX" /D "WIN32" /Fr /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o /win32 "NUL"
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o /win32 "NUL"
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"Debug/renegade.bsc"
LINK32=link.exe
# ADD BASE LINK32 ddraw.lib wwmathd.lib ww3dd.lib libraryd.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib winmm.lib msdsrast.lib srvc4db.lib glide2x.lib /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 ..\directx\lib\d3dx8.lib dinput.lib wwui.lib wwaudio.lib ..\miles6\lib\win\mss32.lib wwsaveload.lib ww3d2.lib wwdebug.lib wwlib.lib wwmath.lib wwnet.lib wwphys.lib wwutil.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib ole32.lib oleaut32.lib uuid.lib winmm.lib dxguid.lib dsound.lib vfw32.lib wsock32.lib kernel32.lib shell32.lib combat.lib wwtranslatedb.lib version.lib binkmovie.lib binkw32.lib bandtest.lib scontrol.lib gamespy.lib /nologo /subsystem:windows /pdb:"..\..\Run\RenegadeD.pdb" /map:"..\..\Run\RenegadeD.map" /debug /machine:I386 /out:"..\..\Run\RenegadeD.exe" /libpath:"../libs/debug" /libpath:"../srsdk1x/msvc6\lib\debug" /libpath:"../directx\lib" /libpath:"../umbra/lib/win32-x86"
# SUBTRACT LINK32 /profile /pdb:none /incremental:no /nodefaultlib

!ELSEIF  "$(CFG)" == "commando - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "command0"
# PROP BASE Intermediate_Dir "command0"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /I "..\sr\h" /I "..\sr\devs.h" /I "..\sr\sr.h" /I "..\wwmath" /I "..\ww3d" /I "..\library" /I "..\glide" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "OBSIDIAN" /YX /FD /c
# ADD CPP /nologo /MT /W4 /Zi /O2 /Ob2 /I "..\wol" /I "..\network" /I "..\wwsound" /I "..\shell" /I "..\modes" /I "..\lan" /I "..\DirectX\include" /I "..\\" /I "..\wwui" /I "..\wwbitpack" /I "..\combat" /I "..\wwaudio" /I "..\miles6\include" /I "..\wolapi" /I "..\wwutil" /I "..\wwlzhl" /I "..\wwnet" /I "..\wwmath" /I "..\ww3d2" /I "..\wwphys" /I "..\wwlib" /I "..\wwdebug" /I "..\wwsaveload" /I "..\wwtranslatedb" /I "..\BinkMovie" /I "..\scontrol" /I "..\GameSpy" /D "NDEBUG" /D WINVER=0x400 /D "_WINDOWS" /D "DIRECTX" /D "WIN32" /Fr /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o /win32 "NUL"
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o /win32 "NUL"
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib winmm.lib ddraw.lib msdsrast.lib srvc4.lib library.lib wwmath.lib ww3d.lib glide2x.lib /nologo /subsystem:windows /machine:I386 /out:"..\..\run\commando_fx.exe"
# ADD LINK32 ..\DirectX\lib\d3dx8.lib dinput.lib wwui.lib wwaudio.lib ..\miles6\lib\win\mss32.lib wwsaveload.lib ww3d2.lib wwdebug.lib wwlib.lib wwmath.lib wwnet.lib wwphys.lib wwutil.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib ole32.lib oleaut32.lib uuid.lib winmm.lib dxguid.lib dsound.lib vfw32.lib wsock32.lib kernel32.lib shell32.lib combat.lib wwtranslatedb.lib version.lib binkmovie.lib binkw32.lib bandtest.lib scontrol.lib gamespy.lib /nologo /subsystem:windows /pdb:"..\..\Run\Renegade.pdb" /map:"..\..\Run\Renegade.map" /debug /machine:I386 /out:"..\..\Run\Renegade.exe" /libpath:"../libs/release" /libpath:"../srsdk1x/msvc6\lib\release" /libpath:"../directx\lib" /libpath:"../umbra/lib/win32-x86"
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=rebase -v -x ..\..\Run\. -a -b 400000 ..\..\Run\Renegade.exe
# End Special Build Tool

!ELSEIF  "$(CFG)" == "commando - Win32 Profile"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "commando"
# PROP BASE Intermediate_Dir "commando"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Profile"
# PROP Intermediate_Dir "Profile"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /I "..\sr\h" /I "..\sr\devs.h" /I "..\sr\sr.h" /I "..\glide" /I "..\wwmath" /I "..\ww3d" /I "..\wwphys" /I "..\library" /I "..\wwdebug" /I "..\directx\inc" /I "..\sr130\sr.h" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "DIRECTX" /YX /FD /c
# ADD CPP /nologo /MT /W4 /Zi /O2 /Op /Ob2 /I "..\wol" /I "..\network" /I "..\wwsound" /I "..\shell" /I "..\modes" /I "..\lan" /I "..\DirectX\include" /I "..\\" /I "..\wwui" /I "..\wwbitpack" /I "..\combat" /I "..\wwaudio" /I "..\miles6\include" /I "..\wolapi" /I "..\wwutil" /I "..\wwlzhl" /I "..\wwnet" /I "..\wwmath" /I "..\ww3d2" /I "..\wwphys" /I "..\wwlib" /I "..\wwdebug" /I "..\wwsaveload" /I "..\wwtranslatedb" /I "..\BinkMovie" /I "..\scontrol" /I "..\GameSpy" /D "NDEBUG" /D "WWDEBUG" /D WINVER=0x400 /D "_WINDOWS" /D "DIRECTX" /D "WIN32" /Fr /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o /win32 "NUL"
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o /win32 "NUL"
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 sr.lib wwmath.lib kernel32.lib wwphys.lib ww3d.lib library.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib winmm.lib ddraw.lib dinput.lib dxguid.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"libc.lib" /nodefaultlib:"LIBC" /out:"..\..\run\cmdo_r.exe" /libpath:"../libs/release" /libpath:"../sr130/release" /libpath:"../directx\lib"
# ADD LINK32 ..\DirectX\lib\d3dx8.lib dinput.lib wwui.lib wwaudio.lib ..\miles6\lib\win\mss32.lib wwsaveload.lib ww3d2.lib wwdebug.lib wwlib.lib wwmath.lib wwnet.lib wwphys.lib wwutil.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib ole32.lib oleaut32.lib uuid.lib winmm.lib dxguid.lib dsound.lib vfw32.lib wsock32.lib kernel32.lib shell32.lib combat.lib wwtranslatedb.lib version.lib binkmovie.lib binkw32.lib bandtest.lib scontrol.lib gamespy.lib /nologo /subsystem:windows /pdb:"..\..\Run\RenegadeP.pdb" /map:"..\..\Run\RenegadeP.map" /debug /machine:I386 /out:"..\..\Run\RenegadeP.exe" /libpath:"../libs/profile" /libpath:"../srsdk1x/msvc6/lib/release" /libpath:"../directx\lib" /libpath:"../umbra/lib/win32-x86"
# SUBTRACT LINK32 /profile /pdb:none

!ENDIF 

# Begin Target

# Name "commando - Win32 Debug"
# Name "commando - Win32 Release"
# Name "commando - Win32 Profile"
# Begin Group "Combat"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\boolean.h
# End Source File
# Begin Source File

SOURCE=.\combatgmode.cpp
# End Source File
# Begin Source File

SOURCE=.\combatgmode.h
# End Source File
# Begin Source File

SOURCE=.\commandochunkids.h
# End Source File
# Begin Source File

SOURCE=.\commandoids.h
# End Source File
# Begin Source File

SOURCE=.\commandosaveload.cpp
# End Source File
# Begin Source File

SOURCE=.\commandosaveload.h
# End Source File
# Begin Source File

SOURCE=.\comnetrcvinst.cpp
# End Source File
# Begin Source File

SOURCE=.\comnetrcvinst.h
# End Source File
# Begin Source File

SOURCE=.\gameinitmgr.cpp
# End Source File
# Begin Source File

SOURCE=.\gameinitmgr.h
# End Source File
# Begin Source File

SOURCE=.\god.cpp
# End Source File
# Begin Source File

SOURCE=.\god.h
# End Source File
# Begin Source File

SOURCE=.\inputconfig.cpp
# End Source File
# Begin Source File

SOURCE=.\inputconfig.h
# End Source File
# Begin Source File

SOURCE=.\inputconfigmgr.cpp
# End Source File
# Begin Source File

SOURCE=.\inputconfigmgr.h
# End Source File
# Begin Source File

SOURCE=.\lev_file.h
# End Source File
# Begin Source File

SOURCE=.\level.cpp
# End Source File
# Begin Source File

SOURCE=.\level.h
# End Source File
# Begin Source File

SOURCE=.\nethandler.cpp
# End Source File
# Begin Source File

SOURCE=.\nethandler.h
# End Source File
# Begin Source File

SOURCE=.\player.cpp
# End Source File
# Begin Source File

SOURCE=.\player.h
# End Source File
# Begin Source File

SOURCE=.\playermanager.cpp
# End Source File
# Begin Source File

SOURCE=.\playermanager.h
# End Source File
# Begin Source File

SOURCE=.\regbool.cpp
# End Source File
# Begin Source File

SOURCE=.\regbool.h
# End Source File
# Begin Source File

SOURCE=.\regfloat.cpp
# End Source File
# Begin Source File

SOURCE=.\regfloat.h
# End Source File
# Begin Source File

SOURCE=.\regint.cpp
# End Source File
# Begin Source File

SOURCE=.\regint.h
# End Source File
# Begin Source File

SOURCE=.\regstring.cpp
# End Source File
# Begin Source File

SOURCE=.\regstring.h
# End Source File
# Begin Source File

SOURCE=.\renegadeplayerterminal.cpp
# End Source File
# Begin Source File

SOURCE=.\renegadeplayerterminal.h
# End Source File
# Begin Source File

SOURCE=.\systemsettings.cpp
# End Source File
# Begin Source File

SOURCE=.\systemsettings.h
# End Source File
# Begin Source File

SOURCE=.\team.cpp
# End Source File
# Begin Source File

SOURCE=.\team.h
# End Source File
# Begin Source File

SOURCE=.\teammanager.cpp
# End Source File
# Begin Source File

SOURCE=.\teammanager.h
# End Source File
# End Group
# Begin Group "Modes"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\campaign.cpp
# End Source File
# Begin Source File

SOURCE=.\campaign.h
# End Source File
# Begin Source File

SOURCE=.\console.cpp
# End Source File
# Begin Source File

SOURCE=.\console.h
# End Source File
# Begin Source File

SOURCE=.\consolefunction.cpp
# End Source File
# Begin Source File

SOURCE=.\consolefunction.h
# End Source File
# Begin Source File

SOURCE=.\gamemenu.cpp
# End Source File
# Begin Source File

SOURCE=.\gamemenu.h
# End Source File
# Begin Source File

SOURCE=.\langmode.cpp
# End Source File
# Begin Source File

SOURCE=.\langmode.h
# End Source File
# Begin Source File

SOURCE=.\movie.cpp
# End Source File
# Begin Source File

SOURCE=.\movie.h
# End Source File
# Begin Source File

SOURCE=.\overlay.cpp
# End Source File
# Begin Source File

SOURCE=.\overlay.h
# End Source File
# Begin Source File

SOURCE=.\scorescreen.cpp
# End Source File
# Begin Source File

SOURCE=.\scorescreen.h
# End Source File
# Begin Source File

SOURCE=.\textdisplay.cpp
# End Source File
# Begin Source File

SOURCE=.\textdisplay.h
# End Source File
# Begin Source File

SOURCE=.\wolgmode.cpp
# End Source File
# Begin Source File

SOURCE=.\wolgmode.h
# End Source File
# End Group
# Begin Group "Shell"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\_globals.h
# End Source File
# Begin Source File

SOURCE=.\afxres.h
# End Source File
# Begin Source File

SOURCE=.\AutoStart.cpp
# End Source File
# Begin Source File

SOURCE=.\AutoStart.h
# End Source File
# Begin Source File

SOURCE=.\buildnum.cpp
# End Source File
# Begin Source File

SOURCE=.\buildnum.h
# End Source File
# Begin Source File

SOURCE=.\cdverify.cpp
# End Source File
# Begin Source File

SOURCE=.\cdverify.h
# End Source File
# Begin Source File

SOURCE=.\cominit.cpp
# End Source File
# Begin Source File

SOURCE=.\cominit.h
# End Source File
# Begin Source File

SOURCE=.\ConsoleMode.cpp
# End Source File
# Begin Source File

SOURCE=.\ConsoleMode.h
# End Source File
# Begin Source File

SOURCE=.\gamemode.cpp
# End Source File
# Begin Source File

SOURCE=.\gamemode.h
# End Source File
# Begin Source File

SOURCE=.\init.cpp
# End Source File
# Begin Source File

SOURCE=.\init.h
# End Source File
# Begin Source File

SOURCE=.\mainloop.cpp
# End Source File
# Begin Source File

SOURCE=.\mainloop.h
# End Source File
# Begin Source File

SOURCE=.\shutdown.cpp
# End Source File
# Begin Source File

SOURCE=.\shutdown.h
# End Source File
# Begin Source File

SOURCE=.\singletoninstancekeeper.cpp
# End Source File
# Begin Source File

SOURCE=.\singletoninstancekeeper.h
# End Source File
# Begin Source File

SOURCE=.\WINMAIN.CPP
# End Source File
# Begin Source File

SOURCE=.\WINMAIN.H
# End Source File
# End Group
# Begin Group "Shortcuts"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Run\_asserts.txt
# End Source File
# Begin Source File

SOURCE=..\..\Run\_except.txt
# End Source File
# Begin Source File

SOURCE=..\..\run\_logfile.txt
# End Source File
# Begin Source File

SOURCE=..\..\..\RenegadeMe\Doc\cncwishlist.txt
# End Source File
# Begin Source File

SOURCE=..\..\..\RenegadeMe\Doc\currentbugs.txt
# End Source File
# Begin Source File

SOURCE=..\..\..\RenegadeMe\Doc\joinprocess.txt
# End Source File
# Begin Source File

SOURCE=.\junkcode.txt
# End Source File
# Begin Source File

SOURCE=..\..\..\RenegadeMe\Doc\priorities.txt
# End Source File
# Begin Source File

SOURCE=..\..\Run\results.txt
# End Source File
# Begin Source File

SOURCE=..\..\..\RenegadeMe\Doc\todo.txt
# End Source File
# End Group
# Begin Group "Wol"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\BandwidthCheck.cpp
# End Source File
# Begin Source File

SOURCE=.\bandwidthcheck.h
# End Source File
# Begin Source File

SOURCE=.\FirewallWait.cpp
# End Source File
# Begin Source File

SOURCE=.\FirewallWait.h
# End Source File
# Begin Source File

SOURCE=.\GameResSend.cpp
# End Source File
# Begin Source File

SOURCE=.\GameResSend.h
# End Source File
# Begin Source File

SOURCE=.\mpsettingsmgr.cpp
# End Source File
# Begin Source File

SOURCE=.\mpsettingsmgr.h
# End Source File
# Begin Source File

SOURCE=.\WOLBuddyMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\WOLBuddyMgr.h
# End Source File
# Begin Source File

SOURCE=.\WOLChatMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\WOLChatMgr.h
# End Source File
# Begin Source File

SOURCE=.\WOLDiags.cpp
# End Source File
# Begin Source File

SOURCE=.\WOLDiags.h
# End Source File
# Begin Source File

SOURCE=.\WOLGameInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\WOLGameInfo.h
# End Source File
# Begin Source File

SOURCE=.\WOLJoinGame.cpp
# End Source File
# Begin Source File

SOURCE=.\WOLJoinGame.h
# End Source File
# Begin Source File

SOURCE=.\WOLLocaleMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\wollocalemgr.h
# End Source File
# Begin Source File

SOURCE=.\WOLLoginProfile.cpp
# End Source File
# Begin Source File

SOURCE=.\WOLLoginProfile.h
# End Source File
# Begin Source File

SOURCE=.\WOLLogonMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\WOLLogonMgr.h
# End Source File
# Begin Source File

SOURCE=.\WOLQuickMatch.cpp
# End Source File
# Begin Source File

SOURCE=.\WOLQuickMatch.h
# End Source File
# End Group
# Begin Group "Lan&Wol"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\chatshre.cpp
# End Source File
# Begin Source File

SOURCE=.\chatshre.h
# End Source File
# Begin Source File

SOURCE=.\gamechanlist.cpp
# End Source File
# Begin Source File

SOURCE=.\gamechanlist.h
# End Source File
# Begin Source File

SOURCE=.\gamechannel.cpp
# End Source File
# Begin Source File

SOURCE=.\gamechannel.h
# End Source File
# Begin Source File

SOURCE=.\gamesideservercontrol.cpp

!IF  "$(CFG)" == "commando - Win32 Debug"

# ADD CPP /FR

!ELSEIF  "$(CFG)" == "commando - Win32 Release"

!ELSEIF  "$(CFG)" == "commando - Win32 Profile"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gamesideservercontrol.h
# End Source File
# Begin Source File

SOURCE=.\netinterface.cpp
# End Source File
# Begin Source File

SOURCE=.\netinterface.h
# End Source File
# Begin Source File

SOURCE=.\ServerSettings.cpp
# End Source File
# Begin Source File

SOURCE=.\ServerSettings.h
# End Source File
# Begin Source File

SOURCE=.\slavemaster.cpp
# End Source File
# Begin Source File

SOURCE=.\slavemaster.h
# End Source File
# End Group
# Begin Group "Network"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\apppacketstats.cpp
# End Source File
# Begin Source File

SOURCE=.\apppacketstats.h
# End Source File
# Begin Source File

SOURCE=.\bandwidth.cpp
# End Source File
# Begin Source File

SOURCE=.\bandwidth.h
# End Source File
# Begin Source File

SOURCE=.\bandwidthgraph.cpp
# End Source File
# Begin Source File

SOURCE=.\bandwidthgraph.h
# End Source File
# Begin Source File

SOURCE=.\clienthintmanager.cpp
# End Source File
# Begin Source File

SOURCE=.\clienthintmanager.h
# End Source File
# Begin Source File

SOURCE=.\clientpingmanager.cpp
# End Source File
# Begin Source File

SOURCE=.\clientpingmanager.h
# End Source File
# Begin Source File

SOURCE=.\cnetwork.cpp
# End Source File
# Begin Source File

SOURCE=.\cnetwork.h
# End Source File
# Begin Source File

SOURCE=.\comnetrcv.h
# End Source File
# Begin Source File

SOURCE=.\datasafe.cpp

!IF  "$(CFG)" == "commando - Win32 Debug"

!ELSEIF  "$(CFG)" == "commando - Win32 Release"

# ADD CPP /Gi- /O2 /Ob2 /FAcs
# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "commando - Win32 Profile"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\datasafe.h
# End Source File
# Begin Source File

SOURCE=.\demosupport.h
# End Source File
# Begin Source File

SOURCE=.\devoptions.cpp
# End Source File
# Begin Source File

SOURCE=.\devoptions.h
# End Source File
# Begin Source File

SOURCE=.\diagnostics.cpp
# End Source File
# Begin Source File

SOURCE=.\diagnostics.h
# End Source File
# Begin Source File

SOURCE=.\messages.cpp
# End Source File
# Begin Source File

SOURCE=.\multihud.cpp
# End Source File
# Begin Source File

SOURCE=.\multihud.h
# End Source File
# Begin Source File

SOURCE=.\NAT.cpp
# End Source File
# Begin Source File

SOURCE=.\nataddr.cpp
# End Source File
# Begin Source File

SOURCE=.\natsock.cpp
# End Source File
# Begin Source File

SOURCE=.\natter.cpp
# End Source File
# Begin Source File

SOURCE=.\netgraphs.cpp
# End Source File
# Begin Source File

SOURCE=.\nicenum.cpp
# End Source File
# Begin Source File

SOURCE=.\nicenum.h
# End Source File
# Begin Source File

SOURCE=.\pkthandlers.cpp
# End Source File
# Begin Source File

SOURCE=.\priority.cpp
# End Source File
# Begin Source File

SOURCE=.\priority.h
# End Source File
# Begin Source File

SOURCE=.\sbbomanager.cpp
# End Source File
# Begin Source File

SOURCE=.\sbbomanager.h
# End Source File
# Begin Source File

SOURCE=.\useroptions.cpp
# End Source File
# Begin Source File

SOURCE=.\useroptions.h
# End Source File
# Begin Source File

SOURCE=.\vendor.cpp
# End Source File
# Begin Source File

SOURCE=.\vendor.h
# End Source File
# End Group
# Begin Group "Wolapi"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\wolapi\chatdefs.h
# End Source File
# Begin Source File

SOURCE=..\wolapi\downloaddefs.h
# End Source File
# Begin Source File

SOURCE=..\wolapi\ftpdefs.h
# End Source File
# Begin Source File

SOURCE=..\wolapi\netutildefs.h
# End Source File
# Begin Source File

SOURCE=..\wolapi\WOLAPI.h
# End Source File
# Begin Source File

SOURCE=..\wolapi\WOLAPI_i.c
# End Source File
# End Group
# Begin Group "GameModes"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\gamedata.cpp
# End Source File
# Begin Source File

SOURCE=.\gamedata.h
# End Source File
# Begin Source File

SOURCE=.\gdcnc.cpp
# End Source File
# Begin Source File

SOURCE=.\gdcnc.h
# End Source File
# Begin Source File

SOURCE=.\gdsingleplayer.cpp
# End Source File
# Begin Source File

SOURCE=.\gdsingleplayer.h
# End Source File
# Begin Source File

SOURCE=.\gdskirmish.cpp
# End Source File
# Begin Source File

SOURCE=.\gdskirmish.h
# End Source File
# End Group
# Begin Group "Lan"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\lanchat.cpp
# End Source File
# Begin Source File

SOURCE=.\lanchat.h
# End Source File
# End Group
# Begin Group "GUI"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\chat.rc
# End Source File
# Begin Source File

SOURCE=.\dialogresource.h
# End Source File
# Begin Source File

SOURCE=.\dialogtests.cpp
# End Source File
# Begin Source File

SOURCE=.\dialogtests.h
# End Source File
# Begin Source File

SOURCE=.\dlgcharacteroptions.cpp
# End Source File
# Begin Source File

SOURCE=.\dlgcharacteroptions.h
# End Source File
# Begin Source File

SOURCE=.\dlgcheatoptions.cpp
# End Source File
# Begin Source File

SOURCE=.\dlgcheatoptions.h
# End Source File
# Begin Source File

SOURCE=.\dlgcncbattleinfo.cpp
# End Source File
# Begin Source File

SOURCE=.\dlgcncbattleinfo.h
# End Source File
# Begin Source File

SOURCE=.\dlgcncpurchasemainmenu.cpp
# End Source File
# Begin Source File

SOURCE=.\dlgcncpurchasemainmenu.h
# End Source File
# Begin Source File

SOURCE=.\dlgcncpurchasemenu.cpp
# End Source File
# Begin Source File

SOURCE=.\dlgcncpurchasemenu.h
# End Source File
# Begin Source File

SOURCE=.\dlgcncreference.cpp
# End Source File
# Begin Source File

SOURCE=.\dlgcncreference.h
# End Source File
# Begin Source File

SOURCE=.\dlgcncserverinfo.cpp
# End Source File
# Begin Source File

SOURCE=.\dlgcncserverinfo.h
# End Source File
# Begin Source File

SOURCE=.\dlgcncteaminfo.cpp
# End Source File
# Begin Source File

SOURCE=.\dlgcncteaminfo.h
# End Source File
# Begin Source File

SOURCE=.\dlgcncwinscreen.cpp
# End Source File
# Begin Source File

SOURCE=.\dlgcncwinscreen.h
# End Source File
# Begin Source File

SOURCE=.\dlgconfigaudiotab.cpp
# End Source File
# Begin Source File

SOURCE=.\dlgconfigaudiotab.h
# End Source File
# Begin Source File

SOURCE=.\dlgconfigperformancetab.cpp
# End Source File
# Begin Source File

SOURCE=.\dlgconfigperformancetab.h
# End Source File
# Begin Source File

SOURCE=.\dlgconfigvideotab.cpp
# End Source File
# Begin Source File

SOURCE=.\dlgconfigvideotab.h
# End Source File
# Begin Source File

SOURCE=.\dlgcontrols.cpp
# End Source File
# Begin Source File

SOURCE=.\dlgcontrols.h
# End Source File
# Begin Source File

SOURCE=.\dlgcontrolsaveload.cpp
# End Source File
# Begin Source File

SOURCE=.\dlgcontrolsaveload.h
# End Source File
# Begin Source File

SOURCE=.\dlgcontrolslisttab.cpp
# End Source File
# Begin Source File

SOURCE=.\dlgcontrolslisttab.h
# End Source File
# Begin Source File

SOURCE=.\dlgcontroltabs.cpp
# End Source File
# Begin Source File

SOURCE=.\dlgcontroltabs.h
# End Source File
# Begin Source File

SOURCE=.\dlgcredits.cpp
# End Source File
# Begin Source File

SOURCE=.\dlgcredits.h
# End Source File
# Begin Source File

SOURCE=.\DlgDownload.cpp
# End Source File
# Begin Source File

SOURCE=.\DlgDownload.h
# End Source File
# Begin Source File

SOURCE=.\dlgevabuildingstab.cpp
# End Source File
# Begin Source File

SOURCE=.\dlgevabuildingstab.h
# End Source File
# Begin Source File

SOURCE=.\dlgevacharacterstab.cpp
# End Source File
# Begin Source File

SOURCE=.\dlgevacharacterstab.h
# End Source File
# Begin Source File

SOURCE=.\dlgevadatatab.cpp
# End Source File
# Begin Source File

SOURCE=.\dlgevadatatab.h
# End Source File
# Begin Source File

SOURCE=.\dlgevaencyclopedia.cpp
# End Source File
# Begin Source File

SOURCE=.\dlgevaencyclopedia.h
# End Source File
# Begin Source File

SOURCE=.\dlgevamaptab.cpp
# End Source File
# Begin Source File

SOURCE=.\dlgevamaptab.h
# End Source File
# Begin Source File

SOURCE=.\dlgevaobjectivestab.cpp
# End Source File
# Begin Source File

SOURCE=.\dlgevaobjectivestab.h
# End Source File
# Begin Source File

SOURCE=.\dlgevavehiclestab.cpp
# End Source File
# Begin Source File

SOURCE=.\dlgevavehiclestab.h
# End Source File
# Begin Source File

SOURCE=.\dlgevaviewertab.cpp
# End Source File
# Begin Source File

SOURCE=.\dlgevaviewertab.h
# End Source File
# Begin Source File

SOURCE=.\dlgevaweaponstab.cpp
# End Source File
# Begin Source File

SOURCE=.\dlgevaweaponstab.h
# End Source File
# Begin Source File

SOURCE=.\dlghelpscreen.cpp
# End Source File
# Begin Source File

SOURCE=.\dlghelpscreen.h
# End Source File
# Begin Source File

SOURCE=.\dlgloadspgame.cpp
# End Source File
# Begin Source File

SOURCE=.\dlgloadspgame.h
# End Source File
# Begin Source File

SOURCE=.\dlgmainmenu.cpp
# End Source File
# Begin Source File

SOURCE=.\dlgmainmenu.h
# End Source File
# Begin Source File

SOURCE=.\DlgMessageBox.cpp
# End Source File
# Begin Source File

SOURCE=.\DlgMessageBox.h
# End Source File
# Begin Source File

SOURCE=.\dlgmovieoptions.cpp
# End Source File
# Begin Source File

SOURCE=.\dlgmovieoptions.h
# End Source File
# Begin Source File

SOURCE=.\dlgmpchangelannickname.cpp
# End Source File
# Begin Source File

SOURCE=.\dlgmpchangelannickname.h
# End Source File
# Begin Source File

SOURCE=.\DlgMPConnect.cpp
# End Source File
# Begin Source File

SOURCE=.\DlgMPConnect.h
# End Source File
# Begin Source File

SOURCE=.\DlgMPConnectionRefused.cpp
# End Source File
# Begin Source File

SOURCE=.\DlgMPConnectionRefused.h
# End Source File
# Begin Source File

SOURCE=.\dlgmpingamechat.cpp
# End Source File
# Begin Source File

SOURCE=.\dlgmpingamechat.h
# End Source File
# Begin Source File

SOURCE=.\dlgmpingameteamchat.cpp
# End Source File
# Begin Source File

SOURCE=.\dlgmpingameteamchat.h
# End Source File
# Begin Source File

SOURCE=.\dlgmplangamelist.cpp
# End Source File
# Begin Source File

SOURCE=.\dlgmplangamelist.h
# End Source File
# Begin Source File

SOURCE=.\dlgmplanhostoptions.cpp
# End Source File
# Begin Source File

SOURCE=.\dlgmplanhostoptions.h
# End Source File
# Begin Source File

SOURCE=.\dlgmpslaveservers.cpp
# End Source File
# Begin Source File

SOURCE=.\DlgMPTeamSelect.cpp
# End Source File
# Begin Source File

SOURCE=.\DlgMPTeamSelect.h
# End Source File
# Begin Source File

SOURCE=.\dlgmpwoladdbuddy.cpp
# End Source File
# Begin Source File

SOURCE=.\dlgmpwoladdbuddy.h
# End Source File
# Begin Source File

SOURCE=.\dlgmpwoladdignoreentry.cpp
# End Source File
# Begin Source File

SOURCE=.\dlgmpwoladdignoreentry.h
# End Source File
# Begin Source File

SOURCE=.\dlgmpwolautologinprompt.cpp
# End Source File
# Begin Source File

SOURCE=.\dlgmpwolautologinprompt.h
# End Source File
# Begin Source File

SOURCE=.\dlgmpwolbuddies.cpp
# End Source File
# Begin Source File

SOURCE=.\dlgmpwolbuddies.h
# End Source File
# Begin Source File

SOURCE=.\dlgmpwolbuddylistpopup.cpp
# End Source File
# Begin Source File

SOURCE=.\dlgmpwolbuddylistpopup.h
# End Source File
# Begin Source File

SOURCE=.\dlgmpwolchat.cpp
# End Source File
# Begin Source File

SOURCE=.\dlgmpwolchat.h
# End Source File
# Begin Source File

SOURCE=.\dlgmpwoldeletebuddy.cpp
# End Source File
# Begin Source File

SOURCE=.\dlgmpwoldeletebuddy.h
# End Source File
# Begin Source File

SOURCE=.\dlgmpwoldeleteignoreentry.cpp
# End Source File
# Begin Source File

SOURCE=.\dlgmpwoldeleteignoreentry.h
# End Source File
# Begin Source File

SOURCE=.\dlgmpwolgamelist.cpp
# End Source File
# Begin Source File

SOURCE=.\dlgmpwolgamelist.h
# End Source File
# Begin Source File

SOURCE=.\dlgmpwolgoodies.cpp
# End Source File
# Begin Source File

SOURCE=.\dlgmpwolgoodies.h
# End Source File
# Begin Source File

SOURCE=.\dlgmpwolignorelist.cpp
# End Source File
# Begin Source File

SOURCE=.\dlgmpwolignorelist.h
# End Source File
# Begin Source File

SOURCE=.\dlgmpwolinvitation.cpp
# End Source File
# Begin Source File

SOURCE=.\dlgmpwolinvitation.h
# End Source File
# Begin Source File

SOURCE=.\dlgmpwollocalenag.cpp
# End Source File
# Begin Source File

SOURCE=.\dlgmpwollocalenag.h
# End Source File
# Begin Source File

SOURCE=.\dlgmpwolmain.cpp
# End Source File
# Begin Source File

SOURCE=.\dlgmpwolmain.h
# End Source File
# Begin Source File

SOURCE=.\dlgmpwolmotd.cpp
# End Source File
# Begin Source File

SOURCE=.\dlgmpwolmotd.h
# End Source File
# Begin Source File

SOURCE=.\dlgmpwolpagebuddy.cpp
# End Source File
# Begin Source File

SOURCE=.\dlgmpwolpagebuddy.h
# End Source File
# Begin Source File

SOURCE=.\dlgmpwolpagereply.cpp
# End Source File
# Begin Source File

SOURCE=.\dlgmpwolpagereply.h
# End Source File
# Begin Source File

SOURCE=.\dlgmpwolquickmatchoptions.cpp
# End Source File
# Begin Source File

SOURCE=.\dlgmpwolquickmatchoptions.h
# End Source File
# Begin Source File

SOURCE=.\dlgmultiplayoptions.cpp
# End Source File
# Begin Source File

SOURCE=.\dlgmultiplayoptions.h
# End Source File
# Begin Source File

SOURCE=.\DlgPasswordPrompt.cpp
# End Source File
# Begin Source File

SOURCE=.\DlgPasswordPrompt.h
# End Source File
# Begin Source File

SOURCE=.\dlgpreviewoptions.cpp
# End Source File
# Begin Source File

SOURCE=.\dlgpreviewoptions.h
# End Source File
# Begin Source File

SOURCE=.\DlgQuickmatch.cpp
# End Source File
# Begin Source File

SOURCE=.\DlgQuickmatch.h
# End Source File
# Begin Source File

SOURCE=.\DlgRestart.cpp
# End Source File
# Begin Source File

SOURCE=.\DlgRestart.h
# End Source File
# Begin Source File

SOURCE=.\dlgsavegame.cpp
# End Source File
# Begin Source File

SOURCE=.\dlgsavegame.h
# End Source File
# Begin Source File

SOURCE=.\dlgserversaveload.cpp
# End Source File
# Begin Source File

SOURCE=.\dlgserversaveload.h
# End Source File
# Begin Source File

SOURCE=.\dlgsidebarhelp.cpp
# End Source File
# Begin Source File

SOURCE=.\dlgsidebarhelp.h
# End Source File
# Begin Source File

SOURCE=.\dlgtechoptions.cpp
# End Source File
# Begin Source File

SOURCE=.\dlgtechoptions.h
# End Source File
# Begin Source File

SOURCE=.\DlgWebPage.cpp
# End Source File
# Begin Source File

SOURCE=.\DlgWebPage.h
# End Source File
# Begin Source File

SOURCE=.\DlgWOLAutoStart.cpp
# End Source File
# Begin Source File

SOURCE=.\DlgWOLAutoStart.h
# End Source File
# Begin Source File

SOURCE=.\DlgWOLLogon.cpp
# End Source File
# Begin Source File

SOURCE=.\DlgWOLLogon.h
# End Source File
# Begin Source File

SOURCE=.\DlgWOLSettings.cpp
# End Source File
# Begin Source File

SOURCE=.\DlgWOLSettings.h
# End Source File
# Begin Source File

SOURCE=.\DlgWOLWait.cpp
# End Source File
# Begin Source File

SOURCE=.\DlgWOLWait.h
# End Source File
# Begin Source File

SOURCE=.\mainmenutransition.cpp
# End Source File
# Begin Source File

SOURCE=.\mainmenutransition.h
# End Source File
# Begin Source File

SOURCE=.\radiocommanddisplay.cpp
# End Source File
# Begin Source File

SOURCE=.\radiocommanddisplay.h
# End Source File
# Begin Source File

SOURCE=.\renegadedialogmgr.cpp
# End Source File
# Begin Source File

SOURCE=.\renegadedialogmgr.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\skinpackage.cpp
# End Source File
# Begin Source File

SOURCE=.\skinpackage.h
# End Source File
# Begin Source File

SOURCE=.\skinpackagemgr.cpp
# End Source File
# Begin Source File

SOURCE=.\skinpackagemgr.h
# End Source File
# Begin Source File

SOURCE=.\WebBrowser.cpp
# End Source File
# Begin Source File

SOURCE=.\WebBrowser.h
# End Source File
# End Group
# Begin Group "NetEvents"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\AnnounceEvent.cpp
# End Source File
# Begin Source File

SOURCE=.\AnnounceEvent.h
# End Source File
# Begin Source File

SOURCE=.\bioevent.cpp
# End Source File
# Begin Source File

SOURCE=.\bioevent.h
# End Source File
# Begin Source File

SOURCE=.\changeteamevent.cpp
# End Source File
# Begin Source File

SOURCE=.\changeteamevent.h
# End Source File
# Begin Source File

SOURCE=.\clientbboevent.cpp
# End Source File
# Begin Source File

SOURCE=.\clientbboevent.h
# End Source File
# Begin Source File

SOURCE=.\clientgoodbyeevent.cpp
# End Source File
# Begin Source File

SOURCE=.\clientgoodbyeevent.h
# End Source File
# Begin Source File

SOURCE=.\consolecommandevent.cpp
# End Source File
# Begin Source File

SOURCE=.\consolecommandevent.h
# End Source File
# Begin Source File

SOURCE=.\csconsolecommandevent.cpp
# End Source File
# Begin Source File

SOURCE=.\csconsolecommandevent.h
# End Source File
# Begin Source File

SOURCE=.\cshint.cpp
# End Source File
# Begin Source File

SOURCE=.\cshint.h
# End Source File
# Begin Source File

SOURCE=.\cspingrequestevent.cpp
# End Source File
# Begin Source File

SOURCE=.\cspingrequestevent.h
# End Source File
# Begin Source File

SOURCE=.\cstextobj.cpp
# End Source File
# Begin Source File

SOURCE=.\cstextobj.h
# End Source File
# Begin Source File

SOURCE=.\donateevent.cpp
# End Source File
# Begin Source File

SOURCE=.\donateevent.h
# End Source File
# Begin Source File

SOURCE=.\evictionevent.cpp
# End Source File
# Begin Source File

SOURCE=.\evictionevent.h
# End Source File
# Begin Source File

SOURCE=.\gamedataupdateevent.cpp
# End Source File
# Begin Source File

SOURCE=.\gamedataupdateevent.h
# End Source File
# Begin Source File

SOURCE=.\gameoptionsevent.cpp
# End Source File
# Begin Source File

SOURCE=.\gameoptionsevent.h
# End Source File
# Begin Source File

SOURCE=.\gamespycschallengeresponseevent.cpp
# End Source File
# Begin Source File

SOURCE=.\gamespycschallengeresponseevent.h
# End Source File
# Begin Source File

SOURCE=.\gamespyscchallengeevent.cpp
# End Source File
# Begin Source File

SOURCE=.\gamespyscchallengeevent.h
# End Source File
# Begin Source File

SOURCE=.\godmodeevent.cpp
# End Source File
# Begin Source File

SOURCE=.\godmodeevent.h
# End Source File
# Begin Source File

SOURCE=.\loadingevent.cpp
# End Source File
# Begin Source File

SOURCE=.\loadingevent.h
# End Source File
# Begin Source File

SOURCE=.\moneyevent.cpp
# End Source File
# Begin Source File

SOURCE=.\moneyevent.h
# End Source File
# Begin Source File

SOURCE=.\netevent.cpp
# End Source File
# Begin Source File

SOURCE=.\netevent.h
# End Source File
# Begin Source File

SOURCE=.\playerkill.cpp
# End Source File
# Begin Source File

SOURCE=.\playerkill.h
# End Source File
# Begin Source File

SOURCE=.\purchaserequestevent.cpp
# End Source File
# Begin Source File

SOURCE=.\purchaserequestevent.h
# End Source File
# Begin Source File

SOURCE=.\purchaseresponseevent.cpp
# End Source File
# Begin Source File

SOURCE=.\purchaseresponseevent.h
# End Source File
# Begin Source File

SOURCE=.\requestkillevent.cpp
# End Source File
# Begin Source File

SOURCE=.\requestkillevent.h
# End Source File
# Begin Source File

SOURCE=.\scoreevent.cpp
# End Source File
# Begin Source File

SOURCE=.\scoreevent.h
# End Source File
# Begin Source File

SOURCE=.\scpingresponseevent.cpp
# End Source File
# Begin Source File

SOURCE=.\scpingresponseevent.h
# End Source File
# Begin Source File

SOURCE=.\sctextobj.cpp
# End Source File
# Begin Source File

SOURCE=.\sctextobj.h
# End Source File
# Begin Source File

SOURCE=.\suicideevent.cpp
# End Source File
# Begin Source File

SOURCE=.\suicideevent.h
# End Source File
# Begin Source File

SOURCE=.\svrgoodbyeevent.cpp
# End Source File
# Begin Source File

SOURCE=.\svrgoodbyeevent.h
# End Source File
# Begin Source File

SOURCE=.\vipmodeevent.cpp
# End Source File
# Begin Source File

SOURCE=.\vipmodeevent.h
# End Source File
# Begin Source File

SOURCE=.\warpevent.cpp
# End Source File
# Begin Source File

SOURCE=.\warpevent.h
# End Source File
# Begin Source File

SOURCE=.\winevent.cpp
# End Source File
# Begin Source File

SOURCE=.\winevent.h
# End Source File
# End Group
# Begin Group "WOLBrowser"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\WOLBrowser\WOLBrowser.h
# End Source File
# Begin Source File

SOURCE=..\WOLBrowser\WOLBrowser_i.c
# End Source File
# End Group
# Begin Group "WWOnline"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\WWOnline\GameResField.cpp
# End Source File
# Begin Source File

SOURCE=..\WWOnline\GameResField.h
# End Source File
# Begin Source File

SOURCE=..\WWOnline\GameResPacket.cpp
# End Source File
# Begin Source File

SOURCE=..\WWOnline\GameResPacket.h
# End Source File
# Begin Source File

SOURCE=..\WWOnline\PingProfile.cpp
# End Source File
# Begin Source File

SOURCE=..\WWOnline\PingProfile.h
# End Source File
# Begin Source File

SOURCE=..\WWOnline\RefCounted.h
# End Source File
# Begin Source File

SOURCE=..\WWOnline\RefPtr.h
# End Source File
# Begin Source File

SOURCE=..\WWOnline\WaitCondition.cpp
# End Source File
# Begin Source File

SOURCE=..\WWOnline\WaitCondition.h
# End Source File
# Begin Source File

SOURCE=..\WWOnline\WOLAgeCheck.h
# End Source File
# Begin Source File

SOURCE=..\WWOnline\WOLChannel.cpp
# End Source File
# Begin Source File

SOURCE=..\WWOnline\WOLChannel.h
# End Source File
# Begin Source File

SOURCE=..\WWOnline\WOLChatMsg.cpp
# End Source File
# Begin Source File

SOURCE=..\WWOnline\WOLChatMsg.h
# End Source File
# Begin Source File

SOURCE=..\WWOnline\WOLChatObserver.cpp
# End Source File
# Begin Source File

SOURCE=..\WWOnline\WOLChatObserver.h
# End Source File
# Begin Source File

SOURCE=..\WWOnline\WOLConnect.cpp
# End Source File
# Begin Source File

SOURCE=..\WWOnline\WOLConnect.h
# End Source File
# Begin Source File

SOURCE=..\WWOnline\WOLDownload.cpp
# End Source File
# Begin Source File

SOURCE=..\WWOnline\WOLDownload.h
# End Source File
# Begin Source File

SOURCE=..\WWOnline\WOLErrorUtil.cpp
# End Source File
# Begin Source File

SOURCE=..\WWOnline\WOLErrorUtil.h
# End Source File
# Begin Source File

SOURCE=..\WWOnline\WOLGame.cpp
# End Source File
# Begin Source File

SOURCE=..\WWOnline\WOLGame.h
# End Source File
# Begin Source File

SOURCE=..\WWOnline\WOLGameOptions.cpp
# End Source File
# Begin Source File

SOURCE=..\WWOnline\WOLGameOptions.h
# End Source File
# Begin Source File

SOURCE=..\WWOnline\WOLLadder.cpp
# End Source File
# Begin Source File

SOURCE=..\WWOnline\WOLLadder.h
# End Source File
# Begin Source File

SOURCE=..\WWOnline\WOLLangCodes.h
# End Source File
# Begin Source File

SOURCE=..\WWOnline\WOLLoginInfo.cpp
# End Source File
# Begin Source File

SOURCE=..\WWOnline\WOLLoginInfo.h
# End Source File
# Begin Source File

SOURCE=..\WWOnline\WOLNetUtilObserver.cpp
# End Source File
# Begin Source File

SOURCE=..\WWOnline\WOLNetUtilObserver.h
# End Source File
# Begin Source File

SOURCE=..\WWOnline\WOLPageMsg.cpp
# End Source File
# Begin Source File

SOURCE=..\WWOnline\WOLPageMsg.h
# End Source File
# Begin Source File

SOURCE=..\WWOnline\WOLPing.h
# End Source File
# Begin Source File

SOURCE=..\WWOnline\WOLProduct.cpp
# End Source File
# Begin Source File

SOURCE=..\WWOnline\WOLProduct.h
# End Source File
# Begin Source File

SOURCE=..\WWOnline\WOLServer.cpp
# End Source File
# Begin Source File

SOURCE=..\WWOnline\WOLServer.h
# End Source File
# Begin Source File

SOURCE=..\WWOnline\WOLSession.cpp
# End Source File
# Begin Source File

SOURCE=..\WWOnline\WOLSession.h
# End Source File
# Begin Source File

SOURCE=..\WWOnline\WOLSquad.cpp
# End Source File
# Begin Source File

SOURCE=..\WWOnline\WOLSquad.h
# End Source File
# Begin Source File

SOURCE=..\WWOnline\WOLString.cpp
# End Source File
# Begin Source File

SOURCE=..\WWOnline\WOLString.h
# End Source File
# Begin Source File

SOURCE=..\WWOnline\WOLUser.cpp
# End Source File
# Begin Source File

SOURCE=..\WWOnline\WOLUser.h
# End Source File
# End Group
# Begin Group "NetObjects"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\clientfps.cpp
# End Source File
# Begin Source File

SOURCE=.\clientfps.h
# End Source File
# Begin Source File

SOURCE=.\serverfps.cpp
# End Source File
# Begin Source File

SOURCE=.\serverfps.h
# End Source File
# End Group
# Begin Group "Unclaimed"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\renegadecheatmgr.cpp
# End Source File
# Begin Source File

SOURCE=.\renegadecheatmgr.h
# End Source File
# End Group
# Begin Group "GameSpy"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CDKeyAuth.cpp
# End Source File
# Begin Source File

SOURCE=.\CDKeyAuth.h
# End Source File
# Begin Source File

SOURCE=.\GameSpy_QnR.cpp
# End Source File
# Begin Source File

SOURCE=.\GameSpy_QnR.h
# End Source File
# Begin Source File

SOURCE=.\gamespyadmin.cpp
# End Source File
# Begin Source File

SOURCE=.\gamespyadmin.h
# End Source File
# Begin Source File

SOURCE=.\gamespyauthmgr.cpp
# End Source File
# Begin Source File

SOURCE=.\gamespyauthmgr.h
# End Source File
# Begin Source File

SOURCE=.\GameSpyBanList.cpp
# End Source File
# Begin Source File

SOURCE=.\GameSpyBanList.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\floodprotectionmgr.cpp
# End Source File
# Begin Source File

SOURCE=.\floodprotectionmgr.h
# End Source File
# Begin Source File

SOURCE=.\modpackage.cpp
# End Source File
# Begin Source File

SOURCE=.\modpackage.h
# End Source File
# Begin Source File

SOURCE=.\modpackagemgr.cpp
# End Source File
# Begin Source File

SOURCE=.\modpackagemgr.h
# End Source File
# Begin Source File

SOURCE=.\Renegade.ico
# End Source File
# End Target
# End Project
