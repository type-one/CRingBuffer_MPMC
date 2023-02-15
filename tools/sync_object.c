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

#include "atomic_helper.h"

#define SYNC_OBJECT_IMPLEM
#include "sync_object.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#if defined(_WIN32)
#include <intrin.h>
#include <windows.h>
#elif defined(__STDC_NO_THREADS__)
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#else
#include <threads.h>
#include <time.h>
#endif


int init_sync_object(struct sync_object* sync, bool initial_state)
{
    if (!sync)
    {
        return -1;
    }

    memset(sync, 0, sizeof(struct sync_object));

    sync->m_signaled = initial_state;
    sync->m_stop = false;
    sync->m_broadcasted = false;

#if defined(_WIN32)

    InitializeCriticalSection(&(sync->m_mutex));
    InitializeConditionVariable(&(sync->m_cond));

    return 0;

#elif defined(__STDC_NO_THREADS__)
    if (pthread_mutex_init(&(sync->m_mutex), NULL) < 0)
    {
        return -1;
    }

    if (pthread_condattr_init(&(sync->m_cond_attr)) < 0)
    {
        goto destroy_mutex;
    }

    if (pthread_condattr_setclock(&(sync->m_cond_attr), CLOCK_MONOTONIC) < 0)
    {
        goto destroy_cond;
    }

    if (pthread_cond_init(&(sync->m_cond), &(sync->m_cond_attr)) < 0)
    {
        goto destroy_cond;
    }

    return 0;

destroy_cond:
    pthread_cond_destroy(&(sync->m_cond));

destroy_mutex:
    pthread_mutex_destroy(&(sync->m_mutex));

    return -1;

#else
    if (thrd_success != mtx_init(&(sync->m_mutex), mtx_plain))
    {
        return -1;
    }

    if (thrd_success != cnd_init(&(sync->m_cond)))
    {
        goto destroy_mutex;
    }

    return 0;

destroy_mutex:
    mtx_destroy(&(sync->m_mutex));

    return -1;

#endif
}

int deinit_sync_object(struct sync_object* sync)
{
    if (!sync)
    {
        return -1;
    }

#if defined(_WIN32)
    EnterCriticalSection(&(sync->m_mutex));
#elif defined(__STDC_NO_THREADS__)
    pthread_mutex_lock(&(sync->m_mutex));
#else
    mtx_lock(&(sync->m_mutex));
#endif

    sync->m_broadcasted = true;
    sync->m_signaled = true;
    sync->m_stop = true;

#if defined(_WIN32)
    LeaveCriticalSection(&(sync->m_mutex));
    WakeAllConditionVariable(&(sync->m_cond));
#elif defined(__STDC_NO_THREADS__)
    pthread_mutex_unlock(&(sync->m_mutex));
    pthread_cond_broadcast(&(sync->m_cond));
#else
    mtx_unlock(&(sync->m_mutex));
    cnd_broadcast(&(sync->m_cond));
#endif

#if defined(_WIN32)
    Sleep(100);
    DeleteCriticalSection(&(sync->m_mutex));
#elif defined(__STDC_NO_THREADS__)
    usleep(100000);
    pthread_cond_destroy(&(sync->m_cond));
    pthread_mutex_destroy(&(sync->m_mutex));
#else
    thrd_sleep(&(struct timespec) { .tv_sec = 0, .tv_nsec = 100000000 }, NULL);
    cnd_destroy(&(sync->m_cond));
    mtx_destroy(&(sync->m_mutex));
#endif

    return 0;
}

int sync_object_signal(struct sync_object* sync)
{
    if (!sync)
    {
        return -1;
    }

#if defined(_WIN32)
    EnterCriticalSection(&(sync->m_mutex));
#elif defined(__STDC_NO_THREADS__)
    pthread_mutex_lock(&(sync->m_mutex));
#else
    mtx_lock(&(sync->m_mutex));
#endif

    sync->m_broadcasted = false;
    sync->m_signaled = true;

#if defined(_WIN32)
    LeaveCriticalSection(&(sync->m_mutex));
#elif defined(__STDC_NO_THREADS__)
    pthread_mutex_unlock(&(sync->m_mutex));
#else
    mtx_unlock(&(sync->m_mutex));
#endif

#if defined(_WIN32)
    WakeConditionVariable(&(sync->m_cond));
#elif defined(__STDC_NO_THREADS__)
    pthread_cond_signal(&(sync->m_cond));
#else
    cnd_signal(&(sync->m_cond));
#endif

    return 0;
}


