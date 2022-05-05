# Microsoft Developer Studio Project File - Name="pixelmap" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=pixelmap - Win32 Debug P6
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "pixelmap.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "pixelmap.mak" CFG="pixelmap - Win32 Debug P6"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "pixelmap - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "pixelmap - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "pixelmap - Win32 Profile" (based on "Win32 (x86) Static Library")
!MESSAGE "pixelmap - Win32 Debug P6" (based on "Win32 (x86) Static Library")
!MESSAGE "pixelmap - Win32 Profile P6" (based on "Win32 (x86) Static Library")
!MESSAGE "pixelmap - Win32 Release P6" (based on "Win32 (x86) Static Library")
!MESSAGE "pixelmap - Win32 Release (Proton)" (based on "Win32 (x86) Static Library")
!MESSAGE "pixelmap - Win32 Debug (Proton)" (based on "Win32 (x86) Static Library")
!MESSAGE "pixelmap - Win32 Profile (Proton)" (based on "Win32 (x86) Static Library")
!MESSAGE "pixelmap - Win32 Debug P6 (Proton)" (based on "Win32 (x86) Static Library")
!MESSAGE "pixelmap - Win32 Profile P6 (Proton)" (based on "Win32 (x86) Static Library")
!MESSAGE "pixelmap - Win32 Release P6 (Proton)" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""$/brender/pixelmap", AGAAAAAA"
# PROP Scc_LocalPath "."
CPP=xicl.exe

