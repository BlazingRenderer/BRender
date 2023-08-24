
#ifndef _BRSTB_H_
#define _BRSTB_H_

/*
 * stb helper functions
 */
void *BR_RESIDENT_ENTRY FmtSTBMalloc(br_size_t size);
void *BR_RESIDENT_ENTRY FmtSTBRealloc(void *ptr, br_size_t size);
void BR_RESIDENT_ENTRY  FmtSTBFree(void *ptr);
void BR_RESIDENT_ENTRY  FmtSTBFileWrite(void *context, void *data, int size);
int BR_RESIDENT_ENTRY   FmtSTBFileRead(void *user, char *data, int size);
void BR_RESIDENT_ENTRY  FmtSTBFileAdvance(void *user, int n);
int BR_RESIDENT_ENTRY   FmtSTBFileEof(void *user);

/*
 * stb_image_write defines & header
 */
#define STBI_NO_STDIO
#define STBI_FAILURE_USERMSG
#define STBI_MALLOC(s)     FmtSTBMalloc(s)
#define STBI_FREE(p)       FmtSTBFree(p)
#define STBI_REALLOC(p, s) FmtSTBRealloc(p, s)
#include "stb_image.h"

/*
 * file writing cbfn structure
 */
extern stbi_io_callbacks br_stb_file_cbfns;

#endif /* _BRSTB_H_ */
