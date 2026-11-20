/*
 * Material flag names, as the engine's own formats spell them.
 */
#ifndef _MATFLAG_H_
#define _MATFLAG_H_

#include "brender.h"

typedef struct br_material_flag {
    const char *name;
    br_uint_64  value;
    br_boolean  canonical; /* written by the formats; false = accepted on read only */
} br_material_flag;

extern const br_material_flag MaterialFlagNames[];

#endif /* _MATFLAG_H_ */