!IF  "$(CFG)" == "pixelmap - Win32 Release"

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
# ADD CPP /nologo /G5 /W3 /GX /O2 /D __BR_V1DB__=0 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D DEBUG=0 /D PARANOID=0 /D EVAL=0 /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /YX /FD /c
# ADD CPP /D_USE_NON_INTEL_COMPILER
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "pixelmap - Win32 Debug"

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
# ADD CPP /nologo /G5 /W3 /GX /Z7 /Od /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /D "PARANOID-1" /D __BR_V1DB__=0 /D DEBUG=1 /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D PARANOID=1 /D EVAL=0 /D STATIC= /FR /YX /FD /c
# ADD CPP /D_USE_NON_INTEL_COMPILER
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "pixelmap - Win32 Profile"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "pixelmap"
# PROP BASE Intermediate_Dir "pixelmap"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Profile"
# PROP Intermediate_Dir "Profile"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D __BR_V1DB__=0 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D DEBUG=0 /D PARANOID=0 /D EVAL=0 /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /YX /FD /c
# ADD CPP /nologo /G5 /W3 /GX /Zi /O2 /D __BR_V1DB__=0 /D DEBUG=0 /D PARANOID=0 /D EVAL=0 /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /YX /FD /c
# ADD CPP /D_USE_NON_INTEL_COMPILER
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "pixelmap - Win32 Debug P6"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "pixelmap"
# PROP BASE Intermediate_Dir "pixelmap"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug P6"
# PROP Intermediate_Dir "Debug P6"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /W3 /GX /Z7 /Od /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /D "PARANOID-1" /D __BR_V1DB__=0 /D DEBUG=1 /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D PARANOID=1 /D EVAL=0 /D STATIC= /FR /YX /FD /c
# ADD CPP /nologo /G6 /W3 /GX /Zi /Od /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /D __BR_V1DB__=0 /D DEBUG=1 /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D PARANOID=1 /D EVAL=0 /D STATIC= /FR /YX /FD /c
# ADD CPP /D_USE_NON_INTEL_COMPILER
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "pixelmap - Win32 Profile P6"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "pixelma0"
# PROP BASE Intermediate_Dir "pixelma0"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Profile P6"
# PROP Intermediate_Dir "Profile P6"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /W3 /GX /Zi /O2 /D __BR_V1DB__=0 /D DEBUG=0 /D PARANOID=0 /D EVAL=0 /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /YX /FD /c
# ADD CPP /nologo /G6 /W3 /GX /Zi /O2 /D __BR_V1DB__=0 /D DEBUG=0 /D PARANOID=0 /D EVAL=0 /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /YX /FD /c
# ADD CPP /D_USE_NON_INTEL_COMPILER
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "pixelmap - Win32 Release P6"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "pixelma1"
# PROP BASE Intermediate_Dir "pixelma1"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release P6"
# PROP Intermediate_Dir "Release P6"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /W3 /GX /O2 /D __BR_V1DB__=0 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D DEBUG=0 /D PARANOID=0 /D EVAL=0 /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /YX /FD /c
# ADD CPP /nologo /G6 /W3 /GX /O2 /D __BR_V1DB__=0 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D DEBUG=0 /D PARANOID=0 /D EVAL=0 /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /YX /FD /c
# ADD CPP /D_USE_NON_INTEL_COMPILER
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "pixelmap - Win32 Release (Proton)"

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
# ADD CPP /nologo /G5 /W3 /GX /O2 /D __BR_V1DB__=0 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D DEBUG=0 /D PARANOID=0 /D EVAL=0 /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /YX /FD /c
# ADD CPP /D_USE_INTEL_COMPILER
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "pixelmap - Win32 Debug (Proton)"

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
# ADD CPP /nologo /G5 /W3 /GX /Z7 /Od /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /D "PARANOID-1" /D __BR_V1DB__=0 /D DEBUG=1 /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D PARANOID=1 /D EVAL=0 /D STATIC= /FR /YX /FD /c
# ADD CPP /D_USE_INTEL_COMPILER
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "pixelmap - Win32 Profile (Proton)"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "pixelmap"
# PROP BASE Intermediate_Dir "pixelmap"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Profile Proton"
# PROP Intermediate_Dir "Profile Proton"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D __BR_V1DB__=0 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D DEBUG=0 /D PARANOID=0 /D EVAL=0 /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /YX /FD /c
# ADD CPP /nologo /G5 /W3 /GX /Zi /O2 /D __BR_V1DB__=0 /D DEBUG=0 /D PARANOID=0 /D EVAL=0 /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /YX /FD /c
# ADD CPP /D_USE_INTEL_COMPILER
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "pixelmap - Win32 Debug P6 (Proton)"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "pixelmap"
# PROP BASE Intermediate_Dir "pixelmap"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug P6 Proton"
# PROP Intermediate_Dir "Debug P6 Proton"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /W3 /GX /Z7 /Od /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /D "PARANOID-1" /D __BR_V1DB__=0 /D DEBUG=1 /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D PARANOID=1 /D EVAL=0 /D STATIC= /FR /YX /FD /c
# ADD CPP /nologo /G6 /W3 /GX /Zi /Od /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /D __BR_V1DB__=0 /D DEBUG=1 /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D PARANOID=1 /D EVAL=0 /D STATIC= /FR /YX /FD /c
# ADD CPP /D_USE_INTEL_COMPILER
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "pixelmap - Win32 Profile P6 (Proton)"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "pixelma0"
# PROP BASE Intermediate_Dir "pixelma0"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Profile P6 Proton"
# PROP Intermediate_Dir "Profile P6 Proton"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /W3 /GX /Zi /O2 /D __BR_V1DB__=0 /D DEBUG=0 /D PARANOID=0 /D EVAL=0 /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /YX /FD /c
# ADD CPP /nologo /G6 /W3 /GX /Zi /O2 /D __BR_V1DB__=0 /D DEBUG=0 /D PARANOID=0 /D EVAL=0 /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /YX /FD /c
# ADD CPP /D_USE_INTEL_COMPILER
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "pixelmap - Win32 Release P6 (Proton)"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "pixelma1"
# PROP BASE Intermediate_Dir "pixelma1"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release P6 Proton"
# PROP Intermediate_Dir "Release P6 Proton"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /W3 /GX /O2 /D __BR_V1DB__=0 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D DEBUG=0 /D PARANOID=0 /D EVAL=0 /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /YX /FD /c
# ADD CPP /nologo /G6 /W3 /GX /O2 /D __BR_V1DB__=0 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D DEBUG=0 /D PARANOID=0 /D EVAL=0 /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /YX /FD /c
# ADD CPP /D_USE_INTEL_COMPILER
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "pixelmap - Win32 Release"
# Name "pixelmap - Win32 Debug"
# Name "pixelmap - Win32 Profile"
# Name "pixelmap - Win32 Debug P6"
# Name "pixelmap - Win32 Profile P6"
# Name "pixelmap - Win32 Release P6"
# Name "pixelmap - Win32 Release (Proton)"
# Name "pixelmap - Win32 Debug (Proton)"
# Name "pixelmap - Win32 Profile (Proton)"
# Name "pixelmap - Win32 Debug P6 (Proton)"
# Name "pixelmap - Win32 Profile P6 (Proton)"
# Name "pixelmap - Win32 Release P6 (Proton)"
# Begin Group "C Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\fontptrs.c
# End Source File
# Begin Source File

