/*
 * Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: deffile.c 1.1 1997/12/10 16:41:06 jon Exp $
 * $Locker: $
 *
 * Default file handler that does nothing
 */
#include "brender.h"

static br_uint_32 BrNullAttributes(void)
{
    return 0;
}

static void BrNullOther(void)
{
    BR_FATAL0("Invald file system call");
}

/*
 * Filesystem structure
 */
br_filesystem BrNullFilesystem = {
    "Null Filesystem", /* identifier */

    BrNullAttributes,    /* attributes */
    (void *)BrNullOther, /* open_read  */
    (void *)BrNullOther, /* openwrite  */
    (void *)BrNullOther, /* close      */
    (void *)BrNullOther, /* eof		  */

    (void *)BrNullOther, /* getchar	  */
    (void *)BrNullOther, /* putchar	  */

    (void *)BrNullOther, /* read		  */
    (void *)BrNullOther, /* write	  */

    (void *)BrNullOther, /* getline	  */
    (void *)BrNullOther, /* putline	  */

    (void *)BrNullOther, /* advance	  */
};

// Nope, we're using std now
///*
// * Global variable that can be overridden by linking something first
// */
// br_filesystem *_BrDefaultFilesystem = &BrNullFilesystem;
