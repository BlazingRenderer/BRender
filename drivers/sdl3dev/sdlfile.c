#include "brddi_p.h"

#include <SDL3/SDL.h>
#include <brender.h>
#include <brassert.h>

static br_uint_32 sdlfs_attributes(void)
{
    return BR_FS_ATTR_READABLE | BR_FS_ATTR_WRITEABLE | BR_FS_ATTR_HAS_TEXT | BR_FS_ATTR_HAS_BINARY | BR_FS_ATTR_HAS_ADVANCE;
}

static void *sdlfs_open_read(const char *name, br_size_t n_magics, br_mode_test_cbfn *identify, int *mode_result)
{
    SDL_IOStream *fh;
    char         *br_path, config_path[512];
    const char   *try_name;
    char         *cp;
    br_uint_8     magics[BR_MAX_FILE_MAGICS];
    int           open_mode = BR_FS_MODE_BINARY;

    /*
     * Try the current directory
     */
    try_name = name;

    if((fh = SDL_IOFromFile(try_name, "rb")) == NULL) {

        /*
         * If that fails, and if a drive or a directory were specified, don't search along path
         */
        if(BrStrCmp("Windows", SDL_GetPlatform())) {
            if(SDL_strchr(name, ':') || SDL_strchr(name, '/') || SDL_strchr(name, '\\'))
                return NULL;
        } else {
            if(SDL_strchr(name, '/'))
                return NULL;
        }

        /*
         * For each element of the path, if it exists
         */
        if(BrSystemConfigQueryString(BRT_BRENDER_PATH_STR, config_path, BR_ASIZE(config_path)) != BRE_OK)
            return NULL;

        br_path = config_path;

        while(*br_path) {
            /*
             * Take characters until next seperator or terminator
             */
            cp = try_name;

            while(*br_path != ';' && *br_path != '\0')
                *cp++ = *br_path++;

            if(*br_path == ';')
                br_path++;

            /*
             * Add a directory seperator if none
             */
            if(cp != try_name && (*(cp - 1) != ':' && *(cp - 1) != '/' && *(cp - 1) != '\\'))
                *cp++ = '/';

            strcpy(cp, name);

            if((fh = SDL_IOFromFile(try_name, "rb")) != NULL)
                break;
        }

        if(fh == NULL)
            return NULL;
    }

    if(mode_result)
        open_mode = *mode_result;

    if(n_magics != 0) {
        /*
         * Now have an open file, try and grab the first bytes from it
         */
        if(SDL_ReadIO(fh, magics, n_magics) != n_magics) {
            /*
             * Could not read all the required data, close and punt
             */
            SDL_CloseIO(fh);
            return NULL;
        }

        /*
         * Try and identify the file
         */
        if(identify)
            open_mode = identify(magics, n_magics);

        if(mode_result)
            *mode_result = open_mode;
    }

    /*
     * Reopen file with its new identity (or abandon if unknown identity)
     */
    SDL_CloseIO(fh);

    switch(open_mode) {
        case BR_FS_MODE_TEXT:
            fh = SDL_IOFromFile(try_name, "r");
            break;

        case BR_FS_MODE_BINARY:
            fh = SDL_IOFromFile(try_name, "rb");
            break;

        case BR_FS_MODE_UNKNOWN:
            fh = NULL;
            break;

        default:
            BR_ERROR1("BrStdFileOpenRead: invalid open_mode %d", open_mode);
    }

    return fh;
}

static void *sdlfs_open_write(const char *name, int mode)
{
    return SDL_IOFromFile(name, (mode == BR_FS_MODE_TEXT) ? "w" : "wb");
}

static void sdlfs_close(void *f)
{
    SDL_CloseIO(f);
}

static int sdlfs_eof(void *f)
{
    return SDL_GetIOStatus(f) == SDL_IO_STATUS_EOF;
}

static int sdlfs_getchar(void *f)
{
    Uint8 val;

    if(!SDL_ReadU8(f, &val)) {
        return -1;
    }

    return val;
}

static void sdlfs_putchar(int c, void *f)
{
    SDL_WriteU8(f, (Uint8)c);
}

static br_size_t sdlfs_read(void *buf, br_size_t size, br_size_t n, void *f)
{
    return SDL_ReadIO(f, buf, size * n);
}

static br_size_t sdlfs_write(const void *buf, br_size_t size, br_size_t n, void *f)
{
    return SDL_WriteIO(f, buf, size * n);
}

static br_size_t sdlfs_getline(char *buf, br_size_t buf_len, void *f)
{
    size_t i;
    Uint8 *b = (Uint8 *)buf;

    if(buf_len == 0)
        return 0;

    for(i = 0; i < buf_len - 1; ++i) {
        if(!SDL_ReadU8(f, b + i)) {
            break;
        }

        if(b[i] == '\n')
            break;
    }

    b[i] = '\0';

    if(i > 0 && b[i - 1] == '\n')
        b[--i] = '\0';

    return i;
}

static void sdlfs_putline(const char *buf, void *f)
{
    SDL_IOprintf(f, "%s\n", buf);
}

static void sdlfs_advance(br_size_t count, void *f)
{
    SDL_SeekIO(f, (Sint64)count, SDL_IO_SEEK_CUR);
}

static void *BR_CALLBACK sdlfs_load(void *vres, const char *name, br_size_t *size)
{
    size_t size_ = 0;
    void  *sdl_data, *data;
    if((sdl_data = SDL_LoadFile(name, &size_)) == NULL)
        return NULL;

    if((data = BrResAllocate(vres, size_, BR_MEMORY_APPLICATION)) == NULL) {
        SDL_free(sdl_data);
        return NULL;
    }

    SDL_memcpy(data, sdl_data, size_);
    SDL_free(sdl_data);

    if(size != NULL)
        *size = size_;

    return data;
}

static br_filesystem BrSDLFilesystem = {
    .identifier = "SDL3 Filesystem",
    .attributes = sdlfs_attributes,
    .open_read  = sdlfs_open_read,
    .open_write = sdlfs_open_write,
    .close      = sdlfs_close,
    .eof        = sdlfs_eof,
    .getchr     = sdlfs_getchar,
    .putchr     = sdlfs_putchar,
    .read       = sdlfs_read,
    .write      = sdlfs_write,
    .getline    = sdlfs_getline,
    .putline    = sdlfs_putline,
    .advance    = sdlfs_advance,
    .load       = sdlfs_load,
};
