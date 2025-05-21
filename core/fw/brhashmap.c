/*
 * Copyright (c) 2025 Zane van Iperen.
 *
 * SPDX-License-Identifier: MIT
 *
 * Hash Map, based off https://github.com/vs49688/vsclib/blob/master/vsclib/hashmap.c
 */

#include "fw.h"
#include "brassert.h"

/* FIXME: expose */
#define BR_MAX(a, b)                          ((a) > (b) ? (a) : (b))

#define BR_HASHMAP_MIN_BUCKET_AUTO_ALLOCATION 16

typedef struct br_hashmap_bucket {
    /**
     * @brief Item hash. Is BR_INVALID_HASH if empty.
     */
    br_hash     hash;
    const void *key;
    void       *value;
} br_hashmap_bucket;

struct br_hashmap {
    br_size_t                size;
    br_size_t                num_buckets;
    br_hashmap_bucket       *buckets;
    br_hashmap_resize_policy resize_policy;
    struct {
        br_uint_16 num, den;
    } load_min;
    struct {
        br_uint_16 num, den;
    } load_max;
    br_hashmap_hash_cbfn    *hash_proc;
    br_hashmap_compare_cbfn *compare_proc;
};

/* This should be optimised out in Release builds. */
static inline void validate(const br_hashmap *hm)
{
    ASSERT(hm != NULL);
    ASSERT(hm->size <= hm->num_buckets);
    ASSERT(hm->hash_proc != NULL);
    ASSERT(hm->compare_proc != NULL);
    ASSERT(hm->load_min.den > 0);
    ASSERT(hm->load_min.num < hm->load_min.den);
    ASSERT(hm->load_max.den > 0);
    ASSERT(hm->load_max.num < hm->load_max.den);
    ASSERT(hm->load_min.num * hm->load_max.den <= hm->load_max.num * hm->load_max.den);
}

static inline br_hashmap_bucket *reset_bucket(br_hashmap_bucket *bkt)
{
    bkt->hash  = BR_INVALID_HASH;
    bkt->key   = NULL;
    bkt->value = NULL;
    return bkt;
}

br_hash BR_RESIDENT_ENTRY BrHashMapHash(const br_hashmap *hm, const void *key)
{
    br_hash hash = hm->hash_proc(key);
    ASSERT(hash != BR_INVALID_HASH);
    return hash;
}

br_boolean BR_RESIDENT_ENTRY BrHashMapCompare(const br_hashmap *hm, const void *a, const void *b)
{
    return hm->compare_proc(a, b);
}

br_hashmap *BR_RESIDENT_ENTRY BrHashMapAllocate(void *vparent, br_hashmap_hash_cbfn *hash, br_hashmap_compare_cbfn *compare)
{
    br_hashmap *hm;
    UASSERT(hash != NULL);
    UASSERT(compare != NULL);

    if((hm = BrResAllocate(vparent, sizeof(br_hashmap), BR_MEMORY_APPLICATION)) == NULL)
        return NULL;

    hm->size          = 0;
    hm->num_buckets   = 0;
    hm->buckets       = NULL;
    hm->resize_policy = BR_HASHMAP_RESIZE_LOAD_FACTOR;
    hm->load_min.num  = 1;
    hm->load_min.den  = 2;
    hm->load_max.num  = 3;
    hm->load_max.den  = 4;
    hm->hash_proc     = hash;
    hm->compare_proc  = compare;

    return hm;
}

void BR_RESIDENT_ENTRY BrHashMapFree(br_hashmap *hm)
{
    BrHashMapReset(hm);
    BrResFree(hm);
}

int BR_RESIDENT_ENTRY BrHashMapClear(br_hashmap *hm)
{
    validate(hm);
    hm->size = 0;
    for(size_t i = 0; i < hm->num_buckets; ++i)
        reset_bucket(hm->buckets + i);

    return 0;
}

void BR_RESIDENT_ENTRY BrHashMapSetResizePolicy(br_hashmap *hm, br_hashmap_resize_policy policy)
{
    validate(hm);

    hm->resize_policy = policy;
}

