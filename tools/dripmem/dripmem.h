#include "compiler.h"

/*
 * Tracking version of malloc()
 */
#ifdef __cplusplus
extern "C" {
#endif

void *BR_CDECL _drip_alloc(size_t size, char *from, char *type);
void BR_CDECL _drip_free(void *ptr);
void BR_CDECL _drip_set_tag(int tag);
void BR_CDECL _drip_set_log(int flag);
void BR_CDECL _drip_set_free(int flag);
void BR_CDECL _drip_checkpoint(char *filename, char *header, int dump_blocks);
void BR_CDECL _drip_dump(char *filename, char *header);

#ifdef __cplusplus
};
#endif

