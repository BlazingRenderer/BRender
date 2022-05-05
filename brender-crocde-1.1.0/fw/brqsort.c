/*
 * $Id: brqsort.c 2.3 1996/02/17 18:00:39 sam Exp $
 * $Locker: $
 *
 * Local quicksort for brender
 *
 * Based on a public domain verion by Raymond Gardner,
 * Englewood CO  February 1991
 */

#include "brender.h"

BR_RCS_ID("$Id: brqsort.c 2.3 1996/02/17 18:00:39 sam Exp $")

#include <stdlib.h>

void BR_RESIDENT_ENTRY BrQsort(void *basep, unsigned int nelems, unsigned int size, br_qsort_cbfn *comp)
{
	qsort(basep, nelems, size, comp);
}
