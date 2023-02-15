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
#define RING_BUFFER_MPMC_IMPLEM
#include "ring_buffer_mpmc.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#if defined(_WIN32)
#include <windows.h>
#elif defined(__STDC_NO_THREADS__)
#include <pthread.h>
#else
#include <threads.h>
#endif


int init_ring_buffer_mpmc(struct ring_buffer_mpmc* fifo)
{
    if (!fifo)
    {
        return -1;
    }

    memset((void*)(fifo->m_buffer), 0, sizeof(fifo->m_buffer));
    sync_atomic_store(fifo->m_read_idx, 0ULL);
    sync_atomic_store(fifo->m_write_idx, 0ULL);
    sync_atomic_store(fifo->m_reading, false);
    sync_atomic_store(fifo->m_writing, false);
    sync_write_release();

#if defined(_WIN32)
    InitializeCriticalSection(&(fifo->m_read_mutex));
    InitializeCriticalSection(&(fifo->m_write_mutex));
#elif defined(__STDC_NO_THREADS__)
    if (0 != pthread_mutex_init(&(fifo->m_read_mutex), NULL))
    {
        return -1;
    }

    if (0 != pthread_mutex_init(&(fifo->m_write_mutex), NULL))
    {
        return -1;
    }
#else
    if (thrd_success != mtx_init(&(fifo->m_read_mutex), mtx_plain))
    {
        return -1;
    }

    if (thrd_success != mtx_init(&(fifo->m_write_mutex), mtx_plain))
    {
        return -1;
    }
#endif

    return 0;
}

int deinit_ring_buffer_mpmc(struct ring_buffer_mpmc* fifo)
{
    if (!fifo)
    {
        return -1;
    }

#if defined(_WIN32)
    DeleteCriticalSection(&(fifo->m_read_mutex));
    DeleteCriticalSection(&(fifo->m_write_mutex));
#elif defined(__STDC_NO_THREADS__)
    pthread_mutex_destroy(&(fifo->m_read_mutex));
    pthread_mutex_destroy(&(fifo->m_write_mutex));
#else
    mtx_destroy(&(fifo->m_read_mutex));
    mtx_destroy(&(fifo->m_write_mutex));
#endif

    return 0;
}

bool ring_buffer_push_sp(struct ring_buffer_mpmc* fifo, void* elem)
{
    if (!fifo || !elem)
    {
        return -1;
    }

    sync_read_acquire();
    const long long snap_write_idx = sync_atomic_load(fifo->m_write_idx);
    const long long snap_read_idx = sync_atomic_load(fifo->m_read_idx);

    /* is full ? */
    if ((snap_read_idx & RING_BUFFER_MASK) == ((snap_write_idx + 1ULL) & RING_BUFFER_MASK))
    {
        return false;
    }

    /* getting close or wrap around, risk of race condition */
    if (((snap_write_idx - snap_read_idx) <= 2) || (snap_write_idx < snap_read_idx))
    {
        do
        {
            sync_read_acquire();
        } while (sync_atomic_load(fifo->m_reading));
    }

    sync_atomic_store(fifo->m_writing, true);
    sync_read_write();
    const long long write_idx = sync_atomic_inc_64(fifo->m_write_idx);
    sync_atomic_store(fifo->m_buffer[write_idx & RING_BUFFER_MASK], (uintptr_t)elem);
    sync_atomic_store(fifo->m_writing, false);

    return true;
}

bool ring_buffer_push_mp(struct ring_buffer_mpmc* fifo, void* elem)
{
    if (!fifo || !elem)
    {
        return -1;
    }

#if defined(_WIN32)
    EnterCriticalSection(&(fifo->m_write_mutex));
#elif defined(__STDC_NO_THREADS__)
    pthread_mutex_lock(&(fifo->m_write_mutex));
#else
    mtx_lock(&(fifo->m_write_mutex));
#endif

    bool ret = ring_buffer_push_sp(fifo, elem);

#if defined(_WIN32)
    LeaveCriticalSection(&(fifo->m_write_mutex));
#elif defined(__STDC_NO_THREADS__)
    pthread_mutex_unlock(&(fifo->m_write_mutex));
#else
    mtx_unlock(&(fifo->m_write_mutex));
#endif

    return ret;
}

bool ring_buffer_pop_sc(struct ring_buffer_mpmc* fifo, void** elem)
{
    if (!fifo || !elem)
    {
        return -1;
    }

    sync_read_acquire();
    const long long snap_write_idx = sync_atomic_load(fifo->m_write_idx);
    const long long snap_read_idx = sync_atomic_load(fifo->m_read_idx);

    /* is empty ? */
    if ((snap_read_idx & RING_BUFFER_MASK) == (snap_write_idx & RING_BUFFER_MASK))
    {
        return false;
    }

    /* getting close or wrap around, risk of race condition */
    if (((snap_write_idx - snap_read_idx) <= 2) || (snap_write_idx < snap_read_idx))
    {
        do
        {
            sync_read_acquire();
        } while (sync_atomic_load(fifo->m_writing));
    }

    sync_atomic_store(fifo->m_reading, true);
    sync_read_write();
    const long long read_idx = sync_atomic_inc_64(fifo->m_read_idx);

#if INTPTR_MAX == INT64_MAX
    /* 64 bit arch */
    *elem = (void*)sync_atomic_exchange_64(fifo->m_buffer[read_idx & RING_BUFFER_MASK], 0ULL);
#elif INTPTR_MAX == INT32_MAX
    /* 32 bit arch */
    *elem = (void*)sync_atomic_exchange_32(fifo->m_buffer[read_idx & RING_BUFFER_MASK], 0UL);
#else
    /* unsupported */
#endif

    sync_atomic_store(fifo->m_reading, false);
    sync_write_release();

    return (*elem == NULL) ? false : true;
}

bool ring_buffer_pop_mc(struct ring_buffer_mpmc* fifo, void** elem)
{
    if (!fifo || !elem)
    {
        return -1;
    }

#if defined(_WIN32)
    EnterCriticalSection(&(fifo->m_read_mutex));
#elif defined(__STDC_NO_THREADS__)
    pthread_mutex_lock(&(fifo->m_read_mutex));
#else
    mtx_lock(&(fifo->m_read_mutex));
#endif

    bool ret = ring_buffer_pop_sc(fifo, elem);

#if defined(_WIN32)
    LeaveCriticalSection(&(fifo->m_read_mutex));
#elif defined(__STDC_NO_THREADS__)
    pthread_mutex_unlock(&(fifo->m_read_mutex));
#else
    mtx_unlock(&(fifo->m_read_mutex));
#endif

    return ret;
}
