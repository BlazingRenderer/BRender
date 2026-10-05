/*
 * Copyright (c) 1993-1995 by Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: brmem.h 1.1 1997/12/10 16:41:16 jon Exp $
 * $Locker: $
 *
 * Brender's interface to memory allocation
 */

#ifndef _BRMEM_H_
#define _BRMEM_H_

/*
 * Instance of a memory allocator
 */
/**
 * \brief An application defined call-back function returning a pointer to newly allocated memory of
 *        specified size and type.
 *
 * \param size Size in bytes of memory block required. More bytes may be allocated, but the caller
 *             will only use the first size bytes. Zero is a valid value.
 * \param type Class (type ID) of memory required. Although it is possible for the class to be
 *             ignored, it may be used for diagnostic purposes, or could be used to provide a more
 *             efficient allocation scheme. See Memory Classes for a description of possible values.
 *
 * \pre The diagnostic handler has been setup (BrDiagHandlerSet()). BRender has not necessarily
 *      completed initialisation. This may not be the first allocator to have been called. BRender
 *      is the only direct caller of this function.
 *
 * \post Obtain a fixed, contiguous, persistent, readable, writable, non-volatile area of memory of
 *       at least the size specified (even if zero), that will remain so until a corresponding call
 *       of brmem_free_cbfn has occurred.
 *
 * \return The address of a block of memory of the required size. If the request cannot be
 *         satisfied, return NULL. No assumption can be made concerning any relation between the
 *         address returned by this call and any previous or subsequent call.
 *
 * \remark Although BRender's default diagnostic handling system does not use the memory handler, if
 *         you supply a diagnostic handler that does, you may have to handle memory allocation
 *         failures directly. The value returned by brmem_inquire_cbfn does not guarantee the
 *         subsequent success or failure of brmem_allocate_cbfn.
 *
 * \sa brmem_free_cbfn, brmem_inquire_cbfn
 *
 * \par Example
 * \code{.c}
 * See stdmem.c for examples of memory allocation functions.
 * \endcode
 */
typedef void *BR_CALLBACK      brmem_allocate_cbfn(br_size_t size, br_uint_8 type);
typedef void *BR_CALLBACK      brmem_reallocate_cbfn(void *ptr, br_size_t size, br_uint_8 type);
/**
 * \brief An application defined call-back function that will reclaim memory previously allocated by
 *        brmem_allocate_cbfn.
 *
 * \param block Address of memory no longer required, previously returned by the corresponding
 *              brmem_allocate_cbfn (with no intervening brmem_free_cbfn). NULL is ignored.
 *
 * \pre The diagnostic handler has been setup (BrDiagHandlerSet()). BRender has not necessarily
 *      completed initialisation. Other allocation handlers may have already been used. BRender is
 *      the only direct caller of this function. The application has disposed of all references to
 *      the memory (obtained using the supplied address).
 *
 * \post Release supplied memory (presumably making it available for re-use). If the supplied
 *       address is invalid (not recognised as previously allocated by the corresponding
 *       brmem_allocate_cbfn), consider this a failure condition and handle appropriately. Note that
 *       the BRender diagnostic handling system may not be available, or even capable of handling
 *       this condition.
 *
 * \remark The memory freed will not necessarily increase the value returned by brmem_inquire_cbfn,
 *         though it is of course a hopeful effect. The application should immediately set to NULL
 *         all pointers referencing the memory freed, (at least in a debug version).
 *
 * \sa brmem_allocate_cbfn, brmem_inquire_cbfn
 *
 * \par Example
 * \code{.c}
 * See stdmem.c for examples of memory deallocation functions.
 * \endcode
 */