int BR_RESIDENT_ENTRY BrHashMapSetLoadFactor(br_hashmap *hm, uint16_t min_num, uint16_t min_den, uint16_t max_num, uint16_t max_den)
{
    uint32_t numa, numb;

    if(min_num == 0 || min_den == 0 || min_num >= min_den)
        return BRE_INVALID;

    if(max_num == 0 || max_den == 0 || max_num >= max_den)
        return BRE_INVALID;

    validate(hm);

    /*
     * Convert the fractions to the same base so we can
     * compare their numerators.
     *
     * a     c   a * d     c * b
     * - and - = ----- and -----
     * b     d   b * d     d * b
     */
    numa = min_num * max_den;
    numb = max_num * min_den;

    if(numa > numb)
        return BRE_INVALID;

    hm->load_min.num = min_num;
    hm->load_min.den = min_den;
    hm->load_max.num = max_num;
    hm->load_max.den = max_den;
    return 0;
}

void BR_RESIDENT_ENTRY BrHashMapReset(br_hashmap *hm)
{
    validate(hm);

    if(hm->buckets != NULL) {
        BrMemFree(hm->buckets);
    }

    hm->num_buckets = 0;
    hm->buckets     = NULL;
}

/*
 * Circularly loop over each of the buckets, starting at index `start`.
 */
#define LOOP_BUCKETS(hm, idxname, start) \
    for(size_t _i = 0, (idxname) = (start) % (hm)->num_buckets; _i < (hm)->num_buckets; ++_i, (idxname) = ((idxname) + 1) % (hm)->num_buckets)

static br_hashmap_bucket *add_or_replace_bucket(br_hashmap *hm, br_hashmap_bucket *buckets, size_t n, const br_hashmap_bucket *bkt, int *added)
{
    if(n == 0)
        return NULL;

    LOOP_BUCKETS(hm, index, bkt->hash % n)
    {
        br_hashmap_bucket *b = buckets + index;

        /* Shortcut: use first empty bucket. */
        if(b->hash == BR_INVALID_HASH) {
            *added = 1;
            *b     = *bkt;
            return b;
        }

        if(b->hash != bkt->hash)
            continue;

        /* Duplicate key, replace it. */
        if(BrHashMapCompare(hm, bkt->key, b->key)) {
            *added = 0;
            *b     = *bkt;
            return b;
        }
    }

    *added = 0;
    return NULL;
}

int BR_RESIDENT_ENTRY BrHashMapResize(br_hashmap *hm, size_t nelem)
{
    br_hashmap_bucket *bkts, *tmpbkts;

    validate(hm);

    if(nelem == 0 || nelem < hm->size)
        return BRE_INVALID;

    /* Nothing to do. */
    if(nelem == hm->size)
        return 0;

    /* Make sure the realloc() size won't overflow. */
    if(nelem >= (SIZE_MAX / sizeof(br_hashmap_bucket)))
        return BRE_RANGE;

    /* Reallocate the buckets. */
    bkts = BrMemReallocate(hm->buckets, sizeof(br_hashmap_bucket) * nelem, BR_MEMORY_APPLICATION);
    if(bkts == NULL)
        return BRE_NO_MEMORY;

    hm->buckets = bkts;

    for(size_t i = hm->num_buckets; i < nelem; ++i)
        reset_bucket(hm->buckets + i);

    /* Shortcut - no items. no problem! */
    if(hm->size == 0) {
        hm->num_buckets = nelem;
        return 0;
    }

    /*
     * Allocate a temp bucket list (hue) to work with. Note that this
     * needs to be done after the hm->buckets alloc to play nice to linear
     * allocators.
     */
    tmpbkts = BrMemCalloc(nelem, sizeof(br_hashmap_bucket), BR_MEMORY_APPLICATION);
    if(tmpbkts == NULL) {

        /*
         * This is a bit of a sticky situation - allocation has failed,
         * and now we have more buckets that we can't redistribute in to.
         * This also means it's not safe to rely on another realloc to shrink
         * it again.
         *
         * Our only safe option is to waste the new memory :(
         */
        return BRE_NO_MEMORY;
    }

    hm->num_buckets = nelem;

    for(size_t i = 0; i < nelem; ++i)
        reset_bucket(tmpbkts + i);

    /* Now redistribute everything. */
    for(size_t i = 0; i < nelem; ++i) {
        int                added;
        br_hashmap_bucket *bkt = hm->buckets + i;
        if(bkt->hash == BR_INVALID_HASH)
            continue;

        added = 0;
        bkt   = add_or_replace_bucket(hm, tmpbkts, nelem, hm->buckets + i, &added);
        ASSERT(bkt != NULL);
    }

    BrMemCpy(hm->buckets, tmpbkts, sizeof(br_hashmap_bucket) * nelem);
    BrMemFree(tmpbkts);

    // fprintf(stderr, "Resizing to %zu\n", hm->num_buckets);
    return 0;
}

