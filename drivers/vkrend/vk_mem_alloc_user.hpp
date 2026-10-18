#ifndef VK_MEM_ALLOC_USER_H_
#define VK_MEM_ALLOC_USER_H_

#include <cassert>
#include <algorithm>
#include <mutex>

#if defined(__DJGPP__) && defined(__cplusplus)

#include <malloc.h>

static inline void *djgpp_aligned_alloc(size_t alignment, size_t size)
{
    if(alignment < sizeof(void *))
        alignment = sizeof(void *);

    return memalign(alignment, size);
}
#define aligned_alloc djgpp_aligned_alloc

class VmaMutexDOS
{
public:
    VmaMutexDOS()                              = default;
    VmaMutexDOS(const VmaMutexDOS&)            = delete;
    VmaMutexDOS& operator=(const VmaMutexDOS&) = delete;

    void Lock()
    {
    }

    void Unlock()
    {
    }

    bool TryLock()
    {
        return true;
    }
};

class VmaRWMutexDOS
{
public:
    VmaRWMutexDOS()                                = default;
    VmaRWMutexDOS(const VmaRWMutexDOS&)            = delete;
    VmaRWMutexDOS& operator=(const VmaRWMutexDOS&) = delete;

    void LockRead()
    {
    }

    void UnlockRead()
    {
    }

    bool TryLockRead()
    {
        return true;
    }

    void LockWrite()
    {
    }

    void UnlockWrite()
    {
    }

    bool TryLockWrite()
    {
        return true;
    }
};

#define VMA_MUTEX                 VmaMutexDOS
#define VMA_RW_MUTEX              VmaRWMutexDOS

#endif

#endif /* VK_MEM_ALLOC_USER_H_ */
