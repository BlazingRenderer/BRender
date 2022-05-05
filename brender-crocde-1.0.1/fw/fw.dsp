# Microsoft Developer Studio Project File - Name="fw" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=fw - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "fw.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "fw.mak" CFG="fw - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "fw - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "fw - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "fw - Win32 Profile" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/brender/fw", EAAAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "fw - Win32 Release"

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
# ADD CPP /nologo /G5 /W3 /GX /O2 /D __BR_V1DB__=0 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D DEBUG=0 /D PARANOID=0 /D EVAL=0 /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /D "_USE_NON_INTEL_COMPILER" /YX /FD /c
# ADD BASE RSC /l 0xc09
# ADD RSC /l 0xc09
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "fw - Win32 Debug"

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
# ADD CPP /nologo /G5 /W3 /GX /Z7 /Od /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /D "PARANOID-1" /D __BR_V1DB__=0 /D DEBUG=1 /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D PARANOID=1 /D EVAL=0 /D STATIC= /D "_USE_NON_INTEL_COMPILER" /FR /YX /FD /c
# ADD BASE RSC /l 0xc09
# ADD RSC /l 0xc09
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "fw - Win32 Profile"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "fw___Win"
# PROP BASE Intermediate_Dir "fw___Win"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Profile"
# PROP Intermediate_Dir "Profile"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D __BR_V1DB__=0 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D DEBUG=0 /D PARANOID=0 /D EVAL=0 /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /YX /FD /c
# ADD CPP /nologo /G5 /W3 /GX /Zi /O2 /D __BR_V1DB__=0 /D DEBUG=0 /D PARANOID=0 /D EVAL=0 /D STATIC=static /D BASED_FIXED=0 /D BASED_FLOAT=1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USE_NON_INTEL_COMPILER" /FR /YX /FD /c
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

# Name "fw - Win32 Release"
# Name "fw - Win32 Debug"
# Name "fw - Win32 Profile"
# Begin Group "C Source"

# PROP Default_Filter ".c"
# Begin Source File

SOURCE=.\assocarr.c
# End Source File
# Begin Source File

SOURCE=.\brbegin.c
# End Source File
# Begin Source File

SOURCE=.\brbhook.c
# End Source File
# Begin Source File

SOURCE=.\brlists.c
# End Source File
# Begin Source File

SOURCE=.\brqsort.c
# End Source File
# Begin Source File

SOURCE=.\bswap.c
# End Source File
# Begin Source File

SOURCE=.\datafile.c
# End Source File
# Begin Source File

SOURCE=.\devlist.c
# End Source File
# Begin Source File

SOURCE=.\devquery.c
# End Source File
# Begin Source File

SOURCE=.\devsetup.c
# End Source File
# Begin Source File

SOURCE=.\diag.c
# End Source File
# Begin Source File

SOURCE=.\error.c
# End Source File
# Begin Source File

SOURCE=.\file.c
# End Source File
# Begin Source File

SOURCE=.\fwsetup.c
# End Source File
# Begin Source File

SOURCE=.\genfile.c
# End Source File
# Begin Source File

SOURCE=.\image.c
# End Source File
# Begin Source File

SOURCE=.\lexer.c
# End Source File
# Begin Source File

SOURCE=.\loader.c
# End Source File
# Begin Source File

SOURCE=.\mem.c
# End Source File
# Begin Source File

SOURCE=.\object.c
# End Source File
# Begin Source File

SOURCE=.\objectc.c
# End Source File
# Begin Source File

SOURCE=.\pattern.c
# End Source File
# Begin Source File

SOURCE=.\pool.c
# End Source File
# Begin Source File

SOURCE=.\printf.c
# End Source File
# Begin Source File

SOURCE=.\register.c
# End Source File
# Begin Source File

SOURCE=.\resource.c
# End Source File
# Begin Source File

SOURCE=.\resreg.c
# End Source File
# Begin Source File

SOURCE=.\scratch.c
# End Source File
# Begin Source File

SOURCE=.\scrstr.c
# End Source File
# Begin Source File

SOURCE=.\sys_conf.c
# End Source File
# Begin Source File

SOURCE=.\token.c
# End Source File
# Begin Source File

SOURCE=.\tokenval.c
# End Source File
# End Group
# Begin Group "Headers"