SOURCE=.\genclip.c
# End Source File
# Begin Source File

SOURCE=.\gencopy.c
# End Source File
# Begin Source File

SOURCE=.\mipsupt.c
# End Source File
# Begin Source File

SOURCE=.\pixelmap.c
# End Source File
# Begin Source File

SOURCE=.\pmdsptch.c
# End Source File
# Begin Source File

SOURCE=.\pmfile.c
# End Source File
# Begin Source File

SOURCE=.\pmgen.c
# End Source File
# Begin Source File

SOURCE=.\pmmem.c
# End Source File
# Begin Source File

SOURCE=.\pmnull.c
# End Source File
# Begin Source File

SOURCE=.\pmsetup.c
# End Source File
# Begin Source File

SOURCE=.\quantize.c
# End Source File
# Begin Source File

SOURCE=.\scale.c
# End Source File
# End Group
# Begin Group "Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\pm.h
# End Source File
# Begin Source File

SOURCE=.\pm_ip.h
# End Source File
# Begin Source File

SOURCE=.\pmmem.h
# End Source File
# End Group
# Begin Group "Perl Scripts"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\resgen.pl
# End Source File
# End Group
# Begin Group "Miscellaneous"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\pmimg.fns
USERDEP__PMIMG="..\resgen.pl"	

!IF  "$(CFG)" == "pixelmap - Win32 Release"

# Begin Custom Build
InputPath=.\pmimg.fns

BuildCmds= \
	perl ..\resgen.pl pmimg.h _img.def <pmimg.fns \
	lib /def:_img.def /nologo /machine:IX86 /out:pmimg.lib \
	del pmimg.exp \
	del _img.def \
	

"pmimg.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"pmimg.lib" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "pixelmap - Win32 Debug"

# Begin Custom Build
InputPath=.\pmimg.fns

BuildCmds= \
	perl ..\resgen.pl pmimg.h _img.def <pmimg.fns \
	lib /def:_img.def /nologo /machine:IX86 /out:pmimg.lib \
	del pmimg.exp \
	del _img.def \
	

"pmimg.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"pmimg.lib" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "pixelmap - Win32 Profile"

# Begin Custom Build
InputPath=.\pmimg.fns

BuildCmds= \
	perl ..\resgen.pl pmimg.h _img.def <pmimg.fns \
	lib /def:_img.def /nologo /machine:IX86 /out:pmimg.lib \
	del pmimg.exp \
	del _img.def \
	

"pmimg.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"pmimg.lib" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "pixelmap - Win32 Debug P6"

# Begin Custom Build
InputPath=.\pmimg.fns

BuildCmds= \
	perl ..\resgen.pl pmimg.h _img.def <pmimg.fns \
	lib /def:_img.def /nologo /machine:IX86 /out:pmimg.lib \
	del pmimg.exp \
	del _img.def \
	

"pmimg.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"pmimg.lib" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "pixelmap - Win32 Profile P6"

# Begin Custom Build
InputPath=.\pmimg.fns

BuildCmds= \
	perl ..\resgen.pl pmimg.h _img.def <pmimg.fns \
	lib /def:_img.def /nologo /machine:IX86 /out:pmimg.lib \
	del pmimg.exp \
	del _img.def \
	

