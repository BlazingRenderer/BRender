# Microsoft Developer Studio Project File - Name="ati" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=ati - Win32 Debug P6
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ati.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ati.mak" CFG="ati - Win32 Debug P6"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ati - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "ati - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "ati - Win32 Profile" (based on "Win32 (x86) Static Library")
!MESSAGE "ati - Win32 Debug P6" (based on "Win32 (x86) Static Library")
!MESSAGE "ati - Win32 Release P6" (based on "Win32 (x86) Static Library")
!MESSAGE "ati - Win32 Profile P6" (based on "Win32 (x86) Static Library")
!MESSAGE "ati - Win32 Release (Proton)" (based on "Win32 (x86) Static Library")
!MESSAGE "ati - Win32 Debug (Proton)" (based on "Win32 (x86) Static Library")
!MESSAGE "ati - Win32 Profile (Proton)" (based on "Win32 (x86) Static Library")
!MESSAGE "ati - Win32 Debug P6 (Proton)" (based on\
 "Win32 (x86) Static Library")
!MESSAGE "ati - Win32 Release P6 (Proton)" (based on\
 "Win32 (x86) Static Library")
!MESSAGE "ati - Win32 Profile P6 (Proton)" (based on\
 "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""$/croc/source/brender/ati", VHABAAAA"
# PROP Scc_LocalPath "."
CPP=xicl.exe

!IF  "$(CFG)" == "ati - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /G5 /W3 /GX /O2 /I "h:\croc\source\brender\inc" /I "h:\croc\source\brender\host" /I "h:\croc\source\brender\fw" /I "h:\croc\source\brender\math" /I "h:\croc\source\brender\fmt" /I "h:\croc\source\brender\pixelmap" /I "h:\croc\source\brender\softrend" /I "h:\croc\source\brender\ati" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "__MSC__" /D BrDrv1Begin=BrDrv1ATIBegin /D DEBUG=0 /D PARANOID=0 /D EVAL=0 /D "_USE_NON_INTEL_COMPILER" /YX /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "ati - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /G5 /W3 /GX /Z7 /Od /I "h:\croc\source\brender\inc" /I "h:\croc\source\brender\host" /I "h:\croc\source\brender\fw" /I "h:\croc\source\brender\math" /I "h:\croc\source\brender\fmt" /I "h:\croc\source\brender\pixelmap" /I "h:\croc\source\brender\softrend" /I "h:\croc\source\brender\ati" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "__MSC__" /D BrDrv1Begin=BrDrv1ATIBegin /D DEBUG=1 /D PARANOID=1 /D EVAL=0 /D "_USE_NON_INTEL_COMPILER" /FR /YX /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "ati - Win32 Profile"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ati___W"
# PROP BASE Intermediate_Dir "ati___W"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Profile"
# PROP Intermediate_Dir "Profile"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /G5 /W3 /GX /Zi /O2 /I "h:\croc\source\brender\inc" /I "h:\croc\source\brender\host" /I "h:\croc\source\brender\fw" /I "h:\croc\source\brender\math" /I "h:\croc\source\brender\fmt" /I "h:\croc\source\brender\pixelmap" /I "h:\croc\source\brender\softrend" /I "h:\croc\source\brender\ati" /D "__MSC__" /D BrDrv1Begin=BrDrv1ATIBegin /D DEBUG=0 /D PARANOID=0 /D EVAL=0 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D BASED_FIXED=0 /D BASED_FLOAT=1 /D "_USE_NON_INTEL_COMPILER" /FR /YX /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "ati - Win32 Debug P6"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "ati___W"
# PROP BASE Intermediate_Dir "ati___W"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug P6"
# PROP Intermediate_Dir "Debug P6"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /W3 /GX /Z7 /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "__MSC__" /D BrDrv1Begin=BrDrv1ATIBegin /D DEBUG=1 /D PARANOID=1 /D EVAL=0 /FR /YX /FD /c
# ADD CPP /nologo /G6 /W3 /GX /Zi /Od /I "h:\croc\source\brender\inc" /I "h:\croc\source\brender\host" /I "h:\croc\source\brender\fw" /I "h:\croc\source\brender\math" /I "h:\croc\source\brender\fmt" /I "h:\croc\source\brender\pixelmap" /I "h:\croc\source\brender\softrend" /I "h:\croc\source\brender\ati" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "__MSC__" /D BrDrv1Begin=BrDrv1ATIBegin /D DEBUG=1 /D PARANOID=1 /D EVAL=0 /D "_USE_NON_INTEL_COMPILER" /FR /YX /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "ati - Win32 Release P6"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ati___0"
# PROP BASE Intermediate_Dir "ati___0"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release P6"
# PROP Intermediate_Dir "Release P6"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "__MSC__" /D BrDrv1Begin=BrDrv1ATIBegin /D DEBUG=0 /D PARANOID=0 /D EVAL=0 /YX /FD /c
# ADD CPP /nologo /G6 /W3 /GX /O2 /I "h:\croc\source\brender\inc" /I "h:\croc\source\brender\host" /I "h:\croc\source\brender\fw" /I "h:\croc\source\brender\math" /I "h:\croc\source\brender\fmt" /I "h:\croc\source\brender\pixelmap" /I "h:\croc\source\brender\softrend" /I "h:\croc\source\brender\ati" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "__MSC__" /D BrDrv1Begin=BrDrv1ATIBegin /D DEBUG=0 /D PARANOID=0 /D EVAL=0 /D "_USE_NON_INTEL_COMPILER" /YX /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "ati - Win32 Profile P6"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ati___1"
# PROP BASE Intermediate_Dir "ati___1"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Profile P6"
# PROP Intermediate_Dir "Profile P6"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /W3 /GX /Zi /O2 /D "__MSC__" /D BrDrv1Begin=BrDrv1ATIBegin /D DEBUG=0 /D PARANOID=0 /D EVAL=0 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D BASED_FIXED=0 /D BASED_FLOAT=1 /FR /YX /FD /c
# ADD CPP /nologo /G6 /W3 /GX /Zi /O2 /I "h:\croc\source\brender\inc" /I "h:\croc\source\brender\host" /I "h:\croc\source\brender\fw" /I "h:\croc\source\brender\math" /I "h:\croc\source\brender\fmt" /I "h:\croc\source\brender\pixelmap" /I "h:\croc\source\brender\softrend" /I "h:\croc\source\brender\ati" /D "__MSC__" /D BrDrv1Begin=BrDrv1ATIBegin /D DEBUG=0 /D PARANOID=0 /D EVAL=0 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D BASED_FIXED=0 /D BASED_FLOAT=1 /D "_USE_NON_INTEL_COMPILER" /FR /YX /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "ati - Win32 Release (Proton)"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release Proton"
# PROP Intermediate_Dir "Release Proton"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /G5 /W3 /GX /O2 /I "h:\croc\source\brender\inc" /I "h:\croc\source\brender\host" /I "h:\croc\source\brender\fw" /I "h:\croc\source\brender\math" /I "h:\croc\source\brender\fmt" /I "h:\croc\source\brender\pixelmap" /I "h:\croc\source\brender\softrend" /I "h:\croc\source\brender\ati" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "__MSC__" /D BrDrv1Begin=BrDrv1ATIBegin /D DEBUG=0 /D PARANOID=0 /D EVAL=0 /D "_USE_INTEL_COMPILER" /YX /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "ati - Win32 Debug (Proton)"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug Proton"
# PROP Intermediate_Dir "Debug Proton"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /G5 /W3 /GX /Z7 /Od /I "h:\croc\source\brender\inc" /I "h:\croc\source\brender\host" /I "h:\croc\source\brender\fw" /I "h:\croc\source\brender\math" /I "h:\croc\source\brender\fmt" /I "h:\croc\source\brender\pixelmap" /I "h:\croc\source\brender\softrend" /I "h:\croc\source\brender\ati" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "__MSC__" /D BrDrv1Begin=BrDrv1ATIBegin /D DEBUG=1 /D PARANOID=1 /D EVAL=0 /D "_USE_INTEL_COMPILER" /FR /YX /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "ati - Win32 Profile (Proton)"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ati___W"
# PROP BASE Intermediate_Dir "ati___W"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Profile Proton"
# PROP Intermediate_Dir "Profile Proton"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /G5 /W3 /GX /Zi /O2 /I "h:\croc\source\brender\inc" /I "h:\croc\source\brender\host" /I "h:\croc\source\brender\fw" /I "h:\croc\source\brender\math" /I "h:\croc\source\brender\fmt" /I "h:\croc\source\brender\pixelmap" /I "h:\croc\source\brender\softrend" /I "h:\croc\source\brender\ati" /D "__MSC__" /D BrDrv1Begin=BrDrv1ATIBegin /D DEBUG=0 /D PARANOID=0 /D EVAL=0 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D BASED_FIXED=0 /D BASED_FLOAT=1 /D "_USE_INTEL_COMPILER" /FR /YX /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "ati - Win32 Debug P6 (Proton)"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "ati___W"
# PROP BASE Intermediate_Dir "ati___W"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug P6 Proton"
# PROP Intermediate_Dir "Debug P6 Proton"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /W3 /GX /Z7 /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "__MSC__" /D BrDrv1Begin=BrDrv1ATIBegin /D DEBUG=1 /D PARANOID=1 /D EVAL=0 /FR /YX /FD /c
# ADD CPP /nologo /G6 /W3 /GX /Zi /Od /I "h:\croc\source\brender\inc" /I "h:\croc\source\brender\host" /I "h:\croc\source\brender\fw" /I "h:\croc\source\brender\math" /I "h:\croc\source\brender\fmt" /I "h:\croc\source\brender\pixelmap" /I "h:\croc\source\brender\softrend" /I "h:\croc\source\brender\ati" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "__MSC__" /D BrDrv1Begin=BrDrv1ATIBegin /D DEBUG=1 /D PARANOID=1 /D EVAL=0 /D "_USE_INTEL_COMPILER" /FR /YX /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "ati - Win32 Release P6 (Proton)"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ati___0"
# PROP BASE Intermediate_Dir "ati___0"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release P6 Proton"
# PROP Intermediate_Dir "Release P6 Proton"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "__MSC__" /D BrDrv1Begin=BrDrv1ATIBegin /D DEBUG=0 /D PARANOID=0 /D EVAL=0 /YX /FD /c
# ADD CPP /nologo /G6 /W3 /GX /O2 /I "h:\croc\source\brender\inc" /I "h:\croc\source\brender\host" /I "h:\croc\source\brender\fw" /I "h:\croc\source\brender\math" /I "h:\croc\source\brender\fmt" /I "h:\croc\source\brender\pixelmap" /I "h:\croc\source\brender\softrend" /I "h:\croc\source\brender\ati" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "__MSC__" /D BrDrv1Begin=BrDrv1ATIBegin /D DEBUG=0 /D PARANOID=0 /D EVAL=0 /D "_USE_INTEL_COMPILER" /YX /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "ati - Win32 Profile P6 (Proton)"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ati___1"
# PROP BASE Intermediate_Dir "ati___1"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Profile P6 Proton"
# PROP Intermediate_Dir "Profile P6 Proton"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /W3 /GX /Zi /O2 /D "__MSC__" /D BrDrv1Begin=BrDrv1ATIBegin /D DEBUG=0 /D PARANOID=0 /D EVAL=0 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D BASED_FIXED=0 /D BASED_FLOAT=1 /FR /YX /FD /c
# ADD CPP /nologo /G6 /W3 /GX /Zi /O2 /I "h:\croc\source\brender\inc" /I "h:\croc\source\brender\host" /I "h:\croc\source\brender\fw" /I "h:\croc\source\brender\math" /I "h:\croc\source\brender\fmt" /I "h:\croc\source\brender\pixelmap" /I "h:\croc\source\brender\softrend" /I "h:\croc\source\brender\ati" /D "__MSC__" /D BrDrv1Begin=BrDrv1ATIBegin /D DEBUG=0 /D PARANOID=0 /D EVAL=0 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D BASED_FIXED=0 /D BASED_FLOAT=1 /D "_USE_INTEL_COMPILER" /FR /YX /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "ati - Win32 Release"
# Name "ati - Win32 Debug"
# Name "ati - Win32 Profile"
# Name "ati - Win32 Debug P6"
# Name "ati - Win32 Release P6"
# Name "ati - Win32 Profile P6"
# Name "ati - Win32 Release (Proton)"
# Name "ati - Win32 Debug (Proton)"
# Name "ati - Win32 Profile (Proton)"
# Name "ati - Win32 Debug P6 (Proton)"
# Name "ati - Win32 Release P6 (Proton)"
# Name "ati - Win32 Profile P6 (Proton)"
# Begin Group "C Source"

# PROP Default_Filter "*.c"
# Begin Source File

SOURCE=.\Atilink.c
# End Source File
# Begin Source File

SOURCE=.\Devclut.c
# End Source File
# Begin Source File

SOURCE=.\Device.c
# End Source File
# Begin Source File

SOURCE=.\Dpmback.c
# End Source File
# Begin Source File

SOURCE=.\Dpmfront.c
# End Source File
# Begin Source File

SOURCE=.\Driver.c
# End Source File
# Begin Source File

SOURCE=.\Match.c

!IF  "$(CFG)" == "ati - Win32 Release"

!ELSEIF  "$(CFG)" == "ati - Win32 Debug"

# ADD CPP /O2 /Ob2

!ELSEIF  "$(CFG)" == "ati - Win32 Profile"

!ELSEIF  "$(CFG)" == "ati - Win32 Debug P6"

# ADD CPP /O2 /Ob2

!ELSEIF  "$(CFG)" == "ati - Win32 Release P6"

!ELSEIF  "$(CFG)" == "ati - Win32 Profile P6"

!ELSEIF  "$(CFG)" == "ati - Win32 Release (Proton)"

!ELSEIF  "$(CFG)" == "ati - Win32 Debug (Proton)"

!ELSEIF  "$(CFG)" == "ati - Win32 Profile (Proton)"

!ELSEIF  "$(CFG)" == "ati - Win32 Debug P6 (Proton)"

!ELSEIF  "$(CFG)" == "ati - Win32 Release P6 (Proton)"

!ELSEIF  "$(CFG)" == "ati - Win32 Profile P6 (Proton)"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Object.c
# End Source File
# Begin Source File

SOURCE=.\Ocfree.c
# End Source File
# Begin Source File

SOURCE=.\Outfcty.c
# End Source File
# Begin Source File

SOURCE=.\Plib.c
# End Source File
# Begin Source File

SOURCE=.\Pstate.c
# End Source File
# Begin Source File

SOURCE=.\Rendfunc.c

!IF  "$(CFG)" == "ati - Win32 Release"

!ELSEIF  "$(CFG)" == "ati - Win32 Debug"

# ADD CPP /O2 /Ob2

!ELSEIF  "$(CFG)" == "ati - Win32 Profile"

!ELSEIF  "$(CFG)" == "ati - Win32 Debug P6"

# ADD CPP /O2 /Ob2

!ELSEIF  "$(CFG)" == "ati - Win32 Release P6"

!ELSEIF  "$(CFG)" == "ati - Win32 Profile P6"

!ELSEIF  "$(CFG)" == "ati - Win32 Release (Proton)"

!ELSEIF  "$(CFG)" == "ati - Win32 Debug (Proton)"

!ELSEIF  "$(CFG)" == "ati - Win32 Profile (Proton)"

!ELSEIF  "$(CFG)" == "ati - Win32 Debug P6 (Proton)"

!ELSEIF  "$(CFG)" == "ati - Win32 Release P6 (Proton)"

!ELSEIF  "$(CFG)" == "ati - Win32 Profile P6 (Proton)"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Sbuffer.c
# End Source File
# End Group
# Begin Group "Headers"

# PROP Default_Filter "*.h"
# Begin Source File

SOURCE=.\Ati3dcif.h
# End Source File
# Begin Source File

SOURCE=.\Atilink.h
# End Source File
# Begin Source File

SOURCE=.\Devclut.h
# End Source File
# Begin Source File

SOURCE=.\Device.h
# End Source File
# Begin Source File

SOURCE=.\Devpixmp.h
# End Source File
# Begin Source File

SOURCE=.\Drv.h
# End Source File
# Begin Source File

SOURCE=.\Drv_ip.h
# End Source File
# Begin Source File

SOURCE=.\Match.h
# End Source File
# Begin Source File

SOURCE=.\Object.h
# End Source File
# Begin Source File

SOURCE=.\Outfcty.h
# End Source File
# Begin Source File

SOURCE=.\Plib.h
# End Source File
# Begin Source File

SOURCE=.\Prim_ip.h
# End Source File
# Begin Source File

SOURCE=.\Pstate.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\Sbuffer.h
# End Source File
# Begin Source File

SOURCE=.\Texture.h
# End Source File
# Begin Source File

SOURCE=.\Timestmp.h
# End Source File
# End Group
# Begin Group "Perl Scripts"

# PROP Default_Filter "*.pl"
# End Group
# Begin Group "Miscellaneous"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Matchinf.c
# PROP Exclude_From_Build 1
# End Source File
# End Group
# Begin Group "Assembly"

# PROP Default_Filter "*.asm"
# End Group
# Begin Group "Assembly Includes"

# PROP Default_Filter "*.inc"
# End Group
# Begin Group "Libraries"

# PROP Default_Filter "*.lib"
# Begin Source File

SOURCE=.\Ati3dcif.lib
# End Source File
# End Group
# End Target
# End Project
