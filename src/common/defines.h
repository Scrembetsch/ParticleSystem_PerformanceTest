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

#define CPU 0
#define CS  0
#define TF  0
#define FS  1

#define SORT 0

#if CPU
	#define PARALLEL 0
	#define INSTANCE 0
#if not INSTANCE
	#define INDEXED  0
#endif
#endif

#if CS
	#define USE_STRUCT 0
#endif

#define WORK_GROUP_SIZE 128

#define NUM_CPU_THREADS (std::thread::hardware_concurrency() / 2)