"pmimg.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"pmimg.lib" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "pixelmap - Win32 Release P6"

# Begin Custom Build
InputPath=.\pmimg.fns

BuildCmds= \
	perl ..\resgen.pl pmimg.h _img.def <pmimg.fns \
	lib /def:_img.def /nologo /machine:IX86 /out:pmimg.lib \
	del pmimg.exp \
	del _img.def \
	

"pmimg.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"pmimg.lib" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "pixelmap - Win32 Release (Proton)"

# Begin Custom Build
InputPath=.\pmimg.fns

BuildCmds= \
	perl ..\resgen.pl pmimg.h _img.def <pmimg.fns \
	lib /def:_img.def /nologo /machine:IX86 /out:pmimg.lib \
	del pmimg.exp \
	del _img.def \
	

"pmimg.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"pmimg.lib" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "pixelmap - Win32 Debug (Proton)"

# Begin Custom Build
InputPath=.\pmimg.fns

BuildCmds= \
	perl ..\resgen.pl pmimg.h _img.def <pmimg.fns \
	lib /def:_img.def /nologo /machine:IX86 /out:pmimg.lib \
	del pmimg.exp \
	del _img.def \
	

"pmimg.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"pmimg.lib" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "pixelmap - Win32 Profile (Proton)"

# Begin Custom Build
InputPath=.\pmimg.fns

BuildCmds= \
	perl ..\resgen.pl pmimg.h _img.def <pmimg.fns \
	lib /def:_img.def /nologo /machine:IX86 /out:pmimg.lib \
	del pmimg.exp \
	del _img.def \
	

"pmimg.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"pmimg.lib" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "pixelmap - Win32 Debug P6 (Proton)"

# Begin Custom Build
InputPath=.\pmimg.fns

BuildCmds= \
	perl ..\resgen.pl pmimg.h _img.def <pmimg.fns \
	lib /def:_img.def /nologo /machine:IX86 /out:pmimg.lib \
	del pmimg.exp \
	del _img.def \
	

"pmimg.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"pmimg.lib" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "pixelmap - Win32 Profile P6 (Proton)"

# Begin Custom Build
InputPath=.\pmimg.fns

BuildCmds= \
	perl ..\resgen.pl pmimg.h _img.def <pmimg.fns \
	lib /def:_img.def /nologo /machine:IX86 /out:pmimg.lib \
	del pmimg.exp \
	del _img.def \
	

"pmimg.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"pmimg.lib" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "pixelmap - Win32 Release P6 (Proton)"

# Begin Custom Build
InputPath=.\pmimg.fns

BuildCmds= \
	perl ..\resgen.pl pmimg.h _img.def <pmimg.fns \
	lib /def:_img.def /nologo /machine:IX86 /out:pmimg.lib \
	del pmimg.exp \
	del _img.def \
	

"pmimg.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"pmimg.lib" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# End Group
# Begin Group "Assembly"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\fnt3x5.asm

!IF  "$(CFG)" == "pixelmap - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=.\fnt3x5.asm
InputName=fnt3x5

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /Cp /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pixelmap - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=.\fnt3x5.asm
InputName=fnt3x5

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /Cp /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pixelmap - Win32 Profile"

# Begin Custom Build
OutDir=.\Profile
InputPath=.\fnt3x5.asm
InputName=fnt3x5

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /Cp /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pixelmap - Win32 Debug P6"

# Begin Custom Build
OutDir=.\Debug P6
InputPath=.\fnt3x5.asm
InputName=fnt3x5

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /Cp /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pixelmap - Win32 Profile P6"

# Begin Custom Build
OutDir=.\Profile P6
InputPath=.\fnt3x5.asm
InputName=fnt3x5

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /Cp /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pixelmap - Win32 Release P6"

# Begin Custom Build
OutDir=.\Release P6
InputPath=.\fnt3x5.asm
InputName=fnt3x5

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /Cp /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pixelmap - Win32 Release (Proton)"

# Begin Custom Build
OutDir=.\Release
InputPath=.\fnt3x5.asm
InputName=fnt3x5

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /Cp /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pixelmap - Win32 Debug (Proton)"

