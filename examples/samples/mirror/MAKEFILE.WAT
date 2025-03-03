# Copyright (c) 1993 Argonaut Software Ltd. All rights reserved.
#
# $Id: makefile.wat 1.2 1996/12/13 16:08:39 OWAIN Exp STEVEW $
# $Locker: STEVEW $
#
# Makefile for Watcom C using WMAKE
#

TARGET=mirror.exe

BRENDER_DIR=..\..

## Fixed, Release, Register
#LIB_TYPE=wxrr
#LIB_TYPE_NB=wrr
#LIB_TYPE_EX=4gwrr
#BASED=FIXED

## Float, Release, Register
LIB_TYPE=wfrr
LIB_TYPE_NB=wrr
LIB_TYPE_EX=4gwrr
BASED=FLOAT

#-d2 -dDEBUG 

CC=wcc386
CFLAGS=-omaxnet -wx -zp4 -5r -fp5&
 -DBASED_$(BASED)=1&
 -I$(BRENDER_DIR)\inc&
 -I$(BRENDER_DIR)\dosio

OBJS=mirror.obj&
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


