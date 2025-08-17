# Microsoft Developer Studio Project File - Name="launcher" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=launcher - Win32 Debug Demo
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "launcher.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "launcher.mak" CFG="launcher - Win32 Debug Demo"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "launcher - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "launcher - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "launcher - Win32 Debug FDS" (based on "Win32 (x86) Application")
!MESSAGE "launcher - Win32 Release FDS" (based on "Win32 (x86) Application")
!MESSAGE "launcher - Win32 Debug Demo" (based on "Win32 (x86) Application")
!MESSAGE "launcher - Win32 Release Demo" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/APILauncher", DJDAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "launcher - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /W3 /O1 /Ob2 /I "ToolKit" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "COPY_PROTECT" /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /version:1.4 /subsystem:windows /machine:I386 /out:"..\..\Run\Launcher.exe"
# SUBTRACT LINK32 /map

!ELSEIF  "$(CFG)" == "launcher - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "ToolKit" /D "_DEBUG" /D "DEBUG" /D "WIN32" /D "_WINDOWS" /D "COPY_PROTECTno" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 
# ADD LINK32 patchw32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib winmm.lib /nologo /subsystem:windows /debug /machine:I386 /out:"c:\renegade\cdver\renegade.exe"  /libpath:"."

!ELSEIF  "$(CFG)" == "launcher - Win32 Debug FDS"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug_FDS"
# PROP BASE Intermediate_Dir "Debug_FDS"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug_FDS"
# PROP Intermediate_Dir "Debug_FDS"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /I "ToolKit" /D "_DEBUG" /D "DEBUG" /D "WIN32" /D "_WINDOWS" /D "COPY_PROTECTno" /FR /YX /FD /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "ToolKit" /D "_DEBUG" /D "DEBUG" /D "COPY_PROTECTno" /D "WIN32" /D "_WINDOWS" /D "FREEDEDICATEDSERVER" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 patchw32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib winmm.lib /nologo /subsystem:windows /debug /machine:I386 /out:"c:\renegade\cdver\renegade.exe"  /libpath:"."
# ADD LINK32 patchw32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib winmm.lib /nologo /subsystem:windows /debug /machine:I386 /out:"..\..\Run\renegadeserver.exe"  /libpath:"."

!ELSEIF  "$(CFG)" == "launcher - Win32 Release FDS"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release_FDS"
# PROP BASE Intermediate_Dir "Release_FDS"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release_FDS"
# PROP Intermediate_Dir "Release_FDS"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /O1 /Ob2 /I "ToolKit" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "COPY_PROTECT" /FR /FD /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /W3 /O1 /Ob2 /I "ToolKit" /D "NDEBUG" /D "COPY_PROTECT" /D "WIN32" /D "_WINDOWS" /D "FREEDEDICATEDSERVER" /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /version:1.4 /subsystem:windows /machine:I386 /out:"..\..\Run\Launcher.exe"
# SUBTRACT BASE LINK32 /map
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /version:1.4 /subsystem:windows /machine:I386 /out:"..\..\Run\renegadeserver.exe"
# SUBTRACT LINK32 /map

!ELSEIF  "$(CFG)" == "launcher - Win32 Debug Demo"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug_Demo"
# PROP BASE Intermediate_Dir "Debug_Demo"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug_Demo"
# PROP Intermediate_Dir "Debug_Demo"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /I "ToolKit" /D "_DEBUG" /D "DEBUG" /D "COPY_PROTECTno" /D "WIN32" /D "_WINDOWS" /D "FREEDEDICATEDSERVER" /FR /YX /FD /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "ToolKit" /D "_DEBUG" /D "DEBUG" /D "COPY_PROTECTno" /D "WIN32" /D "_WINDOWS" /D "MULTIPLAYERDEMO" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 patchw32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib winmm.lib /nologo /subsystem:windows /debug /machine:I386 /out:"..\..\Run\renegadeserver.exe"  /libpath:"."
# ADD LINK32 patchw32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib winmm.lib /nologo /subsystem:windows /debug /machine:I386 /out:"..\..\Run\consoleserverD.exe"  /libpath:"."

!ELSEIF  "$(CFG)" == "launcher - Win32 Release Demo"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release_Demo"
# PROP BASE Intermediate_Dir "Release_Demo"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release_Demo"
# PROP Intermediate_Dir "Release_Demo"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /O1 /Ob2 /I "ToolKit" /D "NDEBUG" /D "COPY_PROTECT" /D "WIN32" /D "_WINDOWS" /D "FREEDEDICATEDSERVER" /FR /FD /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /W3 /O1 /Ob2 /I "ToolKit" /D "NDEBUG" /D "COPY_PROTECT" /D "WIN32" /D "_WINDOWS" /D "MULTIPLAYERDEMO" /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /version:1.4 /subsystem:windows /machine:I386 /out:"..\..\Run\renegadeserver.exe"
# SUBTRACT BASE LINK32 /map
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /version:1.4 /subsystem:windows /machine:I386 /out:"..\..\Run\consoleserver.exe"
# SUBTRACT LINK32 /map

!ENDIF 

# Begin Target

# Name "launcher - Win32 Release"
# Name "launcher - Win32 Debug"
# Name "launcher - Win32 Debug FDS"
# Name "launcher - Win32 Release FDS"
# Name "launcher - Win32 Debug Demo"
# Name "launcher - Win32 Release Demo"
# Begin Group "dbglib"

# PROP Default_Filter ".cpp,.h"
# Begin Source File

SOURCE=.\configfile.cpp
# End Source File
# Begin Source File

