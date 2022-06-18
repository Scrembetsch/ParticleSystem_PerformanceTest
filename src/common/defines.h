#pragma once

#include <thread>
#ifdef _WIN32
	#include "optick/optick.h"
#else
	#define OPTICK_EVENT(...)
	#define OPTICK_SET_MEMORY_ALLOCATOR(...)
	#define OPTICK_THREAD(...)
	#define OPTICK_FRAME(...)
#endif

#define CPU 1
#define CS  0
#define TF  0
#define FS  0

#define SORT 1

#if CPU
	#define PARALLEL 1
	#define INSTANCE 0
#if not INSTANCE
	#define INDEXED  1
#endif
#endif

#define WORK_GROUP_SIZE 256

#define NUM_CPU_THREADS (std::thread::hardware_concurrency() / 2)
//#define NUM_CPU_THREADS 2