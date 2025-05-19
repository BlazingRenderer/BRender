/*
 * Copyright (c) 2025 Zane van Iperen.
 *
 * SPDX-License-Identifier: MIT
 *
 * Hash Map.
 */
#ifndef _BRHASHMAP_H_
#define _BRHASHMAP_H_

typedef enum br_hashmap_resize_policy {
    BR_HASHMAP_RESIZE_LOAD_FACTOR = 0,
    BR_HASHMAP_RESIZE_NONE        = 1,
} br_hashmap_resize_policy;

typedef br_hash BR_CALLBACK    br_hashmap_hash_cbfn(const void *key);
typedef br_boolean BR_CALLBACK br_hashmap_compare_cbfn(const void *a, const void *b);
typedef int BR_CALLBACK        br_hashmap_enum_cbfn(const void *key, void *value, br_hash hash, void *user);

struct br_hashmap;
typedef struct br_hashmap br_hashmap;

#endif /* _BRHASHMAP_H_ */
