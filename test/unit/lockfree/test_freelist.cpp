#include <gtest/gtest.h>
////////////////////////////////////////////////////////////////////////////////
#include <eco/lockfree/queue.hpp>
#include <eco/lockfree/cachepool.hpp>
#include <eco/test/timer.hpp>
#include <eco/sync/waitset.h>
#include <thread>


////////////////////////////////////////////////////////////////////////////////
class Freelist : public ::testing::Test
{
private:
    public:
    // 20 times with 1M object
    //static const int TNUM = 20;
    //static const int DNUM = 1 * 1024 * 1024;
    //static const int UNUM = 1 * 1024;
    static const int SIZE = 40 * 1024 * 1024;

public:
    template<uint32_t size, uint32_t batch, typename freelist_t>
    inline void test(freelist_t& fl)
    {
        ASSERT_EQ(fl.size(),  size);
        ASSERT_EQ(fl.batch(), batch);
        
        // alloc
        int* intv[batch];
        for (uint32_t i = 0; i < fl.batch(); i++)
        {
            intv[i] = fl.template alloc<int>();
            *intv[i] = (i + 1);
        }
        // dealloc
        fl.dealloc(intv[0]);
        ASSERT_EQ(fl.alloc(), intv[0]);
        fl.dealloc(intv[1]);
        ASSERT_EQ(fl.alloc(), intv[1]);
        
        for (uint32_t i = 0; i < fl.batch(); i++)
        {
            fl.dealloc(intv[i]);
        }
        for (uint32_t i = 0; i < fl.batch(); i++)
        {
            ASSERT_EQ(fl.alloc(), intv[fl.batch() - i - 1]);
        }
    }

public:

    template<typename freelist_t>
    struct alloc_fl
    {
    public:
        inline alloc_fl(freelist_t& fl) : m_fl(fl) {}
        inline void* alloc()   { return m_fl.alloc(); }
        inline void  dealloc(void* obj)     { m_fl.dealloc(obj); }
        freelist_t& m_fl;
    };
    struct alloc_os
    {
    public:
        inline alloc_os(uint32_t size) : m_size(size) {}
        inline void* alloc()   { return malloc(m_size); }
        inline void  dealloc(void* obj)     { free(obj); }
        uint32_t m_size;
    };

    class perf
    {
    public:
        uint32_t m_size  = 0;

        struct datas
        {
            inline datas()  {}
            inline ~datas() { free(data); }

            inline datas(uint32_t batch) 
            {
                data = (int**)(malloc(sizeof(int*) * batch));
            }
            inline void reset(uint32_t batch)
            {
                free(data);
                data = (int**)(malloc(sizeof(int*) * batch));
            }
            int**  data = 0;
        };
        
        struct thread
        {
            int64_t cost = 0;
            datas   data;
            std::thread thread;
        };

        

        inline perf(uint32_t size) : m_size(size)
        {}

        template<typename alloc_t>
        inline void test_sync(alloc_t& a, int** vec, int times, int units)
        {
            for (int i = 0; i < times; i++)
            {
                for (int j = 0; j < units; j++)
                {
                    vec[j] = static_cast<int*>(a.alloc());
                    *vec[j] = i - j;
                }
                for (int j = 0; j < units; j++)
                {
                    *vec[j] = i + j;
                    a.dealloc(vec[j]);
                }
            }
        }

        template<typename alloc_t>
        inline int64_t test_sync(alloc_t&& a, int times, int units)
        {
            datas data(units);
            eco::test::timer timer;
            test_sync(a, data.data, times, units);
            return timer.timeup();
        }
   
        template<typename alloc_t>
        inline int64_t test_mpc(alloc_t&& a, int times, int units, int threads)
        {
            int64_t ms_cost = 0;
            std::unique_ptr<thread[]> worker(new thread[threads]);
            for (int i = 0; i < threads; i++)
            {
                thread& tmp = worker[i];
                tmp.data.reset(units);
                tmp.thread = std::thread([=, &a, &tmp]() {
                    eco::test::timer timer;
                    test_sync(a, tmp.data.data, times, units);
                    tmp.cost = timer.timeup();
                    tmp.cost = tmp.cost;
                });
            }

            for (int i = 0; i < threads; i++)
            {
                worker[i].thread.join();
                ms_cost += worker[i].cost;
            }
            return ms_cost;
        }

