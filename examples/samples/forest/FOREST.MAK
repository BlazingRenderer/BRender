# Microsoft Developer Studio Generated NMAKE File, Format Version 4.20
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

!IF "$(CFG)" == ""
CFG=Forest - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to Forest - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "Forest - Win32 Release" && "$(CFG)" != "Forest - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "Forest.mak" CFG="Forest - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Forest - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Forest - Win32 Debug" (based on "Win32 (x86) Application")
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
# PROP Target_Last_Scanned "Forest - Win32 Debug"
CPP=cl.exe
RSC=rc.exe
MTL=mktyplib.exe

!IF  "$(CFG)" == "Forest - Win32 Release"

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

ALL : "$(OUTDIR)\Forest.exe"

CLEAN : 
	-@erase "$(INTDIR)\Br_libs.obj"
	-@erase "$(INTDIR)\Brdemo.res"
	-@erase "$(INTDIR)\Demomenu.obj"
	-@erase "$(INTDIR)\Forest.obj"
	-@erase "$(INTDIR)\Timing.obj"
	-@erase "$(INTDIR)\Win_bits.obj"
	-@erase "$(INTDIR)\Winkey.obj"
	-@erase "$(OUTDIR)\Forest.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D BASED_FLOAT=1 /YX /c
CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D\
 BASED_FLOAT=1 /Fp"$(INTDIR)/Forest.pch" /YX /Fo"$(INTDIR)/" /c 
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
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Forest.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:windows /incremental:no\
 /pdb:"$(OUTDIR)/Forest.pdb" /machine:I386 /out:"$(OUTDIR)/Forest.exe" 
LINK32_OBJS= \
	"$(INTDIR)\Br_libs.obj" \
	"$(INTDIR)\Brdemo.res" \
	"$(INTDIR)\Demomenu.obj" \
	"$(INTDIR)\Forest.obj" \
	"$(INTDIR)\Timing.obj" \
	"$(INTDIR)\Win_bits.obj" \
	"$(INTDIR)\Winkey.obj"

"$(OUTDIR)\Forest.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Forest - Win32 Debug"

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

ALL : "$(OUTDIR)\Forest.exe"

CLEAN : 
	-@erase "$(INTDIR)\Br_libs.obj"
	-@erase "$(INTDIR)\Brdemo.res"
	-@erase "$(INTDIR)\Demomenu.obj"
	-@erase "$(INTDIR)\Forest.obj"
	-@erase "$(INTDIR)\Timing.obj"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(INTDIR)\Win_bits.obj"
	-@erase "$(INTDIR)\Winkey.obj"
	-@erase "$(OUTDIR)\Forest.exe"
	-@erase "$(OUTDIR)\Forest.ilk"
	-@erase "$(OUTDIR)\Forest.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D BASED_FLOAT=1 /YX /c
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS"\
 /D BASED_FLOAT=1 /Fp"$(INTDIR)/Forest.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/"\
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
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Forest.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"libc"
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:windows /incremental:yes\
 /pdb:"$(OUTDIR)/Forest.pdb" /debug /machine:I386 /nodefaultlib:"libc"\
 /out:"$(OUTDIR)/Forest.exe" 
LINK32_OBJS= \
	"$(INTDIR)\Br_libs.obj" \
	"$(INTDIR)\Brdemo.res" \
	"$(INTDIR)\Demomenu.obj" \
	"$(INTDIR)\Forest.obj" \
	"$(INTDIR)\Timing.obj" \
	"$(INTDIR)\Win_bits.obj" \
	"$(INTDIR)\Winkey.obj"

"$(OUTDIR)\Forest.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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

# Name "Forest - Win32 Release"
# Name "Forest - Win32 Debug"

!IF  "$(CFG)" == "Forest - Win32 Release"

!ELSEIF  "$(CFG)" == "Forest - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\Forest.c

!IF  "$(CFG)" == "Forest - Win32 Release"

