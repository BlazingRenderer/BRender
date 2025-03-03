# Microsoft Developer Studio Generated NMAKE File, Format Version 4.20
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

!IF "$(CFG)" == ""
CFG=mirror - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to mirror - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "mirror - Win32 Release" && "$(CFG)" != "mirror - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "Mirror.mak" CFG="mirror - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "mirror - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "mirror - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 
################################################################################
# Begin Project
# PROP Target_Last_Scanned "mirror - Win32 Debug"
CPP=cl.exe
RSC=rc.exe
MTL=mktyplib.exe

!IF  "$(CFG)" == "mirror - Win32 Release"

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
OUTDIR=.\Release
INTDIR=.\Release

ALL : "$(OUTDIR)\Mirror.exe"

CLEAN : 
	-@erase "$(INTDIR)\Br_libs.obj"
	-@erase "$(INTDIR)\Brdemo.res"
	-@erase "$(INTDIR)\Demomenu.obj"
	-@erase "$(INTDIR)\Mirror.obj"
	-@erase "$(INTDIR)\Timing.obj"
	-@erase "$(INTDIR)\Win_bits.obj"
	-@erase "$(INTDIR)\Winkey.obj"
	-@erase "$(OUTDIR)\Mirror.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D BASED_FLOAT=1 /YX /c
CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D\
 BASED_FLOAT=1 /Fp"$(INTDIR)/Mirror.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
RSC_PROJ=/l 0x809 /fo"$(INTDIR)/Brdemo.res" /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Mirror.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:windows /incremental:no\
 /pdb:"$(OUTDIR)/Mirror.pdb" /machine:I386 /out:"$(OUTDIR)/Mirror.exe" 
LINK32_OBJS= \
	"$(INTDIR)\Br_libs.obj" \
	"$(INTDIR)\Brdemo.res" \
	"$(INTDIR)\Demomenu.obj" \
	"$(INTDIR)\Mirror.obj" \
	"$(INTDIR)\Timing.obj" \
	"$(INTDIR)\Win_bits.obj" \
	"$(INTDIR)\Winkey.obj"

"$(OUTDIR)\Mirror.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "mirror - Win32 Debug"

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
OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "$(OUTDIR)\Mirror.exe"

CLEAN : 
	-@erase "$(INTDIR)\Br_libs.obj"
	-@erase "$(INTDIR)\Brdemo.res"
	-@erase "$(INTDIR)\Demomenu.obj"
	-@erase "$(INTDIR)\Mirror.obj"
	-@erase "$(INTDIR)\Timing.obj"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(INTDIR)\Win_bits.obj"
	-@erase "$(INTDIR)\Winkey.obj"
	-@erase "$(OUTDIR)\Mirror.exe"
	-@erase "$(OUTDIR)\Mirror.ilk"
	-@erase "$(OUTDIR)\Mirror.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D BASED_FLOAT=1 /YX /c
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS"\
 /D BASED_FLOAT=1 /Fp"$(INTDIR)/Mirror.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/"\
 /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
RSC_PROJ=/l 0x809 /fo"$(INTDIR)/Brdemo.res" /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Mirror.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"libc"
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:windows /incremental:yes\
 /pdb:"$(OUTDIR)/Mirror.pdb" /debug /machine:I386 /nodefaultlib:"libc"\
 /out:"$(OUTDIR)/Mirror.exe" 
LINK32_OBJS= \
	"$(INTDIR)\Br_libs.obj" \
	"$(INTDIR)\Brdemo.res" \
	"$(INTDIR)\Demomenu.obj" \
	"$(INTDIR)\Mirror.obj" \
	"$(INTDIR)\Timing.obj" \
	"$(INTDIR)\Win_bits.obj" \
	"$(INTDIR)\Winkey.obj"

"$(OUTDIR)\Mirror.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

################################################################################
# Begin Target

# Name "mirror - Win32 Release"
# Name "mirror - Win32 Debug"

!IF  "$(CFG)" == "mirror - Win32 Release"

!ELSEIF  "$(CFG)" == "mirror - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\Mirror.c

