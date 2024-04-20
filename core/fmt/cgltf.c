#include <brender.h>

static void *cgltf_malloc_(size_t size)
{
    (void)size;
    return NULL;
}

static void cgltf_free_(void *p)
{
    (void)p;
}

#define CGLTF_MALLOC cgltf_malloc_
#define CGLTF_FREE   cgltf_free_
#define CGLTF_ATOI   BrAToI
#define CGLTF_ATOF   BrAToF
#define CGLTF_ATOLL  BrAToLL

#define CGLTF_IMPLEMENTATION
#define CGLTF_WRITE_IMPLEMENTATION
#include "cgltf_write.h"
