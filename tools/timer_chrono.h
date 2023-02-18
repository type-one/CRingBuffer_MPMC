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

#if !defined(__TIMER_CHRONO_H__)
#define __TIMER_CHRONO_H__

#if defined(_WIN32)
#include <windows.h>
#elif defined(__MACH__)
#include <mach/mach.h>
#include <mach/mach_time.h>
#elif defined(__unix__) || defined(__linux__)
/* _POSIX_VERSION */
#include <inttypes.h>
#include <time.h>
#include <unistd.h>
#endif

#include <stdint.h>

#if defined(__cplusplus)
extern "C"
{
#endif

    struct timer_chrono
    {
#if defined(_WIN32)

        LARGE_INTEGER m_qw_time;
        LARGE_INTEGER m_qw_app_time;
        double m_fapp_time;
        double m_felapsed_time;
        double m_fsecs_per_tick;

#elif defined(__MACH__)

        uint64_t m_start;
        mach_timebase_info_data_t m_info;

#elif defined(__unix__) || defined(__linux__)

        struct timespec m_start;

#endif
    };


#if defined(TIMER_CHRONO_IMPLEM)
#define EXTERN_TIMER_CHRONO
#else
#define EXTERN_TIMER_CHRONO extern
#endif

    EXTERN_TIMER_CHRONO int init_timer_chrono(struct timer_chrono* ctxt);
    EXTERN_TIMER_CHRONO double timer_chrono_current_time_ms(struct timer_chrono* ctxt);

#if defined(__cplusplus)
};
#endif

#endif //  __TIMER_CHRONO_H__
