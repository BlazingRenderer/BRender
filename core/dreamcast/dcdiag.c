/*
 * Copyright (c) 1993-1995 by Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: stddiag.c 1.1 1997/12/10 16:41:28 jon Exp $
 * $Locker: $
 *
 * Default diagnostic handler that reports through stderr
 */
#include "brender.h"

static void BR_CALLBACK BrDreamcastWarning(const char *message)
{
    fflush(stdout);
    fputs(message, stderr);
    fputc('\n', stderr);
    fflush(stderr);
}

static void BR_CALLBACK BrDreamcastFailure(const char *message)
{
    /*
     * Close down all devices etc.
     */
    BrEnd();

    fflush(stdout);
    fputs(message, stderr);
    fputc('\n', stderr);
    fflush(stderr);

    exit(10);
}

/*
 * ErrorHandler structure
 */
br_diaghandler BrDreamcastDiagHandler = {
    "Stdio DiagHandler",
    BrDreamcastWarning,
    BrDreamcastFailure,
};

/*
 * Override default
 */
br_diaghandler *BR_ASM_DATA _BrDefaultDiagHandler = &BrDreamcastDiagHandler;
