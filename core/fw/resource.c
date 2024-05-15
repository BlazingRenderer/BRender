/*
 * Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: resource.c 1.1 1997/12/10 16:41:10 jon Exp $
 * $Locker: $
 *
 * Generic code for maintaining a hierachy of resources
 *
 * Each resource has -
 *		A class (one of the BR_RES_xxx constants)
 *		A size
 *		A block of memory of 'size' bytes
 *		0 or more child resources
 *
 * XXX
 *	Add optional source file/line tracking
 */
#include <inttypes.h>
#include <stdio.h>
#include "fw.h"
#include "brassert.h"


/*
 * The granularity of resource sizes
 */
#define BR_RES_GRANULARITY_SHIFT 2
#define BR_RES_GRANULARITY       (1 << BR_RES_GRANULARITY_SHIFT)
#define BR_RES_MAGIC             0xDEADBEEF

#define BR_RES_DEFAULT_ALIGN     8

/*
 * The control structure prepended onto resource blocks
 */
struct resource_header {
    br_simple_node node;
    br_simple_list children;

    br_size_t size;

    br_uint_8 class; /* Class of resource	*/

#if BR_RES_TAGGING
    void      *magic_ptr;
    br_uint_32 magic_num;
#endif
};

/*
 * Return size of a resource block
 */
#define RES_SIZE_GET(r) ((r)->size << BR_RES_GRANULARITY_SHIFT)

/*
 * Set size of a resource block
 */
#define RES_SIZE_SET(r, s)                           \
    do {                                             \
        (r)->size = (s) >> BR_RES_GRANULARITY_SHIFT; \
    } while(0)

#define ALIGN(ptr, a) (((br_uintptr_t)(ptr) + ((a)-1)) & ~((a)-1))

#if BR_RES_TAGGING
#define ISRESOURCE(res) ((res->magic_ptr == res) && (res->magic_num == BR_RES_MAGIC))
#else
#define ISRESOURCE(res) 1
#endif

/*
 * Align the resource pointer
 */
static void *ResToUser(struct resource_header *r)
{
    br_int_32 align;

    /*
     * Find the required alignment
     */
    ASSERT(fw.resource_class_index[r->class] != NULL);

    align = fw.resource_class_index[r->class]->alignment;

    if(align <= 0)
        align = BR_RES_DEFAULT_ALIGN;

    /*
     * Check for no-zero and power of 2
     */
    ASSERT(align && (((align - 1) & align) == 0));

    /*
     * Move over the resource_header
     */
    r++;

    /*
     * Bump pointer up to next boundary
     */
    return (void *)ALIGN(r, align);
}

/*
 * Unalign the resource pointer
 */
static struct resource_header *UserToRes(void *r)
{
    br_uint_8 *p = r;

    /*
     * the last byte of the resource will always be non-zero -
     * the class id, or the magic number
     *
     * Move pointer back until we hit it
     */
    while(*(p - 1) == 0)
        --p;

        /* Account for any excess structure padding the compiler's added. */
#if BR_RES_TAGGING
    p -= offsetof(struct resource_header, magic_num) + sizeof(((struct resource_header *)NULL)->magic_num);
#else
    p -= offsetof(struct resource_header, class) + sizeof(((struct resource_header *)NULL)->class);
#endif

    struct resource_header *res = (struct resource_header *)p;
#if BR_RES_TAGGING
    UASSERT(res->magic_ptr == res);
    UASSERT(res->magic_num == BR_RES_MAGIC);
#endif
    return res;
}

/*
 * Create a new resource block of the given class, with 'size' bytes associated
 * with it.
 *
 * If parent is not NULL it adds the new resource as a child
 *
 * Returns a pointer to the first byte of the resource data
 */
void *BR_RESIDENT_ENTRY BrResAllocate(void *vparent, br_size_t size, br_uint_8 res_class)
{
    struct resource_header *res;
    struct resource_header *parent;
    br_int_32               malign, calign, pad;
    br_ptrdiff_t            actual_pad;

    UASSERT(fw.resource_class_index[res_class] != NULL);

    /*
     * Work out size (rounded up to granularity)
     */
    size = ALIGN(size + sizeof(struct resource_header), BR_RES_GRANULARITY);

    /*
     * Get memory alignment
     */
    malign = BrMemAlign(res_class);
    ASSERT(malign && (((malign - 1) & malign) == 0));

    /*
     * Get class alignment
     */
    calign = fw.resource_class_index[res_class]->alignment;

    if(calign <= 0)
        calign = BR_RES_DEFAULT_ALIGN;

    ASSERT(calign && (((calign - 1) & calign) == 0));

    /*
     * Work out how much padding to add at end of block
     */
    pad = (calign - 1) & ~(malign - 1);
    res = BrMemAllocate(size + pad, res_class);

    /*
     * Since loads of BrResAllocate calls check for NULL returns we
     * might as well return NULL sometimes!
     */
    if(res == NULL)
        return NULL;

    /*
     * Check to see if memory allocator lied about alignment
     */
    actual_pad = (ALIGN(res, calign) - ((br_uintptr_t)res));
    ASSERT(actual_pad <= pad);

    if(actual_pad > pad)
        BR_FAILURE0("Memory allocator broke alignment");

    res->class = res_class;
    RES_SIZE_SET(res, size);
    BrSimpleNewList(&res->children);

#if BR_RES_TAGGING
    res->magic_num = BR_RES_MAGIC;
    res->magic_ptr = res;
#endif

    if(vparent) {
        parent = UserToRes(vparent);
        BR_SIMPLEADDHEAD(&parent->children, res);
    }

    /*
     * Return pointer to used data
     */
    return ResToUser(res);
}

