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

static void *BR_CALLBACK BrNullOpenRead(const char *name, br_size_t n_magics, br_mode_test_cbfn *identify, int *mode_result)
{
    BR_FATAL0("Invalid file system call");
    return NULL;
}

static void *BR_CALLBACK BrNullOpenWrite(const char *name, int mode)
{
    BR_FATAL0("Invalid file system call");
    return NULL;
}

static void BR_CALLBACK BrNullClose(void *f)
{
    BR_FATAL0("Invalid file system call");
}

static int BR_CALLBACK BrNullEof(void *f)
{
    BR_FATAL0("Invalid file system call");
    return -1;
}

static int BR_CALLBACK BrNullGetChar(void *f)
{
    BR_FATAL0("Invalid file system call");
    return -1;
}

static void BR_CALLBACK BrNullPutChar(int c, void *f)
{
    BR_FATAL0("Invalid file system call");
}

static br_size_t BR_CALLBACK BrNullRead(void *buf, br_size_t size, br_size_t n, void *f)
{
    BR_FATAL0("Invalid file system call");
    return 0;
}

static br_size_t BR_CALLBACK BrNullWrite(const void *buf, br_size_t size, br_size_t n, void *f)
{
    BR_FATAL0("Invalid file system call");
    return 0;
}

static br_size_t BR_CALLBACK BrNullGetLine(char *buf, br_size_t buf_len, void *f)
{
    BR_FATAL0("Invalid file system call");
    return 0;
}

static void BR_CALLBACK BrNullPutLine(const char *buf, void *f)
{
    BR_FATAL0("Invalid file system call");
}

static void BR_CALLBACK BrNullAdvance(br_size_t count, void *f)
{
    BR_FATAL0("Invalid file system call");
}

static void *BR_CALLBACK BrNullLoad(void *res, const char *name, br_size_t *size)
{
    BR_FATAL0("Invalid file system call");
    return NULL;
}

/*
 * Filesystem structure
 */
br_filesystem BrNullFilesystem = {
    .identifier = "Null Filesystem",
    .attributes = BrNullAttributes,
    .open_read  = BrNullOpenRead,
    .open_write = BrNullOpenWrite,
    .close      = BrNullClose,
    .eof        = BrNullEof,
    .getchr     = BrNullGetChar,
    .putchr     = BrNullPutChar,
    .read       = BrNullRead,
    .write      = BrNullWrite,
    .getline    = BrNullGetLine,
    .putline    = BrNullPutLine,
    .advance    = BrNullAdvance,
    .load       = BrNullLoad,
};

// Nope, we're using std now
///*
// * Global variable that can be overridden by linking something first
// */
// br_filesystem *_BrDefaultFilesystem = &BrNullFilesystem;
