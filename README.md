# CRingBuffer MPMC

Portable C99/C11 implementation of a Ring Buffer with single consumer/producer (lock-free SPSC)
and multiple consumers/producers support (SPMC, MPMC, MPSC).

https://github.com/type-one/CRingBuffer_MPMC

This implementation is provided "as is", without warranty of any kind, express or implied.

# What

This is a fast FIFO structure I implemented for my own tests in 2020, with some cleanups
to publish it on github.  I use this kind of structure to dispatch events, jobs, frames
to process (sound streaming/bitmaps to display) as well as the base for fixed-size
memory pools with recycled memory blocks.

It typically allows concurrent threads to produce frames/messages (given by pointers) and other
concurrent threads to consume these pointers with a minimum overhead.  

The single consumer/producer variant is a lock-free implementation.

The multiple consumers/producers variant uses a pair of mutexes, one used only for concurrent
readers and the other used only for concurrent writers.  To prevent dead-locks the pair of mutexes
cannot be simultaneously holded by a producer or a consumer.

The lock-free operations are using strong memory model, so it should work on single-core/multi-core
x86, arm and ppc.  I did use a similar algorithm in an older C++98/C++11 implementation that is
running fine on core i5/i7, amd ryzen/amd jaguar, raspberry pi 2/3/4, tegra k1/x1, ppc64 (cell, xenon).

Compiled and tested with Visual Studio/MSVC 2019/2022 (win32), gcc 9 (linux),
clang 10 (linux). Older and newer C compilers supporting C99 or C11 should work as well,
as long as the target platform is posix compatible or win32 compliant.

Support win32, posix and standard C11 threads.
Support standard C11 atomics and gcc/clang/win32 legacy atomic intrinsics.

# Why

This structure can be used for classic producers/consumers cases, where you chain several
production/consumption nodes for frame-processing, or for a task/messaging system where you have
a threads pool consuming tasks, with threads that can themselves produce other sub-tasks.

It was also an opportunity to compare Win32, POSIX and C11 portable threads implementations.

# How
Can be compiled and Linux and Windows, and should be easily adapted for other POSIX platforms.

On Linux, just use cmake .
On Windows, just use cmake-gui to generate a Visual Studio solution

Launch in a shell using "cringbuffer_mpsc.exe" or "cringbuffer_mpsc"

In **main.c** you can comment/uncomment some parameters to compile the example with different
producer/consumer scenarios.

- single producer, single consumer, running as fast as possible without blocking (lock-free)

- multiple producers, multiple consumers, running as fast as possible without waiting

- multiple producers, multiple consumers, wait for readers, wait for writers, simulate work load

- single producer, multiple consumers, no wait for readers, no wait for writers, simulate work load

- single producer, multiple consumers, wait for readers, wait for writers, simulate work load

In **ring_buffer_mpmc.h** you can edit *RING_BUFFER_POW2* to grow up or shrink the ring buffer size.
Growing this buffer can help to avoid buffer full situations when 'no wait' is used at producer side.

# Author
Laurent Lardinois / Type One (TFL-TDV)

https://be.linkedin.com/in/laurentlardinois

https://demozoo.org/sceners/19691/
