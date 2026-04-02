//
// mutex.cpp
//
// libc++ external threading — mutex implementation for the Circle bare-metal
// framework.
//

#include <__external_threading>
#include <circle/sched/mutex.h>
#include <new>

namespace std
{

static_assert(sizeof(CMutex) <= sizeof(__libcpp_mutex_t::__storage),
              "CMutex does not fit in __libcpp_mutex_t storage");
static_assert(alignof(CMutex) <= alignof(__libcpp_mutex_t),
              "CMutex alignment exceeds __libcpp_mutex_t alignment");

static CMutex *as_mutex(__libcpp_mutex_t *__m)
{
    return reinterpret_cast<CMutex *>(__m->__storage);
}

static CMutex const *as_mutex(__libcpp_mutex_t const *__m)
{
    return reinterpret_cast<CMutex const *>(__m->__storage);
}

static CMutex *as_mutex(__libcpp_recursive_mutex_t *__m)
{
    return reinterpret_cast<CMutex *>(__m->__storage);
}

// ---------------------------------------------------------------------------
// Non-recursive mutex
//
// Initialised via _LIBCPP_MUTEX_INITIALIZER (zero-init), which is a valid
// CMutex state. No explicit init function exists in the libc++ API for
// non-recursive mutexes.
// ---------------------------------------------------------------------------

int __libcpp_mutex_lock(__libcpp_mutex_t *__m)
{
    as_mutex(__m)->Acquire();
    return 0;
}

bool __libcpp_mutex_trylock(__libcpp_mutex_t *__m)
{
    return as_mutex(__m)->TryAcquire();
}

int __libcpp_mutex_unlock(__libcpp_mutex_t *__m)
{
    as_mutex(__m)->Release();
    return 0;
}

int __libcpp_mutex_destroy(__libcpp_mutex_t *__m)
{
    as_mutex(__m)->~CMutex();
    return 0;
}

// ---------------------------------------------------------------------------
// Recursive mutex
//
// Explicitly initialised via __libcpp_recursive_mutex_init() using
// placement new so the constructor runs properly.
// ---------------------------------------------------------------------------

int __libcpp_recursive_mutex_init(__libcpp_recursive_mutex_t *__m)
{
    new (__m->__storage) CMutex();
    return 0;
}

int __libcpp_recursive_mutex_lock(__libcpp_recursive_mutex_t *__m)
{
    as_mutex(__m)->Acquire();
    return 0;
}

bool __libcpp_recursive_mutex_trylock(__libcpp_recursive_mutex_t *__m)
{
    return as_mutex(__m)->TryAcquire();
}

int __libcpp_recursive_mutex_unlock(__libcpp_recursive_mutex_t *__m)
{
    as_mutex(__m)->Release();
    return 0;
}

int __libcpp_recursive_mutex_destroy(__libcpp_recursive_mutex_t *__m)
{
    as_mutex(__m)->~CMutex();
    return 0;
}

} // namespace std
