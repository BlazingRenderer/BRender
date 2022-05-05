# Microsoft Developer Studio Project File - Name="host" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=host - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "host.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "host.mak" CFG="host - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "host - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "host - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "host - Win32 Profile" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/brender/host", AIAAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "host - Win32 Release"

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
# ADD CPP /nologo /G5 /W3 /GX /O2 /D __WIN_32__=1 /D __BR_V1DB__=0 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D DEBUG=0 /D PARANOID=0 /D EVAL=0 /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /D "_USE_NON_INTEL_COMPILER" /YX /FD /c
# ADD BASE RSC /l 0xc09
# ADD RSC /l 0xc09
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "host - Win32 Debug"

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
# ADD CPP /nologo /G5 /W3 /GX /Z7 /Od /D __WIN_32__=1 /D BASED_FIXED=0 /D BASED_FLOAT=1 /D "PARANOID-1" /D STATIC=static /D __BR_V1DB__=0 /D DEBUG=1 /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D PARANOID=1 /D EVAL=0 /D STATIC= /D "_USE_NON_INTEL_COMPILER" /FR /YX /FD /c
# ADD BASE RSC /l 0xc09
# ADD RSC /l 0xc09
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "host - Win32 Profile"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "host___W"
# PROP BASE Intermediate_Dir "host___W"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Profile"
# PROP Intermediate_Dir "Profile"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D __WIN_32__=1 /D __BR_V1DB__=0 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D DEBUG=0 /D PARANOID=0 /D EVAL=0 /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /YX /FD /c
# ADD CPP /nologo /G5 /W3 /GX /Zi /O2 /D __WIN_32__=1 /D __BR_V1DB__=0 /D DEBUG=0 /D PARANOID=0 /D EVAL=0 /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USE_NON_INTEL_COMPILER" /FR /YX /FD /c
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

# Name "host - Win32 Release"
# Name "host - Win32 Debug"
# Name "host - Win32 Profile"
# Begin Group "C Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\cpu_msvc.c
# End Source File
# Begin Source File

SOURCE=.\CPUInfo.c
# End Source File
# Begin Source File

SOURCE=.\himage.c
# End Source File
# Begin Source File

SOURCE=.\hostcfg.c
# End Source File
# Begin Source File

SOURCE=.\hstsetup.c
# End Source File
# End Group
# Begin Group "Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\cpu.h
# End Source File
# Begin Source File

SOURCE=.\host.h

!IF  "$(CFG)" == "host - Win32 Release"

# PROP Ignore_Default_Tool 1
USERDEP__HOST_="..\h2incfix.pl"	"..\inc\brender.h"	
# Begin Custom Build
InputPath=.\host.h

"host.inc" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	h2inc -nologo -G3 -Zp4 -w -c -WIN32 -D_WIN32 -D__VISUALC -D__H2INC__               -D_NO_PROTOTYPES -Fa_h2inc.tmp host.h 
	perl ..\h2incfix.pl <_h2inc.tmp >host.inc 
	del _h2inc.tmp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "host - Win32 Debug"

!ELSEIF  "$(CFG)" == "host - Win32 Profile"

# PROP Ignore_Default_Tool 1
USERDEP__HOST_="..\h2incfix.pl"	"..\inc\brender.h"	
# Begin Custom Build
InputPath=.\host.h

"host.inc" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	h2inc -nologo -G3 -Zp4 -w -c -WIN32 -D_WIN32 -D__VISUALC -D__H2INC__               -D_NO_PROTOTYPES -Fa_h2inc.tmp host.h 
	perl ..\h2incfix.pl <_h2inc.tmp >host.inc 
	del _h2inc.tmp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\host_ip.h
# End Source File
# Begin Source File

SOURCE=.\host_p.h
# End Source File
# Begin Source File

SOURCE=.\hostimg.h
# End Source File
# End Group
# Begin Group "Perl Scripts"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\h2incfix.pl
# End Source File
# End Group
# Begin Group "Miscellaneous"

# PROP Default_Filter ""
# End Group
# Begin Group "Assembly"

# PROP Default_Filter ""
# End Group
# Begin Group "Assembly Includes"

# PROP Default_Filter ""
# End Group
# End Target
# End Project
