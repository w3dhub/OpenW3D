# Microsoft Developer Studio Project File - Name="WWConfig" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=WWConfig - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "WWConfig.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "WWConfig.mak" CFG="WWConfig - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "WWConfig - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "WWConfig - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Commando/Code/Tools/WWConfig", KDBEAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "WWConfig - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\..\DirectX\include" /I "..\..\combat" /I "..\..\wwdebug" /I "..\..\wwmath" /I "..\..\ww3d2" /I "..\..\wwphys" /I "..\..\wwlib" /I "..\..\wwaudio" /I "..\..\miles6\include" /I "..\..\wwsaveload" /I "..\..\wwnet" /I "..\wwctrl" /I "..\..\wwtranslatedb" /I "..\..\wwbitpack" /I "..\..\wwutil" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 umbra.lib wwaudio.lib ..\..\miles6\lib\win\mss32.lib wwsaveload.lib ww3d2.lib wwdebug.lib wwlib.lib wwmath.lib wwphys.lib wwutil.lib winmm.lib ddraw.lib dinput.lib dxguid.lib dsound.lib vfw32.lib wsock32.lib wwtranslatedb.lib d3dx8.lib d3d8.lib /nologo /subsystem:windows /pdb:"..\..\..\Run\WWConfig.pdb" /map:"..\..\..\Run\WWConfig.map" /debug /machine:I386 /out:"..\..\..\Run\WWConfig.exe" /pdbtype:sept /libpath:"../../libs/release" /libpath:"../../directx\lib" /libpath:"../../umbra/lib/win32-x86"

!ELSEIF  "$(CFG)" == "WWConfig - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\..\DirectX\include" /I "..\..\combat" /I "..\..\wwdebug" /I "..\..\wwmath" /I "..\..\ww3d2" /I "..\..\wwphys" /I "..\..\wwlib" /I "..\..\wwaudio" /I "..\..\miles6\include" /I "..\..\wwsaveload" /I "..\..\wwnet" /I "..\wwctrl" /I "..\..\wwtranslatedb" /I "..\..\wwbitpack" /I "..\..\wwutil" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FR /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /i "Res\\" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 wwnet.lib umbrad.lib wwaudio.lib ..\..\miles6\lib\win\mss32.lib wwsaveload.lib ww3d2.lib wwdebug.lib wwlib.lib wwmath.lib wwphys.lib wwutil.lib winmm.lib ddraw.lib dinput.lib dxguid.lib dsound.lib vfw32.lib wsock32.lib wwtranslatedb.lib d3dx8.lib d3d8.lib /nologo /subsystem:windows /pdb:"..\..\..\Run\WWConfigD.pdb" /map:"..\..\..\Run\WWConfigD.map" /debug /machine:I386 /out:"..\..\..\Run\WWConfigD.exe" /pdbtype:sept /libpath:"../../directx\lib" /libpath:"../../libs/debug" /libpath:"../../umbra/lib/win32-x86"

!ENDIF 

# Begin Target

# Name "WWConfig - Win32 Release"
# Name "WWConfig - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\AudioConfigDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\DriverVersionWarning.cpp
# End Source File
# Begin Source File

SOURCE=.\EZGIMEX.cpp
# End Source File
# Begin Source File

SOURCE=.\locale.cpp
# End Source File
# Begin Source File

SOURCE=.\Locale_API.cpp
# End Source File
# Begin Source File

SOURCE=.\PerformanceConfigDialog.cpp
# End Source File
# Begin Source File

SOURCE=..\..\WWLib\rawfile.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\VideoConfigDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\WWConfig.cpp
# End Source File
# Begin Source File

SOURCE=.\WWConfigDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\..\WWLib\wwfile.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\AudioConfigDialog.h
# End Source File
# Begin Source File

SOURCE=.\DriverVersionWarning.h
# End Source File
# Begin Source File

SOURCE=.\gimex.h
# End Source File
# Begin Source File

SOURCE=.\locale.h
# End Source File
# Begin Source File

SOURCE=.\Locale_API.h
# End Source File
# Begin Source File

SOURCE=.\PerformanceConfigDialog.h
# End Source File
# Begin Source File

SOURCE=..\..\WWLib\rawfile.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\VideoConfigDialog.h
# End Source File
# Begin Source File

SOURCE=.\WWConfig.h
# End Source File
# Begin Source File

SOURCE=.\wwconfig_ids.h
# End Source File
# Begin Source File

SOURCE=.\WWConfigDlg.h
# End Source File
# Begin Source File

SOURCE=..\..\WWLib\wwfile.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\ico00001.ico
# End Source File
# Begin Source File

SOURCE=.\res\ico00002.ico
# End Source File
# Begin Source File

SOURCE=.\res\icon1.ico
# End Source File
# Begin Source File

SOURCE=.\Res\IDL_CHINESE.loc
# End Source File
# Begin Source File

SOURCE=.\Res\IDL_ENGLISH.loc
# End Source File
# Begin Source File

SOURCE=.\Res\IDL_FRENCH.loc
# End Source File
# Begin Source File

SOURCE=.\Res\IDL_GERMAN.loc
# End Source File
# Begin Source File

SOURCE=.\Res\IDL_JAPANESE.loc
# End Source File
# Begin Source File

SOURCE=.\Res\IDL_KOREAN.loc
# End Source File
# Begin Source File

SOURCE=.\res\logo.bmp
# End Source File
# Begin Source File

SOURCE=.\res\WWConfig.ico
# End Source File
# Begin Source File

SOURCE=.\WWConfig.rc
# End Source File
# Begin Source File

SOURCE=.\res\WWConfig.rc2
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
