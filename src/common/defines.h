#pragma once

#include <thread>
#include "optick/optick.h"

#define CPU 0
#define CS  0
#define TF  1

#define SORT 1

#if CPU
	#define PARALLEL 0
	#define INSTANCE 1
#endif

#ifdef _DEBUG
	#define MAX_PARTICLES (1024)
	#define NUM_TO_GENERATE 5
	#define WORK_GROUP_SIZE 256
#else
	#define MAX_PARTICLES (1024 * 1024)
	#define NUM_TO_GENERATE 100000
	#define WORK_GROUP_SIZE 256
#endif

#define NUM_CPU_THREADS (std::thread::hardware_concurrency())