typedef void BR_CALLBACK       brmem_free_cbfn(void *block);
/**
 * \brief An application defined call-back function providing details of memory availability.
 *
 * \param type Class of memory for which information is required. See Memory Classes for a
 *             description of possible values.
 *
 * \pre The diagnostic handler has been setup (BrDiagHandlerSet()). BRender has not necessarily
 *      completed initialisation. Other allocation handlers may have already been used. BRender is
 *      the only direct caller of this function.
 *
 * \post Calculate an estimate of available memory for a particular memory class. This should be the
 *       total number of free bytes, irrespective of fragmentation or likely block-header overheads.
 *       If the class memory space would be extended, then the estimate should be of the potential
 *       maximum size available.
 *
 * \return The total number of bytes remaining unallocated in the specified memory class.
 *
 * \remark This function is only intended to provide an estimate of available memory. It does not
 *         guarantee that a particular allocation would succeed (or fail). However, it could be used
 *         as an indication of likely success or failure. There is no relation defined between the
 *         values returned for each class. In some allocation schemes, each class may have a
 *         separate and fixed amount of memory set aside, whereas in other schemes, all classes may
 *         be sharing all memory. There is no relation defined between the true amount of memory
 *         (whether virtual or physical) in a system, and the value returned by this function.
 *
 * \sa brmem_allocate_cbfn, brmem_free_cbfn
 *
 * \par Example
 * \code{.c}
 * See stdmem.c for examples of memory inquiry functions.
 * \endcode
 */
typedef br_size_t BR_CALLBACK  brmem_inquire_cbfn(br_uint_8 type);
typedef br_uint_32 BR_CALLBACK brmem_align_cbfn(br_uint_8 type);

/**
 * \brief This structure represents the definition of a memory allocation system or memory handler.
 *
 * All BRender's memory allocation is provided by just three functions, which can be specified by
 * the programmer. This is essential in cases where the standard C library functions, which
 * BRender's handler uses by default, are not available. Sometimes, more sophisticated behaviour is
 * desired, or diagnostic features are needed.
 */
typedef struct br_allocator {
    /**
     * \brief Pointer to unique, zero terminated, character string (or NULL if not required).
     *
     * A string constant is recommended.
     */
    char *identifier;

    /**
     * \brief This is a pointer to the memory allocation function (See brmem_allocate_cbfn).
     *
     * This is called by BrMemAllocate().
     */
    brmem_allocate_cbfn *allocate;

    /*
     * (re)allocate a chunk of memory - same semantics as realloc(3).
     */
    brmem_reallocate_cbfn *reallocate;

    /**
     * \brief This is a pointer to the memory deallocation function (See brmem_free_cbfn).
     *
     * This is called by BrMemFree().
     */
    brmem_free_cbfn *free;

    /**
     * \brief This is a pointer to the memory inquiry function (See brmem_inquire_cbfn).
     *
     * This is called by BrMemInquire().
     */
    brmem_inquire_cbfn *inquire;

    /*
     * Inquire as to the minimum alignment (in bytes) that allocations
     * of the supplied type will have
     *
     * Assumed to be 1 is this pointer is NULL
     */
    brmem_align_cbfn *align;

} br_allocator;

/*
 * Classes of resource that brender allocates
 *
 * Valid values are 1 to 255
 */
