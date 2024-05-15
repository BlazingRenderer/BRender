/*
 * $Id: brqsort.c 1.1 1997/12/10 16:41:05 jon Exp $
 * $Locker: $
 *
 * Local quicksort for brender
 *
 * Based on a public domain verion by Raymond Gardner,
 * Englewood CO  February 1991
 */

#include "brender.h"


#include <stdlib.h>

void BR_RESIDENT_ENTRY BrQsort(void *basep, unsigned int nelems, unsigned int size, br_qsort_cbfn *comp)
{
    qsort(basep, nelems, size, comp);
}
