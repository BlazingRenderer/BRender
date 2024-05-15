/*
 * Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: def_otab.c 1.1 1997/12/10 16:41:30 jon Exp $
 * $Locker: $
 *
 * Default order_table for bucket Z-sort renderer
 */
#include "v1db.h"


#define DEFAULT_ORDER_TABLE_SIZE 256

static br_primitive *default_table[DEFAULT_ORDER_TABLE_SIZE];

br_order_table _BrDefaultOrderTable = {
    /*
     * Default order table
     */
    .table  = default_table,               /* Pointer to table */
    .size   = DEFAULT_ORDER_TABLE_SIZE,    /* Size             */
    .next   = NULL,                        /* Next             */
    .min_z  = BR_SCALAR(1.0),              /* Minimum Z        */
    .max_z  = BR_SCALAR(10.0),             /* Maximum Z        */
    .sort_z = BR_SCALAR(0.0),              /* Sort Z           */
    .scale  = BR_SCALAR(1.0 / 9.0),        /* Scale            */
    .flags  = BR_ORDER_TABLE_LEAVE_BOUNDS, /* Flags            */
    .type   = BR_SORT_AVERAGE,             /* Sort type        */
    .visits = 0                            /* Visit count      */
};
