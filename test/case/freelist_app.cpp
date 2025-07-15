#include <eco/lockfree/cachepool.hpp>
#include <eco/test/timer.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <thread>


static const int TNUM = 20;
static const int DNUM = 1 * 1024 * 1024;
static const int UNUM = 1 * 1024;
static const int SIZE = 40 * 1024 * 1024;

struct array_t
{
    inline array_t()  { vec = (int**)(malloc(sizeof(int*) * DNUM)); }
    inline ~array_t() { free(vec); }
    int** vec;
};

struct thread_t
{
    int64_t ms_cost = 0;
    array_t rc;
    std::thread thread;
};

void test_alloc_dealloc(eco::lockfree::freelist& fl, int** vec, int times, int units)
{
    for (int i = 0; i < times; i++)
    {
        for (int j = 0; j < units; j++)
        {
            vec[j] = fl.alloc<int>();
            *vec[j] = i - j;
        }
        for (int j = 0; j < units; j++)
        {
            *vec[j] = i + j;
            fl.dealloc(vec[j]);
        }
    }
}

void test_malloc_free(int** vec, int times, int units)
{
    for (int i = 0; i < times; i++)
    {
        for (int j = 0; j < units; j++)
        {
            vec[j] = static_cast<int*>(malloc(8));
            *vec[j] = i - j;
        }
        for (int j = 0; j < units; j++)
        {
            *vec[j] = i + j;
            free(vec[j]);
        }
    }
}

uint32_t test_concurrent(int tsize, eco::lockfree::freelist* fl)
{
    thread_t worker[tsize];
    for (int i = 0; i < tsize; i++)
    {
        thread_t& tmp = worker[i];
        tmp.thread = std::thread([&]() {
            const uint32_t TIMES = 20 * 1024;
            const uint32_t UNITS = 1024;
            eco::test::timer timer;
            if (fl == NULL)
                test_malloc_free(tmp.rc.vec, TIMES, UNITS);
            else
                test_alloc_dealloc(*fl, tmp.rc.vec, TIMES, UNITS);
            tmp.ms_cost = timer.timeup();
        });
    }
    int64_t ms_cost = 0;
    for (int i = 0; i < tsize; i++)
    {
        worker[i].thread.join();
        ms_cost += worker[i].ms_cost;
    }
    return ms_cost;
}


int main()
{
    eco::lockfree::freepolicy policy;
    policy.memory_size(40 * 1024 * 1024);
    eco::lockfree::cachepool pool(policy);
    eco::lockfree::freelist list(pool, 8, 4096);
    
    // single thread
    int64_t fl = test_concurrent(1, &list);
    int64_t os = test_concurrent(1, NULL);
    printf("[sync] fl=%ld os=%ld, %2f\n", fl, os, double(fl) / os);

    // spsc thread
    fl = test_concurrent(2, &list);
    os = test_concurrent(2, NULL);
    printf("[spsc] fl=%ld os=%ld, %2f\n", fl, os, double(fl) / os);
}