    public:
        struct thread_spsc
        {
            int64_t cost_p = 0;
            int64_t cost_c = 0;
            std::unique_ptr<eco::lockfree::fifo_mpmc> datas;
            std::thread thread_p;
            std::thread thread_c;

            inline void set_capacity(uint32_t capacity)
            {
                datas.reset(new eco::lockfree::fifo_mpmc(capacity));
            }
        };

        template<typename alloc_t>
        inline int64_t test_spsc(alloc_t&& a, int times, int units, int threads)
        {
            int64_t ms_cost = 0;
            std::unique_ptr<thread_spsc[]> worker(new thread_spsc[threads]);
            for (int i = 0; i < threads; i++)
            {
                thread_spsc& tmp = worker[i];
                tmp.set_capacity(units * threads * 2);

                // producer thread
                tmp.thread_p = std::thread([=, &a, &tmp]()
                {
                    eco::test::timer timer;
                    for (int i = 0; i < times; i++)
                    {
                        for (int j = 0; j < units; j++)
                        {
                            int* obj = static_cast<int*>(a.alloc());
                            *obj = i - j;
                            while (!tmp.datas->push_back(obj)) {}
                        }
                    }
                    tmp.cost_p = timer.timeup();
                });

                // consumer thread
                tmp.thread_c = std::thread([=, &a, &tmp]()
                {
                    eco::test::timer timer;
                    for (int i = 0; i < times; i++)
                    {
                        for (int j = 0; j < units; j++)
                        {
                            int* obj = NULL;
                            while ((obj = tmp.datas->pop_front<int>()) == NULL) {}
                            *obj = i - j;
                            a.dealloc(obj);
                        }
                    }
                    tmp.cost_c = timer.timeup();
                });
            }

            for (int i = 0; i < threads; i++)
            {
                worker[i].thread_c.join();
                worker[i].thread_p.join();
                ms_cost += worker[i].cost_p;
                ms_cost += worker[i].cost_c;
            }
            return ms_cost;
        }
    };  
};


////////////////////////////////////////////////////////////////////////////////
class perf_mpmc
{
public:
    enum
    {
        SIZE        = 8192,
        FLAG_IDL    = 0,
        FLAG_USE    = 1
    };

    std::atomic<int*> m_datas[SIZE];
    std::atomic<int>  m_flags[SIZE];
    std::atomic<int>  m_running;
    eco::waitset      m_waitset;

    struct thread_mpmc
    {
        std::thread thread;
        uint64_t alloc = 0;
        uint64_t frees = 0;
        uint32_t index = 0;
    };

    inline perf_mpmc()
    {
        for (uint32_t i = 0; i < SIZE; i++)
        {
            m_datas[i].store(nullptr);
            m_flags[i].store(FLAG_IDL);
        }
        m_running = 1;
    }

    template<typename alloc_t>
    inline void clear(alloc_t& a)
    {
        for (uint32_t i = 0; i < SIZE; i++)
        {
            int* obj = m_datas[i];
            if (obj != NULL)
            {
                a.dealloc(obj);
            }
            m_datas[i].store(nullptr);
            m_flags[i].store(FLAG_IDL);
        }
    }

    inline int* pick(uint32_t& idx)
    {
        while (true)
        {
            int flag_old = m_flags[idx].load();
            if (flag_old == FLAG_IDL)
            {
                if (m_flags[idx].compare_exchange_weak(flag_old, FLAG_USE))
                {
                    break;
                }
            }
            idx = (idx + 1) & (SIZE - 1);
        }
        int* obj = m_datas[idx];
        if (obj != NULL)
        {
            m_datas[idx] = NULL;
            m_flags[idx] = FLAG_IDL;
        }
        return obj;
    }

