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

namespace std {

// ---------------------------------------------------------------------------
// CLibCXXTask — CTask subclass that runs a std::thread entry function
// ---------------------------------------------------------------------------

class CLibCXXTask : public CTask {
public:
    CLibCXXTask(void *(*func)(void *), void *arg)
        : CTask(TASK_STACK_SIZE),
          m_func(func),
          m_arg(arg),
          m_result(nullptr),
          m_detached(false) {}

    void Run() override {
        m_result = m_func(m_arg);
    }

    void   *m_result;
    bool    m_detached;

private:
    void *(*m_func)(void *);
    void   *m_arg;
};

// ---------------------------------------------------------------------------
// Thread
// ---------------------------------------------------------------------------

int __libcpp_thread_create(__libcpp_thread_t *__t, void *(*__func)(void *), void *__arg) {
    __t->__opaque = new CLibCXXTask(__func, __arg);
    return 0;
}

__libcpp_thread_id __libcpp_thread_get_current_id() {
    return reinterpret_cast<uintptr_t>(CScheduler::Get()->GetCurrentTask());
}

__libcpp_thread_id __libcpp_thread_get_id(__libcpp_thread_t const *__t) {
    return reinterpret_cast<uintptr_t>(__t->__opaque);
}

int __libcpp_thread_join(__libcpp_thread_t *__t) {
    CLibCXXTask * const task = static_cast<CLibCXXTask *>(__t->__opaque);
    task->WaitForTermination();
    __t->__opaque = nullptr;
    return 0;
}

int __libcpp_thread_detach(__libcpp_thread_t *__t) {
    static_cast<CLibCXXTask *>(__t->__opaque)->m_detached = true;
    __t->__opaque = nullptr;
    return 0;
}

void __libcpp_thread_yield() {
    CScheduler::Get()->Yield();
}

void __libcpp_thread_sleep_for(chrono::nanoseconds const &__ns) {
    long long const us = __ns.count() / 1000LL;
    if (us > 0)
        CScheduler::Get()->usSleep(static_cast<unsigned>(us));
    else
        CScheduler::Get()->Yield();
}

// ---------------------------------------------------------------------------
// Execute once
//
// Safe without locking: the cooperative scheduler only switches tasks at
// explicit yield points, and __init_routine() runs without any yield.
// ---------------------------------------------------------------------------

int __libcpp_execute_once(__libcpp_exec_once_flag *__flag, void (*__init_routine)()) {
    if (*__flag == 0) {
        *__flag = 1;
        __init_routine();
    }
    return 0;
}

// ---------------------------------------------------------------------------
// Thread-local storage
//
// Each task stores a void*[MAX_TLS_KEYS] array in TASK_USER_DATA_USER slot.
// A global table maps key index → destructor. Keys are assigned with a
// monotonic counter.
// ---------------------------------------------------------------------------

static constexpr unsigned MAX_TLS_KEYS = 32;

static void (*s_destructors[MAX_TLS_KEYS])(void *) = {};
static unsigned s_next_key = 0;

int __libcpp_tls_create(__libcpp_tls_key *__key, void (*__at_exit)(void *)) {
    unsigned const k = s_next_key++;
    s_destructors[k] = __at_exit;
    *__key = k;
    return 0;
}

void *__libcpp_tls_get(__libcpp_tls_key __key) {
    void ** const slots = static_cast<void **>(
        CScheduler::Get()->GetCurrentTask()->GetUserData(TASK_USER_DATA_USER));
    return slots ? slots[__key] : nullptr;
}

int __libcpp_tls_set(__libcpp_tls_key __key, void *__p) {
    CTask * const task = CScheduler::Get()->GetCurrentTask();
    void **slots = static_cast<void **>(task->GetUserData(TASK_USER_DATA_USER));
    if (!slots) {
        slots = new void *[MAX_TLS_KEYS]();
        task->SetUserData(slots, TASK_USER_DATA_USER);
    }
    slots[__key] = __p;
    return 0;
}

} // namespace std