SOURCE=.\configfile.h
# End Source File
# Begin Source File

SOURCE=.\dictionary.h
# End Source File
# Begin Source File

SOURCE=.\filed.h
# End Source File
# Begin Source File

SOURCE=.\monod.cpp
# End Source File
# Begin Source File

SOURCE=.\monod.h
# End Source File
# Begin Source File

SOURCE=.\odevice.h
# End Source File
# Begin Source File

SOURCE=.\streamer.cpp
# End Source File
# Begin Source File

SOURCE=.\streamer.h
# End Source File
# Begin Source File

SOURCE=.\wdebug.cpp
# End Source File
# Begin Source File

SOURCE=.\wdebug.h
# End Source File
# Begin Source File

SOURCE=.\wstring.cpp
# End Source File
# Begin Source File

SOURCE=.\wstring.h
# End Source File
# End Group
# Begin Group "SafeDisk"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\SafeDisk\CdaPfn.h
# End Source File
# End Group
# Begin Group "Resources"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\launcher1.rc
# End Source File
# Begin Source File

SOURCE=.\Register.ico
# End Source File
# Begin Source File

SOURCE=.\renegade.ico
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# End Group
# Begin Group "Toolkit"

# PROP Default_Filter ""
# Begin Group "Storage"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Toolkit\Storage\File.cpp
# End Source File
# Begin Source File

SOURCE=.\Toolkit\Storage\File.h
# End Source File
# Begin Source File

SOURCE=.\Toolkit\Storage\Rights.h
# End Source File
# Begin Source File

SOURCE=.\Toolkit\Storage\Stream.h
# End Source File
# End Group
# Begin Group "Debug"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Toolkit\Debug\DebugPrint.cpp
# End Source File
# Begin Source File

SOURCE=.\Toolkit\Debug\DebugPrint.h
# End Source File
# End Group
# Begin Group "Support"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Toolkit\Support\RefCounted.h
# End Source File
# Begin Source File

SOURCE=.\Toolkit\Support\RefPtr.h
# End Source File
# Begin Source File

SOURCE=.\Toolkit\Support\StringConvert.cpp
# End Source File
# Begin Source File

SOURCE=.\Toolkit\Support\StringConvert.h
# End Source File
# Begin Source File

SOURCE=.\Toolkit\Support\UString.cpp
# End Source File
# Begin Source File

SOURCE=.\Toolkit\Support\UString.h
# End Source File
# Begin Source File

SOURCE=.\Toolkit\Support\UTypes.h
# End Source File
# Begin Source File

SOURCE=.\Toolkit\Support\Visualc.h
# End Source File
# End Group
# End Group
# Begin Source File

SOURCE=.\BFISH.CPP
# End Source File
# Begin Source File

SOURCE=.\BFISH.H
# End Source File
# Begin Source File

SOURCE=.\dialog.cpp
# End Source File
# Begin Source File

SOURCE=.\dialog.h
# End Source File
# Begin Source File

SOURCE=.\findpatch.cpp
# End Source File
# Begin Source File

SOURCE=.\findpatch.h
# End Source File
# Begin Source File

SOURCE=.\Launcher.lcf

!IF  "$(CFG)" == "launcher - Win32 Release"

# Begin Custom Build
InputPath=.\Launcher.lcf
InputName=Launcher

"..\..\Run\$(InputName).lcf" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) ..\..\Run

# End Custom Build

!ELSEIF  "$(CFG)" == "launcher - Win32 Debug"

# Begin Custom Build
InputPath=.\Launcher.lcf
InputName=Launcher

"..\..\Run\$(InputName).lcf" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) ..\..\Run

# End Custom Build

!ELSEIF  "$(CFG)" == "launcher - Win32 Debug FDS"

# Begin Custom Build
InputPath=.\Launcher.lcf
InputName=Launcher

"..\..\Run\$(InputName).lcf" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) ..\..\Run

# End Custom Build

!ELSEIF  "$(CFG)" == "launcher - Win32 Release FDS"

# Begin Custom Build
InputPath=.\Launcher.lcf
InputName=Launcher

"..\..\Run\$(InputName).lcf" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) ..\..\Run

# End Custom Build

!ELSEIF  "$(CFG)" == "launcher - Win32 Debug Demo"

# Begin Custom Build
InputPath=.\Launcher.lcf
InputName=Launcher

"..\..\Run\$(InputName).lcf" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) ..\..\Run

# End Custom Build

!ELSEIF  "$(CFG)" == "launcher - Win32 Release Demo"

# Begin Custom Build
InputPath=.\Launcher.lcf
InputName=Launcher

"..\..\Run\$(InputName).lcf" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) ..\..\Run

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\loadbmp.cpp
# End Source File
# Begin Source File

SOURCE=.\loadbmp.h
# End Source File
# Begin Source File

SOURCE=.\main.cpp
# End Source File
# Begin Source File

SOURCE=.\patch.cpp
# End Source File
# Begin Source File

SOURCE=.\patch.h
# End Source File
# Begin Source File

SOURCE=.\process.cpp
# End Source File
# Begin Source File

SOURCE=.\process.h
# End Source File
# Begin Source File

SOURCE=.\Protect.cpp
# End Source File
# Begin Source File

SOURCE=.\Protect.h
# End Source File
# Begin Source File

SOURCE=.\winblows.cpp
# End Source File
# Begin Source File

SOURCE=.\winblows.h
# End Source File
# Begin Source File

SOURCE=.\wstypes.h
# End Source File
# End Target
# End Project
