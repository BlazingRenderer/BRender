
/* brender headers */
#include "brender.h"
#include "fmt.h"
#include "brstb.h"

/*
 * realloc
 */

void *BR_RESIDENT_ENTRY FmtSTBRealloc(void *ptr, br_size_t size)
{
    return BrMemReallocate(ptr, size, BR_MEMORY_PIXELS);
}

/*
 * malloc
 */

void *BR_RESIDENT_ENTRY FmtSTBMalloc(br_size_t size)
{
    return BrMemAllocate(size, BR_MEMORY_PIXELS);
}

/*
 * free
 */

void BR_RESIDENT_ENTRY FmtSTBFree(void *ptr)
{
    if(ptr != NULL)
        BrMemFree(ptr);
}

/*
 * write data to file
 */

void BR_RESIDENT_ENTRY FmtSTBFileWrite(void *context, void *data, int size)
{
    BrFileWrite(data, size, 1, context);
}

/*
 * read data from file
 */

int BR_RESIDENT_ENTRY FmtSTBFileRead(void *user, char *data, int size)
{
    return BrFileRead(data, 1, size, user);
}

/*
 * advance in file
 */

void BR_RESIDENT_ENTRY FmtSTBFileAdvance(void *user, int n)
{
    BrFileAdvance(n, user);
}

/*
 * return EOF flag
 */

int BR_RESIDENT_ENTRY FmtSTBFileEof(void *user)
{
    return BrFileEof(user);
}

/*
 * io structure
 */

stbi_io_callbacks br_stb_file_cbfns = {
    .read = FmtSTBFileRead,
    .skip = FmtSTBFileAdvance,
    .eof  = FmtSTBFileEof,
};
