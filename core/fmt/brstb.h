
#ifndef _BRSTB_H_
#define _BRSTB_H_

/*
 * stb helper functions
 */
void *FmtSTBMalloc(br_size_t size);
void *FmtSTBRealloc(void *ptr, br_size_t size);
void  FmtSTBFree(void *ptr);
void  FmtSTBFileWrite(void *context, void *data, int size);
int   FmtSTBFileRead(void *user, char *data, int size);
void  FmtSTBFileAdvance(void *user, int n);
int   FmtSTBFileEof(void *user);

/*
 * stb_image defines & header
 */
#define STBI_NO_STDIO
#define STBI_FAILURE_USERMSG
#define STBI_MALLOC(s)     FmtSTBMalloc(s)
#define STBI_FREE(p)       FmtSTBFree(p)
#define STBI_REALLOC(p, s) FmtSTBRealloc(p, s)
#include "stb_image.h"

/*
 * stb_image_write defines & header
 */
#define STBI_WRITE_NO_STDIO
#define STBIW_MALLOC(s)     FmtSTBMalloc(s)
#define STBIW_FREE(p)       FmtSTBFree(p)
#define STBIW_REALLOC(p, s) FmtSTBRealloc(p, s)
#include "stb_image_write.h"

/*
 * file writing cbfn structure
 */
extern stbi_io_callbacks br_stb_file_cbfns;

#endif /* _BRSTB_H_ */