    inline void set(uint32_t idx, int* obj)
    {
        m_datas[idx] = obj;
        m_flags[idx] = FLAG_IDL;
    }

    template<typename alloc_t>
    inline std::pair<uint64_t, uint64_t> test(alloc_t&& a, int secs, int threads)
    {
        m_running = true;
        std::unique_ptr<thread_mpmc[]> worker(new thread_mpmc[threads]);
        uint32_t units = SIZE / threads;
        for (int i = 0; i < threads; i++)
        {
            worker[i].alloc = 0;
            worker[i].frees = 0;
            worker[i].index = i * units;
        }
        
        for (int i = 0; i < threads; i++)
        {
            thread_mpmc& tmp = worker[i];
            tmp.thread = std::thread([=, &a, &tmp]()
            {
                while (m_running)
                {
                    int* obj = this->pick(tmp.index);
                    if (obj == NULL)
                    {
                        obj  = static_cast<int*>(a.alloc());
                        *obj = tmp.index;
                        this->set(tmp.index, obj);
                        tmp.alloc++;
                    }
                    else
                    {
                        a.dealloc(obj);
                        tmp.frees++;
                    }
                    tmp.index++;
                }
            });
        }
        m_waitset.timed_wait(secs * 1000);
        m_running = false;

        std::pair<uint64_t, uint64_t> result(0, 0);
        for (int i = 0; i < threads; i++)
        {
            worker[i].thread.join();
            result.first  += worker[i].alloc;
            result.second += worker[i].frees;
        }
        this->clear(a);
        return result;
    }
};

////////////////////////////////////////////////////////////////////////////////
TEST_F(Freelist, heap_mode_ptr)
{
    eco::lockfree::freepolicy policy;
    eco::lockfree::cachepool  pool;
    ASSERT_TRUE(pool.init(policy));
    ASSERT_TRUE(pool.ready());

    // same freelist: ptr constructor
    typedef eco::lockfree::freelist<> freelist_t;
    freelist_t::ptr int4_1 = pool.make_freelist<>(8, 32);
    ASSERT_TRUE(!int4_1->local_mode());
    ASSERT_EQ(int4_1.refc(), 1u);
    freelist_t::ptr int4_2 = int4_1;
    ASSERT_EQ(int4_1.refc(), 2u);
    ASSERT_EQ(int4_2.refc(), 2u);
    ASSERT_EQ(int4_2.get(), int4_1.get());
    freelist_t::ptr int4_3 = std::move(int4_1);
    ASSERT_EQ(int4_2.refc(), 2u);
    ASSERT_EQ(int4_3.refc(), 2u);
    ASSERT_EQ(nullptr, int4_1.get());
    ASSERT_EQ(int4_2.get(), int4_3.get());
    int4_2.reset();
    ASSERT_EQ(int4_3.refc(), 1u);
    ASSERT_EQ(int4_2.get(), nullptr);
    // same freelist: ptr asign
    int4_2 = int4_3;
    ASSERT_EQ(int4_2.refc(), 2u);
    ASSERT_EQ(int4_3.refc(), 2u);
    ASSERT_EQ(int4_2.get(), int4_3.get());
    int4_1 = std::move(int4_3);
    ASSERT_EQ(int4_1.refc(), 2u);
    ASSERT_EQ(int4_2.refc(), 2u);
    ASSERT_EQ(int4_1.get(), int4_2.get());
    int4_3 = *int4_1;
    ASSERT_EQ(int4_1.refc(), 3u);
    ASSERT_EQ(int4_2.refc(), 3u);
    ASSERT_EQ(int4_3.refc(), 3u);
    ASSERT_EQ(int4_1.get(), int4_2.get());
    ASSERT_EQ(int4_1.get(), int4_3.get());
    int4_1.reset();
    int4_2.reset();
    ASSERT_EQ(int4_1.get(), nullptr);
    ASSERT_EQ(int4_2.get(), nullptr);
    ASSERT_EQ(int4_3.refc(), 1u);

    // diff freelist: ptr asign
    freelist_t::ptr int8_1 = pool.make_freelist<>(8, 32);
    freelist_t::ptr int8_2 = int8_1;
    int4_1 = int8_1;
    ASSERT_EQ(int4_1.refc(), 3u);
    ASSERT_EQ(int8_1.refc(), 3u);
    ASSERT_EQ(int8_2.refc(), 3u);
    ASSERT_EQ(int4_1.get(), int8_1.get());
    ASSERT_EQ(int4_1.get(), int8_2.get());
    int4_2 = *int8_1;
    ASSERT_EQ(int4_1.refc(), 4u);
    ASSERT_EQ(int4_2.refc(), 4u);
    ASSERT_EQ(int8_1.refc(), 4u);
    ASSERT_EQ(int8_2.refc(), 4u);
    ASSERT_EQ(int4_1.get(), int8_1.get());
    ASSERT_EQ(int4_1.get(), int8_2.get());
    ASSERT_EQ(int4_1.get(), int4_2.get());
    int4_3 = std::move(int8_2);
    ASSERT_EQ(int8_2.get(), nullptr);
    ASSERT_EQ(int4_1.refc(), 4u);
    ASSERT_EQ(int4_2.refc(), 4u);
    ASSERT_EQ(int4_3.refc(), 4u);
    ASSERT_EQ(int8_1.refc(), 4u);
    ASSERT_EQ(int8_1.get(), int4_1.get());
    ASSERT_EQ(int8_1.get(), int4_2.get());
    ASSERT_EQ(int8_1.get(), int4_3.get());
    int4_1.reset();
    int4_2.reset();
    int4_3.reset();
    ASSERT_EQ(int8_1.refc(), 1u);
    int8_1.reset();
    ASSERT_EQ(int8_1.get(), nullptr);
}