static inline int intceil(size_t *result, size_t num, size_t den)
{
#if 1
    *result = (num / den) + (size_t)((num % den) != 0);
    return 0;
#else
    if(num >= SIZE_MAX - (den - 1))
        return BRE_RANGE;

    *result = (num + den - 1) / den;
    return 0;
#endif
}

static int maybe_resize(br_hashmap *hm)
{
    size_t thresh, minreq, tmp;
    int    r;

    /*
     * Make sure the threshold calculation doesn't overflow.
     *
     * If this is actively limiting your required size then either:
     * - resize it manually using BrHashMapResize(), or
     * - use a load factor where the numerator is 1.
     */
    if((r = intceil(&tmp, SIZE_MAX, hm->load_max.num)) < 0)
        return r;

    if(hm->num_buckets >= tmp)
        return BRE_RANGE; /* or EOVERFLOW? */

    /* Same as "n * load_factor" */
    if((r = intceil(&thresh, hm->num_buckets * hm->load_max.num, hm->load_max.den)) < 0)
        return r;

    /* Nothing to do */
    if(hm->size < thresh)
        return 0;

    /* Don't allow resize if explicitly disabled. */
    if(hm->resize_policy == BR_HASHMAP_RESIZE_NONE)
        return BRE_PERMISSION;

    if((r = intceil(&tmp, SIZE_MAX, hm->load_max.den)) < 0)
        return r;

    if(hm->size + 1 >= tmp)
        return BRE_RANGE; /* or EOVERFLOW? */

    /*
     * Determine the minimum number of buckets required to match
     * the target load factor, then double it.
     */
    if((r = intceil(&minreq, (hm->size + 1) * hm->load_min.den, hm->load_min.num)) < 0)
        return r;

    return BrHashMapResize(hm, BR_MAX(minreq, BR_HASHMAP_MIN_BUCKET_AUTO_ALLOCATION));
}

int BR_RESIDENT_ENTRY BrHashMapInsert(br_hashmap *hm, const void *key, void *value)
{
    br_hashmap_bucket tmpbkt;
    int               r;
    int               added;

    validate(hm);

    tmpbkt.hash  = BrHashMapHash(hm, key);
    tmpbkt.key   = key;
    tmpbkt.value = value;

    if(tmpbkt.hash == BR_INVALID_HASH)
        return BRE_RANGE;

    /* See if we need to resize. */
    if((r = maybe_resize(hm)) < 0) {
        /*
         * Resize required, but not allowed.
         * Not an error, assume the user knows what they're doing.
         */
        if(r != BRE_PERMISSION)
            return r;
    }

    added = 0;
    if(add_or_replace_bucket(hm, hm->buckets, hm->num_buckets, &tmpbkt, &added) == NULL)
        return BRE_NO_SPACE;

    if(added) {
        ++hm->size;
    }

    return 0;
}