# Begin Custom Build
OutDir=.\Debug
InputPath=.\fnt3x5.asm
InputName=fnt3x5

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /Cp /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pixelmap - Win32 Profile (Proton)"

# Begin Custom Build
OutDir=.\Profile
InputPath=.\fnt3x5.asm
InputName=fnt3x5

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /Cp /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pixelmap - Win32 Debug P6 (Proton)"

# Begin Custom Build
OutDir=.\Debug P6
InputPath=.\fnt3x5.asm
InputName=fnt3x5

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /Cp /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pixelmap - Win32 Profile P6 (Proton)"

# Begin Custom Build
OutDir=.\Profile P6
InputPath=.\fnt3x5.asm
InputName=fnt3x5

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /Cp /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pixelmap - Win32 Release P6 (Proton)"

# Begin Custom Build
OutDir=.\Release P6
InputPath=.\fnt3x5.asm
InputName=fnt3x5

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /Cp /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\fntp4x6.asm

!IF  "$(CFG)" == "pixelmap - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=.\fntp4x6.asm
InputName=fntp4x6

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /Cp /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pixelmap - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=.\fntp4x6.asm
InputName=fntp4x6

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /Cp /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pixelmap - Win32 Profile"

# Begin Custom Build
OutDir=.\Profile
InputPath=.\fntp4x6.asm
InputName=fntp4x6

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /Cp /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pixelmap - Win32 Debug P6"

# Begin Custom Build
OutDir=.\Debug P6
InputPath=.\fntp4x6.asm
InputName=fntp4x6

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /Cp /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pixelmap - Win32 Profile P6"

# Begin Custom Build
OutDir=.\Profile P6
InputPath=.\fntp4x6.asm
InputName=fntp4x6

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /Cp /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pixelmap - Win32 Release P6"

# Begin Custom Build
OutDir=.\Release P6
InputPath=.\fntp4x6.asm
InputName=fntp4x6

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /Cp /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pixelmap - Win32 Release (Proton)"

# Begin Custom Build
OutDir=.\Release
InputPath=.\fntp4x6.asm
InputName=fntp4x6

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /Cp /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pixelmap - Win32 Debug (Proton)"

# Begin Custom Build
OutDir=.\Debug
InputPath=.\fntp4x6.asm
InputName=fntp4x6

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /Cp /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pixelmap - Win32 Profile (Proton)"

# Begin Custom Build
OutDir=.\Profile
InputPath=.\fntp4x6.asm
InputName=fntp4x6

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /Cp /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pixelmap - Win32 Debug P6 (Proton)"

# Begin Custom Build
OutDir=.\Debug P6
InputPath=.\fntp4x6.asm
InputName=fntp4x6

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /Cp /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pixelmap - Win32 Profile P6 (Proton)"

# Begin Custom Build
OutDir=.\Profile P6
InputPath=.\fntp4x6.asm
InputName=fntp4x6

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /Cp /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pixelmap - Win32 Release P6 (Proton)"

# Begin Custom Build
OutDir=.\Release P6
InputPath=.\fntp4x6.asm
InputName=fntp4x6

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /Cp /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\fntp7x9.asm

!IF  "$(CFG)" == "pixelmap - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=.\fntp7x9.asm
InputName=fntp7x9

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /Cp /nologo /Fo"$(OutDir)\$(InputName).obj " $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pixelmap - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=.\fntp7x9.asm
InputName=fntp7x9

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /Cp /nologo /Fo"$(OutDir)\$(InputName).obj " $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pixelmap - Win32 Profile"

# Begin Custom Build
OutDir=.\Profile
InputPath=.\fntp7x9.asm
InputName=fntp7x9

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /Cp /nologo /Fo"$(OutDir)\$(InputName).obj " $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pixelmap - Win32 Debug P6"

# Begin Custom Build
OutDir=.\Debug P6
InputPath=.\fntp7x9.asm
InputName=fntp7x9

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /Cp /nologo /Fo"$(OutDir)\$(InputName).obj " $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pixelmap - Win32 Profile P6"

