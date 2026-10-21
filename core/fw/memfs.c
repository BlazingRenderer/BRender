/*
 * An in-memory filing system.
 *
 * BrPixelmapSave()/BrPixelmapLoad() go through fw.fsys for every character they
 * write or read, so a caller that wants the bytes rather than a file installs this
 * one for the duration of the call and collects the store afterwards. It serves
 * exactly one virtual file: the store names it and records the mode it was written
 * in, so open_read() does not have to guess.
 *
 * Install and restore are the caller's: BrFilesystemSet() returns the previous
 * filesystem, and every path back must put it back. Single-threaded, so one store at
 * a time is enough - and a nested open inside the call would be served by the store
 * too, which is why the store refuses a name it did not write.
 */
#include <stddef.h>
#include <string.h>

#include "fw.h"
#include "fw_ip.h"
#include "brmemfs.h"
#include "brassert.h"

/*
 * The store one of these callbacks is working on
 */
static memfile_store *memfile_current;

static br_uint_32 BR_CALLBACK BrMemFileAttributes(void)
{
    return BR_FS_ATTR_READABLE | BR_FS_ATTR_WRITEABLE | BR_FS_ATTR_HAS_TEXT | BR_FS_ATTR_HAS_BINARY | BR_FS_ATTR_HAS_ADVANCE;
}

/*
 * Open the store for reading. There is one file, so the name has to be the one the
 * store was written under; an empty store is a file that does not exist.
 */
static void *BR_CALLBACK BrMemFileOpenRead(const char *name, br_size_t n_magics, br_mode_test_cbfn *identify, int *mode_result)
{
    memfile_store *store = memfile_current;
    int            mode  = BR_FS_MODE_BINARY;

    if(store == NULL || store->data == NULL || store->size == 0)
        return NULL;

    if(store->filename != NULL && name != NULL && BrStrCmp(store->filename, name) != 0)
        return NULL;

    /* The mode it was written in, or the one the caller already decided on. */
    if(store->mode != BR_FS_MODE_UNKNOWN)
        mode = store->mode;

    if(mode_result != NULL)
        mode = *mode_result != BR_FS_MODE_UNKNOWN ? *mode_result : mode;

    if(n_magics != 0 && identify != NULL) {
        br_uint_8 magics[BR_MAX_FILE_MAGICS];

        if(n_magics > store->size || n_magics > BR_ASIZE(magics))
            return NULL;

        BrMemCpy(magics, store->data, n_magics);
        mode = identify(magics, n_magics);
    }

    if(mode_result != NULL)
        *mode_result = mode;

    if(mode == BR_FS_MODE_UNKNOWN)
        return NULL;

    store->position = 0;

    return store;
}

static void *BR_CALLBACK BrMemFileOpenWrite(const char *name, int mode)
{
    memfile_store *store = memfile_current;

    if(store == NULL)
        return NULL;

    store->filename = name;
    store->mode     = mode;
    store->size     = 0;
    store->position = 0;

    return store;
}

static void BR_CALLBACK BrMemFileClose(void *f)
{
    (void)f;
}

static int BR_CALLBACK BrMemFileEof(void *f)
{
    memfile_store *store = f;

    return store->position >= store->size;
}

static int BR_CALLBACK BrMemFileGetChar(void *f)
{
    memfile_store *store = f;

    if(store->position >= store->size)
        return -1;

    return store->data[store->position++];
}

/*
 * Make room for `extra` more bytes, doubling. Returns BR_FALSE when it cannot.
 */
static br_boolean memfile_reserve(memfile_store *store, br_size_t extra)
{
    br_size_t  capacity;
    br_uint_8 *data;

    if(store->size + extra <= store->capacity)
        return BR_TRUE;

    capacity = store->capacity != 0 ? store->capacity : 256;

    while(capacity < store->size + extra)
        capacity *= 2;

    if((data = BrResAllocate(NULL, capacity, BR_MEMORY_SCRATCH)) == NULL)
        return BR_FALSE;

    if(store->data != NULL) {
        BrMemCpy(data, store->data, store->size);
        BrResFree(store->data);
    }

    store->data     = data;
    store->capacity = capacity;

    return BR_TRUE;
}