enum br_memory_classes {
    /*
     * System classes
     */
    BR_MEMORY_SCRATCH = 1,
    BR_MEMORY_PIXELMAP,
    BR_MEMORY_PIXELS,
    BR_MEMORY_VERTICES,
    BR_MEMORY_FACES,
    BR_MEMORY_GROUPS,
    BR_MEMORY_MODEL,
    BR_MEMORY_MATERIAL,
    BR_MEMORY_MATERIAL_INDEX,
    BR_MEMORY_ACTOR,
    BR_MEMORY_PREPARED_VERTICES,
    BR_MEMORY_PREPARED_FACES,
    BR_MEMORY_LIGHT,
    BR_MEMORY_CAMERA,
    BR_MEMORY_BOUNDS,
    BR_MEMORY_CLIP_PLANE,
    BR_MEMORY_STRING,
    BR_MEMORY_REGISTRY,
    BR_MEMORY_TRANSFORM,
    BR_MEMORY_RESOURCE_CLASS,
    BR_MEMORY_FILE,
    BR_MEMORY_ANCHOR,
    BR_MEMORY_POOL,
    BR_MEMORY_RENDER_MATERIAL,
    BR_MEMORY_DATAFILE,
    BR_MEMORY_IMAGE,
    BR_MEMORY_IMAGE_ARENA,
    BR_MEMORY_IMAGE_SECTIONS,
    BR_MEMORY_IMAGE_NAMES,
    BR_MEMORY_EXCEPTION_HANDLER,
    BR_MEMORY_RENDER_DATA,
    BR_MEMORY_TOKEN,
    BR_MEMORY_TOKEN_MAP,
    BR_MEMORY_OBJECT,
    BR_MEMORY_OBJECT_DATA,
    BR_MEMORY_DRIVER,
    BR_MEMORY_LEXER,
    BR_MEMORY_OBJECT_LIST,
    BR_MEMORY_OBJECT_LIST_ENTRY,
    BR_MEMORY_ENABLED_ACTORS,
    BR_MEMORY_FMT_RESULTS,
    BR_MEMORY_PREPARED_MODEL,
    BR_MEMORY_ORDER_TABLE,
    BR_MEMORY_TOKEN_VALUE,
    BR_MEMORY_TOKEN_TEMPLATE,

    /*
     * Marker for blocks being freed
     */
    BR_MEMORY_FREE = 0x7f,

    /*
     * Application classes
     */
    BR_MEMORY_APPLICATION = 0x80,

    /*
     * User defined classed are BR_MEMORY_APPLICATION + 1 ... 127
     */
    BR_MEMORY_MAX = 256
};

/*
 * A resource class structure
 */
/**
 * \brief An application defined call-back function accepting details of a resource block just
 *        before it will be freed.
 *
 * \param res       The resource block about to be freed.
 * \param res_class The memory class of the resource block's resource class.
 * \param size      The size of the resource block (useful for arrays).
 *
 * \pre BRender has completed initialisation.
 *
 * \post Application defined. Do not free the block - this will be performed after the function
 *       returns. This function will also be subsequently called for any child resource blocks still
 *       attached.
 *
 * \sa BrResFree().
 */
typedef void BR_CALLBACK br_resourcefree_cbfn(void *res, br_uint_8 res_class, br_size_t size);

/**
 * \brief This structure is used for application defined resource classes.
 *
 * Once the structure is initialised, it is registered using BrResClassAdd(), thereafter allowing
 * application defined memory or resource classes to be used.
 */
typedef struct br_resource_class {
    br_uintptr_t          reserved;
    /**
     * \brief Pointer to unique, zero terminated, character string (or NULL if not required).
     *
     * A string constant is recommended. The alternative is to use something like:
     *
     * \code{.c}
     *   my_res_class=BrResAllocate(NULL,sizeof(br_resource_class),
     *                       BR_MEMORY_RESOURCE_CLASS);
     *   my_res_class->identifier=BrResStrDup(my_res_class,"MyClass");
     * \endcode
     */
    const char           *identifier;
    /**
     * \brief The memory class of the new resource class.
     *
     * See Memory Classes for a description of possible values. Only values between
     * BR_MEMORY_APPLICATION+1 and BR_MEMORY_MAX-1 (inclusive) should be used.
     */
    br_uint_8             res_class;
    /**
     * \brief This is a pointer to the destructor function (See br_resourcefree_cbfn) it may be NULL if
     *        not required.
     *
     * Note that this function is not used to free the memory of the resource. It is simply an
     * opportunity for the application to perform any other housekeeping functions indicated by the
     * destruction of the resource. The resource is destroyed after the destructor returns. For example,
     * a user defined resource may be of structures containing pointers to reference counted
     * (non-resource) items. In such a case, the referenced item will need to be dereferenced.
     */
    br_resourcefree_cbfn *free_cb;
    br_uint_32            alignment; /* Alignment required for this class - 0 for default */
} br_resource_class;

#endif
