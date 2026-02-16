/*
 * Copyright (c) 1993-1995 by Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: stdfile.c 1.1 1997/12/10 16:41:28 jon Exp $
 * $Locker: $
 *
 * Default file handler that uses <stdio.h>
 */
#include <stddef.h>
#include <limits.h>

#include "brender.h"
#include "brassert.h"

/*
 * Access functions for stdio
 */
static br_uint_32 BR_CALLBACK BrFreestandingAttributes(void)
{
    return BR_FS_ATTR_READABLE | BR_FS_ATTR_HAS_TEXT | BR_FS_ATTR_HAS_BINARY | BR_FS_ATTR_HAS_ADVANCE;
}

/*
 * Open a file for reading
 *
 * Use BRENDER_PATH to locate the file if necessary
 *
 * Having found the file, invoke a supplied callback, (if present)
 * to find out if the file is text, binary, or unknown
 *
 * Text mode files are primarily used for debugging but it can be
 * useful to allow hand editting of input data
 *
 * Return a void * file handle ('FILE *' cast to 'void *') or NULL
 * if open failed
 */
static void *BR_CALLBACK BrFreestandingOpenRead(const char *name, br_size_t n_magics, br_mode_test_cbfn *identify, int *mode_result)
{
    return NULL;
}

/*
 * Open a file for writing, overwrites any existing file of the same name
 *
 * Return a void * file handle ('FILE *' cast to 'void *') or NULL
 * if open failed
 */
static void *BR_CALLBACK BrFreestandingOpenWrite(const char *name, int mode)
{
    return NULL;
}

/*
 * Close an open file
 */
static void BR_CALLBACK BrFreestandingClose(void *f)
{

}

/*
 * Test EOF
 */
static int BR_CALLBACK BrFreestandingEof(void *f)
{
    return -1;
}

/*
 * Read one character from file
 */
static int BR_CALLBACK BrFreestandingGetChar(void *f)
{
    return -1;
}

/*
 * Write one character to file
 */
static void BR_CALLBACK BrFreestandingPutChar(int c, void *f)
{

}

/*
 * Read a block from a file
 */
static br_size_t BR_CALLBACK BrFreestandingRead(void *buf, br_size_t size, br_size_t n, void *f)
{
    return 0;
}

/*
 * Write a block to a file
 */
static br_size_t BR_CALLBACK BrFreestandingWrite(const void *buf, br_size_t size, br_size_t n, void *f)
{
    return 0;
}

/*
 * Read a line of text from stdin and trim any terminators
 *
 * Return length of line
 */
static br_size_t BR_CALLBACK BrFreestandingGetLine(char *buf, br_size_t buf_len, void *f)
{
    return 0;
}

/*
 * Write a line to text file, followed by newline
 */
static void BR_CALLBACK BrFreestandingPutLine(const char *buf, void *f)
{

}

/*
 * Advance N bytes through a binary stream
 */
static void BR_CALLBACK BrFreestandingAdvance(br_size_t count, void *f)
{

}

static void *BR_CALLBACK BrFreestandingLoad(void *res, const char *name, br_size_t *size)
{
    return NULL;
}

/*
 * Filesystem structure
 */
br_filesystem BrFreestandingFilesystem = {
    .identifier = "Freestanding IO",

    .attributes = BrFreestandingAttributes,
    .open_read  = BrFreestandingOpenRead,
    .open_write = BrFreestandingOpenWrite,
    .close      = BrFreestandingClose,
    .eof        = BrFreestandingEof,

    .getchr = BrFreestandingGetChar,
    .putchr = BrFreestandingPutChar,

    .read  = BrFreestandingRead,
    .write = BrFreestandingWrite,

    .getline = BrFreestandingGetLine,
    .putline = BrFreestandingPutLine,

    .advance = BrFreestandingAdvance,

    .load = BrFreestandingLoad,
};

/*
 * Override global variable s.t. the default filesystem will be stdio
 */
br_filesystem *BR_ASM_DATA _BrDefaultFilesystem = &BrFreestandingFilesystem;
