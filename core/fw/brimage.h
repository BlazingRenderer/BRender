/*
 * Copyright (c) 1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: brimage.h 1.1 1997/12/10 16:41:04 jon Exp $
 * $Locker: $
 *
 */
#ifndef _BRIMAGE_H_
#define _BRIMAGE_H_

typedef void *(*br_image_proc)(void *);

#ifdef __H2INC__
typedef void br_image_function_info;
#else
typedef struct br_image_function_info {
    const char   *name;
    br_image_proc proc;
} br_image_function_info;
#endif

typedef struct br_image {
    /*
     * Anchor block for list of images
     */
    br_node node;

    /*
     * DLL name
     */
    const char *identifier;

    /*
     * Type of DLL - Resident, Host, Framework
     */
    br_int_32 type;

    /*
     * Number of references to this DLL
     */
    br_int_32 ref_count;

    /*
     * Table of exported functions. The ordinal is the function index.
     * Must be sorted by name, suitable for BrStrCmp().
     * Only valid if type != BR_IMG_HOST.
     */
    unsigned int                  n_functions;
    const br_image_function_info *functions;

    /*
     * Type specific pointer
     */
    void *type_pointer;

} br_image;

/*
 * Possible types of DLL
 */
enum {
    /*
     * A library loaded by BRender itself.
     * These are no longer used, this definition is only
     * provided for compatibility.
     */
    BR_IMG_FRAMEWORK = 1,

    /*
     * A statically-linked, or memory-resident library.
     */
    BR_IMG_RESIDENT,

    /*
     * A host-loaded library, e.g. from LoadLibrary() or dlopen().
     */
    BR_IMG_HOST,
};

#endif
