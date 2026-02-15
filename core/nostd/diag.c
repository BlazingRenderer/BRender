/*
 * Copyright (c) 1993-1995 by Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: stddiag.c 1.1 1997/12/10 16:41:28 jon Exp $
 * $Locker: $
 *
 * Default diagnostic handler that reports through stderr
 */
#include "brender.h"

static void BR_CALLBACK BrFreestandingWarning(const char *message)
{
    // TODO
#if 0
    fflush(stdout);
    fputs(message, stderr);
    fputc('\n', stderr);
    fflush(stderr);
#endif
}

static void BR_CALLBACK BrFreestandingFailure(const char *message)
{
    /*
     * Close down all devices etc.
     */
    BrEnd();

    // TODO
#if 0
    fflush(stdout);
    fputs(message, stderr);
    fputc('\n', stderr);
    fflush(stderr);

    exit(10);
#endif
}

/*
 * ErrorHandler structure
 */
br_diaghandler BrFreestandingDiagHandler = {
    "Freestanding DiagHandler",
    BrFreestandingWarning,
    BrFreestandingFailure,
};

/*
 * Override default
 */
br_diaghandler *BR_ASM_DATA _BrDefaultDiagHandler = &BrFreestandingDiagHandler;
