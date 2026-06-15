/*******************************************************************************
@ name

@ function

@ exception

@ note

--------------------------------------------------------------------------------
@ [2025-06-26] ujoy created


--------------------------------------------------------------------------------
* copyright(c) 2024 - 2027, ujoy, reserved all right.

*******************************************************************************/
#include <gtest/gtest.h>
#include <eco/sync/spinlock.hpp>
#include <thread>


////////////////////////////////////////////////////////////////////////////////
class Sync : public ::testing::Test
{
public:
    inline void test_spinlock(int THREADS)
    {
        std::unique_ptr<std::thread[]> T(new std::thread[THREADS]);
        for (int i = 0; i < THREADS; i++)
        {
            T[i] = std::thread([&, i]()
            {
                eco::spinlock lock(m_mutex);
                ++m_count;
                EXPECT_EQ(m_count, 1);

                // heavy works
                const size_t MEM_SIZE = 4 * 1024 * 1024;
                char* buff = static_cast<char*>(malloc(MEM_SIZE));
                for (size_t i = 0; i < MEM_SIZE; i++)
                {
                    buff[i] = 'A' + static_cast<char>(i);
                }
                free(buff);
                
                --m_count;
                EXPECT_EQ(m_count, 0);
            });
        }
        for (int i = 0; i < THREADS; i++)
        {
            T[i].join();
        }
        ASSERT_EQ(m_count, 0);
    }

    int m_count = 0;
    eco::spinmutex   m_mutex;
};


////////////////////////////////////////////////////////////////////////////////
TEST_F(Sync, spinlock)
{
    test_spinlock(10);
    test_spinlock(20);
}