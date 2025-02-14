//-----------------------------------------------------------------------------//
// CRingBuffer MPMC - FIFO helper                                              //
// (c) 2023 Laurent Lardinois https://be.linkedin.com/in/laurentlardinois      //
//                                                                             //
// https://github.com/type-one/CRingBuffer_MPSC                                //
//                                                                             //
// This software is provided 'as-is', without any express or implied           //
// warranty.In no event will the authors be held liable for any damages        //
// arising from the use of this software.                                      //
//                                                                             //
// Permission is granted to anyone to use this software for any purpose,       //
// including commercial applications, and to alter itand redistribute it       //
// freely, subject to the following restrictions :                             //
//                                                                             //
// 1. The origin of this software must not be misrepresented; you must not     //
// claim that you wrote the original software.If you use this software         //
// in a product, an acknowledgment in the product documentation would be       //
// appreciated but is not required.                                            //
// 2. Altered source versions must be plainly marked as such, and must not be  //
// misrepresented as being the original software.                              //
// 3. This notice may not be removed or altered from any source distribution.  //
//-----------------------------------------------------------------------------//

#include "tools/atomic_helper.h"
#include "tools/ring_buffer_mpmc.h"
#include "tools/sync_object.h"
#include "tools/timer_chrono.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <windows.h>
#elif defined(__STDC_NO_THREADS__)
#include <pthread.h>
#include <sched.h>
#else
#include <threads.h>
#endif

/* avoid malloc/free in producer/consumer */
#define NO_DYNAMIC_ALLOC 1

/* no printf output during computation, better to benchmark */
#define NO_STDIO 0

/* single producer, single consumer, running as fast as possible without blocking (lock-free) */
//#define PRODUCER_NO_WAIT 1
//#define CONSUMER_NO_WAIT 1
//#define CONSUMER_SIMULATE_WORK_LOAD 0
//#define SINGLE_CONSUMER 1
//#define SINGLE_PRODUCER 1
//#define PRODUCER_NO_YIELD 1

/* multiple producers, multiple consumers, running as fast as possible without waiting */
#define PRODUCER_NO_WAIT 1
#define CONSUMER_NO_WAIT 1
#define CONSUMER_SIMULATE_WORK_LOAD 0
#define SINGLE_CONSUMER 0
#define SINGLE_PRODUCER 0
#define PRODUCER_NO_YIELD 1

/* multiple producers, multiple consumers, wait for readers, wait for writers, simulate work load */
//#define PRODUCER_NO_WAIT 0
//#define CONSUMER_NO_WAIT 0
//#define CONSUMER_SIMULATE_WORK_LOAD 1
//#define SINGLE_CONSUMER 0
//#define SINGLE_PRODUCER 0
//#define PRODUCER_NO_YIELD 0

/* single producer, multiple consumers, no wait for readers, no wait for writers, simulate work load */
//#define PRODUCER_NO_WAIT 1
//#define CONSUMER_NO_WAIT 1
//#define CONSUMER_SIMULATE_WORK_LOAD 1
//#define SINGLE_CONSUMER 0
//#define SINGLE_PRODUCER 1
//#define PRODUCER_NO_YIELD 0

/* single producer, multiple consumers, wait for readers, wait for writers, simulate work load */
//#define PRODUCER_NO_WAIT 0
//#define CONSUMER_NO_WAIT 0
//#define CONSUMER_SIMULATE_WORK_LOAD 1
//#define SINGLE_CONSUMER 0
//#define SINGLE_PRODUCER 1
//#define PRODUCER_NO_YIELD 0

#if SINGLE_PRODUCER
#define NB_PRODUCERS 1
#else
#define NB_PRODUCERS 4
#endif

#if SINGLE_CONSUMER
#define NB_CONSUMERS 1
#else
#define NB_CONSUMERS 8
#endif

#define NB_THREADS (NB_PRODUCERS + NB_CONSUMERS)

#define NB_MSGS_PER_PRODUCER 1000
#define NB_MSGS_TOTAL (NB_PRODUCERS * NB_MSGS_PER_PRODUCER)

#if NO_STDIO
#define LOG_INFO(...)
#define LOG_ERROR(...)
#else
#define LOG_INFO(...) printf(__VA_ARGS__)
#define LOG_ERROR(...) fprintf(stderr, __VA_ARGS__)
#endif

struct thread_context
{
    _atomic_bool m_stop_thread;
    struct ring_buffer_mpmc m_fifo;
    struct sync_object m_write_sync;
    struct sync_object m_read_sync;
    struct sync_object m_start_sync;
    _atomic_long m_msg_count;
    _atomic_long m_msg_skipped;
};

static void dec_and_check_end(struct thread_context* ctxt)
{
    sync_atomic_dec_32(ctxt->m_msg_count);
    sync_read_write();
    if (sync_atomic_load(ctxt->m_msg_count) <= 0)
    {
        sync_atomic_store(ctxt->m_stop_thread, true);
        sync_write_release();
        sync_object_broadcast(&(ctxt->m_write_sync));
        sync_object_broadcast(&(ctxt->m_read_sync));
    }
}

