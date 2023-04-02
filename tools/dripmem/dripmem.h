#include "compiler.h"

/*
 * Tracking version of malloc()
 */
#ifdef __cplusplus
extern "C" {
#endif

void *BR_CDECL _drip_alloc(size_t size, const char *from, const char *type);
void BR_CDECL  _drip_free(void *ptr);
void BR_CDECL  _drip_set_tag(int tag);
void BR_CDECL  _drip_set_log(int flag);
void BR_CDECL  _drip_set_free(int flag);
void BR_CDECL  _drip_checkpoint(const char *filename, const char *header, int dump_blocks);
void BR_CDECL  _drip_dump(const char *filename, const char *header);

#ifdef __cplusplus
};
#endif
