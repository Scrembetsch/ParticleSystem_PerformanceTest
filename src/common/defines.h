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
#define CS  1
#define TF  0

#define SORT 1

#if CPU
	#define PARALLEL 0
	#define INSTANCE 0
#if not INSTANCE
	#define INDEXED  1
#endif
#endif

#if CS
	#define USE_STRUCT 0
#endif

#ifdef _DEBUG
	#define MAX_PARTICLES (1024)
	#define NUM_TO_GENERATE 100
	#define WORK_GROUP_SIZE 256
#else
	#define MAX_PARTICLES (1024 * 1024 * 2)
	#define NUM_TO_GENERATE 300000
	#define WORK_GROUP_SIZE 256
#endif

#define NUM_CPU_THREADS (std::thread::hardware_concurrency() / 2)