////////////////////////////////////////////////////////////////////////////////
TEST_F(Freelist, local_mode_test)
{
    eco::lockfree::freepolicy policy;
    eco::lockfree::cachepool  pool(policy);
    ASSERT_TRUE(pool.ready());

    // freelist: local mode
    eco::lockfree::freelist<> int8(pool, 8, 512);
    ASSERT_TRUE(int8.local_mode());
    test<8, 512>(int8);
    // 2page
    eco::lockfree::freelist<> int16(pool, 16, 512);
    ASSERT_TRUE(int16.local_mode());
    test<16, 512>(int16);
}
TEST_F(Freelist, heap_mode_test)
{
    eco::lockfree::freepolicy policy;
    eco::lockfree::cachepool  pool;
    ASSERT_TRUE(pool.init(policy));
    ASSERT_TRUE(pool.ready());

    // freelist: local mode
    typedef eco::lockfree::freelist<> freelist_t;
    freelist_t::ptr int8 = pool.make_freelist(8, 512);
    ASSERT_TRUE(!int8->local_mode());
    test<8, 512>(*int8);
    freelist_t::ptr int16 = pool.make_freelist(16, 512);
    ASSERT_TRUE(!int16->local_mode());
    test<16, 512>(*int16);
}


////////////////////////////////////////////////////////////////////////////////
TEST_F(Freelist, none)
{
    typedef eco::lockfree::freelist<eco::lockfree::stack> freelist_t;
    eco::lockfree::freepolicy policy;
    policy.memory_size(SIZE * 2);
    eco::lockfree::cachepool  pool(policy);
    ASSERT_TRUE(pool.ready());
    freelist_t fl(pool, 144, 32);
    uint32_t TIMES = 20000;
    uint32_t UNITS = 1024;

    // [main thread]
    perf test(fl.size());
    int64_t ms_fl = test.test_sync(alloc_fl(fl), TIMES, UNITS);
    int64_t ms_os = test.test_sync(alloc_os(fl.size()), TIMES, UNITS);
    printf("[MT] freelist=%ld os=%ld %2f\n",  ms_fl, ms_os, double(ms_fl) / ms_os);
    
    // [1 thread]
    ms_os = test.test_mpc(alloc_os(fl.size()), TIMES, UNITS, 1);
    ms_fl = test.test_mpc(alloc_fl(fl), TIMES, UNITS, 1);
    printf("[T1] freelist=%ld os=%ld %2f\n",  ms_fl, ms_os, double(ms_fl) / ms_os);

    // [1 thread]
    perf_mpmc perf_mpmc;
    auto rc_fl = perf_mpmc.test(alloc_fl(fl), 2, 1);
    auto rc_os = perf_mpmc.test(alloc_os(fl.size()), 2, 1);
    printf("[MC] freelist=%ld,%ld os=%ld,%ld %2f\n", 
        rc_fl.first, rc_fl.second, rc_os.first, rc_os.second,
        double(rc_os.first + rc_os.second) / (rc_fl.first + rc_fl.second));
}