# PROP Default_Filter ".h"
# Begin Source File

SOURCE=.\assocarr.h
# End Source File
# Begin Source File

SOURCE=.\blockops.h
# End Source File
# Begin Source File

SOURCE=.\brassert.h
# End Source File
# Begin Source File

SOURCE=.\brddi.h
# End Source File
# Begin Source File

SOURCE=.\brddi_p.h
# End Source File
# Begin Source File

SOURCE=.\brexcept.h
# End Source File
# Begin Source File

SOURCE=.\brhton.h
# End Source File
# Begin Source File

SOURCE=.\brimage.h
# End Source File
# Begin Source File

SOURCE=.\brlists.h
# End Source File
# Begin Source File

SOURCE=.\brstdlib.h
# End Source File
# Begin Source File

SOURCE=.\dataf_p.h
# End Source File
# Begin Source File

SOURCE=.\datafile.h
# End Source File
# Begin Source File

SOURCE=.\formats.h
# End Source File
# Begin Source File

SOURCE=.\fw.h
# End Source File
# Begin Source File

SOURCE=.\fw_ip.h
# End Source File
# Begin Source File

SOURCE=.\lexer.h
# End Source File
# Begin Source File

SOURCE=.\pretok.h
# End Source File
# Begin Source File

SOURCE=.\register.h
# End Source File
# Begin Source File

SOURCE=.\shortcut.h
# End Source File
# Begin Source File

SOURCE=.\timing.h
# End Source File
# Begin Source File

SOURCE=.\tokenmap.h
# End Source File
# Begin Source File

SOURCE=.\wincoff.h
# End Source File
# End Group
# Begin Group "Perl Scripts"

# PROP Default_Filter ".pl"
# Begin Source File

SOURCE=.\classgen.pl
# End Source File
# Begin Source File

SOURCE=..\genasmpr.pl
# End Source File
# Begin Source File

SOURCE=..\resgen.pl
# End Source File
# Begin Source File

SOURCE=.\tokgen.pl
# End Source File
# End Group
# Begin Group "Miscellaneous"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\dev_objs.hpp

!IF  "$(CFG)" == "fw - Win32 Release"

USERDEP__DEV_O="classgen.pl"	
# Begin Custom Build
InputPath=.\dev_objs.hpp

"dev_objs.cgh" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl -E -D__CLASSGEN__ dev_objs.hpp >file 
	perl classgen.pl <file >dev_objs.cgh 
	del file 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fw - Win32 Debug"

USERDEP__DEV_O="classgen.pl"	
# Begin Custom Build
InputPath=.\dev_objs.hpp

"dev_objs.cgh" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl -E -D__CLASSGEN__ dev_objs.hpp >file 
	perl classgen.pl <file >dev_objs.cgh 
	del file 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "fw - Win32 Profile"

USERDEP__DEV_O="classgen.pl"	
# Begin Custom Build
InputPath=.\dev_objs.hpp

"dev_objs.cgh" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl -E -D__CLASSGEN__ dev_objs.hpp >file 
	perl classgen.pl <file >dev_objs.cgh 
	del file 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\fw_ix.afn

!IF  "$(CFG)" == "fw - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "fw - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "fw - Win32 Profile"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\fwimg.fns

!IF  "$(CFG)" == "fw - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "fw - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "fw - Win32 Profile"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\pretok.tok

!IF  "$(CFG)" == "fw - Win32 Release"

USERDEP__PRETO="tokgen.pl"	
# Begin Custom Build
InputPath=.\pretok.tok
InputName=pretok

BuildCmds= \
	perl tokgen.pl <$(InputName).tok

"pretok.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"pretok_c.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "fw - Win32 Debug"

USERDEP__PRETO="tokgen.pl"	
# Begin Custom Build
InputPath=.\pretok.tok
InputName=pretok

BuildCmds= \
	perl tokgen.pl <$(InputName).tok

"pretok.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"pretok_c.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "fw - Win32 Profile"

USERDEP__PRETO="tokgen.pl"	
# Begin Custom Build
InputPath=.\pretok.tok
InputName=pretok

BuildCmds= \
	perl tokgen.pl <$(InputName).tok

"pretok.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"pretok_c.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
