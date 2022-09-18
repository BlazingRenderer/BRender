/****************************************************************************
 *                                                                          *
 * SOURCE FILE NAME: App.h                                                  *
 *                                                                          *
 * DESCRIPTIVE NAME: Application header information                         *
 *                                                                          *
 * COMPONENT: Robot                                                         *
 *                                                                          *
 * COPYRIGHT: (C) Copyright Argonaut Software. 1994.  All rights reserved.  *
 *                                                                          *
 ****************************************************************************/

typedef struct _POINTL {
    long x;
    long y;
} POINTL, *PPOINTL;

#define MOUSETRACK_LEFT            0x01
#define MOUSETRACK_RIGHT           0x02

extern int    TrackingFlags;
extern POINTL TrackingValues[4];

int main(int argc, char **argv);
