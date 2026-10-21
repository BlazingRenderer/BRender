/*
 * The in-memory filing system
 *
 * BrPixelmapSave()/BrPixelmapLoad() write and read through fw.fsys for every
 * character rather than only when they open, so a caller that wants the bytes
 * rather than a file installs this filesystem for the duration of the call and
 * collects the store afterwards. The store serves exactly one virtual file: it
 * names it, records the mode it was written in, and holds the bytes. See
 * core/fw/memfs.c.
 *
 * The install and restore pair exist only because br_filesystem carries no
 * context pointer, which leaves the implementation nowhere else to keep the
 * store (see issue #59). They are a DDI rather than a public interface for that
 * reason - a caller outside core/fw has no other way to reach it, and the pair
 * is what #59 replaces.
 */
#ifndef _BRMEMFS_H_
#define _BRMEMFS_H_

#include <brender.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct memfile_store {
    const char *filename; /* The one file this store is */
    int         mode;     /* BR_FS_MODE_*, as opened for writing */
    br_uint_8  *data;
    br_size_t   size; /* Bytes written, or the bytes to read */
    br_size_t   capacity;
    br_size_t   position; /* Read cursor */
} memfile_store;

br_filesystem *BR_RESIDENT_ENTRY BrMemFileInstall(memfile_store *store);
void BR_RESIDENT_ENTRY           BrMemFileRestore(br_filesystem *previous);

#ifdef __cplusplus
};
#endif
#endif