DEP_CPP_FORES=\
	"..\misc\br_demo.h"\
	"..\misc\dos_bits.h"\
	"..\misc\timing.h"\
	"..\misc\win_bits.h"\
	".\Forest.h"\
	"C:\brender\inc\actor.h"\
	"C:\brender\inc\angles.h"\
	"C:\brender\inc\assoca_p.h"\
	"C:\brender\inc\assocarr.h"\
	"C:\brender\inc\brdiag.h"\
	"C:\brender\inc\brerror.h"\
	"C:\brender\inc\brfile.h"\
	"C:\brender\inc\brfont.h"\
	"C:\brender\inc\brlimits.h"\
	"C:\brender\inc\brmem.h"\
	"C:\brender\inc\brstdlib.h"\
	"C:\brender\inc\brv1db.h"\
	"C:\brender\inc\camera.h"\
	"C:\brender\inc\colour.h"\
	"C:\brender\inc\compend.h"\
	"C:\brender\inc\compiler.h"\
	"C:\brender\inc\custsupt.h"\
	"C:\brender\inc\dos_p.h"\
	"C:\brender\inc\dos_x.h"\
	"C:\brender\inc\eventq.h"\
	"C:\brender\inc\fixed.h"\
	"C:\brender\inc\fmt.h"\
	"C:\brender\inc\fmt_p.h"\
	"C:\brender\inc\fw_p.h"\
	"C:\brender\inc\g386ifix.h"\
	"C:\brender\inc\light.h"\
	"C:\brender\inc\material.h"\
	"C:\brender\inc\math_p.h"\
	"C:\brender\inc\math_x.h"\
	"C:\brender\inc\matrix.h"\
	"C:\brender\inc\model.h"\
	"C:\brender\inc\mscifix.h"\
	"C:\brender\inc\order.h"\
	"C:\brender\inc\pixelmap.h"\
	"C:\brender\inc\pm_p.h"\
	"C:\brender\inc\pool.h"\
	"C:\brender\inc\pretok.h"\
	"C:\brender\inc\proifix.h"\
	"C:\brender\inc\quat.h"\
	"C:\brender\inc\render.h"\
	"C:\brender\inc\scalar.h"\
	"C:\brender\inc\syscn_p.h"\
	"C:\brender\inc\token.h"\
	"C:\brender\inc\transfrm.h"\
	"C:\brender\inc\v1db_p.h"\
	"C:\brender\inc\vecfns.h"\
	"C:\brender\inc\vector.h"\
	"C:\brender\inc\wtcifix.h"\
	{$(INCLUDE)}"\Brender.h"\
	{$(INCLUDE)}"\Dosio.h"\
	{$(INCLUDE)}"\Keyboard.h"\
	

"$(INTDIR)\Forest.obj" : $(SOURCE) $(DEP_CPP_FORES) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Forest - Win32 Debug"

DEP_CPP_FORES=\
	"..\misc\br_demo.h"\
	".\Forest.h"\
	{$(INCLUDE)}"\Brender.h"\
	

"$(INTDIR)\Forest.obj" : $(SOURCE) $(DEP_CPP_FORES) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\Brender\V1_3\Dist\Demos\Misc\Winkey.c

!IF  "$(CFG)" == "Forest - Win32 Release"

