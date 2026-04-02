//
// thread.cpp
//
// libc++ external threading — thread, TLS, and execute-once implementation
// for the Circle bare-metal framework.
//

#include <__external_threading>
#include <circle/sched/scheduler.h>
#include <circle/sched/task.h>
#include <new>

namespace std
{

// ---------------------------------------------------------------------------
// Thread-local storage state
// ---------------------------------------------------------------------------
unsigned constexpr MAX_TLS_KEYS = 32;

static void (*s_destructors[MAX_TLS_KEYS])(void *) = {};
static unsigned s_next_key = 0;

// ---------------------------------------------------------------------------
// CLibCXXTask — CTask subclass that runs a std::thread entry function
//
// __libcpp_thread_t::__opaque points at a heap-allocated JoinHandle (below),
// NOT at the CTask directly.  This keeps join() and detach() safe after the
// scheduler has already deleted the CTask via its unconditional
// `delete pTask` in GetNextTask().
//
// JoinHandle ownership rules:
//   - join()  : reads task (may be nullptr), then deletes the handle.
//   - detach(): sets detached = true; if task is already nullptr, deletes
//               handle immediately; otherwise ~CLibCXXTask() will delete it.
//   - ~CLibCXXTask(): always nulls task; deletes handle when detached or
//   joined.
// ---------------------------------------------------------------------------

class CLibCXXTask;

struct JoinHandle
{
    CLibCXXTask *task;
    uintptr_t id; // Stable cached thread ID
    bool detached;
    bool joined;
};

class CLibCXXTask : public CTask
{
public:
    CLibCXXTask(void *(*func)(void *), void *arg, JoinHandle *handle)
        : CTask(TASK_STACK_SIZE), m_func(func), m_arg(arg), m_handle(handle)
    {
    }

    ~CLibCXXTask() override
    {
        m_handle->task = nullptr;
        if (m_handle->detached || m_handle->joined)
            delete m_handle;
    }

    void Run() override
    {
        m_func(m_arg);

        void **const slots =
            static_cast<void **>(GetUserData(TASK_USER_DATA_USER));
        if (slots)
        {
            int constexpr PTHREAD_DESTRUCTOR_ITERATIONS = 4;
            bool destructors_called = true;
            for (int i = 0;
                 i < PTHREAD_DESTRUCTOR_ITERATIONS && destructors_called; ++i)
            {
                destructors_called = false;
                for (unsigned k = 0; k < s_next_key; ++k)
                {
                    if (slots[k] && s_destructors[k])
                    {
                        void *const val = slots[k];
                        slots[k] = nullptr;
                        s_destructors[k](val);
                        destructors_called = true;
                    }
                }
            }
            delete[] slots;
            SetUserData(nullptr, TASK_USER_DATA_USER);
        }
    }

private:
    void *(*m_func)(void *);
    void *m_arg;
    JoinHandle *m_handle;
};

// ---------------------------------------------------------------------------
// Thread
// ---------------------------------------------------------------------------

int __libcpp_thread_create(__libcpp_thread_t *__t, void *(*__func)(void *),
                           void *__arg)
{
    JoinHandle *const h = new JoinHandle{nullptr, 0, false, false};
    CLibCXXTask *const task = new CLibCXXTask(__func, __arg, h);
    h->task = task;
    h->id = reinterpret_cast<uintptr_t>(task);
    __t->__opaque = h;
    return 0;
}

__libcpp_thread_id __libcpp_thread_get_current_id()
{
    return reinterpret_cast<uintptr_t>(CScheduler::Get()->GetCurrentTask());
}

__libcpp_thread_id __libcpp_thread_get_id(__libcpp_thread_t const *__t)
{
    JoinHandle const *const h = static_cast<JoinHandle const *>(__t->__opaque);
    return h->id;
}

int __libcpp_thread_join(__libcpp_thread_t *__t)
{
    JoinHandle *const h = static_cast<JoinHandle *>(__t->__opaque);
    if (h->task == nullptr)
    {
        delete h;
    }
    else
    {
        h->joined = true;
        h->task->WaitForTermination();
    }
    __t->__opaque = nullptr;
    return 0;
}

int __libcpp_thread_detach(__libcpp_thread_t *__t)
{
    JoinHandle *const h = static_cast<JoinHandle *>(__t->__opaque);
    h->detached = true;
    if (h->task == nullptr)
        delete h;
    // else ~CLibCXXTask() will delete h when the task eventually terminates
    __t->__opaque = nullptr;
    return 0;
}

void __libcpp_thread_yield()
{
    CScheduler::Get()->Yield();
}

void __libcpp_thread_sleep_for(chrono::nanoseconds const &__ns)
{
    long long const us = __ns.count() / 1000LL;
    if (us > 0)
        CScheduler::Get()->usSleep(static_cast<unsigned>(us));
    else
        CScheduler::Get()->Yield();
}

// ---------------------------------------------------------------------------
// Execute once
//
// Three-state flag: 0 = not started, 1 = in progress, 2 = done.
//
// If __init_routine() yields internally (e.g. via memory allocation or any
// Circle API that calls Yield()), concurrent callers spin-yield in the inner
// loop until the flag leaves state 1.
//
// If __init_routine() throws (exceptional call per the C++ spec), the flag is
// reset to 0 and the exception propagates to the caller.  The outer loop
// ensures that any task that exits the inner spin because the flag dropped
// back to 0 retries from the top rather than returning success with an
// uninitialised state.
// ---------------------------------------------------------------------------

int __libcpp_execute_once(__libcpp_exec_once_flag *__flag,
                          void (*__init_routine)())
{
    while (true)
    {
        if (*__flag == 2)
            return 0;

        if (*__flag == 0)
        {
            *__flag = 1;
            try
            {
                __init_routine();
            }
            catch (...)
            {
                *__flag = 0; // allow a subsequent call to retry
                throw;
            }
            *__flag = 2;
            return 0;
        }

        // *__flag == 1: another task is initialising; spin-yield until it
        // either succeeds (flag → 2) or throws (flag → 0).
        while (*__flag == 1)
            CScheduler::Get()->Yield();
    }
}

// ---------------------------------------------------------------------------
// Thread-local storage
//
// Each task stores a void*[MAX_TLS_KEYS] array in TASK_USER_DATA_USER slot.
// A global table maps key index → destructor. Keys are assigned with a
// monotonic counter.
// ---------------------------------------------------------------------------

int __libcpp_tls_create(__libcpp_tls_key *__key, void (*__at_exit)(void *))
{
    unsigned const k = s_next_key++;
    s_destructors[k] = __at_exit;
    *__key = k;
    return 0;
}

void *__libcpp_tls_get(__libcpp_tls_key __key)
{
    void **const slots = static_cast<void **>(
        CScheduler::Get()->GetCurrentTask()->GetUserData(TASK_USER_DATA_USER));
    return slots ? slots[__key] : nullptr;
}

int __libcpp_tls_set(__libcpp_tls_key __key, void *__p)
{
    CTask *const task = CScheduler::Get()->GetCurrentTask();
    void **slots = static_cast<void **>(task->GetUserData(TASK_USER_DATA_USER));
    if (!slots)
    {
        slots = new void *[MAX_TLS_KEYS]();
        task->SetUserData(slots, TASK_USER_DATA_USER);
    }
    slots[__key] = __p;
    return 0;
}

} // namespace std