/*
 * Release a resource block - first calls BrResFree() for any child resources
 *
 * Removes resource from any parent list
 *
 * If the resource class has a destructor, that function is called
 */
static void BrResInternalFree(struct resource_header *res, br_boolean callback)
{
    int   c;
    void *r;

    UASSERT(ISRESOURCE(res));

    /*
     * Ignore free requests on things that are already being freed
     */
    if(res->class == BR_MEMORY_FREE)
        return;

    UASSERT(fw.resource_class_index[res->class] != NULL);

    /*
     * Remember class ID and mark object as being freed
     */
    c          = res->class;
    r          = ResToUser(res);
    res->class = BR_MEMORY_FREE;

    if(callback) {
        /*
         * Call class destructor
         */
        if(fw.resource_class_index[c]->free_cb)
            fw.resource_class_index[c]->free_cb(r, (br_uint_8)c, RES_SIZE_GET(res));
    }

    /*
     * Free any children
     */
    while(BR_SIMPLEHEAD(&res->children))
        BrResInternalFree((struct resource_header *)BR_SIMPLEREMOVE(BR_SIMPLEHEAD(&res->children)), BR_TRUE);

    /*
     * Remove from any parent list
     */
    if(BR_SIMPLEINSERTED(res))
        BR_SIMPLEREMOVE(res);

#if BR_RES_TAGGING
    /*
     * Make sure memory is no longer tagged as a resource
     */
    res->magic_num = 1;
    res->magic_ptr = NULL;
#endif

    /*
     * Release block
     */
    BrMemFree(res);
}

/*
 * Public entry for ResFree
 */
void BR_RESIDENT_ENTRY BrResFree(void *vres)
{
    UASSERT(vres != NULL);

    BrResInternalFree(UserToRes(vres), BR_TRUE);
}

void BR_RESIDENT_ENTRY BrResFreeNoCallback(void *vres)
{
    UASSERT(vres != NULL);

    BrResInternalFree(UserToRes(vres), BR_FALSE);
}

/*
 * Add a resource as a child of another
 */
void *BR_RESIDENT_ENTRY BrResAdd(void *vparent, void *vres)
{
    struct resource_header *res    = UserToRes(vres);
    struct resource_header *parent = UserToRes(vparent);

    UASSERT(vres != NULL);
    UASSERT(vparent != NULL);

    UASSERT(ISRESOURCE(res));
    UASSERT(ISRESOURCE(parent));

    /*
     * Remove from any parent list
     */
    if(BR_SIMPLEINSERTED(res))
        BR_SIMPLEREMOVE(res);

    BR_SIMPLEADDHEAD(&parent->children, res);

    return vres;
}

/*
 * Remove resource from parent
 */
void *BR_RESIDENT_ENTRY BrResRemove(void *vres)
{
    struct resource_header *res = UserToRes(vres);

    UASSERT(vres != NULL);

    UASSERT(ISRESOURCE(res));

    BR_SIMPLEREMOVE(res);

    return vres;
}

/*
 * Return the class of a given resource
 */
br_uint_8 BR_RESIDENT_ENTRY BrResClass(void *vres)
{
    struct resource_header *res = UserToRes(vres);

    UASSERT(vres != NULL);

    UASSERT(ISRESOURCE(res));

    return res->class;
}

#if 0
/*
 * Return the parent of a given resource
 */
void * BR_RESIDENT_ENTRY BrResParent(void * vres)
{
	struct resource_header *res = UserToRes(vres);

	UASSERT(vres != NULL);

	UASSERT(ISRESOURCE(res));

	return  res->parent?ResToUser(res->parent):NULL;
}
#endif

/*
 * Return the parent of a given resource
 */
br_boolean BR_RESIDENT_ENTRY BrResIsChild(void *vparent, void *vchild)
{
    struct resource_header *parent = UserToRes(vparent);
    struct resource_header *child  = UserToRes(vchild);
    struct resource_header *cp;

    UASSERT(vparent != NULL);
    UASSERT(vchild != NULL);

    UASSERT(ISRESOURCE(parent));
    UASSERT(ISRESOURCE(child));

    BR_FOR_SIMPLELIST(&parent->children, cp)
        if(cp == child)
            return BR_TRUE;

    return BR_FALSE;
}

/*
 * Return the size of a given resource
 */
