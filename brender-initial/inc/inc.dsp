# Microsoft Developer Studio Project File - Name="inc" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=inc - Win32 Debug P6
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "inc.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "inc.mak" CFG="inc - Win32 Debug P6"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "inc - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "inc - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "inc - Win32 Profile" (based on "Win32 (x86) Static Library")
!MESSAGE "inc - Win32 Debug P6" (based on "Win32 (x86) Static Library")
!MESSAGE "inc - Win32 Profile P6" (based on "Win32 (x86) Static Library")
!MESSAGE "inc - Win32 Release P6" (based on "Win32 (x86) Static Library")
!MESSAGE "inc - Win32 Release (Proton)" (based on "Win32 (x86) Static Library")
!MESSAGE "inc - Win32 Debug (Proton)" (based on "Win32 (x86) Static Library")
!MESSAGE "inc - Win32 Profile (Proton)" (based on "Win32 (x86) Static Library")
!MESSAGE "inc - Win32 Debug P6 (Proton)" (based on "Win32 (x86) Static Library")
!MESSAGE "inc - Win32 Profile P6 (Proton)" (based on "Win32 (x86) Static Library")
!MESSAGE "inc - Win32 Release P6 (Proton)" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""$/brender/inc", TDAAAAAA"
# PROP Scc_LocalPath "."
CPP=xicl.exe

!IF  "$(CFG)" == "inc - Win32 Release"

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
# ADD CPP /nologo /G5 /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D DEBUG=0 /D PARANOID=0 /D EVAL=0 /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /YX /FD /c
# ADD CPP /D_USE_NON_INTEL_COMPILER
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "inc - Win32 Debug"

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
# ADD CPP /nologo /G5 /W3 /GX /Z7 /Od /D BASED_FIXED=0 /D BASED_FLOAT=1 /D "PARANOID-1" /D STATIC=static /D DEBUG=1 /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D PARANOID=1 /D EVAL=0 /D STATIC= /YX /FD /c
# ADD CPP /D_USE_NON_INTEL_COMPILER
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "inc - Win32 Profile"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "inc___Wi"
# PROP BASE Intermediate_Dir "inc___Wi"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Profile"
# PROP Intermediate_Dir "Profile"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D DEBUG=0 /D PARANOID=0 /D EVAL=0 /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /YX /FD /c
# ADD CPP /nologo /G5 /W3 /GX /Zi /O2 /D DEBUG=0 /D PARANOID=0 /D EVAL=0 /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /YX /FD /c
# ADD CPP /D_USE_NON_INTEL_COMPILER
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "inc - Win32 Debug P6"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "inc___Wi"
# PROP BASE Intermediate_Dir "inc___Wi"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug P6"
# PROP Intermediate_Dir "Debug P6"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /W3 /GX /Z7 /Od /D BASED_FIXED=0 /D BASED_FLOAT=1 /D "PARANOID-1" /D STATIC=static /D DEBUG=1 /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D PARANOID=1 /D EVAL=0 /D STATIC= /YX /FD /c
# ADD CPP /nologo /G6 /W3 /GX /Zi /Od /D BASED_FIXED=0 /D BASED_FLOAT=1 /D STATIC=static /D DEBUG=1 /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D PARANOID=1 /D EVAL=0 /D STATIC= /YX /FD /c
# ADD CPP /D_USE_NON_INTEL_COMPILER
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "inc - Win32 Profile P6"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "inc___W0"
# PROP BASE Intermediate_Dir "inc___W0"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Profile P6"
# PROP Intermediate_Dir "Profile P6"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /W3 /GX /Zi /O2 /D DEBUG=0 /D PARANOID=0 /D EVAL=0 /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /YX /FD /c
# ADD CPP /nologo /G6 /W3 /GX /Zi /O2 /D DEBUG=0 /D PARANOID=0 /D EVAL=0 /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /YX /FD /c
# ADD CPP /D_USE_NON_INTEL_COMPILER
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "inc - Win32 Release P6"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "inc___W1"
# PROP BASE Intermediate_Dir "inc___W1"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release P6"
# PROP Intermediate_Dir "Release P6"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D DEBUG=0 /D PARANOID=0 /D EVAL=0 /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /YX /FD /c
# ADD CPP /nologo /G6 /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D DEBUG=0 /D PARANOID=0 /D EVAL=0 /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /YX /FD /c
# ADD CPP /D_USE_NON_INTEL_COMPILER
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "inc - Win32 Release (Proton)"

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
# ADD CPP /nologo /G5 /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D DEBUG=0 /D PARANOID=0 /D EVAL=0 /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /YX /FD /c
# ADD CPP /D_USE_INTEL_COMPILER
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "inc - Win32 Debug (Proton)"

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
# ADD CPP /nologo /G5 /W3 /GX /Z7 /Od /D BASED_FIXED=0 /D BASED_FLOAT=1 /D "PARANOID-1" /D STATIC=static /D DEBUG=1 /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D PARANOID=1 /D EVAL=0 /D STATIC= /YX /FD /c
# ADD CPP /D_USE_INTEL_COMPILER
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "inc - Win32 Profile (Proton)"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "inc___Wi"
# PROP BASE Intermediate_Dir "inc___Wi"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Profile Proton"
# PROP Intermediate_Dir "Profile Proton"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D DEBUG=0 /D PARANOID=0 /D EVAL=0 /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /YX /FD /c
# ADD CPP /nologo /G5 /W3 /GX /Zi /O2 /D DEBUG=0 /D PARANOID=0 /D EVAL=0 /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /YX /FD /c
# ADD CPP /D_USE_INTEL_COMPILER
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "inc - Win32 Debug P6 (Proton)"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "inc___Wi"
# PROP BASE Intermediate_Dir "inc___Wi"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug P6 Proton"
# PROP Intermediate_Dir "Debug P6 Proton"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /W3 /GX /Z7 /Od /D BASED_FIXED=0 /D BASED_FLOAT=1 /D "PARANOID-1" /D STATIC=static /D DEBUG=1 /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D PARANOID=1 /D EVAL=0 /D STATIC= /YX /FD /c
# ADD CPP /nologo /G6 /W3 /GX /Zi /Od /D BASED_FIXED=0 /D BASED_FLOAT=1 /D STATIC=static /D DEBUG=1 /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D PARANOID=1 /D EVAL=0 /D STATIC= /YX /FD /c
# ADD CPP /D_USE_INTEL_COMPILER
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "inc - Win32 Profile P6 (Proton)"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "inc___W0"
# PROP BASE Intermediate_Dir "inc___W0"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Profile P6 Proton"
# PROP Intermediate_Dir "Profile P6 Proton"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /W3 /GX /Zi /O2 /D DEBUG=0 /D PARANOID=0 /D EVAL=0 /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /YX /FD /c
# ADD CPP /nologo /G6 /W3 /GX /Zi /O2 /D DEBUG=0 /D PARANOID=0 /D EVAL=0 /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /YX /FD /c
# ADD CPP /D_USE_INTEL_COMPILER
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "inc - Win32 Release P6 (Proton)"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "inc___W1"
# PROP BASE Intermediate_Dir "inc___W1"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release P6 Proton"
# PROP Intermediate_Dir "Release P6 Proton"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D DEBUG=0 /D PARANOID=0 /D EVAL=0 /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /YX /FD /c
# ADD CPP /nologo /G6 /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D DEBUG=0 /D PARANOID=0 /D EVAL=0 /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /YX /FD /c
# ADD CPP /D_USE_INTEL_COMPILER
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "inc - Win32 Release"
# Name "inc - Win32 Debug"
# Name "inc - Win32 Profile"
# Name "inc - Win32 Debug P6"
# Name "inc - Win32 Profile P6"
# Name "inc - Win32 Release P6"
# Name "inc - Win32 Release (Proton)"
# Name "inc - Win32 Debug (Proton)"
# Name "inc - Win32 Profile (Proton)"
# Name "inc - Win32 Debug P6 (Proton)"
# Name "inc - Win32 Profile P6 (Proton)"
# Name "inc - Win32 Release P6 (Proton)"
# Begin Group "Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\actor.h
# End Source File
# Begin Source File

