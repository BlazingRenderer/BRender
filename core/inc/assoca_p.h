/*
 * Copyright (c) 1993-1995 by Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: assoca_p.h 1.2 1998/02/26 21:17:28 jon Exp $
 * $Locker: $
 *
 * Public function prototypes for BRender associative arrays
 */
#ifndef _ASSOC_P_H_
#define _ASSOC_P_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _NO_PROTOTYPES

BR_API br_associative_array *BrAssociativeArrayAllocate();
BR_API br_error              BrAssociativeArraySet(br_associative_array *array, br_token t, br_value v);
BR_API br_error              BrAssociativeArrayRemove(br_associative_array *array, br_token t);
BR_API br_error              BrAssociativeArrayQuery(br_associative_array *array, br_token t, br_value *value);

/*
 * Compatibility
 */
#define BrAssociativeArraySetEntry    BrAssociativeArraySet
#define BrAssociativeArrayRemoveEntry BrAssociativeArrayRemove

#endif /* _NO_PROTOTYPES */

#ifdef __cplusplus
};
#endif
#endif
