#include "brender.h"

#ifdef __DREAMCAST__

void *BR_RESIDENT_ENTRY BrBSearch(const void *key, const void *base, unsigned int nmemb, unsigned int size, br_bsearch_cbfn *comp)
{
    // TODO
	return NULL;
}

#else

#include <stdlib.h>

void *BR_RESIDENT_ENTRY BrBSearch(const void *key, const void *base, unsigned int nmemb, unsigned int size, br_bsearch_cbfn *comp)
{
    return bsearch(key, base, nmemb, size, comp);
}

#endif
