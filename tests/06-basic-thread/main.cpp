#include "testkernel.h"

#include <mutex>
#include <string>
#include <thread>
#include <exception>

int main(void)
{
    return CTestKernel::RunTests("06-basic-thread");
}

TEST_CASE("Basic thread operations")
{
    std::mutex m;
    int counter = 0;

    auto thread_func = [&m, &counter]()
    {
        for (int i = 0; i < 100; ++i)
        {
            std::lock_guard<std::mutex> lock(m);
            counter++;
        }
    };

    MESSAGE("Creating threads");
    std::thread t1(thread_func);
    std::thread t2(thread_func);

    MESSAGE("Joining threads");
    t1.join();
    t2.join();

    REQUIRE(counter == 200);
    MESSAGE("Thread test successful");
}

TEST_CASE("std::call_once example")
{
    std::once_flag flag1, flag2;
    int simple_called = 0;
    int may_throw_called = 0;
    int may_throw_did_not_throw = 0;

    auto simple_do_once = [&flag1, &simple_called]()
    {
        std::call_once(flag1, [&simple_called](){
            MESSAGE("Simple example: called once");
            simple_called++;
        });
    };

    auto may_throw_function = [&may_throw_called, &may_throw_did_not_throw](bool do_throw)
    {
        may_throw_called++;
        if (do_throw)
        {
            MESSAGE("Throw: call_once will retry");
            throw std::exception();
        }
        MESSAGE("Did not throw, call_once will not attempt again");
        may_throw_did_not_throw++;
    };

    auto do_once = [&flag2, &may_throw_function](bool do_throw)
    {
        try
        {
            std::call_once(flag2, may_throw_function, do_throw);
        }
        catch (...)
        {
        }
    };

    MESSAGE("Testing simple_do_once with 4 threads");
    std::thread st1(simple_do_once);
    std::thread st2(simple_do_once);
    std::thread st3(simple_do_once);
    std::thread st4(simple_do_once);
    st1.join();
    st2.join();
    st3.join();
    st4.join();

    REQUIRE(simple_called == 1);

    MESSAGE("Testing may_throw_function with 4 threads");
    std::thread t1(do_once, true);
    std::thread t2(do_once, true);
    std::thread t3(do_once, false);
    std::thread t4(do_once, true);
    t1.join();
    t2.join();
    t3.join();
    t4.join();

    REQUIRE(may_throw_did_not_throw == 1);
    REQUIRE(may_throw_called >= 1);
}