int sync_object_broadcast(struct sync_object* sync)
{
    if (!sync)
    {
        return -1;
    }

#if defined(_WIN32)
    EnterCriticalSection(&(sync->m_mutex));
#elif defined(__STDC_NO_THREADS__)
    pthread_mutex_lock(&(sync->m_mutex));
#else
    mtx_lock(&(sync->m_mutex));
#endif

    sync->m_broadcasted = true;
    sync->m_signaled = true;

#if defined(_WIN32)
    LeaveCriticalSection(&(sync->m_mutex));
#elif defined(__STDC_NO_THREADS__)
    pthread_mutex_unlock(&(sync->m_mutex));
#else
    mtx_unlock(&(sync->m_mutex));
#endif

#if defined(_WIN32)
    WakeAllConditionVariable(&(sync->m_cond));
#elif defined(__STDC_NO_THREADS__)
    pthread_cond_broadcast(&(sync->m_cond));
#else
    cnd_broadcast(&(sync->m_cond));
#endif

    return 0;
}

int sync_object_wait_for_signal(struct sync_object* sync)
{
    if (!sync)
    {
        return -1;
    }

#if defined(_WIN32)

    EnterCriticalSection(&(sync->m_mutex));
    while (!sync->m_signaled) /* loop to detect spurious wakes */
    {
        if (!SleepConditionVariableCS(&(sync->m_cond), &(sync->m_mutex), INFINITE))
        {
            break; // don't loop
        }
    }
    if (!sync->m_broadcasted)
    {
        /* reset signal, other waiters can sleep */
        sync->m_signaled = sync->m_stop;
    }
    LeaveCriticalSection(&(sync->m_mutex));

#elif defined(__STDC_NO_THREADS__)

    pthread_mutex_lock(&(sync->m_mutex));
    while (!sync->m_signaled) /* loop to detect spurious wakes */
    {
        if (0 != pthread_cond_wait(&(sync->m_cond), &(sync->m_mutex)))
        {
            break; // exit loop in case of error
        }
    }
    if (!sync->m_broadcasted)
    {
        /* reset signal, other waiters can sleep */
        sync->m_signaled = sync->m_stop;
    }
    pthread_mutex_unlock(&(sync->m_mutex));

#else

    mtx_lock(&(sync->m_mutex));
    while (!sync->m_signaled) /* loop to detect spurious wakes */
    {
        if (thrd_success != cnd_wait(&(sync->m_cond), &(sync->m_mutex)))
        {
            break; // exit loop in case of error
        }
    }
    if (!sync->m_broadcasted)
    {
        /* reset signal, other waiters can sleep */
        sync->m_signaled = sync->m_stop;
    }
    mtx_unlock(&(sync->m_mutex));

#endif

    return 0;
}

int sync_object_wait_for_signal_timed(struct sync_object* sync, unsigned long timeout_us)
{
    if (!sync)
    {
        return -1;
    }

#if defined(_WIN32)

    const unsigned long timeout_ms = timeout_us / 1000;
    EnterCriticalSection(&(sync->m_mutex));
    while (!sync->m_signaled) /* loop to detect spurious wakes */
    {
        if (!SleepConditionVariableCS(&(sync->m_cond), &(sync->m_mutex), timeout_ms))
        {
            break; // timeout
        }
    }
    if (!sync->m_broadcasted)
    {
        /* reset signal, other waiters can sleep */
        sync->m_signaled = sync->m_stop;
    }
    LeaveCriticalSection(&(sync->m_mutex));

#elif defined(__STDC_NO_THREADS__)

    struct timespec timeout;
    clock_gettime(CLOCK_MONOTONIC, &timeout);
    timeout.tv_nsec += timeout_us * 1000;

    pthread_mutex_lock(&(sync->m_mutex));
    while (!sync->m_signaled) /* loop to detect spurious wakes */
    {
        if (0 != pthread_cond_timedwait(&(sync->m_cond), &(sync->m_mutex), &timeout))
        {
            break; // timeout (returned ETIMEDOUT) or other error
        }
    }
    if (!sync->m_broadcasted)
    {
        /* reset signal, other waiters can sleep */
        sync->m_signaled = sync->m_stop;
    }
    pthread_mutex_unlock(&(sync->m_mutex));

#else

    struct timespec timeout;
    clock_gettime(CLOCK_MONOTONIC, &timeout);
    timeout.tv_nsec += timeout_us * 1000;

    mtx_lock(&(sync->m_mutex));
    while (!sync->m_signaled) /* loop to detect spurious wakes */
    {
        if (thrd_success != cnd_timedwait(&(sync->m_cond), &(sync->m_mutex), &timeout))
        {
            break; // timeout (returned ETIMEDOUT) or other error
        }
    }
    if (!sync->m_broadcasted)
    {
        /* reset signal, other waiters can sleep */
        sync->m_signaled = sync->m_stop;
    }
    mtx_unlock(&(sync->m_mutex));

#endif

    return 0;
}
