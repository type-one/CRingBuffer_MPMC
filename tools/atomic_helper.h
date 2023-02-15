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

#if !defined(__ATOMIC_HELPER_H__)
#define __ATOMIC_HELPER_H__

#if !defined(__STDC_NO_ATOMICS__)
#if defined(__cplusplus)
#include <atomic>
#define _Atomic(x) std::atomic<x>
#else
#include <stdatomic.h>
#endif
#endif

#include <stdbool.h>
#include <stdint.h>
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
#endif

#if defined(__cplusplus)
extern "C"
{
#endif

#if !defined(__STDC_NO_ATOMICS__)
#define _atomic_bool atomic_bool
#define _atomic_ulong atomic_ulong
#define _atomic_ullong atomic_ullong
#define _atomic_llong atomic_llong
#define _atomic_uintptr atomic_uintptr_t
#else
#define _atomic_bool volatile bool
#define _atomic_ulong volatile unsigned long
#define _atomic_ullong volatile unsigned long long
#define _atomic_llong volatile long long
#define _atomic_uintptr volatile uintptr_t
#endif

#if !defined(__STDC_NO_ATOMICS__)
#define sync_read_acquire()
#define sync_write_release()
#define sync_read_write()
#define sync_atomic_inc_32(ref) atomic_fetch_add(&(ref), 1)
#define sync_atomic_inc_64(ref) atomic_fetch_add(&(ref), 1)
#define sync_atomic_load(ref) atomic_load(&(ref))
#define sync_atomic_store(ref, val) atomic_store(&ref, val)
#define sync_atomic_exchange_32(ref, val) atomic_exchange(&ref, val)
#define sync_atomic_exchange_64(ref, val) atomic_exchange(&ref, val)
#elif defined(_WIN32)
#define sync_read_acquire() _ReadBarrier()
#define sync_write_release() _WriteBarrier()
#define sync_read_write() _ReadWriteBarrier()
#define sync_atomic_inc_32(ref) InterlockedIncrementAcquire(&(ref))
#define sync_atomic_inc_64(ref) InterlockedIncrementAcquire64(&(ref))
#define sync_atomic_load(ref) (ref)
#define sync_atomic_store(ref, val) (ref = val)
#define sync_atomic_exchange_32(ref, val) InterlockedExchange(&ref, val)
#define sync_atomic_exchange_64(ref, val) InterlockedExchange64(&ref, val)
#else
// fallback: assuming GCC/Clang
#define sync_read_acquire() __sync_synchronize()
#define sync_write_release() __sync_synchronize()
#define sync_read_write() __sync_synchronize()
#define sync_atomic_inc_32(ref) __sync_fetch_and_add(&(ref), 1)
#define sync_atomic_inc_64(ref) __sync_fetch_and_add(&(ref), 1)
#define sync_atomic_load(ref) (ref)
#define sync_atomic_store(ref, val) (ref = val)
#if defined(__clang__)
#define sync_atomic_exchange_32(ref, val) __sync_swap(&ref, val)
#define sync_atomic_exchange_64(ref, val) __sync_swap(&ref, val)
#else
#define sync_atomic_exchange_32(ref, val) __sync_lock_test_and_set(&ref, val)
#define sync_atomic_exchange_64(ref, val) __sync_lock_test_and_set(&ref, val)
#endif
#endif

#if defined(__cplusplus)
};
#endif

#endif //  __ATOMIC_HELPER_H__
