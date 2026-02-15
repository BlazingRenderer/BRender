#ifndef _TINYALLOC_H_
#define _TINYALLOC_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>

bool ta_init(const void *base, const void *limit, const size_t heap_blocks, const size_t split_thresh, const size_t alignment);
void *ta_alloc(size_t num);
void *ta_calloc(size_t num, size_t size);
size_t ta_getsize(void *ptr);
void *ta_realloc(void *ptr, size_t num);
bool ta_free(void *ptr);

// counted in blocks
size_t ta_num_free(void);
size_t ta_num_used(void);
size_t ta_num_fresh(void);

// counted in bytes
size_t ta_size_free(void);
size_t ta_size_used(void);
size_t ta_size_fresh(void);

bool ta_check(void);

#ifdef __cplusplus
}
#endif
#endif // _TINYALLOC_H_
