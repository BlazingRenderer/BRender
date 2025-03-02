# Copyright (c) 1993 Argonaut Software Ltd. All rights reserved.
#
# $Id: makefile.wat 1.3 1996/12/12 15:17:59 STEVEW Exp $
# $Locker: $
#
# Makefile for Watcom C using WMAKE
#

TARGET=forest.exe

BRENDER_DIR=c:\brender

LINKOPTS=debug all

## Float, Release, Register
LIB_TYPE=wfrr
LIB_TYPE_NB=wrr
LIB_TYPE_EX=4gwrr
BASED=FLOAT



# Fixed, Debug, Register
#LIB_TYPE=wxdr
#LIB_TYPE_NB=wdr
#LIB_TYPE_EX=4gwdr
#BASED=FIXED


#-omaxnet 

CC=wcc386
CFLAGS=-d2 -dDEBUG -wx -zp4 -5r -fp5&
 -DBASED_$(BASED)=1&
 -I$(BRENDER_DIR)\inc&
 -I$(BRENDER_DIR)\dosio

OBJS=forest.obj&
     dos_bits.obj&
     timing.obj&


LIBS=&
	$(BRENDER_DIR)/lib/brfw$(LIB_TYPE_NB).lib&
	$(BRENDER_DIR)/lib/brfm$(LIB_TYPE).lib&
	$(BRENDER_DIR)/lib/brdb$(LIB_TYPE).lib&
	$(BRENDER_DIR)/lib/brmt$(LIB_TYPE).lib&
	$(BRENDER_DIR)/lib/brpm$(LIB_TYPE_NB).lib&
	$(BRENDER_DIR)/lib/dosio$(LIB_TYPE_NB).lib&
	$(BRENDER_DIR)/lib/brst$(LIB_TYPE_NB).lib&
	$(BRENDER_DIR)/lib/hst$(LIB_TYPE_EX).lib&


# Default rules
#
# Cope with long command lines
#
.c.obj:
	set _ARGS=$(CFLAGS) $*.c
	$(CC) @_ARGS


$(TARGET): $(OBJS) wlink.rsp
	wlink $(LINKOPTS) @wlink.rsp

# Link response file
#
wlink.rsp: makefile.wat
	echo name $(TARGET) >wlink.rsp
	echo system dos4g >>wlink.rsp
	echo option dosseg,caseexact,quiet,stack=64k >>wlink.rsp
	for %i in ($(OBJS)) do echo file %i >>wlink.rsp
	for %i in ($(LIBS)) do echo library %i >>wlink.rsp

dos_bits.obj :  ..\misc\dos_bits.c
        $(CC) $(CFLAGS) ..\misc\dos_bits.c


timing.obj :  ..\misc\timing.c
        $(CC) $(CFLAGS) ..\misc\timing.c


