# Microsoft Developer Studio Project File - Name="v1db" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=v1db - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "v1db.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "v1db.mak" CFG="v1db - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "v1db - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "v1db - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "v1db - Win32 Profile" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/brender/v1db", XGAAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "v1db - Win32 Release"

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
# ADD CPP /nologo /G5 /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D DEBUG=0 /D PARANOID=0 /D EVAL=0 /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /D "_USE_NON_INTEL_COMPILER" /YX /FD /c
# ADD BASE RSC /l 0xc09
# ADD RSC /l 0xc09
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "v1db - Win32 Debug"

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
# ADD CPP /nologo /G5 /W3 /GX /Z7 /Od /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /D "PARANOID-1" /D DEBUG=1 /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D PARANOID=1 /D EVAL=0 /D STATIC= /D "_USE_NON_INTEL_COMPILER" /FR /YX /FD /c
# ADD BASE RSC /l 0xc09
# ADD RSC /l 0xc09
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "v1db - Win32 Profile"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "v1db___W"
# PROP BASE Intermediate_Dir "v1db___W"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Profile"
# PROP Intermediate_Dir "Profile"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D DEBUG=0 /D PARANOID=0 /D EVAL=0 /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /YX /FD /c
# ADD CPP /nologo /G5 /W3 /GX /Zi /O2 /D DEBUG=0 /D PARANOID=0 /D EVAL=0 /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USE_NON_INTEL_COMPILER" /FR /YX /FD /c
# ADD BASE RSC /l 0xc09
# ADD RSC /l 0xc09
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "v1db - Win32 Release"
# Name "v1db - Win32 Debug"
# Name "v1db - Win32 Profile"
# Begin Group "C Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\actsupt.c
# End Source File
# Begin Source File

SOURCE=.\custsupt.c
# End Source File
# Begin Source File

SOURCE=.\dbquery.c
# End Source File
# Begin Source File

SOURCE=.\dbsetup.c
# End Source File
# Begin Source File

SOURCE=.\def_mat.c
# End Source File
# Begin Source File

SOURCE=.\def_mdl.c
# End Source File
# Begin Source File

SOURCE=.\def_otab.c
# End Source File
# Begin Source File

SOURCE=.\enables.c
# End Source File
# Begin Source File

SOURCE=.\ffhooks.c
# End Source File
# Begin Source File

SOURCE=.\matsupt.c
# End Source File
# Begin Source File

SOURCE=.\modrend.c
# End Source File
# Begin Source File

SOURCE=.\modsupt.c
# End Source File
# Begin Source File

SOURCE=.\otable.c
# End Source File
# Begin Source File

SOURCE=.\pick.c
# End Source File
# Begin Source File

SOURCE=.\prelight.c
# End Source File
# Begin Source File

SOURCE=.\prepmap.c
# End Source File
# Begin Source File

SOURCE=.\prepmatl.c
# End Source File
# Begin Source File

SOURCE=.\prepmesh.c
# End Source File
# Begin Source File

SOURCE=.\preptab.c
# End Source File
# Begin Source File

SOURCE=.\regsupt.c
# End Source File
# Begin Source File

SOURCE=.\render.c
# End Source File
# Begin Source File

SOURCE=.\v1dbfile.c
# End Source File
# End Group
# Begin Group "Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\horizon.h
# End Source File
# Begin Source File

SOURCE=.\v1db.h
# End Source File
# Begin Source File

SOURCE=.\v1db_ip.h
# End Source File
# Begin Source File

SOURCE=.\zsmacro.h
# End Source File
# End Group
# Begin Group "Perl Scripts"

# PROP Default_Filter ""
# End Group
# Begin Group "Miscellaneous"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\v1db_ix.afn

!IF  "$(CFG)" == "v1db - Win32 Release"

USERDEP__V1DB_="..\genasmpr.pl"	
# Begin Custom Build
InputPath=.\v1db_ix.afn

"v1db_ix.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl ..\genasmpr.pl <v1db_ix.afn >v1db_ix.h

# End Custom Build

!ELSEIF  "$(CFG)" == "v1db - Win32 Debug"

USERDEP__V1DB_="..\genasmpr.pl"	
# Begin Custom Build
InputPath=.\v1db_ix.afn

"v1db_ix.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl ..\genasmpr.pl <v1db_ix.afn >v1db_ix.h

# End Custom Build

!ELSEIF  "$(CFG)" == "v1db - Win32 Profile"

USERDEP__V1DB_="..\genasmpr.pl"	
# Begin Custom Build
InputPath=.\v1db_ix.afn

"v1db_ix.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl ..\genasmpr.pl <v1db_ix.afn >v1db_ix.h

# End Custom Build

!ENDIF 

# End Source File
# End Group
# Begin Group "Assembly"

# PROP Default_Filter ""
# End Group
# Begin Group "Assembly Includes"

# PROP Default_Filter ""
# End Group
# End Target
# End Project