////////////////////////////////////////////////////////////////////////////////
TEST_F(Freelist, sc)
{
    typedef eco::lockfree::freelist<eco::lockfree::stack_sc> freelist_t;
    eco::lockfree::freepolicy policy;
    policy.memory_size(SIZE * 2);
    eco::lockfree::cachepool  pool(policy);
    ASSERT_TRUE(pool.ready());
    freelist_t fl(pool, 144, 32);
    uint32_t TIMES = 20000;
    uint32_t UNITS = 1024;

    // [main thread]
    perf test(fl.size());
    int64_t ms_fl = test.test_sync(alloc_fl(fl), TIMES, UNITS);
    int64_t ms_os = test.test_sync(alloc_os(fl.size()), TIMES, UNITS);
    printf("[MT] freelist=%ld os=%ld %2f\n",  ms_fl, ms_os, double(ms_fl) / ms_os);
    
    // [1 thread]
    ms_os = test.test_mpc(alloc_os(fl.size()), TIMES, UNITS, 1);
    ms_fl = test.test_mpc(alloc_fl(fl), TIMES, UNITS, 1);
    printf("[T1] freelist=%ld os=%ld %2f\n",  ms_fl, ms_os, double(ms_fl) / ms_os);

    // [1 spsc]
    ms_fl = test.test_spsc(alloc_fl(fl), TIMES, UNITS, 1);
    ms_os = test.test_spsc(alloc_os(fl.size()), TIMES, UNITS, 1);
    printf("[SC] freelist=%ld os=%ld %2f\n",  ms_fl, ms_os, double(ms_fl) / ms_os);

    // [1 thread]
    perf_mpmc perf_mpmc;
    auto rc_fl = perf_mpmc.test(alloc_fl(fl), 2, 1);
    auto rc_os = perf_mpmc.test(alloc_os(fl.size()), 2, 1);
    printf("[MC] freelist=%ld,%ld os=%ld,%ld %2f\n", 
        rc_fl.first, rc_fl.second, rc_os.first, rc_os.second,
        double(rc_os.first + rc_os.second) / (rc_fl.first + rc_fl.second));
}


