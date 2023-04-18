/*
 * Copyright (c) 1993-1995 by Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: ffhooks.c 1.1 1997/12/10 16:41:30 jon Exp $
 * $Locker: $
 *
 * FindFailed hooks that load models/materials/maps/tables from the filesystem
 */
#include "brender.h"

BR_RCS_ID("$Id: ffhooks.c 1.1 1997/12/10 16:41:30 jon Exp $")

/*
 * Find Failed callbacks to automatically load textures & tables
 */
br_pixelmap *BR_CALLBACK BrMapFindFailedLoad(const char *name)
{
    br_pixelmap *pm;

    if((pm = BrPixelmapLoad(name)) != NULL) {
        pm->identifier = BrResStrDup(pm, name);
        BrMapAdd(pm);
    }

    return pm;
}

br_pixelmap *BR_CALLBACK BrTableFindFailedLoad(const char *name)
{
    br_pixelmap *pm;

    if((pm = BrPixelmapLoad(name)) != NULL) {
        pm->identifier = BrResStrDup(pm, name);
        BrTableAdd(pm);
    }

    return pm;
}

br_model *BR_CALLBACK BrModelFindFailedLoad(const char *name)
{
    br_model *m;

    if((m = BrModelLoad(name)) != NULL) {
        m->identifier = BrResStrDup(m, name);
        BrModelAdd(m);
    }

    return m;
}

br_material *BR_CALLBACK BrMaterialFindFailedLoad(const char *name)
{
    br_material *m;

    if((m = BrMaterialLoad(name)) != NULL) {
        m->identifier = BrResStrDup(m, name);
        BrMaterialAdd(m);
    }

    return m;
}