br_size_t BR_RESIDENT_ENTRY BrResSize(void *vres)
{
    struct resource_header *res = UserToRes(vres);

    UASSERT(vres != NULL);
    UASSERT(ISRESOURCE(res));

    return RES_SIZE_GET(res) - sizeof(struct resource_header);
}

/*
 * Return the size of a resource, plus that of all it's children
 */

static br_size_t BR_CALLBACK ResSizeTotal(void *vres, void *ptr)
{
    /*
     * Accumulate this size...
     */
    *(br_size_t *)ptr += BrResSize(vres);

    /*
     * ...then add the sizes of all the children
     */
    BrResChildEnum(vres, ResSizeTotal, ptr);

    return 0;
}

br_size_t BR_RESIDENT_ENTRY BrResSizeTotal(void *vres)
{
    br_size_t total = 0;

    ResSizeTotal(vres, &total);

    return total;
}

/*
 * Invoke a callback for each of the children of a resource
 */
br_size_t BR_RESIDENT_ENTRY BrResChildEnum(void *vres, br_resenum_cbfn *callback, void *arg)
{
    struct resource_header *res = UserToRes(vres);
    struct resource_header *rp;
    br_size_t               r;

    UASSERT(vres != NULL);
    UASSERT(ISRESOURCE(res));
    ASSERT(callback != NULL);

    BR_FOR_SIMPLELIST(&res->children, rp)
        if((r = callback(ResToUser(rp), arg)))
            return r;

    return 0;
}

/*
 * If tagging is enabled, then returns true if *vres is a resource
 * block, otherwise returns no_tag
 */
br_uint_32 BR_RESIDENT_ENTRY BrResCheck(void *vres, int no_tag)
{
#if BR_RES_TAGGING
    struct resource_header *res = UserToRes(vres);

    return (res->magic_ptr == res) && (res->magic_num == BR_RES_MAGIC);
#else
    return no_tag;
#endif
}

/*
 * strdup() equivalent
 */
char *BR_RESIDENT_ENTRY BrResStrDup(void *vparent, const char *str)
{
    br_size_t l;
    char     *nstr;

    UASSERT(str != NULL);

    l = BrStrLen(str);

    nstr = BrResAllocate(vparent, l + 1, BR_MEMORY_STRING);

    BrStrCpy(nstr, str);

    return nstr;
}

char *BR_RESIDENT_ENTRY BrResVSprintf(void *vparent, const char *fmt, va_list ap)
{
    char   *s;
    int     nreq;
    va_list ap2;

    va_copy(ap2, ap);
    nreq = vsnprintf(NULL, 0, fmt, ap2);
    va_end(ap2);

    if(nreq < 0)
        return NULL;

    ++nreq;
    if((s = BrResAllocate(vparent, (size_t)nreq, BR_MEMORY_STRING)) == NULL)
        return NULL;

    if(vsnprintf(s, (size_t)nreq, fmt, ap) != nreq - 1) {
        BrResFree(s);
        return NULL;
    }

    return s;
}

char *BR_RESIDENT_ENTRY BrResSprintf(void *vparent, const char *fmt, ...)
{
    char   *s;
    va_list ap;

    va_start(ap, fmt);
    s = BrResVSprintf(vparent, fmt, ap);
    va_end(ap);

    return s;
}

#if DEBUG
/*
 * Debug function that will dump a resource hierachy
 *
 * Invokes a callback with each text line of the dump
 */
static void InternalResourceDump(struct resource_header *res, br_putline_cbfn *putline, void *arg, int level)
{
    int                     i;
    char                   *cp = BrScratchString();
    struct resource_header *child;
    br_resource_class      *rclass;

    /*
     * Blank out first bit of line
     */
    for(i = 0; i < level * 2; i++)
        *cp++ = ' ';

    /*
     * Describe resource
     */
    rclass = fw.resource_class_index[(unsigned char)res->class];

    if(rclass == NULL) {
        BrSprintf(cp, "0x%" PRIxPTR " ??? (%02X)", (br_uintptr_t)res + 1, res->class);
        putline(BrScratchString(), arg);
        return;
    }

    BrSprintf(cp, "0x%" PRIxPTR " %-20s %zu", (br_uintptr_t)ResToUser(res), rclass->identifier, RES_SIZE_GET(res));
    putline(BrScratchString(), arg);

    /*
     * Display any children
     */
    BR_FOR_SIMPLELIST(&res->children, child)
        InternalResourceDump(child, putline, arg, level + 1);
}
#endif

void BR_RESIDENT_ENTRY BrResDump(void *vres, br_putline_cbfn *putline, void *arg)
{
#if DEBUG
    struct resource_header *res = UserToRes(vres);

    InternalResourceDump(res, putline, arg, 0);
#endif
}

/*
 * Return the name for a resource class
 */
const char *BR_RESIDENT_ENTRY BrResClassIdentifier(br_uint_8 res_class)
{
    br_resource_class *rclass;

    rclass = fw.resource_class_index[res_class];

    return rclass ? rclass->identifier : "<NULL>";
}