SOURCE=.\angles.h
# End Source File
# Begin Source File

SOURCE=.\assoca_p.h
# End Source File
# Begin Source File

SOURCE=.\assocarr.h
# End Source File
# Begin Source File

SOURCE=.\brdiag.h
# End Source File
# Begin Source File

SOURCE=.\brender.h
# End Source File
# Begin Source File

SOURCE=.\brerror.h
# End Source File
# Begin Source File

SOURCE=.\brfile.h
# End Source File
# Begin Source File

SOURCE=.\brfont.h
# End Source File
# Begin Source File

SOURCE=.\brlimits.h
# End Source File
# Begin Source File

SOURCE=.\brmem.h
# End Source File
# Begin Source File

SOURCE=.\brv1db.h
# End Source File
# Begin Source File

SOURCE=.\camera.h
# End Source File
# Begin Source File

SOURCE=.\colour.h
# End Source File
# Begin Source File

SOURCE=.\compend.h
# End Source File
# Begin Source File

SOURCE=.\compiler.h
# End Source File
# Begin Source File

SOURCE=.\custsupt.h
# End Source File
# Begin Source File

SOURCE=.\dmyifix.h
# End Source File
# Begin Source File

SOURCE=.\fixed.h
# End Source File
# Begin Source File

SOURCE=.\fw_p.h
# End Source File
# Begin Source File

SOURCE=.\g386ifix.h
# End Source File
# Begin Source File

SOURCE=.\light.h
# End Source File
# Begin Source File

SOURCE=.\material.h
# End Source File
# Begin Source File

SOURCE=.\math_p.h
# End Source File
# Begin Source File

SOURCE=.\math_x.h
# End Source File
# Begin Source File

SOURCE=.\matrix.h
# End Source File
# Begin Source File

SOURCE=.\model.h
# End Source File
# Begin Source File

SOURCE=.\mscifix.h
# End Source File
# Begin Source File

SOURCE=.\order.h
# End Source File
# Begin Source File

SOURCE=.\pixelmap.h
# End Source File
# Begin Source File

SOURCE=.\pm_p.h
# End Source File
# Begin Source File

SOURCE=.\pool.h
# End Source File
# Begin Source File

SOURCE=.\proifix.h
# End Source File
# Begin Source File

SOURCE=.\quat.h
# End Source File
# Begin Source File

SOURCE=.\render.h
# End Source File
# Begin Source File

SOURCE=.\scalar.h
# End Source File
# Begin Source File

SOURCE=.\syscn_p.h
# End Source File
# Begin Source File

SOURCE=.\token.h
# End Source File
# Begin Source File

SOURCE=.\transfrm.h
# End Source File
# Begin Source File

SOURCE=.\v1db_p.h
# End Source File
# Begin Source File

SOURCE=.\vecfns.h
# End Source File
# Begin Source File

SOURCE=.\vector.h
# End Source File
# Begin Source File

SOURCE=.\wtcifix.h
# End Source File
# End Group
# End Target
# End Project
