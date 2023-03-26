#include "brender.h"

#include <stdlib.h>

void *BR_RESIDENT_ENTRY BrBSearch(const void *key, const void *base, unsigned int nmemb, unsigned int size, br_bsearch_cbfn *comp)
{
    return bsearch(key, base, nmemb, size, comp);
}