#if NO_DYNAMIC_ALLOC
static char st_message[NB_PRODUCERS][NB_MSGS_PER_PRODUCER][256];
#endif

#if defined(_WIN32)
static DWORD WINAPI producer_thread(LPVOID arg)
#elif defined(__STDC_NO_THREADS__)
static void* producer_thread(void* arg)
#else
static int producer_thread(void* arg)
#endif
{
    static int producer_id = 1;
    int my_id = producer_id++;

    struct thread_context* ctxt = (struct thread_context*)arg;

    if (ctxt)
    {
        /* wait signal from main thread before starting to work */
        sync_object_wait_for_signal(&(ctxt->m_start_sync));
    }

    char message[256];

    int count = 0;
    while (ctxt && count++ < NB_MSGS_PER_PRODUCER)
    {
        sync_read_acquire();
        if (sync_atomic_load(ctxt->m_stop_thread))
        {
            break;
        }

        /* produce something */
        snprintf(message, sizeof(message), "job %d-%d from producer %d", count, my_id, my_id);
#if NO_DYNAMIC_ALLOC
        char* duplicata = &st_message[my_id - 1][count][0];
        strncpy(duplicata, message, sizeof(st_message[my_id - 1][count]));
#else
        char* duplicata = strdup(message);
#endif

        if (!duplicata)
        {
            LOG_ERROR("producer %d could not strdup for job %d-%d\n", my_id, count, my_id);
            sync_atomic_inc_32(ctxt->m_msg_skipped);
            dec_and_check_end(ctxt);
        }
        else
        {
#if SINGLE_PRODUCER
            if (!ring_buffer_push_sp(&(ctxt->m_fifo), duplicata))
#else
            if (!ring_buffer_push_mp(&(ctxt->m_fifo), duplicata))
#endif
            {
                LOG_INFO("producer %d: buffer full, skip job %d-%d\n", my_id, count, my_id);
#if !NO_DYNAMIC_ALLOC
                free(duplicata);
#endif
                sync_atomic_inc_32(ctxt->m_msg_skipped);
                dec_and_check_end(ctxt);
            }
            else
            {
                sync_object_signal(&(ctxt->m_write_sync));
            }
        }

        /* wait reader, with a 1s timeout */
#if !PRODUCER_NO_WAIT
        (void)sync_object_wait_for_signal_timed(&(ctxt->m_read_sync), 1000000);
#endif

#if !PRODUCER_NO_YIELD
#if defined(_WIN32)
        Sleep(0);
#elif defined(__STDC_NO_THREADS__)
        sched_yield();
#else
        thrd_yield();
#endif
#endif
    }

#if defined(_WIN32)
    return 0;
#elif defined(__STDC_NO_THREADS__)
    pthread_exit(0);
    return NULL;
#else
    thrd_exit(0);
    return 0;
#endif
}


#if defined(_WIN32)
static DWORD WINAPI consumer_thread(LPVOID arg)
#elif defined(__STDC_NO_THREADS__)
static void* consumer_thread(void* arg)
#else
static int consumer_thread(void* arg)
#endif
{
    static int consumer_id = 1;
    int my_id = consumer_id++;

    struct thread_context* ctxt = (struct thread_context*)arg;

    if (ctxt)
    {
        /* wait signal from main thread before starting to work */
        sync_object_wait_for_signal(&(ctxt->m_start_sync));
    }
    while (ctxt)
    {
        sync_read_acquire();
        if (sync_atomic_load(ctxt->m_stop_thread))
        {
            break;
        }

#if !CONSUMER_NO_WAIT
        /* consume something, 1s timeout */
        (void)sync_object_wait_for_signal_timed(&(ctxt->m_write_sync), 1000000);
#endif

        void* elem = NULL;

#if SINGLE_CONSUMER
        if (!ring_buffer_pop_sc(&(ctxt->m_fifo), &elem))
#else
        if (!ring_buffer_pop_mc(&(ctxt->m_fifo), &elem))
#endif
        {
            LOG_INFO("consumer %d: buffer empty, skip turn\n", my_id);
        }
        else if (!elem)
        {
            LOG_ERROR("consumer %d: anomaly - retrieved ptr is null, skip turn\n", my_id);
            sync_object_signal(&(ctxt->m_read_sync));
        }
        else
        {
            LOG_INFO("consumer %d: received %s\n", my_id, (char*)elem);

            /* job taken */
            sync_object_signal(&(ctxt->m_read_sync));

#if CONSUMER_SIMULATE_WORK_LOAD
            /* simulate some variable time processing */
            for (int i = 0; i < (rand() & 0xffff); ++i)
            {
            }
#endif

#if !NO_DYNAMIC_ALLOC
            free(elem);
#endif

            dec_and_check_end(ctxt);
        }
    }

#if defined(_WIN32)
    return 0;
#elif defined(__STDC_NO_THREADS__)
    pthread_exit(0);
    return NULL;
#else
    thrd_exit(0);
    return 0;
#endif
}

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    int exit_code = 0;

    struct thread_context ctxt;
    struct timer_chrono timer;

    sync_atomic_store(ctxt.m_stop_thread, false);
    sync_atomic_store(ctxt.m_msg_count, NB_MSGS_PER_PRODUCER);
    sync_atomic_store(ctxt.m_msg_skipped, 0);
    sync_write_release();

    (void)init_timer_chrono(&timer);

    if (init_ring_buffer_mpmc(&(ctxt.m_fifo)) < 0)
    {
        return -1;
    }

    if (init_sync_object(&(ctxt.m_write_sync), false) < 0)
    {
        deinit_ring_buffer_mpmc(&(ctxt.m_fifo));
        return -1;
    }

    if (init_sync_object(&(ctxt.m_read_sync), false) < 0)
    {
        deinit_sync_object(&(ctxt.m_write_sync));
        deinit_ring_buffer_mpmc(&(ctxt.m_fifo));
        return -1;
    }

    if (init_sync_object(&(ctxt.m_start_sync), false) < 0)
    {
        deinit_sync_object(&(ctxt.m_read_sync));
        deinit_sync_object(&(ctxt.m_write_sync));
        deinit_ring_buffer_mpmc(&(ctxt.m_fifo));
        return -1;
    }

