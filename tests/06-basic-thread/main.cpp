#include "testkernel.h"

#include <mutex>
#include <string>
#include <thread>
#include <exception>
#include <condition_variable>

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

/*
 * Test std::call_once.
 * based on https://en.cppreference.com/w/cpp/thread/call_once
 */
TEST_CASE("std::call_once test")
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

/*
 * Test std::condition_variable example
 * based on https://en.cppreference.com/w/cpp/thread/condition_variable
 */
TEST_CASE("std::condition_variable test")
{
    std::mutex m;
    std::condition_variable cv;
    std::string data;
    bool ready = false;
    bool processed = false;

    auto worker_thread = [&m, &cv, &data, &ready, &processed]()
    {
        // wait until main() sends data
        std::unique_lock<std::mutex> lk(m);
        cv.wait(lk, [&ready]{ return ready; });

        // after the wait, we own the lock
        MESSAGE("Worker thread is processing data");
        data += " after processing";

        // send data back to main()
        processed = true;
        MESSAGE("Worker thread signals data processing completed");

        // manual unlocking is done before notifying, to avoid waking up
        // the waiting thread only to block again (see notify_one for details)
        lk.unlock();
        cv.notify_one();
    };

    MESSAGE("Testing std::condition_variable with 2 threads");
    std::thread worker(worker_thread);

    data = "Example data";
    // send data to the worker thread
    {
        std::lock_guard<std::mutex> lk(m);
        ready = true;
        MESSAGE("main() signals data ready for processing");
    }
    cv.notify_one();

    // wait for the worker
    {
        std::unique_lock<std::mutex> lk(m);
        cv.wait(lk, [&processed]{ return processed; });
    }
    MESSAGE("Back in main(), data checked");
    REQUIRE(data == "Example data after processing");

    worker.join();
}
