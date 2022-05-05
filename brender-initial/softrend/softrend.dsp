# Microsoft Developer Studio Project File - Name="softrend" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=softrend - Win32 Debug P6
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "softrend.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "softrend.mak" CFG="softrend - Win32 Debug P6"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "softrend - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "softrend - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "softrend - Win32 Profile" (based on "Win32 (x86) Static Library")
!MESSAGE "softrend - Win32 Debug P6" (based on "Win32 (x86) Static Library")
!MESSAGE "softrend - Win32 Profile P6" (based on "Win32 (x86) Static Library")
!MESSAGE "softrend - Win32 Release P6" (based on "Win32 (x86) Static Library")
!MESSAGE "softrend - Win32 Release (Proton)" (based on\
 "Win32 (x86) Static Library")
!MESSAGE "softrend - Win32 Debug (Proton)" (based on\
 "Win32 (x86) Static Library")
!MESSAGE "softrend - Win32 Profile (Proton)" (based on\
 "Win32 (x86) Static Library")
!MESSAGE "softrend - Win32 Debug P6 (Proton)" (based on\
 "Win32 (x86) Static Library")
!MESSAGE "softrend - Win32 Profile P6 (Proton)" (based on\
 "Win32 (x86) Static Library")
!MESSAGE "softrend - Win32 Release P6 (Proton)" (based on\
 "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""$/brender/softrend", VIAAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe

!IF  "$(CFG)" == "softrend - Win32 Release"

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
# ADD CPP /nologo /G5 /W3 /GX /O2 /D "NDEBUG" /D DEBUG=0 /D PARANOID=0 /D "_USE_NON_INTEL_COMPILER" /D BrDrv1Begin=BrDrv1SoftRendBegin /D "WIN32" /D "_WINDOWS" /D EVAL=0 /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /D "__386__" /YX /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "softrend - Win32 Debug"

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
# ADD CPP /nologo /G5 /W3 /GX /Z7 /Od /D "PARANOID-1" /D DEBUG=1 /D "_DEBUG" /D PARANOID=1 /D STATIC= /D "_USE_NON_INTEL_COMPILER" /D BrDrv1Begin=BrDrv1SoftRendBegin /D "WIN32" /D "_WINDOWS" /D EVAL=0 /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /D "__386__" /FR /YX /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "softrend - Win32 Profile"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "softrend"
# PROP BASE Intermediate_Dir "softrend"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Profile"
# PROP Intermediate_Dir "Profile"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D BrDrv1Begin=BrDrv1SoftRendBegin /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D DEBUG=0 /D PARANOID=0 /D EVAL=0 /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /YX /FD /c
# ADD CPP /nologo /G5 /W3 /GX /Zi /O2 /D DEBUG=0 /D PARANOID=0 /D "NDEBUG" /D "_USE_NON_INTEL_COMPILER" /D BrDrv1Begin=BrDrv1SoftRendBegin /D "WIN32" /D "_WINDOWS" /D EVAL=0 /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /D "__386__" /FR /YX /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "softrend - Win32 Debug P6"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "softrend"
# PROP BASE Intermediate_Dir "softrend"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug P6"
# PROP Intermediate_Dir "Debug P6"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /W3 /GX /Z7 /Od /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /D "PARANOID-1" /D BrDrv1Begin=BrDrv1SoftRendBegin /D DEBUG=1 /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D PARANOID=1 /D EVAL=0 /D STATIC= /FR /YX /FD /c
# ADD CPP /nologo /G6 /W3 /GX /Zi /Od /D DEBUG=1 /D "_DEBUG" /D PARANOID=1 /D STATIC= /D "_USE_NON_INTEL_COMPILER" /D BrDrv1Begin=BrDrv1SoftRendBegin /D "WIN32" /D "_WINDOWS" /D EVAL=0 /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /D "__386__" /FR /YX /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "softrend - Win32 Profile P6"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "softren0"
# PROP BASE Intermediate_Dir "softren0"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Profile P6"
# PROP Intermediate_Dir "Profile P6"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /W3 /GX /Zi /O2 /D BrDrv1Begin=BrDrv1SoftRendBegin /D DEBUG=0 /D PARANOID=0 /D EVAL=0 /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /YX /FD /c
# ADD CPP /nologo /G6 /W3 /GX /Zi /O2 /D DEBUG=0 /D PARANOID=0 /D "NDEBUG" /D "_USE_NON_INTEL_COMPILER" /D BrDrv1Begin=BrDrv1SoftRendBegin /D "WIN32" /D "_WINDOWS" /D EVAL=0 /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /D "__386__" /FR /YX /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "softrend - Win32 Release P6"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "softren1"
# PROP BASE Intermediate_Dir "softren1"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release P6"
# PROP Intermediate_Dir "Release P6"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /W3 /GX /O2 /D BrDrv1Begin=BrDrv1SoftRendBegin /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D DEBUG=0 /D PARANOID=0 /D EVAL=0 /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /YX /FD /c
# ADD CPP /nologo /G6 /W3 /GX /O2 /D "NDEBUG" /D DEBUG=0 /D PARANOID=0 /D "_USE_NON_INTEL_COMPILER" /D BrDrv1Begin=BrDrv1SoftRendBegin /D "WIN32" /D "_WINDOWS" /D EVAL=0 /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /D "__386__" /FR /YX /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "softrend - Win32 Release (Proton)"

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
# ADD CPP /nologo /G5 /W3 /GX /O2 /D "NDEBUG" /D DEBUG=0 /D PARANOID=0 /D "_USE_INTEL_COMPILER" /D BrDrv1Begin=BrDrv1SoftRendBegin /D "WIN32" /D "_WINDOWS" /D EVAL=0 /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /D "__386__" /YX /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "softrend - Win32 Debug (Proton)"

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
# ADD CPP /nologo /G5 /W3 /GX /Z7 /Od /D "PARANOID-1" /D DEBUG=1 /D "_DEBUG" /D PARANOID=1 /D STATIC= /D "_USE_INTEL_COMPILER" /D BrDrv1Begin=BrDrv1SoftRendBegin /D "WIN32" /D "_WINDOWS" /D EVAL=0 /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /D "__386__" /FR /YX /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "softrend - Win32 Profile (Proton)"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "softrend"
# PROP BASE Intermediate_Dir "softrend"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Profile Proton"
# PROP Intermediate_Dir "Profile Proton"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D BrDrv1Begin=BrDrv1SoftRendBegin /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D DEBUG=0 /D PARANOID=0 /D EVAL=0 /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /YX /FD /c
# ADD CPP /nologo /G5 /W3 /GX /Zi /O2 /D DEBUG=0 /D PARANOID=0 /D "NDEBUG" /D "_USE_INTEL_COMPILER" /D BrDrv1Begin=BrDrv1SoftRendBegin /D "WIN32" /D "_WINDOWS" /D EVAL=0 /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /D "__386__" /FR /YX /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "softrend - Win32 Debug P6 (Proton)"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "softrend"
# PROP BASE Intermediate_Dir "softrend"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug P6 Proton"
# PROP Intermediate_Dir "Debug P6 Proton"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /W3 /GX /Z7 /Od /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /D "PARANOID-1" /D BrDrv1Begin=BrDrv1SoftRendBegin /D DEBUG=1 /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D PARANOID=1 /D EVAL=0 /D STATIC= /FR /YX /FD /c
# ADD CPP /nologo /G6 /W3 /GX /Zi /Od /D DEBUG=1 /D "_DEBUG" /D PARANOID=1 /D STATIC= /D "_USE_INTEL_COMPILER" /D BrDrv1Begin=BrDrv1SoftRendBegin /D "WIN32" /D "_WINDOWS" /D EVAL=0 /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /D "__386__" /FR /YX /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "softrend - Win32 Profile P6 (Proton)"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "softren0"
# PROP BASE Intermediate_Dir "softren0"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Profile P6 Proton"
# PROP Intermediate_Dir "Profile P6 Proton"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /W3 /GX /Zi /O2 /D BrDrv1Begin=BrDrv1SoftRendBegin /D DEBUG=0 /D PARANOID=0 /D EVAL=0 /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /YX /FD /c
# ADD CPP /nologo /G6 /W3 /GX /Zi /O2 /D DEBUG=0 /D PARANOID=0 /D "NDEBUG" /D "_USE_INTEL_COMPILER" /D BrDrv1Begin=BrDrv1SoftRendBegin /D "WIN32" /D "_WINDOWS" /D EVAL=0 /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /D "__386__" /FR /YX /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "softrend - Win32 Release P6 (Proton)"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "softren1"
# PROP BASE Intermediate_Dir "softren1"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release P6 Proton"
# PROP Intermediate_Dir "Release P6 Proton"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /W3 /GX /O2 /D BrDrv1Begin=BrDrv1SoftRendBegin /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D DEBUG=0 /D PARANOID=0 /D EVAL=0 /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /YX /FD /c
# ADD CPP /nologo /G6 /W3 /GX /O2 /D "NDEBUG" /D DEBUG=0 /D PARANOID=0 /D "_USE_INTEL_COMPILER" /D BrDrv1Begin=BrDrv1SoftRendBegin /D "WIN32" /D "_WINDOWS" /D EVAL=0 /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /D "__386__" /FR /YX /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "softrend - Win32 Release"
# Name "softrend - Win32 Debug"
# Name "softrend - Win32 Profile"
# Name "softrend - Win32 Debug P6"
# Name "softrend - Win32 Profile P6"
# Name "softrend - Win32 Release P6"
# Name "softrend - Win32 Release (Proton)"
# Name "softrend - Win32 Debug (Proton)"
# Name "softrend - Win32 Profile (Proton)"
# Name "softrend - Win32 Debug P6 (Proton)"
# Name "softrend - Win32 Profile P6 (Proton)"
# Name "softrend - Win32 Release P6 (Proton)"
# Begin Group "C Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\alpha.c
# End Source File
# Begin Source File

SOURCE=.\clip.c

!IF  "$(CFG)" == "softrend - Win32 Release"

!ELSEIF  "$(CFG)" == "softrend - Win32 Debug"

!ELSEIF  "$(CFG)" == "softrend - Win32 Profile"

!ELSEIF  "$(CFG)" == "softrend - Win32 Debug P6"

!ELSEIF  "$(CFG)" == "softrend - Win32 Profile P6"

!ELSEIF  "$(CFG)" == "softrend - Win32 Release P6"

# ADD CPP /O2

!ELSEIF  "$(CFG)" == "softrend - Win32 Release (Proton)"

!ELSEIF  "$(CFG)" == "softrend - Win32 Debug (Proton)"

!ELSEIF  "$(CFG)" == "softrend - Win32 Profile (Proton)"

!ELSEIF  "$(CFG)" == "softrend - Win32 Debug P6 (Proton)"

!ELSEIF  "$(CFG)" == "softrend - Win32 Profile P6 (Proton)"

!ELSEIF  "$(CFG)" == "softrend - Win32 Release P6 (Proton)"

# ADD CPP /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\convert.c

!IF  "$(CFG)" == "softrend - Win32 Release"

!ELSEIF  "$(CFG)" == "softrend - Win32 Debug"

!ELSEIF  "$(CFG)" == "softrend - Win32 Profile"

!ELSEIF  "$(CFG)" == "softrend - Win32 Debug P6"

!ELSEIF  "$(CFG)" == "softrend - Win32 Profile P6"

!ELSEIF  "$(CFG)" == "softrend - Win32 Release P6"

# ADD CPP /O2

!ELSEIF  "$(CFG)" == "softrend - Win32 Release (Proton)"

!ELSEIF  "$(CFG)" == "softrend - Win32 Debug (Proton)"

!ELSEIF  "$(CFG)" == "softrend - Win32 Profile (Proton)"

!ELSEIF  "$(CFG)" == "softrend - Win32 Debug P6 (Proton)"

!ELSEIF  "$(CFG)" == "softrend - Win32 Profile P6 (Proton)"

!ELSEIF  "$(CFG)" == "softrend - Win32 Release P6 (Proton)"

# ADD CPP /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\depth.c
# End Source File
# Begin Source File

SOURCE=.\device.c
# End Source File
# Begin Source File

SOURCE=.\driver.c
# End Source File
# Begin Source File

SOURCE=.\faceops.c
# End Source File
# Begin Source File

SOURCE=.\genrend.c

!IF  "$(CFG)" == "softrend - Win32 Release"

!ELSEIF  "$(CFG)" == "softrend - Win32 Debug"

!ELSEIF  "$(CFG)" == "softrend - Win32 Profile"

!ELSEIF  "$(CFG)" == "softrend - Win32 Debug P6"

!ELSEIF  "$(CFG)" == "softrend - Win32 Profile P6"

!ELSEIF  "$(CFG)" == "softrend - Win32 Release P6"

# ADD CPP /O2

!ELSEIF  "$(CFG)" == "softrend - Win32 Release (Proton)"

!ELSEIF  "$(CFG)" == "softrend - Win32 Debug (Proton)"

!ELSEIF  "$(CFG)" == "softrend - Win32 Profile (Proton)"

!ELSEIF  "$(CFG)" == "softrend - Win32 Debug P6 (Proton)"

!ELSEIF  "$(CFG)" == "softrend - Win32 Profile P6 (Proton)"

!ELSEIF  "$(CFG)" == "softrend - Win32 Release P6 (Proton)"

# ADD CPP /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gprim.c
# End Source File
# Begin Source File

SOURCE=.\gv1buckt.c
# End Source File
# Begin Source File

SOURCE=.\gv1model.c
# End Source File
# Begin Source File

SOURCE=.\heap.c
# End Source File
# Begin Source File

SOURCE=.\light24.c
# End Source File
# Begin Source File

SOURCE=.\light8.c
# End Source File
# Begin Source File

SOURCE=.\lighting.c
# End Source File
# Begin Source File

SOURCE=.\mapping.c
# End Source File
# Begin Source File

SOURCE=.\object.c
# End Source File
# Begin Source File

SOURCE=.\ocfree.c
# End Source File
# Begin Source File

SOURCE=.\onscreen.c

!IF  "$(CFG)" == "softrend - Win32 Release"

!ELSEIF  "$(CFG)" == "softrend - Win32 Debug"

!ELSEIF  "$(CFG)" == "softrend - Win32 Profile"

!ELSEIF  "$(CFG)" == "softrend - Win32 Debug P6"

!ELSEIF  "$(CFG)" == "softrend - Win32 Profile P6"

!ELSEIF  "$(CFG)" == "softrend - Win32 Release P6"

# ADD CPP /O2

!ELSEIF  "$(CFG)" == "softrend - Win32 Release (Proton)"

!ELSEIF  "$(CFG)" == "softrend - Win32 Debug (Proton)"

!ELSEIF  "$(CFG)" == "softrend - Win32 Profile (Proton)"

!ELSEIF  "$(CFG)" == "softrend - Win32 Debug P6 (Proton)"

!ELSEIF  "$(CFG)" == "softrend - Win32 Profile P6 (Proton)"

!ELSEIF  "$(CFG)" == "softrend - Win32 Release P6 (Proton)"

# ADD CPP /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\renderer.c
# End Source File
# Begin Source File

SOURCE=.\rendfcty.c
# End Source File
# Begin Source File

SOURCE=.\rndstate.c
# End Source File
# Begin Source File

SOURCE=.\rprim.c
# End Source File
# Begin Source File

SOURCE=.\rv1point.c
# End Source File
# Begin Source File

SOURCE=.\setup.c
# End Source File
# Begin Source File

SOURCE=.\sstate.c
# End Source File
# Begin Source File

SOURCE=.\state.c
# End Source File
# Begin Source File

SOURCE=.\v1model.c

!IF  "$(CFG)" == "softrend - Win32 Release"

!ELSEIF  "$(CFG)" == "softrend - Win32 Debug"

!ELSEIF  "$(CFG)" == "softrend - Win32 Profile"

!ELSEIF  "$(CFG)" == "softrend - Win32 Debug P6"

!ELSEIF  "$(CFG)" == "softrend - Win32 Profile P6"

!ELSEIF  "$(CFG)" == "softrend - Win32 Release P6"

# ADD CPP /O2

!ELSEIF  "$(CFG)" == "softrend - Win32 Release (Proton)"

!ELSEIF  "$(CFG)" == "softrend - Win32 Debug (Proton)"

!ELSEIF  "$(CFG)" == "softrend - Win32 Profile (Proton)"

!ELSEIF  "$(CFG)" == "softrend - Win32 Debug P6 (Proton)"

!ELSEIF  "$(CFG)" == "softrend - Win32 Profile P6 (Proton)"

!ELSEIF  "$(CFG)" == "softrend - Win32 Release P6 (Proton)"

# ADD CPP /O2

!ENDIF 

# End Source File
# End Group
# Begin Group "Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\crocrend.h
# End Source File
# Begin Source File

SOURCE=.\device.h
# End Source File
# Begin Source File

SOURCE=.\drv.h
USERDEP__DRV_H="..\h2incfix.pl"	

!IF  "$(CFG)" == "softrend - Win32 Release"

# Begin Custom Build
InputPath=.\drv.h

"drv.inc" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	h2inc -nologo -G3 -Zp4 -w -c -WIN32 -DBASED_FLOAT=1 -D_WIN32 -D__VISUALC\
                 -D__H2INC__       -D_NO_PROTOTYPES -Fa_h2inc.tmp drv.h 
	perl ..\h2incfix.pl <_h2inc.tmp >drv.inc 
	del _h2inc.tmp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Debug"

# Begin Custom Build
InputPath=.\drv.h

"drv.inc" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	h2inc -nologo -G3 -Zp4 -w -c -WIN32 -DBASED_FLOAT=1 -D_WIN32 -D__VISUALC\
                 -D__H2INC__       -D_NO_PROTOTYPES -Fa_h2inc.tmp drv.h 
	perl ..\h2incfix.pl <_h2inc.tmp >drv.inc 
	del _h2inc.tmp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Profile"

# Begin Custom Build
InputPath=.\drv.h

"drv.inc" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	h2inc -nologo -G3 -Zp4 -w -c -WIN32 -DBASED_FLOAT=1 -D_WIN32 -D__VISUALC\
                 -D__H2INC__       -D_NO_PROTOTYPES -Fa_h2inc.tmp drv.h 
	perl ..\h2incfix.pl <_h2inc.tmp >drv.inc 
	del _h2inc.tmp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Debug P6"

# Begin Custom Build
InputPath=.\drv.h

"drv.inc" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	h2inc -nologo -G3 -Zp4 -w -c -WIN32 -DBASED_FLOAT=1 -D_WIN32 -D__VISUALC\
                 -D__H2INC__       -D_NO_PROTOTYPES -Fa_h2inc.tmp drv.h 
	perl ..\h2incfix.pl <_h2inc.tmp >drv.inc 
	del _h2inc.tmp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Profile P6"

# Begin Custom Build
InputPath=.\drv.h

"drv.inc" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	h2inc -nologo -G3 -Zp4 -w -c -WIN32 -DBASED_FLOAT=1 -D_WIN32 -D__VISUALC\
                 -D__H2INC__       -D_NO_PROTOTYPES -Fa_h2inc.tmp drv.h 
	perl ..\h2incfix.pl <_h2inc.tmp >drv.inc 
	del _h2inc.tmp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Release P6"

# Begin Custom Build
InputPath=.\drv.h

"drv.inc" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	h2inc -nologo -G3 -Zp4 -w -c -WIN32 -DBASED_FLOAT=1 -D_WIN32 -D__VISUALC\
                 -D__H2INC__       -D_NO_PROTOTYPES -Fa_h2inc.tmp drv.h 
	perl ..\h2incfix.pl <_h2inc.tmp >drv.inc 
	del _h2inc.tmp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Release (Proton)"

# Begin Custom Build
InputPath=.\drv.h

"drv.inc" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	h2inc -nologo -G3 -Zp4 -w -c -WIN32 -DBASED_FLOAT=1 -D_WIN32 -D__VISUALC\
                 -D__H2INC__       -D_NO_PROTOTYPES -Fa_h2inc.tmp drv.h 
	perl ..\h2incfix.pl <_h2inc.tmp >drv.inc 
	del _h2inc.tmp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Debug (Proton)"

# Begin Custom Build
InputPath=.\drv.h

"drv.inc" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	h2inc -nologo -G3 -Zp4 -w -c -WIN32 -DBASED_FLOAT=1 -D_WIN32 -D__VISUALC\
                 -D__H2INC__       -D_NO_PROTOTYPES -Fa_h2inc.tmp drv.h 
	perl ..\h2incfix.pl <_h2inc.tmp >drv.inc 
	del _h2inc.tmp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Profile (Proton)"

# Begin Custom Build
InputPath=.\drv.h

"drv.inc" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	h2inc -nologo -G3 -Zp4 -w -c -WIN32 -DBASED_FLOAT=1 -D_WIN32 -D__VISUALC\
                 -D__H2INC__       -D_NO_PROTOTYPES -Fa_h2inc.tmp drv.h 
	perl ..\h2incfix.pl <_h2inc.tmp >drv.inc 
	del _h2inc.tmp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Debug P6 (Proton)"

# Begin Custom Build
InputPath=.\drv.h

"drv.inc" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	h2inc -nologo -G3 -Zp4 -w -c -WIN32 -DBASED_FLOAT=1 -D_WIN32 -D__VISUALC\
                 -D__H2INC__       -D_NO_PROTOTYPES -Fa_h2inc.tmp drv.h 
	perl ..\h2incfix.pl <_h2inc.tmp >drv.inc 
	del _h2inc.tmp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Profile P6 (Proton)"

# Begin Custom Build
InputPath=.\drv.h

"drv.inc" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	h2inc -nologo -G3 -Zp4 -w -c -WIN32 -DBASED_FLOAT=1 -D_WIN32 -D__VISUALC\
                 -D__H2INC__       -D_NO_PROTOTYPES -Fa_h2inc.tmp drv.h 
	perl ..\h2incfix.pl <_h2inc.tmp >drv.inc 
	del _h2inc.tmp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Release P6 (Proton)"

# Begin Custom Build
InputPath=.\drv.h

"drv.inc" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	h2inc -nologo -G3 -Zp4 -w -c -WIN32 -DBASED_FLOAT=1 -D_WIN32 -D__VISUALC\
                 -D__H2INC__       -D_NO_PROTOTYPES -Fa_h2inc.tmp drv.h 
	perl ..\h2incfix.pl <_h2inc.tmp >drv.inc 
	del _h2inc.tmp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\drv_ip.h
# End Source File
# Begin Source File

SOURCE=.\face_386.h
# End Source File
# Begin Source File

SOURCE=.\gen_386.h
# End Source File
# Begin Source File

SOURCE=.\gprim.h
# End Source File
# Begin Source File

SOURCE=.\gv1buckt.h
# End Source File
# Begin Source File

SOURCE=.\gv1model.h
# End Source File
# Begin Source File

SOURCE=.\lighting.h
# End Source File
# Begin Source File

SOURCE=.\lightmac.h
# End Source File
# Begin Source File

SOURCE=.\object.h
# End Source File
# Begin Source File

SOURCE=.\priminfo.h
# End Source File
# Begin Source File

SOURCE=.\rend.h
# End Source File
# Begin Source File

SOURCE=.\renderer.h
# End Source File
# Begin Source File

SOURCE=.\rendfcty.h
# End Source File
# Begin Source File

SOURCE=.\sstate.h
# End Source File
# Begin Source File

SOURCE=.\state.h
# End Source File
# Begin Source File

SOURCE=.\template.h
# End Source File
# Begin Source File

SOURCE=.\timestmp.h
# End Source File
# Begin Source File

SOURCE=.\v1m_386.h
# End Source File
# Begin Source File

SOURCE=.\zsrmacro.h
# End Source File
# End Group
# Begin Group "Perl Scripts"

# PROP Default_Filter ""
# End Group
# Begin Group "Miscellaneous"

# PROP Default_Filter ""
# End Group
# Begin Group "Assembly"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\cull.asm

!IF  "$(CFG)" == "softrend - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=.\cull.asm
InputName=cull

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=.\cull.asm
InputName=cull

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Profile"

# Begin Custom Build
OutDir=.\Profile
InputPath=.\cull.asm
InputName=cull

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Debug P6"

# Begin Custom Build
OutDir=.\Debug P6
InputPath=.\cull.asm
InputName=cull

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Profile P6"

# Begin Custom Build
OutDir=.\Profile P6
InputPath=.\cull.asm
InputName=cull

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Release P6"

# Begin Custom Build
OutDir=.\Release P6
InputPath=.\cull.asm
InputName=cull

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Release (Proton)"

# Begin Custom Build
OutDir=.\Release Proton
InputPath=.\cull.asm
InputName=cull

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Debug (Proton)"

# Begin Custom Build
OutDir=.\Debug Proton
InputPath=.\cull.asm
InputName=cull

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Profile (Proton)"

# Begin Custom Build
OutDir=.\Profile Proton
InputPath=.\cull.asm
InputName=cull

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Debug P6 (Proton)"

# Begin Custom Build
OutDir=.\Debug P6 Proton
InputPath=.\cull.asm
InputName=cull

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Profile P6 (Proton)"

# Begin Custom Build
OutDir=.\Profile P6 Proton
InputPath=.\cull.asm
InputName=cull

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Release P6 (Proton)"

# Begin Custom Build
OutDir=.\Release P6 Proton
InputPath=.\cull.asm
InputName=cull

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\face_386.asm

!IF  "$(CFG)" == "softrend - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=.\face_386.asm
InputName=face_386

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /Zi /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=.\face_386.asm
InputName=face_386

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /Zi /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Profile"

# Begin Custom Build
OutDir=.\Profile
InputPath=.\face_386.asm
InputName=face_386

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /Zi /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Debug P6"

USERDEP__FACE_="drv.h"	"drv.inc"	
# Begin Custom Build
OutDir=.\Debug P6
InputPath=.\face_386.asm
InputName=face_386

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /Zi /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Profile P6"

# Begin Custom Build
OutDir=.\Profile P6
InputPath=.\face_386.asm
InputName=face_386

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /Zi /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Release P6"

# Begin Custom Build
OutDir=.\Release P6
InputPath=.\face_386.asm
InputName=face_386

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /Zi /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Release (Proton)"

# Begin Custom Build
OutDir=.\Release Proton
InputPath=.\face_386.asm
InputName=face_386

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /Zi /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Debug (Proton)"

# Begin Custom Build
OutDir=.\Debug Proton
InputPath=.\face_386.asm
InputName=face_386

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /Zi /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Profile (Proton)"

# Begin Custom Build
OutDir=.\Profile Proton
InputPath=.\face_386.asm
InputName=face_386

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /Zi /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Debug P6 (Proton)"

# Begin Custom Build
OutDir=.\Debug P6 Proton
InputPath=.\face_386.asm
InputName=face_386

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /Zi /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Profile P6 (Proton)"

# Begin Custom Build
OutDir=.\Profile P6 Proton
InputPath=.\face_386.asm
InputName=face_386

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /Zi /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Release P6 (Proton)"

# Begin Custom Build
OutDir=.\Release P6 Proton
InputPath=.\face_386.asm
InputName=face_386

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /Zi /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gen_386.asm
USERDEP__GEN_3="drv.h"	"drv.inc"	

!IF  "$(CFG)" == "softrend - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=.\gen_386.asm
InputName=gen_386

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=.\gen_386.asm
InputName=gen_386

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Profile"

# Begin Custom Build
OutDir=.\Profile
InputPath=.\gen_386.asm
InputName=gen_386

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Debug P6"

# Begin Custom Build
OutDir=.\Debug P6
InputPath=.\gen_386.asm
InputName=gen_386

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Profile P6"

# Begin Custom Build
OutDir=.\Profile P6
InputPath=.\gen_386.asm
InputName=gen_386

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Release P6"

# Begin Custom Build
OutDir=.\Release P6
InputPath=.\gen_386.asm
InputName=gen_386

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Release (Proton)"

# Begin Custom Build
OutDir=.\Release Proton
InputPath=.\gen_386.asm
InputName=gen_386

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Debug (Proton)"

# Begin Custom Build
OutDir=.\Debug Proton
InputPath=.\gen_386.asm
InputName=gen_386

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Profile (Proton)"

# Begin Custom Build
OutDir=.\Profile Proton
InputPath=.\gen_386.asm
InputName=gen_386

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Debug P6 (Proton)"

# Begin Custom Build
OutDir=.\Debug P6 Proton
InputPath=.\gen_386.asm
InputName=gen_386

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Profile P6 (Proton)"

# Begin Custom Build
OutDir=.\Profile P6 Proton
InputPath=.\gen_386.asm
InputName=gen_386

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Release P6 (Proton)"

# Begin Custom Build
OutDir=.\Release P6 Proton
InputPath=.\gen_386.asm
InputName=gen_386

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\geometry.asm
USERDEP__GEOME="drv.h"	"drv.inc"	

!IF  "$(CFG)" == "softrend - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=.\geometry.asm
InputName=geometry

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=.\geometry.asm
InputName=geometry

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Profile"

# Begin Custom Build
OutDir=.\Profile
InputPath=.\geometry.asm
InputName=geometry

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Debug P6"

# Begin Custom Build
OutDir=.\Debug P6
InputPath=.\geometry.asm
InputName=geometry

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /Zi /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Profile P6"

# Begin Custom Build
OutDir=.\Profile P6
InputPath=.\geometry.asm
InputName=geometry

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Release P6"

# Begin Custom Build
OutDir=.\Release P6
InputPath=.\geometry.asm
InputName=geometry

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Release (Proton)"

# Begin Custom Build
OutDir=.\Release Proton
InputPath=.\geometry.asm
InputName=geometry

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Debug (Proton)"

# Begin Custom Build
OutDir=.\Debug Proton
InputPath=.\geometry.asm
InputName=geometry

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Profile (Proton)"

# Begin Custom Build
OutDir=.\Profile Proton
InputPath=.\geometry.asm
InputName=geometry

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Debug P6 (Proton)"

# Begin Custom Build
OutDir=.\Debug P6 Proton
InputPath=.\geometry.asm
InputName=geometry

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Profile P6 (Proton)"

# Begin Custom Build
OutDir=.\Profile P6 Proton
InputPath=.\geometry.asm
InputName=geometry

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Release P6 (Proton)"

# Begin Custom Build
OutDir=.\Release P6 Proton
InputPath=.\geometry.asm
InputName=geometry

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\magicsym.asm

!IF  "$(CFG)" == "softrend - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=.\magicsym.asm
InputName=magicsym

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=.\magicsym.asm
InputName=magicsym

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Profile"

# Begin Custom Build
OutDir=.\Profile
InputPath=.\magicsym.asm
InputName=magicsym

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Debug P6"

# Begin Custom Build
OutDir=.\Debug P6
InputPath=.\magicsym.asm
InputName=magicsym

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Profile P6"

# Begin Custom Build
OutDir=.\Profile P6
InputPath=.\magicsym.asm
InputName=magicsym

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Release P6"

# Begin Custom Build
OutDir=.\Release P6
InputPath=.\magicsym.asm
InputName=magicsym

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Release (Proton)"

# Begin Custom Build
OutDir=.\Release Proton
InputPath=.\magicsym.asm
InputName=magicsym

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Debug (Proton)"

# Begin Custom Build
OutDir=.\Debug Proton
InputPath=.\magicsym.asm
InputName=magicsym

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Profile (Proton)"

# Begin Custom Build
OutDir=.\Profile Proton
InputPath=.\magicsym.asm
InputName=magicsym

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Debug P6 (Proton)"

# Begin Custom Build
OutDir=.\Debug P6 Proton
InputPath=.\magicsym.asm
InputName=magicsym

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Profile P6 (Proton)"

# Begin Custom Build
OutDir=.\Profile P6 Proton
InputPath=.\magicsym.asm
InputName=magicsym

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Release P6 (Proton)"

# Begin Custom Build
OutDir=.\Release P6 Proton
InputPath=.\magicsym.asm
InputName=magicsym

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\subdiv.asm
USERDEP__SUBDI="drv.h"	"drv.inc"	

!IF  "$(CFG)" == "softrend - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=.\subdiv.asm
InputName=subdiv

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo -DBASED_FLOAT=1 /Fo"$(OutDir)\$(InputName).obj"\
                 $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=.\subdiv.asm
InputName=subdiv

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo -DBASED_FLOAT=1 /Fo"$(OutDir)\$(InputName).obj"\
                 $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Profile"

# Begin Custom Build
OutDir=.\Profile
InputPath=.\subdiv.asm
InputName=subdiv

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo -DBASED_FLOAT=1 /Fo"$(OutDir)\$(InputName).obj"\
                 $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Debug P6"

# Begin Custom Build
OutDir=.\Debug P6
InputPath=.\subdiv.asm
InputName=subdiv

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo -DBASED_FLOAT=1 /Fo"$(OutDir)\$(InputName).obj"\
                 $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Profile P6"

# Begin Custom Build
OutDir=.\Profile P6
InputPath=.\subdiv.asm
InputName=subdiv

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo -DBASED_FLOAT=1 /Fo"$(OutDir)\$(InputName).obj"\
                 $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Release P6"

# Begin Custom Build
OutDir=.\Release P6
InputPath=.\subdiv.asm
InputName=subdiv

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo -DBASED_FLOAT=1 /Fo"$(OutDir)\$(InputName).obj"\
                 $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Release (Proton)"

# Begin Custom Build
OutDir=.\Release Proton
InputPath=.\subdiv.asm
InputName=subdiv

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo -DBASED_FLOAT=1 /Fo"$(OutDir)\$(InputName).obj"\
                 $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Debug (Proton)"

# Begin Custom Build
OutDir=.\Debug Proton
InputPath=.\subdiv.asm
InputName=subdiv

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo -DBASED_FLOAT=1 /Fo"$(OutDir)\$(InputName).obj"\
                 $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Profile (Proton)"

# Begin Custom Build
OutDir=.\Profile Proton
InputPath=.\subdiv.asm
InputName=subdiv

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo -DBASED_FLOAT=1 /Fo"$(OutDir)\$(InputName).obj"\
                 $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Debug P6 (Proton)"

# Begin Custom Build
OutDir=.\Debug P6 Proton
InputPath=.\subdiv.asm
InputName=subdiv

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo -DBASED_FLOAT=1 /Fo"$(OutDir)\$(InputName).obj"\
                 $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Profile P6 (Proton)"

# Begin Custom Build
OutDir=.\Profile P6 Proton
InputPath=.\subdiv.asm
InputName=subdiv

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo -DBASED_FLOAT=1 /Fo"$(OutDir)\$(InputName).obj"\
                 $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Release P6 (Proton)"

# Begin Custom Build
OutDir=.\Release P6 Proton
InputPath=.\subdiv.asm
InputName=subdiv

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo -DBASED_FLOAT=1 /Fo"$(OutDir)\$(InputName).obj"\
                 $(InputName).asm

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\v1m_386.asm
USERDEP__V1M_3="drv.h"	"drv.inc"	

!IF  "$(CFG)" == "softrend - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=.\v1m_386.asm
InputName=v1m_386

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=.\v1m_386.asm
InputName=v1m_386

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Profile"

# Begin Custom Build
OutDir=.\Profile
InputPath=.\v1m_386.asm
InputName=v1m_386

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Debug P6"

# Begin Custom Build
OutDir=.\Debug P6
InputPath=.\v1m_386.asm
InputName=v1m_386

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Profile P6"

# Begin Custom Build
OutDir=.\Profile P6
InputPath=.\v1m_386.asm
InputName=v1m_386

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Release P6"

# Begin Custom Build
OutDir=.\Release P6
InputPath=.\v1m_386.asm
InputName=v1m_386

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Release (Proton)"

# Begin Custom Build
OutDir=.\Release Proton
InputPath=.\v1m_386.asm
InputName=v1m_386

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Debug (Proton)"

# Begin Custom Build
OutDir=.\Debug Proton
InputPath=.\v1m_386.asm
InputName=v1m_386

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Profile (Proton)"

# Begin Custom Build
OutDir=.\Profile Proton
InputPath=.\v1m_386.asm
InputName=v1m_386

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Debug P6 (Proton)"

# Begin Custom Build
OutDir=.\Debug P6 Proton
InputPath=.\v1m_386.asm
InputName=v1m_386

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Profile P6 (Proton)"

# Begin Custom Build
OutDir=.\Profile P6 Proton
InputPath=.\v1m_386.asm
InputName=v1m_386

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "softrend - Win32 Release P6 (Proton)"

# Begin Custom Build
OutDir=.\Release P6 Proton
InputPath=.\v1m_386.asm
InputName=v1m_386

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ENDIF 

# End Source File
# End Group
# Begin Group "Assembly Includes"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\586_macs.inc
# End Source File
# Begin Source File

SOURCE=.\klamath.inc
# End Source File
# Begin Source File

SOURCE=.\vertex.inc
# End Source File
# End Group
# End Target
# End Project