!IF  "$(CFG)" == "mirror - Win32 Release"

DEP_CPP_MIRRO=\
	"..\..\Inc\actor.h"\
	"..\..\Inc\angles.h"\
	"..\..\Inc\assoca_p.h"\
	"..\..\Inc\assocarr.h"\
	"..\..\Inc\brdiag.h"\
	"..\..\Inc\brerror.h"\
	"..\..\Inc\brfile.h"\
	"..\..\Inc\brfont.h"\
	"..\..\Inc\brlimits.h"\
	"..\..\Inc\brmem.h"\
	"..\..\Inc\brstdlib.h"\
	"..\..\Inc\brv1db.h"\
	"..\..\Inc\camera.h"\
	"..\..\Inc\colour.h"\
	"..\..\Inc\compend.h"\
	"..\..\Inc\compiler.h"\
	"..\..\Inc\custsupt.h"\
	"..\..\Inc\dos_p.h"\
	"..\..\Inc\dos_x.h"\
	"..\..\Inc\eventq.h"\
	"..\..\Inc\fixed.h"\
	"..\..\Inc\fmt.h"\
	"..\..\Inc\fmt_p.h"\
	"..\..\Inc\fw_p.h"\
	"..\..\Inc\g386ifix.h"\
	"..\..\Inc\light.h"\
	"..\..\Inc\material.h"\
	"..\..\Inc\math_p.h"\
	"..\..\Inc\math_x.h"\
	"..\..\Inc\matrix.h"\
	"..\..\Inc\model.h"\
	"..\..\Inc\mscifix.h"\
	"..\..\Inc\order.h"\
	"..\..\Inc\pixelmap.h"\
	"..\..\Inc\pm_p.h"\
	"..\..\Inc\pool.h"\
	"..\..\Inc\pretok.h"\
	"..\..\Inc\proifix.h"\
	"..\..\Inc\quat.h"\
	"..\..\Inc\render.h"\
	"..\..\Inc\scalar.h"\
	"..\..\Inc\syscn_p.h"\
	"..\..\Inc\token.h"\
	"..\..\Inc\transfrm.h"\
	"..\..\Inc\v1db_p.h"\
	"..\..\Inc\vecfns.h"\
	"..\..\Inc\vector.h"\
	"..\..\Inc\wtcifix.h"\
	"..\misc\br_demo.h"\
	"..\misc\dos_bits.h"\
	"..\misc\timing.h"\
	"..\misc\win_bits.h"\
	{$(INCLUDE)}"\Brender.h"\
	{$(INCLUDE)}"\Dosio.h"\
	{$(INCLUDE)}"\Keyboard.h"\
	

