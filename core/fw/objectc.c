/*
 * Copyright (c) 1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: objectc.c 1.3 1998/07/15 18:32:32 johng Exp $
 * $Locker: $
 *
 * Some generic methods for object container list management
 */
#include "fw.h"
#include "brassert.h"

/*
 * Parts that lists are constructed from
 */
struct object_list {
    br_simple_list l;
};

struct object_list_entry {
    br_simple_node n;
    br_object     *h;
};

/*
 * Exported function to allocate the basic anchor structure
 */
void *BR_RESIDENT_ENTRY BrObjectListAllocate(void *res)
{
    struct object_list *hl;

    hl = BrResAllocate(res, sizeof(struct object_list), BR_MEMORY_OBJECT_LIST);

    BrSimpleNewList(&hl->l);

    return hl;
}

/*
 * Add a new object to the front of the list
 */
br_error BR_CMETHOD_DECL(br_object_container, addFront)(struct br_object_container *self, struct br_object *ph)
{
    struct object_list       *hl;
    struct object_list_entry *he;

    hl = ObjectContainerListQuery(self);
    if(hl == NULL)
        return BRE_FAIL;

    /*
     * Allocate a new list entry and add it to the list
     */
    he = BrResAllocate(hl, sizeof(struct object_list_entry), BR_MEMORY_OBJECT_LIST_ENTRY);

    if(he == NULL)
        return BRE_NO_MEMORY;

    he->h = ph;
    BR_SIMPLEADDHEAD(&hl->l, he);

    return BRE_OK;
}

/*
 * Remove a particular object from the list
 */
br_error BR_CMETHOD_DECL(br_object_container, remove)(struct br_object_container *self, struct br_object *h)
{
    struct object_list       *hl;
    struct object_list_entry *he;

    hl = ObjectContainerListQuery(self);
    if(hl == NULL)
        return BRE_FAIL;

    /*
     * Find object on list
     */
    BR_FOR_SIMPLELIST(&hl->l, he)
        if(he->h == h)
            break;

    if(!(he && he->h == h))
        return BRE_FAIL;

    /*
     * Take object off list and free anchor block
     */
    BR_SIMPLEREMOVE(he);

    BrResFree(he);

    return BRE_OK;
}

/*
 * Remove front element from list
 */
br_error BR_CMETHOD_DECL(br_object_container, removeFront)(struct br_object_container *self, struct br_object **ph)
{
    struct object_list       *hl;
    struct object_list_entry *he;

    hl = ObjectContainerListQuery(self);
    if(hl == NULL)
        return BRE_FAIL;

    /*
     * Take object of list and free it
     */
    he = BR_SIMPLEREMHEAD(&hl->l);

    if(he == NULL) {
        *ph = NULL;
        return BRE_FAIL;
    }

    /*
     * Return object pointer to caller
     */
    *ph = he->h;
    BrResFree(he);

    return BRE_OK;
}

/*
 * Search list for tokens that:
 * 	a)	have a type (al tokens will match BR_NULL_TOKEN)
 * 	b)	have name mathing a pattern ( NULL will match all names)
 *  c)  have properties matching a given token list (NULL will match all objects0
 *
 *
 * How the token list is interpreted is controlled by three other methods:
 *
 *	void * tokensMatchBegin(br_token type, br_tv_template *tv)
 * 		Process the given token list for later matching - the void * pointer
 *		will me passed to the match routine.
 *
 *  br_boolean tokensMatch(br_object *h, void * tvarg)
 *		See if a object matches (tvargs is void * from above)
 *		Returns true if object does match
 *
 *	void tokensMatchEnd(void *tvarg)
 *		Perform any cleaning up.
 */
br_error BR_CMETHOD_DECL(br_object_container, find)(br_object_container *self, br_object **ph, br_token type, const char *pattern,
                                                    br_token_value *tv)
{
    struct object_list       *hl;
    struct object_list_entry *he;
    void                     *tvarg;
    br_error                  r = BRE_FAIL;

    hl = ObjectContainerListQuery(self);
    if(hl == NULL)
        return BRE_FAIL;

    if(tv)
        tvarg = ObjectContainerTokensMatchBegin(self, type, tv);

    /*
     * Find first matching object in list
     */
    BR_FOR_SIMPLELIST(&hl->l, he) {
        if((type == BR_NULL_TOKEN || type == ObjectType(he->h)) && BrNamePatternMatch(pattern, ObjectIdentifier(he->h))) {

            if(tv && !ObjectContainerTokensMatch(self, he->h, tvarg))
                continue;

            *ph = he->h;
            r   = BRE_OK;
        }
    }

    if(tv)
        ObjectContainerTokensMatchEnd(self, tvarg);

    return r;
}

/*
 * Find all objects that match above criteria (and that fit into destination buffer)
 */
br_error BR_CMETHOD_DECL(br_object_container, findMany)(br_object_container *self, br_object **objects, br_int_32 max_objects,
                                                        br_int_32 *num_objects, br_token type, const char *pattern, br_token_value *tv)
{
    struct object_list       *hl;
    struct object_list_entry *he;
    void                     *tvarg;
    int                       n = 0;

    hl = ObjectContainerListQuery(self);
    if(hl == NULL)
        return BRE_FAIL;

    if(tv)
        tvarg = ObjectContainerTokensMatchBegin(self, type, tv);

    /*
     * Find all matching objects in list
     */
    BR_FOR_SIMPLELIST(&hl->l, he) {
        /*
         * If object matches, add to table
         */
        if((type == BR_NULL_TOKEN || type == ObjectType(he->h)) && BrNamePatternMatch(pattern, ObjectIdentifier(he->h))) {

            if(tv != NULL && ObjectContainerTokensMatch(self, he->h, tvarg) == BR_FALSE)
                continue;

            /*
             * Save pointer if there is space in output table
             */
            if(n < max_objects)
                *objects++ = he->h;

            n++;
        }
    }

    if(tv)
        ObjectContainerTokensMatchEnd(self, tvarg);

    *num_objects = n;

    if(n <= max_objects)
        return BRE_OK;
    else
        return BRE_OVERFLOW;
}