# Begin Custom Build
OutDir=.\Profile P6
InputPath=.\fntp7x9.asm
InputName=fntp7x9

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /Cp /nologo /Fo"$(OutDir)\$(InputName).obj " $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pixelmap - Win32 Release P6"

# Begin Custom Build
OutDir=.\Release P6
InputPath=.\fntp7x9.asm
InputName=fntp7x9

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /Cp /nologo /Fo"$(OutDir)\$(InputName).obj " $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pixelmap - Win32 Release (Proton)"

# Begin Custom Build
OutDir=.\Release
InputPath=.\fntp7x9.asm
InputName=fntp7x9

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /Cp /nologo /Fo"$(OutDir)\$(InputName).obj " $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pixelmap - Win32 Debug (Proton)"

# Begin Custom Build
OutDir=.\Debug
InputPath=.\fntp7x9.asm
InputName=fntp7x9

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /Cp /nologo /Fo"$(OutDir)\$(InputName).obj " $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pixelmap - Win32 Profile (Proton)"

# Begin Custom Build
OutDir=.\Profile
InputPath=.\fntp7x9.asm
InputName=fntp7x9

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /Cp /nologo /Fo"$(OutDir)\$(InputName).obj " $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pixelmap - Win32 Debug P6 (Proton)"

# Begin Custom Build
OutDir=.\Debug P6
InputPath=.\fntp7x9.asm
InputName=fntp7x9

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /Cp /nologo /Fo"$(OutDir)\$(InputName).obj " $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pixelmap - Win32 Profile P6 (Proton)"

# Begin Custom Build
OutDir=.\Profile P6
InputPath=.\fntp7x9.asm
InputName=fntp7x9

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /Cp /nologo /Fo"$(OutDir)\$(InputName).obj " $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pixelmap - Win32 Release P6 (Proton)"

# Begin Custom Build
OutDir=.\Release P6
InputPath=.\fntp7x9.asm
InputName=fntp7x9

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /Cp /nologo /Fo"$(OutDir)\$(InputName).obj " $(InputName).asm

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\memloops.asm

!IF  "$(CFG)" == "pixelmap - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=.\memloops.asm
InputName=memloops

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pixelmap - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=.\memloops.asm
InputName=memloops

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pixelmap - Win32 Profile"

# Begin Custom Build
OutDir=.\Profile
InputPath=.\memloops.asm
InputName=memloops

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pixelmap - Win32 Debug P6"

# Begin Custom Build
OutDir=.\Debug P6
InputPath=.\memloops.asm
InputName=memloops

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pixelmap - Win32 Profile P6"

# Begin Custom Build
OutDir=.\Profile P6
InputPath=.\memloops.asm
InputName=memloops

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pixelmap - Win32 Release P6"

# Begin Custom Build
OutDir=.\Release P6
InputPath=.\memloops.asm
InputName=memloops

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pixelmap - Win32 Release (Proton)"

# Begin Custom Build
OutDir=.\Release
InputPath=.\memloops.asm
InputName=memloops

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pixelmap - Win32 Debug (Proton)"

# Begin Custom Build
OutDir=.\Debug
InputPath=.\memloops.asm
InputName=memloops

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pixelmap - Win32 Profile (Proton)"

# Begin Custom Build
OutDir=.\Profile
InputPath=.\memloops.asm
InputName=memloops

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pixelmap - Win32 Debug P6 (Proton)"

# Begin Custom Build
OutDir=.\Debug P6
InputPath=.\memloops.asm
InputName=memloops

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pixelmap - Win32 Profile P6 (Proton)"

# Begin Custom Build
OutDir=.\Profile P6
InputPath=.\memloops.asm
InputName=memloops

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pixelmap - Win32 Release P6 (Proton)"

# Begin Custom Build
OutDir=.\Release P6
InputPath=.\memloops.asm
InputName=memloops

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /coff /nologo /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ENDIF 

# End Source File
# End Group
# Begin Group "Assembly Includes"

# PROP Default_Filter ""
# End Group
# End Target
# End Project
