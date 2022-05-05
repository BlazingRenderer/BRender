# Microsoft Developer Studio Project File - Name="pentprim" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=pentprim - Win32 Debug P6
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "pentprim.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "pentprim.mak" CFG="pentprim - Win32 Debug P6"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "pentprim - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "pentprim - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "pentprim - Win32 Profile" (based on "Win32 (x86) Static Library")
!MESSAGE "pentprim - Win32 Debug P6" (based on "Win32 (x86) Static Library")
!MESSAGE "pentprim - Win32 Profile P6" (based on "Win32 (x86) Static Library")
!MESSAGE "pentprim - Win32 Release P6" (based on "Win32 (x86) Static Library")
!MESSAGE "pentprim - Win32 Release (Proton)" (based on\
 "Win32 (x86) Static Library")
!MESSAGE "pentprim - Win32 Debug (Proton)" (based on\
 "Win32 (x86) Static Library")
!MESSAGE "pentprim - Win32 Profile (Proton)" (based on\
 "Win32 (x86) Static Library")
!MESSAGE "pentprim - Win32 Debug P6 (Proton)" (based on\
 "Win32 (x86) Static Library")
!MESSAGE "pentprim - Win32 Profile P6 (Proton)" (based on\
 "Win32 (x86) Static Library")
!MESSAGE "pentprim - Win32 Release P6 (Proton)" (based on\
 "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""$/brender/pentprim", CLAAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe

!IF  "$(CFG)" == "pentprim - Win32 Release"

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
# ADD CPP /nologo /G5 /W3 /GX /O2 /D PARTS=0xffff /D BrDrv1Begin=BrDrv1SoftPrimBegin /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D DEBUG=0 /D PARANOID=0 /D EVAL=0 /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /D "_USE_NON_INTEL_COMPILER" /YX /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug"

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
# ADD CPP /nologo /G5 /W3 /GX /Z7 /Od /D BASED_FIXED=0 /D BASED_FLOAT=1 /D "PARANOID-1" /D STATIC=static /D BrDrv1Begin=BrDrv1SoftPrimBegin /D PARTS=0xffff /D DEBUG=1 /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D PARANOID=1 /D EVAL=0 /D STATIC= /D "_USE_NON_INTEL_COMPILER" /YX /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "pentprim"
# PROP BASE Intermediate_Dir "pentprim"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Profile"
# PROP Intermediate_Dir "Profile"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D PARTS=0xffff /D BrDrv1Begin=BrDrv1SoftPrimBegin /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D DEBUG=0 /D PARANOID=0 /D EVAL=0 /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /YX /FD /c
# ADD CPP /nologo /G5 /W3 /GX /Zi /O2 /D PARTS=0xffff /D BrDrv1Begin=BrDrv1SoftPrimBegin /D DEBUG=0 /D PARANOID=0 /D EVAL=0 /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USE_NON_INTEL_COMPILER" /FR /YX /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "pentprim"
# PROP BASE Intermediate_Dir "pentprim"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug P6"
# PROP Intermediate_Dir "Debug P6"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /W3 /GX /Z7 /Od /D BASED_FIXED=0 /D BASED_FLOAT=1 /D "PARANOID-1" /D STATIC=static /D BrDrv1Begin=BrDrv1SoftPrimBegin /D PARTS=0xffff /D DEBUG=1 /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D PARANOID=1 /D EVAL=0 /D STATIC= /YX /FD /c
# ADD CPP /nologo /G6 /W3 /GX /Zi /Od /D BASED_FIXED=0 /D BASED_FLOAT=1 /D STATIC=static /D BrDrv1Begin=BrDrv1SoftPrimBegin /D PARTS=0xffff /D DEBUG=1 /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D PARANOID=1 /D EVAL=0 /D STATIC= /D "_USE_NON_INTEL_COMPILER" /YX /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "pentpri0"
# PROP BASE Intermediate_Dir "pentpri0"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Profile P6"
# PROP Intermediate_Dir "Profile P6"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /W3 /GX /Zi /O2 /D PARTS=0xffff /D BrDrv1Begin=BrDrv1SoftPrimBegin /D DEBUG=0 /D PARANOID=0 /D EVAL=0 /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /YX /FD /c
# ADD CPP /nologo /G6 /W3 /GX /Zi /O2 /D PARTS=0xffff /D BrDrv1Begin=BrDrv1SoftPrimBegin /D DEBUG=0 /D PARANOID=0 /D EVAL=0 /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USE_NON_INTEL_COMPILER" /FR /YX /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "pentpri1"
# PROP BASE Intermediate_Dir "pentpri1"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release P6"
# PROP Intermediate_Dir "Release P6"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /W3 /GX /O2 /D PARTS=0xffff /D BrDrv1Begin=BrDrv1SoftPrimBegin /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D DEBUG=0 /D PARANOID=0 /D EVAL=0 /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /YX /FD /c
# ADD CPP /nologo /G6 /W3 /GX /O2 /D PARTS=0xffff /D BrDrv1Begin=BrDrv1SoftPrimBegin /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D DEBUG=0 /D PARANOID=0 /D EVAL=0 /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /D "_USE_NON_INTEL_COMPILER" /YX /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release (Proton)"

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
# ADD CPP /nologo /G5 /W3 /GX /O2 /D PARTS=0xffff /D BrDrv1Begin=BrDrv1SoftPrimBegin /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D DEBUG=0 /D PARANOID=0 /D EVAL=0 /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /D "_USE_INTEL_COMPILER" /YX /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug (Proton)"

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
# ADD CPP /nologo /G5 /W3 /GX /Z7 /Od /D BASED_FIXED=0 /D BASED_FLOAT=1 /D "PARANOID-1" /D STATIC=static /D BrDrv1Begin=BrDrv1SoftPrimBegin /D PARTS=0xffff /D DEBUG=1 /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D PARANOID=1 /D EVAL=0 /D STATIC= /D "_USE_INTEL_COMPILER" /YX /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile (Proton)"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "pentprim"
# PROP BASE Intermediate_Dir "pentprim"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Profile Proton"
# PROP Intermediate_Dir "Profile Proton"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D PARTS=0xffff /D BrDrv1Begin=BrDrv1SoftPrimBegin /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D DEBUG=0 /D PARANOID=0 /D EVAL=0 /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /YX /FD /c
# ADD CPP /nologo /G5 /W3 /GX /Zi /O2 /D PARTS=0xffff /D BrDrv1Begin=BrDrv1SoftPrimBegin /D DEBUG=0 /D PARANOID=0 /D EVAL=0 /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USE_INTEL_COMPILER" /FR /YX /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6 (Proton)"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "pentprim"
# PROP BASE Intermediate_Dir "pentprim"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug P6 Proton"
# PROP Intermediate_Dir "Debug P6 Proton"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /W3 /GX /Z7 /Od /D BASED_FIXED=0 /D BASED_FLOAT=1 /D "PARANOID-1" /D STATIC=static /D BrDrv1Begin=BrDrv1SoftPrimBegin /D PARTS=0xffff /D DEBUG=1 /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D PARANOID=1 /D EVAL=0 /D STATIC= /YX /FD /c
# ADD CPP /nologo /G6 /W3 /GX /Zi /Od /D BASED_FIXED=0 /D BASED_FLOAT=1 /D STATIC=static /D BrDrv1Begin=BrDrv1SoftPrimBegin /D PARTS=0xffff /D DEBUG=1 /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D PARANOID=1 /D EVAL=0 /D STATIC= /D "_USE_INTEL_COMPILER" /YX /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6 (Proton)"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "pentpri0"
# PROP BASE Intermediate_Dir "pentpri0"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Profile P6 Proton"
# PROP Intermediate_Dir "Profile P6 Proton"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /W3 /GX /Zi /O2 /D PARTS=0xffff /D BrDrv1Begin=BrDrv1SoftPrimBegin /D DEBUG=0 /D PARANOID=0 /D EVAL=0 /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /YX /FD /c
# ADD CPP /nologo /G6 /W3 /GX /Zi /O2 /D PARTS=0xffff /D BrDrv1Begin=BrDrv1SoftPrimBegin /D DEBUG=0 /D PARANOID=0 /D EVAL=0 /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USE_INTEL_COMPILER" /FR /YX /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6 (Proton)"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "pentpri1"
# PROP BASE Intermediate_Dir "pentpri1"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release P6 Proton"
# PROP Intermediate_Dir "Release P6 Proton"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /W3 /GX /O2 /D PARTS=0xffff /D BrDrv1Begin=BrDrv1SoftPrimBegin /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D DEBUG=0 /D PARANOID=0 /D EVAL=0 /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /YX /FD /c
# ADD CPP /nologo /G6 /W3 /GX /O2 /D PARTS=0xffff /D BrDrv1Begin=BrDrv1SoftPrimBegin /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D DEBUG=0 /D PARANOID=0 /D EVAL=0 /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /D "_USE_INTEL_COMPILER" /YX /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "pentprim - Win32 Release"
# Name "pentprim - Win32 Debug"
# Name "pentprim - Win32 Profile"
# Name "pentprim - Win32 Debug P6"
# Name "pentprim - Win32 Profile P6"
# Name "pentprim - Win32 Release P6"
# Name "pentprim - Win32 Release (Proton)"
# Name "pentprim - Win32 Debug (Proton)"
# Name "pentprim - Win32 Profile (Proton)"
# Name "pentprim - Win32 Debug P6 (Proton)"
# Name "pentprim - Win32 Profile P6 (Proton)"
# Name "pentprim - Win32 Release P6 (Proton)"
# Begin Group "C Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Awtm.c
# End Source File
# Begin Source File

SOURCE=.\Awtmz.c
# End Source File
# Begin Source File

SOURCE=.\Device.c
# End Source File
# Begin Source File

SOURCE=.\Dither.c
# End Source File
# Begin Source File

SOURCE=.\Driver.c
# End Source File
# Begin Source File

SOURCE=.\Frcp.c
# End Source File
# Begin Source File

SOURCE=.\L_pi.c
# End Source File
# Begin Source File

SOURCE=.\L_piz.c
# End Source File
# Begin Source File

SOURCE=.\Match.c
# ADD CPP /D USE_MMX=1
# End Source File
# Begin Source File

SOURCE=.\Object.c
# End Source File
# Begin Source File

SOURCE=.\P_pi.c
# End Source File
# Begin Source File

SOURCE=.\P_piz.c
# End Source File
# Begin Source File

SOURCE=.\Persp.c
# End Source File
# Begin Source File

SOURCE=.\Perspz.c
# End Source File
# Begin Source File

SOURCE=.\Plib.c
# End Source File
# Begin Source File

SOURCE=.\Pstate.c
# End Source File
# Begin Source File

SOURCE=.\Rcp.c
# End Source File
# Begin Source File

SOURCE=.\Sbuffer.c
# End Source File
# Begin Source File

SOURCE=.\Work.c
# End Source File
# End Group
# Begin Group "Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Awtmi.h
# End Source File
# Begin Source File

SOURCE=.\Device.h
# End Source File
# Begin Source File

SOURCE=.\Drv.h

!IF  "$(CFG)" == "pentprim - Win32 Release"

# Begin Custom Build
InputPath=.\Drv.h

"drv.inc" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	h2inc -nologo -G3 -Zp4 -w -c -WIN32 -DBASED_FIXED=0 -DBASED_FLOAT=1 -D_WIN32\
              -D__VISUALC -D__H2INC__ -D_NO_PROTOTYPES -Fa_h2inc.tmp drv.h 
	perl ..\h2incfix.pl <_h2inc.tmp >drv.inc 
	del _h2inc.tmp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug"

# Begin Custom Build
InputPath=.\Drv.h

"drv.inc" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	h2inc -nologo -G3 -Zp4 -w -c -WIN32 -DBASED_FIXED=0 -DBASED_FLOAT=1 -D_WIN32\
              -D__VISUALC -D__H2INC__ -D_NO_PROTOTYPES -Fa_h2inc.tmp drv.h 
	perl ..\h2incfix.pl <_h2inc.tmp >drv.inc 
	del _h2inc.tmp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile"

# Begin Custom Build
InputPath=.\Drv.h

"drv.inc" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	h2inc -nologo -G3 -Zp4 -w -c -WIN32 -DBASED_FIXED=0 -DBASED_FLOAT=1 -D_WIN32\
              -D__VISUALC -D__H2INC__ -D_NO_PROTOTYPES -Fa_h2inc.tmp drv.h 
	perl ..\h2incfix.pl <_h2inc.tmp >drv.inc 
	del _h2inc.tmp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6"

# Begin Custom Build
InputPath=.\Drv.h

"drv.inc" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	h2inc -nologo -G3 -Zp4 -w -c -WIN32 -DBASED_FIXED=0 -DBASED_FLOAT=1 -D_WIN32\
              -D__VISUALC -D__H2INC__ -D_NO_PROTOTYPES -Fa_h2inc.tmp drv.h 
	perl ..\h2incfix.pl <_h2inc.tmp >drv.inc 
	del _h2inc.tmp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6"

# Begin Custom Build
InputPath=.\Drv.h

"drv.inc" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	h2inc -nologo -G3 -Zp4 -w -c -WIN32 -DBASED_FIXED=0 -DBASED_FLOAT=1 -D_WIN32\
              -D__VISUALC -D__H2INC__ -D_NO_PROTOTYPES -Fa_h2inc.tmp drv.h 
	perl ..\h2incfix.pl <_h2inc.tmp >drv.inc 
	del _h2inc.tmp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6"

# Begin Custom Build
InputPath=.\Drv.h

"drv.inc" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	h2inc -nologo -G3 -Zp4 -w -c -WIN32 -DBASED_FIXED=0 -DBASED_FLOAT=1 -D_WIN32\
              -D__VISUALC -D__H2INC__ -D_NO_PROTOTYPES -Fa_h2inc.tmp drv.h 
	perl ..\h2incfix.pl <_h2inc.tmp >drv.inc 
	del _h2inc.tmp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release (Proton)"

# Begin Custom Build
InputPath=.\Drv.h

"drv.inc" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	h2inc -nologo -G3 -Zp4 -w -c -WIN32 -DBASED_FIXED=0 -DBASED_FLOAT=1 -D_WIN32\
              -D__VISUALC -D__H2INC__ -D_NO_PROTOTYPES -Fa_h2inc.tmp drv.h 
	perl ..\h2incfix.pl <_h2inc.tmp >drv.inc 
	del _h2inc.tmp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug (Proton)"

# Begin Custom Build
InputPath=.\Drv.h

"drv.inc" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	h2inc -nologo -G3 -Zp4 -w -c -WIN32 -DBASED_FIXED=0 -DBASED_FLOAT=1 -D_WIN32\
              -D__VISUALC -D__H2INC__ -D_NO_PROTOTYPES -Fa_h2inc.tmp drv.h 
	perl ..\h2incfix.pl <_h2inc.tmp >drv.inc 
	del _h2inc.tmp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile (Proton)"

# Begin Custom Build
InputPath=.\Drv.h

"drv.inc" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	h2inc -nologo -G3 -Zp4 -w -c -WIN32 -DBASED_FIXED=0 -DBASED_FLOAT=1 -D_WIN32\
              -D__VISUALC -D__H2INC__ -D_NO_PROTOTYPES -Fa_h2inc.tmp drv.h 
	perl ..\h2incfix.pl <_h2inc.tmp >drv.inc 
	del _h2inc.tmp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6 (Proton)"

# Begin Custom Build
InputPath=.\Drv.h

"drv.inc" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	h2inc -nologo -G3 -Zp4 -w -c -WIN32 -DBASED_FIXED=0 -DBASED_FLOAT=1 -D_WIN32\
              -D__VISUALC -D__H2INC__ -D_NO_PROTOTYPES -Fa_h2inc.tmp drv.h 
	perl ..\h2incfix.pl <_h2inc.tmp >drv.inc 
	del _h2inc.tmp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6 (Proton)"

# Begin Custom Build
InputPath=.\Drv.h

"drv.inc" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	h2inc -nologo -G3 -Zp4 -w -c -WIN32 -DBASED_FIXED=0 -DBASED_FLOAT=1 -D_WIN32\
              -D__VISUALC -D__H2INC__ -D_NO_PROTOTYPES -Fa_h2inc.tmp drv.h 
	perl ..\h2incfix.pl <_h2inc.tmp >drv.inc 
	del _h2inc.tmp 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6 (Proton)"

# Begin Custom Build
InputPath=.\Drv.h

"drv.inc" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	h2inc -nologo -G3 -Zp4 -w -c -WIN32 -DBASED_FIXED=0 -DBASED_FLOAT=1 -D_WIN32\
              -D__VISUALC -D__H2INC__ -D_NO_PROTOTYPES -Fa_h2inc.tmp drv.h 
	perl ..\h2incfix.pl <_h2inc.tmp >drv.inc 
	del _h2inc.tmp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Drv_ip.h
# End Source File
# Begin Source File

SOURCE=.\Fperspi.h
# End Source File
# Begin Source File

SOURCE=.\Fperspzi.h
# End Source File
# Begin Source File

SOURCE=.\Match.h
# End Source File
# Begin Source File

SOURCE=.\mmx_ip.h
# End Source File
# Begin Source File

SOURCE=.\Object.h
# End Source File
# Begin Source File

SOURCE=.\Perspi.h
# End Source File
# Begin Source File

SOURCE=.\Perspzi.h
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

SOURCE=.\rastparm.h
# End Source File
# Begin Source File

SOURCE=.\Sbuffer.h
# End Source File
# Begin Source File

SOURCE=.\Template.h
# End Source File
# Begin Source File

SOURCE=.\Timestmp.h
# End Source File
# Begin Source File

SOURCE=.\Work.h
# End Source File
# End Group
# Begin Group "Perl Scripts"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\infogen.pl
# End Source File
# End Group
# Begin Group "Miscellaneous"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\mmx_t15.ifg
USERDEP__MMX_T="infogen.pl"	

!IF  "$(CFG)" == "pentprim - Win32 Release"

# Begin Custom Build
InputPath=.\mmx_t15.ifg
InputName=mmx_t15

"$(InputName)f.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName)f.c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug"

# Begin Custom Build
InputPath=.\mmx_t15.ifg
InputName=mmx_t15

"$(InputName)f.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName)f.c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile"

# Begin Custom Build
InputPath=.\mmx_t15.ifg
InputName=mmx_t15

"$(InputName)f.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName)f.c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6"

# Begin Custom Build
InputPath=.\mmx_t15.ifg
InputName=mmx_t15

"$(InputName)f.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName)f.c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6"

# Begin Custom Build
InputPath=.\mmx_t15.ifg
InputName=mmx_t15

"$(InputName)f.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName)f.c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6"

# Begin Custom Build
InputPath=.\mmx_t15.ifg
InputName=mmx_t15

"$(InputName)f.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName)f.c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release (Proton)"

# Begin Custom Build
InputPath=.\mmx_t15.ifg
InputName=mmx_t15

"$(InputName)f.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName)f.c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug (Proton)"

# Begin Custom Build
InputPath=.\mmx_t15.ifg
InputName=mmx_t15

"$(InputName)f.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName)f.c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile (Proton)"

# Begin Custom Build
InputPath=.\mmx_t15.ifg
InputName=mmx_t15

"$(InputName)f.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName)f.c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6 (Proton)"

# Begin Custom Build
InputPath=.\mmx_t15.ifg
InputName=mmx_t15

"$(InputName)f.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName)f.c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6 (Proton)"

# Begin Custom Build
InputPath=.\mmx_t15.ifg
InputName=mmx_t15

"$(InputName)f.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName)f.c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6 (Proton)"

# Begin Custom Build
InputPath=.\mmx_t15.ifg
InputName=mmx_t15

"$(InputName)f.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName)f.c image_suffix=f\
                   float_components

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\mmx_t16.ifg
USERDEP__MMX_T1="infogen.pl"	

!IF  "$(CFG)" == "pentprim - Win32 Release"

# Begin Custom Build
InputPath=.\mmx_t16.ifg
InputName=mmx_t16

"$(InputName)f.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName)f.c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug"

# Begin Custom Build
InputPath=.\mmx_t16.ifg
InputName=mmx_t16

"$(InputName)f.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName)f.c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile"

# Begin Custom Build
InputPath=.\mmx_t16.ifg
InputName=mmx_t16

"$(InputName)f.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName)f.c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6"

# Begin Custom Build
InputPath=.\mmx_t16.ifg
InputName=mmx_t16

"$(InputName)f.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName)f.c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6"

# Begin Custom Build
InputPath=.\mmx_t16.ifg
InputName=mmx_t16

"$(InputName)f.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName)f.c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6"

# Begin Custom Build
InputPath=.\mmx_t16.ifg
InputName=mmx_t16

"$(InputName)f.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName)f.c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release (Proton)"

# Begin Custom Build
InputPath=.\mmx_t16.ifg
InputName=mmx_t16

"$(InputName)f.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName)f.c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug (Proton)"

# Begin Custom Build
InputPath=.\mmx_t16.ifg
InputName=mmx_t16

"$(InputName)f.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName)f.c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile (Proton)"

# Begin Custom Build
InputPath=.\mmx_t16.ifg
InputName=mmx_t16

"$(InputName)f.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName)f.c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6 (Proton)"

# Begin Custom Build
InputPath=.\mmx_t16.ifg
InputName=mmx_t16

"$(InputName)f.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName)f.c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6 (Proton)"

# Begin Custom Build
InputPath=.\mmx_t16.ifg
InputName=mmx_t16

"$(InputName)f.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName)f.c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6 (Proton)"

# Begin Custom Build
InputPath=.\mmx_t16.ifg
InputName=mmx_t16

"$(InputName)f.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName)f.c image_suffix=f\
                   float_components

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Prim_l15.ifg
USERDEP__PRIM_="infogen.pl"	

!IF  "$(CFG)" == "pentprim - Win32 Release"

# Begin Custom Build
InputPath=.\Prim_l15.ifg
InputName=Prim_l15

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug"

# Begin Custom Build
InputPath=.\Prim_l15.ifg
InputName=Prim_l15

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile"

# Begin Custom Build
InputPath=.\Prim_l15.ifg
InputName=Prim_l15

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6"

# Begin Custom Build
InputPath=.\Prim_l15.ifg
InputName=Prim_l15

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6"

# Begin Custom Build
InputPath=.\Prim_l15.ifg
InputName=Prim_l15

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6"

# Begin Custom Build
InputPath=.\Prim_l15.ifg
InputName=Prim_l15

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release (Proton)"

# Begin Custom Build
InputPath=.\Prim_l15.ifg
InputName=Prim_l15

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug (Proton)"

# Begin Custom Build
InputPath=.\Prim_l15.ifg
InputName=Prim_l15

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile (Proton)"

# Begin Custom Build
InputPath=.\Prim_l15.ifg
InputName=Prim_l15

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6 (Proton)"

# Begin Custom Build
InputPath=.\Prim_l15.ifg
InputName=Prim_l15

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6 (Proton)"

# Begin Custom Build
InputPath=.\Prim_l15.ifg
InputName=Prim_l15

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6 (Proton)"

# Begin Custom Build
InputPath=.\Prim_l15.ifg
InputName=Prim_l15

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Prim_l16.ifg
USERDEP__PRIM_L="infogen.pl"	

!IF  "$(CFG)" == "pentprim - Win32 Release"

# Begin Custom Build
InputPath=.\Prim_l16.ifg
InputName=Prim_l16

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug"

# Begin Custom Build
InputPath=.\Prim_l16.ifg
InputName=Prim_l16

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile"

# Begin Custom Build
InputPath=.\Prim_l16.ifg
InputName=Prim_l16

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6"

# Begin Custom Build
InputPath=.\Prim_l16.ifg
InputName=Prim_l16

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6"

# Begin Custom Build
InputPath=.\Prim_l16.ifg
InputName=Prim_l16

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6"

# Begin Custom Build
InputPath=.\Prim_l16.ifg
InputName=Prim_l16

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release (Proton)"

# Begin Custom Build
InputPath=.\Prim_l16.ifg
InputName=Prim_l16

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug (Proton)"

# Begin Custom Build
InputPath=.\Prim_l16.ifg
InputName=Prim_l16

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile (Proton)"

# Begin Custom Build
InputPath=.\Prim_l16.ifg
InputName=Prim_l16

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6 (Proton)"

# Begin Custom Build
InputPath=.\Prim_l16.ifg
InputName=Prim_l16

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6 (Proton)"

# Begin Custom Build
InputPath=.\Prim_l16.ifg
InputName=Prim_l16

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6 (Proton)"

# Begin Custom Build
InputPath=.\Prim_l16.ifg
InputName=Prim_l16

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Prim_l24.ifg
USERDEP__PRIM_L2="infogen.pl"	

!IF  "$(CFG)" == "pentprim - Win32 Release"

# Begin Custom Build
InputPath=.\Prim_l24.ifg
InputName=Prim_l24

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug"

# Begin Custom Build
InputPath=.\Prim_l24.ifg
InputName=Prim_l24

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile"

# Begin Custom Build
InputPath=.\Prim_l24.ifg
InputName=Prim_l24

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6"

# Begin Custom Build
InputPath=.\Prim_l24.ifg
InputName=Prim_l24

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6"

# Begin Custom Build
InputPath=.\Prim_l24.ifg
InputName=Prim_l24

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6"

# Begin Custom Build
InputPath=.\Prim_l24.ifg
InputName=Prim_l24

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release (Proton)"

# Begin Custom Build
InputPath=.\Prim_l24.ifg
InputName=Prim_l24

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug (Proton)"

# Begin Custom Build
InputPath=.\Prim_l24.ifg
InputName=Prim_l24

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile (Proton)"

# Begin Custom Build
InputPath=.\Prim_l24.ifg
InputName=Prim_l24

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6 (Proton)"

# Begin Custom Build
InputPath=.\Prim_l24.ifg
InputName=Prim_l24

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6 (Proton)"

# Begin Custom Build
InputPath=.\Prim_l24.ifg
InputName=Prim_l24

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6 (Proton)"

# Begin Custom Build
InputPath=.\Prim_l24.ifg
InputName=Prim_l24

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Prim_l8.ifg
USERDEP__PRIM_L8="infogen.pl"	

!IF  "$(CFG)" == "pentprim - Win32 Release"

# Begin Custom Build
InputPath=.\Prim_l8.ifg
InputName=Prim_l8

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug"

# Begin Custom Build
InputPath=.\Prim_l8.ifg
InputName=Prim_l8

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile"

# Begin Custom Build
InputPath=.\Prim_l8.ifg
InputName=Prim_l8

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6"

# Begin Custom Build
InputPath=.\Prim_l8.ifg
InputName=Prim_l8

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6"

# Begin Custom Build
InputPath=.\Prim_l8.ifg
InputName=Prim_l8

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6"

# Begin Custom Build
InputPath=.\Prim_l8.ifg
InputName=Prim_l8

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release (Proton)"

# Begin Custom Build
InputPath=.\Prim_l8.ifg
InputName=Prim_l8

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug (Proton)"

# Begin Custom Build
InputPath=.\Prim_l8.ifg
InputName=Prim_l8

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile (Proton)"

# Begin Custom Build
InputPath=.\Prim_l8.ifg
InputName=Prim_l8

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6 (Proton)"

# Begin Custom Build
InputPath=.\Prim_l8.ifg
InputName=Prim_l8

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6 (Proton)"

# Begin Custom Build
InputPath=.\Prim_l8.ifg
InputName=Prim_l8

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6 (Proton)"

# Begin Custom Build
InputPath=.\Prim_l8.ifg
InputName=Prim_l8

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Prim_p15.ifg
USERDEP__PRIM_P="infogen.pl"	

!IF  "$(CFG)" == "pentprim - Win32 Release"

# Begin Custom Build
InputPath=.\Prim_p15.ifg
InputName=Prim_p15

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug"

# Begin Custom Build
InputPath=.\Prim_p15.ifg
InputName=Prim_p15

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile"

# Begin Custom Build
InputPath=.\Prim_p15.ifg
InputName=Prim_p15

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6"

# Begin Custom Build
InputPath=.\Prim_p15.ifg
InputName=Prim_p15

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6"

# Begin Custom Build
InputPath=.\Prim_p15.ifg
InputName=Prim_p15

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6"

# Begin Custom Build
InputPath=.\Prim_p15.ifg
InputName=Prim_p15

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release (Proton)"

# Begin Custom Build
InputPath=.\Prim_p15.ifg
InputName=Prim_p15

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug (Proton)"

# Begin Custom Build
InputPath=.\Prim_p15.ifg
InputName=Prim_p15

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile (Proton)"

# Begin Custom Build
InputPath=.\Prim_p15.ifg
InputName=Prim_p15

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6 (Proton)"

# Begin Custom Build
InputPath=.\Prim_p15.ifg
InputName=Prim_p15

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6 (Proton)"

# Begin Custom Build
InputPath=.\Prim_p15.ifg
InputName=Prim_p15

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6 (Proton)"

# Begin Custom Build
InputPath=.\Prim_p15.ifg
InputName=Prim_p15

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Prim_p16.ifg
USERDEP__PRIM_P1="infogen.pl"	

!IF  "$(CFG)" == "pentprim - Win32 Release"

# Begin Custom Build
InputPath=.\Prim_p16.ifg
InputName=Prim_p16

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug"

# Begin Custom Build
InputPath=.\Prim_p16.ifg
InputName=Prim_p16

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile"

# Begin Custom Build
InputPath=.\Prim_p16.ifg
InputName=Prim_p16

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6"

# Begin Custom Build
InputPath=.\Prim_p16.ifg
InputName=Prim_p16

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6"

# Begin Custom Build
InputPath=.\Prim_p16.ifg
InputName=Prim_p16

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6"

# Begin Custom Build
InputPath=.\Prim_p16.ifg
InputName=Prim_p16

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release (Proton)"

# Begin Custom Build
InputPath=.\Prim_p16.ifg
InputName=Prim_p16

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug (Proton)"

# Begin Custom Build
InputPath=.\Prim_p16.ifg
InputName=Prim_p16

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile (Proton)"

# Begin Custom Build
InputPath=.\Prim_p16.ifg
InputName=Prim_p16

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6 (Proton)"

# Begin Custom Build
InputPath=.\Prim_p16.ifg
InputName=Prim_p16

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6 (Proton)"

# Begin Custom Build
InputPath=.\Prim_p16.ifg
InputName=Prim_p16

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6 (Proton)"

# Begin Custom Build
InputPath=.\Prim_p16.ifg
InputName=Prim_p16

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Prim_p24.ifg
USERDEP__PRIM_P2="infogen.pl"	

!IF  "$(CFG)" == "pentprim - Win32 Release"

# Begin Custom Build
InputPath=.\Prim_p24.ifg
InputName=Prim_p24

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug"

# Begin Custom Build
InputPath=.\Prim_p24.ifg
InputName=Prim_p24

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile"

# Begin Custom Build
InputPath=.\Prim_p24.ifg
InputName=Prim_p24

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6"

# Begin Custom Build
InputPath=.\Prim_p24.ifg
InputName=Prim_p24

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6"

# Begin Custom Build
InputPath=.\Prim_p24.ifg
InputName=Prim_p24

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6"

# Begin Custom Build
InputPath=.\Prim_p24.ifg
InputName=Prim_p24

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release (Proton)"

# Begin Custom Build
InputPath=.\Prim_p24.ifg
InputName=Prim_p24

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug (Proton)"

# Begin Custom Build
InputPath=.\Prim_p24.ifg
InputName=Prim_p24

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile (Proton)"

# Begin Custom Build
InputPath=.\Prim_p24.ifg
InputName=Prim_p24

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6 (Proton)"

# Begin Custom Build
InputPath=.\Prim_p24.ifg
InputName=Prim_p24

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6 (Proton)"

# Begin Custom Build
InputPath=.\Prim_p24.ifg
InputName=Prim_p24

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6 (Proton)"

# Begin Custom Build
InputPath=.\Prim_p24.ifg
InputName=Prim_p24

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Prim_p8.ifg
USERDEP__PRIM_P8="infogen.pl"	

!IF  "$(CFG)" == "pentprim - Win32 Release"

# Begin Custom Build
InputPath=.\Prim_p8.ifg
InputName=Prim_p8

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug"

# Begin Custom Build
InputPath=.\Prim_p8.ifg
InputName=Prim_p8

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile"

# Begin Custom Build
InputPath=.\Prim_p8.ifg
InputName=Prim_p8

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6"

# Begin Custom Build
InputPath=.\Prim_p8.ifg
InputName=Prim_p8

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6"

# Begin Custom Build
InputPath=.\Prim_p8.ifg
InputName=Prim_p8

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6"

# Begin Custom Build
InputPath=.\Prim_p8.ifg
InputName=Prim_p8

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release (Proton)"

# Begin Custom Build
InputPath=.\Prim_p8.ifg
InputName=Prim_p8

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug (Proton)"

# Begin Custom Build
InputPath=.\Prim_p8.ifg
InputName=Prim_p8

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile (Proton)"

# Begin Custom Build
InputPath=.\Prim_p8.ifg
InputName=Prim_p8

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6 (Proton)"

# Begin Custom Build
InputPath=.\Prim_p8.ifg
InputName=Prim_p8

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6 (Proton)"

# Begin Custom Build
InputPath=.\Prim_p8.ifg
InputName=Prim_p8

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6 (Proton)"

# Begin Custom Build
InputPath=.\Prim_p8.ifg
InputName=Prim_p8

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Prim_t24.ifg
USERDEP__PRIM_T="infogen.pl"	

!IF  "$(CFG)" == "pentprim - Win32 Release"

# Begin Custom Build
InputPath=.\Prim_t24.ifg
InputName=Prim_t24

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug"

# Begin Custom Build
InputPath=.\Prim_t24.ifg
InputName=Prim_t24

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile"

# Begin Custom Build
InputPath=.\Prim_t24.ifg
InputName=Prim_t24

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6"

# Begin Custom Build
InputPath=.\Prim_t24.ifg
InputName=Prim_t24

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6"

# Begin Custom Build
InputPath=.\Prim_t24.ifg
InputName=Prim_t24

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6"

# Begin Custom Build
InputPath=.\Prim_t24.ifg
InputName=Prim_t24

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release (Proton)"

# Begin Custom Build
InputPath=.\Prim_t24.ifg
InputName=Prim_t24

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug (Proton)"

# Begin Custom Build
InputPath=.\Prim_t24.ifg
InputName=Prim_t24

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile (Proton)"

# Begin Custom Build
InputPath=.\Prim_t24.ifg
InputName=Prim_t24

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6 (Proton)"

# Begin Custom Build
InputPath=.\Prim_t24.ifg
InputName=Prim_t24

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6 (Proton)"

# Begin Custom Build
InputPath=.\Prim_t24.ifg
InputName=Prim_t24

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6 (Proton)"

# Begin Custom Build
InputPath=.\Prim_t24.ifg
InputName=Prim_t24

"$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName).c image_suffix=f\
                   float_components

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\prim_t8.ifg
USERDEP__PRIM_T8="infogen.pl"	

!IF  "$(CFG)" == "pentprim - Win32 Release"

# Begin Custom Build
InputPath=.\prim_t8.ifg
InputName=prim_t8

"$(InputName)f.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName)f.c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug"

# Begin Custom Build
InputPath=.\prim_t8.ifg
InputName=prim_t8

"$(InputName)f.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName)f.c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile"

# Begin Custom Build
InputPath=.\prim_t8.ifg
InputName=prim_t8

"$(InputName)f.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName)f.c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6"

# Begin Custom Build
InputPath=.\prim_t8.ifg
InputName=prim_t8

"$(InputName)f.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName)f.c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6"

# Begin Custom Build
InputPath=.\prim_t8.ifg
InputName=prim_t8

"$(InputName)f.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName)f.c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6"

# Begin Custom Build
InputPath=.\prim_t8.ifg
InputName=prim_t8

"$(InputName)f.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName)f.c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release (Proton)"

# Begin Custom Build
InputPath=.\prim_t8.ifg
InputName=prim_t8

"$(InputName)f.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName)f.c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug (Proton)"

# Begin Custom Build
InputPath=.\prim_t8.ifg
InputName=prim_t8

"$(InputName)f.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName)f.c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile (Proton)"

# Begin Custom Build
InputPath=.\prim_t8.ifg
InputName=prim_t8

"$(InputName)f.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName)f.c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6 (Proton)"

# Begin Custom Build
InputPath=.\prim_t8.ifg
InputName=prim_t8

"$(InputName)f.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName)f.c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6 (Proton)"

# Begin Custom Build
InputPath=.\prim_t8.ifg
InputName=prim_t8

"$(InputName)f.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName)f.c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6 (Proton)"

# Begin Custom Build
InputPath=.\prim_t8.ifg
InputName=prim_t8

"$(InputName)f.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName)f.c image_suffix=f\
                   float_components

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\prm_t15.ifg
USERDEP__PRM_T="infogen.pl"	

!IF  "$(CFG)" == "pentprim - Win32 Release"

# Begin Custom Build
InputPath=.\prm_t15.ifg
InputName=prm_t15

"$(InputName)f.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName)f.c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug"

# Begin Custom Build
InputPath=.\prm_t15.ifg
InputName=prm_t15

"$(InputName)f.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName)f.c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile"

# Begin Custom Build
InputPath=.\prm_t15.ifg
InputName=prm_t15

"$(InputName)f.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName)f.c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6"

# Begin Custom Build
InputPath=.\prm_t15.ifg
InputName=prm_t15

"$(InputName)f.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName)f.c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6"

# Begin Custom Build
InputPath=.\prm_t15.ifg
InputName=prm_t15

"$(InputName)f.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName)f.c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6"

# Begin Custom Build
InputPath=.\prm_t15.ifg
InputName=prm_t15

"$(InputName)f.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName)f.c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release (Proton)"

# Begin Custom Build
InputPath=.\prm_t15.ifg
InputName=prm_t15

"$(InputName)f.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName)f.c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug (Proton)"

# Begin Custom Build
InputPath=.\prm_t15.ifg
InputName=prm_t15

"$(InputName)f.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName)f.c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile (Proton)"

# Begin Custom Build
InputPath=.\prm_t15.ifg
InputName=prm_t15

"$(InputName)f.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName)f.c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6 (Proton)"

# Begin Custom Build
InputPath=.\prm_t15.ifg
InputName=prm_t15

"$(InputName)f.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName)f.c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6 (Proton)"

# Begin Custom Build
InputPath=.\prm_t15.ifg
InputName=prm_t15

"$(InputName)f.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName)f.c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6 (Proton)"

# Begin Custom Build
InputPath=.\prm_t15.ifg
InputName=prm_t15

"$(InputName)f.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName)f.c image_suffix=f\
                   float_components

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\prm_t16.ifg
USERDEP__PRM_T1="infogen.pl"	

!IF  "$(CFG)" == "pentprim - Win32 Release"

# Begin Custom Build
InputPath=.\prm_t16.ifg
InputName=prm_t16

"$(InputName)f.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName)f.c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug"

# Begin Custom Build
InputPath=.\prm_t16.ifg
InputName=prm_t16

"$(InputName)f.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName)f.c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile"

# Begin Custom Build
InputPath=.\prm_t16.ifg
InputName=prm_t16

"$(InputName)f.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName)f.c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6"

# Begin Custom Build
InputPath=.\prm_t16.ifg
InputName=prm_t16

"$(InputName)f.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName)f.c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6"

# Begin Custom Build
InputPath=.\prm_t16.ifg
InputName=prm_t16

"$(InputName)f.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName)f.c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6"

# Begin Custom Build
InputPath=.\prm_t16.ifg
InputName=prm_t16

"$(InputName)f.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName)f.c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release (Proton)"

# Begin Custom Build
InputPath=.\prm_t16.ifg
InputName=prm_t16

"$(InputName)f.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName)f.c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug (Proton)"

# Begin Custom Build
InputPath=.\prm_t16.ifg
InputName=prm_t16

"$(InputName)f.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName)f.c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile (Proton)"

# Begin Custom Build
InputPath=.\prm_t16.ifg
InputName=prm_t16

"$(InputName)f.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName)f.c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6 (Proton)"

# Begin Custom Build
InputPath=.\prm_t16.ifg
InputName=prm_t16

"$(InputName)f.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName)f.c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6 (Proton)"

# Begin Custom Build
InputPath=.\prm_t16.ifg
InputName=prm_t16

"$(InputName)f.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName)f.c image_suffix=f\
                   float_components

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6 (Proton)"

# Begin Custom Build
InputPath=.\prm_t16.ifg
InputName=prm_t16

"$(InputName)f.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	perl infogen.pl <$(InputName).ifg >$(InputName)f.c image_suffix=f\
                   float_components

# End Custom Build

!ENDIF 

# End Source File
# End Group
# Begin Group "Assembly"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\decal.asm
USERDEP__DECAL="drv.inc"	

!IF  "$(CFG)" == "pentprim - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=.\decal.asm
InputName=decal

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=.\decal.asm
InputName=decal

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile"

# Begin Custom Build
OutDir=.\Profile
InputPath=.\decal.asm
InputName=decal

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6"

# Begin Custom Build
OutDir=.\Debug P6
InputPath=.\decal.asm
InputName=decal

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6"

# Begin Custom Build
OutDir=.\Profile P6
InputPath=.\decal.asm
InputName=decal

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6"

# Begin Custom Build
OutDir=.\Release P6
InputPath=.\decal.asm
InputName=decal

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release (Proton)"

# Begin Custom Build
OutDir=.\Release Proton
InputPath=.\decal.asm
InputName=decal

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug (Proton)"

# Begin Custom Build
OutDir=.\Debug Proton
InputPath=.\decal.asm
InputName=decal

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile (Proton)"

# Begin Custom Build
OutDir=.\Profile Proton
InputPath=.\decal.asm
InputName=decal

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6 (Proton)"

# Begin Custom Build
OutDir=.\Debug P6 Proton
InputPath=.\decal.asm
InputName=decal

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6 (Proton)"

# Begin Custom Build
OutDir=.\Profile P6 Proton
InputPath=.\decal.asm
InputName=decal

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6 (Proton)"

# Begin Custom Build
OutDir=.\Release P6 Proton
InputPath=.\decal.asm
InputName=decal

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Fpsetup.asm
USERDEP__FPSET="drv.inc"	

!IF  "$(CFG)" == "pentprim - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=.\Fpsetup.asm
InputName=Fpsetup

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=.\Fpsetup.asm
InputName=Fpsetup

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile"

# Begin Custom Build
OutDir=.\Profile
InputPath=.\Fpsetup.asm
InputName=Fpsetup

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6"

# Begin Custom Build
OutDir=.\Debug P6
InputPath=.\Fpsetup.asm
InputName=Fpsetup

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6"

# Begin Custom Build
OutDir=.\Profile P6
InputPath=.\Fpsetup.asm
InputName=Fpsetup

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6"

# Begin Custom Build
OutDir=.\Release P6
InputPath=.\Fpsetup.asm
InputName=Fpsetup

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release (Proton)"

# Begin Custom Build
OutDir=.\Release Proton
InputPath=.\Fpsetup.asm
InputName=Fpsetup

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug (Proton)"

# Begin Custom Build
OutDir=.\Debug Proton
InputPath=.\Fpsetup.asm
InputName=Fpsetup

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile (Proton)"

# Begin Custom Build
OutDir=.\Profile Proton
InputPath=.\Fpsetup.asm
InputName=Fpsetup

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6 (Proton)"

# Begin Custom Build
OutDir=.\Debug P6 Proton
InputPath=.\Fpsetup.asm
InputName=Fpsetup

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6 (Proton)"

# Begin Custom Build
OutDir=.\Profile P6 Proton
InputPath=.\Fpsetup.asm
InputName=Fpsetup

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6 (Proton)"

# Begin Custom Build
OutDir=.\Release P6 Proton
InputPath=.\Fpsetup.asm
InputName=Fpsetup

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Fpustub.asm
USERDEP__FPUST="drv.inc"	

!IF  "$(CFG)" == "pentprim - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=.\Fpustub.asm
InputName=Fpustub

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=.\Fpustub.asm
InputName=Fpustub

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile"

# Begin Custom Build
OutDir=.\Profile
InputPath=.\Fpustub.asm
InputName=Fpustub

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6"

# Begin Custom Build
OutDir=.\Debug P6
InputPath=.\Fpustub.asm
InputName=Fpustub

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6"

# Begin Custom Build
OutDir=.\Profile P6
InputPath=.\Fpustub.asm
InputName=Fpustub

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6"

# Begin Custom Build
OutDir=.\Release P6
InputPath=.\Fpustub.asm
InputName=Fpustub

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release (Proton)"

# Begin Custom Build
OutDir=.\Release Proton
InputPath=.\Fpustub.asm
InputName=Fpustub

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug (Proton)"

# Begin Custom Build
OutDir=.\Debug Proton
InputPath=.\Fpustub.asm
InputName=Fpustub

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile (Proton)"

# Begin Custom Build
OutDir=.\Profile Proton
InputPath=.\Fpustub.asm
InputName=Fpustub

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6 (Proton)"

# Begin Custom Build
OutDir=.\Debug P6 Proton
InputPath=.\Fpustub.asm
InputName=Fpustub

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6 (Proton)"

# Begin Custom Build
OutDir=.\Profile P6 Proton
InputPath=.\Fpustub.asm
InputName=Fpustub

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6 (Proton)"

# Begin Custom Build
OutDir=.\Release P6 Proton
InputPath=.\Fpustub.asm
InputName=Fpustub

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\fti8_pip.asm
USERDEP__FTI8_="drv.inc"	

!IF  "$(CFG)" == "pentprim - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=.\fti8_pip.asm
InputName=fti8_pip

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=.\fti8_pip.asm
InputName=fti8_pip

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile"

# Begin Custom Build
OutDir=.\Profile
InputPath=.\fti8_pip.asm
InputName=fti8_pip

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6"

# Begin Custom Build
OutDir=.\Debug P6
InputPath=.\fti8_pip.asm
InputName=fti8_pip

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6"

# Begin Custom Build
OutDir=.\Profile P6
InputPath=.\fti8_pip.asm
InputName=fti8_pip

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6"

# Begin Custom Build
OutDir=.\Release P6
InputPath=.\fti8_pip.asm
InputName=fti8_pip

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release (Proton)"

# Begin Custom Build
OutDir=.\Release Proton
InputPath=.\fti8_pip.asm
InputName=fti8_pip

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug (Proton)"

# Begin Custom Build
OutDir=.\Debug Proton
InputPath=.\fti8_pip.asm
InputName=fti8_pip

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile (Proton)"

# Begin Custom Build
OutDir=.\Profile Proton
InputPath=.\fti8_pip.asm
InputName=fti8_pip

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6 (Proton)"

# Begin Custom Build
OutDir=.\Debug P6 Proton
InputPath=.\fti8_pip.asm
InputName=fti8_pip

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6 (Proton)"

# Begin Custom Build
OutDir=.\Profile P6 Proton
InputPath=.\fti8_pip.asm
InputName=fti8_pip

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6 (Proton)"

# Begin Custom Build
OutDir=.\Release P6 Proton
InputPath=.\fti8_pip.asm
InputName=fti8_pip

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\fti8_piz.asm
USERDEP__FTI8_P="drv.inc"	

!IF  "$(CFG)" == "pentprim - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=.\fti8_piz.asm
InputName=fti8_piz

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=.\fti8_piz.asm
InputName=fti8_piz

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile"

# Begin Custom Build
OutDir=.\Profile
InputPath=.\fti8_piz.asm
InputName=fti8_piz

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6"

# Begin Custom Build
OutDir=.\Debug P6
InputPath=.\fti8_piz.asm
InputName=fti8_piz

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6"

# Begin Custom Build
OutDir=.\Profile P6
InputPath=.\fti8_piz.asm
InputName=fti8_piz

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6"

# Begin Custom Build
OutDir=.\Release P6
InputPath=.\fti8_piz.asm
InputName=fti8_piz

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release (Proton)"

# Begin Custom Build
OutDir=.\Release Proton
InputPath=.\fti8_piz.asm
InputName=fti8_piz

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug (Proton)"

# Begin Custom Build
OutDir=.\Debug Proton
InputPath=.\fti8_piz.asm
InputName=fti8_piz

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile (Proton)"

# Begin Custom Build
OutDir=.\Profile Proton
InputPath=.\fti8_piz.asm
InputName=fti8_piz

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6 (Proton)"

# Begin Custom Build
OutDir=.\Debug P6 Proton
InputPath=.\fti8_piz.asm
InputName=fti8_piz

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6 (Proton)"

# Begin Custom Build
OutDir=.\Profile P6 Proton
InputPath=.\fti8_piz.asm
InputName=fti8_piz

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6 (Proton)"

# Begin Custom Build
OutDir=.\Release P6 Proton
InputPath=.\fti8_piz.asm
InputName=fti8_piz

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\fti8pizp.asm
USERDEP__FTI8P="drv.inc"	

!IF  "$(CFG)" == "pentprim - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=.\fti8pizp.asm
InputName=fti8pizp

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=.\fti8pizp.asm
InputName=fti8pizp

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile"

# Begin Custom Build
OutDir=.\Profile
InputPath=.\fti8pizp.asm
InputName=fti8pizp

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6"

# Begin Custom Build
OutDir=.\Debug P6
InputPath=.\fti8pizp.asm
InputName=fti8pizp

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6"

# Begin Custom Build
OutDir=.\Profile P6
InputPath=.\fti8pizp.asm
InputName=fti8pizp

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6"

# Begin Custom Build
OutDir=.\Release P6
InputPath=.\fti8pizp.asm
InputName=fti8pizp

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release (Proton)"

# Begin Custom Build
OutDir=.\Release Proton
InputPath=.\fti8pizp.asm
InputName=fti8pizp

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug (Proton)"

# Begin Custom Build
OutDir=.\Debug Proton
InputPath=.\fti8pizp.asm
InputName=fti8pizp

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile (Proton)"

# Begin Custom Build
OutDir=.\Profile Proton
InputPath=.\fti8pizp.asm
InputName=fti8pizp

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6 (Proton)"

# Begin Custom Build
OutDir=.\Debug P6 Proton
InputPath=.\fti8pizp.asm
InputName=fti8pizp

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6 (Proton)"

# Begin Custom Build
OutDir=.\Profile P6 Proton
InputPath=.\fti8pizp.asm
InputName=fti8pizp

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6 (Proton)"

# Begin Custom Build
OutDir=.\Release P6 Proton
InputPath=.\fti8pizp.asm
InputName=fti8pizp

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gsetuptf.asm
USERDEP__GSETU="drv.inc"	

!IF  "$(CFG)" == "pentprim - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=.\gsetuptf.asm
InputName=gsetuptf

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=.\gsetuptf.asm
InputName=gsetuptf

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile"

# Begin Custom Build
OutDir=.\Profile
InputPath=.\gsetuptf.asm
InputName=gsetuptf

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6"

# Begin Custom Build
OutDir=.\Debug P6
InputPath=.\gsetuptf.asm
InputName=gsetuptf

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6"

# Begin Custom Build
OutDir=.\Profile P6
InputPath=.\gsetuptf.asm
InputName=gsetuptf

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6"

# Begin Custom Build
OutDir=.\Release P6
InputPath=.\gsetuptf.asm
InputName=gsetuptf

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release (Proton)"

# Begin Custom Build
OutDir=.\Release Proton
InputPath=.\gsetuptf.asm
InputName=gsetuptf

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug (Proton)"

# Begin Custom Build
OutDir=.\Debug Proton
InputPath=.\gsetuptf.asm
InputName=gsetuptf

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile (Proton)"

# Begin Custom Build
OutDir=.\Profile Proton
InputPath=.\gsetuptf.asm
InputName=gsetuptf

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6 (Proton)"

# Begin Custom Build
OutDir=.\Debug P6 Proton
InputPath=.\gsetuptf.asm
InputName=gsetuptf

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6 (Proton)"

# Begin Custom Build
OutDir=.\Profile P6 Proton
InputPath=.\gsetuptf.asm
InputName=gsetuptf

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6 (Proton)"

# Begin Custom Build
OutDir=.\Release P6 Proton
InputPath=.\gsetuptf.asm
InputName=gsetuptf

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gsetuptx.asm
USERDEP__GSETUP="drv.inc"	

!IF  "$(CFG)" == "pentprim - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=.\gsetuptx.asm
InputName=gsetuptx

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=.\gsetuptx.asm
InputName=gsetuptx

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile"

# Begin Custom Build
OutDir=.\Profile
InputPath=.\gsetuptx.asm
InputName=gsetuptx

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6"

# Begin Custom Build
OutDir=.\Debug P6
InputPath=.\gsetuptx.asm
InputName=gsetuptx

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6"

# Begin Custom Build
OutDir=.\Profile P6
InputPath=.\gsetuptx.asm
InputName=gsetuptx

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6"

# Begin Custom Build
OutDir=.\Release P6
InputPath=.\gsetuptx.asm
InputName=gsetuptx

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release (Proton)"

# Begin Custom Build
OutDir=.\Release Proton
InputPath=.\gsetuptx.asm
InputName=gsetuptx

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug (Proton)"

# Begin Custom Build
OutDir=.\Debug Proton
InputPath=.\gsetuptx.asm
InputName=gsetuptx

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile (Proton)"

# Begin Custom Build
OutDir=.\Profile Proton
InputPath=.\gsetuptx.asm
InputName=gsetuptx

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6 (Proton)"

# Begin Custom Build
OutDir=.\Debug P6 Proton
InputPath=.\gsetuptx.asm
InputName=gsetuptx

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6 (Proton)"

# Begin Custom Build
OutDir=.\Profile P6 Proton
InputPath=.\gsetuptx.asm
InputName=gsetuptx

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6 (Proton)"

# Begin Custom Build
OutDir=.\Release P6 Proton
InputPath=.\gsetuptx.asm
InputName=gsetuptx

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Magicsym.asm
USERDEP__MAGIC="drv.inc"	

!IF  "$(CFG)" == "pentprim - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=.\Magicsym.asm
InputName=Magicsym

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=.\Magicsym.asm
InputName=Magicsym

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile"

# Begin Custom Build
OutDir=.\Profile
InputPath=.\Magicsym.asm
InputName=Magicsym

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6"

# Begin Custom Build
OutDir=.\Debug P6
InputPath=.\Magicsym.asm
InputName=Magicsym

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6"

# Begin Custom Build
OutDir=.\Profile P6
InputPath=.\Magicsym.asm
InputName=Magicsym

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6"

# Begin Custom Build
OutDir=.\Release P6
InputPath=.\Magicsym.asm
InputName=Magicsym

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release (Proton)"

# Begin Custom Build
OutDir=.\Release Proton
InputPath=.\Magicsym.asm
InputName=Magicsym

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug (Proton)"

# Begin Custom Build
OutDir=.\Debug Proton
InputPath=.\Magicsym.asm
InputName=Magicsym

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile (Proton)"

# Begin Custom Build
OutDir=.\Profile Proton
InputPath=.\Magicsym.asm
InputName=Magicsym

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6 (Proton)"

# Begin Custom Build
OutDir=.\Debug P6 Proton
InputPath=.\Magicsym.asm
InputName=Magicsym

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6 (Proton)"

# Begin Custom Build
OutDir=.\Profile P6 Proton
InputPath=.\Magicsym.asm
InputName=Magicsym

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6 (Proton)"

# Begin Custom Build
OutDir=.\Release P6 Proton
InputPath=.\Magicsym.asm
InputName=Magicsym

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\pfpsetup.asm
USERDEP__PFPSE="drv.inc"	

!IF  "$(CFG)" == "pentprim - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=.\pfpsetup.asm
InputName=pfpsetup

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=.\pfpsetup.asm
InputName=pfpsetup

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile"

# Begin Custom Build
OutDir=.\Profile
InputPath=.\pfpsetup.asm
InputName=pfpsetup

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6"

# Begin Custom Build
OutDir=.\Debug P6
InputPath=.\pfpsetup.asm
InputName=pfpsetup

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6"

# Begin Custom Build
OutDir=.\Profile P6
InputPath=.\pfpsetup.asm
InputName=pfpsetup

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6"

# Begin Custom Build
OutDir=.\Release P6
InputPath=.\pfpsetup.asm
InputName=pfpsetup

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release (Proton)"

# Begin Custom Build
OutDir=.\Release Proton
InputPath=.\pfpsetup.asm
InputName=pfpsetup

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug (Proton)"

# Begin Custom Build
OutDir=.\Debug Proton
InputPath=.\pfpsetup.asm
InputName=pfpsetup

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile (Proton)"

# Begin Custom Build
OutDir=.\Profile Proton
InputPath=.\pfpsetup.asm
InputName=pfpsetup

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6 (Proton)"

# Begin Custom Build
OutDir=.\Debug P6 Proton
InputPath=.\pfpsetup.asm
InputName=pfpsetup

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6 (Proton)"

# Begin Custom Build
OutDir=.\Profile P6 Proton
InputPath=.\pfpsetup.asm
InputName=pfpsetup

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6 (Proton)"

# Begin Custom Build
OutDir=.\Release P6 Proton
InputPath=.\pfpsetup.asm
InputName=pfpsetup

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\rastbuff.asm
USERDEP__RASTB="drv.inc"	

!IF  "$(CFG)" == "pentprim - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=.\rastbuff.asm
InputName=rastbuff

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=.\rastbuff.asm
InputName=rastbuff

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile"

# Begin Custom Build
OutDir=.\Profile
InputPath=.\rastbuff.asm
InputName=rastbuff

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6"

# Begin Custom Build
OutDir=.\Debug P6
InputPath=.\rastbuff.asm
InputName=rastbuff

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6"

# Begin Custom Build
OutDir=.\Profile P6
InputPath=.\rastbuff.asm
InputName=rastbuff

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6"

# Begin Custom Build
OutDir=.\Release P6
InputPath=.\rastbuff.asm
InputName=rastbuff

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release (Proton)"

# Begin Custom Build
OutDir=.\Release Proton
InputPath=.\rastbuff.asm
InputName=rastbuff

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug (Proton)"

# Begin Custom Build
OutDir=.\Debug Proton
InputPath=.\rastbuff.asm
InputName=rastbuff

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile (Proton)"

# Begin Custom Build
OutDir=.\Profile Proton
InputPath=.\rastbuff.asm
InputName=rastbuff

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6 (Proton)"

# Begin Custom Build
OutDir=.\Debug P6 Proton
InputPath=.\rastbuff.asm
InputName=rastbuff

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6 (Proton)"

# Begin Custom Build
OutDir=.\Profile P6 Proton
InputPath=.\rastbuff.asm
InputName=rastbuff

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6 (Proton)"

# Begin Custom Build
OutDir=.\Release P6 Proton
InputPath=.\rastbuff.asm
InputName=rastbuff

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\rastrise.asm
USERDEP__RASTR="drv.inc"	

!IF  "$(CFG)" == "pentprim - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=.\rastrise.asm
InputName=rastrise

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=.\rastrise.asm
InputName=rastrise

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile"

# Begin Custom Build
OutDir=.\Profile
InputPath=.\rastrise.asm
InputName=rastrise

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6"

# Begin Custom Build
OutDir=.\Debug P6
InputPath=.\rastrise.asm
InputName=rastrise

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6"

# Begin Custom Build
OutDir=.\Profile P6
InputPath=.\rastrise.asm
InputName=rastrise

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6"

# Begin Custom Build
OutDir=.\Release P6
InputPath=.\rastrise.asm
InputName=rastrise

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release (Proton)"

# Begin Custom Build
OutDir=.\Release Proton
InputPath=.\rastrise.asm
InputName=rastrise

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug (Proton)"

# Begin Custom Build
OutDir=.\Debug Proton
InputPath=.\rastrise.asm
InputName=rastrise

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile (Proton)"

# Begin Custom Build
OutDir=.\Profile Proton
InputPath=.\rastrise.asm
InputName=rastrise

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6 (Proton)"

# Begin Custom Build
OutDir=.\Debug P6 Proton
InputPath=.\rastrise.asm
InputName=rastrise

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6 (Proton)"

# Begin Custom Build
OutDir=.\Profile P6 Proton
InputPath=.\rastrise.asm
InputName=rastrise

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6 (Proton)"

# Begin Custom Build
OutDir=.\Release P6 Proton
InputPath=.\rastrise.asm
InputName=rastrise

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Safediv.asm
USERDEP__SAFED="drv.inc"	

!IF  "$(CFG)" == "pentprim - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=.\Safediv.asm
InputName=Safediv

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=.\Safediv.asm
InputName=Safediv

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile"

# Begin Custom Build
OutDir=.\Profile
InputPath=.\Safediv.asm
InputName=Safediv

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6"

# Begin Custom Build
OutDir=.\Debug P6
InputPath=.\Safediv.asm
InputName=Safediv

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6"

# Begin Custom Build
OutDir=.\Profile P6
InputPath=.\Safediv.asm
InputName=Safediv

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6"

# Begin Custom Build
OutDir=.\Release P6
InputPath=.\Safediv.asm
InputName=Safediv

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release (Proton)"

# Begin Custom Build
OutDir=.\Release Proton
InputPath=.\Safediv.asm
InputName=Safediv

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug (Proton)"

# Begin Custom Build
OutDir=.\Debug Proton
InputPath=.\Safediv.asm
InputName=Safediv

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile (Proton)"

# Begin Custom Build
OutDir=.\Profile Proton
InputPath=.\Safediv.asm
InputName=Safediv

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6 (Proton)"

# Begin Custom Build
OutDir=.\Debug P6 Proton
InputPath=.\Safediv.asm
InputName=Safediv

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6 (Proton)"

# Begin Custom Build
OutDir=.\Profile P6 Proton
InputPath=.\Safediv.asm
InputName=Safediv

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6 (Proton)"

# Begin Custom Build
OutDir=.\Release P6 Proton
InputPath=.\Safediv.asm
InputName=Safediv

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Sar16.asm
USERDEP__SAR16="drv.inc"	

!IF  "$(CFG)" == "pentprim - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=.\Sar16.asm
InputName=Sar16

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=.\Sar16.asm
InputName=Sar16

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile"

# Begin Custom Build
OutDir=.\Profile
InputPath=.\Sar16.asm
InputName=Sar16

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6"

# Begin Custom Build
OutDir=.\Debug P6
InputPath=.\Sar16.asm
InputName=Sar16

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6"

# Begin Custom Build
OutDir=.\Profile P6
InputPath=.\Sar16.asm
InputName=Sar16

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6"

# Begin Custom Build
OutDir=.\Release P6
InputPath=.\Sar16.asm
InputName=Sar16

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release (Proton)"

# Begin Custom Build
OutDir=.\Release Proton
InputPath=.\Sar16.asm
InputName=Sar16

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug (Proton)"

# Begin Custom Build
OutDir=.\Debug Proton
InputPath=.\Sar16.asm
InputName=Sar16

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile (Proton)"

# Begin Custom Build
OutDir=.\Profile Proton
InputPath=.\Sar16.asm
InputName=Sar16

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6 (Proton)"

# Begin Custom Build
OutDir=.\Debug P6 Proton
InputPath=.\Sar16.asm
InputName=Sar16

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6 (Proton)"

# Begin Custom Build
OutDir=.\Profile P6 Proton
InputPath=.\Sar16.asm
InputName=Sar16

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6 (Proton)"

# Begin Custom Build
OutDir=.\Release P6 Proton
InputPath=.\Sar16.asm
InputName=Sar16

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\setupdat.asm
USERDEP__SETUP="drv.inc"	

!IF  "$(CFG)" == "pentprim - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=.\setupdat.asm
InputName=setupdat

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=.\setupdat.asm
InputName=setupdat

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile"

# Begin Custom Build
OutDir=.\Profile
InputPath=.\setupdat.asm
InputName=setupdat

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6"

# Begin Custom Build
OutDir=.\Debug P6
InputPath=.\setupdat.asm
InputName=setupdat

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6"

# Begin Custom Build
OutDir=.\Profile P6
InputPath=.\setupdat.asm
InputName=setupdat

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6"

# Begin Custom Build
OutDir=.\Release P6
InputPath=.\setupdat.asm
InputName=setupdat

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release (Proton)"

# Begin Custom Build
OutDir=.\Release Proton
InputPath=.\setupdat.asm
InputName=setupdat

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug (Proton)"

# Begin Custom Build
OutDir=.\Debug Proton
InputPath=.\setupdat.asm
InputName=setupdat

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile (Proton)"

# Begin Custom Build
OutDir=.\Profile Proton
InputPath=.\setupdat.asm
InputName=setupdat

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6 (Proton)"

# Begin Custom Build
OutDir=.\Debug P6 Proton
InputPath=.\setupdat.asm
InputName=setupdat

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6 (Proton)"

# Begin Custom Build
OutDir=.\Profile P6 Proton
InputPath=.\setupdat.asm
InputName=setupdat

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6 (Proton)"

# Begin Custom Build
OutDir=.\Release P6 Proton
InputPath=.\setupdat.asm
InputName=setupdat

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\T15_pip.asm
USERDEP__T15_P="drv.inc"	

!IF  "$(CFG)" == "pentprim - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=.\T15_pip.asm
InputName=T15_pip

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=.\T15_pip.asm
InputName=T15_pip

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile"

# Begin Custom Build
OutDir=.\Profile
InputPath=.\T15_pip.asm
InputName=T15_pip

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6"

# Begin Custom Build
OutDir=.\Debug P6
InputPath=.\T15_pip.asm
InputName=T15_pip

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6"

# Begin Custom Build
OutDir=.\Profile P6
InputPath=.\T15_pip.asm
InputName=T15_pip

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6"

# Begin Custom Build
OutDir=.\Release P6
InputPath=.\T15_pip.asm
InputName=T15_pip

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release (Proton)"

# Begin Custom Build
OutDir=.\Release Proton
InputPath=.\T15_pip.asm
InputName=T15_pip

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug (Proton)"

# Begin Custom Build
OutDir=.\Debug Proton
InputPath=.\T15_pip.asm
InputName=T15_pip

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile (Proton)"

# Begin Custom Build
OutDir=.\Profile Proton
InputPath=.\T15_pip.asm
InputName=T15_pip

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6 (Proton)"

# Begin Custom Build
OutDir=.\Debug P6 Proton
InputPath=.\T15_pip.asm
InputName=T15_pip

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6 (Proton)"

# Begin Custom Build
OutDir=.\Profile P6 Proton
InputPath=.\T15_pip.asm
InputName=T15_pip

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6 (Proton)"

# Begin Custom Build
OutDir=.\Release P6 Proton
InputPath=.\T15_pip.asm
InputName=T15_pip

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\T15_pizp.asm
USERDEP__T15_PI="drv.inc"	

!IF  "$(CFG)" == "pentprim - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=.\T15_pizp.asm
InputName=T15_pizp

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=.\T15_pizp.asm
InputName=T15_pizp

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile"

# Begin Custom Build
OutDir=.\Profile
InputPath=.\T15_pizp.asm
InputName=T15_pizp

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6"

# Begin Custom Build
OutDir=.\Debug P6
InputPath=.\T15_pizp.asm
InputName=T15_pizp

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6"

# Begin Custom Build
OutDir=.\Profile P6
InputPath=.\T15_pizp.asm
InputName=T15_pizp

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6"

# Begin Custom Build
OutDir=.\Release P6
InputPath=.\T15_pizp.asm
InputName=T15_pizp

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release (Proton)"

# Begin Custom Build
OutDir=.\Release Proton
InputPath=.\T15_pizp.asm
InputName=T15_pizp

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug (Proton)"

# Begin Custom Build
OutDir=.\Debug Proton
InputPath=.\T15_pizp.asm
InputName=T15_pizp

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile (Proton)"

# Begin Custom Build
OutDir=.\Profile Proton
InputPath=.\T15_pizp.asm
InputName=T15_pizp

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6 (Proton)"

# Begin Custom Build
OutDir=.\Debug P6 Proton
InputPath=.\T15_pizp.asm
InputName=T15_pizp

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6 (Proton)"

# Begin Custom Build
OutDir=.\Profile P6 Proton
InputPath=.\T15_pizp.asm
InputName=T15_pizp

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6 (Proton)"

# Begin Custom Build
OutDir=.\Release P6 Proton
InputPath=.\T15_pizp.asm
InputName=T15_pizp

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\T_pia.asm
USERDEP__T_PIA="drv.inc"	

!IF  "$(CFG)" == "pentprim - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=.\T_pia.asm
InputName=T_pia

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=.\T_pia.asm
InputName=T_pia

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile"

# Begin Custom Build
OutDir=.\Profile
InputPath=.\T_pia.asm
InputName=T_pia

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6"

# Begin Custom Build
OutDir=.\Debug P6
InputPath=.\T_pia.asm
InputName=T_pia

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6"

# Begin Custom Build
OutDir=.\Profile P6
InputPath=.\T_pia.asm
InputName=T_pia

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6"

# Begin Custom Build
OutDir=.\Release P6
InputPath=.\T_pia.asm
InputName=T_pia

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release (Proton)"

# Begin Custom Build
OutDir=.\Release Proton
InputPath=.\T_pia.asm
InputName=T_pia

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug (Proton)"

# Begin Custom Build
OutDir=.\Debug Proton
InputPath=.\T_pia.asm
InputName=T_pia

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile (Proton)"

# Begin Custom Build
OutDir=.\Profile Proton
InputPath=.\T_pia.asm
InputName=T_pia

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6 (Proton)"

# Begin Custom Build
OutDir=.\Debug P6 Proton
InputPath=.\T_pia.asm
InputName=T_pia

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6 (Proton)"

# Begin Custom Build
OutDir=.\Profile P6 Proton
InputPath=.\T_pia.asm
InputName=T_pia

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6 (Proton)"

# Begin Custom Build
OutDir=.\Release P6 Proton
InputPath=.\T_pia.asm
InputName=T_pia

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\T_piza.asm
USERDEP__T_PIZ="drv.inc"	

!IF  "$(CFG)" == "pentprim - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=.\T_piza.asm
InputName=T_piza

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=.\T_piza.asm
InputName=T_piza

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile"

# Begin Custom Build
OutDir=.\Profile
InputPath=.\T_piza.asm
InputName=T_piza

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6"

# Begin Custom Build
OutDir=.\Debug P6
InputPath=.\T_piza.asm
InputName=T_piza

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6"

# Begin Custom Build
OutDir=.\Profile P6
InputPath=.\T_piza.asm
InputName=T_piza

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6"

# Begin Custom Build
OutDir=.\Release P6
InputPath=.\T_piza.asm
InputName=T_piza

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release (Proton)"

# Begin Custom Build
OutDir=.\Release Proton
InputPath=.\T_piza.asm
InputName=T_piza

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug (Proton)"

# Begin Custom Build
OutDir=.\Debug Proton
InputPath=.\T_piza.asm
InputName=T_piza

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile (Proton)"

# Begin Custom Build
OutDir=.\Profile Proton
InputPath=.\T_piza.asm
InputName=T_piza

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6 (Proton)"

# Begin Custom Build
OutDir=.\Debug P6 Proton
InputPath=.\T_piza.asm
InputName=T_piza

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6 (Proton)"

# Begin Custom Build
OutDir=.\Profile P6 Proton
InputPath=.\T_piza.asm
InputName=T_piza

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6 (Proton)"

# Begin Custom Build
OutDir=.\Release P6 Proton
InputPath=.\T_piza.asm
InputName=T_piza

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Ti8_pi.asm
USERDEP__TI8_P="drv.inc"	

!IF  "$(CFG)" == "pentprim - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=.\Ti8_pi.asm
InputName=Ti8_pi

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=.\Ti8_pi.asm
InputName=Ti8_pi

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile"

# Begin Custom Build
OutDir=.\Profile
InputPath=.\Ti8_pi.asm
InputName=Ti8_pi

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6"

# Begin Custom Build
OutDir=.\Debug P6
InputPath=.\Ti8_pi.asm
InputName=Ti8_pi

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6"

# Begin Custom Build
OutDir=.\Profile P6
InputPath=.\Ti8_pi.asm
InputName=Ti8_pi

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6"

# Begin Custom Build
OutDir=.\Release P6
InputPath=.\Ti8_pi.asm
InputName=Ti8_pi

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release (Proton)"

# Begin Custom Build
OutDir=.\Release Proton
InputPath=.\Ti8_pi.asm
InputName=Ti8_pi

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug (Proton)"

# Begin Custom Build
OutDir=.\Debug Proton
InputPath=.\Ti8_pi.asm
InputName=Ti8_pi

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile (Proton)"

# Begin Custom Build
OutDir=.\Profile Proton
InputPath=.\Ti8_pi.asm
InputName=Ti8_pi

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6 (Proton)"

# Begin Custom Build
OutDir=.\Debug P6 Proton
InputPath=.\Ti8_pi.asm
InputName=Ti8_pi

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6 (Proton)"

# Begin Custom Build
OutDir=.\Profile P6 Proton
InputPath=.\Ti8_pi.asm
InputName=Ti8_pi

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6 (Proton)"

# Begin Custom Build
OutDir=.\Release P6 Proton
InputPath=.\Ti8_pi.asm
InputName=Ti8_pi

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Ti8_pip.asm
USERDEP__TI8_PI="drv.inc"	

!IF  "$(CFG)" == "pentprim - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=.\Ti8_pip.asm
InputName=Ti8_pip

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=.\Ti8_pip.asm
InputName=Ti8_pip

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile"

# Begin Custom Build
OutDir=.\Profile
InputPath=.\Ti8_pip.asm
InputName=Ti8_pip

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6"

# Begin Custom Build
OutDir=.\Debug P6
InputPath=.\Ti8_pip.asm
InputName=Ti8_pip

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6"

# Begin Custom Build
OutDir=.\Profile P6
InputPath=.\Ti8_pip.asm
InputName=Ti8_pip

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6"

# Begin Custom Build
OutDir=.\Release P6
InputPath=.\Ti8_pip.asm
InputName=Ti8_pip

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release (Proton)"

# Begin Custom Build
OutDir=.\Release Proton
InputPath=.\Ti8_pip.asm
InputName=Ti8_pip

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug (Proton)"

# Begin Custom Build
OutDir=.\Debug Proton
InputPath=.\Ti8_pip.asm
InputName=Ti8_pip

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile (Proton)"

# Begin Custom Build
OutDir=.\Profile Proton
InputPath=.\Ti8_pip.asm
InputName=Ti8_pip

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6 (Proton)"

# Begin Custom Build
OutDir=.\Debug P6 Proton
InputPath=.\Ti8_pip.asm
InputName=Ti8_pip

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6 (Proton)"

# Begin Custom Build
OutDir=.\Profile P6 Proton
InputPath=.\Ti8_pip.asm
InputName=Ti8_pip

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6 (Proton)"

# Begin Custom Build
OutDir=.\Release P6 Proton
InputPath=.\Ti8_pip.asm
InputName=Ti8_pip

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Ti8_piz.asm
USERDEP__TI8_PIZ="drv.inc"	

!IF  "$(CFG)" == "pentprim - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=.\Ti8_piz.asm
InputName=Ti8_piz

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=.\Ti8_piz.asm
InputName=Ti8_piz

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile"

# Begin Custom Build
OutDir=.\Profile
InputPath=.\Ti8_piz.asm
InputName=Ti8_piz

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6"

# Begin Custom Build
OutDir=.\Debug P6
InputPath=.\Ti8_piz.asm
InputName=Ti8_piz

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6"

# Begin Custom Build
OutDir=.\Profile P6
InputPath=.\Ti8_piz.asm
InputName=Ti8_piz

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6"

# Begin Custom Build
OutDir=.\Release P6
InputPath=.\Ti8_piz.asm
InputName=Ti8_piz

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release (Proton)"

# Begin Custom Build
OutDir=.\Release Proton
InputPath=.\Ti8_piz.asm
InputName=Ti8_piz

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug (Proton)"

# Begin Custom Build
OutDir=.\Debug Proton
InputPath=.\Ti8_piz.asm
InputName=Ti8_piz

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile (Proton)"

# Begin Custom Build
OutDir=.\Profile Proton
InputPath=.\Ti8_piz.asm
InputName=Ti8_piz

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6 (Proton)"

# Begin Custom Build
OutDir=.\Debug P6 Proton
InputPath=.\Ti8_piz.asm
InputName=Ti8_piz

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6 (Proton)"

# Begin Custom Build
OutDir=.\Profile P6 Proton
InputPath=.\Ti8_piz.asm
InputName=Ti8_piz

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6 (Proton)"

# Begin Custom Build
OutDir=.\Release P6 Proton
InputPath=.\Ti8_piz.asm
InputName=Ti8_piz

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Ti8_pizp.asm
USERDEP__TI8_PIZP="drv.inc"	

!IF  "$(CFG)" == "pentprim - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=.\Ti8_pizp.asm
InputName=Ti8_pizp

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=.\Ti8_pizp.asm
InputName=Ti8_pizp

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile"

# Begin Custom Build
OutDir=.\Profile
InputPath=.\Ti8_pizp.asm
InputName=Ti8_pizp

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6"

# Begin Custom Build
OutDir=.\Debug P6
InputPath=.\Ti8_pizp.asm
InputName=Ti8_pizp

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6"

# Begin Custom Build
OutDir=.\Profile P6
InputPath=.\Ti8_pizp.asm
InputName=Ti8_pizp

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6"

# Begin Custom Build
OutDir=.\Release P6
InputPath=.\Ti8_pizp.asm
InputName=Ti8_pizp

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release (Proton)"

# Begin Custom Build
OutDir=.\Release Proton
InputPath=.\Ti8_pizp.asm
InputName=Ti8_pizp

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug (Proton)"

# Begin Custom Build
OutDir=.\Debug Proton
InputPath=.\Ti8_pizp.asm
InputName=Ti8_pizp

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile (Proton)"

# Begin Custom Build
OutDir=.\Profile Proton
InputPath=.\Ti8_pizp.asm
InputName=Ti8_pizp

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6 (Proton)"

# Begin Custom Build
OutDir=.\Debug P6 Proton
InputPath=.\Ti8_pizp.asm
InputName=Ti8_pizp

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6 (Proton)"

# Begin Custom Build
OutDir=.\Profile P6 Proton
InputPath=.\Ti8_pizp.asm
InputName=Ti8_pizp

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6 (Proton)"

# Begin Custom Build
OutDir=.\Release P6 Proton
InputPath=.\Ti8_pizp.asm
InputName=Ti8_pizp

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Tt15_pi.asm
USERDEP__TT15_="drv.inc"	

!IF  "$(CFG)" == "pentprim - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=.\Tt15_pi.asm
InputName=Tt15_pi

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=.\Tt15_pi.asm
InputName=Tt15_pi

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile"

# Begin Custom Build
OutDir=.\Profile
InputPath=.\Tt15_pi.asm
InputName=Tt15_pi

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6"

# Begin Custom Build
OutDir=.\Debug P6
InputPath=.\Tt15_pi.asm
InputName=Tt15_pi

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6"

# Begin Custom Build
OutDir=.\Profile P6
InputPath=.\Tt15_pi.asm
InputName=Tt15_pi

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6"

# Begin Custom Build
OutDir=.\Release P6
InputPath=.\Tt15_pi.asm
InputName=Tt15_pi

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release (Proton)"

# Begin Custom Build
OutDir=.\Release Proton
InputPath=.\Tt15_pi.asm
InputName=Tt15_pi

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug (Proton)"

# Begin Custom Build
OutDir=.\Debug Proton
InputPath=.\Tt15_pi.asm
InputName=Tt15_pi

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile (Proton)"

# Begin Custom Build
OutDir=.\Profile Proton
InputPath=.\Tt15_pi.asm
InputName=Tt15_pi

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6 (Proton)"

# Begin Custom Build
OutDir=.\Debug P6 Proton
InputPath=.\Tt15_pi.asm
InputName=Tt15_pi

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6 (Proton)"

# Begin Custom Build
OutDir=.\Profile P6 Proton
InputPath=.\Tt15_pi.asm
InputName=Tt15_pi

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6 (Proton)"

# Begin Custom Build
OutDir=.\Release P6 Proton
InputPath=.\Tt15_pi.asm
InputName=Tt15_pi

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Tt15_piz.asm
USERDEP__TT15_P="drv.inc"	

!IF  "$(CFG)" == "pentprim - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=.\Tt15_piz.asm
InputName=Tt15_piz

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=.\Tt15_piz.asm
InputName=Tt15_piz

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile"

# Begin Custom Build
OutDir=.\Profile
InputPath=.\Tt15_piz.asm
InputName=Tt15_piz

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6"

# Begin Custom Build
OutDir=.\Debug P6
InputPath=.\Tt15_piz.asm
InputName=Tt15_piz

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6"

# Begin Custom Build
OutDir=.\Profile P6
InputPath=.\Tt15_piz.asm
InputName=Tt15_piz

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6"

# Begin Custom Build
OutDir=.\Release P6
InputPath=.\Tt15_piz.asm
InputName=Tt15_piz

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release (Proton)"

# Begin Custom Build
OutDir=.\Release Proton
InputPath=.\Tt15_piz.asm
InputName=Tt15_piz

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug (Proton)"

# Begin Custom Build
OutDir=.\Debug Proton
InputPath=.\Tt15_piz.asm
InputName=Tt15_piz

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile (Proton)"

# Begin Custom Build
OutDir=.\Profile Proton
InputPath=.\Tt15_piz.asm
InputName=Tt15_piz

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6 (Proton)"

# Begin Custom Build
OutDir=.\Debug P6 Proton
InputPath=.\Tt15_piz.asm
InputName=Tt15_piz

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6 (Proton)"

# Begin Custom Build
OutDir=.\Profile P6 Proton
InputPath=.\Tt15_piz.asm
InputName=Tt15_piz

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6 (Proton)"

# Begin Custom Build
OutDir=.\Release P6 Proton
InputPath=.\Tt15_piz.asm
InputName=Tt15_piz

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Tt16_pi.asm
USERDEP__TT16_="drv.inc"	

!IF  "$(CFG)" == "pentprim - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=.\Tt16_pi.asm
InputName=Tt16_pi

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=.\Tt16_pi.asm
InputName=Tt16_pi

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile"

# Begin Custom Build
OutDir=.\Profile
InputPath=.\Tt16_pi.asm
InputName=Tt16_pi

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6"

# Begin Custom Build
OutDir=.\Debug P6
InputPath=.\Tt16_pi.asm
InputName=Tt16_pi

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6"

# Begin Custom Build
OutDir=.\Profile P6
InputPath=.\Tt16_pi.asm
InputName=Tt16_pi

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6"

# Begin Custom Build
OutDir=.\Release P6
InputPath=.\Tt16_pi.asm
InputName=Tt16_pi

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release (Proton)"

# Begin Custom Build
OutDir=.\Release Proton
InputPath=.\Tt16_pi.asm
InputName=Tt16_pi

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug (Proton)"

# Begin Custom Build
OutDir=.\Debug Proton
InputPath=.\Tt16_pi.asm
InputName=Tt16_pi

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile (Proton)"

# Begin Custom Build
OutDir=.\Profile Proton
InputPath=.\Tt16_pi.asm
InputName=Tt16_pi

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6 (Proton)"

# Begin Custom Build
OutDir=.\Debug P6 Proton
InputPath=.\Tt16_pi.asm
InputName=Tt16_pi

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6 (Proton)"

# Begin Custom Build
OutDir=.\Profile P6 Proton
InputPath=.\Tt16_pi.asm
InputName=Tt16_pi

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6 (Proton)"

# Begin Custom Build
OutDir=.\Release P6 Proton
InputPath=.\Tt16_pi.asm
InputName=Tt16_pi

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Tt16_piz.asm
USERDEP__TT16_P="drv.inc"	

!IF  "$(CFG)" == "pentprim - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=.\Tt16_piz.asm
InputName=Tt16_piz

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=.\Tt16_piz.asm
InputName=Tt16_piz

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile"

# Begin Custom Build
OutDir=.\Profile
InputPath=.\Tt16_piz.asm
InputName=Tt16_piz

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6"

# Begin Custom Build
OutDir=.\Debug P6
InputPath=.\Tt16_piz.asm
InputName=Tt16_piz

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6"

# Begin Custom Build
OutDir=.\Profile P6
InputPath=.\Tt16_piz.asm
InputName=Tt16_piz

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6"

# Begin Custom Build
OutDir=.\Release P6
InputPath=.\Tt16_piz.asm
InputName=Tt16_piz

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release (Proton)"

# Begin Custom Build
OutDir=.\Release Proton
InputPath=.\Tt16_piz.asm
InputName=Tt16_piz

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug (Proton)"

# Begin Custom Build
OutDir=.\Debug Proton
InputPath=.\Tt16_piz.asm
InputName=Tt16_piz

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile (Proton)"

# Begin Custom Build
OutDir=.\Profile Proton
InputPath=.\Tt16_piz.asm
InputName=Tt16_piz

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6 (Proton)"

# Begin Custom Build
OutDir=.\Debug P6 Proton
InputPath=.\Tt16_piz.asm
InputName=Tt16_piz

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6 (Proton)"

# Begin Custom Build
OutDir=.\Profile P6 Proton
InputPath=.\Tt16_piz.asm
InputName=Tt16_piz

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6 (Proton)"

# Begin Custom Build
OutDir=.\Release P6 Proton
InputPath=.\Tt16_piz.asm
InputName=Tt16_piz

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\tt24_pi.asm
USERDEP__TT24_="drv.inc"	

!IF  "$(CFG)" == "pentprim - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=.\tt24_pi.asm
InputName=tt24_pi

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=.\tt24_pi.asm
InputName=tt24_pi

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile"

# Begin Custom Build
OutDir=.\Profile
InputPath=.\tt24_pi.asm
InputName=tt24_pi

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6"

# Begin Custom Build
OutDir=.\Debug P6
InputPath=.\tt24_pi.asm
InputName=tt24_pi

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6"

# Begin Custom Build
OutDir=.\Profile P6
InputPath=.\tt24_pi.asm
InputName=tt24_pi

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6"

# Begin Custom Build
OutDir=.\Release P6
InputPath=.\tt24_pi.asm
InputName=tt24_pi

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release (Proton)"

# Begin Custom Build
OutDir=.\Release Proton
InputPath=.\tt24_pi.asm
InputName=tt24_pi

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug (Proton)"

# Begin Custom Build
OutDir=.\Debug Proton
InputPath=.\tt24_pi.asm
InputName=tt24_pi

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile (Proton)"

# Begin Custom Build
OutDir=.\Profile Proton
InputPath=.\tt24_pi.asm
InputName=tt24_pi

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6 (Proton)"

# Begin Custom Build
OutDir=.\Debug P6 Proton
InputPath=.\tt24_pi.asm
InputName=tt24_pi

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6 (Proton)"

# Begin Custom Build
OutDir=.\Profile P6 Proton
InputPath=.\tt24_pi.asm
InputName=tt24_pi

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6 (Proton)"

# Begin Custom Build
OutDir=.\Release P6 Proton
InputPath=.\tt24_pi.asm
InputName=tt24_pi

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Tt24_piz.asm
USERDEP__TT24_P="drv.inc"	

!IF  "$(CFG)" == "pentprim - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=.\Tt24_piz.asm
InputName=Tt24_piz

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=.\Tt24_piz.asm
InputName=Tt24_piz

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile"

# Begin Custom Build
OutDir=.\Profile
InputPath=.\Tt24_piz.asm
InputName=Tt24_piz

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6"

# Begin Custom Build
OutDir=.\Debug P6
InputPath=.\Tt24_piz.asm
InputName=Tt24_piz

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6"

# Begin Custom Build
OutDir=.\Profile P6
InputPath=.\Tt24_piz.asm
InputName=Tt24_piz

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6"

# Begin Custom Build
OutDir=.\Release P6
InputPath=.\Tt24_piz.asm
InputName=Tt24_piz

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release (Proton)"

# Begin Custom Build
OutDir=.\Release Proton
InputPath=.\Tt24_piz.asm
InputName=Tt24_piz

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug (Proton)"

# Begin Custom Build
OutDir=.\Debug Proton
InputPath=.\Tt24_piz.asm
InputName=Tt24_piz

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile (Proton)"

# Begin Custom Build
OutDir=.\Profile Proton
InputPath=.\Tt24_piz.asm
InputName=Tt24_piz

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6 (Proton)"

# Begin Custom Build
OutDir=.\Debug P6 Proton
InputPath=.\Tt24_piz.asm
InputName=Tt24_piz

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6 (Proton)"

# Begin Custom Build
OutDir=.\Profile P6 Proton
InputPath=.\Tt24_piz.asm
InputName=Tt24_piz

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6 (Proton)"

# Begin Custom Build
OutDir=.\Release P6 Proton
InputPath=.\Tt24_piz.asm
InputName=Tt24_piz

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\xzrgb.asm
USERDEP__XZRGB="drv.inc"	

!IF  "$(CFG)" == "pentprim - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=.\xzrgb.asm
InputName=xzrgb

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=.\xzrgb.asm
InputName=xzrgb

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile"

# Begin Custom Build
OutDir=.\Profile
InputPath=.\xzrgb.asm
InputName=xzrgb

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6"

# Begin Custom Build
OutDir=.\Debug P6
InputPath=.\xzrgb.asm
InputName=xzrgb

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6"

# Begin Custom Build
OutDir=.\Profile P6
InputPath=.\xzrgb.asm
InputName=xzrgb

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6"

# Begin Custom Build
OutDir=.\Release P6
InputPath=.\xzrgb.asm
InputName=xzrgb

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release (Proton)"

# Begin Custom Build
OutDir=.\Release Proton
InputPath=.\xzrgb.asm
InputName=xzrgb

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug (Proton)"

# Begin Custom Build
OutDir=.\Debug Proton
InputPath=.\xzrgb.asm
InputName=xzrgb

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile (Proton)"

# Begin Custom Build
OutDir=.\Profile Proton
InputPath=.\xzrgb.asm
InputName=xzrgb

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6 (Proton)"

# Begin Custom Build
OutDir=.\Debug P6 Proton
InputPath=.\xzrgb.asm
InputName=xzrgb

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6 (Proton)"

# Begin Custom Build
OutDir=.\Profile P6 Proton
InputPath=.\xzrgb.asm
InputName=xzrgb

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6 (Proton)"

# Begin Custom Build
OutDir=.\Release P6 Proton
InputPath=.\xzrgb.asm
InputName=xzrgb

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\xzrgb555.asm

!IF  "$(CFG)" == "pentprim - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=.\xzrgb555.asm
InputName=xzrgb555

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=.\xzrgb555.asm
InputName=xzrgb555

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile"

# Begin Custom Build
OutDir=.\Profile
InputPath=.\xzrgb555.asm
InputName=xzrgb555

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6"

# Begin Custom Build
OutDir=.\Debug P6
InputPath=.\xzrgb555.asm
InputName=xzrgb555

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6"

# Begin Custom Build
OutDir=.\Profile P6
InputPath=.\xzrgb555.asm
InputName=xzrgb555

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6"

# Begin Custom Build
OutDir=.\Release P6
InputPath=.\xzrgb555.asm
InputName=xzrgb555

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release (Proton)"

# Begin Custom Build
OutDir=.\Release Proton
InputPath=.\xzrgb555.asm
InputName=xzrgb555

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug (Proton)"

# Begin Custom Build
OutDir=.\Debug Proton
InputPath=.\xzrgb555.asm
InputName=xzrgb555

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile (Proton)"

# Begin Custom Build
OutDir=.\Profile Proton
InputPath=.\xzrgb555.asm
InputName=xzrgb555

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6 (Proton)"

# Begin Custom Build
OutDir=.\Debug P6 Proton
InputPath=.\xzrgb555.asm
InputName=xzrgb555

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6 (Proton)"

# Begin Custom Build
OutDir=.\Profile P6 Proton
InputPath=.\xzrgb555.asm
InputName=xzrgb555

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6 (Proton)"

# Begin Custom Build
OutDir=.\Release P6 Proton
InputPath=.\xzrgb555.asm
InputName=xzrgb555

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\xzuv.asm
USERDEP__XZUV_="drv.inc"	

!IF  "$(CFG)" == "pentprim - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=.\xzuv.asm
InputName=xzuv

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=.\xzuv.asm
InputName=xzuv

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile"

# Begin Custom Build
OutDir=.\Profile
InputPath=.\xzuv.asm
InputName=xzuv

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6"

# Begin Custom Build
OutDir=.\Debug P6
InputPath=.\xzuv.asm
InputName=xzuv

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6"

# Begin Custom Build
OutDir=.\Profile P6
InputPath=.\xzuv.asm
InputName=xzuv

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6"

# Begin Custom Build
OutDir=.\Release P6
InputPath=.\xzuv.asm
InputName=xzuv

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release (Proton)"

# Begin Custom Build
OutDir=.\Release Proton
InputPath=.\xzuv.asm
InputName=xzuv

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug (Proton)"

# Begin Custom Build
OutDir=.\Debug Proton
InputPath=.\xzuv.asm
InputName=xzuv

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile (Proton)"

# Begin Custom Build
OutDir=.\Profile Proton
InputPath=.\xzuv.asm
InputName=xzuv

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6 (Proton)"

# Begin Custom Build
OutDir=.\Debug P6 Proton
InputPath=.\xzuv.asm
InputName=xzuv

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6 (Proton)"

# Begin Custom Build
OutDir=.\Profile P6 Proton
InputPath=.\xzuv.asm
InputName=xzuv

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6 (Proton)"

# Begin Custom Build
OutDir=.\Release P6 Proton
InputPath=.\xzuv.asm
InputName=xzuv

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\xzuv555.asm

!IF  "$(CFG)" == "pentprim - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=.\xzuv555.asm
InputName=xzuv555

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=.\xzuv555.asm
InputName=xzuv555

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile"

# Begin Custom Build
OutDir=.\Profile
InputPath=.\xzuv555.asm
InputName=xzuv555

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6"

# Begin Custom Build
OutDir=.\Debug P6
InputPath=.\xzuv555.asm
InputName=xzuv555

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6"

# Begin Custom Build
OutDir=.\Profile P6
InputPath=.\xzuv555.asm
InputName=xzuv555

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6"

# Begin Custom Build
OutDir=.\Release P6
InputPath=.\xzuv555.asm
InputName=xzuv555

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release (Proton)"

# Begin Custom Build
OutDir=.\Release Proton
InputPath=.\xzuv555.asm
InputName=xzuv555

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug (Proton)"

# Begin Custom Build
OutDir=.\Debug Proton
InputPath=.\xzuv555.asm
InputName=xzuv555

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile (Proton)"

# Begin Custom Build
OutDir=.\Profile Proton
InputPath=.\xzuv555.asm
InputName=xzuv555

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6 (Proton)"

# Begin Custom Build
OutDir=.\Debug P6 Proton
InputPath=.\xzuv555.asm
InputName=xzuv555

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6 (Proton)"

# Begin Custom Build
OutDir=.\Profile P6 Proton
InputPath=.\xzuv555.asm
InputName=xzuv555

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6 (Proton)"

# Begin Custom Build
OutDir=.\Release P6 Proton
InputPath=.\xzuv555.asm
InputName=xzuv555

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\xzuvc.asm
USERDEP__XZUVC="drv.inc"	

!IF  "$(CFG)" == "pentprim - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=.\xzuvc.asm
InputName=xzuvc

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=.\xzuvc.asm
InputName=xzuvc

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile"

# Begin Custom Build
OutDir=.\Profile
InputPath=.\xzuvc.asm
InputName=xzuvc

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6"

# Begin Custom Build
OutDir=.\Debug P6
InputPath=.\xzuvc.asm
InputName=xzuvc

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6"

# Begin Custom Build
OutDir=.\Profile P6
InputPath=.\xzuvc.asm
InputName=xzuvc

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6"

# Begin Custom Build
OutDir=.\Release P6
InputPath=.\xzuvc.asm
InputName=xzuvc

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release (Proton)"

# Begin Custom Build
OutDir=.\Release Proton
InputPath=.\xzuvc.asm
InputName=xzuvc

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug (Proton)"

# Begin Custom Build
OutDir=.\Debug Proton
InputPath=.\xzuvc.asm
InputName=xzuvc

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile (Proton)"

# Begin Custom Build
OutDir=.\Profile Proton
InputPath=.\xzuvc.asm
InputName=xzuvc

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6 (Proton)"

# Begin Custom Build
OutDir=.\Debug P6 Proton
InputPath=.\xzuvc.asm
InputName=xzuvc

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6 (Proton)"

# Begin Custom Build
OutDir=.\Profile P6 Proton
InputPath=.\xzuvc.asm
InputName=xzuvc

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6 (Proton)"

# Begin Custom Build
OutDir=.\Release P6 Proton
InputPath=.\xzuvc.asm
InputName=xzuvc

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\xzuvc555.asm

!IF  "$(CFG)" == "pentprim - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=.\xzuvc555.asm
InputName=xzuvc555

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=.\xzuvc555.asm
InputName=xzuvc555

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile"

# Begin Custom Build
OutDir=.\Profile
InputPath=.\xzuvc555.asm
InputName=xzuvc555

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6"

# Begin Custom Build
OutDir=.\Debug P6
InputPath=.\xzuvc555.asm
InputName=xzuvc555

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6"

# Begin Custom Build
OutDir=.\Profile P6
InputPath=.\xzuvc555.asm
InputName=xzuvc555

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6"

# Begin Custom Build
OutDir=.\Release P6
InputPath=.\xzuvc555.asm
InputName=xzuvc555

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release (Proton)"

# Begin Custom Build
OutDir=.\Release Proton
InputPath=.\xzuvc555.asm
InputName=xzuvc555

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug (Proton)"

# Begin Custom Build
OutDir=.\Debug Proton
InputPath=.\xzuvc555.asm
InputName=xzuvc555

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile (Proton)"

# Begin Custom Build
OutDir=.\Profile Proton
InputPath=.\xzuvc555.asm
InputName=xzuvc555

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6 (Proton)"

# Begin Custom Build
OutDir=.\Debug P6 Proton
InputPath=.\xzuvc555.asm
InputName=xzuvc555

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6 (Proton)"

# Begin Custom Build
OutDir=.\Profile P6 Proton
InputPath=.\xzuvc555.asm
InputName=xzuvc555

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6 (Proton)"

# Begin Custom Build
OutDir=.\Release P6 Proton
InputPath=.\xzuvc555.asm
InputName=xzuvc555

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\xzuvrgb.asm
USERDEP__XZUVR="drv.inc"	

!IF  "$(CFG)" == "pentprim - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=.\xzuvrgb.asm
InputName=xzuvrgb

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=.\xzuvrgb.asm
InputName=xzuvrgb

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile"

# Begin Custom Build
OutDir=.\Profile
InputPath=.\xzuvrgb.asm
InputName=xzuvrgb

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6"

# Begin Custom Build
OutDir=.\Debug P6
InputPath=.\xzuvrgb.asm
InputName=xzuvrgb

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6"

# Begin Custom Build
OutDir=.\Profile P6
InputPath=.\xzuvrgb.asm
InputName=xzuvrgb

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6"

# Begin Custom Build
OutDir=.\Release P6
InputPath=.\xzuvrgb.asm
InputName=xzuvrgb

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release (Proton)"

# Begin Custom Build
OutDir=.\Release Proton
InputPath=.\xzuvrgb.asm
InputName=xzuvrgb

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug (Proton)"

# Begin Custom Build
OutDir=.\Debug Proton
InputPath=.\xzuvrgb.asm
InputName=xzuvrgb

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile (Proton)"

# Begin Custom Build
OutDir=.\Profile Proton
InputPath=.\xzuvrgb.asm
InputName=xzuvrgb

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6 (Proton)"

# Begin Custom Build
OutDir=.\Debug P6 Proton
InputPath=.\xzuvrgb.asm
InputName=xzuvrgb

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6 (Proton)"

# Begin Custom Build
OutDir=.\Profile P6 Proton
InputPath=.\xzuvrgb.asm
InputName=xzuvrgb

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6 (Proton)"

# Begin Custom Build
OutDir=.\Release P6 Proton
InputPath=.\xzuvrgb.asm
InputName=xzuvrgb

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\xzuvrgb555.asm

!IF  "$(CFG)" == "pentprim - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=.\xzuvrgb555.asm
InputName=xzuvrgb555

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=.\xzuvrgb555.asm
InputName=xzuvrgb555

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile"

# Begin Custom Build
OutDir=.\Profile
InputPath=.\xzuvrgb555.asm
InputName=xzuvrgb555

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6"

# Begin Custom Build
OutDir=.\Debug P6
InputPath=.\xzuvrgb555.asm
InputName=xzuvrgb555

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6"

# Begin Custom Build
OutDir=.\Profile P6
InputPath=.\xzuvrgb555.asm
InputName=xzuvrgb555

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6"

# Begin Custom Build
OutDir=.\Release P6
InputPath=.\xzuvrgb555.asm
InputName=xzuvrgb555

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release (Proton)"

# Begin Custom Build
OutDir=.\Release Proton
InputPath=.\xzuvrgb555.asm
InputName=xzuvrgb555

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug (Proton)"

# Begin Custom Build
OutDir=.\Debug Proton
InputPath=.\xzuvrgb555.asm
InputName=xzuvrgb555

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile (Proton)"

# Begin Custom Build
OutDir=.\Profile Proton
InputPath=.\xzuvrgb555.asm
InputName=xzuvrgb555

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6 (Proton)"

# Begin Custom Build
OutDir=.\Debug P6 Proton
InputPath=.\xzuvrgb555.asm
InputName=xzuvrgb555

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6 (Proton)"

# Begin Custom Build
OutDir=.\Profile P6 Proton
InputPath=.\xzuvrgb555.asm
InputName=xzuvrgb555

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6 (Proton)"

# Begin Custom Build
OutDir=.\Release P6 Proton
InputPath=.\xzuvrgb555.asm
InputName=xzuvrgb555

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\zb16p2ul.asm
USERDEP__ZB16P="drv.inc"	

!IF  "$(CFG)" == "pentprim - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=.\zb16p2ul.asm
InputName=zb16p2ul

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=.\zb16p2ul.asm
InputName=zb16p2ul

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile"

# Begin Custom Build
OutDir=.\Profile
InputPath=.\zb16p2ul.asm
InputName=zb16p2ul

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6"

# Begin Custom Build
OutDir=.\Debug P6
InputPath=.\zb16p2ul.asm
InputName=zb16p2ul

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6"

# Begin Custom Build
OutDir=.\Profile P6
InputPath=.\zb16p2ul.asm
InputName=zb16p2ul

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6"

# Begin Custom Build
OutDir=.\Release P6
InputPath=.\zb16p2ul.asm
InputName=zb16p2ul

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release (Proton)"

# Begin Custom Build
OutDir=.\Release Proton
InputPath=.\zb16p2ul.asm
InputName=zb16p2ul

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug (Proton)"

# Begin Custom Build
OutDir=.\Debug Proton
InputPath=.\zb16p2ul.asm
InputName=zb16p2ul

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile (Proton)"

# Begin Custom Build
OutDir=.\Profile Proton
InputPath=.\zb16p2ul.asm
InputName=zb16p2ul

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6 (Proton)"

# Begin Custom Build
OutDir=.\Debug P6 Proton
InputPath=.\zb16p2ul.asm
InputName=zb16p2ul

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6 (Proton)"

# Begin Custom Build
OutDir=.\Profile P6 Proton
InputPath=.\zb16p2ul.asm
InputName=zb16p2ul

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6 (Proton)"

# Begin Custom Build
OutDir=.\Release P6 Proton
InputPath=.\zb16p2ul.asm
InputName=zb16p2ul

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\zb8.asm
USERDEP__ZB8_A="drv.inc"	

!IF  "$(CFG)" == "pentprim - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=.\zb8.asm
InputName=zb8

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=.\zb8.asm
InputName=zb8

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile"

# Begin Custom Build
OutDir=.\Profile
InputPath=.\zb8.asm
InputName=zb8

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6"

# Begin Custom Build
OutDir=.\Debug P6
InputPath=.\zb8.asm
InputName=zb8

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6"

# Begin Custom Build
OutDir=.\Profile P6
InputPath=.\zb8.asm
InputName=zb8

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6"

# Begin Custom Build
OutDir=.\Release P6
InputPath=.\zb8.asm
InputName=zb8

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release (Proton)"

# Begin Custom Build
OutDir=.\Release Proton
InputPath=.\zb8.asm
InputName=zb8

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug (Proton)"

# Begin Custom Build
OutDir=.\Debug Proton
InputPath=.\zb8.asm
InputName=zb8

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile (Proton)"

# Begin Custom Build
OutDir=.\Profile Proton
InputPath=.\zb8.asm
InputName=zb8

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6 (Proton)"

# Begin Custom Build
OutDir=.\Debug P6 Proton
InputPath=.\zb8.asm
InputName=zb8

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6 (Proton)"

# Begin Custom Build
OutDir=.\Profile P6 Proton
InputPath=.\zb8.asm
InputName=zb8

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6 (Proton)"

# Begin Custom Build
OutDir=.\Release P6 Proton
InputPath=.\zb8.asm
InputName=zb8

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\zb8awtm.asm
USERDEP__ZB8AW="drv.inc"	

!IF  "$(CFG)" == "pentprim - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=.\zb8awtm.asm
InputName=zb8awtm

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=.\zb8awtm.asm
InputName=zb8awtm

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile"

# Begin Custom Build
OutDir=.\Profile
InputPath=.\zb8awtm.asm
InputName=zb8awtm

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6"

# Begin Custom Build
OutDir=.\Debug P6
InputPath=.\zb8awtm.asm
InputName=zb8awtm

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6"

# Begin Custom Build
OutDir=.\Profile P6
InputPath=.\zb8awtm.asm
InputName=zb8awtm

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6"

# Begin Custom Build
OutDir=.\Release P6
InputPath=.\zb8awtm.asm
InputName=zb8awtm

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release (Proton)"

# Begin Custom Build
OutDir=.\Release Proton
InputPath=.\zb8awtm.asm
InputName=zb8awtm

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug (Proton)"

# Begin Custom Build
OutDir=.\Debug Proton
InputPath=.\zb8awtm.asm
InputName=zb8awtm

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile (Proton)"

# Begin Custom Build
OutDir=.\Profile Proton
InputPath=.\zb8awtm.asm
InputName=zb8awtm

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6 (Proton)"

# Begin Custom Build
OutDir=.\Debug P6 Proton
InputPath=.\zb8awtm.asm
InputName=zb8awtm

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6 (Proton)"

# Begin Custom Build
OutDir=.\Profile P6 Proton
InputPath=.\zb8awtm.asm
InputName=zb8awtm

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6 (Proton)"

# Begin Custom Build
OutDir=.\Release P6 Proton
InputPath=.\zb8awtm.asm
InputName=zb8awtm

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Zb8p2fla.asm
USERDEP__ZB8P2="drv.inc"	

!IF  "$(CFG)" == "pentprim - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=.\Zb8p2fla.asm
InputName=Zb8p2fla

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=.\Zb8p2fla.asm
InputName=Zb8p2fla

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile"

# Begin Custom Build
OutDir=.\Profile
InputPath=.\Zb8p2fla.asm
InputName=Zb8p2fla

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6"

# Begin Custom Build
OutDir=.\Debug P6
InputPath=.\Zb8p2fla.asm
InputName=Zb8p2fla

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6"

# Begin Custom Build
OutDir=.\Profile P6
InputPath=.\Zb8p2fla.asm
InputName=Zb8p2fla

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6"

# Begin Custom Build
OutDir=.\Release P6
InputPath=.\Zb8p2fla.asm
InputName=Zb8p2fla

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release (Proton)"

# Begin Custom Build
OutDir=.\Release Proton
InputPath=.\Zb8p2fla.asm
InputName=Zb8p2fla

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug (Proton)"

# Begin Custom Build
OutDir=.\Debug Proton
InputPath=.\Zb8p2fla.asm
InputName=Zb8p2fla

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile (Proton)"

# Begin Custom Build
OutDir=.\Profile Proton
InputPath=.\Zb8p2fla.asm
InputName=Zb8p2fla

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6 (Proton)"

# Begin Custom Build
OutDir=.\Debug P6 Proton
InputPath=.\Zb8p2fla.asm
InputName=Zb8p2fla

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6 (Proton)"

# Begin Custom Build
OutDir=.\Profile P6 Proton
InputPath=.\Zb8p2fla.asm
InputName=Zb8p2fla

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6 (Proton)"

# Begin Custom Build
OutDir=.\Release P6 Proton
InputPath=.\Zb8p2fla.asm
InputName=Zb8p2fla

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\zb8p2flab.asm

!IF  "$(CFG)" == "pentprim - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=.\zb8p2flab.asm
InputName=zb8p2flab

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /Zi /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=.\zb8p2flab.asm
InputName=zb8p2flab

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /Zi /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile"

# Begin Custom Build
OutDir=.\Profile
InputPath=.\zb8p2flab.asm
InputName=zb8p2flab

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /Zi /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6"

# Begin Custom Build
OutDir=.\Debug P6
InputPath=.\zb8p2flab.asm
InputName=zb8p2flab

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /Zi /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6"

# Begin Custom Build
OutDir=.\Profile P6
InputPath=.\zb8p2flab.asm
InputName=zb8p2flab

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /Zi /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6"

# Begin Custom Build
OutDir=.\Release P6
InputPath=.\zb8p2flab.asm
InputName=zb8p2flab

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /Zi /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release (Proton)"

# Begin Custom Build
OutDir=.\Release Proton
InputPath=.\zb8p2flab.asm
InputName=zb8p2flab

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /Zi /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug (Proton)"

# Begin Custom Build
OutDir=.\Debug Proton
InputPath=.\zb8p2flab.asm
InputName=zb8p2flab

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /Zi /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile (Proton)"

# Begin Custom Build
OutDir=.\Profile Proton
InputPath=.\zb8p2flab.asm
InputName=zb8p2flab

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /Zi /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6 (Proton)"

# Begin Custom Build
OutDir=.\Debug P6 Proton
InputPath=.\zb8p2flab.asm
InputName=zb8p2flab

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /Zi /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6 (Proton)"

# Begin Custom Build
OutDir=.\Profile P6 Proton
InputPath=.\zb8p2flab.asm
InputName=zb8p2flab

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /Zi /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6 (Proton)"

# Begin Custom Build
OutDir=.\Release P6 Proton
InputPath=.\zb8p2flab.asm
InputName=zb8p2flab

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /Zi /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Zb8p2lit.asm
USERDEP__ZB8P2L="drv.inc"	

!IF  "$(CFG)" == "pentprim - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=.\Zb8p2lit.asm
InputName=Zb8p2lit

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=.\Zb8p2lit.asm
InputName=Zb8p2lit

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile"

# Begin Custom Build
OutDir=.\Profile
InputPath=.\Zb8p2lit.asm
InputName=Zb8p2lit

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6"

# Begin Custom Build
OutDir=.\Debug P6
InputPath=.\Zb8p2lit.asm
InputName=Zb8p2lit

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6"

# Begin Custom Build
OutDir=.\Profile P6
InputPath=.\Zb8p2lit.asm
InputName=Zb8p2lit

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6"

# Begin Custom Build
OutDir=.\Release P6
InputPath=.\Zb8p2lit.asm
InputName=Zb8p2lit

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release (Proton)"

# Begin Custom Build
OutDir=.\Release Proton
InputPath=.\Zb8p2lit.asm
InputName=Zb8p2lit

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug (Proton)"

# Begin Custom Build
OutDir=.\Debug Proton
InputPath=.\Zb8p2lit.asm
InputName=Zb8p2lit

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile (Proton)"

# Begin Custom Build
OutDir=.\Profile Proton
InputPath=.\Zb8p2lit.asm
InputName=Zb8p2lit

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6 (Proton)"

# Begin Custom Build
OutDir=.\Debug P6 Proton
InputPath=.\Zb8p2lit.asm
InputName=Zb8p2lit

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6 (Proton)"

# Begin Custom Build
OutDir=.\Profile P6 Proton
InputPath=.\Zb8p2lit.asm
InputName=Zb8p2lit

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6 (Proton)"

# Begin Custom Build
OutDir=.\Release P6 Proton
InputPath=.\Zb8p2lit.asm
InputName=Zb8p2lit

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\zb8p2litb.asm

!IF  "$(CFG)" == "pentprim - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=.\zb8p2litb.asm
InputName=zb8p2litb

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /Zi /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=.\zb8p2litb.asm
InputName=zb8p2litb

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /Zi /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile"

# Begin Custom Build
OutDir=.\Profile
InputPath=.\zb8p2litb.asm
InputName=zb8p2litb

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /Zi /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6"

# Begin Custom Build
OutDir=.\Debug P6
InputPath=.\zb8p2litb.asm
InputName=zb8p2litb

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /Zi /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6"

# Begin Custom Build
OutDir=.\Profile P6
InputPath=.\zb8p2litb.asm
InputName=zb8p2litb

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /Zi /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6"

# Begin Custom Build
OutDir=.\Release P6
InputPath=.\zb8p2litb.asm
InputName=zb8p2litb

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /Zi /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release (Proton)"

# Begin Custom Build
OutDir=.\Release Proton
InputPath=.\zb8p2litb.asm
InputName=zb8p2litb

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /Zi /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug (Proton)"

# Begin Custom Build
OutDir=.\Debug Proton
InputPath=.\zb8p2litb.asm
InputName=zb8p2litb

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /Zi /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile (Proton)"

# Begin Custom Build
OutDir=.\Profile Proton
InputPath=.\zb8p2litb.asm
InputName=zb8p2litb

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /Zi /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6 (Proton)"

# Begin Custom Build
OutDir=.\Debug P6 Proton
InputPath=.\zb8p2litb.asm
InputName=zb8p2litb

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /Zi /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6 (Proton)"

# Begin Custom Build
OutDir=.\Profile P6 Proton
InputPath=.\zb8p2litb.asm
InputName=zb8p2litb

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /Zi /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6 (Proton)"

# Begin Custom Build
OutDir=.\Release P6 Proton
InputPath=.\zb8p2litb.asm
InputName=zb8p2litb

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /Zi /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Zb8p2unl.asm
USERDEP__ZB8P2U="drv.inc"	

!IF  "$(CFG)" == "pentprim - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=.\Zb8p2unl.asm
InputName=Zb8p2unl

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=.\Zb8p2unl.asm
InputName=Zb8p2unl

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile"

# Begin Custom Build
OutDir=.\Profile
InputPath=.\Zb8p2unl.asm
InputName=Zb8p2unl

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6"

# Begin Custom Build
OutDir=.\Debug P6
InputPath=.\Zb8p2unl.asm
InputName=Zb8p2unl

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6"

# Begin Custom Build
OutDir=.\Profile P6
InputPath=.\Zb8p2unl.asm
InputName=Zb8p2unl

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6"

# Begin Custom Build
OutDir=.\Release P6
InputPath=.\Zb8p2unl.asm
InputName=Zb8p2unl

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release (Proton)"

# Begin Custom Build
OutDir=.\Release Proton
InputPath=.\Zb8p2unl.asm
InputName=Zb8p2unl

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug (Proton)"

# Begin Custom Build
OutDir=.\Debug Proton
InputPath=.\Zb8p2unl.asm
InputName=Zb8p2unl

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile (Proton)"

# Begin Custom Build
OutDir=.\Profile Proton
InputPath=.\Zb8p2unl.asm
InputName=Zb8p2unl

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6 (Proton)"

# Begin Custom Build
OutDir=.\Debug P6 Proton
InputPath=.\Zb8p2unl.asm
InputName=Zb8p2unl

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6 (Proton)"

# Begin Custom Build
OutDir=.\Profile P6 Proton
InputPath=.\Zb8p2unl.asm
InputName=Zb8p2unl

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6 (Proton)"

# Begin Custom Build
OutDir=.\Release P6 Proton
InputPath=.\Zb8p2unl.asm
InputName=Zb8p2unl

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\zb8p2unlb.asm

!IF  "$(CFG)" == "pentprim - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=.\zb8p2unlb.asm
InputName=zb8p2unlb

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /Zi /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=.\zb8p2unlb.asm
InputName=zb8p2unlb

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /Zi /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile"

# Begin Custom Build
OutDir=.\Profile
InputPath=.\zb8p2unlb.asm
InputName=zb8p2unlb

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /Zi /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6"

# Begin Custom Build
OutDir=.\Debug P6
InputPath=.\zb8p2unlb.asm
InputName=zb8p2unlb

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /Zi /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6"

# Begin Custom Build
OutDir=.\Profile P6
InputPath=.\zb8p2unlb.asm
InputName=zb8p2unlb

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /Zi /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6"

# Begin Custom Build
OutDir=.\Release P6
InputPath=.\zb8p2unlb.asm
InputName=zb8p2unlb

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /Zi /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release (Proton)"

# Begin Custom Build
OutDir=.\Release Proton
InputPath=.\zb8p2unlb.asm
InputName=zb8p2unlb

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /Zi /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug (Proton)"

# Begin Custom Build
OutDir=.\Debug Proton
InputPath=.\zb8p2unlb.asm
InputName=zb8p2unlb

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /Zi /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile (Proton)"

# Begin Custom Build
OutDir=.\Profile Proton
InputPath=.\zb8p2unlb.asm
InputName=zb8p2unlb

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /Zi /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6 (Proton)"

# Begin Custom Build
OutDir=.\Debug P6 Proton
InputPath=.\zb8p2unlb.asm
InputName=zb8p2unlb

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /Zi /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6 (Proton)"

# Begin Custom Build
OutDir=.\Profile P6 Proton
InputPath=.\zb8p2unlb.asm
InputName=zb8p2unlb

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /Zi /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6 (Proton)"

# Begin Custom Build
OutDir=.\Release P6 Proton
InputPath=.\zb8p2unlb.asm
InputName=zb8p2unlb

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /Zi /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\zb8sh.asm
USERDEP__ZB8SH="drv.inc"	

!IF  "$(CFG)" == "pentprim - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=.\zb8sh.asm
InputName=zb8sh

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=.\zb8sh.asm
InputName=zb8sh

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile"

# Begin Custom Build
OutDir=.\Profile
InputPath=.\zb8sh.asm
InputName=zb8sh

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6"

# Begin Custom Build
OutDir=.\Debug P6
InputPath=.\zb8sh.asm
InputName=zb8sh

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6"

# Begin Custom Build
OutDir=.\Profile P6
InputPath=.\zb8sh.asm
InputName=zb8sh

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6"

# Begin Custom Build
OutDir=.\Release P6
InputPath=.\zb8sh.asm
InputName=zb8sh

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release (Proton)"

# Begin Custom Build
OutDir=.\Release Proton
InputPath=.\zb8sh.asm
InputName=zb8sh

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug (Proton)"

# Begin Custom Build
OutDir=.\Debug Proton
InputPath=.\zb8sh.asm
InputName=zb8sh

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile (Proton)"

# Begin Custom Build
OutDir=.\Profile Proton
InputPath=.\zb8sh.asm
InputName=zb8sh

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6 (Proton)"

# Begin Custom Build
OutDir=.\Debug P6 Proton
InputPath=.\zb8sh.asm
InputName=zb8sh

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6 (Proton)"

# Begin Custom Build
OutDir=.\Profile P6 Proton
InputPath=.\zb8sh.asm
InputName=zb8sh

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6 (Proton)"

# Begin Custom Build
OutDir=.\Release P6 Proton
InputPath=.\zb8sh.asm
InputName=zb8sh

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\zs8.asm
USERDEP__ZS8_A="drv.inc"	

!IF  "$(CFG)" == "pentprim - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=.\zs8.asm
InputName=zs8

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=.\zs8.asm
InputName=zs8

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile"

# Begin Custom Build
OutDir=.\Profile
InputPath=.\zs8.asm
InputName=zs8

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6"

# Begin Custom Build
OutDir=.\Debug P6
InputPath=.\zs8.asm
InputName=zs8

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6"

# Begin Custom Build
OutDir=.\Profile P6
InputPath=.\zs8.asm
InputName=zs8

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6"

# Begin Custom Build
OutDir=.\Release P6
InputPath=.\zs8.asm
InputName=zs8

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release (Proton)"

# Begin Custom Build
OutDir=.\Release Proton
InputPath=.\zs8.asm
InputName=zs8

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug (Proton)"

# Begin Custom Build
OutDir=.\Debug Proton
InputPath=.\zs8.asm
InputName=zs8

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile (Proton)"

# Begin Custom Build
OutDir=.\Profile Proton
InputPath=.\zs8.asm
InputName=zs8

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6 (Proton)"

# Begin Custom Build
OutDir=.\Debug P6 Proton
InputPath=.\zs8.asm
InputName=zs8

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6 (Proton)"

# Begin Custom Build
OutDir=.\Profile P6 Proton
InputPath=.\zs8.asm
InputName=zs8

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6 (Proton)"

# Begin Custom Build
OutDir=.\Release P6 Proton
InputPath=.\zs8.asm
InputName=zs8

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\zs8awtm.asm
USERDEP__ZS8AW="drv.inc"	

!IF  "$(CFG)" == "pentprim - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=.\zs8awtm.asm
InputName=zs8awtm

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=.\zs8awtm.asm
InputName=zs8awtm

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile"

# Begin Custom Build
OutDir=.\Profile
InputPath=.\zs8awtm.asm
InputName=zs8awtm

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6"

# Begin Custom Build
OutDir=.\Debug P6
InputPath=.\zs8awtm.asm
InputName=zs8awtm

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6"

# Begin Custom Build
OutDir=.\Profile P6
InputPath=.\zs8awtm.asm
InputName=zs8awtm

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6"

# Begin Custom Build
OutDir=.\Release P6
InputPath=.\zs8awtm.asm
InputName=zs8awtm

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release (Proton)"

# Begin Custom Build
OutDir=.\Release Proton
InputPath=.\zs8awtm.asm
InputName=zs8awtm

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug (Proton)"

# Begin Custom Build
OutDir=.\Debug Proton
InputPath=.\zs8awtm.asm
InputName=zs8awtm

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile (Proton)"

# Begin Custom Build
OutDir=.\Profile Proton
InputPath=.\zs8awtm.asm
InputName=zs8awtm

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6 (Proton)"

# Begin Custom Build
OutDir=.\Debug P6 Proton
InputPath=.\zs8awtm.asm
InputName=zs8awtm

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6 (Proton)"

# Begin Custom Build
OutDir=.\Profile P6 Proton
InputPath=.\zs8awtm.asm
InputName=zs8awtm

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6 (Proton)"

# Begin Custom Build
OutDir=.\Release P6 Proton
InputPath=.\zs8awtm.asm
InputName=zs8awtm

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\zs8p2unl.asm
USERDEP__ZS8P2="drv.inc"	

!IF  "$(CFG)" == "pentprim - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=.\zs8p2unl.asm
InputName=zs8p2unl

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=.\zs8p2unl.asm
InputName=zs8p2unl

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile"

# Begin Custom Build
OutDir=.\Profile
InputPath=.\zs8p2unl.asm
InputName=zs8p2unl

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6"

# Begin Custom Build
OutDir=.\Debug P6
InputPath=.\zs8p2unl.asm
InputName=zs8p2unl

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6"

# Begin Custom Build
OutDir=.\Profile P6
InputPath=.\zs8p2unl.asm
InputName=zs8p2unl

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6"

# Begin Custom Build
OutDir=.\Release P6
InputPath=.\zs8p2unl.asm
InputName=zs8p2unl

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release (Proton)"

# Begin Custom Build
OutDir=.\Release Proton
InputPath=.\zs8p2unl.asm
InputName=zs8p2unl

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug (Proton)"

# Begin Custom Build
OutDir=.\Debug Proton
InputPath=.\zs8p2unl.asm
InputName=zs8p2unl

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile (Proton)"

# Begin Custom Build
OutDir=.\Profile Proton
InputPath=.\zs8p2unl.asm
InputName=zs8p2unl

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Debug P6 (Proton)"

# Begin Custom Build
OutDir=.\Debug P6 Proton
InputPath=.\zs8p2unl.asm
InputName=zs8p2unl

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Profile P6 (Proton)"

# Begin Custom Build
OutDir=.\Profile P6 Proton
InputPath=.\zs8p2unl.asm
InputName=zs8p2unl

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "pentprim - Win32 Release P6 (Proton)"

# Begin Custom Build
OutDir=.\Release P6 Proton
InputPath=.\zs8p2unl.asm
InputName=zs8p2unl

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /c /Cp /coff /nologo  /DBASED_FLOAT=1 /DBASED_FIXED=0 /DPARTS=0ffffh\
              /DAUTOLOAD=0 /DUSE_MMX-1 /Fo"$(OutDir)\$(InputName).obj" $(InputName).asm

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

SOURCE=.\common.inc
# End Source File
# Begin Source File

SOURCE=.\fpsetup.inc
# End Source File
# Begin Source File

SOURCE=.\fpu.inc
# End Source File
# Begin Source File

SOURCE=.\Fpwork.inc
# End Source File
# Begin Source File

SOURCE=.\iammx.inc
# End Source File
# Begin Source File

SOURCE=.\locmacs.inc
# End Source File
# Begin Source File

SOURCE=.\rastmacs.inc
# End Source File
# Begin Source File

SOURCE=.\setupdat.inc
# End Source File
# Begin Source File

SOURCE=.\unpack.inc
# End Source File
# Begin Source File

SOURCE=.\xzrgb.inc
# End Source File
# Begin Source File

SOURCE=.\xzrgbs.inc
# End Source File
# Begin Source File

SOURCE=.\xzuv1.inc
# End Source File
# Begin Source File

SOURCE=.\xzuvb.inc
# End Source File
# Begin Source File

SOURCE=.\xzuvc.inc
# End Source File
# Begin Source File

SOURCE=.\xzuvcb.inc
# End Source File
# Begin Source File

SOURCE=.\xzuvrgb.inc
# End Source File
# End Group
# End Target
# End Project
