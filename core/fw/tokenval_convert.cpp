extern "C" {
#include "fw.h"
}

template <typename To, typename From>
using Mapper = To (*)(From);

/**
 * \brief Copy values onto the end of a buffer, whilst converting from {@From} to {@To}.
 *
 * The buffer pointer and remaining space are passed in by reference - and are
 * updated according to the size of the block
 *
 * \returns A pointer to start of destination block, or NULL if there was not enough space.
 */
template <typename To, typename From>
static To *ConvertTransform(To **pextra, const From *src, br_size_t count, br_size_t *pextra_space, Mapper<To, From> mapper) noexcept
{
    To *ret;

    if(pextra == nullptr || *pextra == nullptr || pextra_space == nullptr)
        return nullptr;

    ret = *pextra;

    /*
     * Check there is space for the data
     */
    if((count * sizeof(To)) > *pextra_space)
        return nullptr;

    *pextra_space -= count * sizeof(To);

    while(count--)
        *(*pextra)++ = mapper(*src++);

    return ret;
}

template <typename T>
static T TfIdentity(T val)
{
    return val;
}

static br_fixed_ls TfFloatToFixed(br_float val)
{
    return BrFloatToFixed(val);
}

static br_float TfFixedToFloat(br_fixed_ls val)
{
    return BrFixedToFloat(val);
}

extern "C" br_fixed_ls *BrTVConvertFloatToFixed(br_fixed_ls **pextra, const br_float *src, br_size_t count, br_size_t *pextra_space)
{
    return ConvertTransform(pextra, src, count, pextra_space, TfFloatToFixed);
}

extern "C" br_float *BrTVConvertFixedToFloat(br_float **pextra, const br_fixed_ls *src, br_size_t count, br_size_t *pextra_space)
{
    return ConvertTransform(pextra, src, count, pextra_space, TfFixedToFloat);
}

extern "C" br_uint_32 *BrTVConvertLongCopy(br_uint_32 **pextra, const br_uint_32 *src, br_size_t count, br_size_t *pextra_space)
{
    return ConvertTransform(pextra, src, count, pextra_space, TfIdentity);
}

extern "C" br_uintptr_t *BrTVConvertPtrCopy(br_uintptr_t **pextra, const br_uintptr_t *src, br_size_t count, br_size_t *pextra_space)
{
    return ConvertTransform(pextra, src, count, pextra_space, TfIdentity);
}