DEP_CPP_WINKE=\
	"..\misc\br_demo.h"\
	"..\misc\dos_bits.h"\
	"..\misc\timing.h"\
	"..\misc\win_bits.h"\
	"C:\brender\inc\actor.h"\
	"C:\brender\inc\angles.h"\
	"C:\brender\inc\assoca_p.h"\
	"C:\brender\inc\assocarr.h"\
	"C:\brender\inc\brdiag.h"\
	"C:\brender\inc\brerror.h"\
	"C:\brender\inc\brfile.h"\
	"C:\brender\inc\brfont.h"\
	"C:\brender\inc\brlimits.h"\
	"C:\brender\inc\brmem.h"\
	"C:\brender\inc\brstdlib.h"\
	"C:\brender\inc\brv1db.h"\
	"C:\brender\inc\camera.h"\
	"C:\brender\inc\colour.h"\
	"C:\brender\inc\compend.h"\
	"C:\brender\inc\compiler.h"\
	"C:\brender\inc\custsupt.h"\
	"C:\brender\inc\dos_p.h"\
	"C:\brender\inc\dos_x.h"\
	"C:\brender\inc\eventq.h"\
	"C:\brender\inc\fixed.h"\
	"C:\brender\inc\fmt.h"\
	"C:\brender\inc\fmt_p.h"\
	"C:\brender\inc\fw_p.h"\
	"C:\brender\inc\g386ifix.h"\
	"C:\brender\inc\light.h"\
	"C:\brender\inc\material.h"\
	"C:\brender\inc\math_p.h"\
	"C:\brender\inc\math_x.h"\
	"C:\brender\inc\matrix.h"\
	"C:\brender\inc\model.h"\
	"C:\brender\inc\mscifix.h"\
	"C:\brender\inc\order.h"\
	"C:\brender\inc\pixelmap.h"\
	"C:\brender\inc\pm_p.h"\
	"C:\brender\inc\pool.h"\
	"C:\brender\inc\pretok.h"\
	"C:\brender\inc\proifix.h"\
	"C:\brender\inc\quat.h"\
	"C:\brender\inc\render.h"\
	"C:\brender\inc\scalar.h"\
	"C:\brender\inc\syscn_p.h"\
	"C:\brender\inc\token.h"\
	"C:\brender\inc\transfrm.h"\
	"C:\brender\inc\v1db_p.h"\
	"C:\brender\inc\vecfns.h"\
	"C:\brender\inc\vector.h"\
	"C:\brender\inc\wtcifix.h"\
	{$(INCLUDE)}"\Brender.h"\
	{$(INCLUDE)}"\Dosio.h"\
	{$(INCLUDE)}"\Keyboard.h"\
	