/*
 * Count number of matching objects
 */
br_error BR_CMETHOD_DECL(br_object_container, count)(br_object_container *self, br_int_32 *pcount, br_token type, const char *pattern,
                                                     br_token_value *tv)
{
    struct object_list       *hl;
    struct object_list_entry *he;
    void                     *tvarg;
    int                       n;

    hl = ObjectContainerListQuery(self);
    if(hl == NULL)
        return BRE_FAIL;

    if(tv)
        tvarg = ObjectContainerTokensMatchBegin(self, type, tv);

    /*
     * Count all matching objects
     */
    n = 0;

    BR_FOR_SIMPLELIST(&hl->l, he) {
        if((type == BR_NULL_TOKEN || type == ObjectType(he->h)) && BrNamePatternMatch(pattern, ObjectIdentifier(he->h))) {

            if(tv != NULL && ObjectContainerTokensMatch(self, he->h, tvarg) == BR_FALSE)
                continue;

            n++;
        }
    }

    if(tv)
        ObjectContainerTokensMatchEnd(self, tvarg);

    *pcount = n;

    return BRE_OK;
}

/*
 * Default token matching does nothing other than make all tokens match
 *
 * makes a copy of token/value list
 */
struct token_match {
    br_token_value *original;
    br_token_value *query;
    br_token        type;
    br_int_32       n;
    void           *extra;
    br_size_t       extra_size;
};

void *BR_CMETHOD_DECL(br_object_container, tokensMatchBegin)(struct br_object_container *self, br_token t, br_token_value *tv)
{
    struct token_match *tm;
    br_int_32           i;

    (void)self;

    if(tv == NULL)
        return NULL;

    tm           = BrResAllocate(fw.res, sizeof(*tm), BR_MEMORY_APPLICATION);
    tm->original = tv;
    tm->type     = t;

    for(i = 0; tv[i].t != BR_NULL_TOKEN; i++)
        ;

    tm->n     = i + 1;
    tm->query = BrResAllocate(tm, tm->n * sizeof(br_token_value), BR_MEMORY_APPLICATION);
    BrMemCpy(tm->query, tv, i * sizeof(br_token_value));
    return (void *)tm;
}

br_boolean BR_CMETHOD_DECL(br_object_container, tokensMatch)(struct br_object_container *self, br_object *h, void *arg)
{
    struct token_match     *tm = arg;
    const br_tv_match_info *info;
    br_size_t               s;
    br_int_32               n;

    if(arg == NULL)
        return BR_TRUE;

    /*
     * Make a query on the object and then compare with the original tokens
     */
    ObjectQueryManySize(h, &s, tm->query);

    if(s > tm->extra_size) {
        if(tm->extra)
            BrResFree(tm->extra);
        tm->extra      = BrResAllocate(tm, s, BR_MEMORY_APPLICATION);
        tm->extra_size = s;
    }

    ObjectQueryMany(h, tm->query, tm->extra, tm->extra_size, &n);

    /*
     * Ensure that all tokens were found
     */
    if(tm->query[n].t != BR_NULL_TOKEN)
        return BR_FALSE;

    /*
     * Check if we've got extra match requirements.
     */
    info = ObjectContainerTokensMatchInfoQuery(self);
    for(const br_tv_match_info *i = info; i->type != BR_NULL_TOKEN; ++i) {
        if(tm->type == i->type)
            return BrTokenValueComparePartial(tm->original, tm->query, i->insignificant);
    }

    /*
     * Compare the two token lists
     */
    return BrTokenValueCompare(tm->original, tm->query);
}

void BR_CMETHOD_DECL(br_object_container, tokensMatchEnd)(struct br_object_container *self, void *arg)
{
    (void)self;

    if(arg)
        BrResFree(arg);
}

const br_tv_match_info *BR_CMETHOD_DECL(br_object_container, tokensMatchInfoQuery)(struct br_object_container *self)
{
    static const br_tv_match_info matchinfo[] = {{.type = BR_NULL_TOKEN}};

    (void)self;
    return matchinfo;
}

/*
 * Utility function to free all the objects in a container
 */
br_error BR_RESIDENT_ENTRY BrObjectContainerFree(br_object_container *self, br_token type, char *pattern, br_token_value *tv)
{
    br_error    r;
    br_object **handles;
    br_int_32   count, n, i;

    r = ObjectContainerCount(self, &count, type, pattern, tv);

    if(r != BRE_OK)
        return r;

    if(count == 0)
        return BRE_OK;

    handles = BrMemAllocate(count * sizeof(br_object *), BR_MEMORY_DRIVER);

    r = ObjectContainerFindMany(self, handles, count, &n, type, pattern, tv);

    for(i = 0; i < n; i++) {
        ObjectContainerRemove(self, handles[i]);
        ObjectFree(handles[i]);
    }

    BrMemFree(handles);

    return BRE_OK;
}