#if defined(_WIN32)

    DWORD thread_tid[NB_THREADS];
    HANDLE thread_hnd[NB_THREADS];

    memset(thread_tid, 0, sizeof(thread_tid));
    memset(thread_hnd, 0, sizeof(thread_hnd));

    for (int i = 0; i < NB_PRODUCERS; ++i)
    {
        thread_hnd[i] = CreateThread(0, 4096, producer_thread, &ctxt, 0, &thread_tid[i]);
        if (NULL == thread_hnd[i])
        {
            goto exit_error;
        }
    }

    for (int i = 0; i < NB_CONSUMERS; ++i)
    {
        thread_hnd[NB_PRODUCERS + i] = CreateThread(0, 4096, consumer_thread, &ctxt, 0, &thread_tid[NB_PRODUCERS + i]);
        if (NULL == thread_hnd[i])
        {
            goto exit_error;
        }
    }

#elif defined(__STDC_NO_THREADS__)

    pthread_t thread_tid[NB_THREADS];
    memset(thread_tid, 0, sizeof(pthread_t));

    for (int i = 0; i < NB_PRODUCERS; ++i)
    {
        if (0 != pthread_create(&thread_tid[i], NULL, producer_thread, &ctxt))
        {
            goto exit_error;
        }
    }

    for (int i = 0; i < NB_CONSUMERS; ++i)
    {
        if (0 != pthread_create(&thread_tid[NB_PRODUCERS + i], NULL, consumer_thread, &ctxt))
        {
            goto exit_error;
        }
    }

#else

    thrd_t thread_tid[NB_THREADS];
    memset(thread_tid, 0, sizeof(thrd_t));

    for (int i = 0; i < NB_PRODUCERS; ++i)
    {
        if (thrd_success != thrd_create(&thread_tid[i], producer_thread, &ctxt))
        {
            goto exit_error;
        }
    }

    for (int i = 0; i < NB_CONSUMERS; ++i)
    {
        if (thrd_success != thrd_create(&thread_tid[NB_PRODUCERS + i], consumer_thread, &ctxt))
        {
            goto exit_error;
        }
    }
#endif

    /* this is the end */
    goto exit_main;

exit_error:
    exit_code = -1;

    /* cause early threads exit */
    sync_atomic_store(ctxt.m_stop_thread, true);
    sync_write_release();

exit_main:

    /* signal to launch working */
    sync_object_broadcast(&(ctxt.m_start_sync));

    double start_time = timer_chrono_current_time_ms(&timer);

    /* wait threads */

#if defined(_WIN32)

    for (int i = 0; (i < NB_THREADS) && thread_hnd[i]; ++i)
    {
        WaitForSingleObject(thread_hnd[i], INFINITE);
        CloseHandle(thread_hnd[i]);
    }

#elif defined(__STDC_NO_THREADS__)

    for (int i = 0; (i < NB_THREADS) && thread_tid[i]; ++i)
    {
        void* ret;
        pthread_join(thread_tid[i], &ret);
    }

#else

    for (int i = 0; (i < NB_THREADS) && thread_tid[i]; ++i)
    {
        int ret;
        thrd_join(thread_tid[i], &ret);
    }

#endif

    double end_time = timer_chrono_current_time_ms(&timer);

    long skip_counter = sync_atomic_load(ctxt.m_msg_skipped);
    printf("\n%ld messages processed, %ld messages skipped\n", NB_MSGS_TOTAL - skip_counter, skip_counter);
    printf("execution time is %lf ms\n", end_time - start_time);
    printf("average of %lf ms per message processed\n", (end_time - start_time) / (NB_MSGS_TOTAL - skip_counter));

    (void)deinit_sync_object(&(ctxt.m_start_sync));
    (void)deinit_sync_object(&(ctxt.m_read_sync));
    (void)deinit_sync_object(&(ctxt.m_write_sync));
    (void)deinit_ring_buffer_mpmc(&(ctxt.m_fifo));

    return exit_code;
}
