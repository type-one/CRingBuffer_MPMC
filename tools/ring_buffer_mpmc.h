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

#pragma once

#if !defined(__RING_BUFFER_MPMC_H__)
#define __RING_BUFFER_MPMC_H__

#include "atomic_helper.h"

#include <stdbool.h>
#include <stdint.h>

#if defined(_WIN32)
#include <windows.h>
#elif defined(__STDC_NO_THREADS__)
#include <pthread.h>
#else
#include <threads.h>
#endif

#if defined(RING_BUFFER_MPMC_IMPLEM)
#define EXTERN_RING_BUFFER_MPMC
#else
#define EXTERN_RING_BUFFER_MPMC extern
#endif

#if defined(__cplusplus)
extern "C"
{
#endif

#define RING_BUFFER_POW2 12U /* 2^x entries, can be growed up for no waits situations to limit buffer full cases */
#define RING_BUFFER_SIZE (1ULL << RING_BUFFER_POW2)
#define RING_BUFFER_MASK (RING_BUFFER_SIZE - 1ULL)

    struct ring_buffer_mpmc
    {
        _atomic_uintptr m_buffer[RING_BUFFER_SIZE];
        _atomic_llong m_read_idx;
        _atomic_llong m_write_idx;
        _atomic_bool m_reading;
        _atomic_bool m_writing;

#if defined(_WIN32)
        CRITICAL_SECTION m_read_mutex;
        CRITICAL_SECTION m_write_mutex;
#elif defined(__STDC_NO_THREADS__)
        pthread_mutex_t m_read_mutex;
        pthread_mutex_t m_write_mutex;
#else
        mtx_t m_read_mutex;
        mtx_t m_write_mutex;
#endif
    };

    EXTERN_RING_BUFFER_MPMC int init_ring_buffer_mpmc(struct ring_buffer_mpmc* fifo);
    EXTERN_RING_BUFFER_MPMC int deinit_ring_buffer_mpmc(struct ring_buffer_mpmc* fifo);
    EXTERN_RING_BUFFER_MPMC bool ring_buffer_push_sp(struct ring_buffer_mpmc* fifo, void* elem);
    EXTERN_RING_BUFFER_MPMC bool ring_buffer_push_mp(struct ring_buffer_mpmc* fifo, void* elem);
    EXTERN_RING_BUFFER_MPMC bool ring_buffer_pop_sc(struct ring_buffer_mpmc* fifo, void** elem);
    EXTERN_RING_BUFFER_MPMC bool ring_buffer_pop_mc(struct ring_buffer_mpmc* fifo, void** elem);

#endif /*  __RING_BUFFER_MPMC_H__ */