static void BR_CALLBACK BrMemFilePutChar(int c, void *f)
{
    memfile_store *store = f;

    if(!memfile_reserve(store, 1))
        return;

    store->data[store->size++] = (br_uint_8)c;
}

/*
 * Read the whole store at once, which is what BrFileLoad() asks for. The buffer is a copy:
 * the store belongs to the caller that installed it, and whatever is read from here is
 * freed through the resource anchor it was allocated from.
 */
static void *BR_CALLBACK BrMemFileLoad(void *res, const char *name, br_size_t *size)
{
    memfile_store *store = memfile_current;
    void          *buffer;

    if(store == NULL || store->data == NULL || store->size == 0)
        return NULL;

    if(store->filename != NULL && name != NULL && BrStrCmp(store->filename, name) != 0)
        return NULL;

    if((buffer = BrResAllocate(res, store->size, BR_MEMORY_APPLICATION)) == NULL)
        return NULL;

    BrMemCpy(buffer, store->data, store->size);

    if(size != NULL)
        *size = store->size;

    return buffer;
}

static br_size_t BR_CALLBACK BrMemFileRead(void *buf, br_size_t size, br_size_t n, void *f)
{
    memfile_store *store = f;
    br_size_t      want;
    br_size_t      got;

    if(size == 0 || n == 0)
        return 0;

    want = size * n;
    got  = store->position < store->size ? store->size - store->position : 0;

    if(got > want)
        got = want;

    /* fread() semantics: whole elements only. */
    got -= got % size;

    BrMemCpy(buf, store->data + store->position, got);
    store->position += got;

    return got / size;
}

static br_size_t BR_CALLBACK BrMemFileWrite(const void *buf, br_size_t size, br_size_t n, void *f)
{
    memfile_store *store = f;
    br_size_t      total = size * n;

    if(total == 0)
        return 0;

    if(!memfile_reserve(store, total))
        return 0;

    BrMemCpy(store->data + store->size, buf, total);
    store->size += total;

    return n;
}

static br_size_t BR_CALLBACK BrMemFileGetLine(char *buf, br_size_t buf_len, void *f)
{
    memfile_store *store = f;
    br_size_t      len   = 0;

    if(buf_len == 0)
        return 0;

    while(len + 1 < buf_len && store->position < store->size) {
        const char c = (char)store->data[store->position++];

        if(c == '\n')
            break;

        buf[len++] = c;
    }

    /* fgets() leaves the terminator on; the default filesystem strips it. */
    if(len > 0 && buf[len - 1] == '\r')
        len--;

    buf[len] = '\0';

    return len;
}

static void BR_CALLBACK BrMemFilePutLine(const char *buf, void *f)
{
    memfile_store *store = f;
    br_size_t      len   = BrStrLen(buf);

    if(!memfile_reserve(store, len + 1))
        return;

    BrMemCpy(store->data + store->size, buf, len);
    store->size += len;
    store->data[store->size++] = '\n';
}

static void BR_CALLBACK BrMemFileAdvance(br_size_t count, void *f)
{
    memfile_store *store = f;

    store->position = count < store->size - store->position ? store->position + count : store->size;
}

static br_filesystem BrMemFilesystem = {
    .identifier = "memory",
    .attributes = BrMemFileAttributes,
    .open_read  = BrMemFileOpenRead,
    .open_write = BrMemFileOpenWrite,
    .close      = BrMemFileClose,
    .eof        = BrMemFileEof,
    .getchr     = BrMemFileGetChar,
    .putchr     = BrMemFilePutChar,
    .read       = BrMemFileRead,
    .write      = BrMemFileWrite,
    .getline    = BrMemFileGetLine,
    .putline    = BrMemFilePutLine,
    .advance    = BrMemFileAdvance,
    .load       = BrMemFileLoad,
};

br_filesystem *BR_RESIDENT_ENTRY BrMemFileInstall(memfile_store *store)
{
    memfile_current = store;

    return BrFilesystemSet(&BrMemFilesystem);
}

void BR_RESIDENT_ENTRY BrMemFileRestore(br_filesystem *previous)
{
    BrFilesystemSet(previous);
    memfile_current = NULL;
}