void *BR_RESIDENT_ENTRY BrHashMapFindByHash(const br_hashmap *hm, br_hash hash)
{
    if(hash == BR_INVALID_HASH)
        return NULL;

    validate(hm);

    if(hm->num_buckets == 0)
        return NULL;

    LOOP_BUCKETS(hm, index, hash % hm->num_buckets)
    {
        const br_hashmap_bucket *bkt = hm->buckets + index;

        if(bkt->hash == BR_INVALID_HASH)
            break;

        if(bkt->hash == hash)
            return bkt->value;
    }

    return NULL;
}

static const br_hashmap_bucket *find_bucket(const br_hashmap *hm, const void *key, size_t *outindex)
{
    br_hash hash;

    validate(hm);

    hash = BrHashMapHash(hm, key);

    if(hm->num_buckets == 0)
        return NULL;

    /*
     * Search for the key starting at index until we:
     * 1. find it,
     * 2. hit an empty bucket (not found),
     * 3. do a complete loop  (not found)
     *    - This is O(n), but should almost never happen if your
     *      hash function is good enough and there's enough buckets.
     */
    LOOP_BUCKETS(hm, index, hash % hm->num_buckets)
    {
        const br_hashmap_bucket *bkt = hm->buckets + index;

        /* Stop at first empty bucket, item isn't here. */
        if(bkt->hash == BR_INVALID_HASH)
            break;

        if(bkt->hash != hash)
            continue;

        /* In case of hash collision. */
        if(!BrHashMapCompare(hm, key, bkt->key))
            continue;

        if(outindex != NULL)
            *outindex = index;
        return bkt;
    }

    return NULL;
}

void *BR_RESIDENT_ENTRY BrHashMapFind(const br_hashmap *hm, const void *key)
{
    const br_hashmap_bucket *bkt;

    if((bkt = find_bucket(hm, key, NULL)) == NULL)
        return NULL;

    return bkt->value;
}

void *BR_RESIDENT_ENTRY BrHashMapRemove(br_hashmap *hm, const void *key)
{
    br_hashmap_bucket *bkt;
    size_t             index;
    void              *val;

    bkt = (br_hashmap_bucket *)find_bucket(hm, key, &index);
    if(bkt == NULL)
        return NULL;

    val = bkt->value;
    reset_bucket(bkt);

    /* Search through the remaining buckets, to see if we can fill the gap. */
    LOOP_BUCKETS(hm, cidx, index + 1)
    {
        size_t             nidx;
        br_hashmap_bucket *bkt2 = hm->buckets + cidx;

        /* Have an empty bucket, we're done here! */
        if(bkt2->hash == BR_INVALID_HASH)
            break;

        /* If our hash can be moved back, do it. */
        nidx = bkt2->hash % hm->num_buckets;
        if(nidx <= index) {
            *bkt = *bkt2;
            reset_bucket(bkt2);
            index = cidx;
            bkt   = bkt2;
        }
    }

    --hm->size;
    return val;
}

br_size_t BR_RESIDENT_ENTRY BrHashMapSize(const br_hashmap *hm)
{
    validate(hm);
    return hm->size;
}

int BR_RESIDENT_ENTRY BrHashMapEnumerate(const br_hashmap *hm, br_hashmap_enum_cbfn proc, void *user)
{
    int r;

    validate(hm);

    for(size_t i = 0; i < hm->num_buckets; ++i) {
        const br_hashmap_bucket *bkt = hm->buckets + i;

        if(bkt->hash == BR_INVALID_HASH)
            continue;

        if((r = proc(bkt->key, bkt->value, bkt->hash, user)) != 0)
            return r;
    }

    return 0;
}

br_hash BR_RESIDENT_ENTRY BrHashMapDefaultHash(const void *k)
{
    return (br_hash)k;
}

int BR_RESIDENT_ENTRY BrHashMapDefaultCompare(const void *a, const void *b)
{
    return a == b;
}

br_hash BR_RESIDENT_ENTRY BrHashMapStringHash(const void *s)
{
    return BrHashString(s);
}

int BR_RESIDENT_ENTRY BrHashMapStringCompare(const void *a, const void *b)
{
    if(a == b)
        return 0;

    if(a == NULL || b == NULL)
        return 0;

    return BrStrCmp(a, b) == 0;
}
