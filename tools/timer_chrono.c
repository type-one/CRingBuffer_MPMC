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

#define TIMER_CHRONO_IMPLEM
#include "timer_chrono.h"

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

int init_timer_chrono(struct timer_chrono* ctxt)
{
    if (!ctxt)
    {
        return -1;
    }

#if defined(_WIN32)
    /* Get the frequency of the timer */
    LARGE_INTEGER qw_ticks_per_sec;
    QueryPerformanceFrequency(&qw_ticks_per_sec);
    ctxt->m_fsecs_per_tick = 1.0 / (double)qw_ticks_per_sec.QuadPart;

    /* Save the start time */
    QueryPerformanceCounter(&(ctxt->m_qw_time));

#elif defined(__MACH__)

    mach_timebase_info(&(context->m_info));
    context->m_start = mach_absolute_time();

#elif defined(__unix__) || defined(__linux__)

    clock_gettime(CLOCK_REALTIME, &(ctxt->m_start));

#endif

    return 0;
}

double timer_chrono_current_time_ms(struct timer_chrono* ctxt)
{
    if (!ctxt)
    {
        return 0.0;
    }

    double current_time = 0.0;

#if defined(_WIN32)

    LARGE_INTEGER qw_new_time;
    LARGE_INTEGER qw_delta_time;

    QueryPerformanceCounter(&qw_new_time);
    qw_delta_time.QuadPart = qw_new_time.QuadPart - ctxt->m_qw_time.QuadPart;

    ctxt->m_qw_app_time.QuadPart += qw_delta_time.QuadPart;
    ctxt->m_qw_time.QuadPart = qw_new_time.QuadPart;

    ctxt->m_felapsed_time = ctxt->m_fsecs_per_tick * ((double)(qw_delta_time.QuadPart));
    ctxt->m_fapp_time = ctxt->m_fsecs_per_tick * ((double)(ctxt->m_qw_app_time.QuadPart));

    current_time = ctxt->m_fapp_time * 1000.0;


#elif defined(__MACH__)

    uint64_t a_time = mach_absolute_time() - ctxt->m_start;
    /* return in ms (cpu cycles to ns to ms) */
    current_time = (a_time * ctxt->m_info.numer / ctxt->m_info.denom) / 1000000.0;

#elif defined(__unix__) || defined(__linux__)

    struct timespec spec;
    clock_gettime(CLOCK_MONOTONIC, &spec);
    time_t s = spec.tv_sec;                             /* seconds */
    current_time = s * 1000.0 + (spec.tv_nsec / 1.0e6); /* convert nanoseconds to ms */

#endif

    return current_time;
}
