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

#if !defined(__SYNC_OBJECT_H__)
#define __SYNC_OBJECT_H__

#include "atomic_helper.h"

#include <stdbool.h>

#if defined(_WIN32)
#include <windows.h>
#elif defined(__STDC_NO_THREADS__)
#include <pthread.h>
#include <unistd.h>
#else
#include <threads.h>
#endif

#if defined(__cplusplus)
extern "C"
{
#endif

    struct sync_object
    {
        bool m_stop;
        bool m_signaled;
        bool m_broadcasted;

#if defined(_WIN32)
        CRITICAL_SECTION m_mutex;
        CONDITION_VARIABLE m_cond;
#elif defined(__STDC_NO_THREADS__)
        pthread_mutex_t m_mutex;
        pthread_cond_t m_cond;
        pthread_condattr_t m_cond_attr;
#else
        mtx_t m_mutex;
        cnd_t m_cond;
#endif
    };

#if defined(SYNC_OBJECT_IMPLEM)
#define EXTERN_SYNC_OBJECT
#else
#define EXTERN_SYNC_OBJECT extern
#endif

    EXTERN_SYNC_OBJECT int init_sync_object(struct sync_object* sync, bool initial_state);
    EXTERN_SYNC_OBJECT int deinit_sync_object(struct sync_object* sync);

    EXTERN_SYNC_OBJECT int sync_object_signal(struct sync_object* sync);
    EXTERN_SYNC_OBJECT int sync_object_broadcast(struct sync_object* sync);
    EXTERN_SYNC_OBJECT int sync_object_wait_for_signal(struct sync_object* sync);
    EXTERN_SYNC_OBJECT int sync_object_wait_for_signal_timed(struct sync_object* sync, unsigned long timeout_us);

#if defined(__cplusplus)
};
#endif

#endif //  __SYNC_OBJECT_H__
