#include "testkernel.h"

#include <mutex>
#include <string>
#include <thread>

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
