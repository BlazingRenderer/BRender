/*
 * Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: error.c 1.1 1997/12/10 16:41:06 jon Exp $
 * $Locker: $
 *
 * Error value support
 */

#include "fw.h"

br_error BR_RESIDENT_ENTRY BrLastErrorGet(void **valuep)
{
    if(valuep)
        valuep = fw.last_error_value;

    return fw.last_error_type;
}

void BR_RESIDENT_ENTRY BrLastErrorSet(br_error type, void *value)
{
    fw.last_error_type  = type;
    fw.last_error_value = value;
}

// clang-format off
const char *br_error_misc_subclasses[] = {
    [0] = "No error",
    [1] = "Unknown",
    [2] = "Failure",
    [3] = "Not enough memory",
    [4] = "Overflow",
    [5] = "Underflow",
    [6] = "Not active",
    [7] = "Already active",
    [8] = "Unsupported",
    [9] = "Invalid",
};
// clang-format on

// clang-format off
const char *br_error_device_subclasses[] = {
    [0]  = "No error",
    [1]  = "General device failure",
    [2]  = "Hardware not detected",
    [3]  = "Failed general memory allocation",
    [4]  = "Failed video memory allocation",
    [5]  = "Capabilities unsupported",
    [6]  = "Hardware setup failed",
    [7]  = "Hardware not ready",
    [8]  = "Hardware driver not present",
    [9]  = "Hardware request refused",
    [10] = "Hardware locking call failed",
    [11] = "Hardware output facility is not available",
    [12] = "Hardware is already initialized",
    [13] = "DirectDraw error",
};
// clang-format on

// clang-format off
br_error_class br_error_classes[] = {
    {.classname = "None",        .subclasses = NULL},
    {.classname = "Misc",        .subclasses = br_error_misc_subclasses},
    {.classname = "Application", .subclasses = NULL},
    {.classname = "Database",    .subclasses = NULL},
    {.classname = "Renderer",    .subclasses = NULL},
    {.classname = "File",        .subclasses = NULL},
    {.classname = "Token",       .subclasses = NULL},
    {.classname = "Math",        .subclasses = NULL},
    {.classname = "DOS I/O",     .subclasses = NULL},
    {.classname = "Host",        .subclasses = NULL},
    {.classname = "Device",      .subclasses = br_error_device_subclasses},
};
// clang-format on

const char *BR_RESIDENT_ENTRY BrStrError(br_error error)
{
    /* get class and subclass ids and names */
    br_error class           = BR_ERROR_CLASS(error) / 4096;
    br_error    subclass     = BR_ERROR_SUBCLASS(error);
    const char *classname    = br_error_classes[class].classname;
    const char *subclassname = NULL;

    /* clear string buffer */
    BrMemSet(fw.last_error_string, 0, sizeof(fw.last_error_string));

    /* not all classes have defined subclasses (yet?)... */
    if(br_error_classes[class].subclasses) {
        /* get subclass name */
        subclassname = br_error_classes[class].subclasses[subclass];

        /* sprintf it */
        BrSprintfN(fw.last_error_string, BR_ASIZE(fw.last_error_string), "%s: %s", classname, subclassname);
    } else {
        /* if subclass == 0, that means there's no error */
        if(subclass == 0) {
            BrSprintfN(fw.last_error_string, BR_ASIZE(fw.last_error_string), "%s: No error", classname);
        } else {
            BrSprintfN(fw.last_error_string, BR_ASIZE(fw.last_error_string), "%s: Failure", classname);
        }
    }

    /* add null terminator */
    fw.last_error_string[BR_ASIZE(fw.last_error_string) - 1] = '\0';

    /* return ptr to buffer */
    return fw.last_error_string;
}