"$(INTDIR)\Mirror.obj" : $(SOURCE) $(DEP_CPP_MIRRO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "mirror - Win32 Debug"

DEP_CPP_MIRRO=\
	"..\..\Inc\actor.h"\
	"..\..\Inc\angles.h"\
	"..\..\Inc\assoca_p.h"\
	"..\..\Inc\assocarr.h"\
	"..\..\Inc\brdiag.h"\
	"..\..\Inc\brerror.h"\
	"..\..\Inc\brfile.h"\
	"..\..\Inc\brfont.h"\
	"..\..\Inc\brlimits.h"\
	"..\..\Inc\brmem.h"\
	"..\..\Inc\brstdlib.h"\
	"..\..\Inc\brv1db.h"\
	"..\..\Inc\camera.h"\
	"..\..\Inc\colour.h"\
	"..\..\Inc\compend.h"\
	"..\..\Inc\compiler.h"\
	"..\..\Inc\custsupt.h"\
	"..\..\Inc\dos_p.h"\
	"..\..\Inc\dos_x.h"\
	"..\..\Inc\eventq.h"\
	"..\..\Inc\fixed.h"\
	"..\..\Inc\fmt.h"\
	"..\..\Inc\fmt_p.h"\
	"..\..\Inc\fw_p.h"\
	"..\..\Inc\g386ifix.h"\
	"..\..\Inc\light.h"\
	"..\..\Inc\material.h"\
	"..\..\Inc\math_p.h"\
	"..\..\Inc\math_x.h"\
	"..\..\Inc\matrix.h"\
	"..\..\Inc\model.h"\
	"..\..\Inc\mscifix.h"\
	"..\..\Inc\order.h"\
	"..\..\Inc\pixelmap.h"\
	"..\..\Inc\pm_p.h"\
	"..\..\Inc\pool.h"\
	"..\..\Inc\pretok.h"\
	"..\..\Inc\proifix.h"\
	"..\..\Inc\quat.h"\
	"..\..\Inc\render.h"\
	"..\..\Inc\scalar.h"\
	"..\..\Inc\syscn_p.h"\
	"..\..\Inc\token.h"\
	"..\..\Inc\transfrm.h"\
	"..\..\Inc\v1db_p.h"\
	"..\..\Inc\vecfns.h"\
	"..\..\Inc\vector.h"\
	"..\..\Inc\wtcifix.h"\
	"..\misc\br_demo.h"\
	"..\misc\dos_bits.h"\
	"..\misc\timing.h"\
	"..\misc\win_bits.h"\
	{$(INCLUDE)}"\Brender.h"\
	{$(INCLUDE)}"\Dosio.h"\
	{$(INCLUDE)}"\Keyboard.h"\
	

"$(INTDIR)\Mirror.obj" : $(SOURCE) $(DEP_CPP_MIRRO) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\Brender\V1_3\Dist\Demos\Misc\Br_libs.c

"$(INTDIR)\Br_libs.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=\Brender\V1_3\Dist\Demos\Misc\Demomenu.c

!IF  "$(CFG)" == "mirror - Win32 Release"

DEP_CPP_DEMOM=\
	"..\..\Inc\actor.h"\
	"..\..\Inc\angles.h"\
	"..\..\Inc\assoca_p.h"\
	"..\..\Inc\assocarr.h"\
	"..\..\Inc\brdiag.h"\
	"..\..\Inc\brerror.h"\
	"..\..\Inc\brfile.h"\
	"..\..\Inc\brfont.h"\
	"..\..\Inc\brlimits.h"\
	"..\..\Inc\brmem.h"\
	"..\..\Inc\brstdlib.h"\
	"..\..\Inc\brv1db.h"\
	"..\..\Inc\camera.h"\
	"..\..\Inc\colour.h"\
	"..\..\Inc\compend.h"\
	"..\..\Inc\compiler.h"\
	"..\..\Inc\custsupt.h"\
	"..\..\Inc\dos_p.h"\
	"..\..\Inc\dos_x.h"\
	"..\..\Inc\eventq.h"\
	"..\..\Inc\fixed.h"\
	"..\..\Inc\fmt.h"\
	"..\..\Inc\fmt_p.h"\
	"..\..\Inc\fw_p.h"\
	"..\..\Inc\g386ifix.h"\
	"..\..\Inc\light.h"\
	"..\..\Inc\material.h"\
	"..\..\Inc\math_p.h"\
	"..\..\Inc\math_x.h"\
	"..\..\Inc\matrix.h"\
	"..\..\Inc\model.h"\
	"..\..\Inc\mscifix.h"\
	"..\..\Inc\order.h"\
	"..\..\Inc\pixelmap.h"\
	"..\..\Inc\pm_p.h"\
	"..\..\Inc\pool.h"\
	"..\..\Inc\pretok.h"\
	"..\..\Inc\proifix.h"\
	"..\..\Inc\quat.h"\
	"..\..\Inc\render.h"\
	"..\..\Inc\scalar.h"\
	"..\..\Inc\syscn_p.h"\
	"..\..\Inc\token.h"\
	"..\..\Inc\transfrm.h"\
	"..\..\Inc\v1db_p.h"\
	"..\..\Inc\vecfns.h"\
	"..\..\Inc\vector.h"\
	"..\..\Inc\wtcifix.h"\
	"..\misc\br_demo.h"\
	"..\misc\dos_bits.h"\
	"..\misc\timing.h"\
	"..\misc\win_bits.h"\
	{$(INCLUDE)}"\Brender.h"\
	{$(INCLUDE)}"\Dosio.h"\
	{$(INCLUDE)}"\Keyboard.h"\
	

"$(INTDIR)\Demomenu.obj" : $(SOURCE) $(DEP_CPP_DEMOM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "mirror - Win32 Debug"

DEP_CPP_DEMOM=\
	"..\..\Inc\actor.h"\
	"..\..\Inc\angles.h"\
	"..\..\Inc\assoca_p.h"\
	"..\..\Inc\assocarr.h"\
	"..\..\Inc\brdiag.h"\
	"..\..\Inc\brerror.h"\
	"..\..\Inc\brfile.h"\
	"..\..\Inc\brfont.h"\
	"..\..\Inc\brlimits.h"\
	"..\..\Inc\brmem.h"\
	"..\..\Inc\brstdlib.h"\
	"..\..\Inc\brv1db.h"\
	"..\..\Inc\camera.h"\
	"..\..\Inc\colour.h"\
	"..\..\Inc\compend.h"\
	"..\..\Inc\compiler.h"\
	"..\..\Inc\custsupt.h"\
	"..\..\Inc\dos_p.h"\
	"..\..\Inc\dos_x.h"\
	"..\..\Inc\eventq.h"\
	"..\..\Inc\fixed.h"\
	"..\..\Inc\fmt.h"\
	"..\..\Inc\fmt_p.h"\
	"..\..\Inc\fw_p.h"\
	"..\..\Inc\g386ifix.h"\
	"..\..\Inc\light.h"\
	"..\..\Inc\material.h"\
	"..\..\Inc\math_p.h"\
	"..\..\Inc\math_x.h"\
	"..\..\Inc\matrix.h"\
	"..\..\Inc\model.h"\
	"..\..\Inc\mscifix.h"\
	"..\..\Inc\order.h"\
	"..\..\Inc\pixelmap.h"\
	"..\..\Inc\pm_p.h"\
	"..\..\Inc\pool.h"\
	"..\..\Inc\pretok.h"\
	"..\..\Inc\proifix.h"\
	"..\..\Inc\quat.h"\
	"..\..\Inc\render.h"\
	"..\..\Inc\scalar.h"\
	"..\..\Inc\syscn_p.h"\
	"..\..\Inc\token.h"\
	"..\..\Inc\transfrm.h"\
	"..\..\Inc\v1db_p.h"\
	"..\..\Inc\vecfns.h"\
	"..\..\Inc\vector.h"\
	"..\..\Inc\wtcifix.h"\
	"..\misc\br_demo.h"\
	"..\misc\dos_bits.h"\
	"..\misc\timing.h"\
	"..\misc\win_bits.h"\
	{$(INCLUDE)}"\Brender.h"\
	{$(INCLUDE)}"\Dosio.h"\
	{$(INCLUDE)}"\Keyboard.h"\
	

"$(INTDIR)\Demomenu.obj" : $(SOURCE) $(DEP_CPP_DEMOM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\Brender\V1_3\Dist\Demos\Misc\Timing.c

!IF  "$(CFG)" == "mirror - Win32 Release"

DEP_CPP_TIMIN=\
	"..\..\Inc\actor.h"\
	"..\..\Inc\angles.h"\
	"..\..\Inc\assoca_p.h"\
	"..\..\Inc\assocarr.h"\
	"..\..\Inc\brdiag.h"\
	"..\..\Inc\brerror.h"\
	"..\..\Inc\brfile.h"\
	"..\..\Inc\brfont.h"\
	"..\..\Inc\brlimits.h"\
	"..\..\Inc\brmem.h"\
	"..\..\Inc\brstdlib.h"\
	"..\..\Inc\brv1db.h"\
	"..\..\Inc\camera.h"\
	"..\..\Inc\colour.h"\
	"..\..\Inc\compend.h"\
	"..\..\Inc\compiler.h"\
	"..\..\Inc\custsupt.h"\
	"..\..\Inc\dos_p.h"\
	"..\..\Inc\dos_x.h"\
	"..\..\Inc\eventq.h"\
	"..\..\Inc\fixed.h"\
	"..\..\Inc\fmt.h"\
	"..\..\Inc\fmt_p.h"\
	"..\..\Inc\fw_p.h"\
	"..\..\Inc\g386ifix.h"\
	"..\..\Inc\light.h"\
	"..\..\Inc\material.h"\
	"..\..\Inc\math_p.h"\
	"..\..\Inc\math_x.h"\
	"..\..\Inc\matrix.h"\
	"..\..\Inc\model.h"\
	"..\..\Inc\mscifix.h"\
	"..\..\Inc\order.h"\
	"..\..\Inc\pixelmap.h"\
	"..\..\Inc\pm_p.h"\
	"..\..\Inc\pool.h"\
	"..\..\Inc\pretok.h"\
	"..\..\Inc\proifix.h"\
	"..\..\Inc\quat.h"\
	"..\..\Inc\render.h"\
	"..\..\Inc\scalar.h"\
	"..\..\Inc\syscn_p.h"\
	"..\..\Inc\token.h"\
	"..\..\Inc\transfrm.h"\
	"..\..\Inc\v1db_p.h"\
	"..\..\Inc\vecfns.h"\
	"..\..\Inc\vector.h"\
	"..\..\Inc\wtcifix.h"\
	"..\misc\br_demo.h"\
	"..\misc\dos_bits.h"\
	"..\misc\timing.h"\
	"..\misc\win_bits.h"\
	{$(INCLUDE)}"\Brender.h"\
	{$(INCLUDE)}"\Dosio.h"\
	{$(INCLUDE)}"\Keyboard.h"\
	

"$(INTDIR)\Timing.obj" : $(SOURCE) $(DEP_CPP_TIMIN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "mirror - Win32 Debug"

DEP_CPP_TIMIN=\
	"..\..\Inc\actor.h"\
	"..\..\Inc\angles.h"\
	"..\..\Inc\assoca_p.h"\
	"..\..\Inc\assocarr.h"\
	"..\..\Inc\brdiag.h"\
	"..\..\Inc\brerror.h"\
	"..\..\Inc\brfile.h"\
	"..\..\Inc\brfont.h"\
	"..\..\Inc\brlimits.h"\
	"..\..\Inc\brmem.h"\
	"..\..\Inc\brstdlib.h"\
	"..\..\Inc\brv1db.h"\
	"..\..\Inc\camera.h"\
	"..\..\Inc\colour.h"\
	"..\..\Inc\compend.h"\
	"..\..\Inc\compiler.h"\
	"..\..\Inc\custsupt.h"\
	"..\..\Inc\dos_p.h"\
	"..\..\Inc\dos_x.h"\
	"..\..\Inc\eventq.h"\
	"..\..\Inc\fixed.h"\
	"..\..\Inc\fmt.h"\
	"..\..\Inc\fmt_p.h"\
	"..\..\Inc\fw_p.h"\
	"..\..\Inc\g386ifix.h"\
	"..\..\Inc\light.h"\
	"..\..\Inc\material.h"\
	"..\..\Inc\math_p.h"\
	"..\..\Inc\math_x.h"\
	"..\..\Inc\matrix.h"\
	"..\..\Inc\model.h"\
	"..\..\Inc\mscifix.h"\
	"..\..\Inc\order.h"\
	"..\..\Inc\pixelmap.h"\
	"..\..\Inc\pm_p.h"\
	"..\..\Inc\pool.h"\
	"..\..\Inc\pretok.h"\
	"..\..\Inc\proifix.h"\
	"..\..\Inc\quat.h"\
	"..\..\Inc\render.h"\
	"..\..\Inc\scalar.h"\
	"..\..\Inc\syscn_p.h"\
	"..\..\Inc\token.h"\
	"..\..\Inc\transfrm.h"\
	"..\..\Inc\v1db_p.h"\
	"..\..\Inc\vecfns.h"\
	"..\..\Inc\vector.h"\
	"..\..\Inc\wtcifix.h"\
	"..\misc\br_demo.h"\
	"..\misc\dos_bits.h"\
	"..\misc\timing.h"\
	"..\misc\win_bits.h"\
	{$(INCLUDE)}"\Brender.h"\
	{$(INCLUDE)}"\Dosio.h"\
	{$(INCLUDE)}"\Keyboard.h"\
	

"$(INTDIR)\Timing.obj" : $(SOURCE) $(DEP_CPP_TIMIN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\Brender\V1_3\Dist\Demos\Misc\Win_bits.c

!IF  "$(CFG)" == "mirror - Win32 Release"

DEP_CPP_WIN_B=\
	"..\..\Inc\actor.h"\
	"..\..\Inc\angles.h"\
	"..\..\Inc\assoca_p.h"\
	"..\..\Inc\assocarr.h"\
	"..\..\Inc\brdiag.h"\
	"..\..\Inc\brerror.h"\
	"..\..\Inc\brfile.h"\
	"..\..\Inc\brfont.h"\
	"..\..\Inc\brlimits.h"\
	"..\..\Inc\brmem.h"\
	"..\..\Inc\brstdlib.h"\
	"..\..\Inc\brv1db.h"\
	"..\..\Inc\camera.h"\
	"..\..\Inc\colour.h"\
	"..\..\Inc\compend.h"\
	"..\..\Inc\compiler.h"\
	"..\..\Inc\custsupt.h"\
	"..\..\Inc\dos_p.h"\
	"..\..\Inc\dos_x.h"\
	"..\..\Inc\eventq.h"\
	"..\..\Inc\fixed.h"\
	"..\..\Inc\fmt.h"\
	"..\..\Inc\fmt_p.h"\
	"..\..\Inc\fw_p.h"\
	"..\..\Inc\g386ifix.h"\
	"..\..\Inc\light.h"\
	"..\..\Inc\material.h"\
	"..\..\Inc\math_p.h"\
	"..\..\Inc\math_x.h"\
	"..\..\Inc\matrix.h"\
	"..\..\Inc\model.h"\
	"..\..\Inc\mscifix.h"\
	"..\..\Inc\order.h"\
	"..\..\Inc\pixelmap.h"\
	"..\..\Inc\pm_p.h"\
	"..\..\Inc\pool.h"\
	"..\..\Inc\pretok.h"\
	"..\..\Inc\proifix.h"\
	"..\..\Inc\quat.h"\
	"..\..\Inc\render.h"\
	"..\..\Inc\scalar.h"\
	"..\..\Inc\syscn_p.h"\
	"..\..\Inc\token.h"\
	"..\..\Inc\transfrm.h"\
	"..\..\Inc\v1db_p.h"\
	"..\..\Inc\vecfns.h"\
	"..\..\Inc\vector.h"\
	"..\..\Inc\wtcifix.h"\
	"..\misc\br_demo.h"\
	"..\misc\dos_bits.h"\
	"..\misc\timing.h"\
	"..\misc\win_bits.h"\
	{$(INCLUDE)}"\Brender.h"\
	{$(INCLUDE)}"\Dosio.h"\
	{$(INCLUDE)}"\Keyboard.h"\
	

"$(INTDIR)\Win_bits.obj" : $(SOURCE) $(DEP_CPP_WIN_B) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "mirror - Win32 Debug"

DEP_CPP_WIN_B=\
	"..\..\Inc\actor.h"\
	"..\..\Inc\angles.h"\
	"..\..\Inc\assoca_p.h"\
	"..\..\Inc\assocarr.h"\
	"..\..\Inc\brdiag.h"\
	"..\..\Inc\brerror.h"\
	"..\..\Inc\brfile.h"\
	"..\..\Inc\brfont.h"\
	"..\..\Inc\brlimits.h"\
	"..\..\Inc\brmem.h"\
	"..\..\Inc\brstdlib.h"\
	"..\..\Inc\brv1db.h"\
	"..\..\Inc\camera.h"\
	"..\..\Inc\colour.h"\
	"..\..\Inc\compend.h"\
	"..\..\Inc\compiler.h"\
	"..\..\Inc\custsupt.h"\
	"..\..\Inc\dos_p.h"\
	"..\..\Inc\dos_x.h"\
	"..\..\Inc\eventq.h"\
	"..\..\Inc\fixed.h"\
	"..\..\Inc\fmt.h"\
	"..\..\Inc\fmt_p.h"\
	"..\..\Inc\fw_p.h"\
	"..\..\Inc\g386ifix.h"\
	"..\..\Inc\light.h"\
	"..\..\Inc\material.h"\
	"..\..\Inc\math_p.h"\
	"..\..\Inc\math_x.h"\
	"..\..\Inc\matrix.h"\
	"..\..\Inc\model.h"\
	"..\..\Inc\mscifix.h"\
	"..\..\Inc\order.h"\
	"..\..\Inc\pixelmap.h"\
	"..\..\Inc\pm_p.h"\
	"..\..\Inc\pool.h"\
	"..\..\Inc\pretok.h"\
	"..\..\Inc\proifix.h"\
	"..\..\Inc\quat.h"\
	"..\..\Inc\render.h"\
	"..\..\Inc\scalar.h"\
	"..\..\Inc\syscn_p.h"\
	"..\..\Inc\token.h"\
	"..\..\Inc\transfrm.h"\
	"..\..\Inc\v1db_p.h"\
	"..\..\Inc\vecfns.h"\
	"..\..\Inc\vector.h"\
	"..\..\Inc\wtcifix.h"\
	"..\misc\br_demo.h"\
	"..\misc\dos_bits.h"\
	"..\misc\timing.h"\
	"..\misc\win_bits.h"\
	{$(INCLUDE)}"\Brender.h"\
	{$(INCLUDE)}"\Dosio.h"\
	{$(INCLUDE)}"\Keyboard.h"\
	

"$(INTDIR)\Win_bits.obj" : $(SOURCE) $(DEP_CPP_WIN_B) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\Brender\V1_3\Dist\Demos\Misc\Winkey.c

!IF  "$(CFG)" == "mirror - Win32 Release"

DEP_CPP_WINKE=\
	"..\..\Inc\actor.h"\
	"..\..\Inc\angles.h"\
	"..\..\Inc\assoca_p.h"\
	"..\..\Inc\assocarr.h"\
	"..\..\Inc\brdiag.h"\
	"..\..\Inc\brerror.h"\
	"..\..\Inc\brfile.h"\
	"..\..\Inc\brfont.h"\
	"..\..\Inc\brlimits.h"\
	"..\..\Inc\brmem.h"\
	"..\..\Inc\brstdlib.h"\
	"..\..\Inc\brv1db.h"\
	"..\..\Inc\camera.h"\
	"..\..\Inc\colour.h"\
	"..\..\Inc\compend.h"\
	"..\..\Inc\compiler.h"\
	"..\..\Inc\custsupt.h"\
	"..\..\Inc\dos_p.h"\
	"..\..\Inc\dos_x.h"\
	"..\..\Inc\eventq.h"\
	"..\..\Inc\fixed.h"\
	"..\..\Inc\fmt.h"\
	"..\..\Inc\fmt_p.h"\
	"..\..\Inc\fw_p.h"\
	"..\..\Inc\g386ifix.h"\
	"..\..\Inc\light.h"\
	"..\..\Inc\material.h"\
	"..\..\Inc\math_p.h"\
	"..\..\Inc\math_x.h"\
	"..\..\Inc\matrix.h"\
	"..\..\Inc\model.h"\
	"..\..\Inc\mscifix.h"\
	"..\..\Inc\order.h"\
	"..\..\Inc\pixelmap.h"\
	"..\..\Inc\pm_p.h"\
	"..\..\Inc\pool.h"\
	"..\..\Inc\pretok.h"\
	"..\..\Inc\proifix.h"\
	"..\..\Inc\quat.h"\
	"..\..\Inc\render.h"\
	"..\..\Inc\scalar.h"\
	"..\..\Inc\syscn_p.h"\
	"..\..\Inc\token.h"\
	"..\..\Inc\transfrm.h"\
	"..\..\Inc\v1db_p.h"\
	"..\..\Inc\vecfns.h"\
	"..\..\Inc\vector.h"\
	"..\..\Inc\wtcifix.h"\
	"..\misc\br_demo.h"\
	"..\misc\dos_bits.h"\
	"..\misc\timing.h"\
	"..\misc\win_bits.h"\
	{$(INCLUDE)}"\Brender.h"\
	{$(INCLUDE)}"\Dosio.h"\
	{$(INCLUDE)}"\Keyboard.h"\
	

"$(INTDIR)\Winkey.obj" : $(SOURCE) $(DEP_CPP_WINKE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "mirror - Win32 Debug"

DEP_CPP_WINKE=\
	"..\..\Inc\actor.h"\
	"..\..\Inc\angles.h"\
	"..\..\Inc\assoca_p.h"\
	"..\..\Inc\assocarr.h"\
	"..\..\Inc\brdiag.h"\
	"..\..\Inc\brerror.h"\
	"..\..\Inc\brfile.h"\
	"..\..\Inc\brfont.h"\
	"..\..\Inc\brlimits.h"\
	"..\..\Inc\brmem.h"\
	"..\..\Inc\brstdlib.h"\
	"..\..\Inc\brv1db.h"\
	"..\..\Inc\camera.h"\
	"..\..\Inc\colour.h"\
	"..\..\Inc\compend.h"\
	"..\..\Inc\compiler.h"\
	"..\..\Inc\custsupt.h"\
	"..\..\Inc\dos_p.h"\
	"..\..\Inc\dos_x.h"\
	"..\..\Inc\eventq.h"\
	"..\..\Inc\fixed.h"\
	"..\..\Inc\fmt.h"\
	"..\..\Inc\fmt_p.h"\
	"..\..\Inc\fw_p.h"\
	"..\..\Inc\g386ifix.h"\
	"..\..\Inc\light.h"\
	"..\..\Inc\material.h"\
	"..\..\Inc\math_p.h"\
	"..\..\Inc\math_x.h"\
	"..\..\Inc\matrix.h"\
	"..\..\Inc\model.h"\
	"..\..\Inc\mscifix.h"\
	"..\..\Inc\order.h"\
	"..\..\Inc\pixelmap.h"\
	"..\..\Inc\pm_p.h"\
	"..\..\Inc\pool.h"\
	"..\..\Inc\pretok.h"\
	"..\..\Inc\proifix.h"\
	"..\..\Inc\quat.h"\
	"..\..\Inc\render.h"\
	"..\..\Inc\scalar.h"\
	"..\..\Inc\syscn_p.h"\
	"..\..\Inc\token.h"\
	"..\..\Inc\transfrm.h"\
	"..\..\Inc\v1db_p.h"\
	"..\..\Inc\vecfns.h"\
	"..\..\Inc\vector.h"\
	"..\..\Inc\wtcifix.h"\
	"..\misc\br_demo.h"\
	"..\misc\dos_bits.h"\
	"..\misc\timing.h"\
	"..\misc\win_bits.h"\
	{$(INCLUDE)}"\Brender.h"\
	{$(INCLUDE)}"\Dosio.h"\
	{$(INCLUDE)}"\Keyboard.h"\
	

"$(INTDIR)\Winkey.obj" : $(SOURCE) $(DEP_CPP_WINKE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\Brender\V1_3\Dist\Demos\Misc\Brdemo.rc

!IF  "$(CFG)" == "mirror - Win32 Release"


"$(INTDIR)\Brdemo.res" : $(SOURCE) "$(INTDIR)"
   $(RSC) /l 0x809 /fo"$(INTDIR)/Brdemo.res" /i "\Brender\V1_3\Dist\Demos\Misc"\
 /d "NDEBUG" $(SOURCE)


!ELSEIF  "$(CFG)" == "mirror - Win32 Debug"


"$(INTDIR)\Brdemo.res" : $(SOURCE) "$(INTDIR)"
   $(RSC) /l 0x809 /fo"$(INTDIR)/Brdemo.res" /i "\Brender\V1_3\Dist\Demos\Misc"\
 /d "_DEBUG" $(SOURCE)


!ENDIF 

# End Source File
# End Target
# End Project
################################################################################