////////////////////////////////////////////////////////////////////////////////
inline void freelist_mc(uint32_t size)
{
    typedef eco::lockfree::freelist<eco::lockfree::stack_mc> freelist_t;
    eco::lockfree::freepolicy policy;
    policy.memory_size(Freelist::SIZE * 2);
    eco::lockfree::cachepool  pool(policy);
    ASSERT_TRUE(pool.ready());
    freelist_t fl(pool, size, 32);
    uint32_t TIMES = 50000;
    uint32_t UNITS = 1024;

    // [main thread]
    Freelist::perf test(fl.size());
    int64_t ms_fl = test.test_sync(Freelist::alloc_fl(fl), TIMES, UNITS);
    int64_t ms_os = test.test_sync(Freelist::alloc_os(fl.size()), TIMES, UNITS);
    printf("[MT] freelist=%ld os=%ld %2f (%u)\n",  ms_fl, ms_os, double(ms_fl) / ms_os, size);
    
    // [1 thread]
    ms_os = test.test_mpc(Freelist::alloc_os(fl.size()), TIMES, UNITS, 1);
    ms_fl = test.test_mpc(Freelist::alloc_fl(fl), TIMES, UNITS, 1);
    printf("[T1] freelist=%ld os=%ld %2f (%u)\n",  ms_fl, ms_os, double(ms_fl) / ms_os, size);

    // [2 thread]
    ms_os = test.test_mpc(Freelist::alloc_os(fl.size()), TIMES, UNITS, 2);
    ms_fl = test.test_mpc(Freelist::alloc_fl(fl), TIMES, UNITS, 2);
    printf("[T2] freelist=%ld os=%ld %2f (%u)\n",  ms_fl, ms_os, double(ms_fl) / ms_os, size);

    // [3 thread]
    ms_os = test.test_mpc(Freelist::alloc_os(fl.size()), TIMES, UNITS, 3);
    ms_fl = test.test_mpc(Freelist::alloc_fl(fl), TIMES, UNITS, 3);
    printf("[T3] freelist=%ld os=%ld %2f (%u)\n",  ms_fl, ms_os, double(ms_fl) / ms_os, size);
}
////////////////////////////////////////////////////////////////////////////////
TEST_F(Freelist, mc)
{
    freelist_mc(8);
    freelist_mc(144);
    freelist_mc(512);
}


////////////////////////////////////////////////////////////////////////////////
inline void freelist_mpmc(uint32_t size)
{
    typedef eco::lockfree::freelist<eco::lockfree::stack_mc> freelist_t;
    eco::lockfree::freepolicy policy;
    policy.memory_size(Freelist::SIZE * 2);
    eco::lockfree::cachepool  pool(policy);
    ASSERT_TRUE(pool.ready());
    freelist_t fl(pool, size, 64);
    
    // [1 thread]
    perf_mpmc perf;
    auto rc_fl = perf.test(Freelist::alloc_fl(fl), 2, 1);
    auto rc_os = perf.test(Freelist::alloc_os(fl.size()), 2, 1);
    printf("[T1] freelist=%ld,%ld os=%ld,%ld %2f (%u)\n",  
        rc_fl.first, rc_fl.second, rc_os.first, rc_os.second,
        double(rc_os.first + rc_os.second) / (rc_fl.first + rc_fl.second), size);

    // [2 thread]
    rc_fl = perf.test(Freelist::alloc_fl(fl), 2, 2);
    rc_os = perf.test(Freelist::alloc_os(fl.size()), 2, 2);
    printf("[T2] freelist=%ld,%ld os=%ld,%ld %2f (%u)\n",  
        rc_fl.first, rc_fl.second, rc_os.first, rc_os.second,
        double(rc_os.first + rc_os.second) / (rc_fl.first + rc_fl.second), size);

    // [3 thread]
    rc_fl = perf.test(Freelist::alloc_fl(fl), 2, 3);
    rc_os = perf.test(Freelist::alloc_os(fl.size()), 2, 3);
    printf("[T3] freelist=%ld,%ld os=%ld,%ld %2f (%u)\n",  
        rc_fl.first, rc_fl.second, rc_os.first, rc_os.second,
        double(rc_os.first + rc_os.second) / (rc_fl.first + rc_fl.second), size);

    // [4 thread]
    rc_fl = perf.test(Freelist::alloc_fl(fl), 2, 4);
    rc_os = perf.test(Freelist::alloc_os(fl.size()), 2, 4);
    printf("[T4] freelist=%ld,%ld os=%ld,%ld %2f (%u)\n",  
        rc_fl.first, rc_fl.second, rc_os.first, rc_os.second,
        double(rc_os.first + rc_os.second) / (rc_fl.first + rc_fl.second), size);
}
////////////////////////////////////////////////////////////////////////////////
TEST_F(Freelist, mpmc_512)
{
    freelist_mpmc(8);
    freelist_mpmc(144);
    freelist_mpmc(512);
}