"$(INTDIR)\Winkey.obj" : $(SOURCE) $(DEP_CPP_WINKE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Forest - Win32 Debug"

DEP_CPP_WINKE=\
	"..\misc\br_demo.h"\
	{$(INCLUDE)}"\Brender.h"\
	{$(INCLUDE)}"\Keyboard.h"\
	

"$(INTDIR)\Winkey.obj" : $(SOURCE) $(DEP_CPP_WINKE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\Brender\V1_3\Dist\Demos\Misc\Demomenu.c

!IF  "$(CFG)" == "Forest - Win32 Release"

DEP_CPP_DEMOM=\
	"..\misc\br_demo.h"\
	"..\misc\dos_bits.h"\
	"..\misc\timing.h"\
	"..\misc\win_bits.h"\
	"C:\brender\inc\actor.h"\
	"C:\brender\inc\angles.h"\
	"C:\brender\inc\assoca_p.h"\
	"C:\brender\inc\assocarr.h"\
	"C:\brender\inc\brdiag.h"\
	"C:\brender\inc\brerror.h"\
	"C:\brender\inc\brfile.h"\
	"C:\brender\inc\brfont.h"\
	"C:\brender\inc\brlimits.h"\
	"C:\brender\inc\brmem.h"\
	"C:\brender\inc\brstdlib.h"\
	"C:\brender\inc\brv1db.h"\
	"C:\brender\inc\camera.h"\
	"C:\brender\inc\colour.h"\
	"C:\brender\inc\compend.h"\
	"C:\brender\inc\compiler.h"\
	"C:\brender\inc\custsupt.h"\
	"C:\brender\inc\dos_p.h"\
	"C:\brender\inc\dos_x.h"\
	"C:\brender\inc\eventq.h"\
	"C:\brender\inc\fixed.h"\
	"C:\brender\inc\fmt.h"\
	"C:\brender\inc\fmt_p.h"\
	"C:\brender\inc\fw_p.h"\
	"C:\brender\inc\g386ifix.h"\
	"C:\brender\inc\light.h"\
	"C:\brender\inc\material.h"\
	"C:\brender\inc\math_p.h"\
	"C:\brender\inc\math_x.h"\
	"C:\brender\inc\matrix.h"\
	"C:\brender\inc\model.h"\
	"C:\brender\inc\mscifix.h"\
	"C:\brender\inc\order.h"\
	"C:\brender\inc\pixelmap.h"\
	"C:\brender\inc\pm_p.h"\
	"C:\brender\inc\pool.h"\
	"C:\brender\inc\pretok.h"\
	"C:\brender\inc\proifix.h"\
	"C:\brender\inc\quat.h"\
	"C:\brender\inc\render.h"\
	"C:\brender\inc\scalar.h"\
	"C:\brender\inc\syscn_p.h"\
	"C:\brender\inc\token.h"\
	"C:\brender\inc\transfrm.h"\
	"C:\brender\inc\v1db_p.h"\
	"C:\brender\inc\vecfns.h"\
	"C:\brender\inc\vector.h"\
	"C:\brender\inc\wtcifix.h"\
	{$(INCLUDE)}"\Brender.h"\
	{$(INCLUDE)}"\Dosio.h"\
	{$(INCLUDE)}"\Keyboard.h"\
	

"$(INTDIR)\Demomenu.obj" : $(SOURCE) $(DEP_CPP_DEMOM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Forest - Win32 Debug"

DEP_CPP_DEMOM=\
	"..\misc\br_demo.h"\
	{$(INCLUDE)}"\Brender.h"\
	

"$(INTDIR)\Demomenu.obj" : $(SOURCE) $(DEP_CPP_DEMOM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\Brender\V1_3\Dist\Demos\Misc\Timing.c

!IF  "$(CFG)" == "Forest - Win32 Release"

DEP_CPP_TIMIN=\
	"..\misc\br_demo.h"\
	"..\misc\dos_bits.h"\
	"..\misc\timing.h"\
	"..\misc\win_bits.h"\
	"C:\brender\inc\actor.h"\
	"C:\brender\inc\angles.h"\
	"C:\brender\inc\assoca_p.h"\
	"C:\brender\inc\assocarr.h"\
	"C:\brender\inc\brdiag.h"\
	"C:\brender\inc\brerror.h"\
	"C:\brender\inc\brfile.h"\
	"C:\brender\inc\brfont.h"\
	"C:\brender\inc\brlimits.h"\
	"C:\brender\inc\brmem.h"\
	"C:\brender\inc\brstdlib.h"\
	"C:\brender\inc\brv1db.h"\
	"C:\brender\inc\camera.h"\
	"C:\brender\inc\colour.h"\
	"C:\brender\inc\compend.h"\
	"C:\brender\inc\compiler.h"\
	"C:\brender\inc\custsupt.h"\
	"C:\brender\inc\dos_p.h"\
	"C:\brender\inc\dos_x.h"\
	"C:\brender\inc\eventq.h"\
	"C:\brender\inc\fixed.h"\
	"C:\brender\inc\fmt.h"\
	"C:\brender\inc\fmt_p.h"\
	"C:\brender\inc\fw_p.h"\
	"C:\brender\inc\g386ifix.h"\
	"C:\brender\inc\light.h"\
	"C:\brender\inc\material.h"\
	"C:\brender\inc\math_p.h"\
	"C:\brender\inc\math_x.h"\
	"C:\brender\inc\matrix.h"\
	"C:\brender\inc\model.h"\
	"C:\brender\inc\mscifix.h"\
	"C:\brender\inc\order.h"\
	"C:\brender\inc\pixelmap.h"\
	"C:\brender\inc\pm_p.h"\
	"C:\brender\inc\pool.h"\
	"C:\brender\inc\pretok.h"\
	"C:\brender\inc\proifix.h"\
	"C:\brender\inc\quat.h"\
	"C:\brender\inc\render.h"\
	"C:\brender\inc\scalar.h"\
	"C:\brender\inc\syscn_p.h"\
	"C:\brender\inc\token.h"\
	"C:\brender\inc\transfrm.h"\
	"C:\brender\inc\v1db_p.h"\
	"C:\brender\inc\vecfns.h"\
	"C:\brender\inc\vector.h"\
	"C:\brender\inc\wtcifix.h"\
	{$(INCLUDE)}"\Brender.h"\
	{$(INCLUDE)}"\Dosio.h"\
	{$(INCLUDE)}"\Keyboard.h"\
	

"$(INTDIR)\Timing.obj" : $(SOURCE) $(DEP_CPP_TIMIN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Forest - Win32 Debug"

DEP_CPP_TIMIN=\
	"..\misc\br_demo.h"\
	{$(INCLUDE)}"\Brender.h"\
	

"$(INTDIR)\Timing.obj" : $(SOURCE) $(DEP_CPP_TIMIN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\Brender\V1_3\Dist\Demos\Misc\Win_bits.c

!IF  "$(CFG)" == "Forest - Win32 Release"

DEP_CPP_WIN_B=\
	"..\misc\br_demo.h"\
	"..\misc\dos_bits.h"\
	"..\misc\timing.h"\
	"..\misc\win_bits.h"\
	"C:\brender\inc\actor.h"\
	"C:\brender\inc\angles.h"\
	"C:\brender\inc\assoca_p.h"\
	"C:\brender\inc\assocarr.h"\
	"C:\brender\inc\brdiag.h"\
	"C:\brender\inc\brerror.h"\
	"C:\brender\inc\brfile.h"\
	"C:\brender\inc\brfont.h"\
	"C:\brender\inc\brlimits.h"\
	"C:\brender\inc\brmem.h"\
	"C:\brender\inc\brstdlib.h"\
	"C:\brender\inc\brv1db.h"\
	"C:\brender\inc\camera.h"\
	"C:\brender\inc\colour.h"\
	"C:\brender\inc\compend.h"\
	"C:\brender\inc\compiler.h"\
	"C:\brender\inc\custsupt.h"\
	"C:\brender\inc\dos_p.h"\
	"C:\brender\inc\dos_x.h"\
	"C:\brender\inc\eventq.h"\
	"C:\brender\inc\fixed.h"\
	"C:\brender\inc\fmt.h"\
	"C:\brender\inc\fmt_p.h"\
	"C:\brender\inc\fw_p.h"\
	"C:\brender\inc\g386ifix.h"\
	"C:\brender\inc\light.h"\
	"C:\brender\inc\material.h"\
	"C:\brender\inc\math_p.h"\
	"C:\brender\inc\math_x.h"\
	"C:\brender\inc\matrix.h"\
	"C:\brender\inc\model.h"\
	"C:\brender\inc\mscifix.h"\
	"C:\brender\inc\order.h"\
	"C:\brender\inc\pixelmap.h"\
	"C:\brender\inc\pm_p.h"\
	"C:\brender\inc\pool.h"\
	"C:\brender\inc\pretok.h"\
	"C:\brender\inc\proifix.h"\
	"C:\brender\inc\quat.h"\
	"C:\brender\inc\render.h"\
	"C:\brender\inc\scalar.h"\
	"C:\brender\inc\syscn_p.h"\
	"C:\brender\inc\token.h"\
	"C:\brender\inc\transfrm.h"\
	"C:\brender\inc\v1db_p.h"\
	"C:\brender\inc\vecfns.h"\
	"C:\brender\inc\vector.h"\
	"C:\brender\inc\wtcifix.h"\
	{$(INCLUDE)}"\Brender.h"\
	{$(INCLUDE)}"\Dosio.h"\
	{$(INCLUDE)}"\Keyboard.h"\
	

"$(INTDIR)\Win_bits.obj" : $(SOURCE) $(DEP_CPP_WIN_B) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Forest - Win32 Debug"

DEP_CPP_WIN_B=\
	"..\misc\br_demo.h"\
	{$(INCLUDE)}"\Brender.h"\
	

"$(INTDIR)\Win_bits.obj" : $(SOURCE) $(DEP_CPP_WIN_B) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


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

SOURCE=\Brender\V1_3\Dist\Demos\Misc\Brdemo.rc

!IF  "$(CFG)" == "Forest - Win32 Release"


"$(INTDIR)\Brdemo.res" : $(SOURCE) "$(INTDIR)"
   $(RSC) /l 0x809 /fo"$(INTDIR)/Brdemo.res" /i "\Brender\V1_3\Dist\Demos\Misc"\
 /d "NDEBUG" $(SOURCE)


!ELSEIF  "$(CFG)" == "Forest - Win32 Debug"


"$(INTDIR)\Brdemo.res" : $(SOURCE) "$(INTDIR)"
   $(RSC) /l 0x809 /fo"$(INTDIR)/Brdemo.res" /i "\Brender\V1_3\Dist\Demos\Misc"\
 /d "_DEBUG" $(SOURCE)


!ENDIF 

# End Source File
# End Target
# End Project
################################################################################
